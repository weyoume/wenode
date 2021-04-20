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


   //=======================//
   // === Community API === //
   //=======================//


vector< extended_community > database_api::get_communities( vector< string > communities )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_communities( communities );
   });
}

vector< extended_community > database_api_impl::get_communities( vector< string > communities )const
{
   vector< extended_community > results;
   const auto& community_idx = _db.get_index< community_index >().indices().get< by_name >();
   const auto& community_permission_idx = _db.get_index< community_permission_index >().indices().get< by_name >();
   const auto& community_req_idx = _db.get_index< community_member_request_index >().indices().get< by_community_type_account >();
   const auto& community_event_idx = _db.get_index< community_event_index >().indices().get< by_community_time >();
   const auto& community_event_attend_idx = _db.get_index< community_event_attend_index >().indices().get< by_event_id >();
   const auto& community_federation_a_idx = _db.get_index< community_federation_index >().indices().get< by_community_a >();
   const auto& community_federation_b_idx = _db.get_index< community_federation_index >().indices().get< by_community_b >();
   const auto& community_inbox_idx = _db.get_index< message_index >().indices().get< by_community_inbox >();
   const auto& comment_blog_idx = _db.get_index< comment_blog_index >().indices().get< by_new_community_blog >();
   const auto& directive_idx = _db.get_index< community_directive_index >().indices().get< by_community_votes >();
   const auto& directive_approve_idx = _db.get_index< community_directive_vote_index >().indices().get< by_directive_approve >();
   
   for( string community : communities )
   {
      auto community_itr = community_idx.find( community );
      if( community_itr != community_idx.end() )
      results.push_back( extended_community( *community_itr ) );
      auto community_permission_itr = community_permission_idx.find( community );
      if( community_permission_itr != community_permission_idx.end() )
      {
         for( auto sub : community_permission_itr->subscribers )
         {
            results.back().subscribers.push_back( sub );
         }
         for( auto mem : community_permission_itr->members )
         {
            results.back().members.push_back( mem );
         }
         for( auto mod : community_permission_itr->moderators )
         {
            results.back().moderators.push_back( mod );
         }
         for( auto admin : community_permission_itr->administrators )
         {
            results.back().administrators.push_back( admin );
         }
         for( auto bl : community_permission_itr->blacklist )
         {
            results.back().blacklist.push_back( bl );
         }
         for( auto weight : community_permission_itr->vote_weight )
         {
            results.back().vote_weight[ weight.first ] = weight.second.value;
         }
         results.back().total_vote_weight = community_permission_itr->total_vote_weight.value;
      }
      
      auto community_inbox_itr = community_inbox_idx.lower_bound( community );
      while( community_inbox_itr != community_inbox_idx.end() && 
         community_inbox_itr->community == community )
      {
         results.back().messages.push_back( message_api_obj( *community_inbox_itr ) );
         ++community_inbox_itr;
      }
      
      auto community_federation_a_itr = community_federation_a_idx.lower_bound( boost::make_tuple( community, community_federation_type::MEMBER_FEDERATION ) );
      auto community_federation_b_itr = community_federation_b_idx.lower_bound( boost::make_tuple( community, community_federation_type::MEMBER_FEDERATION ) );
      while( community_federation_a_itr != community_federation_a_idx.end() && 
         community_federation_a_itr->community_a == community &&
         community_federation_a_itr->federation_type == community_federation_type::MEMBER_FEDERATION )
      {
         if( community_federation_a_itr->share_accounts_a )
         {
            results.back().upstream_member_federations[ community_federation_a_itr->community_b ] = community_federation_api_obj( *community_federation_a_itr );
         }
         if( community_federation_a_itr->share_accounts_b )
         {
            results.back().downstream_member_federations[ community_federation_a_itr->community_b ] = community_federation_api_obj( *community_federation_a_itr );
         }
         ++community_federation_a_itr;
      }

      while( community_federation_b_itr != community_federation_b_idx.end() && 
         community_federation_b_itr->community_b == community &&
         community_federation_b_itr->federation_type == community_federation_type::MEMBER_FEDERATION )
      {
         if( community_federation_b_itr->share_accounts_b )
         {
            results.back().upstream_member_federations[ community_federation_b_itr->community_a ] = community_federation_api_obj( *community_federation_b_itr );
         }
         if( community_federation_b_itr->share_accounts_a )
         {
            results.back().downstream_member_federations[ community_federation_b_itr->community_a ] = community_federation_api_obj( *community_federation_b_itr );
         }
         ++community_federation_b_itr;
      }

      community_federation_a_itr = community_federation_a_idx.lower_bound( boost::make_tuple( community, community_federation_type::MODERATOR_FEDERATION ) );
      community_federation_b_itr = community_federation_b_idx.lower_bound( boost::make_tuple( community, community_federation_type::MODERATOR_FEDERATION ) );
      while( community_federation_a_itr != community_federation_a_idx.end() && 
         community_federation_a_itr->community_a == community &&
         community_federation_a_itr->federation_type == community_federation_type::MODERATOR_FEDERATION )
      {
         if( community_federation_a_itr->share_accounts_a )
         {
            results.back().upstream_moderator_federations[ community_federation_a_itr->community_b ] = community_federation_api_obj( *community_federation_a_itr );
         }
         if( community_federation_a_itr->share_accounts_b )
         {
            results.back().downstream_moderator_federations[ community_federation_a_itr->community_b ] = community_federation_api_obj( *community_federation_a_itr );
         }
         ++community_federation_a_itr;
      }

      while( community_federation_b_itr != community_federation_b_idx.end() && 
         community_federation_b_itr->community_b == community &&
         community_federation_b_itr->federation_type == community_federation_type::MODERATOR_FEDERATION )
      {
         if( community_federation_b_itr->share_accounts_b )
         {
            results.back().upstream_moderator_federations[ community_federation_b_itr->community_a ] = community_federation_api_obj( *community_federation_b_itr );
         }
         if( community_federation_b_itr->share_accounts_a )
         {
            results.back().downstream_moderator_federations[ community_federation_b_itr->community_a ] = community_federation_api_obj( *community_federation_b_itr );
         }
         ++community_federation_b_itr;
      }

      community_federation_a_itr = community_federation_a_idx.lower_bound( boost::make_tuple( community, community_federation_type::ADMIN_FEDERATION ) );
      community_federation_b_itr = community_federation_b_idx.lower_bound( boost::make_tuple( community, community_federation_type::ADMIN_FEDERATION ) );
      while( community_federation_a_itr != community_federation_a_idx.end() && 
         community_federation_a_itr->community_a == community &&
         community_federation_a_itr->federation_type == community_federation_type::ADMIN_FEDERATION )
      {
         if( community_federation_a_itr->share_accounts_a )
         {
            results.back().upstream_admin_federations[ community_federation_a_itr->community_b ] = community_federation_api_obj( *community_federation_a_itr );
         }
         if( community_federation_a_itr->share_accounts_b )
         {
            results.back().downstream_admin_federations[ community_federation_a_itr->community_b ] = community_federation_api_obj( *community_federation_a_itr );
         }
         ++community_federation_a_itr;
      }

      while( community_federation_b_itr != community_federation_b_idx.end() && 
         community_federation_b_itr->community_b == community &&
         community_federation_b_itr->federation_type == community_federation_type::ADMIN_FEDERATION )
      {
         if( community_federation_b_itr->share_accounts_b )
         {
            results.back().upstream_admin_federations[ community_federation_b_itr->community_a ] = community_federation_api_obj( *community_federation_b_itr );
         }
         if( community_federation_b_itr->share_accounts_a )
         {
            results.back().downstream_admin_federations[ community_federation_b_itr->community_a ] = community_federation_api_obj( *community_federation_b_itr );
         }
         ++community_federation_b_itr;
      }
      
      auto community_event_itr = community_event_idx.lower_bound( community );
      auto community_event_attend_itr = community_event_attend_idx.lower_bound( community );

      while( community_event_itr != community_event_idx.end() && 
         community_event_itr->community == community )
      {
         results.back().events.push_back( community_event_state( *community_event_itr ) );

         while( community_event_attend_itr != community_event_attend_idx.end() && 
            community_event_attend_itr->community == community && 
            community_event_attend_itr->event_id == community_event_itr->event_id )
         {
            results.back().events.back().attend_objs.push_back( *community_event_attend_itr );
            ++community_event_attend_itr;
         }
         
         ++community_event_itr;
      }

      auto directive_itr = directive_idx.lower_bound( boost::make_tuple( community, true ) );

      while( directive_itr != directive_idx.end() && 
         directive_itr->community == community &&
         directive_itr->member_active )
      {
         community_directive_state cdstate = community_directive_state( *directive_itr );
         auto directive_approve_itr = directive_approve_idx.lower_bound( directive_itr->id );

         while( directive_approve_itr != directive_approve_idx.end() && 
            directive_approve_itr->directive == directive_itr->id )
         {
            cdstate.votes.push_back( community_directive_vote_api_obj( *directive_approve_itr ) );
            ++directive_approve_itr;
         }

         results.back().directives.push_back( cdstate );
         ++directive_approve_itr;
      }

      auto community_req_itr = community_req_idx.lower_bound( community );
      while( community_req_itr != community_req_idx.end() && community_req_itr->community == community )
      {
         results.back().requests[ community_req_itr->account ] = community_member_request_api_obj( *community_req_itr );
         ++community_req_itr;
      }

      if( results.back().pinned_author.size() && results.back().pinned_permlink.size() )
      {
         results.back().pinned_post = get_content( results.back().pinned_author, results.back().pinned_permlink );
      }

      auto comment_blog_itr = comment_blog_idx.lower_bound( community );
      if( comment_blog_itr != comment_blog_idx.end() && comment_blog_itr->community == community )
      {
         results.back().latest_post = get_discussion( comment_blog_itr->comment, 0 );
         results.back().latest_post.blog = comment_blog_api_obj( *comment_blog_itr );
      }
   }
   return results;
}

vector< extended_community > database_api::get_communities_by_subscribers( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_communities_by_subscribers( from, limit );
   });
}

vector< extended_community > database_api_impl::get_communities_by_subscribers( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< extended_community > results;
   results.reserve( limit );

   const auto& community_idx = _db.get_index< community_index >().indices().get< by_subscriber_count >();
   const auto& name_idx = _db.get_index< community_index >().indices().get< by_name >();
   const auto& community_permission_idx = _db.get_index< community_permission_index >().indices().get< by_name >();
   const auto& community_req_idx = _db.get_index< community_member_request_index >().indices().get< by_community_type_account >();
   const auto& community_event_idx = _db.get_index< community_event_index >().indices().get< by_community_time >();
   const auto& community_event_attend_idx = _db.get_index< community_event_attend_index >().indices().get< by_event_id >();
   const auto& community_federation_a_idx = _db.get_index< community_federation_index >().indices().get< by_community_a >();
   const auto& community_federation_b_idx = _db.get_index< community_federation_index >().indices().get< by_community_b >();
   const auto& community_inbox_idx = _db.get_index< message_index >().indices().get< by_community_inbox >();
   const auto& comment_blog_idx = _db.get_index< comment_blog_index >().indices().get< by_new_community_blog >();

   auto community_itr = community_idx.begin();
  
   if( from.size() )
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid Community name ${n}", ("n",from) );
      community_itr = community_idx.iterator_to( *name_itr );
   }

   while( community_itr != community_idx.end() && 
      results.size() < limit )
   {
      results.push_back( extended_community( *community_itr ) );
      community_name_type community = community_itr->name;
      
      auto community_permission_itr = community_permission_idx.find( community );
      if( community_permission_itr != community_permission_idx.end() )
      {
         for( auto sub : community_permission_itr->subscribers )
         {
            results.back().subscribers.push_back( sub );
         }
         for( auto mem : community_permission_itr->members )
         {
            results.back().members.push_back( mem );
         }
         for( auto mod : community_permission_itr->moderators )
         {
            results.back().moderators.push_back( mod );
         }
         for( auto admin : community_permission_itr->administrators )
         {
            results.back().administrators.push_back( admin );
         }
         for( auto bl : community_permission_itr->blacklist )
         {
            results.back().blacklist.push_back( bl );
         }
         for( auto weight : community_permission_itr->vote_weight )
         {
            results.back().vote_weight[ weight.first ] = weight.second.value;
         }
         results.back().total_vote_weight = community_permission_itr->total_vote_weight.value;
      }
      
      auto community_inbox_itr = community_inbox_idx.lower_bound( community );
      while( community_inbox_itr != community_inbox_idx.end() && 
         community_inbox_itr->community == community )
      {
         results.back().messages.push_back( message_api_obj( *community_inbox_itr ) );
         ++community_inbox_itr;
      }
      
      auto community_federation_a_itr = community_federation_a_idx.lower_bound( boost::make_tuple( community, community_federation_type::MEMBER_FEDERATION ) );
      auto community_federation_b_itr = community_federation_b_idx.lower_bound( boost::make_tuple( community, community_federation_type::MEMBER_FEDERATION ) );
      while( community_federation_a_itr != community_federation_a_idx.end() && 
         community_federation_a_itr->community_a == community &&
         community_federation_a_itr->federation_type == community_federation_type::MEMBER_FEDERATION )
      {
         if( community_federation_a_itr->share_accounts_a )
         {
            results.back().upstream_member_federations[ community_federation_a_itr->community_b ] = community_federation_api_obj( *community_federation_a_itr );
         }
         if( community_federation_a_itr->share_accounts_b )
         {
            results.back().downstream_member_federations[ community_federation_a_itr->community_b ] = community_federation_api_obj( *community_federation_a_itr );
         }
         ++community_federation_a_itr;
      }

      while( community_federation_b_itr != community_federation_b_idx.end() && 
         community_federation_b_itr->community_b == community &&
         community_federation_b_itr->federation_type == community_federation_type::MEMBER_FEDERATION )
      {
         if( community_federation_b_itr->share_accounts_b )
         {
            results.back().upstream_member_federations[ community_federation_b_itr->community_a ] = community_federation_api_obj( *community_federation_b_itr );
         }
         if( community_federation_b_itr->share_accounts_a )
         {
            results.back().downstream_member_federations[ community_federation_b_itr->community_a ] = community_federation_api_obj( *community_federation_b_itr );
         }
         ++community_federation_b_itr;
      }

      community_federation_a_itr = community_federation_a_idx.lower_bound( boost::make_tuple( community, community_federation_type::MODERATOR_FEDERATION ) );
      community_federation_b_itr = community_federation_b_idx.lower_bound( boost::make_tuple( community, community_federation_type::MODERATOR_FEDERATION ) );
      while( community_federation_a_itr != community_federation_a_idx.end() && 
         community_federation_a_itr->community_a == community &&
         community_federation_a_itr->federation_type == community_federation_type::MODERATOR_FEDERATION )
      {
         if( community_federation_a_itr->share_accounts_a )
         {
            results.back().upstream_moderator_federations[ community_federation_a_itr->community_b ] = community_federation_api_obj( *community_federation_a_itr );
         }
         if( community_federation_a_itr->share_accounts_b )
         {
            results.back().downstream_moderator_federations[ community_federation_a_itr->community_b ] = community_federation_api_obj( *community_federation_a_itr );
         }
         ++community_federation_a_itr;
      }

      while( community_federation_b_itr != community_federation_b_idx.end() && 
         community_federation_b_itr->community_b == community &&
         community_federation_b_itr->federation_type == community_federation_type::MODERATOR_FEDERATION )
      {
         if( community_federation_b_itr->share_accounts_b )
         {
            results.back().upstream_moderator_federations[ community_federation_b_itr->community_a ] = community_federation_api_obj( *community_federation_b_itr );
         }
         if( community_federation_b_itr->share_accounts_a )
         {
            results.back().downstream_moderator_federations[ community_federation_b_itr->community_a ] = community_federation_api_obj( *community_federation_b_itr );
         }
         ++community_federation_b_itr;
      }

      community_federation_a_itr = community_federation_a_idx.lower_bound( boost::make_tuple( community, community_federation_type::ADMIN_FEDERATION ) );
      community_federation_b_itr = community_federation_b_idx.lower_bound( boost::make_tuple( community, community_federation_type::ADMIN_FEDERATION ) );
      while( community_federation_a_itr != community_federation_a_idx.end() && 
         community_federation_a_itr->community_a == community &&
         community_federation_a_itr->federation_type == community_federation_type::ADMIN_FEDERATION )
      {
         if( community_federation_a_itr->share_accounts_a )
         {
            results.back().upstream_admin_federations[ community_federation_a_itr->community_b ] = community_federation_api_obj( *community_federation_a_itr );
         }
         if( community_federation_a_itr->share_accounts_b )
         {
            results.back().downstream_admin_federations[ community_federation_a_itr->community_b ] = community_federation_api_obj( *community_federation_a_itr );
         }
         ++community_federation_a_itr;
      }

      while( community_federation_b_itr != community_federation_b_idx.end() && 
         community_federation_b_itr->community_b == community &&
         community_federation_b_itr->federation_type == community_federation_type::ADMIN_FEDERATION )
      {
         if( community_federation_b_itr->share_accounts_b )
         {
            results.back().upstream_admin_federations[ community_federation_b_itr->community_a ] = community_federation_api_obj( *community_federation_b_itr );
         }
         if( community_federation_b_itr->share_accounts_a )
         {
            results.back().downstream_admin_federations[ community_federation_b_itr->community_a ] = community_federation_api_obj( *community_federation_b_itr );
         }
         ++community_federation_b_itr;
      }
      
      auto community_event_itr = community_event_idx.lower_bound( community );
      auto community_event_attend_itr = community_event_attend_idx.lower_bound( community );

      while( community_event_itr != community_event_idx.end() && 
         community_event_itr->community == community )
      {
         results.back().events.push_back( community_event_state( *community_event_itr ) );

         while( community_event_attend_itr != community_event_attend_idx.end() && 
            community_event_attend_itr->community == community && 
            community_event_attend_itr->event_id == community_event_itr->event_id )
         {
            results.back().events.back().attend_objs.push_back( *community_event_attend_itr );
            ++community_event_attend_itr;
         }
         
         ++community_event_itr;
      }

      auto community_req_itr = community_req_idx.lower_bound( community );
      while( community_req_itr != community_req_idx.end() && community_req_itr->community == community )
      {
         results.back().requests[ community_req_itr->account ] = community_member_request_api_obj( *community_req_itr );
         ++community_req_itr;
      }

      if( results.back().pinned_author.size() && results.back().pinned_permlink.size() )
      {
         results.back().pinned_post = get_content( results.back().pinned_author, results.back().pinned_permlink );
      }

      auto comment_blog_itr = comment_blog_idx.lower_bound( community );
      if( comment_blog_itr != comment_blog_idx.end() && comment_blog_itr->community == community )
      {
         results.back().latest_post = get_discussion( comment_blog_itr->comment, 0 );
         results.back().latest_post.blog = comment_blog_api_obj( *comment_blog_itr );
      }
   }
   return results;
}


} } // node::app