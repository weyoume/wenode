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
   

const product_sale_object& database::get_product_sale( const account_name_type& name, const shared_string& product_id )const
{ try {
   return get< product_sale_object, by_product_id >( boost::make_tuple( name, product_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(product_id) ) }

const product_sale_object* database::find_product_sale( const account_name_type& name, const shared_string& product_id )const
{
   return find< product_sale_object, by_product_id >( boost::make_tuple( name, product_id ) );
}

const product_sale_object& database::get_product_sale( const account_name_type& name, const string& product_id )const
{ try {
   return get< product_sale_object, by_product_id >( boost::make_tuple( name, product_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(product_id) ) }

const product_sale_object* database::find_product_sale( const account_name_type& name, const string& product_id )const
{
   return find< product_sale_object, by_product_id >( boost::make_tuple( name, product_id ) );
}

const product_purchase_object& database::get_product_purchase( const account_name_type& name, const shared_string& order_id )const
{ try {
   return get< product_purchase_object, by_order_id >( boost::make_tuple( name, order_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(order_id) ) }

const product_purchase_object* database::find_product_purchase( const account_name_type& name, const shared_string& order_id )const
{
   return find< product_purchase_object, by_order_id >( boost::make_tuple( name, order_id ) );
}

const product_purchase_object& database::get_product_purchase( const account_name_type& name, const string& order_id )const
{ try {
   return get< product_purchase_object, by_order_id >( boost::make_tuple( name, order_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(order_id) ) }

const product_purchase_object* database::find_product_purchase( const account_name_type& name, const string& order_id )const
{
   return find< product_purchase_object, by_order_id >( boost::make_tuple( name, order_id ) );
}

const product_auction_sale_object& database::get_product_auction_sale( const account_name_type& name, const shared_string& auction_id )const
{ try {
   return get< product_auction_sale_object, by_auction_id >( boost::make_tuple( name, auction_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(auction_id) ) }

const product_auction_sale_object* database::find_product_auction_sale( const account_name_type& name, const shared_string& auction_id )const
{
   return find< product_auction_sale_object, by_auction_id >( boost::make_tuple( name, auction_id ) );
}

const product_auction_sale_object& database::get_product_auction_sale( const account_name_type& name, const string& auction_id )const
{ try {
   return get< product_auction_sale_object, by_auction_id >( boost::make_tuple( name, auction_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(auction_id) ) }

const product_auction_sale_object* database::find_product_auction_sale( const account_name_type& name, const string& auction_id )const
{
   return find< product_auction_sale_object, by_auction_id >( boost::make_tuple( name, auction_id ) );
}

const product_auction_bid_object& database::get_product_auction_bid( const account_name_type& name, const shared_string& bid_id )const
{ try {
   return get< product_auction_bid_object, by_bid_id >( boost::make_tuple( name, bid_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(bid_id) ) }

const product_auction_bid_object* database::find_product_auction_bid( const account_name_type& name, const shared_string& bid_id )const
{
   return find< product_auction_bid_object, by_bid_id >( boost::make_tuple( name, bid_id ) );
}

const product_auction_bid_object& database::get_product_auction_bid( const account_name_type& name, const string& bid_id )const
{ try {
   return get< product_auction_bid_object, by_bid_id >( boost::make_tuple( name, bid_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(bid_id) ) }

const product_auction_bid_object* database::find_product_auction_bid( const account_name_type& name, const string& bid_id )const
{
   return find< product_auction_bid_object, by_bid_id >( boost::make_tuple( name, bid_id ) );
}

const escrow_object& database::get_escrow( const account_name_type& name, const shared_string& escrow_id )const
{ try {
   return get< escrow_object, by_from_id >( boost::make_tuple( name, escrow_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(escrow_id) ) }

const escrow_object* database::find_escrow( const account_name_type& name, const shared_string& escrow_id )const
{
   return find< escrow_object, by_from_id >( boost::make_tuple( name, escrow_id ) );
}

const escrow_object& database::get_escrow( const account_name_type& name, const string& escrow_id )const
{ try {
   return get< escrow_object, by_from_id >( boost::make_tuple( name, escrow_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(escrow_id) ) }

const escrow_object* database::find_escrow( const account_name_type& name, const string& escrow_id )const
{
   return find< escrow_object, by_from_id >( boost::make_tuple( name, escrow_id ) );
}


void database::process_product_auctions()
{ try {
   time_point now = head_block_time();
   const auto& auction_idx = get_index< product_auction_sale_index >().indices().get< by_completion_time >();
   auto auction_itr = auction_idx.begin();
   const auto& bid_idx = get_index< product_auction_bid_index >().indices().get< by_highest_bid >();

   while( auction_itr != auction_idx.end() &&
      !auction_itr->completed &&
      auction_itr->completion_time <= now )
   {
      const product_auction_sale_object& auction = *auction_itr;
      ilog( "Processing Product Auction - Account: ${a} - ID: ${id} - Type: ${t} - Bid Count: ${c}",
         ("a",auction.account)("id",auction.auction_id )("t",auction.auction_type)("c",auction.bid_count));
      auto bid_itr = bid_idx.lower_bound( boost::make_tuple( auction.account, auction.auction_id, share_type::max(), product_auction_bid_id_type() ) );
      asset bid_price = auction.reserve_bid;
      
      if( bid_itr != bid_idx.end() )
      {
         const product_auction_bid_object& bid = *bid_itr;
         ilog( "Got Top auction bid: ${b}",("b",bid));

         if( bid.seller == auction.account &&
            bid.auction_id == auction.auction_id )
         {
            bid_price = asset( bid.public_bid_amount, auction.bid_asset() );

            if( auction.auction_type == product_auction_type::CONCEALED_SECOND_PRICE_AUCTION )
            {
               ++bid_itr;

               if( bid_itr != bid_idx.end() )
               {
                  const product_auction_bid_object& second_bid = *bid_itr;
                  ilog( "Got Second Price Auction bid: ${b}",("b",second_bid));

                  if( second_bid.seller == auction.account &&
                     second_bid.auction_id == auction.auction_id )
                  {
                     bid_price = asset( second_bid.public_bid_amount, auction.bid_asset() );
                  }
               }
            }
           
            const escrow_object& escrow = create< escrow_object >([&]( escrow_object& esc )
            {
               esc.from = bid.buyer;
               esc.to = bid.seller;
               esc.from_mediator = account_name_type();
               esc.to_mediator = account_name_type();
               esc.payment = bid_price + bid.delivery_value;
               esc.balance = asset( 0, bid.bid_asset );
               esc.escrow_id = bid.bid_id;
               esc.memo = bid.memo;
               esc.json = bid.json;
               esc.acceptance_time = now + fc::days(7);
               esc.escrow_expiration = now + fc::days(14);
               esc.dispute_release_time = time_point::maximum();
               esc.approvals[ bid.buyer ] = false;
               esc.approvals[ bid.seller ] = false;
               esc.created = now;
               esc.last_updated = now;
            });

            modify( bid, [&]( product_auction_bid_object& pabo )
            {
               pabo.winning_bid = true;
               pabo.last_updated = now;
            });

            modify( auction, [&]( product_auction_sale_object& paso )
            {
               paso.winning_bidder = bid.buyer;
               paso.winning_bid_id = bid.bid_id;
            });

            ilog( "Winning Bid from Buyer: ${w} at Bid Price: ${p}: ${b} \n Created escrow: ${e} \n",
               ("w",bid.buyer)("p",bid_price)("b",bid.bid_id)("e",escrow));
         }
      }

      modify( auction, [&]( product_auction_sale_object& paso )
      {
         paso.completed = true;
         paso.last_updated = now;
      });

      ++auction_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


void database::process_escrow_transfers()
{
   // ilog( "Process Escrow Transfers" );

   const auto& escrow_acc_idx = get_index< escrow_index >().indices().get< by_acceptance_time >();
   auto escrow_acc_itr = escrow_acc_idx.lower_bound( false );
   time_point now = head_block_time();

   while( escrow_acc_itr != escrow_acc_idx.end() && 
      !escrow_acc_itr->is_approved() && 
      escrow_acc_itr->acceptance_time <= now )
   {
      const escrow_object& old_escrow = *escrow_acc_itr;
      ++escrow_acc_itr;

      release_escrow( old_escrow );
   }

   const auto& escrow_dis_idx = get_index< escrow_index >().indices().get< by_dispute_release_time >();
   auto escrow_dis_itr = escrow_dis_idx.lower_bound( true );

   while( escrow_dis_itr != escrow_dis_idx.end() && 
      escrow_dis_itr->disputed && 
      escrow_dis_itr->dispute_release_time <= now )
   {
      const escrow_object& old_escrow = *escrow_dis_itr;
      ++escrow_dis_itr;

      release_escrow( old_escrow );
   }
}


void database::dispute_escrow( const escrow_object& escrow )
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;

   const auto& mediator_idx = get_index< mediator_index >().indices().get< by_virtual_position >();
   auto mediator_itr = mediator_idx.begin();

   vector< account_name_type > top_mediators;
   vector< account_name_type > shuffled_mediators;
   flat_set< account_name_type > allocated_mediators;

   // Increment all mediators virtual position by their mediation stake balance.
   while( mediator_itr != mediator_idx.end() )
   {
      if( mediator_itr->active )
      {
         modify( *mediator_itr, [&]( mediator_object& m )
         {
            m.mediation_virtual_position += m.mediator_bond.amount.value;
         });
      }
      ++mediator_itr;
   }

   mediator_itr = mediator_idx.begin();

   // Select top position mediators
   while( mediator_itr != mediator_idx.end() && 
      top_mediators.size() < ( 10 * ESCROW_DISPUTE_MEDIATOR_AMOUNT ) )
   {
      if( mediator_itr->active )
      {
         top_mediators.push_back( mediator_itr->account );
      }
      ++mediator_itr;
   }

   shuffled_mediators = shuffle_accounts( top_mediators );   // Get a random ordered vector of mediator names

   for( auto i = 0; i < ESCROW_DISPUTE_MEDIATOR_AMOUNT; i++ )
   {
      allocated_mediators.insert( shuffled_mediators[ i ] );

      const mediator_object& mediator = get_mediator( shuffled_mediators[ i ] );
      modify( mediator, [&]( mediator_object& m )
      {
         m.mediation_virtual_position = 0;
      });
   }

   modify( escrow, [&]( escrow_object& esc )
   {
      esc.disputed = true;
      esc.mediators = allocated_mediators;
      esc.last_updated = now;
      esc.dispute_release_time = now + ESCROW_DISPUTE_DURATION;
   });

   ilog( "Disputing Escrow: \n ${e} \n", ("e",escrow) );

} FC_CAPTURE_AND_RETHROW() }


/**
 * Selects the median release percentage in the escrow and divides the payment between the TO and FROM accounts.
 * 
 * Fofeits security bonds based on the difference between median
 * and individual votes to create an incentive to reach 
 * cooperative consensus between the mediators about the
 * escrow details.
 * 
 * All voters receive a split of all forfeited bonds, distributing funds
 * as a net profit to accounts the voted closest to the median.
 */
void database::release_escrow( const escrow_object& escrow )
{ try {
   const median_chain_property_object& median_props = get_median_chain_properties();
   asset escrow_bond = asset( ( escrow.payment.amount * median_props.escrow_bond_percent ) / PERCENT_100, escrow.payment.symbol );

   if( escrow.is_approved() )
   {
      vector< uint16_t > release_percentages;
      uint16_t median_release = PERCENT_100;

      for( auto p : escrow.release_percentages )
      {
         release_percentages.push_back( p.second );
      }

      size_t offset = release_percentages.size()/2;

      std::nth_element( release_percentages.begin(), release_percentages.begin()+offset, release_percentages.end(),
      []( uint16_t a, uint16_t b )
      {
         return a < b;
      });

      median_release = release_percentages[ offset ];

      asset to_share = ( escrow.payment * median_release ) / PERCENT_100;
      asset from_share = escrow.payment - to_share;
      asset balance = escrow.balance;

      adjust_liquid_balance( escrow.to, to_share );
      balance -= to_share;
      adjust_liquid_balance( escrow.from, from_share );
      balance -= from_share;

      if( escrow.disputed )
      {
         for( auto a : escrow.release_percentages )     // Refund escrow bonds, minus loss from vote differential
         {
            int16_t delta = abs( median_release - a.second );
            asset escrow_bond_return = asset( ( escrow_bond.amount * ( PERCENT_100 - delta ) ) / PERCENT_100, escrow_bond.symbol );
            balance -= escrow_bond_return;
            adjust_liquid_balance( a.first, escrow_bond_return );
         }
         asset escrow_split = asset( balance.amount / escrow.release_percentages.size(), balance.symbol );
         for( auto a : escrow.release_percentages )     // Distribute remaining balance evenly between voters
         {
            balance -= escrow_split;
            adjust_liquid_balance( a.first, escrow_split );
         }
      }
      else
      {
         for( auto a : escrow.approvals )     // Refund escrow bonds to all approving accounts
         {
            if( a.second == true )
            {
               balance -= escrow_bond;
               adjust_liquid_balance( a.first, escrow_bond );
            }
         }
      }

      adjust_liquid_balance( escrow.from, balance );   // Return remaining balance to FROM.

      ilog( "Released Approved Escrow: ${e}", ("e",escrow) );
   }
   else      // Escrow is being released before being approved, all accounts refunded
   {
      for( auto a : escrow.approvals )
      {
         if( a.second == true )
         {
            if( a.first == escrow.from )
            {
               adjust_liquid_balance( a.first, escrow.payment + escrow_bond );
            }
            else
            {
               adjust_liquid_balance( a.first, escrow_bond );
            }
         }
      }
      ilog( "Cancelled Unapproved Escrow: ${e}", ("e",escrow) );
   }

   adjust_pending_supply( -escrow.balance );
   remove( escrow );

} FC_CAPTURE_AND_RETHROW() }


} } // node::chain