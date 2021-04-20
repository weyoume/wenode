
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



//============================//
// === Network Evaluators === //
//============================//


/**
 * Creates or updates a network officer for a member account, 
 * Network officers can earn from the reward pools for each type
 * by providing services to the protocol, and receiving votes from
 * network stakeholders. The top 50 members of each pool earn a proportional share of
 * the reward available each day.
 */
void network_officer_update_evaluator::do_apply( const network_officer_update_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );

   time_point now = _db.head_block_time();
   
   FC_ASSERT( account.membership != membership_tier_type::NONE,
      "Account must be a member to create a network officer." );

   const asset_currency_data_object& currency = _db.get_currency_data( o.reward_currency );
   const network_officer_object* net_off_ptr = _db.find_network_officer( o.account );
   network_officer_role_type role_type = network_officer_role_type::DEVELOPMENT;

   for( size_t i = 0; i < network_officer_role_values.size(); i++ )
   {
      if( o.officer_type == network_officer_role_values[ i ] )
      {
         role_type = network_officer_role_type( i );
         break;
      }
   }

   if( net_off_ptr != nullptr )        // updating existing network officer
   { 
      _db.modify( *net_off_ptr, [&]( network_officer_object& noo )
      {
         noo.officer_type = role_type;       // Selects development, marketing or advocacy
         if( o.url.size() )
         {
            from_string( noo.url, o.url );
         }
         if( o.details.size() )
         {
            from_string( noo.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( noo.json, o.json );
         }
         noo.reward_currency = currency.symbol;
         noo.active = o.active;
      });
   }
   else  // create new network officer
   {
      _db.create< network_officer_object >( [&]( network_officer_object& noo )
      {
         noo.account = o.account;
         noo.officer_type = role_type;    // Selects development, marketing or advocacy
         if( o.url.size() )
         {
            from_string( noo.url, o.url );
         }
         if( o.details.size() )
         {
            from_string( noo.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( noo.json, o.json );
         }
         noo.officer_approved = false;
         noo.reward_currency = currency.symbol;
         noo.created = now;
         noo.active = true;
      });
   } 
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void network_officer_vote_evaluator::do_apply( const network_officer_vote_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );

   const account_object& voter = _db.get_account( o.account );
   const account_object& officer_account = _db.get_account( o.network_officer );
   FC_ASSERT( officer_account.active, 
      "Account: ${s} must be active to vote as officer.",("s", o.network_officer) );
   const network_officer_object& officer = _db.get_network_officer( o.network_officer );
   FC_ASSERT( officer.active, 
      "Account: ${s} must be active to vote as officer.",("s", o.network_officer) );
   const producer_schedule_object& pso = _db.get_producer_schedule();
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   time_point now = _db.head_block_time();

   if( o.approved )
   {
      FC_ASSERT( voter.can_vote,
         "Account has declined its voting rights." );
      FC_ASSERT( officer.active,
         "Network Officer is inactive, and not accepting approval votes at this time." );
   }
   
   const auto& account_type_rank_idx = _db.get_index< network_officer_vote_index >().indices().get< by_account_type_rank >();
   const auto& account_officer_idx = _db.get_index< network_officer_vote_index >().indices().get< by_account_officer >();
   auto account_type_rank_itr = account_type_rank_idx.find( boost::make_tuple( voter.name, officer.officer_type, o.vote_rank) );      // vote at type and rank number
   auto account_officer_itr = account_officer_idx.find( boost::make_tuple( voter.name, o.network_officer ) );       // vote for specified producer

   if( o.approved )       // Adding or modifying vote
   {
      if( account_officer_itr == account_officer_idx.end() && account_type_rank_itr == account_type_rank_idx.end() )     // No vote for producer or type rank, create new vote.
      {
         FC_ASSERT( voter.officer_vote_count < MAX_OFFICER_VOTES,
            "Account has voted for too many network officers." );

         _db.create< network_officer_vote_object >( [&]( network_officer_vote_object& v )
         {
            v.network_officer = officer.account;
            v.officer_type = officer.officer_type;
            v.account = voter.name;
            v.vote_rank = o.vote_rank;
            v.last_updated = now;
            v.created = now;
         });
         
         _db.network_officer_update_votes( voter );
      }
      else
      {
         if( account_officer_itr != account_officer_idx.end() )
         {
            ilog( "Removed: ${v}",("v",*account_officer_itr));
            _db.remove( *account_officer_itr );
         }

         _db.network_officer_update_votes( voter, o.network_officer, officer.officer_type, o.vote_rank );   // Remove existing officer vote, and add at new rank. 
      }
   }
   else  // Removing existing vote
   {
      if( account_officer_itr != account_officer_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*account_officer_itr));
         _db.remove( *account_officer_itr );
      }
      else if( account_type_rank_itr != account_type_rank_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*account_type_rank_itr));
         _db.remove( *account_type_rank_itr );
      }
      _db.network_officer_update_votes( voter );
   }

   _db.network_officer_update( officer, pso, props );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void supernode_update_evaluator::do_apply( const supernode_update_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   
   time_point now = _db.head_block_time();
   const supernode_object* sup_ptr = _db.find_supernode( o.account );

   if( sup_ptr != nullptr )      // updating existing supernode
   { 
      _db.modify( *sup_ptr, [&]( supernode_object& s )
      { 
         if( o.url.size() )
         {
            from_string( s.url, o.url );
         }
         if( o.ipfs_endpoint.size() )
         {
            from_string( s.ipfs_endpoint, o.ipfs_endpoint );
         }
         if( o.node_api_endpoint.size() )
         {
            from_string( s.node_api_endpoint, o.node_api_endpoint );
         }
         if( o.notification_api_endpoint.size() )
         {
            from_string( s.notification_api_endpoint, o.notification_api_endpoint );
         }
         if( o.auth_api_endpoint.size() )
         {
            from_string( s.auth_api_endpoint, o.auth_api_endpoint );
         }
         if( o.bittorrent_endpoint.size() )
         {
            from_string( s.bittorrent_endpoint, o.bittorrent_endpoint );
         }
         if( o.details.size() )
         {
            from_string( s.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( s.json, o.json );
         }
         s.active = o.active;
         if( s.active == false && o.active == true )   // Reactivating Supernode. 
         {
            s.last_activation_time = now;
         }
      });
   }
   else  // create new supernode
   {
      _db.create< supernode_object >( [&]( supernode_object& s )
      {
         s.account = o.account;
         if( o.url.size())
         {
            from_string( s.url, o.url );
         }
         if( o.ipfs_endpoint.size())
         {
            from_string( s.ipfs_endpoint, o.ipfs_endpoint );
         }
         if( o.node_api_endpoint.size())
         {
            from_string( s.node_api_endpoint, o.node_api_endpoint );
         }
         if( o.notification_api_endpoint.size())
         {
            from_string( s.notification_api_endpoint, o.notification_api_endpoint );
         }
         if( o.auth_api_endpoint.size())
         {
            from_string( s.auth_api_endpoint, o.auth_api_endpoint );
         }
         if( o.bittorrent_endpoint.size())
         {
            from_string( s.bittorrent_endpoint, o.bittorrent_endpoint );
         }
         if( o.details.size())
         {
            from_string( s.details, o.details );
         }
         if( o.json.size())
         {
            from_string( s.json, o.json );
         }
         s.active = true;
         s.created = now;
         s.last_updated = now;
         
         s.last_activation_time = now;
      });
   } 
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void interface_update_evaluator::do_apply( const interface_update_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   time_point now = _db.head_block_time();

   FC_ASSERT( account.membership != membership_tier_type::NONE,
      "Account must be a member to create an Interface.");

   const interface_object* int_ptr = _db.find_interface( o.account );

   if( int_ptr != nullptr ) // updating existing interface
   { 
      _db.modify( *int_ptr, [&]( interface_object& i )
      { 
         if( o.url.size() )
         {
            from_string( i.url, o.url );
         }
         if( o.details.size() )
         {
            from_string( i.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( i.json, o.json );
         }
         i.active = o.active;
         i.decay_weights( now );
      });
   }
   else  // create new interface
   {
      _db.create< interface_object >( [&]( interface_object& i )
      {
         i.account = o.account;
         if( o.url.size() )
         {
            from_string( i.url, o.url );
         }
         if( o.details.size() )
         {
            from_string( i.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( i.json, o.json );
         }
         i.active = true;
         i.created = now;
         i.last_updated = now;
      });
   } 
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void mediator_update_evaluator::do_apply( const mediator_update_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   
   time_point now = _db.head_block_time();
   asset liquid = _db.get_liquid_balance( o.account, SYMBOL_COIN );

   FC_ASSERT( account.membership != membership_tier_type::NONE, 
      "Account must be a member to act as a mediator." );
   
   const mediator_object* med_ptr = _db.find_mediator( o.account );

   if( med_ptr != nullptr )      // updating existing mediator
   {
      const mediator_object& mediator = *med_ptr;
      asset delta = o.mediator_bond - mediator.mediator_bond;

      FC_ASSERT( liquid >= delta, 
         "Account has insufficient liquid balance for specified mediator bond." );

      _db.adjust_liquid_balance( o.account, -delta );
      _db.adjust_pending_supply( delta );

      _db.modify( mediator, [&]( mediator_object& m )
      { 
         if( o.url.size() )
         {
            from_string( m.url, o.url );
         }
         if( o.details.size() )
         {
            from_string( m.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( m.json, o.json );
         }
         m.mediator_bond = o.mediator_bond;
         m.last_updated = now;
         m.active = o.active;
      });
   }
   else       // create new mediator
   {
      _db.adjust_liquid_balance( o.account, -o.mediator_bond );
      _db.adjust_pending_supply( o.mediator_bond );

      _db.create< mediator_object >( [&]( mediator_object& m )
      {
         m.account = o.account;
         if( o.url.size() )
         {
            from_string( m.url, o.url );
         }
         if( o.details.size() )
         {
            from_string( m.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( m.json, o.json );
         }
         m.active = true;
         m.created = now;
         m.last_updated = now;
         m.mediator_bond = o.mediator_bond;
      });
   } 
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void enterprise_update_evaluator::do_apply( const enterprise_update_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );

   time_point now = _db.head_block_time();
   const asset_currency_data_object& currency = _db.get_currency_data( o.budget.symbol );

   FC_ASSERT( currency.enterprise_fund_reward_percent > 0,
      "Daily Budget reward currency does not issue to Enterprise proposals." );
   const auto& enterprise_idx = _db.get_index< enterprise_index >().indices().get< by_enterprise_id >();
   auto enterprise_itr = enterprise_idx.find( boost::make_tuple( o.account, o.enterprise_id ) );

   if( enterprise_itr != enterprise_idx.end() )               // Updating or removing existing proposal 
   {
      const enterprise_object& enterprise = *enterprise_itr;
      
      _db.modify( enterprise, [&]( enterprise_object& eo )
      {
         if( o.details.size() )
         {
            from_string( eo.details, o.details );
         }
         if( o.url.size() )
         {
            from_string( eo.url, o.url );
         }
         if( o.json.size() )
         {
            from_string( eo.json, o.json );
         }
         eo.active = o.active;
         eo.last_updated = now;
      });

      ilog( "Account: ${a} edited community enterprise id: ${id}",
         ("a",o.account)("id",o.enterprise_id));
   }
   else        // Create new community enterprise proposal
   {
      _db.create< enterprise_object >( [&]( enterprise_object& eo )
      {
         eo.account = o.account;
         from_string( eo.enterprise_id, o.enterprise_id );

         if( o.details.size() )
         {
            from_string( eo.details, o.details );
         }
         if( o.url.size() )
         {
            from_string( eo.url, o.url );
         }
         if( o.json.size() )
         {
            from_string( eo.json, o.json );
         }

         eo.budget = o.budget;
         eo.distributed = asset( 0, o.budget.symbol );
         eo.total_funding = asset( 0, o.budget.symbol );
         eo.active = true;
         eo.approved = false;
         eo.last_updated = now;
         eo.created = now;
      });

      ilog( "Account: ${a} created community enterprise id: ${id}",
         ("a",o.account)("id",o.enterprise_id));
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void enterprise_vote_evaluator::do_apply( const enterprise_vote_operation& o )
{ try {
   const account_object& voter = _db.get_account( o.voter );
   const account_object& account = _db.get_account( o.account );
   const enterprise_object& enterprise = _db.get_enterprise( o.account, o.enterprise_id );
   time_point now = _db.head_block_time();

   if( o.approved )
   {
      FC_ASSERT( voter.active, 
         "Account: ${a} must be active to broadcast transaction.",
         ("a", o.voter));
      FC_ASSERT( account.active, 
         "Account: ${a} must be active to broadcast transaction.",
         ("a", o.account));
      FC_ASSERT( voter.can_vote,
         "Account: ${a} has declined its voting rights.",
         ("a",o.voter));
      FC_ASSERT( enterprise.active,
         "Enterprise Proposal is inactive, and not accepting approval votes at this time." );
   }
      
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   const producer_schedule_object& producer_schedule = _db.get_producer_schedule();
   const auto& account_rank_idx = _db.get_index< enterprise_vote_index >().indices().get< by_account_rank >();
   const auto& account_enterprise_idx = _db.get_index< enterprise_vote_index >().indices().get< by_account_enterprise >();
   auto account_rank_itr = account_rank_idx.find( boost::make_tuple( o.voter, o.vote_rank ) );
   auto account_enterprise_itr = account_enterprise_idx.find( boost::make_tuple( o.voter, enterprise.account, o.enterprise_id ) );

   if( o.approved )      // Adding or modifying vote
   {
      if( account_enterprise_itr == account_enterprise_idx.end() && 
         account_rank_itr == account_rank_idx.end() )
      {
         const enterprise_vote_object& vote = _db.create< enterprise_vote_object >( [&]( enterprise_vote_object& v )
         {
            v.voter = o.voter;
            v.account = o.account;
            from_string( v.enterprise_id, o.enterprise_id );
            v.vote_rank = o.vote_rank;
            v.last_updated = now;
            v.created = now;
         });
         
         _db.update_enterprise_votes( voter );

         ilog( "Account: ${v} created new Enterprise Vote: \n ${ev} \n",
            ("v",o.voter)("ev",vote));
      }
      else
      {
         if( account_enterprise_itr != account_enterprise_idx.end() )
         {
            ilog( "Removed: ${v}",("v",*account_enterprise_itr));
            _db.remove( *account_enterprise_itr );
         }

         _db.update_enterprise_votes( voter, o.account, o.enterprise_id, o.vote_rank );

         ilog( "Account: ${v} edited enterprise proposal: Account: ${a} ID: ${id} Vote rank: ${r}",
            ("v",o.voter)("a",o.account)("id",o.enterprise_id)("r",o.vote_rank));
      }
   }
   else       // Removing existing vote
   {
      if( account_enterprise_itr != account_enterprise_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*account_enterprise_itr));
         _db.remove( *account_enterprise_itr );
      }
      else if( account_rank_itr != account_rank_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*account_rank_itr));
         _db.remove( *account_rank_itr );
      }
      _db.update_enterprise_votes( voter );
      ilog( "Account: ${v} removed enterprise proposal: Account: ${a} ID: ${id} Vote rank: ${r}",
         ("v",o.voter)("a",o.account)("id",o.enterprise_id)("r",o.vote_rank));
   }

   _db.update_enterprise( enterprise, producer_schedule, props );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void enterprise_fund_evaluator::do_apply( const enterprise_fund_operation& o )
{ try {
   const account_object& funder = _db.get_account( o.funder );
   FC_ASSERT( funder.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.funder));

   const enterprise_object& enterprise = _db.get_enterprise( o.account, o.enterprise_id );
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   const producer_schedule_object& producer_schedule = _db.get_producer_schedule();
   asset liquid = _db.get_liquid_balance( o.funder, o.amount.symbol );
   time_point now = _db.head_block_time();

   FC_ASSERT( enterprise.budget.symbol == o.amount.symbol,
      "Funding asset: ${a} must be the same as the budget asset of the Enterprise it is contributing to: ${b}.",
      ("a",o.amount.symbol)("b",enterprise.budget.symbol) );

   const auto& enterprise_fund_idx = _db.get_index< enterprise_fund_index >().indices().get< by_funder_account_enterprise >();
   auto enterprise_fund_itr = enterprise_fund_idx.find( boost::make_tuple( o.funder, o.account, o.enterprise_id ) );

   FC_ASSERT( liquid >= o.amount,
      "Funder Account: ${f} has insufficient liquid balance to contribute amount: ${a}.",
      ("f",o.funder)("a",o.amount));
   
   _db.adjust_liquid_balance( o.funder, -o.amount );
   _db.adjust_liquid_balance( enterprise.account, o.amount );

   if( enterprise_fund_itr == enterprise_fund_idx.end() )
   {
      const enterprise_fund_object& fund = _db.create< enterprise_fund_object >( [&]( enterprise_fund_object& efo )
      {
         efo.funder = o.funder;
         efo.account = o.account;
         from_string( efo.enterprise_id, o.enterprise_id );
         efo.amount = o.amount;
         efo.last_updated = now;
         efo.created = now;
      });
      
      ilog( "Account: ${a} created new Enterprise Fund for Proposal: ${f}",
         ("a",o.funder)("f",fund));
   }
   else
   {
      const enterprise_fund_object& fund = *enterprise_fund_itr;

      _db.modify( fund, [&]( enterprise_fund_object& efo )
      {
         efo.amount += o.amount;
         efo.last_updated = now;
      });
      
      ilog( "Account: ${f} edited Enterprise Fund: Account: ${a} ID: ${id} Amount: ${am}",
         ("f",o.funder)("a",o.account)("id",o.enterprise_id)("am",o.amount));
   }

   _db.modify( enterprise, [&]( enterprise_object& e )
      {
         e.distributed += o.amount;
         e.last_updated = now;
      });

   _db.update_enterprise( enterprise, producer_schedule, props );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain