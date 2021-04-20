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

const governance_object& database::get_governance( const account_name_type& account )const
{ try {
	return get< governance_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const governance_object* database::find_governance( const account_name_type& account )const
{
   return find< governance_object, by_account >( account );
}

const governance_permission_object& database::get_governance_permission( const account_name_type& account )const
{ try {
	return get< governance_permission_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const governance_permission_object* database::find_governance_permission( const account_name_type& account )const
{
   return find< governance_permission_object, by_account >( account );
}

const governance_executive_object& database::get_governance_executive( const account_name_type& governance, const account_name_type& executive )const
{ try {
	return get< governance_executive_object, by_governance_executive >( boost::make_tuple( governance, executive ) );
} FC_CAPTURE_AND_RETHROW( (governance)(executive) ) }

const governance_executive_object* database::find_governance_executive( const account_name_type& governance, const account_name_type& executive )const
{
   return find< governance_executive_object, by_governance_executive >( boost::make_tuple( governance, executive ) );
}

const governance_executive_vote_object& database::get_governance_executive_vote( const account_name_type& director, const account_name_type& governance )const
{ try {
	return get< governance_executive_vote_object, by_director_governance >( boost::make_tuple( director, governance ) );
} FC_CAPTURE_AND_RETHROW( (director)(governance) ) }

const governance_executive_vote_object* database::find_governance_executive_vote( const account_name_type& director, const account_name_type& governance )const
{
   return find< governance_executive_vote_object, by_director_governance >( boost::make_tuple( director, governance ) );
}

const governance_director_object& database::get_governance_director( const account_name_type& governance, const account_name_type& director )const
{ try {
	return get< governance_director_object, by_governance_director >( boost::make_tuple( governance, director ) );
} FC_CAPTURE_AND_RETHROW( (governance)(director) ) }

const governance_director_object* database::find_governance_director( const account_name_type& governance, const account_name_type& director )const
{
   return find< governance_director_object, by_governance_director >( boost::make_tuple( governance, director ) );
}

const governance_director_vote_object& database::get_governance_director_vote( const account_name_type& account, const account_name_type& governance )const
{ try {
	return get< governance_director_vote_object, by_account_governance >( boost::make_tuple( account, governance ) );
} FC_CAPTURE_AND_RETHROW( (account)(governance) ) }

const governance_director_vote_object* database::find_governance_director_vote( const account_name_type& account, const account_name_type& governance )const
{
   return find< governance_director_vote_object, by_account_governance >( boost::make_tuple( account, governance ) );
}

const governance_member_object& database::get_governance_member( const account_name_type& account )const
{ try {
	return get< governance_member_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const governance_member_object* database::find_governance_member( const account_name_type& account )const
{
   return find< governance_member_object, by_account >( account );
}

const governance_member_request_object& database::get_governance_member_request( const account_name_type& account )const
{ try {
	return get< governance_member_request_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const governance_member_request_object* database::find_governance_member_request( const account_name_type& account )const
{
   return find< governance_member_request_object, by_account >( account );
}

const governance_resolution_object& database::get_governance_resolution( const account_name_type& governance, const string& resolution_id, const string& ammendment_id )const
{ try {
	return get< governance_resolution_object, by_resolution_id >( boost::make_tuple( governance, resolution_id, ammendment_id ) );
} FC_CAPTURE_AND_RETHROW( (governance)(resolution_id)(ammendment_id) ) }

const governance_resolution_object* database::find_governance_resolution( const account_name_type& governance, const string& resolution_id, const string& ammendment_id )const
{
   return find< governance_resolution_object, by_resolution_id >( boost::make_tuple( governance, resolution_id, ammendment_id ) );
}

const governance_resolution_object& database::get_governance_resolution( const account_name_type& governance, const shared_string& resolution_id, const shared_string& ammendment_id )const
{ try {
	return get< governance_resolution_object, by_resolution_id >( boost::make_tuple( governance, resolution_id, ammendment_id ) );
} FC_CAPTURE_AND_RETHROW( (governance)(resolution_id)(ammendment_id) ) }

const governance_resolution_object* database::find_governance_resolution( const account_name_type& governance, const shared_string& resolution_id, const shared_string& ammendment_id )const
{
   return find< governance_resolution_object, by_resolution_id >( boost::make_tuple( governance, resolution_id, ammendment_id ) );
}

const governance_resolution_vote_object& database::get_governance_resolution_vote( const account_name_type& account, const account_name_type& governance, const string& resolution_id, const string& ammendment_id )const
{ try {
	return get< governance_resolution_vote_object, by_account_resolution_id >( boost::make_tuple( account, governance, resolution_id, ammendment_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(governance)(resolution_id)(ammendment_id) ) }

const governance_resolution_vote_object* database::find_governance_resolution_vote( const account_name_type& account, const account_name_type& governance, const string& resolution_id, const string& ammendment_id )const
{
   return find< governance_resolution_vote_object, by_account_resolution_id >( boost::make_tuple( account, governance, resolution_id, ammendment_id ) );
}

const governance_resolution_vote_object& database::get_governance_resolution_vote( const account_name_type& account, const account_name_type& governance, const shared_string& resolution_id, const shared_string& ammendment_id )const
{ try {
	return get< governance_resolution_vote_object, by_account_resolution_id >( boost::make_tuple( account, governance, resolution_id, ammendment_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(governance)(resolution_id)(ammendment_id) ) }

const governance_resolution_vote_object* database::find_governance_resolution_vote( const account_name_type& account, const account_name_type& governance, const shared_string& resolution_id, const shared_string& ammendment_id )const
{
   return find< governance_resolution_vote_object, by_account_resolution_id >( boost::make_tuple( account, governance, resolution_id, ammendment_id ) );
}


/**
 * Updates the voting statistics, directors, and chief executive of the governance.
 */
void database::update_governance( const governance_object& governance )
{ try {
   const auto& governance_director_vote_idx = get_index< governance_director_vote_index >().indices().get< by_governance_account >();
   const auto& governance_executive_vote_idx = get_index< governance_executive_vote_index >().indices().get< by_director_governance >();
   const auto& governance_director_idx = get_index< governance_director_index >().indices().get< by_governance_director >();
   const auto& account_balance_idx = get_index< account_balance_index >().indices().get< by_symbol >();
   time_point now = head_block_time();

   const governance_permission_object& governance_permission = get_governance_permission( governance.account );
   flat_map< account_name_type, share_type > director_map;
   flat_map< account_name_type, share_type > executive_map;
   flat_set< account_name_type > director_set;
   flat_set< account_name_type > executive_set;
   vector< pair< account_name_type, share_type > > director_rank;
   vector< pair< account_name_type, share_type > > executive_rank;

   ilog( "Updating governance: \n ${g} \n",
      ("g",governance));
   
   auto governance_director_vote_itr = governance_director_vote_idx.lower_bound( governance.account );

   while( governance_director_vote_itr != governance_director_vote_idx.end() &&
      governance_director_vote_itr->governance == governance.account )
   {
      const account_object& voter = get_account( governance_director_vote_itr->account );
      share_type weight = get_equity_voting_power( governance_director_vote_itr->account, governance );

      while( governance_director_vote_itr != governance_director_vote_idx.end() && 
         governance_director_vote_itr->governance == governance.account &&
         governance_director_vote_itr->account == voter.name )
      {
         const governance_director_vote_object& vote = *governance_director_vote_itr;
         ++governance_director_vote_itr;

         ilog( "Governance: ${g} director vote (weight: ${w}): \n ${m} \n",
            ("g",governance.account)("w",weight)("m",vote));

         if( weight >= BLOCKCHAIN_PRECISION )
         {
            if( director_map.count( vote.director ) )
            {
               director_map[ vote.director ] += share_type( 1 );
            }
            else
            {
               director_map[ vote.director ] = share_type( 1 );
            }
         }
      }
   }

   ilog( "Governance: ${g} director votes: \n ${m} \n",
      ("g",governance.account)("m",director_map));

   auto account_balance_itr = account_balance_idx.lower_bound( governance.equity_asset );

   uint128_t equity_holder_count;

   while( account_balance_itr != account_balance_idx.end() &&
      account_balance_itr->symbol == governance.equity_asset )
   {
      const account_balance_object& balance = *account_balance_itr;
      if( balance.staked_balance >= BLOCKCHAIN_PRECISION )      // Minimum balance to obtain governance vote
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

   auto governance_director_itr = governance_director_idx.lower_bound( governance.account );

   while( governance_director_itr != governance_director_idx.end() &&
      governance_director_itr->governance == governance.account )
   {
      const governance_director_object& director = *governance_director_itr;

      modify( director, [&]( governance_director_object& bdo )
      {
         bdo.appointed = false;     // Reset director appointed to false;
      });

      // If a director is active, and is in the top directors, and votes for an executive that is active, then appoint the director and count the executive vote.

      if( director.active )
      {
         if( std::find( director_set.begin(), director_set.end(), director.director ) != director_set.end() )
         {
            auto governance_executive_vote_itr = governance_executive_vote_idx.find( boost::make_tuple( director.director, governance.account ) );

            if( governance_executive_vote_itr != governance_executive_vote_idx.end() )
            {
               const governance_executive_vote_object& vote = *governance_executive_vote_itr;
               const governance_executive_object& executive = get_governance_executive( governance.account, vote.executive );

               if( executive.active )
               {
                  executive_map[ vote.executive ]++;

                  modify( director, [&]( governance_director_object& bdo )
                  {
                     bdo.appointed = true;
                  });

                  ilog( "Governance: ${g} appoints active director: \n ${d} \n ",
                     ("g",governance.account)("m",director));
               }
            }
         }
      }

      ++governance_director_itr;
   }

   ilog( "Governance: ${g} executive votes: \n ${m} \n",
      ("g",governance.account)("m",executive_map));

   for( auto e : executive_map )
   {
      executive_rank.push_back( std::make_pair( e.first, e.second ) );
   }
   
   std::sort( executive_rank.begin(), executive_rank.end(), [&]( pair< account_name_type, share_type > a, pair< account_name_type, share_type > b )
   {
      return a.second < b.second;  // Ordered vector of all active governance executives.
   });

   account_name_type chief_executive_name = governance_permission.chief_executive;

   auto executive_rank_itr = executive_rank.begin();

   if( executive_rank_itr != executive_rank.end() )
   {
      chief_executive_name = executive_rank_itr->first;
   }

   const governance_executive_object& chief_executive = get_governance_executive( governance.account, chief_executive_name );

   modify( governance_permission, [&]( governance_permission_object& gpo )
   {
      gpo.directors = director_set;
      gpo.chief_executive = chief_executive.executive;
      gpo.last_updated = now;
   });

   ilog( "Updated governance Permissions: \n ${b} \n",
      ("b",governance_permission));

} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the executive board votes and positions of officers in a governance account.
 */
void database::update_governance_set()
{ try {
   if( (head_block_num() % SET_UPDATE_BLOCK_INTERVAL) != 0 )    // Runs once per day
      return;

   const auto& governance_idx = get_index< governance_index >().indices().get< by_account >();
   auto governance_itr = governance_idx.begin();

   while( governance_itr != governance_idx.end() )
   {
      update_governance( *governance_itr );
      ++governance_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


} } //node::chain