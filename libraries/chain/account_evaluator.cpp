
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
// === Account Evaluators === //
//============================//


void account_create_evaluator::do_apply( const account_create_operation& o )
{ try {
   const account_name_type& signed_for = o.registrar;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_general(o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
   const account_object* account_ptr = _db.find_account( o.new_account_name );  // Ensure account name is not already in use.

   FC_ASSERT( account_ptr == nullptr,
      "Account with the name: ${n} already exists.", ("n", o.new_account_name) );
   
   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   time_point now = _db.head_block_time();
   size_t name_length = o.new_account_name.size();
   asset acc_fee = median_props.account_creation_fee;

   if( is_premium_account_name( o.new_account_name ) )    // Double fee per character less than 8 characters.
   {
      acc_fee.amount = share_type( acc_fee.amount.value << uint16_t( 8 - name_length ) );
   }
   
   FC_ASSERT( o.fee >= asset( acc_fee.amount, SYMBOL_COIN ), 
      "Insufficient Fee: ${f} required, ${p} provided.", ("f", acc_fee )("p", o.fee) );
   const auto& registrar_balance = _db.get_account_balance( o.registrar, SYMBOL_COIN );
   FC_ASSERT( registrar_balance.get_liquid_balance() >= o.fee, 
      "Insufficient balance to create account.", ( "registrar balance", registrar_balance.liquid_balance )( "required", o.fee ) );

   FC_ASSERT( registrar_balance.staked_balance - registrar_balance.delegated_balance - registrar_balance.to_unstake + registrar_balance.total_unstaked >= o.delegation.amount, 
      "Insufficient Stake to delegate to new account.",
      ( "registrar_balance.staked_balance", registrar_balance.staked_balance )
      ( "registrar_balance.delegated_balance", registrar_balance.delegated_balance )( "required", o.delegation ) );

   auto target_delegation = asset( acc_fee.amount * CREATE_ACCOUNT_DELEGATION_RATIO, SYMBOL_COIN );
   auto current_delegation = asset( o.fee.amount * CREATE_ACCOUNT_DELEGATION_RATIO, SYMBOL_COIN ) + o.delegation;

   FC_ASSERT( current_delegation >= target_delegation, "Insufficient Delegation ${f} required, ${p} provided.",
      ("f", target_delegation )
      ( "p", current_delegation )
      ( "account_creation_fee", acc_fee )
      ( "o.fee", o.fee )
      ( "o.delegation", o.delegation ) );
   
   const account_object& registrar = _db.get_account( o.registrar );   // Ensure all referenced accounts exist
   FC_ASSERT( registrar.active,
      "Account: ${s} is not active, please select a different registrar account.",("s", o.registrar) );

   if( o.recovery_account.size() )
   {
      const account_object& recovery_account = _db.get_account( o.recovery_account );
      FC_ASSERT( recovery_account.active,
         "Account: ${s} is not active, please select a different recovery account.",("s", o.recovery_account) );
   }
   if( o.reset_account.size() )
   {
      const account_object& reset_account = _db.get_account( o.reset_account );
      FC_ASSERT( reset_account.active,
         "Account: ${s} is not active, please select a different reset account.",("s", o.reset_account) );
   }
   if( o.referrer.size() )
   {
      const account_object& referrer = _db.get_account( o.referrer );
      FC_ASSERT( referrer.active,
         "Account: ${s} is not active, please select a different referrer account.",("s", o.referrer) );
   }
   if( o.proxy.size() )
   {
      const account_object& proxy = _db.get_account( o.proxy );
      FC_ASSERT( proxy.active,
         "Account: ${s} is not active, please select a different proxy account.",("s", o.proxy) );
   }
   if( o.governance_account.size() )
   {
      const governance_account_object& governance_account = _db.get_governance_account( o.governance_account );
      FC_ASSERT( governance_account.active,
         "Governance account: ${a} is not active, please select a different governance account.",("a", o.governance_account) );
   }
   
   for( auto& a : o.owner_auth.account_auths )
   {
      _db.get_account( a.first );
   }

   for( auto& a : o.active_auth.account_auths )
   {
      _db.get_account( a.first );
   }

   for( auto& a : o.posting_auth.account_auths )
   {
      _db.get_account( a.first );
   }
   
   _db.adjust_liquid_balance( registrar.name, -o.fee );
   _db.adjust_delegated_balance( registrar.name, o.delegation );

   const account_object& new_account = _db.create< account_object >( [&]( account_object& a )
   {
      a.name = o.new_account_name;
      a.registrar = registrar.name;

      if( o.recovery_account.size() )
      {
         a.recovery_account = o.recovery_account;
      }
      if( o.reset_account.size() )
      {
         a.reset_account = o.reset_account;
      }
      if( o.referrer.size() )
      {
         a.referrer = o.referrer;
      }
      if( o.proxy.size() )
      {
         a.proxy = o.proxy;
      }

      from_string( a.json, o.json );
      from_string( a.json_private, o.json_private );
      from_string( a.details, o.details );
      from_string( a.url, o.url );
      from_string( a.image, o.image );

      a.membership = membership_tier_type::NONE;

      a.secure_public_key = public_key_type( o.secure_public_key );
      a.connection_public_key = public_key_type( o.connection_public_key );
      a.friend_public_key = public_key_type( o.friend_public_key );
      a.companion_public_key = public_key_type( o.companion_public_key );

      a.created = now;
      a.last_account_update = now;
      a.last_vote_time = now;
      a.last_post = now;
      a.last_root_post = now;
      a.last_transfer_time = now;
      a.last_activity_reward = now;
      a.last_account_recovery = now;
      a.last_community_created = now;
      a.last_asset_created = now;
      a.membership_expiration = time_point::min();
      a.mined = false;
   });

   if( o.fee.amount > 0 )
   {
      _db.adjust_staked_balance( o.new_account_name, o.fee );
   }
   if( o.delegation.amount > 0 )
   {
      _db.adjust_receiving_balance( o.new_account_name, o.delegation );
   }
   
   _db.create< asset_delegation_object >( [&]( asset_delegation_object& vdo )
   {
      vdo.delegator = o.registrar;
      vdo.delegatee = o.new_account_name;
      vdo.amount = o.delegation;
   });
   
   _db.create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = o.new_account_name;
      auth.owner_auth = o.owner_auth;
      auth.active_auth = o.active_auth;
      auth.posting_auth = o.posting_auth;
      auth.last_owner_update = now;
   });

   _db.create< account_following_object >( [&]( account_following_object& afo )
   {
      afo.account = o.new_account_name;
      afo.last_updated = now;
   }); 

   if( o.governance_account.size() )
   {
      _db.create< governance_subscription_object >( [&]( governance_subscription_object& gso )
      {
         gso.governance_account = o.governance_account;
         gso.account = o.new_account_name;
      });

      _db.modify( new_account, [&]( account_object& a )
      {
         a.governance_subscriptions++;
      });
   }

   if( _db.find_network_officer( o.registrar ) != nullptr )
   {
      _db.create< network_officer_vote_object >( [&]( network_officer_vote_object& novo )
      {
         novo.network_officer = o.registrar;
         novo.account = o.new_account_name;
      });

      _db.modify( new_account, [&]( account_object& a )
      {
         a.officer_vote_count++;
      });
   }

   if( _db.find_executive_board( o.registrar ) != nullptr )
   {
      _db.create< executive_board_vote_object >( [&]( executive_board_vote_object& ebvo )
      {
         ebvo.executive_board = o.registrar;
         ebvo.account = o.new_account_name;
      });

      _db.modify( new_account, [&]( account_object& a )
      {
         a.officer_vote_count++;
      });
   }

   if( _db.find_producer( o.registrar ) != nullptr )
   {
      _db.create< producer_vote_object >( [&]( producer_vote_object& wvo )
      {
         wvo.producer = o.registrar;
         wvo.account = o.new_account_name;
         wvo.vote_rank = 1;
      });

      _db.modify( new_account, [&]( account_object& a )
      {
         a.officer_vote_count++;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_update_evaluator::do_apply( const account_update_operation& o )
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
      FC_ASSERT( b.is_chief( o.signatory ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();
   const account_object& account = _db.get_account( o.account );
   const account_authority_object& account_auth = _db.get< account_authority_object, by_account >( o.account );
   
   FC_ASSERT( now - account_auth.last_owner_update > OWNER_UPDATE_LIMIT,
      "Owner authority can only be updated once an hour." );

   for( auto a: o.owner_auth.account_auths )
   {
      _db.get_account( a.first );
   }
   
   _db.update_owner_authority( account, o.owner_auth );

   for( auto a: o.active_auth.account_auths )
   {
      _db.get_account( a.first );
   }

   for( auto a: o.posting_auth.account_auths )
   {
      _db.get_account( a.first );
   }
   
   const comment_object* pinned_post_ptr = nullptr;
   if( o.pinned_permlink.size() )
   {
      pinned_post_ptr = _db.find_comment( o.account, o.pinned_permlink );
      FC_ASSERT( pinned_post_ptr != nullptr,
         "Cannot find valid Pinned Post." );
      FC_ASSERT( pinned_post_ptr->root == true,
         "Pinned post must be a root comment." );
   }

   _db.modify( account, [&]( account_object& a )
   {
      if( o.secure_public_key.size() )
      {
         a.secure_public_key = public_key_type( o.secure_public_key );
      }
      if( o.connection_public_key.size() )
      {
         a.connection_public_key = public_key_type( o.connection_public_key );
      }
      if( o.friend_public_key.size() )
      {
         a.friend_public_key = public_key_type( o.friend_public_key );
      }
      if( o.companion_public_key.size() )
      {
         a.companion_public_key = public_key_type( o.companion_public_key );
      }

      a.last_account_update = now;

      if ( o.json.size() > 0 )
      {
         from_string( a.json, o.json );
      }
      if ( o.details.size() > 0 )
      {
         from_string( a.details, o.details );
      }
      if ( o.url.size() > 0 )
      {
         from_string( a.url, o.url );
      }
      if ( o.json_private.size() > 0 )
      {
         from_string( a.json_private, o.json_private );
      }
      if( pinned_post_ptr != nullptr )
      {
         a.pinned_post = pinned_post_ptr->id;
      }
      a.active = o.active;
   });

   _db.modify( account_auth, [&]( account_authority_object& auth )
   {
      auth.active_auth = o.active_auth;
      auth.posting_auth = o.posting_auth;
   });
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_profile_evaluator::do_apply( const account_profile_operation& o )
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
      FC_ASSERT( b.is_chief( o.signatory ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();
   const account_object& gov_account = _db.get_account( o.governance_account );
   FC_ASSERT( gov_account.active, 
      "Governance Account: ${s} must be active.",("s", o.governance_account) );

   const auto& profile_idx = _db.get_index< account_profile_index >().indices().get< by_account >();
   auto profile_itr = profile_idx.find( o.account );

   if( profile_itr == profile_idx.end() )    // New profile 
   {
      _db.create< account_profile_object >( [&]( account_profile_object& apo )
      {
         apo.account = o.account;
         apo.governance_account = o.governance_account;
         apo.profile_public_key = public_key_type( o.profile_public_key );
         from_string( apo.first_name, o.first_name );
         from_string( apo.last_name, o.last_name );
         from_string( apo.gender, o.gender );
         from_string( apo.date_of_birth, o.date_of_birth );
         from_string( apo.email, o.email );
         from_string( apo.phone, o.phone );
         from_string( apo.nationality, o.nationality );
         from_string( apo.address, o.address );
         apo.last_updated = now;
         apo.created = now;
      });
   }
   else
   {
      const account_profile_object& profile = *profile_itr;

      _db.modify( profile, [&]( account_profile_object& apo )
      {
         apo.governance_account = o.governance_account;
         apo.profile_public_key = public_key_type( o.profile_public_key );
         from_string( apo.first_name, o.first_name );
         from_string( apo.last_name, o.last_name );
         from_string( apo.gender, o.gender );
         from_string( apo.date_of_birth, o.date_of_birth );
         from_string( apo.email, o.email );
         from_string( apo.phone, o.phone );
         from_string( apo.nationality, o.nationality );
         from_string( apo.address, o.address );
         apo.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_verification_evaluator::do_apply( const account_verification_operation& o )
{ try {
   const account_name_type& signed_for = o.verifier_account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_chief( o.signatory ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();
   const account_object& verified_account = _db.get_account( o.verified_account );
   FC_ASSERT( verified_account.active, 
      "Account: ${s} must be active.",("s", o.verified_account) );

   const account_profile_object& profile = _db.get_account_profile( o.verified_account );

   const auto& verification_idx = _db.get_index< account_verification_index >().indices().get< by_verifier_verified >();
   auto verification_itr = verification_idx.find( boost::make_tuple( o.verifier_account, o.verified_account ) );

   digest_type::encoder enc;
   fc::raw::pack( enc, o.shared_image );

   public_key_type recovered_pub_key = fc::ecc::public_key( o.image_signature, enc.result() );

   FC_ASSERT( profile.profile_public_key == recovered_pub_key,
      "Shared Image must be signed with the profile key of the Verified account." );

   if( verification_itr == verification_idx.end() )
   {
      _db.create< account_verification_object >( [&]( account_verification_object& avo )
      {
         avo.verifier_account = o.verifier_account;
         avo.verified_account = o.verified_account;
         avo.verified_profile_public_key = profile.profile_public_key;
         from_string( avo.shared_image, o.shared_image );
         avo.image_signature = o.image_signature;
         avo.last_updated = now;
         avo.created = now;
      });
   }
   else
   {
      const account_verification_object& verification = *verification_itr;

      _db.modify( verification, [&]( account_verification_object& avo )
      {
         avo.verified_profile_public_key = profile.profile_public_key;
         from_string( avo.shared_image, o.shared_image );
         avo.image_signature = o.image_signature;
         avo.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_business_evaluator::do_apply( const account_business_operation& o )
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
      FC_ASSERT( b.is_chief( o.signatory ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();
   const account_object& gov_account = _db.get_account( o.governance_account );
   FC_ASSERT( gov_account.active, 
      "Governance Account: ${s} must be active.",("s", o.governance_account) );

   business_structure_type business_structure = business_structure_type::PUBLIC_BUSINESS;

   for( size_t i = 0; i < business_structure_values.size(); i++ )
   {
      if( o.business_type == business_structure_values[ i ] )
      {
         business_structure = business_structure_type( i );
         break;
      }
   }

   const auto& business_idx = _db.get_index< account_business_index >().indices().get< by_account >();
   auto business_itr = business_idx.find( o.account );

   if( business_itr == business_idx.end() )    // New business 
   {
      const account_business_object& business = _db.create< account_business_object >( [&]( account_business_object& abo )
      {
         abo.account = o.account;
         abo.governance_account = o.governance_account;
         abo.business_type = business_structure;
         abo.business_public_key = public_key_type( o.business_public_key );
         abo.executive_board.CHIEF_EXECUTIVE_OFFICER = o.init_ceo_account;
         abo.officer_vote_threshold = o.officer_vote_threshold;
         abo.last_updated = now;
         abo.created = now;
      });

      _db.create< account_officer_vote_object >( [&]( account_officer_vote_object& aovo )
      {
         aovo.account = o.init_ceo_account;
         aovo.business_account = o.account;
         aovo.officer_account = o.init_ceo_account;
      });

      _db.create< account_executive_vote_object >( [&]( account_executive_vote_object& aevo )
      {
         aevo.account = o.init_ceo_account;
         aevo.business_account = o.account;
         aevo.executive_account = o.init_ceo_account;
         aevo.role = executive_role_type::CHIEF_EXECUTIVE_OFFICER;
      });

      _db.update_business_account( business );
   }
   else
   {
      const account_business_object& business = *business_itr;

      _db.modify( business, [&]( account_business_object& abo )
      {
         abo.governance_account = o.governance_account;
         abo.business_public_key = public_key_type( o.business_public_key );
         abo.officer_vote_threshold = o.officer_vote_threshold;
         abo.last_updated = now;
      });

      _db.update_business_account( business );
   }

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_membership_evaluator::do_apply( const account_membership_operation& o )
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
   const account_object* int_account_ptr = nullptr;

   if( o.interface.size() )
   {
      int_account_ptr = _db.find_account( o.interface );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }
   
   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   time_point now = _db.head_block_time();
   asset liquid = _db.get_liquid_balance( o.account, SYMBOL_COIN );
   asset monthly_fee = asset( 0, SYMBOL_USD );

   membership_tier_type mem_tier = membership_tier_type::STANDARD_MEMBERSHIP;

   for( size_t i = 0; i < membership_tier_values.size(); i++ )
   {
      if( o.membership_type == membership_tier_values[ i ] )
      {
         mem_tier = membership_tier_type( i );
         break;
      }
   }

   switch( mem_tier )
   {
      case membership_tier_type::NONE:
      {
         
      }
      break; 
      case membership_tier_type::STANDARD_MEMBERSHIP:
      {
         monthly_fee = median_props.membership_base_price;
      }
      break;
      case membership_tier_type::MID_MEMBERSHIP:
      {
         monthly_fee = median_props.membership_mid_price;
      }
      break; 
      case membership_tier_type::TOP_MEMBERSHIP:
      {
         monthly_fee = median_props.membership_top_price;
      }
      break;
      default:
      {
         FC_ASSERT( false, 
            "Membership type Invalid: ${m}.", ("m", o.membership_type ) ); 
      }
      break;
   }

   asset carried_fees = asset( 0, SYMBOL_USD );
   fc::microseconds remaining = account.membership_expiration - now;

   switch( account.membership )
   {
      case membership_tier_type::NONE:
      {
          
      }
      break;
      case membership_tier_type::STANDARD_MEMBERSHIP:
      {
         carried_fees = asset( ( median_props.membership_base_price.amount * remaining.count() ) / fc::days( 30 ).count(), SYMBOL_USD );
      }
      break;
      case membership_tier_type::MID_MEMBERSHIP:
      {
         carried_fees = asset( ( median_props.membership_mid_price.amount * remaining.count() ) / fc::days( 30 ).count(), SYMBOL_USD );
      }
      break;
      case membership_tier_type::TOP_MEMBERSHIP:
      {
         carried_fees = asset( ( median_props.membership_top_price.amount * remaining.count() ) / fc::days( 30 ).count(), SYMBOL_USD );
      }
      break;
      default:
      {
         FC_ASSERT( false, "Membership type Invalid: ${m}.", ("m", account.membership ) );
      }
      break;
   }

   asset total_fees = std::max( asset( 0, SYMBOL_USD ), monthly_fee * o.months - carried_fees);   // Applies a discount on new membership if an existing membership is still active.

   FC_ASSERT( liquid >= total_fees, 
      "Account has insufficent liquid balance to pay for the requested membership duration." );

   if( total_fees.amount > 0 )
   {
      if( int_account_ptr != nullptr )
      {
         _db.pay_membership_fees( account, total_fees, *int_account_ptr );      // Pays splits to interface, premium partners, and network
      }
      else
      {
         _db.pay_membership_fees( account, total_fees );
      }
   }

   _db.modify( account, [&]( account_object& a )
   {
      a.membership = mem_tier;
      a.membership_expiration = now + fc::days( 30 * o.months );
      if( o.interface.size() )
      {
         a.membership_interface = o.interface;
      }
      if( o.recurring )
      {
         a.recurring_membership = o.months;
      }
      else
      {
         a.recurring_membership = 0;
      } 
   });
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_vote_executive_evaluator::do_apply( const account_vote_executive_operation& o )
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
      FC_ASSERT( b.is_authorized_vote_executive( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& voter = _db.get_account( o.account );
   const account_object& executive = _db.get_account( o.executive_account );
   FC_ASSERT( executive.active, 
      "Account: ${s} must be active to be voted for.",("s", o.executive_account) );
   const account_object& business = _db.get_account( o.business_account );
   FC_ASSERT( business.active, 
      "Account: ${s} must be active to accept executive votes.",("s", o.business_account) );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );
   share_type voting_power = _db.get_equity_voting_power( o.account, bus_acc );

   FC_ASSERT( voting_power > 0,
      "Account must hold a balance of voting power in the equity assets of the business account in order to vote for executives." );

   if( o.approved )
   {
      FC_ASSERT( voter.can_vote,
         "Account has declined its voting rights." );
      FC_ASSERT( bus_acc.is_authorized_vote_executive( voter.name, _db.get_account_permissions( o.business_account ) ),
         "Account: ${a} is not authorized to vote for an Officer in business: ${b} .", ("a", o.account)("b", o.business_account));
      FC_ASSERT( bus_acc.is_officer( executive.name ),
         "Account: ${a} must be an officer of business: ${b} before being voted as Executive.",
         ("a", o.executive_account)("b", o.business_account) );
   }

   executive_role_type exec_role = executive_role_type::CHIEF_EXECUTIVE_OFFICER;

   for( size_t i = 0; i < executive_role_values.size(); i++ )
   {
      if( o.role == executive_role_values[ i ] )
      {
         exec_role = executive_role_type( i );
         break;
      }
   }
   
   const auto& rank_idx = _db.get_index< account_executive_vote_index >().indices().get< by_account_business_role_rank >();
   const auto& executive_idx = _db.get_index< account_executive_vote_index >().indices().get< by_account_business_role_executive >();
   auto rank_itr = rank_idx.find( boost::make_tuple( voter.name, o.business_account, exec_role, o.vote_rank ) ); 
   auto executive_itr = executive_idx.find( boost::make_tuple( voter.name, o.business_account, exec_role, o.executive_account ) );

   if( o.approved ) // Adding or modifying vote
   {
      if( executive_itr == executive_idx.end() && rank_itr == rank_idx.end() ) // No vote for executive board or type rank, create new vote.
      {
         _db.create< account_executive_vote_object>( [&]( account_executive_vote_object& v )
         {
            v.account = voter.name;
            v.vote_rank = o.vote_rank;
            v.executive_account = o.executive_account;
            v.role = exec_role;
         });
         
         _db.update_account_executive_votes( voter, o.business_account );
      }
      else
      {
         if( executive_itr != executive_idx.end() && rank_itr != rank_idx.end() )
         {
            FC_ASSERT( executive_itr->executive_account != rank_itr->executive_account,
               "Vote at for role at selected rank is already specified executive account." );
         }
         
         if( executive_itr != executive_idx.end() )
         {
            _db.remove( *executive_itr );
         }

         _db.update_account_executive_votes( voter, o.business_account, executive, exec_role, o.vote_rank );
      }
   }
   else  // Removing existing vote
   {
      if( executive_itr != executive_idx.end() )
      {
         _db.remove( *executive_itr );
      }
      else if( rank_itr != rank_idx.end() )
      {
         _db.remove( *rank_itr );
      }
      _db.update_account_executive_votes( voter, o.business_account );
   }

   _db.update_business_account( bus_acc );   // updates the voting status of the business account to reflect all voting changes.

} FC_CAPTURE_AND_RETHROW( ( o )) }


void account_vote_officer_evaluator::do_apply( const account_vote_officer_operation& o )
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
      FC_ASSERT( b.is_authorized_vote_officer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& voter = _db.get_account( o.account );
   const account_object& officer = _db.get_account( o.officer_account );
   FC_ASSERT( officer.active, 
      "Account: ${s} must be active to be voted for.",("s", o.officer_account) );
   const account_object& business = _db.get_account( o.business_account );
   FC_ASSERT( business.active, 
      "Account: ${s} must be active to accept officer votes.",("s", o.business_account) );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );
   share_type voting_power = _db.get_equity_voting_power( o.account, bus_acc );

   FC_ASSERT( voting_power > 0,
      "Account must hold a balance of voting power in the equity assets of the business account in order to vote for officers." );

   if( o.approved )
   {
      FC_ASSERT( voter.can_vote,
         "Account has declined its voting rights." );
      FC_ASSERT( bus_acc.is_authorized_vote_officer( voter.name, _db.get_account_permissions( o.business_account ) ),
         "Account: ${a} is not authorized to vote for an Officer in business: ${b} .", ("a", o.account)("b", o.business_account));
      FC_ASSERT( bus_acc.is_member( officer.name ),
         "Account: ${a} must be an officer of business: ${b} before being voted as Officer.", ("a", o.officer_account)("b", o.business_account));
   }

   const auto& rank_idx = _db.get_index< account_officer_vote_index >().indices().get< by_account_business_rank >();
   const auto& officer_idx = _db.get_index< account_officer_vote_index >().indices().get< by_account_business_officer >();
   auto rank_itr = rank_idx.find( boost::make_tuple( voter.name, o.business_account, o.vote_rank ) );
   auto officer_itr = officer_idx.find( boost::make_tuple( voter.name, o.business_account, o.officer_account ) );

   if( o.approved )       // Adding or modifying vote
   {
      if( officer_itr == officer_idx.end() && rank_itr == rank_idx.end() ) // No vote for officer or type rank, create new vote.
      {
         _db.create< account_officer_vote_object>( [&]( account_officer_vote_object& v )
         {
            v.account = voter.name;
            v.vote_rank = o.vote_rank;
            v.officer_account = o.officer_account;
         });
         
         _db.update_account_officer_votes( voter, o.business_account );
      }
      else
      {
         if( officer_itr != officer_idx.end() && rank_itr != rank_idx.end() )
         {
            FC_ASSERT( officer_itr->officer_account != rank_itr->officer_account,
               "Vote at for role at selected rank is already specified officer account." );
         }
         
         if( officer_itr != officer_idx.end() )
         {
            _db.remove( *officer_itr );
         }

         _db.update_account_officer_votes( voter, o.business_account, officer, o.vote_rank );
      }
   }
   else  // Removing existing vote
   {
      if( officer_itr != officer_idx.end() )
      {
         _db.remove( *officer_itr );
      }
      else if( rank_itr != rank_idx.end() )
      {
         _db.remove( *rank_itr );
      }
      _db.update_account_officer_votes( voter, o.business_account );
   }

   _db.update_business_account( bus_acc );   // updates the voting status of the business account to reflect all voting changes.

} FC_CAPTURE_AND_RETHROW( ( o )) }


void account_member_request_evaluator::do_apply( const account_member_request_operation& o )
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
      FC_ASSERT( b.is_authorized_request( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
   const account_object& account = _db.get_account( o.account );
   const account_object& business = _db.get_account( o.business_account );
   FC_ASSERT( business.active, 
      "Account: ${s} must be active to accept member requests.",("s", o.business_account) );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );

   FC_ASSERT( !bus_acc.is_member( account.name ), 
      "Account: ${a} is already a member of the business: ${b}.", ("a", o.account)("b", o.business_account)); 
   FC_ASSERT( bus_acc.is_authorized_request( account.name, _db.get_account_permissions( o.business_account ) ), 
      "Account: ${a} is not authorised to request to join the business account: ${b}.", ("a", o.account)("b", o.business_account) );
   
   time_point now = _db.head_block_time();
   const auto& req_idx = _db.get_index< account_member_request_index >().indices().get< by_account_business >();
   auto itr = req_idx.find( boost::make_tuple( o.account, o.business_account ) );

   if( itr == req_idx.end())    // Request does not exist yet
   {
      FC_ASSERT( o.requested,
         "Account membership request does not exist, requested should be set to true." );

      _db.create< account_member_request_object >( [&]( account_member_request_object& amro )
      {
         amro.account = account.name;
         amro.business_account = o.business_account;
         from_string( amro.message, o.message );
         amro.expiration = now + CONNECTION_REQUEST_DURATION;
      });
   }
   else     // Request exists and is being deleted
   {
      FC_ASSERT( !o.requested,
         "Request already exists, Requested should be set to false to remove existing request." );
      _db.remove( *itr );
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void account_member_invite_evaluator::do_apply( const account_member_invite_operation& o )
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
      FC_ASSERT( b.is_authorized_invite( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
   const account_object& account = _db.get_account( o.account );
   const account_object& member = _db.get_account( o.member);
   FC_ASSERT( member.active, 
      "Account: ${s} must be active to accept member invites.",("s", o.member) );
   const account_object& business = _db.get_account( o.business_account );
   FC_ASSERT( business.active, 
      "Account: ${s} must be active to accept member invites.",("s", o.business_account) );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );

   FC_ASSERT( !bus_acc.is_member( member.name ),
      "Account: ${a} is already a member of the Business Account: ${b}.", ("a", o.member)("b", o.business_account) );
   FC_ASSERT( bus_acc.is_authorized_invite( account.name, _db.get_account_permissions( o.business_account ) ),
      "Account: ${a} is not authorised to send Business account: ${b} membership invitations.", ("a", o.account)("b", o.business_account) );
   
   time_point now = _db.head_block_time();
   const auto& inv_idx = _db.get_index< account_member_invite_index >().indices().get< by_member_business >();
   const auto& key_idx = _db.get_index< account_member_key_index >().indices().get< by_member_business >();
   auto itr = inv_idx.find( boost::make_tuple( o.member, o.business_account ) );

   if( itr == inv_idx.end() )    // Invite does not exist yet
   {
      FC_ASSERT( o.invited,
         "Business Account invite request does not exist, invited should be set to true." );

      _db.create< account_member_invite_object >( [&]( account_member_invite_object& amio )
      {
         amio.account = account.name;
         amio.member = member.name;
         amio.business_account = o.business_account;
         from_string( amio.message, o.message );
         amio.expiration = now + CONNECTION_REQUEST_DURATION;
      });

      _db.create< account_member_key_object >( [&]( account_member_key_object& amko )
      {
         amko.account = account.name;
         amko.member = member.name;
         amko.business_account = o.business_account;
         amko.encrypted_business_key = encrypted_keypair_type( member.secure_public_key, bus_acc.business_public_key, o.encrypted_business_key );
      });
   }
   else     // Invite exists and is being deleted.
   {
      FC_ASSERT( !o.invited,
         "Invite already exists, Invited should be set to false to remove existing Invitation." );
      auto key_itr = key_idx.find( std::make_tuple( o.member, o.business_account ) );
      if( key_itr != key_idx.end() )
      {
         _db.remove( *key_itr );  // Remove the account key 
      }
      _db.remove( *itr );     // Remove the invitation
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void account_accept_request_evaluator::do_apply( const account_accept_request_operation& o )
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
      FC_ASSERT( b.is_authorized_invite( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account ); 
   const account_object& member = _db.get_account( o.member);
   FC_ASSERT( member.active, 
      "Account: ${s} must be active to accept member requests.",("s", o.member) );
   const account_object& business = _db.get_account( o.business_account );
   FC_ASSERT( business.active, 
      "Account: ${s} must be active to accept member requests.",("s", o.business_account) );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );

   FC_ASSERT( !bus_acc.is_member( member.name ),
      "Account: ${a} is already a member of the business account: ${b}.", ("a", o.member)("b", o.business_account) );
   FC_ASSERT( bus_acc.is_authorized_invite( account.name, _db.get_account_permissions( o.business_account ) ),
      "Account: ${a} is not authorized to accept membership requests to the business account: ${b}.", ("a", o.account)("b", o.business_account ) );

   const auto& req_idx = _db.get_index< account_member_request_index >().indices().get< by_account_business >();
   auto itr = req_idx.find( boost::make_tuple( o.member, o.business_account ) );

   FC_ASSERT( itr != req_idx.end(),
      "Business account membership request does not exist." );    // Ensure Request exists

   if( o.accepted )   // Accepting the request, skipped if rejecting
   {
      _db.modify( bus_acc, [&]( account_business_object& abo )
      {
         abo.members.insert( member.name );
      });

      _db.create< account_member_key_object >( [&]( account_member_key_object& amko )
      {
         amko.account = account.name;
         amko.member = member.name;
         amko.business_account = o.business_account;
         amko.encrypted_business_key = encrypted_keypair_type( member.secure_public_key, bus_acc.business_public_key, o.encrypted_business_key );
      });
   }

   _db.remove( *itr );

} FC_CAPTURE_AND_RETHROW( ( o )) }


void account_accept_invite_evaluator::do_apply( const account_accept_invite_operation& o )
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
   const account_object& business = _db.get_account( o.business_account );
   FC_ASSERT( business.active, 
      "Account: ${s} must be active to accept member invites",("s", o.business_account) );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );

   FC_ASSERT( !bus_acc.is_member( account.name ),
      "Account: ${a} is already a member of the business account: ${b}.", ("a", o.account)("b", o.business_account));

   const auto& inv_idx = _db.get_index< account_member_invite_index >().indices().get< by_member_business >();
   auto itr = inv_idx.find( std::make_tuple( o.account, o.business_account ) );
   
   FC_ASSERT( itr != inv_idx.end(),
      "Business account membership invitation does not exist." );     // Ensure Invitation exists

   const account_member_invite_object& invite = *itr;

   FC_ASSERT( bus_acc.is_authorized_invite( invite.account, _db.get_account_permissions( o.business_account ) ),
      "Account: ${a} is no longer authorised to send business account: ${b} membership invitations.", 
      ("a", invite.account)("b", o.business_account));     // Ensure inviting account is still authorised to send invitations
   
   const auto& key_idx = _db.get_index< account_member_key_index >().indices().get< by_member_business >();
   auto key_itr = key_idx.find( std::make_tuple( invite.member, o.business_account ) );

   FC_ASSERT( key_itr != key_idx.end(),
      "Business account key for invited account does not exist." );

   if( o.accepted )   // Accepting the request, skipped if rejecting
   {
      _db.modify( bus_acc , [&]( account_business_object& abo )
      {
         abo.members.insert( invite.member );
      });
   }

   _db.remove( invite );

} FC_CAPTURE_AND_RETHROW( ( o )) }


void account_remove_member_evaluator::do_apply( const account_remove_member_operation& o )
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
      FC_ASSERT( b.is_authorized_blacklist( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account ); 
   const account_object& member_acc = _db.get_account( o.member );
   FC_ASSERT( member_acc.active, 
      "Account: ${s} must be active to accept member requests.",("s", o.member) );
   const account_object& business = _db.get_account( o.business_account );
   FC_ASSERT( business.active, 
      "Account: ${s} must be active to remove members.",("s", o.business_account) );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );

   FC_ASSERT( bus_acc.is_member( member_acc.name ), 
      "Account: ${a} is not a member of business: ${b}.", ("a", o.member)("b", o.business_account) );
   FC_ASSERT( !bus_acc.is_executive( member_acc.name ), 
      "Account: ${a} cannot be removed while an executive of business: ${b}.", ("a", o.member)("b", o.business_account) );

   if( account.name != member_acc.name )     // Account can remove itself from  membership.  
   {
      FC_ASSERT( bus_acc.is_authorized_blacklist( o.account, _db.get_account_permissions( o.business_account ) ), 
         "Account: ${a} is not authorised to remove accounts from the business: ${b}.", ("a", o.account)("b", o.business_account)); 
   }

   const auto& key_idx = _db.get_index< account_member_key_index >().indices().get< by_member_business >();
   auto key_itr = key_idx.find( std::make_tuple( o.member, o.business_account ) );
   
   _db.modify( bus_acc, [&]( account_business_object& abo )
   {
      abo.members.erase( member_acc.name );
      abo.officers.erase( member_acc.name );
   });
   if( key_itr != key_idx.end() )
   {
      const account_member_key_object& key = *key_itr;
      _db.remove( key );
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void account_update_list_evaluator::do_apply( const account_update_list_operation& o )
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
      FC_ASSERT( b.is_authorized_blacklist( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
   account_name_type account_name;
   asset_symbol_type asset_sym;

   if( o.listed_account.valid() )
   {
      FC_ASSERT( o.account != *o.listed_account,
         "Account: ${a} cannot add or remove itself from its own blacklist or whitelist." );
         account_name = *o.listed_account;
   }
   if( o.listed_asset.valid() )
   {
      const asset_object& asset_obj = _db.get_asset( *o.listed_asset );
      FC_ASSERT( asset_obj.issuer != o.account,
         "Account: ${a} cannot add an asset it is the issuer of to its own blacklist or whitelist." );
      asset_sym = *o.listed_asset;
   }

   const account_permission_object& perm = _db.get_account_permissions( o.account );
   const account_business_object* bus_acc_ptr = _db.find_account_business( o.account );

   if( bus_acc_ptr != nullptr )
   {
      FC_ASSERT( !bus_acc_ptr->is_member( account_name ),
         "Account: ${a} cannot be blacklisted while a member of business: ${b}. Remove them first.", ("a", account_name)("b", o.account));
      FC_ASSERT( !bus_acc_ptr->is_officer( account_name ),
         "Account: ${a} cannot be blacklisted while a officer of business: ${b}. Remove them first.", ("a", account_name)("b", o.account));
      FC_ASSERT( !bus_acc_ptr->is_executive( account_name ),
         "Account: ${a} cannot be blacklisted while an executive of business: ${b}. Remove them first.", ("a", account_name)("b", o.account));
   }
   
   _db.modify( perm, [&]( account_permission_object& apo )
   {
      if( o.blacklisted )
      {
         if( account_name.size() )
         {
            apo.blacklisted_accounts.insert( account_name );
            apo.whitelisted_accounts.erase( account_name );
         }
         if( asset_sym.size() )
         {
            apo.blacklisted_assets.insert( asset_sym );
            apo.whitelisted_assets.erase( asset_sym );
         }
      }
      else if( o.whitelisted )
      {
         if( account_name.size() )
         {
            apo.whitelisted_accounts.insert( account_name );
            apo.blacklisted_accounts.erase( account_name );
         }
         if( asset_sym.size() )
         {
            apo.whitelisted_assets.insert( asset_sym );
            apo.blacklisted_assets.erase( asset_sym );
         }
      }
      else
      {
         if( account_name.size() )
         {
            apo.whitelisted_accounts.erase( account_name );
            apo.blacklisted_accounts.erase( account_name );
         }
         if( asset_sym.size() )
         {
            apo.whitelisted_assets.erase( asset_sym );
            apo.blacklisted_assets.erase( asset_sym );
         }
      }
   });
} FC_CAPTURE_AND_RETHROW( ( o )) }


void account_producer_vote_evaluator::do_apply( const account_producer_vote_operation& o )
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
   const account_object& voter = _db.get_account( o.account );
   const producer_object& producer = _db.get_producer( o.producer );

   FC_ASSERT( voter.proxy.size() == 0,
      "A proxy is currently set, please clear the proxy before voting for a producer." );

   if( o.approved )
   {
      FC_ASSERT( voter.can_vote,
         "Account has declined its voting rights." );
      FC_ASSERT( producer.active,
         "Producer is inactive, and not accepting approval votes at this time." );
   }

   const auto& account_rank_idx = _db.get_index< producer_vote_index >().indices().get< by_account_rank >();
   const auto& account_producer_idx = _db.get_index< producer_vote_index >().indices().get< by_account_producer >();
   auto rank_itr = account_rank_idx.find( boost::make_tuple( voter.name, o.vote_rank ) );   // vote at rank number
   auto producer_itr = account_producer_idx.find( boost::make_tuple( voter.name, producer.owner ) );    // vote for specified producer

   if( o.approved )       // Adding or modifying vote
   {
      if( producer_itr == account_producer_idx.end() && rank_itr == account_rank_idx.end() )    // No vote for producer or rank
      {
         FC_ASSERT( voter.producer_vote_count < MAX_ACCOUNT_VOTES,
            "Account has voted for too many producers." );

         _db.create< producer_vote_object >( [&]( producer_vote_object& v )
         {
            v.producer = producer.owner;
            v.account = voter.name;
            v.vote_rank = o.vote_rank;
         });
         
         _db.update_producer_votes( voter );
      }
      else
      {
         if( producer_itr != account_producer_idx.end() && rank_itr != account_rank_idx.end() )
         {
            FC_ASSERT( producer_itr->producer != rank_itr->producer,
               "Vote at rank is already specified producer." );
         }
         
         if( producer_itr != account_producer_idx.end() )
         {
            _db.remove( *producer_itr );
         }

         _db.update_producer_votes( voter, o.producer, o.vote_rank );   // Remove existing producer vote, and add at new rank.
      }
   }
   else  // Removing existing vote
   {
      if( producer_itr != account_producer_idx.end() )
      {
         _db.remove( *producer_itr );
      }
      else if( rank_itr != account_rank_idx.end() )
      {
         _db.remove( *rank_itr );
      }
      _db.update_producer_votes( voter );
   }

   _db.process_update_producer_set();     // Recalculates the voting power for all producers.

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_update_proxy_evaluator::do_apply( const account_update_proxy_operation& o )
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
   
   FC_ASSERT( account.proxy != o.proxy,
      "Proxy must change." );
   FC_ASSERT( account.can_vote,
      "Account has declined the ability to vote and cannot proxy votes." );

   if( o.proxy.size() ) 
   {
      const auto& new_proxy = _db.get_account( o.proxy );
      flat_set< account_id_type > proxy_chain( { account.id, new_proxy.id } );
      proxy_chain.reserve( MAX_PROXY_RECURSION_DEPTH + 1 );

      auto cprox = &new_proxy;
      while( cprox->proxy.size() != 0 )    // check for proxy loops and fail to update the proxy if it would create a loop
      {
         const account_object next_proxy = _db.get_account( cprox->proxy );

         FC_ASSERT( proxy_chain.insert( next_proxy.id ).second,
            "This proxy would create a proxy loop." );
         cprox = &next_proxy;
         FC_ASSERT( proxy_chain.size() <= MAX_PROXY_RECURSION_DEPTH,
            "Proxy chain is too long." );
      }

      _db.clear_network_votes( account );    // clear all individual vote records

      _db.modify( account, [&]( account_object& a )
      {
         a.proxy = o.proxy;
      });

      _db.modify( new_proxy, [&]( account_object& a )
      {
         a.proxied.insert( o.account );
      });
   } 
   else 
   {        
      const account_object& old_proxy = _db.get_account( account.proxy );

      _db.modify( account, [&]( account_object& a )
      {
         a.proxy = o.proxy;      // we are clearing the proxy which means we simply update the accounts
      });

      _db.modify( old_proxy, [&]( account_object& a )
      {
         a.proxied.erase( o.account );       // Remove name from old proxy.
      });
   }

   _db.process_update_producer_set();    // Recalculates the voting power for all producers.

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void request_account_recovery_evaluator::do_apply( const request_account_recovery_operation& o )
{ try {
   const account_name_type& signed_for = o.recovery_account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_executive( o.signatory ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& account_to_recover = _db.get_account( o.account_to_recover );
   time_point now = _db.head_block_time();
   const auto& producer_idx = _db.get_index< producer_index >().indices().get< by_voting_power >();

   if( account_to_recover.recovery_account.length() )   // Make sure recovery matches expected recovery account
   {
      FC_ASSERT( account_to_recover.recovery_account == o.recovery_account,
         "Cannot recover an account that does not specify the given recovery account." );
   }
   else     // Empty string recovery account defaults to top producer
   {
      FC_ASSERT( producer_idx.begin()->owner == o.recovery_account,
         "Top producer must recover an account with no recovery partner." );
   }                                           

   const auto& recovery_request_idx = _db.get_index< account_recovery_request_index >().indices().get< by_account >();
   auto request = recovery_request_idx.find( o.account_to_recover );

   if( request == recovery_request_idx.end() )       // New Request
   {
      FC_ASSERT( !o.new_owner_authority.is_impossible(),
         "Cannot recover using an impossible authority." );
      FC_ASSERT( o.new_owner_authority.weight_threshold,
         "Cannot recover using an open authority." );

      for( auto& a : o.new_owner_authority.account_auths )
      {
         _db.get_account( a.first );      // Check accounts in the new authority exist
      }

      _db.create< account_recovery_request_object >( [&]( account_recovery_request_object& req )
      {
         req.account_to_recover = o.account_to_recover;
         req.new_owner_authority = o.new_owner_authority;
         req.expiration = now + ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD;
      });
   }
   else if( o.new_owner_authority.weight_threshold == 0 ) // Cancel Request if authority is open
   {
      _db.remove( *request );
   }
   else        // Change Request
   {
      FC_ASSERT( !o.new_owner_authority.is_impossible(),
         "Cannot recover using an impossible authority." );

      for( auto& a : o.new_owner_authority.account_auths )
      {
         _db.get_account( a.first );       // Check accounts in the new authority exist
      }
      
      _db.modify( *request, [&]( account_recovery_request_object& req )
      {
         req.new_owner_authority = o.new_owner_authority;
         req.expiration = now + ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void recover_account_evaluator::do_apply( const recover_account_operation& o )
{ try {
   const account_name_type& signed_for = o.account_to_recover;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_executive( o.signatory ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& account = _db.get_account( o.account_to_recover );
   time_point now = _db.head_block_time();

   FC_ASSERT( now - account.last_account_recovery > OWNER_UPDATE_LIMIT,
      "Owner authority can only be updated once an hour." );

   const auto& recovery_request_idx = _db.get_index< account_recovery_request_index >().indices().get< by_account >();
   auto request = recovery_request_idx.find( o.account_to_recover );

   FC_ASSERT( request != recovery_request_idx.end(),
      "There are no active recovery requests for this account." );
   FC_ASSERT( request->new_owner_authority == o.new_owner_authority,
      "New owner authority does not match recovery request." );

   const auto& recent_auth_idx = _db.get_index< owner_authority_history_index >().indices().get< by_account >();
   auto hist = recent_auth_idx.lower_bound( o.account_to_recover );
   bool found = false;

   while( hist != recent_auth_idx.end() && hist->account == o.account_to_recover && !found )
   {
      found = hist->previous_owner_authority == o.recent_owner_authority;
      if( found ) break;
      ++hist;
   }

   FC_ASSERT( found, 
      "Recent authority not found in authority history." );

   _db.remove( *request );     // Remove first, update_owner_authority may invalidate iterator
   _db.update_owner_authority( account, o.new_owner_authority );
   _db.modify( account, [&]( account_object& a )
   {
      a.last_account_recovery = now;
   });
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void reset_account_evaluator::do_apply( const reset_account_operation& o )
{ try {
   const account_name_type& signed_for = o.account_to_reset;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_executive( o.signatory ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account_to_reset );
   const account_object& reset_account = _db.get_account( o.reset_account );
   FC_ASSERT( reset_account.active, 
      "Account: ${s} must be active to reset account.",("s", o.reset_account) );
   fc::microseconds delay = fc::days( account.reset_account_delay_days );
   time_point now = _db.head_block_time();

   FC_ASSERT( ( now - account.last_post ) > delay,
      "Account must be inactive to be reset." );
   FC_ASSERT( ( now - account.last_root_post ) > delay,
      "Account must be inactive to be reset." );
   FC_ASSERT( ( now - account.last_vote_time ) > delay,
      "Account must be inactive to be reset." );
   FC_ASSERT( ( now - account.last_view_time ) > delay,
      "Account must be inactive to be reset." );
   FC_ASSERT( ( now - account.last_vote_time ) > delay,
      "Account must be inactive to be reset." );
   FC_ASSERT( account.reset_account == o.reset_account,
      "Reset account does not match reset account on account." );

   _db.update_owner_authority( account, o.new_owner_authority );
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void set_reset_account_evaluator::do_apply( const set_reset_account_operation& o )
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
      FC_ASSERT( b.is_executive( o.signatory ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& account = _db.get_account( o.account );
   const account_object& new_reset_account = _db.get_account( o.new_reset_account );
   FC_ASSERT( new_reset_account.active, 
      "Account: ${s} must be active to become new reset account.",("s", o.new_reset_account) );

   FC_ASSERT( account.reset_account != o.new_reset_account,
      "Reset account must change." );
   
   _db.modify( account, [&]( account_object& a )
   {
      a.reset_account = o.new_reset_account;
      a.reset_account_delay_days = o.days;
   });
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void change_recovery_account_evaluator::do_apply( const change_recovery_account_operation& o )
{ try {
   const account_name_type& signed_for = o.account_to_recover;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_executive( o.signatory ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& new_recovery_account = _db.get_account( o.new_recovery_account );
   FC_ASSERT( new_recovery_account.active,
      "Account: ${s} must be active to be a new recovery account.",("s", o.new_recovery_account) );
   const account_object& account_to_recover = _db.get_account( o.account_to_recover );
   time_point now = _db.head_block_time();

   const auto& change_recovery_idx = _db.get_index< change_recovery_account_request_index >().indices().get< by_account >();
   auto request = change_recovery_idx.find( o.account_to_recover );

   if( request == change_recovery_idx.end() ) // New request
   {
      _db.create< change_recovery_account_request_object >( [&]( change_recovery_account_request_object& req )
      {
         req.account_to_recover = o.account_to_recover;
         req.recovery_account = o.new_recovery_account;
         req.effective_on = now + OWNER_AUTH_RECOVERY_PERIOD;
      });
   }
   else if( account_to_recover.recovery_account != o.new_recovery_account ) // Change existing request
   {
      _db.modify( *request, [&]( change_recovery_account_request_object& req )
      {
         req.recovery_account = o.new_recovery_account;
         req.effective_on = now + OWNER_AUTH_RECOVERY_PERIOD;
      });
   }
   else // Request exists and changing back to current recovery account
   {
      _db.remove( *request );
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void decline_voting_rights_evaluator::do_apply( const decline_voting_rights_operation& o )
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
      FC_ASSERT( b.is_chief( o.signatory ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& account = _db.get_account( o.account );
   time_point now = _db.head_block_time();

   const auto& req_idx = _db.get_index< decline_voting_rights_request_index >().indices().get< by_account >();
   auto req_itr = req_idx.find( o.account );

   if( o.declined )
   {
      FC_ASSERT( req_itr == req_idx.end(),
         "Decline voting rights request already exists for this account." );
      
      _db.create< decline_voting_rights_request_object >( [&]( decline_voting_rights_request_object& dvrro )
      {
         dvrro.account = account.name;
         dvrro.effective_date = now + DECLINE_VOTING_RIGHTS_DURATION;
      });
   }
   else
   {
      FC_ASSERT( req_itr != req_idx.end(),
         "Decline voting rights request does not yet exist for this account to cancel." );

      _db.remove( *req_itr );
   } 
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void connection_request_evaluator::do_apply( const connection_request_operation& o )
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
   const account_object& req_account = _db.get_account( o.requested_account );
   time_point now = _db.head_block_time();

   const auto& req_idx = _db.get_index< connection_request_index >().indices().get< by_account_req >();
   const auto& acc_idx = _db.get_index< connection_request_index >().indices().get< by_req_account >();
   auto req_itr = req_idx.find( boost::make_tuple( account.name, req_account.name ) );
   auto acc_itr = acc_idx.find( boost::make_tuple( account.name, req_account.name ) );

   account_name_type account_a_name;
   account_name_type account_b_name;

   if( account.id < req_account.id )      // Connection objects are sorted with lowest ID is account A. 
   {
      account_a_name = account.name;
      account_b_name = req_account.name;
   }
   else
   {
      account_b_name = account.name;
      account_a_name = req_account.name;
   }

   connection_tier_type connection_tier = connection_tier_type::CONNECTION;

   for( size_t i = 0; i < connection_tier_values.size(); i++ )
   {
      if( o.connection_type == connection_tier_values[ i ] )
      {
         connection_tier = connection_tier_type( i );
         break;
      }
   }

   const auto& con_idx = _db.get_index< connection_index >().indices().get< by_accounts >();
   auto con_itr = con_idx.find( boost::make_tuple( account_a_name, account_b_name, connection_tier_type::CONNECTION ) );

   if( req_itr == req_idx.end() && acc_itr == acc_idx.end() )      // New connection request 
   {
      FC_ASSERT( o.requested,
         "Request doesn't exist, user must select to request connection with the account." );
      if( con_itr == con_idx.end() )      // No existing connection object.
      { 
         FC_ASSERT( connection_tier == connection_tier_type::CONNECTION,
            "First connection request must be of standard Connection type before elevation to higher levels." );

         _db.create< connection_request_object >( [&]( connection_request_object& cro )
         {
            cro.account = account.name;
            cro.requested_account = req_account.name;
            cro.connection_type = connection_tier;
            from_string( cro.message, o.message );
            cro.expiration = now + CONNECTION_REQUEST_DURATION;
         });
      }
      else        // Connection object found, requesting level increase.
      {
         const connection_object& connection_obj = *con_itr;

         auto friend_itr = con_idx.find( boost::make_tuple( account_a_name, account_b_name, connection_tier_type::FRIEND ) );
         auto comp_itr = con_idx.find( boost::make_tuple( account_a_name, account_b_name, connection_tier_type::COMPANION ) );

         FC_ASSERT( connection_tier != connection_tier_type::CONNECTION,
            "Connection of this type already exists, should request a type increase." );

         if( connection_tier == connection_tier_type::FRIEND )
         {
            FC_ASSERT( friend_itr == con_idx.end(),
               "Friend level connection already exists." );
            FC_ASSERT( now > ( connection_obj.created + CONNECTION_REQUEST_DURATION ),
               "Friend Connection must wait one week from first connection." );
         }
         else if( connection_tier == connection_tier_type::COMPANION )
         {
            FC_ASSERT( friend_itr != con_idx.end(),
               "Companion connection must follow a friend connection." );
               FC_ASSERT( comp_itr == con_idx.end(),
               "companion level connection already exists." );
            FC_ASSERT( now > ( friend_itr->created + CONNECTION_REQUEST_DURATION ),
               "Companion Connection must wait one week from Friend connection." );
         }

         _db.create< connection_request_object >( [&]( connection_request_object& cro ) 
         {
            cro.requested_account = req_account.name;
            cro.account = account.name;
            cro.connection_type = connection_tier;
            from_string( cro.message, o.message );
            cro.expiration = now + CONNECTION_REQUEST_DURATION;
         });
      }
   } 
   else // Request exists and is being cancelled.
   { 
      FC_ASSERT( !o.requested,
         "Connection currently exists, set request to false to cancel." );
 
      _db.remove( *req_itr );
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void connection_accept_evaluator::do_apply( const connection_accept_operation& o )
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
   const account_object& req_account = _db.get_account( o.requesting_account );
   time_point now = _db.head_block_time();
   public_key_type public_key;

   connection_tier_type connection_tier = connection_tier_type::CONNECTION;

   for( size_t i = 0; i < connection_tier_values.size(); i++ )
   {
      if( o.connection_type == connection_tier_values[ i ] )
      {
         connection_tier = connection_tier_type( i );
         break;
      }
   }

   switch( connection_tier )
   {
      case connection_tier_type::CONNECTION:
      {
         public_key = account.connection_public_key;
      }
      break;
      case connection_tier_type::FRIEND:
      {
         public_key = account.friend_public_key;
      }
      break;
      case connection_tier_type::COMPANION:
      {
         public_key = account.companion_public_key;
      }
      break;
      default:
      {
         FC_ASSERT( false, 
            "Invalid connection type." );
      }
   }

   account_name_type account_a_name;
   account_name_type account_b_name;

   if( account.id < req_account.id )         // Connection objects are sorted with lowest ID is account A.
   {
      account_a_name = account.name;
      account_b_name = req_account.name;
   }
   else
   {
      account_b_name = account.name;
      account_a_name = req_account.name;
   }

   const auto& con_idx = _db.get_index< connection_index >().indices().get< by_accounts >();
   auto con_itr = con_idx.find( boost::make_tuple( account_a_name, account_b_name, connection_tier ) );

   const auto& req_idx = _db.get_index< connection_request_index >().indices().get< by_account_req >();
   auto req_itr = req_idx.find( boost::make_tuple( o.requesting_account, o.account ) );

   const account_following_object& a_following_set = _db.get_account_following( account_a_name );
   const account_following_object& b_following_set = _db.get_account_following( account_b_name );

   if( con_itr == con_idx.end() )       // No existing connection object of that type, creating new connection.
   {
      FC_ASSERT( o.connected,
         "Connection doesn't exist, must select to connect with account" );
      FC_ASSERT( req_itr != req_idx.end(),
         "Connection Request doesn't exist to accept." );
      const connection_request_object& request = *req_itr;
      FC_ASSERT( connection_tier == request.connection_type,
         "Connection request must be of the same level as acceptance" );

      _db.create< connection_object >( [&]( connection_object& co )
      {
         co.account_a = account_a_name;
         co.account_b = account_b_name;

         if( account_a_name == account.name )      // We're account A
         {
            co.encrypted_key_a = encrypted_keypair_type( req_account.secure_public_key, public_key, o.encrypted_key );
         } 
         else        // We're account B
         {
            co.encrypted_key_b = encrypted_keypair_type( req_account.secure_public_key, public_key, o.encrypted_key );
         }

         co.connection_type = connection_tier;
         from_string( co.connection_id, o.connection_id );
         co.last_message_time_a = now;
         co.last_message_time_b = now;
         co.message_count = 0;
         co.consecutive_days = 0;
         co.last_updated = now;
         co.created = now;
      });

      _db.modify( a_following_set, [&]( account_following_object& afo )
      {
         if( connection_tier == connection_tier_type::CONNECTION )
         {
            afo.connections.insert( account_b_name );
         }
         else if( connection_tier == connection_tier_type::FRIEND )
         {
            afo.friends.insert( account_b_name );
         }
         else if( connection_tier == connection_tier_type::COMPANION )
         {
            afo.companions.insert( account_b_name );
         }
         afo.last_updated = now;
      });

      _db.modify( b_following_set, [&]( account_following_object& afo )
      {
         if( connection_tier == connection_tier_type::CONNECTION )
         {
            afo.connections.insert( account_a_name );
         }
         else if( connection_tier == connection_tier_type::FRIEND )
         {
            afo.friends.insert( account_a_name );
         }
         else if( connection_tier == connection_tier_type::COMPANION )
         {
            afo.companions.insert( account_a_name );
         }
         afo.last_updated = now;
      });

      _db.remove( request );  // Remove initial request object
   }
   else 
   {
      const connection_object& connection_obj = *con_itr;

      if( o.connected ) // Connection object found, adding returning acceptance or editing keys.
      {
         _db.modify( connection_obj, [&]( connection_object& co )
         {
            if( account_a_name == account.name )    // We're account A
            {
               co.encrypted_key_a = encrypted_keypair_type( req_account.secure_public_key, public_key, o.encrypted_key );
            } 
            else     // We're account B
            {
               co.encrypted_key_b = encrypted_keypair_type( req_account.secure_public_key, public_key, o.encrypted_key );
            }
         }); 
      }
      else  // Connection object is found, and is being unconnected.
      {
         _db.modify( a_following_set, [&]( account_following_object& afo )
         {
            if( connection_tier == connection_tier_type::CONNECTION )
            {
               afo.connections.erase( account_b_name );
            }
            else if( connection_tier == connection_tier_type::FRIEND )
            {
               afo.friends.erase( account_b_name );
            }
            else if( connection_tier == connection_tier_type::COMPANION )
            {
               afo.companions.erase( account_b_name );
            }
            afo.last_updated = now;
         });

         _db.modify( b_following_set, [&]( account_following_object& afo )
         {
            if( connection_tier == connection_tier_type::CONNECTION )
            {
               afo.connections.erase( account_a_name );
            }
            else if( connection_tier == connection_tier_type::FRIEND )
            {
               afo.friends.erase( account_a_name );
            }
            else if( connection_tier == connection_tier_type::COMPANION )
            {
               afo.companions.erase( account_a_name );
            }
            afo.last_updated = now;
         });

         _db.remove( connection_obj );
      } 
   }

   _db.update_account_in_feed( o.account, o.requesting_account );
   _db.update_account_in_feed( o.requesting_account, o.account );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


/**
 * Enables an account to follow another account by adding it to
 * the account's following object. 
 * Additionally allows for filtering accounts from interfaces.
 */
void account_follow_evaluator::do_apply( const account_follow_operation& o )
{ try {
   const account_name_type& signed_for = o.follower;
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
   const account_object& follower = _db.get_account( o.follower );
   const account_object& following = _db.get_account( o.following );
   const account_following_object& follower_set = _db.get_account_following( o.follower );
   const account_following_object& following_set = _db.get_account_following( o.following );
   time_point now = _db.head_block_time();

   if( o.added )
   {
      if( o.followed )    // Creating new follow relation
      {
         FC_ASSERT( !follower_set.is_following( following.name ),
            "Account is already followed." );

         _db.modify( follower_set, [&]( account_following_object& afo ) 
         {
            afo.add_following( following.name );
            afo.last_updated = now;
         });
         
         _db.modify( following_set, [&]( account_following_object& afo ) 
         {
            afo.add_follower( follower.name );
            afo.last_updated = now;
         });

         if( follower.membership == membership_tier_type::NONE )     // Check for the presence of an ad bid on this follow.
         {
            const auto& bid_idx = _db.get_index< ad_bid_index >().indices().get< by_provider_metric_author_objective_price >();
            auto bid_itr = bid_idx.lower_bound( std::make_tuple( o.interface, ad_metric_type::FOLLOW_METRIC, o.following, o.following ) );

            while( bid_itr != bid_idx.end() &&
               bid_itr->provider == o.interface &&
               bid_itr->metric == ad_metric_type::FOLLOW_METRIC &&
               bid_itr->author == o.following &&
               to_string( bid_itr->objective ) == string( o.following ) )    // Retrieves highest paying bids for this share by this interface.
            {
               const ad_bid_object& bid = *bid_itr;
               const ad_audience_object& audience = _db.get_ad_audience( bid.bidder, bid.audience_id );

               if( !bid.is_delivered( o.follower ) && audience.is_audience( o.follower ) )
               {
                  _db.deliver_ad_bid( bid, follower );
                  break;
               }

               ++bid_itr;
            }
         }
      }  
      else    // Creating new filter relation
      {
         FC_ASSERT( !follower_set.is_following( following.name ),
            "Cannot filter an account that you follow, unfollow first." );

         _db.modify( follower_set, [&]( account_following_object& afo )
         {
            afo.add_filtered( following.name );
            afo.last_updated = now;
         });
      }
   }
   else
   {
      if( o.followed )    // Unfollowing
      {
         FC_ASSERT( follower_set.is_following( following.name ),
            "Cannot unfollow an account that you do not follow." );

         _db.modify( follower_set, [&]( account_following_object& afo )
         {
            afo.remove_following( following.name );
            afo.last_updated = now;
         });
         
         _db.modify( following_set, [&]( account_following_object& afo )
         {
            afo.remove_follower( follower.name );
            afo.last_updated = now;
         });
      }  
      else    // Unfiltering
      {
         FC_ASSERT( follower_set.is_filtered( following.name ),
            "Cannot unfilter an account that you do not filter." );

         _db.modify( follower_set, [&]( account_following_object& afo )
         {
            afo.remove_filtered( following.name );
            afo.last_updated = now;
         });
      }
   }

   _db.modify( following, [&]( account_object& a ) 
   {
      a.follower_count = following_set.followers.size();
      a.following_count = following_set.following.size();
   });

   _db.modify( follower, [&]( account_object& a ) 
   {
      a.follower_count = follower_set.followers.size();
      a.following_count = follower_set.following.size();
   });

   _db.update_account_in_feed( o.follower, o.following );
   _db.update_account_in_feed( o.following, o.follower );
   
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void tag_follow_evaluator::do_apply( const tag_follow_operation& o )
{ try {
   const account_name_type& signed_for = o.follower;
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
   const account_object& follower = _db.get_account( o.follower );
   const account_following_object& follower_set = _db.get_account_following( o.follower );
   const tag_following_object* tag_ptr = _db.find_tag_following( o.tag );
   time_point now = _db.head_block_time();

   if( tag_ptr != nullptr )      // Tag follow already exists
   {
      if( o.added )
      {
         if( o.followed )        // Creating new follow relation
         {
            FC_ASSERT( !follower_set.is_followed_tag( o.tag ),
               "Tag is already followed." );

            _db.modify( follower_set, [&]( account_following_object& afo )
            {
               afo.add_followed_tag( o.tag );
               afo.last_updated = now;
            });
            
            _db.modify( *tag_ptr, [&]( tag_following_object& tfo )
            {
               tfo.add_follower( follower.name );
               tfo.last_updated = now;
            });

         }  
         else        // Creating new filter relation
         {
            FC_ASSERT( !follower_set.is_followed_tag( o.tag ),
               "Cannot filter a tag that you follow, unfollow first." );

            _db.modify( follower_set, [&]( account_following_object& afo )
            {
               afo.add_filtered_tag( o.tag );
               afo.last_updated = now;
            });
         }
      }
      else
      {
         if( o.followed )    // Unfollowing the tag
         {
            FC_ASSERT( follower_set.is_followed_tag( o.tag ),
               "Cannot unfollow a tag that you do not follow." );

            _db.modify( follower_set, [&]( account_following_object& afo )
            {
               afo.remove_followed_tag( o.tag );
               afo.last_updated = now;
            });
            
            _db.modify( *tag_ptr, [&]( tag_following_object& tfo )
            {
               tfo.remove_follower( follower.name );
               tfo.last_updated = now;
            });
         }  
         else        // Unfiltering
         {
            FC_ASSERT( follower_set.is_filtered_tag( o.tag ),
               "Cannot unfilter a tag that you do not filter." );

            _db.modify( follower_set, [&]( account_following_object& afo )
            {
               afo.remove_filtered_tag( o.tag );
               afo.last_updated = now;
            });
         }
      }
   }
   else     // New tag following object
   {
      FC_ASSERT( o.added,
         "Tag does not yet exist, cannot unfollow." );

      if( o.followed )
      {
         _db.modify( follower_set, [&]( account_following_object& afo )
         {
            afo.add_followed_tag( o.tag );
            afo.last_updated = now;
         });

         _db.create< tag_following_object >( [&]( tag_following_object& tfo )
         {
            tfo.tag = o.tag;
            tfo.add_follower( follower.name );
            tfo.last_updated = now;
         });
      }
      else    // Adding filter
      {
         FC_ASSERT( !follower_set.is_followed_tag( o.tag ),
            "Cannot filter a tag that you follow, unfollow first." );

         _db.modify( follower_set, [&]( account_following_object& afo )
         {
            afo.add_filtered_tag( o.tag );
            afo.last_updated = now;
         });

         _db.create< tag_following_object >( [&]( tag_following_object& tfo )
         {
            tfo.tag = o.tag;
            tfo.last_updated = now;
         });
      }
   }

   _db.update_tag_in_feed( o.follower, o.tag );
   
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void activity_reward_evaluator::do_apply( const activity_reward_operation& o )
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
      FC_ASSERT( b.is_officer( o.signatory ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();
   const account_object& account = _db.get_account( o.account );

   FC_ASSERT( account.producer_vote_count >= MIN_ACTIVITY_PRODUCERS,
      "Account must have at least 10 producer votes to claim activity reward." );

   asset stake = _db.get_staked_balance( o.account, SYMBOL_EQUITY );

   FC_ASSERT( stake >= asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ),
      "Account must have at least one equity asset to claim activity reward." );

   const comment_metrics_object& comment_metrics = _db.get_comment_metrics();
   const comment_object& comment = _db.get_comment( o.account, o.permlink );
   const comment_view_object& view = _db.get_comment_view( o.account, o.view_id );
   const comment_vote_object& vote = _db.get_comment_vote( o.account, o.vote_id );
   const comment_object& view_comment = _db.get( comment_id_type( o.view_id ) );
   const comment_object& vote_comment = _db.get( comment_id_type( o.vote_id ) );

   FC_ASSERT( comment.id != vote_comment.id,
      "Created post and voted must must be different comments." );
   FC_ASSERT( comment.id != view_comment.id,
      "Created post and voted must must be different comments." );
   FC_ASSERT( view_comment.id != vote_comment.id,
      "Viewed post and voted must must be different comments." );

   FC_ASSERT( uint32_t( comment.net_votes ) >= ( comment_metrics.median_vote_count / 5 ),
      "Referred recent Post should have at least 20% of median number of votes." );
   FC_ASSERT( comment.view_count >= ( comment_metrics.median_view_count / 5 ),
      "Referred recent Post should have at least 20% of median number of views." );
   FC_ASSERT( comment.vote_power >= ( comment_metrics.median_vote_power / 5 ),
      "Referred recent Post should have at least 20% of median vote power." );
   FC_ASSERT( comment.view_power >= ( comment_metrics.median_view_power / 5 ),
      "Referred recent Post should have at least 20% of median view power." );

   FC_ASSERT( now < ( account.last_activity_reward + fc::days(1) ),
      "Can only claim activity reward once per 24 hours." );
   FC_ASSERT( now < ( comment.created + fc::days(1) ),
      "Referred Recent Post should have been made in the last 24 hours." );
   FC_ASSERT( now < ( view.created + fc::days(1) ),
      "Referred Recent View should have been made in the last 24 hours." );
   FC_ASSERT( now < ( vote.created + fc::days(1) ),
      "Referred Recent Vote should have been made in the last 24 hours." );

   const auto& vote_idx = _db.get_index< producer_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, 1 ) );     // Gets top voted producer of account.

   const producer_object& producer = _db.get_producer( vote_itr->producer );

   _db.claim_activity_reward( account, producer, comment.reward_currency );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }

} } // node::chain