
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
void update_network_officer_evaluator::do_apply( const update_network_officer_operation& o )
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
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();
   const account_object& account = _db.get_account( o.account );

   FC_ASSERT( account.membership != membership_tier_type::NONE,
      "Account must be a member to create a network officer." );

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
         noo.created = now;
         noo.active = true;
      });
   } 
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void network_officer_vote_evaluator::do_apply( const network_officer_vote_operation& o )
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
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& voter = _db.get_account( o.account );
   const account_object& officer_account = _db.get_account( o.network_officer );
   FC_ASSERT( officer_account.active, 
      "Account: ${s} must be active to vote as officer.",("s", o.network_officer) );
   const network_officer_object& officer = _db.get_network_officer( o.network_officer );
   FC_ASSERT( officer.active, 
      "Account: ${s} must be active to vote as officer.",("s", o.network_officer) );
   const producer_schedule_object& pso = _db.get_producer_schedule();
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();

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
         });
         
         _db.update_network_officer_votes( voter );
      }
      else
      {
         if( account_officer_itr != account_officer_idx.end() && account_type_rank_itr != account_type_rank_idx.end() )
         {
            FC_ASSERT( account_officer_itr->network_officer != account_type_rank_itr->network_officer, 
               "Vote at rank is already specified network officer." );
         }
         
         if( account_officer_itr != account_officer_idx.end() )
         {
            ilog( "Removed: ${v}",("v",*account_officer_itr));
            _db.remove( *account_officer_itr );
         }

         _db.update_network_officer_votes( voter, o.network_officer, officer.officer_type, o.vote_rank );   // Remove existing officer vote, and add at new rank. 
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
      _db.update_network_officer_votes( voter );
   }

   _db.update_network_officer( officer, pso, props );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }

/**
 * Creates or updates an executive board object for a team of developers and employees.
 * Executive boards enable the issuance of network credit asset for operating a
 * multifaceted development, marketing, and avocacy team for the protocol.
 * 
 * Executive boards must:
 * 1 - Hold a minimum balance of 100 Equity assets.
 * 2 - Operate active supernode with at least 100 daily active users.
 * 3 - Operate active interface account with at least 100 daily active users.
 * 4 - Operate an active governance account with at least 100 subscribers.
 * 5 - Have at least 3 officers that are top 50 network officers, 1 from each role.
 * 6 - Have at least one officer that is an active top 50 voting producer.
 * 
 * Executive boards receive their budget payments when:
 * 1 - They are approved by at least 100 Accounts, with at least 10% of total voting power.
 * 2 - They are approved by at least 20 producers, with at least 10% of total producer voting power.
 * 3 - The Current price of the credit asset is greater than $0.90 USD.
 */
void update_executive_board_evaluator::do_apply( const update_executive_board_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   const account_business_object& b = _db.get_account_business( signed_for );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
   const account_object& account = _db.get_account( o.account );
   const account_object& executive = _db.get_account( o.executive );
   FC_ASSERT( executive.active, 
      "Account: ${s} must be active to update executive board.",("s", o.executive) );
   FC_ASSERT( o.executive == o.account || 
      b.is_authorized_network( o.account, _db.get_account_permissions( o.account ) ),
      "Account is not authorized to update Executive board." );
   const governance_account_object& gov_account = _db.get_governance_account( o.executive );
   FC_ASSERT( gov_account.active, 
      "Account: ${s} must be have governance account active to update executive board.",("s", o.executive) );
   const interface_object& interface = _db.get_interface( o.executive );
   FC_ASSERT( interface.active, 
      "Account: ${s} must be have interface active to update executive board.",("s", o.executive) );
   const supernode_object& supernode = _db.get_supernode( o.executive );
   FC_ASSERT( supernode.active, 
      "Account: ${s} must be have supernode active to update executive board.",("s", o.executive) );
   asset stake = _db.get_staked_balance( account.name, SYMBOL_COIN );
   const median_chain_property_object& median_props = _db.get_median_chain_properties();

   FC_ASSERT( o.budget <= median_props.max_exec_budget, 
      "Executive board Budget is too high. Please reduce budget." );

   const producer_schedule_object& pso = _db.get_producer_schedule();
   time_point now = _db.head_block_time();
     
   const executive_board_object* exec_ptr = _db.find_executive_board( o.executive );

   FC_ASSERT( b.executive_board.CHIEF_EXECUTIVE_OFFICER.size(),
      "Executive board requires chief executive officer.");
   FC_ASSERT( b.officers.size(), 
      "Executive board requires at least one officer." );

   if( exec_ptr != nullptr )      // Updating existing Executive board
   {
      const executive_board_object& exec_board = *exec_ptr;

      _db.modify( exec_board, [&]( executive_board_object& ebo )
      {
         ebo.budget = o.budget;
         if( o.url.size() )
         {
            from_string( ebo.url, o.url );
         }
         if( o.details.size() )
         {
            from_string( ebo.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( ebo.json, o.json );
         }
         ebo.active = o.active;
      });

      ilog( "Account: ${a} edited Executive Board: \n ${b} \n",
         ("a",o.account)("b",exec_board));
   }
   else         // create new executive board
   {
      // Perform Executive board eligibility checks when creating new community.
      FC_ASSERT( o.active, 
         "Executive board does not exist for this account, set active to true." );
      FC_ASSERT( supernode.active && interface.active && gov_account.active, 
         "Executive board must have an active interface, supernode, and governance account." );
      FC_ASSERT( executive.membership == membership_tier_type::TOP_MEMBERSHIP, 
         "Account must be a top level member to create an Executive board.");  
      FC_ASSERT( stake.amount >= 100*BLOCKCHAIN_PRECISION, 
         "Must have a minimum stake of 100 Core assets.");
      FC_ASSERT( gov_account.subscriber_count >= 100, 
         "Interface requires 100 daily active users to create Executive board." );
      FC_ASSERT( interface.daily_active_users >= 100 * PERCENT_100, 
         "Interface requires 100 daily active users to create Executive board." );
      FC_ASSERT( supernode.daily_active_users >= 100 * PERCENT_100, 
         "Supernode requires 100 daily active users to create Executive board." );
      
      bool producer_check = false;
      bool dev_check = false;
      bool mar_check = false;
      bool adv_check = false;

      if( pso.is_top_voting_producer( executive.name ) )
      {
         producer_check = true;
      }
      const network_officer_object* off_ptr = _db.find_network_officer( executive.name );
      if( off_ptr != nullptr )
      {
         const network_officer_object& officer = *off_ptr;
         switch( officer.officer_type)
         {
            case network_officer_role_type::DEVELOPMENT: 
            {
               dev_check = true;
            }
            break;
            case network_officer_role_type::MARKETING: 
            {
               mar_check = true;
            }
            break;
            case network_officer_role_type::ADVOCACY: 
            {
               adv_check = true;
            } 
            break;
            default:
            {
               FC_ASSERT( false, "Invalid officer type" );
            }
         }
      }

      for( auto name : b.executives )
      {
         if( pso.is_top_voting_producer( name ) )
         {
            producer_check = true;
         }
         const account_object& off_account = _db.get_account( name );
         FC_ASSERT( off_account.active, 
            "Account: ${s} must be active to vote as officer.",("s", name) );
         const network_officer_object* off_ptr = _db.find_network_officer( name );
         if( off_ptr != nullptr )
         {
            const network_officer_object& officer = *off_ptr;
            switch( officer.officer_type )
            {
               case network_officer_role_type::DEVELOPMENT: 
               {
                  dev_check = true;
               }
               continue;
               case network_officer_role_type::MARKETING: 
               {
                  mar_check = true;
               }
               continue;
               case network_officer_role_type::ADVOCACY:
               {
                  adv_check = true;
               }
               continue;
               default:
               {
                  FC_ASSERT( false, "Invalid officer type" );
               }
            }
         }  
      }

      for( auto name : b.officers )
      {
         if( pso.is_top_voting_producer( name ) )
         {
            producer_check = true;
         }
         const account_object& off_account = _db.get_account( name );
         FC_ASSERT( off_account.active, 
            "Account: ${s} must be active to vote as officer.",("s", name) );
         const network_officer_object* off_ptr = _db.find_network_officer( name );
         if( off_ptr != nullptr )
         {
            const network_officer_object& officer = *off_ptr;
            switch( officer.officer_type )
            {
               case network_officer_role_type::DEVELOPMENT: 
               {
                  dev_check = true;
               }
               continue;
               case network_officer_role_type::MARKETING: 
               {
                  mar_check = true;
               }
               continue;
               case network_officer_role_type::ADVOCACY: 
               {
                  adv_check = true;
               }
               continue;
               default:
               {
                  FC_ASSERT( false, "Invalid officer type" );
               }
            }
         } 
      }

      FC_ASSERT( producer_check && dev_check && mar_check && adv_check, 
         "Executive board must contain at least one producer, developer, marketer, and advocate.");

      const executive_board_object& exec_board = _db.create< executive_board_object >( [&]( executive_board_object& ebo )
      {
         ebo.account = o.executive;
         ebo.budget = o.budget;
         if( o.url.size() )
         {
            from_string( ebo.url, o.url );
         }
         if( o.details.size() )
         {
            from_string( ebo.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( ebo.json, o.json );
         }
         ebo.active = true;
         ebo.created = now;
      });

      ilog( "Account: ${a} created new Executive Board: \n ${b} \n",
         ("a",o.account)("b",exec_board));
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void executive_board_vote_evaluator::do_apply( const executive_board_vote_operation& o )
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
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& voter = _db.get_account( o.account );
   const executive_board_object& executive = _db.get_executive_board( o.executive_board );
   FC_ASSERT( executive.active, 
      "Account: ${s} must be active to receive executive board vote.",("s", o.executive_board ) );
   const account_object& executive_account = _db.get_account( o.executive_board );
   FC_ASSERT( executive_account.active, 
      "Account: ${s} must be active to receive executive board vote.",("s", o.executive_board ) );
   const producer_schedule_object& pso = _db.get_producer_schedule();
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();

   if( o.approved )
   {
      FC_ASSERT( voter.can_vote, 
         "Account has declined its voting rights." );
      FC_ASSERT( executive.active, 
         "Executive board is inactive, and not accepting approval votes at this time." );
   }
   
   const auto& account_rank_idx = _db.get_index< executive_board_vote_index >().indices().get< by_account_rank >();
   const auto& account_executive_idx = _db.get_index< executive_board_vote_index >().indices().get< by_account_executive >();
   auto account_rank_itr = account_rank_idx.find( boost::make_tuple( voter.name, o.vote_rank ) );   // vote at rank number
   auto account_executive_itr = account_executive_idx.find( boost::make_tuple( voter.name, executive.account ) );    // vote for specified executive board

   if( o.approved )      // Adding or modifying vote
   {
      if( account_executive_itr == account_executive_idx.end() && account_rank_itr == account_rank_idx.end() )     // No vote for executive or rank, create new vote.
      {
         FC_ASSERT( voter.executive_board_vote_count < MAX_EXEC_VOTES, 
            "Account has voted for too many Executive boards." );

         _db.create< executive_board_vote_object >( [&]( executive_board_vote_object& v )
         {
            v.executive_board = executive.account;
            v.account = voter.name;
            v.vote_rank = o.vote_rank;
         });
         
         _db.update_executive_board_votes( voter );
      }
      else
      {
         if( account_executive_itr != account_executive_idx.end() && account_rank_itr != account_rank_idx.end() )
         {
            FC_ASSERT( account_executive_itr->executive_board != account_rank_itr->executive_board, 
               "Vote at rank is already specified Executive board." );
         }
         
         if( account_executive_itr != account_executive_idx.end() )
         {
            ilog( "Removed: ${v}",("v",*account_executive_itr));
            _db.remove( *account_executive_itr );
         }

         _db.update_executive_board_votes( voter, o.executive_board, o.vote_rank );   // Remove existing officer vote, and add at new rank. 
      }
   }
   else       // Removing existing vote
   {
      if( account_executive_itr != account_executive_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*account_executive_itr));
         _db.remove( *account_executive_itr );
      }
      else if( account_rank_itr != account_rank_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*account_rank_itr));
         _db.remove( *account_rank_itr );
      }
      _db.update_executive_board_votes( voter );
   }

   _db.update_executive_board( executive, pso, props );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void update_governance_evaluator::do_apply( const update_governance_operation& o )
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
   time_point now = _db.head_block_time();
   const account_object& account = _db.get_account( o.account );
   
   FC_ASSERT( account.membership == membership_tier_type::TOP_MEMBERSHIP,
      "Account must be a Top level member to create governance account" );

   const governance_account_object* gov_acc_ptr = _db.find_governance_account( o.account );

   if( gov_acc_ptr != nullptr )      // updating existing governance account
   { 
      _db.modify( *gov_acc_ptr, [&]( governance_account_object& gao )
      {
         if( o.url.size() )
         {
            from_string( gao.url, o.url );
         }
         if( o.details.size() )
         {
            from_string( gao.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( gao.json, o.json );
         }
         gao.active = o.active;
      });
   }
   else       // create new governance account
   {
      _db.create< governance_account_object >( [&]( governance_account_object& gao )
      {
         gao.account = o.account;
         if( o.url.size() )
         {
            from_string( gao.url, o.url );
         }
         if( o.details.size() )
         {
            from_string( gao.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( gao.json, o.json );
         }
         gao.created = now;
         gao.active = true;
      });
   } 
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void subscribe_governance_evaluator::do_apply( const subscribe_governance_operation& o )
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
   const governance_account_object& gov_account = _db.get_governance_account( o.governance_account );
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   const producer_schedule_object& pso = _db.get_producer_schedule();
   const auto& gov_idx = _db.get_index< governance_subscription_index >().indices().get< by_account_governance >();
   auto itr = gov_idx.find( boost::make_tuple( o.account, gov_account.account ) );

   if( itr == gov_idx.end() )    // Account is new subscriber
   { 
      FC_ASSERT( o.subscribe, 
         "subscription doesn't exist, user must select to subscribe to the account." );
      FC_ASSERT( account.governance_subscriptions < MAX_GOV_ACCOUNTS, 
         "Account has too many governance subscriptions." );

      _db.create< governance_subscription_object >( [&]( governance_subscription_object& gso )
      {
         gso.governance_account = o.governance_account;
         gso.account = o.account;
      });
   
      _db.modify( account, [&]( account_object& a ) 
      {
         a.governance_subscriptions++;
      });

   } 
   else  // Account is already subscribed and is unsubscribing
   { 
      FC_ASSERT( !o.subscribe,
         "Subscription currently exists, user must select to unsubscribe." );
       
      _db.modify( account, [&]( account_object& a ) 
      {
         a.governance_subscriptions--;
      });

      ilog( "Removed: ${v}",("v",*itr));
      _db.remove( *itr );
   }

   _db.update_governance_account( gov_account, pso, props );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void update_supernode_evaluator::do_apply( const update_supernode_operation& o )
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
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
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


void update_interface_evaluator::do_apply( const update_interface_operation& o )
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
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& account = _db.get_account( o.account );
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


void update_mediator_evaluator::do_apply( const update_mediator_operation& o )
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
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   
   time_point now = _db.head_block_time();
   asset liquid = _db.get_liquid_balance( o.account, SYMBOL_COIN );

   FC_ASSERT( account.membership != membership_tier_type::NONE, 
      "Account must be a member to act as a mediator." );
   
   const mediator_object* med_ptr = _db.find_mediator( o.account );

   if( med_ptr != nullptr ) // updating existing mediator
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
   else  // create new mediator
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


void create_community_enterprise_evaluator::do_apply( const create_community_enterprise_operation& o )
{ try {
   const account_name_type& signed_for = o.creator;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();
   const reward_fund_object& reward_fund = _db.get_reward_fund( o.daily_budget.symbol );
   const asset_currency_data_object& currency = _db.get_currency_data( o.daily_budget.symbol );

   FC_ASSERT( currency.community_fund_reward_percent > 0,
      "Daily Budget reward currency does not issue to Enterprise proposals." );

   share_type daily_budget_total = ( currency.block_reward.amount * BLOCKS_PER_DAY * currency.community_fund_reward_percent) / PERCENT_100 ;
   FC_ASSERT( o.daily_budget.amount < daily_budget_total,
      "Daily Budget must specify a budget less than the total amount of funds available per day." );
   
   uint16_t milestone_sum = 0;
   for( auto milestone : o.milestone_shares )
   {
      milestone_sum += milestone;
   }

   FC_ASSERT( milestone_sum == PERCENT_100, 
      "Milestone Sum must equal 100 percent." );
   
   uint16_t beneficiary_sum = 0;
   for( auto beneficiary : o.beneficiaries )
   {
      beneficiary_sum += beneficiary.second;
   }

   FC_ASSERT( beneficiary_sum == PERCENT_100, 
      "Beneficiary Sum must equal 100 percent." );
   FC_ASSERT( o.beneficiaries.size() >= 1 && o.beneficiaries.size() <= 100, 
      "Funding Proposal must have between 1 and 100 beneficiaries." );
   
   const community_enterprise_object* ent_ptr = _db.find_community_enterprise( o.creator, o.enterprise_id );

   if( ent_ptr != nullptr ) // Updating or removing existing proposal 
   {
      const community_enterprise_object& enterprise = *ent_ptr;
      if( enterprise.approved_milestones == -1 )
      {
         FC_ASSERT( o.begin  > ( enterprise.created + fc::days(7) ), 
            "Begin time must be at least 7 days after creation." );
         
         uint16_t milestone_sum = 0;
         for( auto milestone : o.milestone_shares )
         {
            milestone_sum += milestone;
         }
         FC_ASSERT( milestone_sum == PERCENT_100, 
            "Milestone Sum must equal 100 percent." );
      }
      
      _db.modify( *ent_ptr, [&]( community_enterprise_object& ceo )
      { 
         if( o.url.size() )
         {
            from_string( ceo.url, o.url );
         }
         if( o.details.size() )
         {
            from_string( ceo.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( ceo.json, o.json );
         }
         ceo.active = o.active;
         if( ceo.approved_milestones == -1 )     // Proposal not yet accepted, able to modify. 
         {
            ceo.beneficiaries = o.beneficiaries;
            ceo.begin = o.begin;
            ceo.duration = o.duration;
            ceo.end = o.begin + fc::days( o.duration );
            ceo.milestone_shares.reserve( o.milestone_shares.size() );

            for( auto& mile : o.milestone_shares )
            {
               ceo.milestone_shares.push_back( mile );
            }
         }
      });

      ilog( "Account: ${a} edited community enterprise id: ${id}",
         ("a",o.creator)("id",o.enterprise_id));
   }
   else        // Create new community enterprise proposal
   {
      FC_ASSERT( o.begin  > ( now + fc::days(7) ), 
         "Begin time must be at least 7 days in the future." );
      FC_ASSERT( o.fee.amount >= 10 * BLOCKCHAIN_PRECISION, 
         "Proposal Requires a fee of 10 currency assets." );
      asset liquid = _db.get_liquid_balance( o.creator, o.daily_budget.symbol );
      FC_ASSERT( liquid >= o.fee,
         "Creator has insufficient balance to pay community enterprise proposal fee." );

      _db.adjust_liquid_balance( o.creator, -o.fee );
      _db.modify( reward_fund, [&]( reward_fund_object& rfo )
      { 
         rfo.adjust_community_fund_balance( o.fee );
      });
      _db.adjust_pending_supply( o.fee );

      _db.create< community_enterprise_object >( [&]( community_enterprise_object& ceo )
      {
         ceo.creator = o.creator;
         from_string( ceo.enterprise_id, o.enterprise_id );
         ceo.beneficiaries = o.beneficiaries;
         ceo.begin = o.begin;
         ceo.end = o.begin + fc::days( o.duration );
         ceo.expiration = ceo.end + fc::days(365);
         ceo.duration = o.duration;
         ceo.approved_milestones = -1;
         ceo.claimed_milestones = 0;
         ceo.daily_budget = o.daily_budget;
         ceo.pending_budget = asset( 0, o.daily_budget.symbol );
         ceo.total_distributed = asset( 0, o.daily_budget.symbol );

         ceo.milestone_shares.reserve( o.milestone_shares.size() );
         for( auto m : o.milestone_shares )
         {
            ceo.milestone_shares.push_back( m );
         }
         if( o.url.size() )
         {
            from_string( ceo.url, o.url );
         }
         if( o.details.size() )
         {
            from_string( ceo.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( ceo.json, o.json );
         }
         ceo.active = true;
         ceo.created = now;
      });

      ilog( "Account: ${a} created community enterprise id: ${id}",
         ("a",o.creator)("id",o.enterprise_id));
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void claim_enterprise_milestone_evaluator::do_apply( const claim_enterprise_milestone_operation& o )
{ try {
   const account_name_type& signed_for = o.creator;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
   const community_enterprise_object& enterprise = _db.get_community_enterprise( o.creator, o.enterprise_id );
   FC_ASSERT( o.milestone > enterprise.approved_milestones,
      "Claimed milestone has already been approved." );
   FC_ASSERT( o.milestone != enterprise.claimed_milestones,
      "This operaiton would not change the claimed milestone." );

   const dynamic_global_property_object props = _db.get_dynamic_global_properties();
   const producer_schedule_object& producer_schedule = _db.get_producer_schedule();

   _db.modify( enterprise, [&]( community_enterprise_object& ceo )
   { 
      ceo.claimed_milestones = o.milestone;
   });

   _db.update_enterprise( enterprise, producer_schedule, props );

   ilog("Account: ${a} Claimed milestone: ${m} on enterprise id: ${id}",
      ("a",o.creator)("m",enterprise.claimed_milestones)("id",enterprise.enterprise_id));
   
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void approve_enterprise_milestone_evaluator::do_apply( const approve_enterprise_milestone_operation& o )
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
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& account = _db.get_account( o.account );
   const community_enterprise_object& enterprise = _db.get_community_enterprise( o.creator, o.enterprise_id );

   FC_ASSERT( o.milestone <= enterprise.claimed_milestones,
      "Cannot approve milestone that has not yet been claimed by the enterprise creator." );

   if( o.approved )
   {
      FC_ASSERT( account.can_vote, 
         "Account has declined its voting rights." );
      FC_ASSERT( enterprise.active, 
         "Executive board is inactive, and not accepting approval votes at this time." );
   }
      
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   const producer_schedule_object& producer_schedule = _db.get_producer_schedule();
   const auto& account_rank_idx = _db.get_index< enterprise_approval_index >().indices().get< by_account_rank >();
   const auto& account_enterprise_idx = _db.get_index< enterprise_approval_index >().indices().get< by_account_enterprise >();
   auto account_rank_itr = account_rank_idx.find( boost::make_tuple( o.account, o.vote_rank ) );
   auto account_enterprise_itr = account_enterprise_idx.find( boost::make_tuple( o.account, enterprise.creator, o.enterprise_id ) );

   if( o.approved )      // Adding or modifying vote
   {
      if( account_enterprise_itr == account_enterprise_idx.end() && 
         account_rank_itr == account_rank_idx.end() )
      {
         FC_ASSERT( account.enterprise_approval_count < MAX_ACCOUNT_VOTES,
            "Account has voted for too many Enterprise proposals." );

         const enterprise_approval_object& app = _db.create< enterprise_approval_object >( [&]( enterprise_approval_object& v )
         {
            v.account = o.account;
            v.creator = o.creator;
            from_string( v.enterprise_id, o.enterprise_id );
            v.vote_rank = o.vote_rank;
            v.milestone = o.milestone;
         });
         
         _db.update_enterprise_approvals( account );

         ilog( "Account: ${a} created new Enterprise Approval - \n ${ap} \n",
            ("a",o.account)("ap",app));
      }
      else
      {
         if( account_enterprise_itr != account_enterprise_idx.end() && 
            account_rank_itr != account_rank_idx.end() )
         {
            FC_ASSERT( account_enterprise_itr->creator != account_rank_itr->creator && 
               account_enterprise_itr->enterprise_id != account_rank_itr->enterprise_id, 
               "Vote at rank is already specified Enterprise proposal." );
         }
         
         if( account_enterprise_itr != account_enterprise_idx.end() )
         {
            ilog( "Removed: ${v}",("v",*account_enterprise_itr));
            _db.remove( *account_enterprise_itr );
         }

         _db.update_enterprise_approvals( account, o.creator, o.enterprise_id, o.vote_rank, o.milestone );

         ilog( "Account: ${a} edited enterprise proposal - creator: ${c} id: ${id} vote rank: ${r} milestone: ${m}",
            ("a",o.account)("c",o.creator)("id",o.enterprise_id)("r",o.vote_rank)("m",o.milestone));
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
      _db.update_enterprise_approvals( account );
      ilog( "Account: ${a} removed enterprise proposal - creator: ${c} id: ${id} vote rank: ${r} milestone: ${m}",
            ("a",o.account)("c",o.creator)("id",o.enterprise_id)("r",o.vote_rank)("m",o.milestone));
   }

   _db.update_enterprise( enterprise, producer_schedule, props );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain