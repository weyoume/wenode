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


   //======================//
   // === Account API ==== //
   //======================//



vector< account_api_obj > database_api::get_accounts( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_accounts( names );
   });
}

vector< account_api_obj > database_api_impl::get_accounts( vector< string > names )const
{
   const auto& account_idx  = _db.get_index< account_index >().indices().get< by_name >();

   vector< account_api_obj > results;

   for( auto name: names )
   {
      auto account_itr = account_idx.find( name );
      if( account_itr != account_idx.end() )
      {
         results.push_back( account_api_obj( *account_itr, _db ) );
      }  
   }

   return results;
}


vector< account_api_obj > database_api::get_accounts_by_followers( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_accounts_by_followers( from, limit );
   });
}

vector< account_api_obj > database_api_impl::get_accounts_by_followers( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< account_api_obj > results;
   results.reserve( limit );

   const auto& account_idx  = _db.get_index< account_index >().indices().get< by_follower_count >();
   const auto& name_idx  = _db.get_index< account_index >().indices().get< by_name >();

   auto account_itr = account_idx.begin();
  
   if( from.size() )
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid Community name ${n}", ("n",from) );
      account_itr = account_idx.iterator_to( *name_itr );
   }

   while( account_itr != account_idx.end() && results.size() < limit )
   {
      results.push_back( account_api_obj( *account_itr, _db ) );
      ++account_itr;
   }
   return results;
}

vector< account_concise_api_obj > database_api::get_concise_accounts( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_concise_accounts( names );
   });
}

vector< account_concise_api_obj > database_api_impl::get_concise_accounts( vector< string > names )const
{
   const auto& account_idx  = _db.get_index< account_index >().indices().get< by_name >();

   vector< account_concise_api_obj > results;

   for( auto name: names )
   {
      auto account_itr = account_idx.find( name );
      if ( account_itr != account_idx.end() )
      {
         results.push_back( *account_itr );
      }  
   }

   return results;
}

vector< extended_account > database_api::get_full_accounts( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_full_accounts( names );
   });
}

/**
 * Retrieves all relevant state information regarding a list of specified accounts, including:
 * - Balances, active transfers and requests.
 * - Business Account details, invites and requests.
 * - Governance Account details, requests, votes, resolutions.
 * - Connection Details and requests.
 * - Incoming and Outgoing messages, and conversations with accounts.
 * - Community Moderation and ownership details, and invites and requests.
 * - Producer, network officer, and executive board details and votes.
 * - Interface and Supernode details.
 * - Advertising Campaigns, inventory, creative, bids, and audiences.
 */
vector< extended_account > database_api_impl::get_full_accounts( vector< string > names )const
{
   const auto& account_idx  = _db.get_index< account_index >().indices().get< by_name >();
   const auto& balance_idx = _db.get_index< account_balance_index >().indices().get< by_owner >();

   const auto& verified_verifier_idx = _db.get_index< account_verification_index >().indices().get< by_verified_verifier >();
   const auto& verifier_verified_idx = _db.get_index< account_verification_index >().indices().get< by_verifier_verified >();

   const auto& community_member_idx = _db.get_index< community_member_index >().indices().get< by_member_community_type >();
   const auto& following_idx = _db.get_index< account_following_index >().indices().get< by_account >();
   const auto& permission_idx = _db.get_index< account_permission_index >().indices().get< by_account >();
   const auto& connection_a_idx = _db.get_index< account_connection_index >().indices().get< by_account_a >();
   const auto& connection_b_idx = _db.get_index< account_connection_index >().indices().get< by_account_b >();
   const auto& inbox_idx = _db.get_index< message_index >().indices().get< by_account_inbox >();
   const auto& outbox_idx = _db.get_index< message_index >().indices().get< by_account_outbox >();
   const auto& community_inbox_idx = _db.get_index< message_index >().indices().get< by_community_inbox >();

   const auto& limit_idx = _db.get_index< limit_order_index >().indices().get< by_account >();
   const auto& margin_idx = _db.get_index< margin_order_index >().indices().get< by_account >();
   const auto& call_idx = _db.get_index< call_order_index >().indices().get< by_account >();
   const auto& loan_idx = _db.get_index< credit_loan_index >().indices().get< by_owner >();
   const auto& collateral_idx = _db.get_index< credit_collateral_index >().indices().get< by_owner >();

   const auto& business_idx = _db.get_index< business_index >().indices().get< by_account >();
   const auto& business_permission_idx = _db.get_index< business_permission_index >().indices().get< by_account >();
   const auto& business_executive_idx = _db.get_index< business_executive_index >().indices().get< by_business_executive >();
   const auto& business_director_idx = _db.get_index< business_director_index >().indices().get< by_business_director >();

   const auto& incoming_business_account_director_idx = _db.get_index< business_director_vote_index >().indices().get< by_director >();
   const auto& incoming_business_account_exec_idx = _db.get_index< business_executive_vote_index >().indices().get< by_executive >();
   const auto& incoming_business_director_idx = _db.get_index< business_director_vote_index >().indices().get< by_business_rank >();
   const auto& incoming_business_exec_idx = _db.get_index< business_executive_vote_index >().indices().get< by_business >();
   const auto& outgoing_business_account_director_idx = _db.get_index< business_director_vote_index >().indices().get< by_account_business_rank >();
   const auto& outgoing_business_account_exec_idx = _db.get_index< business_executive_vote_index >().indices().get< by_director_business >();

   const auto& governance_idx = _db.get_index< governance_index >().indices().get< by_account >();
   const auto& governance_permission_idx = _db.get_index< governance_permission_index >().indices().get< by_account >();
   const auto& governance_executive_idx = _db.get_index< governance_executive_index >().indices().get< by_governance_executive >();
   const auto& governance_director_idx = _db.get_index< governance_director_index >().indices().get< by_governance_director >();

   const auto& incoming_governance_account_director_idx = _db.get_index< governance_director_vote_index >().indices().get< by_director >();
   const auto& incoming_governance_account_exec_idx = _db.get_index< governance_executive_vote_index >().indices().get< by_executive >();
   const auto& incoming_governance_director_idx = _db.get_index< governance_director_vote_index >().indices().get< by_governance >();
   const auto& incoming_governance_exec_idx = _db.get_index< governance_executive_vote_index >().indices().get< by_governance >();
   const auto& outgoing_governance_account_director_idx = _db.get_index< governance_director_vote_index >().indices().get< by_account_governance >();
   const auto& outgoing_governance_account_exec_idx = _db.get_index< governance_executive_vote_index >().indices().get< by_director_governance >();

   const auto& incoming_governance_member_idx = _db.get_index< governance_member_index >().indices().get< by_account >();
   const auto& outgoing_governance_member_idx = _db.get_index< governance_member_index >().indices().get< by_governance_account >();
   const auto& incoming_governance_member_request_idx = _db.get_index< governance_member_request_index >().indices().get< by_account >();
   const auto& outgoing_governance_member_request_idx = _db.get_index< governance_member_request_index >().indices().get< by_governance_account >();

   const auto& incoming_governance_resolution_idx = _db.get_index< governance_resolution_index >().indices().get< by_governance_recent >();
   const auto& outgoing_governance_resolution_idx = _db.get_index< governance_resolution_index >().indices().get< by_resolution_id >();
   const auto& incoming_governance_resolution_vote_idx = _db.get_index< governance_resolution_vote_index >().indices().get< by_governance_resolution_id >();
   const auto& outgoing_governance_resolution_vote_idx = _db.get_index< governance_resolution_vote_index >().indices().get< by_account_recent >();

   const auto& community_req_idx = _db.get_index< community_member_request_index >().indices().get< by_account_community_type >();
   const auto& incoming_member_idx = _db.get_index< community_member_vote_index >().indices().get< by_member_community_rank >();
   const auto& outgoing_member_idx = _db.get_index< community_member_vote_index >().indices().get< by_account_community_rank >();
   const auto& community_event_idx = _db.get_index< community_event_index >().indices().get< by_event_id >();
   const auto& community_event_attend_idx = _db.get_index< community_event_attend_index >().indices().get< by_attendee >();
   const auto& community_event_attend_id_idx = _db.get_index< community_event_attend_index >().indices().get< by_event_id >();

   const auto& directive_member_idx = _db.get_index< community_directive_member_index >().indices().get< by_member_community >();
   const auto& directive_member_votes_idx = _db.get_index< community_directive_member_index >().indices().get< by_community_votes >();
   const auto& member_vote_idx = _db.get_index< community_directive_member_vote_index >().indices().get< by_community_voter_member >();
   const auto& member_community_voter_idx = _db.get_index< community_directive_member_vote_index >().indices().get< by_member_community_voter >();
   const auto& directive_idx = _db.get_index< community_directive_index >().indices().get< by_community_votes >();
   const auto& directive_id_idx = _db.get_index< community_directive_index >().indices().get< by_directive_id >();
   const auto& directive_vote_idx = _db.get_index< community_directive_vote_index >().indices().get< by_voter_community_directive >();
   const auto& directive_approve_idx = _db.get_index< community_directive_vote_index >().indices().get< by_directive_approve >();

   const auto& transfer_req_idx = _db.get_index< transfer_request_index >().indices().get< by_request_id >();
   const auto& transfer_from_req_idx = _db.get_index< transfer_request_index >().indices().get< by_from_account >();
   const auto& recurring_idx = _db.get_index< transfer_recurring_index >().indices().get< by_transfer_id >();
   const auto& recurring_to_idx = _db.get_index< transfer_recurring_index >().indices().get< by_to_account >();
   const auto& recurring_req_idx = _db.get_index< transfer_recurring_request_index >().indices().get< by_request_id >();
   const auto& recurring_from_req_idx = _db.get_index< transfer_recurring_request_index >().indices().get< by_from_account >();

   const auto& producer_idx = _db.get_index< producer_index >().indices().get< by_name >();
   const auto& officer_idx = _db.get_index< network_officer_index >().indices().get< by_account >();
   const auto& enterprise_idx = _db.get_index< enterprise_index >().indices().get< by_account >();
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

   const auto& creative_idx = _db.get_index< ad_creative_index >().indices().get< by_latest >();
   const auto& campaign_idx = _db.get_index< ad_campaign_index >().indices().get< by_latest >();
   const auto& audience_idx = _db.get_index< ad_audience_index >().indices().get< by_latest >();
   const auto& inventory_idx = _db.get_index< ad_inventory_index >().indices().get< by_latest >();

   const auto& creative_id_idx = _db.get_index< ad_creative_index >().indices().get< by_creative_id >();
   const auto& campaign_id_idx = _db.get_index< ad_campaign_index >().indices().get< by_campaign_id >();
   const auto& audience_id_idx = _db.get_index< ad_audience_index >().indices().get< by_audience_id >();
   const auto& inventory_id_idx = _db.get_index< ad_inventory_index >().indices().get< by_inventory_id >();

   const auto& bidder_bid_idx = _db.get_index< ad_bid_index >().indices().get< by_bidder_updated >();
   const auto& account_bid_idx = _db.get_index< ad_bid_index >().indices().get< by_account_updated >();
   const auto& author_bid_idx = _db.get_index< ad_bid_index >().indices().get< by_author_updated >();
   const auto& provider_bid_idx = _db.get_index< ad_bid_index >().indices().get< by_provider_updated >();

   const auto& product_idx = _db.get_index< product_sale_index >().indices().get< by_product_id >();
   const auto& seller_purchase_idx = _db.get_index< product_purchase_index >().indices().get< by_product_id >();
   const auto& buyer_purchase_idx = _db.get_index< product_purchase_index >().indices().get< by_order_id >();

   const auto& auction_idx = _db.get_index< product_auction_sale_index >().indices().get< by_auction_id >();
   const auto& seller_bid_idx = _db.get_index< product_auction_bid_index >().indices().get< by_auction_id >();
   const auto& buyer_bid_idx = _db.get_index< product_auction_bid_index >().indices().get< by_bid_id >();

   const auto& account_premium_purchase_idx = _db.get_index< premium_purchase_index >().indices().get< by_account_comment >();
   const auto& author_premium_purchase_idx = _db.get_index< premium_purchase_index >().indices().get< by_author_comment >();
   const auto& account_premium_purchase_key_idx = _db.get_index< premium_purchase_key_index >().indices().get< by_account_comment >();
   const auto& author_premium_purchase_key_idx = _db.get_index< premium_purchase_key_index >().indices().get< by_author_comment >();

   const auto& comment_blog_idx = _db.get_index< comment_blog_index >().indices().get< by_new_account_blog >();

   const auto& owner_history_idx = _db.get_index< account_authority_history_index >().indices().get< by_account >();
   const auto& recovery_idx = _db.get_index< account_recovery_request_index >().indices().get< by_account >();

   const auto& history_idx = _db.get_index< account_history_index >().indices().get< by_account >();

   vector< extended_account > results;

   for( auto name: names )
   {
      auto account_itr = account_idx.find( name );
      if ( account_itr != account_idx.end() )
      {
         results.push_back( extended_account( *account_itr, _db ) );

         auto following_itr = following_idx.find( name );
         if( following_itr != following_idx.end() )
         {
            results.back().following = account_following_api_obj( *following_itr );
         }

         auto connection_a_itr = connection_a_idx.lower_bound( boost::make_tuple( name, connection_tier_type::CONNECTION ) );
         auto connection_b_itr = connection_b_idx.lower_bound( boost::make_tuple( name, connection_tier_type::CONNECTION ) );
         while( connection_a_itr != connection_a_idx.end() && 
            connection_a_itr->account_a == name &&
            connection_a_itr->connection_type == connection_tier_type::CONNECTION )
         {
            results.back().connections.connections[ connection_a_itr->account_b ] = account_connection_api_obj( *connection_a_itr );
            results.back().keychain.connection_keys[ connection_a_itr->account_b ] = connection_a_itr->encrypted_key_b;
            ++connection_a_itr;
         }
         while( connection_b_itr != connection_b_idx.end() && 
            connection_b_itr->account_b == name &&
            connection_b_itr->connection_type == connection_tier_type::CONNECTION )
         {
            results.back().connections.connections[ connection_b_itr->account_a ] = account_connection_api_obj( *connection_b_itr );
            results.back().keychain.connection_keys[ connection_b_itr->account_a ] = connection_b_itr->encrypted_key_a;
            ++connection_b_itr;
         }

         connection_a_itr = connection_a_idx.lower_bound( boost::make_tuple( name, connection_tier_type::FRIEND ) );
         connection_b_itr = connection_b_idx.lower_bound( boost::make_tuple( name, connection_tier_type::FRIEND ) );
         while( connection_a_itr != connection_a_idx.end() && 
            connection_a_itr->account_a == name &&
            connection_a_itr->connection_type == connection_tier_type::FRIEND )
         {
            results.back().connections.friends[ connection_a_itr->account_b ] = account_connection_api_obj( *connection_a_itr );
            results.back().keychain.friend_keys[ connection_a_itr->account_b ] = connection_a_itr->encrypted_key_b;
            ++connection_a_itr;
         }
         while( connection_b_itr != connection_b_idx.end() && 
            connection_b_itr->account_b == name &&
            connection_b_itr->connection_type == connection_tier_type::FRIEND )
         {
            results.back().connections.friends[ connection_b_itr->account_a ] = account_connection_api_obj( *connection_b_itr );
            results.back().keychain.friend_keys[ connection_b_itr->account_a ] = connection_b_itr->encrypted_key_a;
            ++connection_b_itr;
         }

         connection_a_itr = connection_a_idx.lower_bound( boost::make_tuple( name, connection_tier_type::COMPANION ) );
         connection_b_itr = connection_b_idx.lower_bound( boost::make_tuple( name, connection_tier_type::COMPANION ) );
         while( connection_a_itr != connection_a_idx.end() && 
            connection_a_itr->account_a == name &&
            connection_a_itr->connection_type == connection_tier_type::COMPANION )
         {
            results.back().connections.companions[ connection_a_itr->account_b ] = account_connection_api_obj( *connection_a_itr );
            results.back().keychain.companion_keys[ connection_a_itr->account_b ] = connection_a_itr->encrypted_key_b;
            ++connection_a_itr;
         }
         while( connection_b_itr != connection_b_idx.end() && 
            connection_b_itr->account_b == name &&
            connection_b_itr->connection_type == connection_tier_type::COMPANION )
         {
            results.back().connections.companions[ connection_b_itr->account_a ] = account_connection_api_obj( *connection_b_itr );
            results.back().keychain.companion_keys[ connection_b_itr->account_a ] = connection_b_itr->encrypted_key_a;
            ++connection_b_itr;
         }

         auto verifier_verified_itr = verifier_verified_idx.lower_bound( name );
         while( verifier_verified_itr != verifier_verified_idx.end() &&
            verifier_verified_itr->verifier_account == name )
         {
            results.back().connections.outgoing_verifications[ verifier_verified_itr->verified_account ] = account_verification_api_obj( *verifier_verified_itr );
            ++verifier_verified_itr;
         }

         auto verified_verifier_itr = verified_verifier_idx.lower_bound( name );
         while( verified_verifier_itr != verified_verifier_idx.end() &&
            verified_verifier_itr->verified_account == name )
         {
            results.back().connections.incoming_verifications[ verified_verifier_itr->verifier_account ] = account_verification_api_obj( *verified_verifier_itr );
            ++verified_verifier_itr;
         }

         auto permission_itr = permission_idx.find( name );
         if( permission_itr != permission_idx.end() )
         {
            results.back().permissions = account_permission_api_obj( *permission_itr );
         }


         // Business


         auto business_itr = business_idx.find( name );
         if( business_itr != business_idx.end() )
         {
            results.back().business.business = business_api_obj( *business_itr );
         }

         auto business_permission_itr = business_permission_idx.find( name );
         if( business_permission_itr != business_permission_idx.end() )
         {
            results.back().business.permissions = business_permission_api_obj( *business_permission_itr );
         }

         auto business_executive_itr = business_executive_idx.lower_bound( name );
         while( business_executive_itr != business_executive_idx.end() && 
            business_executive_itr->business == name )
         {
            results.back().business.executives[ business_executive_itr->executive ] = business_executive_api_obj( *business_executive_itr );
            ++business_executive_itr;
         }

         auto business_director_itr = business_director_idx.lower_bound( name );
         while( business_director_itr != business_director_idx.end() && 
            business_director_itr->business == name )
         {
            results.back().business.directors[ business_director_itr->director ] = business_director_api_obj( *business_director_itr );
            ++business_director_itr;
         }

         auto incoming_business_account_exec_itr = incoming_business_account_exec_idx.lower_bound( name );
         while( incoming_business_account_exec_itr != incoming_business_account_exec_idx.end() && 
            incoming_business_account_exec_itr->executive == name )
         {
            results.back().business.incoming_executive_votes[ incoming_business_account_exec_itr->business ][ incoming_business_account_exec_itr->director ] = business_executive_vote_api_obj( *incoming_business_account_exec_itr );
            ++incoming_business_account_exec_itr;
         }

         auto incoming_business_account_director_itr = incoming_business_account_director_idx.lower_bound( name );
         while( incoming_business_account_director_itr != incoming_business_account_director_idx.end() && 
            incoming_business_account_director_itr->director == name )
         {
            results.back().business.incoming_director_votes[ incoming_business_account_director_itr->business ][ incoming_business_account_director_itr->account ] = business_director_vote_api_obj( *incoming_business_account_director_itr );
            ++incoming_business_account_director_itr;
         }

         auto incoming_business_exec_itr = incoming_business_exec_idx.lower_bound( name );
         while( incoming_business_exec_itr != incoming_business_exec_idx.end() && 
            incoming_business_exec_itr->business == name )
         {
            results.back().business.incoming_executive_votes[ incoming_business_exec_itr->executive ][ incoming_business_exec_itr->director ] = business_executive_vote_api_obj( *incoming_business_exec_itr );
            ++incoming_business_exec_itr;
         }

         auto incoming_business_director_itr = incoming_business_director_idx.lower_bound( name );
         while( incoming_business_director_itr != incoming_business_director_idx.end() && 
            incoming_business_director_itr->business == name )
         {
            results.back().business.incoming_director_votes[ incoming_business_director_itr->director ][ incoming_business_director_itr->account ] = business_director_vote_api_obj( *incoming_business_director_itr );
            ++incoming_business_director_itr;
         }

         auto outgoing_business_account_exec_itr = outgoing_business_account_exec_idx.lower_bound( name );
         while( outgoing_business_account_exec_itr != outgoing_business_account_exec_idx.end() && 
            outgoing_business_account_exec_itr->director == name )
         {
            results.back().business.outgoing_executive_votes[ outgoing_business_account_exec_itr->business ][ outgoing_business_account_exec_itr->executive ] = business_executive_vote_api_obj( *outgoing_business_account_exec_itr );
            ++outgoing_business_account_exec_itr;
         }

         auto outgoing_business_account_director_itr = outgoing_business_account_director_idx.lower_bound( name );
         while( outgoing_business_account_director_itr != outgoing_business_account_director_idx.end() && 
            outgoing_business_account_director_itr->account == name )
         {
            results.back().business.outgoing_director_votes[ outgoing_business_account_director_itr->business ][ outgoing_business_account_director_itr->director ] = business_director_vote_api_obj( *outgoing_business_account_director_itr );
            ++outgoing_business_account_director_itr;
         }

         // Governance

         auto governance_itr = governance_idx.find( name );
         if( governance_itr != governance_idx.end() )
         {
            results.back().governance.governance = governance_api_obj( *governance_itr );
         }

         auto governance_permission_itr = governance_permission_idx.find( name );
         if( governance_permission_itr != governance_permission_idx.end() )
         {
            results.back().governance.permissions = governance_permission_api_obj( *governance_permission_itr );
         }

         auto governance_executive_itr = governance_executive_idx.lower_bound( name );
         while( governance_executive_itr != governance_executive_idx.end() && 
            governance_executive_itr->governance == name )
         {
            results.back().governance.executives[ governance_executive_itr->executive ] = governance_executive_api_obj( *governance_executive_itr );
            ++governance_executive_itr;
         }

         auto governance_director_itr = governance_director_idx.lower_bound( name );
         while( governance_director_itr != governance_director_idx.end() && 
            governance_director_itr->governance == name )
         {
            results.back().governance.directors[ governance_director_itr->director ] = governance_director_api_obj( *governance_director_itr );
            ++governance_director_itr;
         }

         auto incoming_governance_account_exec_itr = incoming_governance_account_exec_idx.lower_bound( name );
         while( incoming_governance_account_exec_itr != incoming_governance_account_exec_idx.end() && 
            incoming_governance_account_exec_itr->executive == name )
         {
            results.back().governance.incoming_executive_votes[ incoming_governance_account_exec_itr->governance ][ incoming_governance_account_exec_itr->director ] = governance_executive_vote_api_obj( *incoming_governance_account_exec_itr );
            ++incoming_governance_account_exec_itr;
         }

         auto incoming_governance_account_director_itr = incoming_governance_account_director_idx.lower_bound( name );
         while( incoming_governance_account_director_itr != incoming_governance_account_director_idx.end() && 
            incoming_governance_account_director_itr->director == name )
         {
            results.back().governance.incoming_director_votes[ incoming_governance_account_director_itr->governance ][ incoming_governance_account_director_itr->account ] = governance_director_vote_api_obj( *incoming_governance_account_director_itr );
            ++incoming_governance_account_director_itr;
         }

         auto incoming_governance_exec_itr = incoming_governance_exec_idx.lower_bound( name );
         while( incoming_governance_exec_itr != incoming_governance_exec_idx.end() && 
            incoming_governance_exec_itr->governance == name )
         {
            results.back().governance.incoming_executive_votes[ incoming_governance_exec_itr->executive ][ incoming_governance_exec_itr->director ] = governance_executive_vote_api_obj( *incoming_governance_exec_itr );
            ++incoming_governance_exec_itr;
         }

         auto incoming_governance_director_itr = incoming_governance_director_idx.lower_bound( name );
         while( incoming_governance_director_itr != incoming_governance_director_idx.end() && 
            incoming_governance_director_itr->governance == name )
         {
            results.back().governance.incoming_director_votes[ incoming_governance_director_itr->director ][ incoming_governance_director_itr->account ] = governance_director_vote_api_obj( *incoming_governance_director_itr );
            ++incoming_governance_director_itr;
         }

         auto outgoing_governance_account_exec_itr = outgoing_governance_account_exec_idx.lower_bound( name );
         while( outgoing_governance_account_exec_itr != outgoing_governance_account_exec_idx.end() && 
            outgoing_governance_account_exec_itr->director == name )
         {
            results.back().governance.outgoing_executive_votes[ outgoing_governance_account_exec_itr->governance ][ outgoing_governance_account_exec_itr->executive ] = governance_executive_vote_api_obj( *outgoing_governance_account_exec_itr );
            ++outgoing_governance_account_exec_itr;
         }

         auto outgoing_governance_account_director_itr = outgoing_governance_account_director_idx.lower_bound( name );
         while( outgoing_governance_account_director_itr != outgoing_governance_account_director_idx.end() && 
            outgoing_governance_account_director_itr->account == name )
         {
            results.back().governance.outgoing_director_votes[ outgoing_governance_account_director_itr->governance ][ outgoing_governance_account_director_itr->director ] = governance_director_vote_api_obj( *outgoing_governance_account_director_itr );
            ++outgoing_governance_account_director_itr;
         }

         auto incoming_governance_member_itr = incoming_governance_member_idx.lower_bound( name );
         while( incoming_governance_member_itr != incoming_governance_member_idx.end() && 
            incoming_governance_member_itr->governance == name )
         {
            results.back().governance.incoming_members[ incoming_governance_member_itr->account ] = governance_member_api_obj( *incoming_governance_member_itr );
            ++incoming_governance_member_itr;
         }

         auto outgoing_governance_member_itr = outgoing_governance_member_idx.find( name );
         if( outgoing_governance_member_itr != outgoing_governance_member_idx.end() )
         {
            results.back().governance.outgoing_member = governance_member_api_obj( *outgoing_governance_member_itr );
         }

         auto incoming_governance_member_request_itr = incoming_governance_member_request_idx.lower_bound( name );
         while( incoming_governance_member_request_itr != incoming_governance_member_request_idx.end() && 
            incoming_governance_member_request_itr->governance == name )
         {
            results.back().governance.incoming_requests[ incoming_governance_member_request_itr->account ] = governance_member_request_api_obj( *incoming_governance_member_request_itr );
            ++incoming_governance_member_request_itr;
         }

         auto outgoing_governance_member_request_itr = outgoing_governance_member_request_idx.find( name );
         if( outgoing_governance_member_request_itr != outgoing_governance_member_request_idx.end() )
         {
            results.back().governance.outgoing_request = governance_member_request_api_obj( *outgoing_governance_member_request_itr );
         }

         auto incoming_governance_resolution_itr = incoming_governance_resolution_idx.lower_bound( name );
         auto incoming_governance_resolution_vote_itr = incoming_governance_resolution_vote_idx.lower_bound( name );

         while( incoming_governance_resolution_itr != incoming_governance_resolution_idx.end() && 
            incoming_governance_resolution_itr->governance == name )
         {
            governance_resolution_state gstate;
            gstate.resolution = *incoming_governance_resolution_itr;

            incoming_governance_resolution_vote_itr = incoming_governance_resolution_vote_idx.lower_bound( boost::make_tuple( name, incoming_governance_resolution_itr->resolution_id, incoming_governance_resolution_itr->ammendment_id ) );

            while( incoming_governance_resolution_vote_itr != incoming_governance_resolution_vote_idx.end() && 
               incoming_governance_resolution_vote_itr->governance == name &&
               incoming_governance_resolution_vote_itr->resolution_id == incoming_governance_resolution_itr->resolution_id &&
               incoming_governance_resolution_vote_itr->ammendment_id == incoming_governance_resolution_itr->ammendment_id )
            {
               if( incoming_governance_resolution_vote_itr->approved )
               {
                  gstate.approval_votes[ incoming_governance_resolution_vote_itr->account ] = governance_resolution_vote_api_obj( *incoming_governance_resolution_vote_itr );
                  gstate.approval_vote_count++;
               }
               else
               {
                  gstate.disapproval_votes[ incoming_governance_resolution_vote_itr->account ] = governance_resolution_vote_api_obj( *incoming_governance_resolution_vote_itr );
                  gstate.disapproval_vote_count++;
               }
               
               ++incoming_governance_resolution_vote_itr;
            }

            results.back().governance.incoming_resolutions.push_back( gstate );

            ++incoming_governance_resolution_itr;
         }

         auto outgoing_governance_resolution_itr = outgoing_governance_resolution_idx.lower_bound( name );
         auto outgoing_governance_resolution_vote_itr = outgoing_governance_resolution_vote_idx.lower_bound( name );

         while( outgoing_governance_resolution_vote_itr != outgoing_governance_resolution_vote_idx.end() && 
            outgoing_governance_resolution_vote_itr->account == name )
         {
            outgoing_governance_resolution_itr = outgoing_governance_resolution_idx.lower_bound( boost::make_tuple( outgoing_governance_resolution_vote_itr->governance, outgoing_governance_resolution_vote_itr->resolution_id, outgoing_governance_resolution_vote_itr->ammendment_id ) );
            incoming_governance_resolution_vote_itr = incoming_governance_resolution_vote_idx.lower_bound( boost::make_tuple( outgoing_governance_resolution_vote_itr->governance, outgoing_governance_resolution_vote_itr->resolution_id, outgoing_governance_resolution_vote_itr->ammendment_id ) );

            governance_resolution_state gstate;
            gstate.resolution = *outgoing_governance_resolution_itr;

            while( incoming_governance_resolution_vote_itr != incoming_governance_resolution_vote_idx.end() &&
               incoming_governance_resolution_vote_itr->governance == outgoing_governance_resolution_itr->governance &&
               incoming_governance_resolution_vote_itr->resolution_id == outgoing_governance_resolution_itr->resolution_id &&
               incoming_governance_resolution_vote_itr->ammendment_id == outgoing_governance_resolution_itr->ammendment_id )
            {
               if( incoming_governance_resolution_vote_itr->approved )
               {
                  gstate.approval_votes[ incoming_governance_resolution_vote_itr->account ] = governance_resolution_vote_api_obj( *incoming_governance_resolution_vote_itr );
                  gstate.approval_vote_count++;
               }
               else
               {
                  gstate.disapproval_votes[ incoming_governance_resolution_vote_itr->account ] = governance_resolution_vote_api_obj( *incoming_governance_resolution_vote_itr );
                  gstate.disapproval_vote_count++;
               }
               
               ++incoming_governance_resolution_vote_itr;
            }

            results.back().governance.outgoing_resolutions.push_back( gstate );

            ++outgoing_governance_resolution_vote_itr;
         }

         // Network

         auto officer_itr = officer_idx.find( name );
         if( officer_itr != officer_idx.end() )
         {
            results.back().network.network_officer = network_officer_api_obj( *officer_itr );
         }

         auto interface_itr = interface_idx.find( name );
         if( interface_itr != interface_idx.end() )
         {
            results.back().network.interface = interface_api_obj( *interface_itr );
         }

         auto enterprise_itr = enterprise_idx.lower_bound( name );
         while( enterprise_itr != enterprise_idx.end() && enterprise_itr->account == name )
         {
            results.back().network.enterprise_proposals.push_back( enterprise_api_obj( *enterprise_itr ) );
            ++enterprise_itr;
         }

         auto validation_itr = validation_idx.lower_bound( name );
         while( validation_itr != validation_idx.end() && 
            validation_itr->producer == name &&
            results.back().network.block_validations.size() < 100 )
         {
            results.back().network.block_validations.push_back( block_validation_api_obj( *validation_itr ) );
            ++validation_itr;
         }

         auto incoming_producer_vote_itr = incoming_producer_vote_idx.lower_bound( name );
         while( incoming_producer_vote_itr != incoming_producer_vote_idx.end() && 
            incoming_producer_vote_itr->producer == name )
         {
            results.back().network.incoming_producer_votes[ incoming_producer_vote_itr->account ] = producer_vote_api_obj( *incoming_producer_vote_itr );
            ++incoming_producer_vote_itr;
         }

         auto incoming_officer_vote_itr = incoming_officer_vote_idx.lower_bound( name );
         while( incoming_officer_vote_itr != incoming_officer_vote_idx.end() && 
            incoming_officer_vote_itr->network_officer == name )
         {
            results.back().network.incoming_network_officer_votes[ incoming_officer_vote_itr->account ] = network_officer_vote_api_obj( *incoming_officer_vote_itr );
            ++incoming_officer_vote_itr;
         }

         auto incoming_enterprise_vote_itr = incoming_enterprise_vote_idx.lower_bound( name );
         while( incoming_enterprise_vote_itr != incoming_enterprise_vote_idx.end() && 
            incoming_enterprise_vote_itr->account == name )
         {
            results.back().network.incoming_enterprise_votes[ incoming_enterprise_vote_itr->voter ][ to_string( incoming_enterprise_vote_itr->enterprise_id ) ] = enterprise_vote_api_obj( *incoming_enterprise_vote_itr );
            ++incoming_enterprise_vote_itr;
         }

         auto incoming_enterprise_fund_itr = incoming_enterprise_fund_idx.lower_bound( name );
         while( incoming_enterprise_fund_itr != incoming_enterprise_fund_idx.end() && 
            incoming_enterprise_fund_itr->account == name )
         {
            results.back().network.incoming_enterprise_funds[ incoming_enterprise_fund_itr->funder ][ to_string( incoming_enterprise_fund_itr->enterprise_id ) ] = enterprise_fund_api_obj( *incoming_enterprise_fund_itr );
            ++incoming_enterprise_fund_itr;
         }

         auto incoming_commit_violation_itr = incoming_commit_violation_idx.lower_bound( name );
         while( incoming_commit_violation_itr != incoming_commit_violation_idx.end() && 
            incoming_commit_violation_itr->producer == name &&
            results.back().network.incoming_commit_violations.size() < 100 )
         {
            results.back().network.incoming_commit_violations[ incoming_commit_violation_itr->reporter ] = commit_violation_api_obj( *incoming_commit_violation_itr );
            ++incoming_commit_violation_itr;
         }

         auto outgoing_producer_vote_itr = outgoing_producer_vote_idx.lower_bound( name );
         while( outgoing_producer_vote_itr != outgoing_producer_vote_idx.end() && 
            outgoing_producer_vote_itr->account == name ) 
         {
            results.back().network.outgoing_producer_votes[ outgoing_producer_vote_itr->producer ] = producer_vote_api_obj( *outgoing_producer_vote_itr );
            ++outgoing_producer_vote_itr;
         }

         auto outgoing_officer_vote_itr = outgoing_officer_vote_idx.lower_bound( name );
         while( outgoing_officer_vote_itr != outgoing_officer_vote_idx.end() && 
            outgoing_officer_vote_itr->account == name )
         {
            results.back().network.outgoing_network_officer_votes[ outgoing_officer_vote_itr->network_officer ] = network_officer_vote_api_obj( *outgoing_officer_vote_itr );
            ++outgoing_officer_vote_itr;
         }

         auto outgoing_enterprise_vote_itr = outgoing_enterprise_vote_idx.lower_bound( name );
         while( outgoing_enterprise_vote_itr != outgoing_enterprise_vote_idx.end() && 
            outgoing_enterprise_vote_itr->voter == name )
         {
            results.back().network.outgoing_enterprise_votes[ outgoing_enterprise_vote_itr->account ][ to_string( outgoing_enterprise_vote_itr->enterprise_id ) ] = enterprise_vote_api_obj( *outgoing_enterprise_vote_itr );
            ++outgoing_enterprise_vote_itr;
         }

         auto outgoing_enterprise_fund_itr = outgoing_enterprise_fund_idx.lower_bound( name );
         while( outgoing_enterprise_fund_itr != outgoing_enterprise_fund_idx.end() && 
            outgoing_enterprise_fund_itr->funder == name )
         {
            results.back().network.outgoing_enterprise_funds[ outgoing_enterprise_fund_itr->account ][ to_string( outgoing_enterprise_fund_itr->enterprise_id ) ] = enterprise_fund_api_obj( *outgoing_enterprise_fund_itr );
            ++outgoing_enterprise_fund_itr;
         }

         auto outgoing_commit_violation_itr = outgoing_commit_violation_idx.lower_bound( name );
         while( outgoing_commit_violation_itr != outgoing_commit_violation_idx.end() && 
            outgoing_commit_violation_itr->reporter == name &&
            results.back().network.outgoing_commit_violations.size() < 100 )
         {
            results.back().network.outgoing_commit_violations[ outgoing_commit_violation_itr->producer ] = commit_violation_api_obj( *outgoing_commit_violation_itr );
            ++outgoing_commit_violation_itr;
         }

         auto supernode_itr = supernode_idx.find( name );
         if( supernode_itr != supernode_idx.end() )
         {
            results.back().network.supernode = supernode_api_obj( *supernode_itr );
         }

         // Balance

         auto balance_itr = balance_idx.lower_bound( name );
         while( balance_itr != balance_idx.end() && balance_itr->owner == name )
         {
            results.back().balances.balances[ balance_itr->symbol ] = account_balance_api_obj( *balance_itr );
            ++balance_itr;
         }
   
         auto limit_itr = limit_idx.lower_bound( name );
         while( limit_itr != limit_idx.end() && limit_itr->seller == name ) 
         {
            results.back().orders.limit_orders.push_back( limit_order_api_obj( *limit_itr ) );
            ++limit_itr;
         }

         auto margin_itr = margin_idx.lower_bound( name );
         while( margin_itr != margin_idx.end() && margin_itr->owner == name )
         {
            results.back().orders.margin_orders.push_back( margin_order_api_obj( *margin_itr ) );
            ++margin_itr;
         }

         auto call_itr = call_idx.lower_bound( name );
         while( call_itr != call_idx.end() && call_itr->borrower == name )
         {
            results.back().orders.call_orders.push_back( call_order_api_obj( *call_itr ) );
            ++call_itr;
         }

         auto loan_itr = loan_idx.lower_bound( name );
         while( loan_itr != loan_idx.end() && loan_itr->owner == name )
         {
            results.back().orders.loan_orders.push_back( credit_loan_api_obj( *loan_itr ) );
            ++loan_itr;
         }

         auto collateral_itr = collateral_idx.lower_bound( name );
         while( collateral_itr != collateral_idx.end() && collateral_itr->owner == name )
         {
            results.back().orders.collateral.push_back( credit_collateral_api_obj( *collateral_itr ) );
            ++collateral_itr;
         }

         auto inbox_itr = inbox_idx.lower_bound( name );
         auto outbox_itr = outbox_idx.lower_bound( name );
         auto community_inbox_itr = community_inbox_idx.begin();

         vector< message_api_obj > inbox;
         vector< message_api_obj > outbox;

         map< account_name_type, vector< message_api_obj > > account_conversations;
         map< community_name_type, vector< message_api_obj > > community_conversations;

         while( inbox_itr != inbox_idx.end() && inbox_itr->recipient == name )
         {
            inbox.push_back( message_api_obj( *inbox_itr ) );
            ++inbox_itr;
         }

         while( outbox_itr != outbox_idx.end() && outbox_itr->sender == name )
         {
            outbox.push_back( message_api_obj( *outbox_itr ) );
            ++outbox_itr;
         }

         for( auto message : inbox )
         {
            account_conversations[ message.sender ].push_back( message );
         }

         for( auto message : outbox )
         {
            account_conversations[ message.recipient ].push_back( message );
         }

         const account_following_object& account_following = *following_itr;

         for( auto community : account_following.member_communities )
         {
            community_inbox_itr = community_inbox_idx.lower_bound( community );
            while( community_inbox_itr != community_inbox_idx.end() && 
               community_inbox_itr->community == community )
            {
               community_conversations[ community ].push_back( message_api_obj( *community_inbox_itr ) );
               ++community_inbox_itr;
            }
         }

         for( auto conv : account_conversations )
         {
            vector< message_api_obj > thread = conv.second;
            std::sort( thread.begin(), thread.end(), [&]( message_api_obj a, message_api_obj b )
            {
               return a.created < b.created;
            });
            account_conversations[ conv.first ] = thread;
         }

         for( auto conv : community_conversations )
         {
            vector< message_api_obj > thread = conv.second;
            std::sort( thread.begin(), thread.end(), [&]( message_api_obj a, message_api_obj b )
            {
               return a.created < b.created;
            });
            community_conversations[ conv.first ] = thread;
         }

         account_message_state mstate;

         mstate.account_conversations = account_conversations;
         mstate.community_conversations = community_conversations;

         results.back().messages = mstate;

         auto transfer_req_itr = transfer_req_idx.lower_bound( name );
         auto transfer_from_req_itr = transfer_from_req_idx.lower_bound( name );
         auto recurring_itr = recurring_idx.lower_bound( name );
         auto recurring_to_itr = recurring_to_idx.lower_bound( name );
         auto recurring_req_itr = recurring_req_idx.lower_bound( name );
         auto recurring_from_req_itr = recurring_from_req_idx.lower_bound( name );

         while( transfer_req_itr != transfer_req_idx.end() && 
            transfer_req_itr->to == name )
         {
            results.back().transfers.outgoing_requests[ transfer_req_itr->from ] = transfer_request_api_obj( *transfer_req_itr );
            ++transfer_req_itr;
         }

         while( transfer_from_req_itr != transfer_from_req_idx.end() && 
            transfer_from_req_itr->from == name )
         {
            results.back().transfers.incoming_requests[ transfer_from_req_itr->to ] = transfer_request_api_obj( *transfer_from_req_itr );
            ++transfer_from_req_itr;
         }

         while( recurring_itr != recurring_idx.end() && 
            recurring_itr->from == name )
         {
            results.back().transfers.outgoing_recurring_transfers[ recurring_itr->to ] = transfer_recurring_api_obj( *recurring_itr );
            ++recurring_itr;
         }

         while( recurring_to_itr != recurring_to_idx.end() && 
            recurring_to_itr->to == name )
         {
            results.back().transfers.incoming_recurring_transfers[ recurring_to_itr->from ] = transfer_recurring_api_obj( *recurring_to_itr );
            ++recurring_to_itr;
         }

         while( recurring_req_itr != recurring_req_idx.end() && 
            recurring_req_itr->to == name )
         {
            results.back().transfers.outgoing_recurring_transfer_requests[ recurring_req_itr->from ] = transfer_recurring_request_api_obj( *recurring_req_itr );
            ++recurring_req_itr;
         }

         while( recurring_from_req_itr != recurring_from_req_idx.end() && 
            recurring_from_req_itr->from == name )
         {
            results.back().transfers.incoming_recurring_transfer_requests[ recurring_from_req_itr->to ] = transfer_recurring_request_api_obj( *recurring_from_req_itr );
            ++recurring_from_req_itr;
         }

         auto community_req_itr = community_req_idx.lower_bound( name );
         auto incoming_member_itr = incoming_member_idx.lower_bound( name );
         auto outgoing_member_itr = outgoing_member_idx.lower_bound( name );

         while( community_req_itr != community_req_idx.end() &&
            community_req_itr->account == name )
         {
            results.back().communities.pending_requests[ community_req_itr->community ] = community_member_request_api_obj( *community_req_itr );
            ++community_req_itr;
         }

         while( incoming_member_itr != incoming_member_idx.end() && 
            incoming_member_itr->member == name )
         {
            results.back().communities.incoming_member_votes[ incoming_member_itr->community ].push_back( community_member_vote_api_obj( *incoming_member_itr ) );
            ++incoming_member_itr;
         }

         while( outgoing_member_itr != outgoing_member_idx.end() && 
            outgoing_member_itr->account == name )
         {
            results.back().communities.outgoing_member_votes[ outgoing_member_itr->community ].push_back( community_member_vote_api_obj( *outgoing_member_itr ) );
            ++outgoing_member_itr;
         }

         auto community_member_itr = community_member_idx.lower_bound( name );
         while( community_member_itr != community_member_idx.end() && 
            community_member_itr->member == name )
         {
            switch( community_member_itr->member_type )
            {
               case community_permission_type::MEMBER_PERMISSION:
               {
                  results.back().keychain.community_member_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  results.back().keychain.community_moderator_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  results.back().keychain.community_admin_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  results.back().keychain.community_secure_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  results.back().keychain.community_standard_premium_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  results.back().keychain.community_mid_premium_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  results.back().keychain.community_top_premium_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid Communiy Member type" );
               }
            }
            
            ++community_member_itr;
         }

         auto directive_member_itr = directive_member_idx.lower_bound( name );

         while( directive_member_itr != directive_member_idx.end() && 
            directive_member_itr->account == name )
         {
            const community_directive_member_object& member = *directive_member_itr;
            results.back().communities.directive_members[ member.community ] = community_directive_member_api_obj( member );

            auto incoming_vote_itr = member_community_voter_idx.lower_bound( boost::make_tuple( member.account, member.community ) );

            while( incoming_vote_itr != member_community_voter_idx.end() &&
               incoming_vote_itr->member == member.account &&
               incoming_vote_itr->community == member.community )
            {
               const community_directive_member_vote_object& vote = *incoming_vote_itr;

               results.back().communities.incoming_directive_member_votes[ member.community ].push_back( community_directive_member_vote_api_obj( vote ) );
               ++incoming_vote_itr;
            }

            auto directive_member_votes_itr = directive_member_votes_idx.lower_bound( member.community );

            const community_directive_object* command_directive_ptr = nullptr;
         
            if( directive_member_votes_itr != directive_member_votes_idx.end() && 
               directive_member_votes_itr->community == member.community )
            {
               const community_directive_member_object& member = *directive_member_votes_itr;
               command_directive_ptr = _db.find_community_directive( member.account, member.command_directive_id );
            }

            const community_directive_object* consensus_directive_ptr = nullptr;

            auto consensus_itr = directive_idx.lower_bound( boost::make_tuple( member.community, true ) );

            if( consensus_itr != directive_idx.end() )
            {
               consensus_directive_ptr = &(*consensus_itr);
            }

            const community_directive_object* delegate_directive_ptr = nullptr;
            
            auto delegate_vote_itr = member_vote_idx.lower_bound( boost::make_tuple( member.community, member.account ) );
            int32_t top_delegate_votes = 0;

            while( delegate_vote_itr != member_vote_idx.end() &&
               delegate_vote_itr->community == member.community &&
               delegate_vote_itr->voter == member.account )
            {
               const community_directive_member_vote_object& vote = *delegate_vote_itr;

               results.back().communities.outgoing_directive_member_votes[ member.community ].push_back( community_directive_member_vote_api_obj( vote ) );
               const community_directive_member_object& voted_member = _db.get_community_directive_member( vote.member, vote.community );

               if( voted_member.net_votes > top_delegate_votes && vote.approve )
               {
                  top_delegate_votes = voted_member.net_votes;
                  delegate_directive_ptr = _db.find_community_directive( voted_member.account, voted_member.delegate_directive_id );
               }

               ++delegate_vote_itr;
            }

            const community_directive_object* emergent_directive_ptr = nullptr;

            auto emergent_vote_itr = directive_vote_idx.lower_bound( boost::make_tuple( member.account, member.community ) );
            int32_t top_emergent_votes = 0;

            while( emergent_vote_itr != directive_vote_idx.end() && 
               emergent_vote_itr->voter == member.account &&
               emergent_vote_itr->community == member.community )
            {
               const community_directive_vote_object& vote = *emergent_vote_itr;

               results.back().communities.outgoing_directive_votes[ member.community ].push_back( community_directive_vote_api_obj( vote ) );
               const community_directive_object& emergent_directive = _db.get( vote.directive );

               if( emergent_directive.net_votes > top_emergent_votes && vote.approve )
               {
                  top_emergent_votes = emergent_directive.net_votes;
                  emergent_directive_ptr = &emergent_directive;
               }

               ++emergent_vote_itr;
            }

            if( command_directive_ptr != nullptr )
            {
               community_directive_state cdstate = community_directive_state( *command_directive_ptr );
               auto directive_approve_itr = directive_approve_idx.lower_bound( command_directive_ptr->id );

               while( directive_approve_itr != directive_approve_idx.end() && 
                  directive_approve_itr->directive == command_directive_ptr->id )
               {
                  cdstate.votes.push_back( community_directive_vote_api_obj( *directive_approve_itr ) );
                  ++directive_approve_itr;
               }

               results.back().communities.incoming_directives[ member.community ].push_back( cdstate );
            }

            if( delegate_directive_ptr != nullptr )
            {
               community_directive_state cdstate = community_directive_state( *delegate_directive_ptr );
               auto directive_approve_itr = directive_approve_idx.lower_bound( delegate_directive_ptr->id );

               while( directive_approve_itr != directive_approve_idx.end() && 
                  directive_approve_itr->directive == delegate_directive_ptr->id )
               {
                  cdstate.votes.push_back( community_directive_vote_api_obj( *directive_approve_itr ) );
                  ++directive_approve_itr;
               }

               results.back().communities.incoming_directives[ member.community ].push_back( cdstate );
            }

            if( consensus_directive_ptr != nullptr )
            {
               community_directive_state cdstate = community_directive_state( *consensus_directive_ptr );
               auto directive_approve_itr = directive_approve_idx.lower_bound( consensus_directive_ptr->id );

               while( directive_approve_itr != directive_approve_idx.end() && 
                  directive_approve_itr->directive == consensus_directive_ptr->id )
               {
                  cdstate.votes.push_back( community_directive_vote_api_obj( *directive_approve_itr ) );
                  ++directive_approve_itr;
               }

               results.back().communities.incoming_directives[ member.community ].push_back( cdstate );
            }

            if( emergent_directive_ptr != nullptr )
            {
               community_directive_state cdstate = community_directive_state( *emergent_directive_ptr );
               auto directive_approve_itr = directive_approve_idx.lower_bound( emergent_directive_ptr->id );

               while( directive_approve_itr != directive_approve_idx.end() && 
                  directive_approve_itr->directive == emergent_directive_ptr->id )
               {
                  cdstate.votes.push_back( community_directive_vote_api_obj( *directive_approve_itr ) );
                  ++directive_approve_itr;
               }

               results.back().communities.incoming_directives[ member.community ].push_back( cdstate );
            }

            auto directive_id_itr = directive_id_idx.lower_bound( member.account );

            while( directive_id_itr != directive_id_idx.end() && 
               directive_id_itr->account == member.account )
            {
               community_directive_state cdstate = community_directive_state( *directive_id_itr );
               auto directive_approve_itr = directive_approve_idx.lower_bound( directive_id_itr->id );

               while( directive_approve_itr != directive_approve_idx.end() && 
                  directive_approve_itr->directive == directive_id_itr->id )
               {
                  cdstate.votes.push_back( community_directive_vote_api_obj( *directive_approve_itr ) );
                  results.back().communities.incoming_directive_votes[ member.community ].push_back( community_directive_vote_api_obj( *directive_approve_itr ) );
                  ++directive_approve_itr;
               }

               results.back().communities.outgoing_directives[ member.community ].push_back( cdstate );

               ++directive_approve_itr;
            }

            ++directive_member_itr;
         }

         auto community_event_attend_itr = community_event_attend_idx.lower_bound( name );

         while( community_event_attend_itr != community_event_attend_idx.end() && 
            community_event_attend_itr->attendee == name )
         {
            const community_event_attend_object& attend = *community_event_attend_itr;
            auto community_event_itr = community_event_idx.find( boost::make_tuple( attend.community, attend.event_id ) );
            community_event_state estate = community_event_state( *community_event_itr );
            auto community_event_attend_id_itr = community_event_attend_id_idx.lower_bound( boost::make_tuple( attend.community, attend.event_id ) );

            while( community_event_attend_id_itr != community_event_attend_id_idx.end() && 
               community_event_attend_id_itr->community == attend.community &&
               community_event_attend_id_itr->event_id == attend.event_id )
            {
               estate.attend_objs.push_back( *community_event_attend_id_itr );
               ++community_event_attend_id_itr;
            }
         
            if( attend.interested )
            {
               if( attend.attending )
               {
                  results.back().communities.interested_attending[ attend.community ].push_back( estate );
               }
               else
               {
                  results.back().communities.interested_not_attending[ attend.community ].push_back( estate );
               }
            }
            else
            {
               if( attend.attending )
               {
                  results.back().communities.not_interested_attending[ attend.community ].push_back( estate );
               }
               else
               {
                  results.back().communities.not_interested_not_attending[ attend.community ].push_back( estate );
               }
            }
            
            ++community_event_attend_itr;
         }
         

         auto producer_itr = producer_idx.find( name );
         if( producer_itr != producer_idx.end() )
         {
            results.back().network.producer = producer_api_obj( *producer_itr );
         }

         auto creative_itr = creative_idx.lower_bound( name );
         while( creative_itr != creative_idx.end() && creative_itr->author == name )
         {
            results.back().ads.creatives.push_back( ad_creative_api_obj( *creative_itr ) );
            ++creative_itr;
         }

         auto campaign_itr = campaign_idx.lower_bound( name );
         while( campaign_itr != campaign_idx.end() && campaign_itr->account == name )
         {
            results.back().ads.campaigns.push_back( ad_campaign_api_obj( *campaign_itr ) );
            ++campaign_itr;
         }

         auto audience_itr = audience_idx.lower_bound( name );
         while( audience_itr != audience_idx.end() && audience_itr->account == name )
         {
            results.back().ads.audiences.push_back( ad_audience_api_obj( *audience_itr ) );
            ++audience_itr;
         }

         auto inventory_itr = inventory_idx.lower_bound( name );
         while( inventory_itr != inventory_idx.end() && inventory_itr->provider == name )
         {
            results.back().ads.inventories.push_back( ad_inventory_api_obj( *inventory_itr ) );
            ++inventory_itr;
         }

         auto bidder_bid_itr = bidder_bid_idx.lower_bound( name );
         while( bidder_bid_itr != bidder_bid_idx.end() && bidder_bid_itr->bidder == name )
         {
            results.back().ads.created_bids.push_back( ad_bid_api_obj( *bidder_bid_itr ) );
            ++bidder_bid_itr;
         }

         auto account_bid_itr = account_bid_idx.lower_bound( name );
         while( account_bid_itr != account_bid_idx.end() && account_bid_itr->account == name )
         {
            results.back().ads.account_bids.push_back( ad_bid_api_obj( *account_bid_itr ) );
            ++account_bid_itr;
         }

         auto author_bid_itr = author_bid_idx.lower_bound( name );
         while( author_bid_itr != author_bid_idx.end() && author_bid_itr->author == name )
         {
            results.back().ads.creative_bids.push_back( ad_bid_api_obj( *author_bid_itr ) );
            ++author_bid_itr;
         }

         auto provider_bid_itr = provider_bid_idx.lower_bound( name );
         while( provider_bid_itr != provider_bid_idx.end() && provider_bid_itr->provider == name )
         {
            results.back().ads.incoming_bids.push_back( ad_bid_state( *provider_bid_itr ) );

            auto cr_itr = creative_id_idx.find( boost::make_tuple( provider_bid_itr->author, provider_bid_itr->creative_id ) );
            if( cr_itr != creative_id_idx.end() )
            {
               results.back().ads.incoming_bids.back().creative = ad_creative_api_obj( *cr_itr );
            }

            auto c_itr = campaign_id_idx.find( boost::make_tuple( provider_bid_itr->account, provider_bid_itr->campaign_id ) );
            if( c_itr != campaign_id_idx.end() )
            {
               results.back().ads.incoming_bids.back().campaign = ad_campaign_api_obj( *c_itr );
            }

            auto i_itr = inventory_id_idx.find( boost::make_tuple( provider_bid_itr->provider, provider_bid_itr->inventory_id ) );
            if( i_itr != inventory_id_idx.end() )
            {
               results.back().ads.incoming_bids.back().inventory = ad_inventory_api_obj( *i_itr );
            }

            auto a_itr = audience_id_idx.find( boost::make_tuple( provider_bid_itr->bidder, provider_bid_itr->audience_id ) );
            if( a_itr != audience_id_idx.end() )
            {
               results.back().ads.incoming_bids.back().audience = ad_audience_api_obj( *a_itr );
            }

            ++provider_bid_itr;
         }

         auto product_itr = product_idx.lower_bound( name );
         while( product_itr != product_idx.end() && 
            product_itr->account == name )
         {
            results.back().products.seller_products.push_back( product_sale_api_obj( *product_itr ) );
            ++product_itr;
         }

         auto seller_purchase_itr = seller_purchase_idx.lower_bound( name );
         while( seller_purchase_itr != seller_purchase_idx.end() && 
            seller_purchase_itr->seller == name )
         {
            results.back().products.seller_orders.push_back( product_purchase_api_obj( *seller_purchase_itr ) );
            ++seller_purchase_itr;
         }

         auto buyer_purchase_itr = buyer_purchase_idx.lower_bound( name );
         while( buyer_purchase_itr != buyer_purchase_idx.end() && 
            buyer_purchase_itr->buyer == name )
         {
            results.back().products.buyer_orders.push_back( product_purchase_api_obj( *buyer_purchase_itr ) );
            ++buyer_purchase_itr;
         }

         flat_set< pair < account_name_type, string > > buyer_products;

         for( auto product : results.back().products.buyer_orders )
         {
            buyer_products.insert( std::make_pair( product.seller, product.product_id ) );
         }

         for( auto product : buyer_products )
         {
            product_itr = product_idx.find( boost::make_tuple( product.first, product.second ) );
            if( product_itr != product_idx.end() )
            {
               results.back().products.buyer_products.push_back( product_sale_api_obj( *product_itr ) );
            }
         }

         auto auction_itr = auction_idx.lower_bound( name );
         while( auction_itr != auction_idx.end() && 
            auction_itr->account == name )
         {
            results.back().products.seller_auctions.push_back( product_auction_sale_api_obj( *auction_itr ) );
            ++auction_itr;
         }

         auto seller_bid_itr = seller_bid_idx.lower_bound( name );
         while( seller_bid_itr != seller_bid_idx.end() && 
            seller_bid_itr->seller == name )
         {
            results.back().products.seller_bids.push_back( product_auction_bid_api_obj( *seller_bid_itr ) );
            ++seller_bid_itr;
         }

         auto buyer_bid_itr = buyer_bid_idx.lower_bound( name );
         while( buyer_bid_itr != buyer_bid_idx.end() && 
            buyer_bid_itr->buyer == name )
         {
            results.back().products.buyer_bids.push_back( product_auction_bid_api_obj( *buyer_bid_itr ) );
            ++buyer_bid_itr;
         }

         flat_set< pair < account_name_type, string > > buyer_auctions;

         for( auto bid : results.back().products.buyer_bids )
         {
            buyer_auctions.insert( std::make_pair( bid.seller, bid.bid_id ) );
         }

         for( auto auction : buyer_auctions )
         {
            auction_itr = auction_idx.find( boost::make_tuple( auction.first, auction.second ) );
            if( auction_itr != auction_idx.end() )
            {
               results.back().products.buyer_auctions.push_back( product_auction_sale_api_obj( *auction_itr ) );
            }
         }

         auto author_premium_purchase_itr = author_premium_purchase_idx.lower_bound( name );
         while( author_premium_purchase_itr != author_premium_purchase_idx.end() && 
            author_premium_purchase_itr->author == name )
         {
            results.back().premium.incoming_premium_purchases.push_back( premium_purchase_api_obj( *author_premium_purchase_itr ) );
            ++author_premium_purchase_itr;
         }

         auto account_premium_purchase_itr = account_premium_purchase_idx.lower_bound( name );
         while( account_premium_purchase_itr != account_premium_purchase_idx.end() && 
            account_premium_purchase_itr->account == name )
         {
            results.back().premium.outgoing_premium_purchases.push_back( premium_purchase_api_obj( *account_premium_purchase_itr ) );
            ++account_premium_purchase_itr;
         }

         auto author_premium_purchase_key_itr = author_premium_purchase_key_idx.lower_bound( name );
         while( author_premium_purchase_key_itr != author_premium_purchase_key_idx.end() && 
            author_premium_purchase_key_itr->author == name )
         {
            results.back().premium.incoming_premium_purchase_keys.push_back( premium_purchase_key_api_obj( *author_premium_purchase_key_itr ) );
            ++author_premium_purchase_key_itr;
         }

         auto account_premium_purchase_key_itr = account_premium_purchase_key_idx.lower_bound( name );
         while( account_premium_purchase_key_itr != account_premium_purchase_key_idx.end() && 
            account_premium_purchase_key_itr->account == name )
         {
            results.back().premium.outgoing_premium_purchase_keys.push_back( premium_purchase_key_api_obj( *account_premium_purchase_key_itr ) );
            ++account_premium_purchase_key_itr;
         }

         if( _db.has_index< tags::account_curation_metrics_index >() )
         {
            const auto& account_curation_metrics_idx = _db.get_index< tags::account_curation_metrics_index >().indices().get< tags::by_account >();
            auto account_curation_metrics_itr = account_curation_metrics_idx.find( name );

            if( account_curation_metrics_itr != account_curation_metrics_idx.end() )
            {
               results.back().curation_metrics = account_curation_metrics_api_obj( *account_curation_metrics_itr );
            }
         }

         if( results.back().pinned_permlink.size() )
         {
            results.back().pinned_post = get_content( name, results.back().pinned_permlink );
         }

         auto comment_blog_itr = comment_blog_idx.lower_bound( name );
         if( comment_blog_itr != comment_blog_idx.end() && comment_blog_itr->account == name )
         {
            results.back().latest_post = get_discussion( comment_blog_itr->comment, 0 );
            results.back().latest_post.blog = comment_blog_api_obj( *comment_blog_itr );
         }

         auto history_itr = history_idx.lower_bound( name );
   
         map< uint32_t, applied_operation> operation_history;
         
         while( history_itr != history_idx.end() && history_itr->account == name )
         {
            operation_history[ history_itr->sequence ] = _db.get( history_itr->op );
            ++history_itr;
         }

         auto owner_history_itr = owner_history_idx.lower_bound( name );

         while( owner_history_itr != owner_history_idx.end() && 
            owner_history_itr->account == name )
         {
            results.back().owner_history.push_back( owner_authority_history_api_obj( *owner_history_itr ) );
            ++owner_history_itr;
         }

         auto recovery_itr = recovery_idx.find( name );

         if( recovery_itr != recovery_idx.end() )
         {
            results.back().recovery = account_recovery_request_api_obj( *recovery_itr );
         }

         for( auto& item : operation_history )
         {
            switch( item.second.op.which() )
            {
               case operation::tag< account_create_operation  >::value:
               case operation::tag< account_update_operation >::value:
               case operation::tag< account_verification_operation >::value:
               case operation::tag< account_membership_operation >::value:
               case operation::tag< account_update_list_operation >::value:
               case operation::tag< account_producer_vote_operation >::value:
               case operation::tag< account_update_proxy_operation >::value:
               case operation::tag< account_request_recovery_operation >::value:
               case operation::tag< account_recover_operation >::value:
               case operation::tag< account_reset_operation >::value:
               case operation::tag< account_reset_update_operation >::value:
               case operation::tag< account_recovery_update_operation >::value:
               case operation::tag< account_decline_voting_operation >::value:
               {
                  results.back().operations.account_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< account_connection_operation >::value:
               {
                  results.back().operations.connection_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< account_follow_operation >::value:
               case operation::tag< account_follow_tag_operation >::value:
               {
                  results.back().operations.follow_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< account_activity_operation >::value:
               {
                  results.back().operations.activity_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< business_create_operation >::value:
               case operation::tag< business_update_operation >::value:
               case operation::tag< business_executive_operation >::value:
               case operation::tag< business_executive_vote_operation >::value:
               case operation::tag< business_director_operation >::value:
               case operation::tag< business_director_vote_operation >::value:
               {
                  results.back().operations.business_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< governance_create_operation >::value:
               case operation::tag< governance_update_operation >::value:
               case operation::tag< governance_executive_operation >::value:
               case operation::tag< governance_executive_vote_operation >::value:
               case operation::tag< governance_director_operation >::value:
               case operation::tag< governance_director_vote_operation >::value:
               case operation::tag< governance_member_operation >::value:
               case operation::tag< governance_member_request_operation >::value:
               case operation::tag< governance_resolution_operation >::value:
               case operation::tag< governance_resolution_vote_operation >::value:
               {
                  results.back().operations.governance_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< network_officer_update_operation >::value:
               case operation::tag< network_officer_vote_operation >::value:
               case operation::tag< supernode_update_operation >::value:
               case operation::tag< supernode_reward_operation >::value:
               case operation::tag< interface_update_operation >::value:
               case operation::tag< mediator_update_operation >::value:
               case operation::tag< enterprise_update_operation >::value:
               case operation::tag< enterprise_vote_operation >::value:
               case operation::tag< enterprise_fund_operation >::value:
               {
                  results.back().operations.network_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< comment_operation >::value:
               case operation::tag< author_reward_operation >::value:
               case operation::tag< content_reward_operation >::value:
               case operation::tag< comment_reward_operation >::value:
               case operation::tag< comment_benefactor_reward_operation >::value:
               case operation::tag< list_operation >::value:
               case operation::tag< poll_operation >::value:
               case operation::tag< poll_vote_operation >::value:
               {
                  results.back().operations.comment_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< premium_purchase_operation >::value:
               case operation::tag< premium_release_operation >::value:
               {
                  results.back().operations.premium_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< comment_vote_operation >::value:
               case operation::tag< vote_reward_operation >::value:
               {
                  results.back().operations.vote_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< comment_view_operation >::value:
               case operation::tag< view_reward_operation >::value:
               {
                  results.back().operations.view_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< comment_share_operation >::value:
               case operation::tag< share_reward_operation >::value:
               {
                  results.back().operations.share_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< comment_moderation_operation >::value:
               case operation::tag< moderation_reward_operation >::value:
               {
                  results.back().operations.moderation_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< message_operation >::value:
               {
                  results.back().operations.message_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< community_create_operation >::value:
               case operation::tag< community_update_operation >::value:
               case operation::tag< community_member_operation >::value:
               case operation::tag< community_member_request_operation >::value:
               case operation::tag< community_member_vote_operation >::value:
               case operation::tag< community_subscribe_operation >::value:
               case operation::tag< community_blacklist_operation >::value:
               case operation::tag< community_federation_operation >::value:
               case operation::tag< community_event_operation >::value:
               case operation::tag< community_event_attend_operation >::value:
               case operation::tag< community_directive_operation >::value:
               case operation::tag< community_directive_vote_operation >::value:
               case operation::tag< community_directive_member_operation >::value:
               case operation::tag< community_directive_member_vote_operation >::value:
               {
                  results.back().operations.community_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< ad_creative_operation >::value:
               case operation::tag< ad_campaign_operation >::value:
               case operation::tag< ad_inventory_operation >::value:
               case operation::tag< ad_audience_operation >::value:
               case operation::tag< ad_bid_operation >::value:
               {
                  results.back().operations.ad_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< graph_node_operation >::value:
               case operation::tag< graph_edge_operation >::value:
               case operation::tag< graph_node_property_operation >::value:
               case operation::tag< graph_edge_property_operation >::value:
               {
                  results.back().operations.graph_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< transfer_operation >::value:
               case operation::tag< transfer_request_operation >::value:
               case operation::tag< transfer_accept_operation >::value:
               case operation::tag< transfer_recurring_operation >::value:
               case operation::tag< transfer_recurring_request_operation >::value:
               case operation::tag< transfer_recurring_accept_operation >::value:
               case operation::tag< transfer_confidential_operation >::value:
               case operation::tag< transfer_to_confidential_operation >::value:
               case operation::tag< transfer_from_confidential_operation >::value:
               {
                  results.back().operations.transfer_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< claim_reward_balance_operation >::value:
               case operation::tag< stake_asset_operation >::value:
               case operation::tag< unstake_asset_operation >::value:
               case operation::tag< transfer_to_savings_operation >::value:
               case operation::tag< transfer_from_savings_operation >::value:
               case operation::tag< delegate_asset_operation >::value:
               {
                  results.back().operations.balance_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< product_sale_operation >::value:
               case operation::tag< product_purchase_operation >::value:
               case operation::tag< product_auction_sale_operation >::value:
               case operation::tag< product_auction_bid_operation >::value:
               {
                  results.back().operations.product_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< escrow_transfer_operation >::value:
               case operation::tag< escrow_approve_operation >::value:
               case operation::tag< escrow_dispute_operation >::value:
               case operation::tag< escrow_release_operation >::value:
               {
                  results.back().operations.escrow_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< limit_order_operation >::value:
               case operation::tag< margin_order_operation >::value:
               case operation::tag< auction_order_operation >::value:
               case operation::tag< call_order_operation >::value:
               case operation::tag< fill_order_operation >::value:
               {
                  results.back().operations.trading_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< liquidity_pool_create_operation >::value:
               case operation::tag< liquidity_pool_exchange_operation >::value:
               case operation::tag< liquidity_pool_fund_operation >::value:
               case operation::tag< liquidity_pool_withdraw_operation >::value:
               {
                  results.back().operations.liquidity_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< credit_pool_collateral_operation >::value:
               case operation::tag< credit_pool_borrow_operation >::value:
               case operation::tag< credit_pool_lend_operation >::value:
               case operation::tag< credit_pool_withdraw_operation >::value:
               {
                  results.back().operations.credit_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< option_order_operation >::value:
               case operation::tag< option_pool_create_operation >::value:
               case operation::tag< asset_option_exercise_operation >::value:
               {
                  results.back().operations.option_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< prediction_pool_create_operation >::value:
               case operation::tag< prediction_pool_exchange_operation >::value:
               case operation::tag< prediction_pool_resolve_operation >::value:
               {
                  results.back().operations.prediction_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< asset_create_operation >::value:
               case operation::tag< asset_update_operation >::value:
               case operation::tag< asset_issue_operation >::value:
               case operation::tag< asset_reserve_operation >::value:
               case operation::tag< asset_update_issuer_operation >::value:
               case operation::tag< asset_distribution_operation >::value:
               case operation::tag< asset_distribution_fund_operation >::value:
               case operation::tag< asset_stimulus_fund_operation >::value:
               case operation::tag< asset_update_feed_producers_operation >::value:
               case operation::tag< asset_publish_feed_operation >::value:
               case operation::tag< asset_settle_operation >::value:
               case operation::tag< asset_global_settle_operation >::value:
               case operation::tag< asset_collateral_bid_operation >::value:
               {
                  results.back().operations.asset_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< producer_update_operation >::value:
               case operation::tag< proof_of_work_operation >::value:
               case operation::tag< producer_reward_operation >::value:
               case operation::tag< verify_block_operation >::value:
               case operation::tag< commit_block_operation >::value:
               case operation::tag< producer_violation_operation >::value:
               {
                  results.back().operations.producer_history[ item.first ] = item.second;
               }
               break;
               case operation::tag< custom_operation >::value:
               case operation::tag< custom_json_operation >::value:
               default:
                  results.back().operations.other_history[ item.first ] = item.second;
            }
         }
      }
   }
   return results;
}

map< uint32_t, applied_operation > database_api::get_account_history( string account, uint64_t from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_history( account, from, limit );
   });
}

map< uint32_t, applied_operation > database_api_impl::get_account_history( string account, uint64_t from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 10000 ) );
   FC_ASSERT( from >= limit,
      "From must be greater than limit." );

   const auto& history_idx = _db.get_index< account_history_index >().indices().get< by_account >();
   auto history_itr = history_idx.lower_bound( boost::make_tuple( account, from ) );

   uint32_t n = 0;
   map< uint32_t, applied_operation > results;
   
   while( true )
   {
      if( history_itr == history_idx.end() )
      {
         break;
      }
      if( history_itr->account != account )
      {
         break;
      }
      if( n >= limit )
      {
         break;
      }
      results[ history_itr->sequence ] = _db.get( history_itr->op );
      ++history_itr;
      ++n;
   }
   return results;
}

vector< account_balance_state > database_api::get_account_balances( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_balances( names );
   });
}

vector< account_balance_state > database_api_impl::get_account_balances( vector< string > names )const
{
   const auto& balance_idx = _db.get_index< account_balance_index >().indices().get< by_owner >();
   const auto& withdraw_route_idx = _db.get_index< unstake_asset_route_index >().indices().get< by_withdraw_route >();
   const auto& destination_route_idx = _db.get_index< unstake_asset_route_index >().indices().get< by_destination >();
   const auto& savings_withdrawals_from_idx = _db.get_index< savings_withdraw_index >().indices().get< by_request_id >();
   const auto& savings_withdrawals_to_idx = _db.get_index< savings_withdraw_index >().indices().get< by_to_complete >();
   const auto& delegation_from_idx = _db.get_index< asset_delegation_index >().indices().get< by_delegator >();
   const auto& delegation_to_idx = _db.get_index< asset_delegation_index >().indices().get< by_delegatee >();
   const auto& expiration_from_idx = _db.get_index< asset_delegation_expiration_index >().indices().get< by_delegator >();
   const auto& expiration_to_idx = _db.get_index< asset_delegation_expiration_index >().indices().get< by_delegatee >();
   
   vector< account_balance_state > results;

   for( auto name: names )
   {
      account_balance_state bstate;

      auto balance_itr = balance_idx.lower_bound( name );
      while( balance_itr != balance_idx.end() && balance_itr->owner == name )
      {
         bstate.balances[ balance_itr->symbol ] = account_balance_api_obj( *balance_itr );
      }

      const account_object& acc = _db.get_account( name );

      auto withdraw_route_itr = withdraw_route_idx.lower_bound( acc.name );
      while( withdraw_route_itr != withdraw_route_idx.end() && 
         withdraw_route_itr->from == acc.name )
      {
         withdraw_route r;
         r.from = withdraw_route_itr->from;
         r.to = withdraw_route_itr->to;
         r.percent = withdraw_route_itr->percent;
         r.auto_stake = withdraw_route_itr->auto_stake;
         bstate.withdraw_routes.push_back( r );
         ++withdraw_route_itr;
      }

      auto destination_route_itr = destination_route_idx.lower_bound( acc.name );
      while( destination_route_itr != destination_route_idx.end() && 
         destination_route_itr->to == acc.name )
      {
         withdraw_route r;
         r.from = destination_route_itr->from;
         r.to = destination_route_itr->to;
         r.percent = destination_route_itr->percent;
         r.auto_stake = destination_route_itr->auto_stake;
         bstate.withdraw_routes.push_back( r );
         ++destination_route_itr;
      }

      auto savings_withdrawals_from_itr = savings_withdrawals_from_idx.lower_bound( name );
      while( savings_withdrawals_from_itr != savings_withdrawals_from_idx.end() && 
         savings_withdrawals_from_itr->from == name )
      {
         bstate.savings_withdrawals_from.push_back( savings_withdraw_api_obj( *savings_withdrawals_from_itr ) );
         ++savings_withdrawals_from_itr;
      }

      auto savings_withdrawals_to_itr = savings_withdrawals_to_idx.lower_bound( name );
      while( savings_withdrawals_to_itr != savings_withdrawals_to_idx.end() && 
         savings_withdrawals_to_itr->to == name ) 
      {
         bstate.savings_withdrawals_to.push_back( savings_withdraw_api_obj( *savings_withdrawals_to_itr ) );
         ++savings_withdrawals_to_itr;
      }

      auto delegation_from_itr = delegation_from_idx.lower_bound( name );
      while( delegation_from_itr != delegation_from_idx.end() && 
         delegation_from_itr->delegator == name )
      {
         bstate.delegations_from.push_back( *delegation_from_itr );
         ++delegation_from_itr;
      }

      auto delegation_to_itr = delegation_to_idx.lower_bound( name );
      while( delegation_to_itr != delegation_to_idx.end() && 
         delegation_to_itr->delegatee == name )
      {
         bstate.delegations_to.push_back( *delegation_to_itr );
         ++delegation_to_itr;
      }

      auto expiration_from_itr = expiration_from_idx.lower_bound( name );
      while( expiration_from_itr != expiration_from_idx.end() && 
         expiration_from_itr->delegator == name )
      {
         bstate.expirations_from.push_back( *expiration_from_itr );
         ++expiration_from_itr;
      }

      auto expiration_to_itr = expiration_to_idx.lower_bound( name );
      while( expiration_to_itr != expiration_to_idx.end() && 
         expiration_to_itr->delegator == name )
      {
         bstate.expirations_to.push_back( *expiration_to_itr );
         ++expiration_to_itr;
      }

      results.push_back( bstate );
   }

   return results;
}


vector< confidential_balance_api_obj > database_api::get_confidential_balances( const confidential_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_confidential_balances( query );
   });
}

vector< confidential_balance_api_obj > database_api_impl::get_confidential_balances( const confidential_query& query )const
{
   const auto& commit_idx = _db.get_index< confidential_balance_index >().indices().get< by_commitment >();
   const auto& key_idx = _db.get_index< confidential_balance_index >().indices().get< by_key_auth >();
   const auto& account_idx = _db.get_index< confidential_balance_index >().indices().get< by_account_auth >();

   vector< confidential_balance_api_obj > results;

   for( auto commit: query.select_commitments )
   {
      auto commit_itr = commit_idx.lower_bound( commit );
      while( commit_itr!= commit_idx.end() &&
         commit_itr->commitment == commit )
      {
         results.push_back( confidential_balance_api_obj( *commit_itr ) );
         ++commit_itr;
      }
   }

   for( auto key: query.select_key_auths )
   {
      auto key_itr = key_idx.lower_bound( public_key_type( key ) );
      while( key_itr!= key_idx.end() &&
         key_itr->key_auth() == public_key_type( key ) )
      {
         results.push_back( confidential_balance_api_obj( *key_itr ) );
         ++key_itr;
      }
   }

   for( auto acc: query.select_account_auths )
   {
      auto account_itr = account_idx.lower_bound( acc );
      while( account_itr!= account_idx.end() &&
         account_itr->account_auth() == acc )
      {
         results.push_back( confidential_balance_api_obj( *account_itr ) );
         ++account_itr;
      }
   }

   return results;
}

vector< account_message_state > database_api::get_account_messages( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_messages( names );
   });
}

vector< account_message_state > database_api_impl::get_account_messages( vector< string > names )const
{
   const auto& inbox_idx = _db.get_index< message_index >().indices().get< by_account_inbox >();
   const auto& outbox_idx = _db.get_index< message_index >().indices().get< by_account_outbox >();
   const auto& community_inbox_idx = _db.get_index< message_index >().indices().get< by_community_inbox >();
   const auto& following_idx = _db.get_index< account_following_index >().indices().get< by_account >();

   vector< account_message_state > results;

   for( auto name: names )
   {
      auto inbox_itr = inbox_idx.lower_bound( name );
      auto outbox_itr = outbox_idx.lower_bound( name );
      vector< message_api_obj > inbox;
      vector< message_api_obj > outbox;

      map< account_name_type, vector< message_api_obj > > account_conversations;
      map< community_name_type, vector< message_api_obj > > community_conversations;

      while( inbox_itr != inbox_idx.end() && inbox_itr->recipient == name )
      {
         inbox.push_back( message_api_obj( *inbox_itr ) );
      }

      while( outbox_itr != outbox_idx.end() && outbox_itr->sender == name )
      {
         outbox.push_back( message_api_obj( *outbox_itr ) );
      }

      for( auto message : inbox )
      {
         account_conversations[ message.sender ].push_back( message );
      }

      for( auto message : outbox )
      {
         account_conversations[ message.recipient ].push_back( message );
      }

      auto following_itr = following_idx.find( name );

      const account_following_object& account_following = *following_itr;

      for( auto community : account_following.member_communities )
      {
         auto community_inbox_itr = community_inbox_idx.lower_bound( community );
         while( community_inbox_itr != community_inbox_idx.end() && 
            community_inbox_itr->community == community )
         {
            community_conversations[ community ].push_back( message_api_obj( *community_inbox_itr ) );
            ++community_inbox_itr;
         }
      }

      for( auto conv : account_conversations )
      {
         vector< message_api_obj > thread = conv.second;
         std::sort( thread.begin(), thread.end(), [&]( message_api_obj a, message_api_obj b )
         {
            return a.created < b.created;
         });
         account_conversations[ conv.first ] = thread;
      }

      for( auto conv : community_conversations )
      {
         vector< message_api_obj > thread = conv.second;
         std::sort( thread.begin(), thread.end(), [&]( message_api_obj a, message_api_obj b )
         {
            return a.created < b.created;
         });
         community_conversations[ conv.first ] = thread;
      }

      account_message_state mstate;

      mstate.account_conversations = account_conversations;
      mstate.community_conversations = community_conversations;
      results.push_back( mstate );
   }

   return results;
}


list_state database_api::get_list( string name, string list_id )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_list( name, list_id );
   });
}


list_state database_api_impl::get_list( string name, string list_id )const
{
   const auto& list_idx = _db.get_index< list_index >().indices().get< by_list_id >();

   list_state lstate;
   
   auto list_itr = list_idx.find( boost::make_tuple( name, list_id ) );

   if( list_itr != list_idx.end() )
   {
      const list_object& list = *list_itr;

      lstate = list_state( list );
      
      for( account_id_type id : list.accounts )
      {
         lstate.account_objs.push_back( account_api_obj( _db.get( id ), _db ) );
      }
      for( comment_id_type id : list.comments )
      {
         lstate.comment_objs.push_back( comment_api_obj( _db.get( id ) ) );
      }
      for( community_id_type id : list.communities )
      {
         lstate.community_objs.push_back( community_api_obj( _db.get( id ) ) );
      }
      for( asset_id_type id : list.assets )
      {
         lstate.asset_objs.push_back( asset_api_obj( _db.get( id ) ) );
      }
      for( product_sale_id_type id : list.products )
      {
         lstate.product_objs.push_back( product_sale_api_obj( _db.get( id ) ) );
      }
      for( product_auction_sale_id_type id : list.auctions )
      {
         lstate.auction_objs.push_back( product_auction_sale_api_obj( _db.get( id ) ) );
      }
      for( graph_node_id_type id : list.nodes )
      {
         lstate.node_objs.push_back( graph_node_api_obj( _db.get( id ) ) );
      }
      for( graph_edge_id_type id : list.edges )
      {
         lstate.edge_objs.push_back( graph_edge_api_obj( _db.get( id ) ) );
      }
      for( graph_node_property_id_type id : list.node_types )
      {
         lstate.node_type_objs.push_back( graph_node_property_api_obj( _db.get( id ) ) );
      }
      for( graph_edge_property_id_type id : list.edge_types )
      {
         lstate.edge_type_objs.push_back( graph_edge_property_api_obj( _db.get( id ) ) );
      }
   }

   return lstate;
}


vector< account_list_state > database_api::get_account_lists( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_lists( names );
   });
}


vector< account_list_state > database_api_impl::get_account_lists( vector< string > names )const
{
   const auto& list_idx = _db.get_index< list_index >().indices().get< by_list_id >();
   auto list_itr = list_idx.begin();
   
   vector< account_list_state > results;

   for( auto name : names )
   {
      list_itr = list_idx.lower_bound( name );

      account_list_state account_lstate;

      while( list_itr != list_idx.end() &&
         list_itr->creator == name )
      {
         const list_object& list = *list_itr;

         list_state lstate = list_state( list );

         for( account_id_type id : list.accounts )
         {
            lstate.account_objs.push_back( account_api_obj( _db.get( id ), _db ) );
         }
         for( comment_id_type id : list.comments )
         {
            lstate.comment_objs.push_back( comment_api_obj( _db.get( id ) ) );
         }
         for( community_id_type id : list.communities )
         {
            lstate.community_objs.push_back( community_api_obj( _db.get( id ) ) );
         }
         for( asset_id_type id : list.assets )
         {
            lstate.asset_objs.push_back( asset_api_obj( _db.get( id ) ) );
         }
         for( product_sale_id_type id : list.products )
         {
            lstate.product_objs.push_back( product_sale_api_obj( _db.get( id ) ) );
         }
         for( product_auction_sale_id_type id : list.auctions )
         {
            lstate.auction_objs.push_back( product_auction_sale_api_obj( _db.get( id ) ) );
         }
         for( graph_node_id_type id : list.nodes )
         {
            lstate.node_objs.push_back( graph_node_api_obj( _db.get( id ) ) );
         }
         for( graph_edge_id_type id : list.edges )
         {
            lstate.edge_objs.push_back( graph_edge_api_obj( _db.get( id ) ) );
         }
         for( graph_node_property_id_type id : list.node_types )
         {
            lstate.node_type_objs.push_back( graph_node_property_api_obj( _db.get( id ) ) );
         }
         for( graph_edge_property_id_type id : list.edge_types )
         {
            lstate.edge_type_objs.push_back( graph_edge_property_api_obj( _db.get( id ) ) );
         }

         account_lstate.lists.push_back( lstate );

         ++list_itr;
      }

      results.push_back( account_lstate );
   }

   return results;
}


poll_state database_api::get_poll( string name, string poll_id )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_poll( name, poll_id );
   });
}


poll_state database_api_impl::get_poll( string name, string poll_id )const
{
   const auto& poll_idx = _db.get_index< poll_index >().indices().get< by_poll_id >();
   const auto& vote_idx = _db.get_index< poll_vote_index >().indices().get< by_poll_id >();
   
   poll_state pstate;
   
   auto poll_itr = poll_idx.find( boost::make_tuple( name, poll_id ) );
   auto vote_itr = vote_idx.begin();

   if( poll_itr != poll_idx.end() )
   {
      const poll_object& poll = *poll_itr;
      pstate = poll_state( poll );
      pstate.vote_count.reserve( 10 );

      for( auto i = 0; i < 10; i++ )
      {
         pstate.vote_count[ i ] = 0;
      }

      vote_itr = vote_idx.lower_bound( boost::make_tuple( name, poll_id ) );

      while( vote_itr != vote_idx.end() &&
         vote_itr->creator == name &&
         to_string( vote_itr->poll_id ) == poll_id )
      {
         pstate.vote_count[ vote_itr->poll_option ]++;
         pstate.votes[ vote_itr->voter ] = poll_vote_api_obj( *vote_itr );
         ++vote_itr;
      }
   }

   return pstate;
}


vector< account_poll_state > database_api::get_account_polls( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_polls( names );
   });
}


vector< account_poll_state > database_api_impl::get_account_polls( vector< string > names )const
{
   const auto& poll_idx = _db.get_index< poll_index >().indices().get< by_poll_id >();
   const auto& vote_idx = _db.get_index< poll_vote_index >().indices().get< by_poll_id >();
   
   vector< account_poll_state > results;

   for( auto name : names )
   {
      auto poll_itr = poll_idx.lower_bound( name );

      account_poll_state account_pstate;

      while( poll_itr != poll_idx.end() &&
         poll_itr->creator == name )
      {
         const poll_object& poll = *poll_itr;
         poll_state pstate = poll_state( poll );

         auto vote_itr = vote_idx.lower_bound( boost::make_tuple( name, poll.poll_id ) );

         while( vote_itr != vote_idx.end() &&
            vote_itr->creator == name &&
            vote_itr->poll_id == poll.poll_id )
         {
            pstate.vote_count[ vote_itr->poll_option ]++;
            pstate.votes[ vote_itr->voter ] = poll_vote_api_obj( *vote_itr );
            ++vote_itr;
         }

         account_pstate.polls[ poll_itr->community ].push_back( pstate );

         ++poll_itr;
      }

      results.push_back( account_pstate );
   }

   return results;
}


vector< key_state > database_api::get_keychains( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_keychains( names );
   });
}


vector< key_state > database_api_impl::get_keychains( vector< string > names )const
{
   const auto& connection_a_idx = _db.get_index< account_connection_index >().indices().get< by_account_a >();
   const auto& connection_b_idx = _db.get_index< account_connection_index >().indices().get< by_account_b >();
   const auto& community_member_idx = _db.get_index< community_member_index >().indices().get< by_member_community_type >();

   vector< key_state > results;

   for( auto name : names )
   {
      key_state kstate;

      auto connection_a_itr = connection_a_idx.lower_bound( boost::make_tuple( name, connection_tier_type::CONNECTION ) );
      auto connection_b_itr = connection_b_idx.lower_bound( boost::make_tuple( name, connection_tier_type::CONNECTION ) );
      while( connection_a_itr != connection_a_idx.end() && 
         connection_a_itr->account_a == name &&
         connection_a_itr->connection_type == connection_tier_type::CONNECTION )
      {
         kstate.connection_keys[ connection_a_itr->account_b ] = connection_a_itr->encrypted_key_b;
         ++connection_a_itr;
      }
      while( connection_b_itr != connection_b_idx.end() && 
         connection_b_itr->account_b == name &&
         connection_b_itr->connection_type == connection_tier_type::CONNECTION )
      {
         kstate.connection_keys[ connection_b_itr->account_a ] = connection_b_itr->encrypted_key_a;
         ++connection_b_itr;
      }

      connection_a_itr = connection_a_idx.lower_bound( boost::make_tuple( name, connection_tier_type::FRIEND ) );
      connection_b_itr = connection_b_idx.lower_bound( boost::make_tuple( name, connection_tier_type::FRIEND ) );
      while( connection_a_itr != connection_a_idx.end() && 
         connection_a_itr->account_a == name &&
         connection_a_itr->connection_type == connection_tier_type::FRIEND )
      {
         kstate.friend_keys[ connection_a_itr->account_b ] = connection_a_itr->encrypted_key_b;
         ++connection_a_itr;
      }
      while( connection_b_itr != connection_b_idx.end() && 
         connection_b_itr->account_b == name &&
         connection_b_itr->connection_type == connection_tier_type::FRIEND )
      {
         kstate.friend_keys[ connection_b_itr->account_a ] = connection_b_itr->encrypted_key_a;
         ++connection_b_itr;
      }

      connection_a_itr = connection_a_idx.lower_bound( boost::make_tuple( name, connection_tier_type::COMPANION ) );
      connection_b_itr = connection_b_idx.lower_bound( boost::make_tuple( name, connection_tier_type::COMPANION ) );
      while( connection_a_itr != connection_a_idx.end() && 
         connection_a_itr->account_a == name &&
         connection_a_itr->connection_type == connection_tier_type::COMPANION )
      {
         kstate.companion_keys[ connection_a_itr->account_b ] = connection_a_itr->encrypted_key_b;
         ++connection_a_itr;
      }
      while( connection_b_itr != connection_b_idx.end() && 
         connection_b_itr->account_b == name &&
         connection_b_itr->connection_type == connection_tier_type::COMPANION )
      {
         kstate.companion_keys[ connection_b_itr->account_a ] = connection_b_itr->encrypted_key_a;
         ++connection_b_itr;
      }

      auto community_member_itr = community_member_idx.lower_bound( name );
      while( community_member_itr != community_member_idx.end() && 
         community_member_itr->member == name )
      {
         switch( community_member_itr->member_type )
         {
            case community_permission_type::MEMBER_PERMISSION:
            {
               kstate.community_member_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
            }
            break;
            case community_permission_type::STANDARD_PREMIUM_PERMISSION:
            {
               kstate.community_standard_premium_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
            }
            break;
            case community_permission_type::MID_PREMIUM_PERMISSION:
            {
               kstate.community_mid_premium_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
            }
            break;
            case community_permission_type::TOP_PREMIUM_PERMISSION:
            {
               kstate.community_top_premium_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
            }
            break;
            case community_permission_type::MODERATOR_PERMISSION:
            {
               kstate.community_moderator_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
            }
            break;
            case community_permission_type::ADMIN_PERMISSION:
            {
               kstate.community_admin_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
            }
            break;
            case community_permission_type::FOUNDER_PERMISSION:
            {
               kstate.community_secure_keys[ community_member_itr->community ] = community_member_itr->encrypted_community_key;
            }
            break;
            default:
            {
               FC_ASSERT( false, "Invalid Federation type" );
            }
         }
         
         ++community_member_itr;
      }

      results.push_back( kstate );
   }
   return results;
}

set< string > database_api::lookup_accounts( string lower_bound_name, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->lookup_accounts( lower_bound_name, limit );
   });
}

set< string > database_api_impl::lookup_accounts( string lower_bound_name, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   const auto& accounts_by_name = _db.get_index< account_index >().indices().get< by_name >();
   set< string > results;

   for( auto acc_itr = accounts_by_name.lower_bound( lower_bound_name );
      limit-- && acc_itr != accounts_by_name.end();
      ++acc_itr )
   {
      results.insert( acc_itr->name );
   }

   return results;
}

uint64_t database_api::get_account_count()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_count();
   });
}

uint64_t database_api_impl::get_account_count()const
{
   return _db.get_index< account_index >().indices().size();
}

} } // node::app