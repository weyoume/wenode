
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


//================================//
// === Advertising Evaluators === //
//================================//


void ad_creative_evaluator::do_apply( const ad_creative_operation& o )
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
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();
   const account_object& author = _db.get_account( o.author );

   FC_ASSERT( author.active, 
      "Author: ${s} must be active for content to be used as ad creative.",("s", o.author) );

   const auto& creative_idx = _db.get_index< ad_creative_index >().indices().get< by_creative_id >();
   auto creative_itr = creative_idx.find( boost::make_tuple( o.account, o.creative_id ) );

   ad_format_type ad_format = ad_format_type::STANDARD_FORMAT;

   for( size_t i = 0; i < ad_format_values.size(); i++ )
   {
      if( o.format_type == ad_format_values[ i ] )
      {
         ad_format = ad_format_type( i );
         break;
      }
   }

   switch( ad_format )
   {
      case ad_format_type::STANDARD_FORMAT:
      {
         const comment_object& creative_obj = _db.get_comment( o.author, o.objective );
         FC_ASSERT( creative_obj.root,
            "Creative post must be a root comment" );
         FC_ASSERT( !creative_obj.deleted,
            "Creative post has been deleted." );
         FC_ASSERT( !creative_obj.is_encrypted(),
            "Creative post must be public." );
         FC_ASSERT( creative_obj.premium_price.amount == 0,
            "Creative post must not be a premium post." );
      }
      break;
      case ad_format_type::PREMIUM_FORMAT:
      {
         const comment_object& creative_obj = _db.get_comment( o.author, o.objective );
         FC_ASSERT( creative_obj.root,
            "Creative premium post must be a root comment" );
         FC_ASSERT( !creative_obj.deleted,
            "Creative premium post has been deleted." );
         FC_ASSERT( creative_obj.is_encrypted(),
            "Creative premium post must be encrypted." );
         FC_ASSERT( creative_obj.premium_price.amount > 0,
            "Creative post must be a premium post." );
      }
      break;
      case ad_format_type::PRODUCT_FORMAT:
      {
         const product_sale_object& creative_obj = _db.get_product_sale( o.author, o.objective );
         FC_ASSERT( creative_obj.active,
            "Creative product has been deleted." );
      }
      break;
      case ad_format_type::LINK_FORMAT:
      {
         validate_url( o.objective );
      }
      break;
      case ad_format_type::ACCOUNT_FORMAT:
      {
         const account_object& creative_obj = _db.get_account( account_name_type( o.objective ) );
         FC_ASSERT( creative_obj.active,
            "Creative account is inactive." );
      }
      break;
      case ad_format_type::COMMUNITY_FORMAT:
      {
         const community_object& creative_obj = _db.get_community( community_name_type( o.objective ) );
         FC_ASSERT( creative_obj.active,
            "Creative community is inactive and cannot be selected." );
      }
      break;
      case ad_format_type::ASSET_FORMAT:
      {
         const asset_object& creative_obj = _db.get_asset( asset_symbol_type( o.objective ) );
         FC_ASSERT( creative_obj.symbol == asset_symbol_type( o.objective ),
            "Creative Asset is inactive and cannot be selected." );
      }
      break;
      default:
      {
         FC_ASSERT( false, "Ad format type is invalid." );
      }
      break;
   };

   if( creative_itr == creative_idx.end() )    // Ad creative does not exist
   {
      const ad_creative_object& creative = _db.create< ad_creative_object >( [&]( ad_creative_object& aco )
      {
         aco.account = o.account;
         aco.author = o.author;
         aco.format_type = ad_format;
         from_string( aco.creative_id, o.creative_id );
         from_string( aco.objective, o.objective );
         from_string( aco.creative, o.creative );
         from_string( aco.json, o.json );
         aco.active = o.active;
         aco.created = now;
         aco.last_updated = now;
      });
      ilog( "Creative New Ad creative: \n ${c} \n", ("c",creative));
   }
   else  // Creative exists, editing
   {
      const ad_creative_object& creative = *creative_itr;

      _db.modify( creative, [&]( ad_creative_object& aco )
      {
         aco.format_type = ad_format;
         from_string( aco.objective, o.objective );
         from_string( aco.creative, o.creative );
         from_string( aco.json, o.json );
         aco.active = o.active;
         aco.last_updated = now;
      });

      ilog( "Updated Ad creative: \n ${c} \n", ("c",creative));
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void ad_campaign_evaluator::do_apply( const ad_campaign_operation& o )
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
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   flat_set< account_name_type > agent_set;
   agent_set.insert( o.account );

   for( auto a : o.agents )   // Ensure all agent accounts exist
   {
      const account_object& acc = _db.get_account( a );
      if( acc.active )
      {
         agent_set.insert( a );
      }
   }

   const interface_object& interface = _db.get_interface( o.interface );
   FC_ASSERT( interface.active,
      "Ad Interface must be active." );
   
   time_point now = _db.head_block_time();

   const auto& campaign_idx = _db.get_index< ad_campaign_index >().indices().get< by_campaign_id >();
   auto campaign_itr = campaign_idx.find( boost::make_tuple( o.account, o.campaign_id ) );

   if( campaign_itr == campaign_idx.end() )    // Ad campaign does not exist
   {
      FC_ASSERT( o.budget.amount > BLOCKCHAIN_PRECISION,
         "New Campaign requires a minimum budget of 1 Asset" );

      _db.adjust_liquid_balance( o.account, -o.budget );
      _db.adjust_pending_supply( o.budget );

      const ad_campaign_object& campaign = _db.create< ad_campaign_object >( [&]( ad_campaign_object& aco )
      {
         aco.account = o.account;
         from_string( aco.campaign_id, o.campaign_id );
         from_string( aco.json, o.json );
         aco.budget = o.budget;
         aco.total_bids = asset( 0, o.budget.symbol );
         aco.interface = o.interface;
         aco.begin = o.begin;
         aco.end = o.end;
         aco.agents = agent_set;
         aco.created = now;
         aco.last_updated = now;
         aco.active = o.active;
      });

      ilog( "Created New Ad campaign: \n ${c} \n", ("c",campaign));
   }
   else  // campaign exists, editing
   {
      const ad_campaign_object& campaign = *campaign_itr;
      FC_ASSERT( campaign.budget.symbol == o.budget.symbol,
         "Budget asset must be the same as the campaigns existing budget asset.");
      FC_ASSERT( o.budget >= campaign.total_bids,
         "New Budget cannot bring campaign below its outstanding total bids. Cancel bids, or increase budget.");

      asset delta = campaign.budget - o.budget;

      _db.adjust_liquid_balance( o.account, delta );  // Deduct balance by new additional budget amount, or refund difference. 
      _db.adjust_pending_supply( -delta );

      _db.modify( campaign, [&]( ad_campaign_object& aco )
      {
         from_string( aco.json, o.json );
         aco.budget = o.budget;
         aco.begin = o.begin;
         aco.end = o.end;
         aco.agents = agent_set;
         aco.last_updated = now;
         aco.active = o.active;
      });

      ilog( "Updated Ad campaign: \n ${c} \n", ("c",campaign));

      if( campaign.budget.amount == 0 )
      {
         ilog( "Removed: ${v}",("v",campaign));
         _db.remove( campaign );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void ad_inventory_evaluator::do_apply( const ad_inventory_operation& o )
{ try {
   const account_name_type& signed_for = o.provider;
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
   const interface_object& interface = _db.get_interface( o.provider );

   FC_ASSERT( interface.active,
      "Creating Ad Inventory requires an active interface." );

   const account_object& provider = _db.get_account( o.provider );
   const ad_audience_object& audience = _db.get_ad_audience( provider.name, o.audience_id );
   FC_ASSERT( audience.active, 
      "Audience: ${s} must be active to broadcast transaction.",("s", o.audience_id) );

   time_point now = _db.head_block_time();

   const auto& inventory_idx = _db.get_index< ad_inventory_index >().indices().get< by_inventory_id >();
   auto inventory_itr = inventory_idx.find( boost::make_tuple( o.provider, o.inventory_id ) );

   ad_metric_type ad_metric = ad_metric_type::VIEW_METRIC;

   for( size_t i = 0; i < ad_metric_values.size(); i++ )
   {
      if( o.metric == ad_metric_values[ i ] )
      {
         ad_metric = ad_metric_type( i );
         break;
      }
   }

   if( inventory_itr == inventory_idx.end() )    // Ad inventory does not exist
   {
      const ad_inventory_object& inventory = _db.create< ad_inventory_object >( [&]( ad_inventory_object& aio )
      {
         aio.provider = o.provider;
         from_string( aio.inventory_id, o.inventory_id );
         from_string( aio.json, o.json );
         aio.metric = ad_metric;
         from_string( aio.audience_id, o.audience_id );
         aio.min_price = o.min_price;
         aio.inventory = o.inventory;
         aio.remaining = o.inventory;
         aio.created = now;
         aio.last_updated = now;
         aio.active = o.active;
      });

      ilog( "New Ad Inventory: \n ${i} \n", ("i",inventory));
   }
   else       // Inventory exists, editing
   {
      const ad_inventory_object& inventory = *inventory_itr;

      uint32_t prev_inventory = inventory.inventory;
      uint32_t prev_remaining = inventory.remaining;
      uint32_t delta = o.inventory - prev_inventory;
      uint32_t new_remaining = prev_remaining + delta;

      FC_ASSERT( new_remaining >= 0 ,
         "Cannot lower remaining inventory to below 0.");

      _db.modify( inventory, [&]( ad_inventory_object& aio )
      {
         from_string( aio.json, o.json );
         aio.min_price = o.min_price;
         aio.inventory = o.inventory;
         aio.remaining = new_remaining;
         aio.last_updated = now;
         aio.active = o.active;
      });

      ilog( "Updated Ad Inventory: \n ${i} \n", ("i",inventory));

      if( inventory.remaining == 0 )
      {
         ilog( "Removed: ${v}",("v",inventory));
         _db.remove( inventory );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void ad_audience_evaluator::do_apply( const ad_audience_operation& o )
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

   flat_set< account_name_type > audience_set;

   for( auto a : o.audience )   // Ensure all audience member accounts exist
   {
      const account_object& acc = _db.get_account( a );
      if( acc.active && acc.membership == membership_tier_type::NONE )
      {
         audience_set.insert( a );
      }
   }

   FC_ASSERT( audience_set.size(), 
      "Audience must contain at least one valid account." );

   time_point now = _db.head_block_time();

   const auto& audience_idx = _db.get_index< ad_audience_index >().indices().get< by_audience_id >();
   auto audience_itr = audience_idx.find( boost::make_tuple( o.account, o.audience_id ) );

   if( audience_itr == audience_idx.end() )    // Ad audience does not exist
   {
      const ad_audience_object& audience = _db.create< ad_audience_object >( [&]( ad_audience_object& aao )
      {
         aao.account = o.account;
         from_string( aao.audience_id, o.audience_id );
         from_string( aao.json, o.json );
         aao.audience = audience_set;
         aao.created = now;
         aao.last_updated = now;
         aao.active = o.active;
      });

      ilog( "New Ad Audience: \n ${a} \n",
         ("a",audience));
   }
   else  // audience exists, editing
   {
      const ad_audience_object& audience = *audience_itr;

      _db.modify( audience, [&]( ad_audience_object& aao )
      {
         from_string( aao.json, o.json );
         aao.audience = audience_set;
         aao.last_updated = now;
         aao.active = o.active;
      });

      ilog( "Updated Ad Audience: \n ${a} \n",
         ("a",audience));
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void ad_bid_evaluator::do_apply( const ad_bid_operation& o )
{ try {
   const account_name_type& signed_for = o.bidder;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& bidder = _db.get_account( o.bidder );
   const account_object& account = _db.get_account( o.account );
   const account_object& author = _db.get_account( o.author );
   const account_object& provider = _db.get_account( o.provider );

   const ad_campaign_object& campaign = _db.get_ad_campaign( account.name, o.campaign_id );
   const ad_creative_object& creative = _db.get_ad_creative( author.name, o.creative_id );
   const ad_inventory_object& inventory = _db.get_ad_inventory( provider.name, o.inventory_id );
   const ad_audience_object& inventory_audience = _db.get_ad_audience( provider.name, inventory.audience_id );

   time_point now = _db.head_block_time();

   FC_ASSERT( now < o.expiration,
      "Bid expiration time has to be in the future." );
   FC_ASSERT( campaign.active,
      "Campaign must be set to active to create a bid." );
   FC_ASSERT( creative.active,
      "Creative must be set to active to create a bid." );
   FC_ASSERT( now > campaign.begin,
      "Campaign has not begun yet, please wait until the beginning time to create bids." );
   FC_ASSERT( now < campaign.end,
      "Campaign has expired.");
   FC_ASSERT( inventory.active,
      "Inventory must be set to active to create a bid." );
   FC_ASSERT( inventory.remaining >= o.requested,
      "Bid request exceeds the inventory remaining from the provider." );
   FC_ASSERT( o.bid_price >= inventory.min_price,
      "Bid price per metric must be greater than the inventory minimum price." );

   asset_symbol_type budget_asset = campaign.budget.symbol;
   asset_symbol_type inventory_asset = inventory.min_price.symbol;

   FC_ASSERT( budget_asset == inventory_asset,
      "Campaign's budget asset must match the inventory minimum price asset." );
   FC_ASSERT( campaign.is_agent( bidder.name ),
      "Bidder must be a designated agent of the campaign to create a bid on its behalf." );

   flat_set< account_name_type > combined_audience;
   
   if( o.included_audiences.size() )
   {
      for( auto a : o.included_audiences )   // Ensure all include audience objects exist
      {
         const ad_audience_object& aud = _db.get_ad_audience( bidder.name, a );
         FC_ASSERT( aud.active,
            "Bid contains an inactive audience.");
         for( account_name_type name : aud.audience )
         {
            if( inventory_audience.is_audience( name ) )
            {
               combined_audience.insert( name );
            }
         }
      }
   }
   else
   {
      for( account_name_type name : inventory_audience.audience )    // Use entire audience when none are specified
      {
         combined_audience.insert( name );
      }
   }

   if( o.excluded_audiences.size() )
   {
      for( auto a : o.excluded_audiences )   // Ensure all excluded audience objects exist
      {
         const ad_audience_object& aud = _db.get_ad_audience( bidder.name, a );
         FC_ASSERT( aud.active,
            "Bid contains an inactive audience.");
         for( account_name_type name : aud.audience )
         {
            combined_audience.erase( name );
         }
      }
   }

   const auto& audience_idx = _db.get_index< ad_audience_index >().indices().get< by_audience_id >();
   auto audience_itr = audience_idx.find( boost::make_tuple( o.bidder, o.audience_id ) );
   
   if( audience_itr == audience_idx.end() )    // Combined Ad audience for this bid_id does not exist, creating new one.
   {
      _db.create< ad_audience_object >( [&]( ad_audience_object& aao )
      {
         aao.account = o.bidder;
         from_string( aao.audience_id, o.audience_id );     // New audience ID is bid_id of the bid.
         from_string( aao.json, o.json );
         aao.audience = combined_audience;
         aao.created = now;
         aao.last_updated = now;
      });
   }
   else    // Bid audience already exists for this bid id, editing bid.
   {
      const ad_audience_object& audience = *audience_itr;

      _db.modify( audience, [&]( ad_audience_object& aao )
      {
         from_string( aao.json, o.json );
         aao.audience = combined_audience;
         aao.last_updated = now;
      }); 
   }
   
   const auto& bid_idx = _db.get_index< ad_bid_index >().indices().get< by_bid_id >();
   auto bid_itr = bid_idx.find( boost::make_tuple( bidder.name, o.bid_id ) );

   if( bid_itr == bid_idx.end() )    // Ad bid does not exist, creating new bid.
   {
      FC_ASSERT( o.active, 
         "Bid does not exist, set active to true." );

      asset new_total_bids = campaign.total_bids + o.bid_price * o.requested;

      FC_ASSERT( campaign.budget >= new_total_bids,
         "Total Bids cannot exceed the budget of the campaign, please increase the budget balance of the campaign." );

      const ad_bid_object& bid = _db.create< ad_bid_object >( [&]( ad_bid_object& abo )
      {
         abo.bidder = o.bidder;
         from_string( abo.bid_id, o.bid_id );
         from_string( abo.audience_id, o.audience_id );
         abo.account = o.account;
         from_string( abo.campaign_id, o.campaign_id );
         abo.author = o.author;
         from_string( abo.creative_id, o.creative_id );
         abo.provider = o.provider;
         from_string( abo.inventory_id, o.inventory_id );
         abo.bid_price = o.bid_price;
         abo.format = creative.format_type;
         abo.metric = inventory.metric;
         
         abo.objective = creative.objective;
         abo.requested = o.requested;
         abo.remaining = o.requested;
         from_string( abo.json, o.json );
         abo.created = now;
         abo.last_updated = now;
         abo.expiration = o.expiration;
      });

      ilog( "Created New Ad Bid: \n ${b} \n",("b",bid));

      _db.modify( campaign, [&]( ad_campaign_object& aco )
      {
         aco.total_bids = new_total_bids;
      }); 
   }
   else  
   {
      const ad_bid_object& bid = *bid_itr;

      FC_ASSERT( o.audience_id == to_string( bid.audience_id ),
         "Audience ID cannot be changed on existing bid." );

      uint32_t prev_requested = bid.requested;
      uint32_t prev_remaining = bid.remaining;
      asset prev_price = bid.bid_price;

      if( o.active )    // Bid exists and is being edited.
      {
         uint32_t delta_req = o.requested - prev_requested;
         asset delta_price = o.bid_price - prev_price;
         uint32_t new_remaining = prev_remaining + delta_req;

         FC_ASSERT( new_remaining >= 0, 
            "New inventory requested remaining must be equal to or greater than zero." );
         
         asset delta_total_bids = prev_remaining * delta_price + delta_req * o.bid_price;
         asset adjusted_total_bids = campaign.total_bids + delta_total_bids;

         FC_ASSERT( campaign.budget >= adjusted_total_bids, 
            "Total Bids cannot exceed the budget of the campaign, please increase the budget balance of the campaign." );

         _db.modify( bid, [&]( ad_bid_object& abo )
         {
            abo.bid_price = o.bid_price;
            abo.requested = o.requested;
            abo.remaining = new_remaining;
            from_string( abo.json, o.json );
            abo.last_updated = now;
            abo.expiration = o.expiration;
         });

         ilog( "Updated Ad Bid: \n ${b} \n", ("b",bid));

         if( delta_total_bids.amount != 0 )
         {
            _db.modify( campaign, [&]( ad_campaign_object& aco )
            {
               aco.total_bids = adjusted_total_bids;
            });
         }
      }
      else     // Removing bid object.
      {
         asset bid_total_remaining = prev_remaining * prev_price;

         _db.modify( campaign, [&]( ad_campaign_object& aco )
         {
            aco.total_bids -= bid_total_remaining;
         });

         ilog( "Removed: ${v}",("v",bid));
         _db.remove( bid );
      }  
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


} } // node::chain