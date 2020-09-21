#include <node/protocol/types.hpp>
#include <node/protocol/authority.hpp>
#include <node/protocol/transaction.hpp>
#include <node/chain/database.hpp>

#include <node/chain/node_object_types.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/custom_operation_interpreter.hpp>

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
#include <fc/io/json.hpp>

#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>

namespace node { namespace chain {


const ad_creative_object& database::get_ad_creative( const account_name_type& account, const shared_string& creative_id )const
{ try {
   return get< ad_creative_object, by_creative_id >( boost::make_tuple( account, creative_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(creative_id) ) }

const ad_creative_object* database::find_ad_creative( const account_name_type& account, const shared_string& creative_id )const
{
   return find< ad_creative_object, by_creative_id >( boost::make_tuple( account, creative_id ) );
}

const ad_creative_object& database::get_ad_creative( const account_name_type& account, const string& creative_id )const
{ try {
   return get< ad_creative_object, by_creative_id >( boost::make_tuple( account, creative_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(creative_id) ) }

const ad_creative_object* database::find_ad_creative( const account_name_type& account, const string& creative_id )const
{
   return find< ad_creative_object, by_creative_id >( boost::make_tuple( account, creative_id ) );
}

const ad_campaign_object& database::get_ad_campaign( const account_name_type& account, const shared_string& campaign_id )const
{ try {
   return get< ad_campaign_object, by_campaign_id >( boost::make_tuple( account, campaign_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(campaign_id) ) }

const ad_campaign_object* database::find_ad_campaign( const account_name_type& account, const shared_string& campaign_id )const
{
   return find< ad_campaign_object, by_campaign_id >( boost::make_tuple( account, campaign_id ) );
}

const ad_campaign_object& database::get_ad_campaign( const account_name_type& account, const string& campaign_id )const
{ try {
   return get< ad_campaign_object, by_campaign_id >( boost::make_tuple( account, campaign_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(campaign_id) ) }

const ad_campaign_object* database::find_ad_campaign( const account_name_type& account, const string& campaign_id )const
{
   return find< ad_campaign_object, by_campaign_id >( boost::make_tuple( account, campaign_id ) );
}

const ad_inventory_object& database::get_ad_inventory( const account_name_type& account, const shared_string& inventory_id )const
{ try {
   return get< ad_inventory_object, by_inventory_id >( boost::make_tuple( account, inventory_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(inventory_id) ) }

const ad_inventory_object* database::find_ad_inventory( const account_name_type& account, const shared_string& inventory_id )const
{
   return find< ad_inventory_object, by_inventory_id >( boost::make_tuple( account, inventory_id ) );
}

const ad_inventory_object& database::get_ad_inventory( const account_name_type& account, const string& inventory_id )const
{ try {
   return get< ad_inventory_object, by_inventory_id >( boost::make_tuple( account, inventory_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(inventory_id) ) }

const ad_inventory_object* database::find_ad_inventory( const account_name_type& account, const string& inventory_id )const
{
   return find< ad_inventory_object, by_inventory_id >( boost::make_tuple( account, inventory_id ) );
}

const ad_audience_object& database::get_ad_audience( const account_name_type& account, const shared_string& audience_id )const
{ try {
   return get< ad_audience_object, by_audience_id >( boost::make_tuple( account, audience_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(audience_id) ) }

const ad_audience_object* database::find_ad_audience( const account_name_type& account, const shared_string& audience_id )const
{
   return find< ad_audience_object, by_audience_id >( boost::make_tuple( account, audience_id ) );
}

const ad_audience_object& database::get_ad_audience( const account_name_type& account, const string& audience_id )const
{ try {
   return get< ad_audience_object, by_audience_id >( boost::make_tuple( account, audience_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(audience_id) ) }

const ad_audience_object* database::find_ad_audience( const account_name_type& account, const string& audience_id )const
{
   return find< ad_audience_object, by_audience_id >( boost::make_tuple( account, audience_id ) );
}

const ad_bid_object& database::get_ad_bid( const account_name_type& account, const shared_string& bid_id )const
{ try {
   return get< ad_bid_object, by_bid_id >( boost::make_tuple( account, bid_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(bid_id) ) }

const ad_bid_object* database::find_ad_bid( const account_name_type& account, const shared_string& bid_id )const
{
   return find< ad_bid_object, by_bid_id >( boost::make_tuple( account, bid_id ) );
}

const ad_bid_object& database::get_ad_bid( const account_name_type& account, const string& bid_id )const
{ try {
   return get< ad_bid_object, by_bid_id >( boost::make_tuple( account, bid_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(bid_id) ) }

const ad_bid_object* database::find_ad_bid( const account_name_type& account, const string& bid_id )const
{
   return find< ad_bid_object, by_bid_id >( boost::make_tuple( account, bid_id ) );
}


/**
 * Pays an advertising delivery operation to the provider
 * and pays a fee split to the demand side interface, the delivery provider
 * the bidder account, the audience members, and the network.
 */
asset database::pay_advertising_delivery( const account_object& provider, const account_object& demand, 
   const account_object& audience, const asset& value )
{ try {
   asset total_fees = ( value * ADVERTISING_FEE_PERCENT ) / PERCENT_100;
   
   asset demand_share = ( total_fees * DEMAND_ADVERTISING_FEE_PERCENT ) / PERCENT_100;
   asset audience_share = ( total_fees * AUDIENCE_ADVERTISING_FEE_PERCENT ) / PERCENT_100;
   asset network_fee = ( total_fees * NETWORK_ADVERTISING_FEE_PERCENT ) / PERCENT_100;

   asset demand_paid = pay_fee_share( demand, demand_share, true );
   asset audience_paid = pay_fee_share( audience, audience_share, true );
   pay_network_fees( provider, network_fee );

   asset fees_paid = network_fee + demand_paid + audience_paid;
   asset delivery_paid = value - fees_paid;

   adjust_liquid_balance( provider.name, delivery_paid );
   adjust_pending_supply( -value );

   ilog( "Account: ${a} paid advertising delivery: ${v}",
      ("a",provider.name)("v",value.to_string()));

   return value;
} FC_CAPTURE_AND_RETHROW() }


/**
 * Activates the delivery process for an ad bid.
 * 
 * Triggered by an operation broadcast from an audience member.
 * Rewards the Provider of the inventory, in addition to the 
 * audience member account that received the ad display.
 */
void database::deliver_ad_bid( const ad_bid_object& bid, const account_object& viewer )
{ try {
   const account_object& bidder = get_account( bid.bidder );
   const account_object& account = get_account( bid.account );
   const account_object& author = get_account( bid.author );
   const account_object& provider = get_account( bid.provider );

   const ad_campaign_object& campaign = get_ad_campaign( account.name, bid.campaign_id );
   const ad_inventory_object& inventory = get_ad_inventory( provider.name, bid.inventory_id );
   const ad_audience_object& audience = get_ad_audience( bidder.name, bid.audience_id );
   const ad_creative_object& creative = get_ad_creative( author.name, bid.creative_id );

   FC_ASSERT( campaign.budget >= bid.bid_price,
      "Campaign does not have sufficient budget to pay the delivery." );
   FC_ASSERT( !bid.is_delivered( viewer.name ) ,
      "Viewer has already been delivered by this bid." );

   const account_object& demand = get_account( campaign.interface );
   time_point now = head_block_time();

   if( campaign.active && 
      inventory.active && 
      audience.active && 
      creative.active && 
      now < bid.expiration &&
      now > campaign.begin &&
      now < campaign.end )
   {
      modify( campaign, [&]( ad_campaign_object& aco )
      {
         aco.budget -= bid.bid_price;
         aco.total_bids -= bid.bid_price;
         aco.last_updated = now;
      });

      modify( inventory, [&]( ad_inventory_object& aio )
      {
         aio.remaining--;
         aio.last_updated = now;
      });

      modify( bid, [&]( ad_bid_object& abo )
      {
         abo.remaining--;
         abo.delivered.insert( viewer.name );
         abo.last_updated = now;
      });

      pay_advertising_delivery( provider, demand, viewer, bid.bid_price );

      ilog( "Delivered Ad Bid to audience Account: ${v} Bid: ${b}",
         ("v",viewer.name)("b",bid.bid_id));

      if( bid.remaining == 0 )
      {
         ilog( "Removed: ${v}",("v",bid));
         remove( bid );
      }
      if( inventory.remaining == 0 )
      {
         ilog( "Removed: ${v}",("v",inventory));
         remove( inventory );
      }
      if( campaign.budget.amount == 0 )
      {
         ilog( "Removed: ${v}",("v",campaign));
         remove( campaign );
      }
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the ad campaign of a bidder, and removes an ad bid object.
 */
void database::cancel_ad_bid( const ad_bid_object& bid )
{ try {
   uint32_t prev_remaining = bid.remaining;
   asset prev_price = bid.bid_price;
   asset bid_total_remaining = prev_remaining * prev_price;
   const ad_campaign_object& campaign = get_ad_campaign( bid.account, bid.campaign_id );

   modify( campaign, [&]( ad_campaign_object& aco )
   {
      aco.total_bids -= bid_total_remaining;
   });

   ilog( "Removed: ${v}",("v",bid));
   remove( bid );
} FC_CAPTURE_AND_RETHROW() }


} } // node::chain