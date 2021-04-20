#include <node/app/api_context.hpp>
#include <node/app/application.hpp>
#include <node/app/database_api.hpp>

#include <node/protocol/get_config.hpp>

#include <node/chain/util/reward.hpp>

#include <fc/bloom_filter.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/crypto/hex.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>


#include <cctype>

#include <cfenv>
#include <iostream>

namespace node { namespace app {


   //=====================//
   // === Network API === //
   //=====================//


vector< account_network_state > database_api::get_account_network_state( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_network_state( names );
   });
}

vector< account_network_state > database_api_impl::get_account_network_state( vector< string > names )const
{
   vector< account_network_state > results;

   results.reserve( names.size() );

   const auto& producer_idx = _db.get_index< producer_index >().indices().get< by_name >();
   const auto& officer_idx = _db.get_index< network_officer_index >().indices().get< by_account >();
   const auto& enterprise_idx = _db.get_index< enterprise_index >().indices().get< by_enterprise_id >();
   const auto& interface_idx = _db.get_index< interface_index >().indices().get< by_account >();
   const auto& supernode_idx = _db.get_index< supernode_index >().indices().get< by_account >();
   const auto& validation_idx = _db.get_index< block_validation_index >().indices().get< by_producer_height >();
   
   const auto& incoming_producer_vote_idx = _db.get_index< producer_vote_index >().indices().get< by_producer_account >();
   const auto& incoming_officer_vote_idx = _db.get_index< network_officer_vote_index >().indices().get< by_officer_account >();
   const auto& incoming_enterprise_vote_idx = _db.get_index< enterprise_vote_index >().indices().get< by_enterprise_id >();
   const auto& incoming_enterprise_fund_idx = _db.get_index< enterprise_fund_index >().indices().get< by_account_enterprise_funder >();
   const auto& incoming_commit_violation_idx = _db.get_index< commit_violation_index >().indices().get< by_producer_height >();

   const auto& outgoing_producer_vote_idx = _db.get_index< producer_vote_index >().indices().get< by_account_rank >();
   const auto& outgoing_officer_vote_idx = _db.get_index< network_officer_vote_index >().indices().get< by_account_type_rank >();
   const auto& outgoing_enterprise_vote_idx = _db.get_index< enterprise_vote_index >().indices().get< by_account_rank >();
   const auto& outgoing_enterprise_fund_idx = _db.get_index< enterprise_fund_index >().indices().get< by_funder_account_enterprise >();
   const auto& outgoing_commit_violation_idx = _db.get_index< commit_violation_index >().indices().get< by_reporter_height >();

   for( auto name : names )
   {
      account_network_state nstate;

      auto producer_itr = producer_idx.find( name );
      if( producer_itr != producer_idx.end() )
      {
         nstate.producer = producer_api_obj( *producer_itr );
      }

      auto officer_itr = officer_idx.find( name );
      if( officer_itr != officer_idx.end() )
      {
         nstate.network_officer = network_officer_api_obj( *officer_itr );
      }

      auto interface_itr = interface_idx.find( name );
      if( interface_itr != interface_idx.end() )
      {
         nstate.interface = interface_api_obj( *interface_itr );
      }

      auto supernode_itr = supernode_idx.find( name );
      if( supernode_itr != supernode_idx.end() )
      {
         nstate.supernode = supernode_api_obj( *supernode_itr );
      }

      auto enterprise_itr = enterprise_idx.lower_bound( name );
      while( enterprise_itr != enterprise_idx.end() && enterprise_itr->account == name )
      {
         nstate.enterprise_proposals.push_back( enterprise_api_obj( *enterprise_itr ) );
         ++enterprise_itr;
      }

      auto validation_itr = validation_idx.lower_bound( name );
      while( validation_itr != validation_idx.end() && 
         validation_itr->producer == name &&
         nstate.block_validations.size() < 100 )
      {
         nstate.block_validations.push_back( block_validation_api_obj( *validation_itr ) );
         ++validation_itr;
      }

      auto incoming_producer_vote_itr = incoming_producer_vote_idx.lower_bound( name );
      while( incoming_producer_vote_itr != incoming_producer_vote_idx.end() && 
         incoming_producer_vote_itr->producer == name )
      {
         nstate.incoming_producer_votes[ incoming_producer_vote_itr->account ] = producer_vote_api_obj( *incoming_producer_vote_itr );
         ++incoming_producer_vote_itr;
      }

      auto incoming_officer_vote_itr = incoming_officer_vote_idx.lower_bound( name );
      while( incoming_officer_vote_itr != incoming_officer_vote_idx.end() && 
         incoming_officer_vote_itr->network_officer == name )
      {
         nstate.incoming_network_officer_votes[ incoming_officer_vote_itr->account ] = network_officer_vote_api_obj( *incoming_officer_vote_itr );
         ++incoming_officer_vote_itr;
      }

      auto incoming_enterprise_vote_itr = incoming_enterprise_vote_idx.lower_bound( name );
      while( incoming_enterprise_vote_itr != incoming_enterprise_vote_idx.end() && 
         incoming_enterprise_vote_itr->account == name )
      {
         nstate.incoming_enterprise_votes[ incoming_enterprise_vote_itr->voter ][ to_string( incoming_enterprise_vote_itr->enterprise_id ) ] = enterprise_vote_api_obj( *incoming_enterprise_vote_itr );
         ++incoming_enterprise_vote_itr;
      }

      auto incoming_enterprise_fund_itr = incoming_enterprise_fund_idx.lower_bound( name );
      while( incoming_enterprise_fund_itr != incoming_enterprise_fund_idx.end() && 
         incoming_enterprise_fund_itr->account == name )
      {
         nstate.incoming_enterprise_funds[ incoming_enterprise_fund_itr->funder ][ to_string( incoming_enterprise_fund_itr->enterprise_id ) ] = enterprise_fund_api_obj( *incoming_enterprise_fund_itr );
         ++incoming_enterprise_fund_itr;
      }

      auto incoming_commit_violation_itr = incoming_commit_violation_idx.lower_bound( name );
      while( incoming_commit_violation_itr != incoming_commit_violation_idx.end() && 
         incoming_commit_violation_itr->producer == name &&
         nstate.incoming_commit_violations.size() < 100 )
      {
         nstate.incoming_commit_violations[ incoming_commit_violation_itr->reporter ] = commit_violation_api_obj( *incoming_commit_violation_itr );
         ++incoming_commit_violation_itr;
      }

      auto outgoing_producer_vote_itr = outgoing_producer_vote_idx.lower_bound( name );
      while( outgoing_producer_vote_itr != outgoing_producer_vote_idx.end() && 
         outgoing_producer_vote_itr->account == name ) 
      {
         nstate.outgoing_producer_votes[ outgoing_producer_vote_itr->producer ] = producer_vote_api_obj( *outgoing_producer_vote_itr );
         ++outgoing_producer_vote_itr;
      }

      auto outgoing_officer_vote_itr = outgoing_officer_vote_idx.lower_bound( name );
      while( outgoing_officer_vote_itr != outgoing_officer_vote_idx.end() && 
         outgoing_officer_vote_itr->account == name )
      {
         nstate.outgoing_network_officer_votes[ outgoing_officer_vote_itr->network_officer ] = network_officer_vote_api_obj( *outgoing_officer_vote_itr );
         ++outgoing_officer_vote_itr;
      }

      auto outgoing_enterprise_vote_itr = outgoing_enterprise_vote_idx.lower_bound( name );
      while( outgoing_enterprise_vote_itr != outgoing_enterprise_vote_idx.end() && 
         outgoing_enterprise_vote_itr->voter == name )
      {
         nstate.outgoing_enterprise_votes[ outgoing_enterprise_vote_itr->account ][ to_string( outgoing_enterprise_vote_itr->enterprise_id ) ] = enterprise_vote_api_obj( *outgoing_enterprise_vote_itr );
         ++outgoing_enterprise_vote_itr;
      }

      auto outgoing_enterprise_fund_itr = outgoing_enterprise_fund_idx.lower_bound( name );
      while( outgoing_enterprise_fund_itr != outgoing_enterprise_fund_idx.end() && 
         outgoing_enterprise_fund_itr->funder == name )
      {
         nstate.outgoing_enterprise_funds[ outgoing_enterprise_fund_itr->account ][ to_string( outgoing_enterprise_fund_itr->enterprise_id ) ] = enterprise_fund_api_obj( *outgoing_enterprise_fund_itr );
         ++outgoing_enterprise_fund_itr;
      }

      auto outgoing_commit_violation_itr = outgoing_commit_violation_idx.lower_bound( name );
      while( outgoing_commit_violation_itr != outgoing_commit_violation_idx.end() && 
         outgoing_commit_violation_itr->reporter == name &&
         nstate.outgoing_commit_violations.size() < 100 )
      {
         nstate.outgoing_commit_violations[ outgoing_commit_violation_itr->producer ] = commit_violation_api_obj( *outgoing_commit_violation_itr );
         ++outgoing_commit_violation_itr;
      }

      results.push_back( nstate );
   }

   return results;
}

vector< account_name_type > database_api::get_active_producers()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_active_producers();
   });
}

vector< account_name_type > database_api_impl::get_active_producers()const
{
   const producer_schedule_object& pso = _db.get_producer_schedule();
   size_t n = pso.current_shuffled_producers.size();
   vector< account_name_type > results;
   results.reserve( n );
   
   for( size_t i=0; i<n; i++ )
   {
      results.push_back( pso.current_shuffled_producers[ i ] );
   }
      
   return results;
}

vector< producer_api_obj > database_api::get_producers_by_voting_power( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_producers_by_voting_power( from, limit );
   });
}

vector< producer_api_obj > database_api_impl::get_producers_by_voting_power( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< producer_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< producer_index >().indices().get< by_name >();
   const auto& vote_idx = _db.get_index< producer_index >().indices().get< by_voting_power >();

   auto vote_itr = vote_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid producer name ${n}", ("n",from) );
      vote_itr = vote_idx.iterator_to( *name_itr );
   }

   while( vote_itr != vote_idx.end() && results.size() < limit && vote_itr->vote_count > 0 )
   {
      results.push_back( producer_api_obj( *vote_itr ) );
      ++vote_itr;
   }
   return results;
}

vector< producer_api_obj > database_api::get_producers_by_mining_power( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_producers_by_mining_power( from, limit );
   });
}

vector< producer_api_obj > database_api_impl::get_producers_by_mining_power( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< producer_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< producer_index >().indices().get< by_name >();
   const auto& mining_idx = _db.get_index< producer_index >().indices().get< by_mining_power >();

   auto mining_itr = mining_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid producer name ${n}", ("n",from) );
      mining_itr = mining_idx.iterator_to( *name_itr );
   }

   while( mining_itr != mining_idx.end() &&
      results.size() < limit && mining_itr->mining_count > 0 )
   {
      results.push_back( producer_api_obj( *mining_itr ) );
      ++mining_itr;
   }
   return results;
}


vector< network_officer_api_obj > database_api::get_development_officers_by_voting_power( string currency, string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_development_officers_by_voting_power( currency, from, limit );
   });
}

vector< network_officer_api_obj > database_api_impl::get_development_officers_by_voting_power( string currency, string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< network_officer_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< network_officer_index >().indices().get< by_account >();
   const auto& vote_idx = _db.get_index< network_officer_index >().indices().get< by_symbol_type_voting_power >();

   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( currency, network_officer_role_type::DEVELOPMENT ) );

   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid network officer name ${n}", ("n",from) );
      vote_itr = vote_idx.iterator_to( *name_itr );
   }

   while( vote_itr != vote_idx.end() && 
      results.size() < limit && 
      vote_itr->vote_count > 0 && 
      vote_itr->officer_type == network_officer_role_type::DEVELOPMENT )
   {
      results.push_back( network_officer_api_obj( *vote_itr ) );
      ++vote_itr;
   }
   return results;
}

vector< network_officer_api_obj > database_api::get_marketing_officers_by_voting_power( string currency, string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_marketing_officers_by_voting_power( currency, from, limit );
   });
}

vector< network_officer_api_obj > database_api_impl::get_marketing_officers_by_voting_power( string currency, string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< network_officer_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< network_officer_index >().indices().get< by_account >();
   const auto& vote_idx = _db.get_index< network_officer_index >().indices().get< by_symbol_type_voting_power >();

   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( currency, network_officer_role_type::MARKETING ) );

   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(), "Invalid network officer name ${n}", ("n",from) );
      vote_itr = vote_idx.iterator_to( *name_itr );
   }

   while( vote_itr != vote_idx.end() && results.size() < limit && vote_itr->vote_count > 0 && vote_itr->officer_type == network_officer_role_type::MARKETING )
   {
      results.push_back( network_officer_api_obj( *vote_itr ) );
      ++vote_itr;
   }
   return results;
}

vector< network_officer_api_obj > database_api::get_advocacy_officers_by_voting_power( string currency, string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_advocacy_officers_by_voting_power( currency, from, limit );
   });
}

vector< network_officer_api_obj > database_api_impl::get_advocacy_officers_by_voting_power( string currency, string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< network_officer_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< network_officer_index >().indices().get< by_account >();
   const auto& vote_idx = _db.get_index< network_officer_index >().indices().get< by_symbol_type_voting_power >();

   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( currency, network_officer_role_type::ADVOCACY ) );

   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid network officer name ${n}", ("n",from) );
      vote_itr = vote_idx.iterator_to( *name_itr );
   }

   while( vote_itr != vote_idx.end() && 
      results.size() < limit && 
      vote_itr->vote_count > 0 && 
      vote_itr->officer_type == network_officer_role_type::ADVOCACY )
   {
      results.push_back( network_officer_api_obj( *vote_itr ) );
      ++vote_itr;
   }
   return results;
}

vector< supernode_api_obj > database_api::get_supernodes_by_view_weight( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_supernodes_by_view_weight( from, limit );
   });
}

vector< supernode_api_obj > database_api_impl::get_supernodes_by_view_weight( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< supernode_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< supernode_index >().indices().get< by_account >();
   const auto& view_idx = _db.get_index< supernode_index >().indices().get< by_view_weight >();

   auto view_itr = view_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid supernode name ${n}", ("n",from) );
      view_itr = view_idx.iterator_to( *name_itr );
   }

   while( view_itr != view_idx.end() && 
      results.size() < limit && 
      view_itr->monthly_active_users > 0 )
   {
      results.push_back( supernode_api_obj( *view_itr ) );
      ++view_itr;
   }
   return results;
}

vector< interface_api_obj > database_api::get_interfaces_by_users( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_interfaces_by_users( from, limit );
   });
}

vector< interface_api_obj > database_api_impl::get_interfaces_by_users( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );

   vector< interface_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< interface_index >().indices().get< by_account >();
   const auto& user_idx = _db.get_index< interface_index >().indices().get< by_monthly_active_users >();

   auto user_itr = user_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid interface name ${n}", ("n",from) );
      user_itr = user_idx.iterator_to( *name_itr );
   }

   while( user_itr != user_idx.end() && 
      results.size() < limit && 
      user_itr->monthly_active_users > 0 )
   {
      results.push_back( interface_api_obj( *user_itr ) );
      ++user_itr;
   }
   return results;
}

vector< governance_api_obj > database_api::get_governances_by_members( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_governances_by_members( from, limit );
   });
}

vector< governance_api_obj > database_api_impl::get_governances_by_members( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );

   vector< governance_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< governance_index >().indices().get< by_account >();
   const auto& sub_idx = _db.get_index< governance_index >().indices().get< by_members >();

   auto sub_itr = sub_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid governance account name ${n}",
         ("n",from) );

      sub_itr = sub_idx.iterator_to( *name_itr );
   }

   while( sub_itr != sub_idx.end() && 
      results.size() < limit && 
      sub_itr->member_count > 0 )
   {
      results.push_back( governance_api_obj( *sub_itr ) );
      ++sub_itr;
   }
   return results;
}

vector< enterprise_api_obj > database_api::get_enterprise_by_voting_power( string from, string from_id, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_enterprise_by_voting_power( from, from_id, limit );
   });
}

vector< enterprise_api_obj > database_api_impl::get_enterprise_by_voting_power( string from, string from_id, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );

   vector< enterprise_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< enterprise_index >().indices().get< by_enterprise_id >();
   const auto& vote_idx = _db.get_index< enterprise_index >().indices().get< by_total_voting_power >();

   auto vote_itr = vote_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( boost::make_tuple( from, from_id ) );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid enterprise Creator: ${c} with enterprise_id: ${i}", ("c",from)("i",from_id) );
      vote_itr = vote_idx.iterator_to( *name_itr );
   }

   while( vote_itr != vote_idx.end() && 
      results.size() < limit && 
      vote_itr->vote_count > 0 )
   {
      results.push_back( enterprise_api_obj( *vote_itr ) );
      ++vote_itr;
   }
   return results;
}


} } // node::app