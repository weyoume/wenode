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


   //================//
   // === Ad API === //
   //================//


vector< account_ad_state > database_api::get_account_ads( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_ads( names );
   });
}

vector< account_ad_state > database_api_impl::get_account_ads( vector< string > names )const
{
   vector< account_ad_state > results;
   results.reserve( names.size() );

   const auto& creative_idx = _db.get_index< ad_creative_index >().indices().get< by_latest >();
   const auto& campaign_idx = _db.get_index< ad_campaign_index >().indices().get< by_latest >();
   const auto& audience_idx = _db.get_index< ad_audience_index >().indices().get< by_latest >();
   const auto& inventory_idx = _db.get_index< ad_inventory_index >().indices().get< by_latest >();

   const auto& creative_id_idx = _db.get_index< ad_creative_index >().indices().get< by_creative_id >();
   const auto& campaign_id_idx = _db.get_index< ad_campaign_index >().indices().get< by_campaign_id >();
   const auto& audience_id_idx = _db.get_index< ad_audience_index >().indices().get< by_audience_id >();
   const auto& inventory_id_idx = _db.get_index< ad_inventory_index >().indices().get< by_inventory_id >();

   const auto& bidder_idx = _db.get_index< ad_bid_index >().indices().get< by_bidder_updated >();
   const auto& account_idx = _db.get_index< ad_bid_index >().indices().get< by_account_updated >();
   const auto& author_idx = _db.get_index< ad_bid_index >().indices().get< by_author_updated >();
   const auto& provider_idx = _db.get_index< ad_bid_index >().indices().get< by_provider_updated >();

   for( auto name : names )
   {
      account_ad_state astate;

      auto creative_itr = creative_idx.lower_bound( name );
      while( creative_itr != creative_idx.end() && creative_itr->author == name )
      {
         astate.creatives.push_back( ad_creative_api_obj( *creative_itr ) );
         ++creative_itr;
      }

      auto campaign_itr = campaign_idx.lower_bound( name );
      while( campaign_itr != campaign_idx.end() && campaign_itr->account == name )
      {
         astate.campaigns.push_back( ad_campaign_api_obj( *campaign_itr ) );
         ++campaign_itr;
      }

      auto audience_itr = audience_idx.lower_bound( name );
      while( audience_itr != audience_idx.end() && audience_itr->account == name )
      {
         astate.audiences.push_back( ad_audience_api_obj( *audience_itr ) );
         ++audience_itr;
      }

      auto inventory_itr = inventory_idx.lower_bound( name );
      while( inventory_itr != inventory_idx.end() && inventory_itr->provider == name )
      {
         astate.inventories.push_back( ad_inventory_api_obj( *inventory_itr ) );
         ++inventory_itr;
      }

      auto bidder_itr = bidder_idx.lower_bound( name );
      while( bidder_itr != bidder_idx.end() && bidder_itr->bidder == name )
      {
         astate.created_bids.push_back( ad_bid_api_obj( *bidder_itr ) );
         ++bidder_itr;
      }

      auto account_itr = account_idx.lower_bound( name );
      while( account_itr != account_idx.end() && account_itr->account == name )
      {
         astate.account_bids.push_back( ad_bid_api_obj( *account_itr ) );
         ++account_itr;
      }

      auto author_itr = author_idx.lower_bound( name );
      while( author_itr != author_idx.end() && author_itr->author == name )
      {
         astate.creative_bids.push_back( ad_bid_api_obj( *author_itr ) );
         ++author_itr;
      }

      auto provider_itr = provider_idx.lower_bound( name );
      while( provider_itr != provider_idx.end() && provider_itr->provider == name )
      {
         astate.incoming_bids.push_back( ad_bid_state( *provider_itr ) );

         auto cr_itr = creative_id_idx.find( boost::make_tuple( provider_itr->author, provider_itr->creative_id ) );
         if( cr_itr != creative_id_idx.end() )
         {
            astate.incoming_bids.back().creative = ad_creative_api_obj( *cr_itr );
         }

         auto c_itr = campaign_id_idx.find( boost::make_tuple( provider_itr->account, provider_itr->campaign_id ) );
         if( c_itr != campaign_id_idx.end() )
         {
            astate.incoming_bids.back().campaign = ad_campaign_api_obj( *c_itr );
         }

         auto i_itr = inventory_id_idx.find( boost::make_tuple( provider_itr->provider, provider_itr->inventory_id ) );
         if( i_itr != inventory_id_idx.end() )
         {
            astate.incoming_bids.back().inventory = ad_inventory_api_obj( *i_itr );
         }

         auto a_itr = audience_id_idx.find( boost::make_tuple( provider_itr->bidder, provider_itr->audience_id ) );
         if( a_itr != audience_id_idx.end() )
         {
            astate.incoming_bids.back().audience = ad_audience_api_obj( *a_itr );
         }

         ++provider_itr;
      }

      results.push_back( astate );
   }

   return results;
}


vector< ad_bid_state > database_api::get_interface_audience_bids( const ad_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_interface_audience_bids( query );
   });
}

/**
 * Retrieves all bids for an interface that includes a specified
 * account in its audience set. 
 */
vector< ad_bid_state > database_api_impl::get_interface_audience_bids( const ad_query& query )const
{
   vector< ad_bid_state > results;
   account_name_type interface = query.interface;
   account_name_type viewer = query.viewer;

   ad_format_type format = ad_format_type::STANDARD_FORMAT;
   ad_metric_type metric = ad_metric_type::VIEW_METRIC;

   for( size_t i = 0; i < ad_format_values.size(); i++ )
   {
      if( query.ad_format == ad_format_values[ i ] )
      {
         format = ad_format_type( i );
         break;
      }
   }

   for( size_t i = 0; i < ad_metric_values.size(); i++ )
   {
      if( query.ad_metric == ad_metric_values[ i ] )
      {
         metric = ad_metric_type( i );
         break;
      }
   }
   
   const auto& creative_id_idx = _db.get_index< ad_creative_index >().indices().get< by_creative_id >();
   const auto& campaign_id_idx = _db.get_index< ad_campaign_index >().indices().get< by_campaign_id >();
   const auto& audience_id_idx = _db.get_index< ad_audience_index >().indices().get< by_audience_id >();
   const auto& inventory_id_idx = _db.get_index< ad_inventory_index >().indices().get< by_inventory_id >();
   const auto& provider_idx = _db.get_index< ad_bid_index >().indices().get< by_provider_metric_format_price >();

   auto provider_itr = provider_idx.lower_bound( boost::make_tuple( interface, metric, format ) );
   auto provider_end = provider_idx.upper_bound( boost::make_tuple( interface, metric, format ) );

   while( provider_itr != provider_idx.end() && 
      provider_itr != provider_end &&
      results.size() < query.limit )
   {
      auto a_itr = audience_id_idx.find( boost::make_tuple( provider_itr->bidder, provider_itr->audience_id ) );
      if( a_itr != audience_id_idx.end() )
      {
         const ad_audience_object& aud = *a_itr;
         if( aud.is_audience( viewer ) )
         {
            results.push_back( ad_bid_state( *provider_itr ) );
            results.back().audience = ad_audience_api_obj( *a_itr );
            auto cr_itr = creative_id_idx.find( boost::make_tuple( provider_itr->author, provider_itr->creative_id ) );
            if( cr_itr != creative_id_idx.end() )
            {
               results.back().creative = ad_creative_api_obj( *cr_itr );
            }
            auto c_itr = campaign_id_idx.find( boost::make_tuple( provider_itr->account, provider_itr->campaign_id ) );
            if( c_itr != campaign_id_idx.end() )
            {
               results.back().campaign = ad_campaign_api_obj( *c_itr );
            }
            auto i_itr = inventory_id_idx.find( boost::make_tuple( provider_itr->provider, provider_itr->inventory_id ) );
            if( i_itr != inventory_id_idx.end() )
            {
               results.back().inventory = ad_inventory_api_obj( *i_itr );
            }
         }
      }
      ++provider_itr;
   }
   return results;
}


} } // node::app