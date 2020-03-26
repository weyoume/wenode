
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
// === Marketplace Evaluators === //
//================================//


void product_update_evaluator::do_apply( const product_update_operation& o )
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

   time_point now = _db.head_block_time();

   product_sale_type product_sale = product_sale_type::FIXED_PRICE_SALE;

   for( size_t i = 0; i < product_sale_values.size(); i++ )
   {
      if( o.sale_type == product_sale_values[ i ] )
      {
         product_sale = product_sale_type( i );
         break;
      }
   }

   const auto& product_idx = _db.get_index< product_index >().indices().get< by_product_id >();
   auto product_itr = product_idx.find( std::make_tuple( o.account, o.product_id ) );

   if( product_itr == product_idx.end() )
   {
      _db.create< product_object >([&]( product_object& p )
      {
         p.account = o.account;
         from_string( p.product_id, o.product_id );
         from_string( p.name, o.name );
         p.sale_type = product_sale;
         from_string( p.url, o.url );
         from_string( p.json, o.json );

         p.product_variants.reserve( o.product_variants.size() );
         for( size_t i = 0; i < o.product_variants.size(); i++ )
         {
            from_string( p.product_variants[ i ], o.product_variants[ i ] );
         }

         p.product_details.reserve( o.product_details.size() );
         for( size_t i = 0; i < o.product_details.size(); i++ )
         {
            from_string( p.product_details[ i ], o.product_details[ i ] );
         }

         p.product_images.reserve( o.product_images.size() );
         for( size_t i = 0; i < o.product_images.size(); i++ )
         {
            from_string( p.product_images[ i ], o.product_images[ i ] );
         }

         for( auto pr : o.product_prices )
         {
            p.product_prices.push_back( pr );
         }

         for( auto sa : o.stock_available )
         {
            p.stock_available.push_back( sa );
         }

         p.delivery_variants.reserve( o.delivery_variants.size() );
         for( size_t i = 0; i < o.delivery_variants.size(); i++ )
         {
            from_string( p.delivery_variants[ i ], o.delivery_variants[ i ] );
         }

         p.delivery_details.reserve( o.delivery_details.size() );
         for( size_t i = 0; i < o.delivery_details.size(); i++ )
         {
            from_string( p.delivery_details[ i ], o.delivery_details[ i ] );
         }

         for( auto dp : o.delivery_prices )
         {
            p.delivery_prices.push_back( dp );
         }
         
         p.created = now;
         p.last_updated = now;
      });
   }
   else
   {
      const product_object& product = *product_itr;

      _db.modify( product, [&]( product_object& p )
      {
         from_string( p.name, o.name );
         p.sale_type = product_sale;
         from_string( p.url, o.url );
         from_string( p.json, o.json );

         p.product_variants.reserve( o.product_variants.size() );
         for( size_t i = 0; i < o.product_variants.size(); i++ )
         {
            from_string( p.product_variants[ i ], o.product_variants[ i ] );
         }

         p.product_details.reserve( o.product_details.size() );
         for( size_t i = 0; i < o.product_details.size(); i++ )
         {
            from_string( p.product_details[ i ], o.product_details[ i ] );
         }

         p.product_images.reserve( o.product_images.size() );
         for( size_t i = 0; i < o.product_images.size(); i++ )
         {
            from_string( p.product_images[ i ], o.product_images[ i ] );
         }

         for( auto pr : o.product_prices )
         {
            p.product_prices.push_back( pr );
         }

         for( auto sa : o.stock_available )
         {
            p.stock_available.push_back( sa );
         }

         p.delivery_variants.reserve( o.delivery_variants.size() );
         for( size_t i = 0; i < o.delivery_variants.size(); i++ )
         {
            from_string( p.delivery_variants[ i ], o.delivery_variants[ i ] );
         }

         p.delivery_details.reserve( o.delivery_details.size() );
         for( size_t i = 0; i < o.delivery_details.size(); i++ )
         {
            from_string( p.delivery_details[ i ], o.delivery_details[ i ] );
         }

         for( auto dp : o.delivery_prices )
         {
            p.delivery_prices.push_back( dp );
         }
         p.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void product_purchase_evaluator::do_apply( const product_purchase_operation& o )
{ try {
   const account_name_type& signed_for = o.buyer;
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

   time_point now = _db.head_block_time();

   const product_object& product = _db.get_product( o.seller, o.product_id );

   asset payment = asset( 0, product.product_prices[0].symbol );

   flat_map< string, asset > price_map;

   for( size_t i = 0; i < product.product_variants.size(); i++ )
   {
      price_map[ to_string( product.product_variants[ i ] ) ] = product.product_prices[ i ];
   }

   flat_map< string, asset > delivery_map;

   for( size_t i = 0; i < product.delivery_variants.size(); i++ )
   {
      delivery_map[ to_string( product.delivery_variants[ i ] ) ] = product.delivery_prices[ i ];
   }

   for( size_t i = 0; i < o.order_variants.size(); i++ )
   {
      payment += price_map[ o.order_variants[ i ] ] * o.order_size[ i ];
   }

   payment += delivery_map[ o.delivery_variant ];

   const auto& escrow_idx = _db.get_index< escrow_index >().indices().get< by_from_id >();
   auto escrow_itr = escrow_idx.find( std::make_tuple( o.buyer, o.order_id ) );

   const auto& purchase_order_idx = _db.get_index< purchase_order_index >().indices().get< by_order_id >();
   auto purchase_order_itr = purchase_order_idx.find( std::make_tuple( o.buyer, o.order_id ) );

   if( escrow_itr == escrow_idx.end() && purchase_order_itr == purchase_order_idx.end() )
   {
      _db.create< purchase_order_object >([&]( purchase_order_object& p )
      {
         p.buyer = o.buyer;
         from_string( p.order_id, o.order_id );
         p.seller = o.seller;
         from_string( p.product_id, o.product_id );
         p.order_variants.reserve( o.order_variants.size() );
         for( size_t i = 0; i < o.order_variants.size(); i++ )
         {
            from_string( p.order_variants[ i ], o.order_variants[ i ] );
         }
         for( auto os : o.order_size )
         {
            p.order_size.push_back( os );
         }
         p.order_value = payment;
         from_string( p.memo, o.memo );
         from_string( p.json, o.json );
         from_string( p.shipping_address, o.shipping_address );
         from_string( p.delivery_variant, o.delivery_variant );
         from_string( p.delivery_details, o.delivery_details );
         p.created = now;
         p.last_updated = now;
         p.completed = false;
      });

      _db.create< escrow_object >([&]( escrow_object& esc )
      {
         esc.from = o.buyer;
         esc.to = o.seller;
         esc.from_mediator = account_name_type();
         esc.to_mediator = account_name_type();
         esc.payment = payment;
         esc.balance = asset( 0, payment.symbol );
         from_string( esc.escrow_id, o.order_id );     // Order ID used for escrow ID
         from_string( esc.memo, o.memo );
         from_string( esc.json, o.json );
         esc.acceptance_time = o.acceptance_time;
         esc.escrow_expiration = o.escrow_expiration;
         esc.dispute_release_time = time_point::maximum();
         esc.approvals[ o.buyer ] = false;
         esc.approvals[ o.seller ] = false;
         esc.created = now;
         esc.last_updated = now;
      });
   }
   else if( escrow_itr != escrow_idx.end() && purchase_order_itr != purchase_order_idx.end() )
   {
      const escrow_object& escrow = *escrow_itr;
      const purchase_order_object& order = *purchase_order_itr;

      for( auto a : escrow.approvals )
      {
         FC_ASSERT( a.second == false, 
            "Cannot edit product purchase escrow after approvals have been made." );
      }

      FC_ASSERT( order.completed == false, 
         "Cannot edit purchase after completion." );

      _db.modify( order, [&]( purchase_order_object& p )
      {
         p.order_variants.reserve( o.order_variants.size() );
         for( size_t i = 0; i < o.order_variants.size(); i++ )
         {
            from_string( p.order_variants[ i ], o.order_variants[ i ] );
         }
         for( auto os : o.order_size )
         {
            p.order_size.push_back( os );
         }
         p.order_value = payment;
         from_string( p.memo, o.memo );
         from_string( p.json, o.json );
         from_string( p.shipping_address, o.shipping_address );
         from_string( p.delivery_variant, o.delivery_variant );
         from_string( p.delivery_details, o.delivery_details );
         p.last_updated = now;
         p.completed = o.completed;
      });

      _db.modify( escrow, [&]( escrow_object& esc )
      {
         esc.payment = payment;
         from_string( esc.memo, o.memo );
         from_string( esc.json, o.json );
         esc.acceptance_time = o.acceptance_time;
         esc.escrow_expiration = o.escrow_expiration;
         esc.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }



void escrow_transfer_evaluator::do_apply( const escrow_transfer_operation& o )
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
   time_point now = _db.head_block_time();

   FC_ASSERT( o.account == o.to || o.account == o.from,
      "Account must be a party to the escrow transaction to initiate transfer proposal." );
   FC_ASSERT( o.acceptance_time > now, 
      "The escrow ratification deadline must be after head block time." );
   FC_ASSERT( o.escrow_expiration > now, 
      "The escrow expiration must be after head block time." );

   const account_object& to_account = _db.get_account( o.to );
   FC_ASSERT( to_account.active, 
      "Account: ${s} must be active to begin escrow transfer.",("s", o.to) );
   const account_object& from_account = _db.get_account( o.from );
   FC_ASSERT( from_account.active, 
      "Account: ${s} must be active to begin escrow transfer.",("s", o.from) );
   asset liquid = _db.get_liquid_balance( o.from, o.amount.symbol );

   FC_ASSERT( liquid >= o.amount,
      "Account cannot cover costs of escrow. Required: ${r} Available: ${a}", ("r",o.amount)("a",liquid) );

   const auto& escrow_idx = _db.get_index< escrow_index >().indices().get< by_from_id >();
   auto escrow_itr = escrow_idx.find( std::make_tuple( o.from, o.escrow_id ) );

   if( escrow_itr == escrow_idx.end() )
   {
      _db.create< escrow_object >([&]( escrow_object& esc )
      {
         esc.from = o.from;
         esc.to = o.to;
         esc.from_mediator = account_name_type();
         esc.to_mediator = account_name_type();
         esc.acceptance_time = o.acceptance_time;
         esc.escrow_expiration = o.escrow_expiration;
         esc.payment = o.amount;
         esc.balance = asset( 0, o.amount.symbol );
         from_string( esc.escrow_id, o.escrow_id );
         from_string( esc.memo, o.memo );
         from_string( esc.json, o.json );
         esc.dispute_release_time = time_point::maximum();
         esc.approvals[ o.from ] = false;
         esc.approvals[ o.to ] = false;
         esc.created = now;
         esc.last_updated = now;
      });
   }
   else
   {
      const escrow_object& escrow = *escrow_itr;

      for( auto a : escrow.approvals )
      {
         FC_ASSERT( a.second == false, 
            "Cannot edit escrow after approvals have been made." );
      }

      _db.modify( escrow, [&]( escrow_object& esc )
      {
         esc.acceptance_time = o.acceptance_time;
         esc.escrow_expiration = o.escrow_expiration;
         esc.payment = o.amount;
         from_string( esc.memo, o.memo );
         from_string( esc.json, o.json );
         esc.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void escrow_approve_evaluator::do_apply( const escrow_approve_operation& o )
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

   const escrow_object& escrow = _db.get_escrow( o.escrow_from, o.escrow_id );
   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   const account_object& mediator_account = _db.get_account( o.mediator );
   FC_ASSERT( mediator_account.active, 
      "Account: ${s} must be active to be assigned to escrow transfer.",("s", o.mediator) );
   const mediator_object& mediator_obj = _db.get_mediator( o.mediator );
   FC_ASSERT( mediator_obj.active, 
      "Mediator: ${s} must be active to be assigned to escrow transfer.",("s", o.mediator) );

   asset liquid = _db.get_liquid_balance( o.account, escrow.payment.symbol );
   // Escrow bond is a percentage paid as security in the event of dispute, and can be forfeited.
   asset escrow_bond = asset( ( escrow.payment.amount * median_props.escrow_bond_percent ) / PERCENT_100, escrow.payment.symbol );

   flat_map< account_name_type, bool > approvals = escrow.approvals;

   FC_ASSERT( !approvals[ o.account ],
      "Account: ${a} has already approved the escrow.", ("a", o.account ) );

   time_point now = _db.head_block_time();

   if( escrow.disputed )
   {
      FC_ASSERT( escrow.is_mediator( o.account ),
         "Account must be an allocated mediator to approve during a dispute." );
      FC_ASSERT( o.approved,
         "Mediator cannot select to cancel a disputed escrow" );
   }
   else
   {
      FC_ASSERT( o.account == escrow.to || o.account == escrow.from || 
         o.account == escrow.to_mediator || o.account == escrow.from_mediator,
         "Account must be a party to the escrow transaction to approve." );
   }

   if( o.approved )
   {
      if( o.account == escrow.from )
      {
         FC_ASSERT( liquid >= escrow.payment + escrow_bond,
            "Account cannot cover costs of escrow. Required: ${r} Available: ${a}", ("r", escrow_bond)("a",liquid) );
         _db.adjust_liquid_balance( o.account, -( escrow.payment + escrow_bond ) );
         _db.adjust_pending_supply( escrow.payment + escrow_bond );
      }
      else
      {
         FC_ASSERT( liquid >= escrow_bond,
            "Account cannot cover costs of escrow. Required: ${r} Available: ${a}", ("r", escrow_bond)("a",liquid) );
         _db.adjust_liquid_balance( o.account, -escrow_bond );
         _db.adjust_pending_supply( escrow_bond );
      }

      _db.modify( escrow, [&]( escrow_object& esc )
      {
         esc.approvals[ o.account ] = true;
         if( o.account == escrow.from )
         {
            esc.balance += ( escrow.payment + escrow_bond );
            esc.from_mediator = o.mediator;     // Adds selected mediator to escrow
         }
         else if( o.account == escrow.to )
         {
            esc.balance += escrow_bond;
            esc.to_mediator = o.mediator;      // Adds selected mediator to escrow
         }
         else if( o.account == escrow.from_mediator )
         {
            esc.balance += escrow_bond;
         }
         else if( o.account == escrow.to_mediator )
         {
            esc.balance += escrow_bond;
         }
         esc.last_updated = now;
      });
   }
   else       // Any accounts can cancel the escrow and refund all previous payments
   {
      _db.release_escrow( escrow );
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void escrow_dispute_evaluator::do_apply( const escrow_dispute_operation& o )
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

   const escrow_object& escrow = _db.get_escrow( o.escrow_from, o.escrow_id );
   time_point now = _db.head_block_time();

   FC_ASSERT( o.account == escrow.to || o.account == escrow.from,
      "Account must be either sender or receiver of the escrow transaction to dispute." );
   FC_ASSERT( now < escrow.escrow_expiration,
      "Disputing the escrow must happen before expiration." );
   FC_ASSERT( escrow.is_approved(),
      "The escrow must be approved by all parties and selected mediators before a dispute can be raised." );
   FC_ASSERT( !escrow.disputed,
      "The escrow is already under dispute." );

   _db.dispute_escrow( escrow );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void escrow_release_evaluator::do_apply( const escrow_release_operation& o )
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

   const escrow_object& escrow = _db.get_escrow( o.escrow_from, o.escrow_id );
   time_point now = _db.head_block_time();
   
   FC_ASSERT( escrow.is_approved(),
      "Funds cannot be released prior to escrow approval by all participating accounts." );
   FC_ASSERT( o.account == escrow.to || o.account == escrow.from || 
      o.account == escrow.to_mediator || o.account == escrow.from_mediator || escrow.is_mediator( o.account ),
      "Account must be a participant in the escrow transaction to release funds." );
   FC_ASSERT( escrow.approvals.at( o.account ) == true,
      "Account must have approved and deposited an escrow bond with the escrow transaction to release funds." );
   
   if( escrow.disputed )  // If there is a dispute, create release percent vote
   {
      _db.modify( escrow, [&]( escrow_object& esc )
      {
         esc.release_percentages[ o.account ] = o.release_percent;
      });
   }
   else
   {
      FC_ASSERT( o.account == escrow.from || o.account == escrow.to,
         "Only Sender: ${f} and receiver: ${t} can release funds from a non-disputed escrow",
         ("f", escrow.from)("t", escrow.to ) );

      if( escrow.escrow_expiration > now )     // If escrow expires and there is no dispute, either party can release funds to either party.
      {
         if( o.account == escrow.from )    // If there is no dispute and escrow has not expired, either party can release funds to the other.
         {
            FC_ASSERT( o.release_percent == PERCENT_100,
               "Release percent should be PERCENT_100 before expiration." );
         }
         else if( o.account == escrow.to )
         {
            FC_ASSERT( o.release_percent == 0,
               "Release percent should be 0 before expiration." );
         }
      }

      _db.modify( escrow, [&]( escrow_object& esc )
      {
         esc.release_percentages[ o.account ] = o.release_percent;
      });

      _db.release_escrow( escrow );
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain