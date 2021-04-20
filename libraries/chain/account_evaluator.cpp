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
   _db.check_namespace( o.new_account_name );
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
      "Insufficient balance: ${b} of registrar ${r} to create account: ${a} with fee: ${f}.", 
      ( "b", registrar_balance.liquid_balance )("r", o.registrar)("a", o.new_account_name)("f", o.fee) );

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
      "Account: ${s} is not active, please select a different registrar account.",
      ("s", o.registrar) );

   if( o.recovery_account.size() )
   {
      const account_object& recovery_account = _db.get_account( o.recovery_account );
      FC_ASSERT( recovery_account.active,
         "Account: ${s} is not active, please select a different recovery account.",
         ("s", o.recovery_account) );
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
      if( o.referrer.size() )
      {
         a.referrer = o.referrer;
      }
      if( o.proxy.size() )
      {
         a.proxy = o.proxy;
      }
      if( o.recovery_account.size() )
      {
         a.recovery_account = o.recovery_account;
      }
      if( o.reset_account.size() )
      {
         a.reset_account = o.reset_account;
      }

      from_string( a.details, o.details );
      from_string( a.url, o.url );
      from_string( a.profile_image, o.profile_image );
      from_string( a.cover_image, o.cover_image );
      from_string( a.json, o.json );
      from_string( a.json_private, o.json_private );
      from_string( a.first_name, o.first_name );
      from_string( a.last_name, o.last_name );
      from_string( a.gender, o.gender );
      from_string( a.date_of_birth, o.date_of_birth );
      from_string( a.email, o.email );
      from_string( a.phone, o.phone );
      from_string( a.nationality, o.nationality );
      from_string( a.relationship, o.relationship );
      from_string( a.political_alignment, o.political_alignment );

      for( auto t : o.interests )
      {
         a.interests.insert( t );
      }
      
      a.membership = membership_tier_type::NONE;

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

      a.created = now;
      a.last_updated = now;
      a.last_vote_time = now;
      a.last_view_time = now;
      a.last_share_time = now;
      a.last_post = now;
      a.last_root_post = now;
      a.last_transfer_time = now;
      a.last_activity_reward = now;
      a.last_account_recovery = now;
      a.last_community_created = now;
      a.last_asset_created = now;
      a.membership_expiration = time_point::maximum();

      a.voting_power = PERCENT_100;
      a.viewing_power = PERCENT_100;
      a.sharing_power = PERCENT_100;
      a.commenting_power = PERCENT_100;

      a.mined = false;
      a.can_vote = true;
      a.revenue_share = false;
      a.active = true;
   });

   if( o.fee.amount > 0 )
   {
      _db.adjust_staked_balance( o.new_account_name, o.fee );
   }

   if( o.delegation.amount > 0 )
   {
      _db.adjust_receiving_balance( o.new_account_name, o.delegation );

      _db.create< asset_delegation_object >( [&]( asset_delegation_object& vdo )
      {
         vdo.delegator = o.registrar;
         vdo.delegatee = o.new_account_name;
         vdo.amount = o.delegation;
      });
   }
   
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

   _db.create< account_permission_object >( [&]( account_permission_object& aao )
   {
      aao.account = o.new_account_name;
   });

   if( _db.find_producer( o.registrar ) != nullptr )
   {
      _db.create< producer_vote_object >( [&]( producer_vote_object& wvo )
      {
         wvo.producer = o.registrar;
         wvo.account = o.new_account_name;
         wvo.vote_rank = 1;
         wvo.last_updated = now;
         wvo.created = now;
      });

      _db.modify( new_account, [&]( account_object& a )
      {
         a.producer_vote_count++;
      });
   }

   if( _db.find_network_officer( o.registrar ) != nullptr )
   {
      _db.create< network_officer_vote_object >( [&]( network_officer_vote_object& novo )
      {
         novo.network_officer = o.registrar;
         novo.account = o.new_account_name;
         novo.vote_rank = 1;
         novo.last_updated = now;
         novo.created = now;
      });

      _db.modify( new_account, [&]( account_object& a )
      {
         a.officer_vote_count++;
      });
   }

   // ilog( "Registrar: ${r} created new account: ${a}",("r",o.registrar)("a",o.new_account_name));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_update_evaluator::do_apply( const account_update_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   const account_authority_object& account_auth = _db.get< account_authority_object, by_account >( o.account );
   time_point now = _db.head_block_time();
   
   FC_ASSERT( ( now - account_auth.last_owner_update ) >= OWNER_UPDATE_LIMIT,
      "Owner authority can only be updated once an hour." );

   for( auto a : o.owner_auth.account_auths )
   {
      const account_object& account_auth = _db.get_account( a.first );
      FC_ASSERT( account_auth.active, 
         "Account: ${s} must be active to add account authority.",
         ("s", account_auth) );
   }

   if( o.owner_auth.num_auths() > 0 )
   {
      _db.update_owner_authority( account, o.owner_auth );
   }
   
   for( auto a : o.active_auth.account_auths )
   {
      const account_object& account_auth = _db.get_account( a.first );
      FC_ASSERT( account_auth.active, 
         "Account: ${s} must be active to add account authority.",
         ("s", account_auth) );
   }

   for( auto a : o.posting_auth.account_auths )
   {
      const account_object& account_auth = _db.get_account( a.first );
      FC_ASSERT( account_auth.active, 
         "Account: ${s} must be active to add account authority.",
         ("s", account_auth) );
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

      a.last_updated = now;

      if( o.details.size() > 0 )
      {
         from_string( a.details, o.details );
      }
      if( o.url.size() > 0 )
      {
         from_string( a.url, o.url );
      }
      if( o.profile_image.size() > 0 )
      {
         from_string( a.profile_image, o.profile_image );
      }
      if( o.cover_image.size() > 0 )
      {
         from_string( a.cover_image, o.cover_image );
      }
      if( o.json.size() > 0 )
      {
         from_string( a.json, o.json );
      }
      if( o.json_private.size() > 0 )
      {
         from_string( a.json_private, o.json_private );
      }
      if( o.first_name.size() > 0 )
      {
         from_string( a.first_name, o.first_name );
      }
      if( o.last_name.size() > 0 )
      {
         from_string( a.last_name, o.last_name );
      }
      if( o.gender.size() > 0 )
      {
         from_string( a.gender, o.gender );
      }
      if( o.date_of_birth.size() > 0 )
      {
         from_string( a.date_of_birth, o.date_of_birth );
      }
      if( o.email.size() > 0 )
      {
         from_string( a.email, o.email );
      }
      if( o.phone.size() > 0 )
      {
         from_string( a.phone, o.phone );
      }
      if( o.nationality.size() > 0 )
      {
         from_string( a.nationality, o.nationality );
      }
      if( o.relationship.size() > 0 )
      {
         from_string( a.relationship, o.relationship );
      }
      if( o.political_alignment.size() > 0 )
      {
         from_string( a.political_alignment, o.political_alignment );
      }
      if( o.pinned_permlink.size() > 0 )
      {
         from_string( a.pinned_permlink, o.pinned_permlink );
      }
      a.interests.clear();
      for( auto t : o.interests )
      {
         a.interests.insert( t );
      }
      a.active = o.active;
   });

   _db.modify( account_auth, [&]( account_authority_object& auth )
   {
      if( o.owner_auth.num_auths() > 0 )
      {
         auth.owner_auth = o.owner_auth;
      }
      if( o.active_auth.num_auths() > 0 )
      {
         auth.active_auth = o.active_auth;
      }
      if( o.posting_auth.num_auths() > 0 )
      {
         auth.posting_auth = o.posting_auth;
      }
   });
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_verification_evaluator::do_apply( const account_verification_operation& o )
{ try {
   time_point now = _db.head_block_time();
   const account_object& verified_account = _db.get_account( o.verified_account );
   const account_object& verifier_account = _db.get_account( o.verifier_account );
   
   FC_ASSERT( verified_account.active, 
      "Account: ${s} must be active.",("s", o.verified_account) );
   FC_ASSERT( verifier_account.active, 
      "Account: ${s} must be active.",("s", o.verifier_account) );

   const auto& verification_idx = _db.get_index< account_verification_index >().indices().get< by_verifier_verified >();
   auto verification_itr = verification_idx.find( boost::make_tuple( o.verifier_account, o.verified_account ) );

   account_name_type account_a_name;
   account_name_type account_b_name;

   if( verified_account.id < verifier_account.id )      // Connection objects are sorted with lowest ID is account A. 
   {
      account_a_name = verified_account.name;
      account_b_name = verifier_account.name;
   }
   else
   {
      account_b_name = verified_account.name;
      account_a_name = verifier_account.name;
   }

   const auto& con_idx = _db.get_index< account_connection_index >().indices().get< by_accounts >();
   auto con_itr = con_idx.find( boost::make_tuple( account_a_name, account_b_name, connection_tier_type::CONNECTION ) );

   FC_ASSERT( con_itr != con_idx.end() && 
      con_itr->approved(),
      "Accounts must have an approved connection before creating a verification." );

   if( verification_itr == verification_idx.end() )
   {
      FC_ASSERT( o.verified, 
         "Cannot remove, no verification found." );

      _db.create< account_verification_object >( [&]( account_verification_object& avo )
      {
         avo.verifier_account = o.verifier_account;
         avo.verified_account = o.verified_account;
         from_string( avo.shared_image, o.shared_image );
         avo.last_updated = now;
         avo.created = now;
      });
   }
   else
   {
      const account_verification_object& verification = *verification_itr;

      if( o.verified )
      {
         _db.modify( verification, [&]( account_verification_object& avo )
         {
            from_string( avo.shared_image, o.shared_image );
            avo.last_updated = now;
         });
      }
      else
      {
         ilog( "Removed: ${v}",("v",verification));
         _db.remove( verification );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_membership_evaluator::do_apply( const account_membership_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", o.account) );
   const account_object* int_account_ptr = nullptr;

   if( o.interface.size() )
   {
      int_account_ptr = _db.find_account( o.interface );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }
   
   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   time_point now = _db.head_block_time();
   asset liquid = _db.get_liquid_balance( o.account, SYMBOL_COIN );
   asset monthly_fee = asset( 0, SYMBOL_USD );
   price usd_coin_price = props.current_median_usd_price;

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

   if( account.membership_expiration >= now )     // If existing membership is active.
   {
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
   }

   asset total_fees = std::max( asset( 0, SYMBOL_USD ), monthly_fee * o.months - carried_fees );       // Applies a discount on new membership if an existing membership is still active.
   asset fee_coin_value = total_fees * usd_coin_price;

   FC_ASSERT( liquid >= fee_coin_value,
      "Account: ${a} has insufficent liquid balance: ${b} to pay for the requested membership duration fees: ${f} with USD price of: ${p}.",
      ("a",o.account)("b", liquid)("f", fee_coin_value)("p", usd_coin_price) );

   if( total_fees.amount > 0 )
   {
      if( int_account_ptr != nullptr )
      {
         _db.pay_membership_fees( account, total_fees, *int_account_ptr );      // Pays splits to interface, and network.
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
      a.last_updated = now;
   });
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_update_list_evaluator::do_apply( const account_update_list_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", o.account) );
   
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
   const account_object& voter = _db.get_account( o.account );
   FC_ASSERT( voter.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
      
   const producer_object& producer = _db.get_producer( o.producer );
   time_point now = _db.head_block_time();

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
         _db.create< producer_vote_object >( [&]( producer_vote_object& v )
         {
            v.producer = producer.owner;
            v.account = voter.name;
            v.vote_rank = o.vote_rank;
            v.last_updated = now;
            v.created = now;
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
            ilog( "Removed: ${v}",("v",*producer_itr));
            _db.remove( *producer_itr );
         }

         _db.update_producer_votes( voter, o.producer, o.vote_rank );   // Remove existing producer vote, and add at new rank.
      }
   }
   else  // Removing existing vote
   {
      if( producer_itr != account_producer_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*producer_itr));
         _db.remove( *producer_itr );
      }
      else if( rank_itr != account_rank_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*rank_itr));
         _db.remove( *rank_itr );
      }
      _db.update_producer_votes( voter );
   }

   _db.process_update_producer_set();     // Recalculates the voting power for all producers.

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_update_proxy_evaluator::do_apply( const account_update_proxy_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   time_point now = _db.head_block_time();
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   FC_ASSERT( account.proxy != o.proxy,
      "Proxy must change." );
   FC_ASSERT( account.can_vote,
      "Account has declined the ability to vote and cannot proxy votes." );

   if( account.proxy.size() )
   {
      const account_object& old_proxy = _db.get_account( account.proxy );

      _db.modify( old_proxy, [&]( account_object& a )
      {
         a.proxied.erase( o.account );       // Remove name from old proxy.
      });
   }

   if( o.proxy.size() ) 
   {
      const account_object& new_proxy = _db.get_account( o.proxy );
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

      _db.clear_account_votes( account.name );    // clear all individual vote records

      _db.modify( account, [&]( account_object& a )
      {
         a.proxy = o.proxy;
         a.last_updated = now;
      });

      _db.modify( new_proxy, [&]( account_object& a )
      {
         a.proxied.insert( o.account );
      });
   } 
   else 
   {        
      _db.modify( account, [&]( account_object& a )
      {
         a.proxy = PROXY_TO_SELF_ACCOUNT;
         a.last_updated = now;
      });
   }

   _db.process_update_producer_set();    // Recalculates the voting power for all producers.

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_request_recovery_evaluator::do_apply( const account_request_recovery_operation& o )
{ try {
   const account_object& recovery_account = _db.get_account( o.recovery_account );
   FC_ASSERT( recovery_account.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.recovery_account) );
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
      ilog( "Removed: ${v}",("v",*request));
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


void account_recover_evaluator::do_apply( const account_recover_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account_to_recover );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.account_to_recover) );
   time_point now = _db.head_block_time();

   FC_ASSERT( now - account.last_account_recovery >= OWNER_UPDATE_LIMIT,
      "Owner authority can only be updated once an hour." );

   const auto& recovery_request_idx = _db.get_index< account_recovery_request_index >().indices().get< by_account >();
   auto request = recovery_request_idx.find( o.account_to_recover );

   FC_ASSERT( request != recovery_request_idx.end(),
      "There are no active recovery requests for this account." );
   FC_ASSERT( request->new_owner_authority == o.new_owner_authority,
      "New owner authority does not match recovery request." );

   const auto& recent_auth_idx = _db.get_index< account_authority_history_index >().indices().get< by_account >();
   auto recent_auth_itr = recent_auth_idx.lower_bound( o.account_to_recover );
   bool found = false;

   while( recent_auth_itr != recent_auth_idx.end() && recent_auth_itr->account == o.account_to_recover && !found )
   {
      found = recent_auth_itr->previous_owner_authority == o.recent_owner_authority;
      ++recent_auth_itr;
   }

   FC_ASSERT( found, 
      "Recent authority: ${a} not found in authority history.", ("a", o.recent_owner_authority ) );

   _db.remove( *request );     // Remove first, update_owner_authority may invalidate iterator
   _db.update_owner_authority( account, o.new_owner_authority );
   _db.modify( account, [&]( account_object& a )
   {
      a.last_account_recovery = now;
      a.last_updated = now;
   });
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_reset_evaluator::do_apply( const account_reset_operation& o )
{ try {
   const account_object& reset_account = _db.get_account( o.reset_account );
   FC_ASSERT( reset_account.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", o.reset_account) );
   const account_object& account = _db.get_account( o.account_to_reset );
   
   FC_ASSERT( account.reset_account == o.reset_account,
      "Reset account does not match reset account on account." );
   
   fc::microseconds delay = fc::days( account.reset_delay_days );
   time_point now = _db.head_block_time();

   FC_ASSERT( now > ( account.last_updated + delay ),
      "Account must be inactive to be reset. Last updated at: ${t}",
      ("t",account.last_updated));
   
   FC_ASSERT( now > ( account.last_vote_time + delay ),
      "Account must be inactive to be reset. Last Vote made at: ${t}",
      ("t",account.last_vote_time));
   FC_ASSERT( now > ( account.last_view_time + delay ),
      "Account must be inactive to be reset. Last View made at: ${t}",
      ("t",account.last_view_time));
   FC_ASSERT( now > ( account.last_share_time + delay ),
      "Account must be inactive to be reset. Last Share made at: ${t}",
      ("t",account.last_share_time));

   FC_ASSERT( now > ( account.last_post + delay ),
      "Account must be inactive to be reset. Last Comment made at: ${t}",
      ("t",account.last_post));
   FC_ASSERT( now > ( account.last_root_post + delay ),
      "Account must be inactive to be reset. Last Root Post made at: ${t}",
      ("t",account.last_root_post));

   FC_ASSERT( now > ( account.last_transfer_time + delay ),
      "Account must be inactive to be reset. Last Transfer made at: ${t}",
      ("t",account.last_transfer_time));
   FC_ASSERT( now > ( account.last_activity_reward + delay ),
      "Account must be inactive to be reset. Last Activity Reward made at: ${t}",
      ("t",account.last_activity_reward));
   
   _db.update_owner_authority( account, o.new_owner_authority );
   
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_reset_update_evaluator::do_apply( const account_reset_update_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.account) );
   const account_object& new_reset_account = _db.get_account( o.new_reset_account );
   FC_ASSERT( new_reset_account.active,
      "Account: ${s} must be active to become new reset account.",
      ("s", o.new_reset_account) );
   FC_ASSERT( account.reset_account != o.new_reset_account,
      "Reset account must change." );
   time_point now = _db.head_block_time();
   
   _db.modify( account, [&]( account_object& a )
   {
      a.reset_account = o.new_reset_account;
      a.reset_delay_days = o.days;
      a.last_updated = now;
   });

   ilog("Account: ${a} reset account updated to: ${r}",
      ("a",o.account)("r",o.new_reset_account));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_recovery_update_evaluator::do_apply( const account_recovery_update_operation& o )
{ try {
   const account_object& account_to_recover = _db.get_account( o.account_to_recover );
   FC_ASSERT( account_to_recover.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.account_to_recover) );
   const account_object& new_recovery_account = _db.get_account( o.new_recovery_account );
   FC_ASSERT( new_recovery_account.active,
      "Account: ${s} must be active to be a new recovery account.",("s", o.new_recovery_account) );
   time_point now = _db.head_block_time();

   const auto& change_recovery_idx = _db.get_index< account_recovery_update_request_index >().indices().get< by_account >();
   auto request = change_recovery_idx.find( o.account_to_recover );

   if( request == change_recovery_idx.end() ) // New request
   {
      _db.create< account_recovery_update_request_object >( [&]( account_recovery_update_request_object& req )
      {
         req.account_to_recover = o.account_to_recover;
         req.recovery_account = o.new_recovery_account;
         req.effective_on = now + OWNER_AUTH_RECOVERY_PERIOD;
      });
      
      ilog("Account: ${a} Requested to change recovery account to: ${n}",
         ("a",o.account_to_recover)("n",o.new_recovery_account));
   }
   else if( account_to_recover.recovery_account != o.new_recovery_account ) // Change existing request
   {
      _db.modify( *request, [&]( account_recovery_update_request_object& req )
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


void account_decline_voting_evaluator::do_apply( const account_decline_voting_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.account) );
   time_point now = _db.head_block_time();

   const auto& req_idx = _db.get_index< account_decline_voting_request_index >().indices().get< by_account >();
   auto req_itr = req_idx.find( o.account );

   if( o.declined )
   {
      FC_ASSERT( account.can_vote,
         "Account has already declined its voting rights." );
      FC_ASSERT( req_itr == req_idx.end(),
         "Decline voting rights request already exists for this account." );
      
      _db.create< account_decline_voting_request_object >( [&]( account_decline_voting_request_object& dvrro )
      {
         dvrro.account = o.account;
         dvrro.effective_date = now + DECLINE_VOTING_RIGHTS_DURATION;
      });
   }
   else
   {
      FC_ASSERT( req_itr != req_idx.end(),
         "Decline voting rights request does not yet exist for this account to cancel." );

      ilog( "Removed: ${v}",("v",*req_itr));
      _db.remove( *req_itr );
   } 
} FC_CAPTURE_AND_RETHROW( ( o ) ) }



void account_connection_evaluator::do_apply( const account_connection_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.account) );
   const account_object& con_account = _db.get_account( o.connecting_account );
   FC_ASSERT( con_account.active, 
      "Account: ${s} must be active to create or update connection.",("s", o.connecting_account) );
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

   if( account.id < con_account.id )         // Connection objects are sorted with lowest ID is account A.
   {
      account_a_name = account.name;
      account_b_name = con_account.name;
   }
   else
   {
      account_b_name = account.name;
      account_a_name = con_account.name;
   }

   const auto& con_idx = _db.get_index< account_connection_index >().indices().get< by_accounts >();
   auto con_itr = con_idx.find( boost::make_tuple( account_a_name, account_b_name, connection_tier ) );

   const account_following_object& a_following_set = _db.get_account_following( account_a_name );
   const account_following_object& b_following_set = _db.get_account_following( account_b_name );

   if( con_itr == con_idx.end() )       // No existing connection object of that type, creating new connection.
   {
      FC_ASSERT( o.connected,
         "Connection doesn't exist, must select to connect with account" );

      const account_connection_object& new_connection = _db.create< account_connection_object >( [&]( account_connection_object& co )
      {
         co.account_a = account_a_name;
         co.account_b = account_b_name;

         if( account_a_name == account.name )      // We're account A
         {
            co.encrypted_key_a = encrypted_keypair_type( con_account.secure_public_key, public_key, o.encrypted_key );
            from_string( co.message_a, o.message );
            from_string( co.json_a, o.json );
            co.approved_a = true;
         } 
         else        // We're account B
         {
            co.encrypted_key_b = encrypted_keypair_type( con_account.secure_public_key, public_key, o.encrypted_key );
            from_string( co.message_b, o.message );
            from_string( co.json_b, o.json );
            co.approved_b = true;
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

      ilog( "Account: ${a} accepted new connection with ${b} - \n ${c} \n",
      ("a", o.account)("b",o.connecting_account )("c", new_connection ) );
   }
   else 
   {
      const account_connection_object& connection_obj = *con_itr;

      if( o.connected ) // Connection object found, adding returning acceptance or editing keys.
      {
         _db.modify( connection_obj, [&]( account_connection_object& co )
         {
            if( account_a_name == account.name )    // We're account A
            {
               co.encrypted_key_a = encrypted_keypair_type( con_account.secure_public_key, public_key, o.encrypted_key );
               from_string( co.message_a, o.message );
               from_string( co.json_a, o.json );
               co.approved_a = true;
            } 
            else     // We're account B
            {
               co.encrypted_key_b = encrypted_keypair_type( con_account.secure_public_key, public_key, o.encrypted_key );
               from_string( co.message_b, o.message );
               from_string( co.json_b, o.json );
               co.approved_b = true;
            }
         });

         _db.modify( a_following_set, [&]( account_following_object& afo )
         {
            if( connection_tier == connection_tier_type::CONNECTION )
            {
               afo.add_connection( account_b_name );
            }
            else if( connection_tier == connection_tier_type::FRIEND )
            {
               afo.add_friend( account_b_name );
            }
            else if( connection_tier == connection_tier_type::COMPANION )
            {
               afo.add_companion( account_b_name );
            }
            afo.last_updated = now;
         });

         _db.modify( b_following_set, [&]( account_following_object& afo )
         {
            if( connection_tier == connection_tier_type::CONNECTION )
            {
               afo.add_connection( account_a_name );
            }
            else if( connection_tier == connection_tier_type::FRIEND )
            {
               afo.add_friend( account_a_name );
            }
            else if( connection_tier == connection_tier_type::COMPANION )
            {
               afo.add_companion( account_a_name );
            }
            afo.last_updated = now;
         });
      }
      else  // Connection object is found, and is being unconnected.
      {
         _db.modify( a_following_set, [&]( account_following_object& afo )
         {
            if( connection_tier == connection_tier_type::CONNECTION )
            {
               afo.remove_connection( account_b_name );
            }
            else if( connection_tier == connection_tier_type::FRIEND )
            {
               afo.remove_friend( account_b_name );
            }
            else if( connection_tier == connection_tier_type::COMPANION )
            {
               afo.remove_companion( account_b_name );
            }
            afo.last_updated = now;
         });

         _db.modify( b_following_set, [&]( account_following_object& afo )
         {
            if( connection_tier == connection_tier_type::CONNECTION )
            {
               afo.remove_connection( account_a_name );
            }
            else if( connection_tier == connection_tier_type::FRIEND )
            {
               afo.remove_friend( account_a_name );
            }
            else if( connection_tier == connection_tier_type::COMPANION )
            {
               afo.remove_companion( account_a_name );
            }
            afo.last_updated = now;
         });

         ilog( "Removed: ${v}",("v",connection_obj));
         _db.remove( connection_obj );
      }

      _db.update_account_in_feed( o.account, o.connecting_account );
      _db.update_account_in_feed( o.connecting_account, o.account );

      ilog( "Account: ${a} updated connection with ${b} - \n ${c} \n",
      ("a", o.account)("b",o.connecting_account )("c", connection_obj ) );
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


/**
 * Enables an account to follow another account by adding it to
 * the account's following object. 
 * Additionally allows for filtering accounts from interfaces.
 */
void account_follow_evaluator::do_apply( const account_follow_operation& o )
{ try {
   const account_object& follower = _db.get_account( o.follower );
   FC_ASSERT( follower.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.follower) );
   const account_object& following = _db.get_account( o.following );
   FC_ASSERT( following.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.following) );

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
            auto bid_itr = bid_idx.lower_bound( std::make_tuple( o.interface, ad_metric_type::FOLLOW_METRIC, account_name_type(), o.following ) );

            while( bid_itr != bid_idx.end() &&
               bid_itr->provider == o.interface &&
               bid_itr->metric == ad_metric_type::FOLLOW_METRIC &&
               bid_itr->author == account_name_type() &&
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


void account_follow_tag_evaluator::do_apply( const account_follow_tag_operation& o )
{ try {
   const account_object& follower = _db.get_account( o.follower );
   FC_ASSERT( follower.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.follower) );
   const account_following_object& follower_set = _db.get_account_following( o.follower );
   const account_tag_following_object* tag_ptr = _db.find_account_tag_following( o.tag );
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
            
            _db.modify( *tag_ptr, [&]( account_tag_following_object& tfo )
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
            
            _db.modify( *tag_ptr, [&]( account_tag_following_object& tfo )
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

         _db.create< account_tag_following_object >( [&]( account_tag_following_object& tfo )
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

         _db.create< account_tag_following_object >( [&]( account_tag_following_object& tfo )
         {
            tfo.tag = o.tag;
            tfo.last_updated = now;
         });
      }
   }

   _db.update_tag_in_feed( o.follower, o.tag );
   
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void account_activity_evaluator::do_apply( const account_activity_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", o.account) );
   time_point now = _db.head_block_time();

   FC_ASSERT( now >= ( account.last_activity_reward + fc::days(1) ),
      "Can only claim activity reward once per 24 hours." );
   FC_ASSERT( account.producer_vote_count >= MIN_ACTIVITY_PRODUCERS,
      "Account: ${a} must have at least 10 producer votes to claim activity reward. Votes: ${v}",
      ("a", o.account)("v", account.producer_vote_count) );

   const comment_metrics_object& comment_metrics = _db.get_comment_metrics();
   const comment_object& comment = _db.get_comment( o.account, o.permlink );

   const auto& recent_vote_idx = _db.get_index< comment_vote_index >().indices().get< by_voter_recent >();
   auto recent_vote_itr = recent_vote_idx.lower_bound( o.account );

   const auto& recent_view_idx = _db.get_index< comment_view_index >().indices().get< by_viewer_recent >();
   auto recent_view_itr = recent_view_idx.lower_bound( o.account );

   FC_ASSERT( recent_vote_itr != recent_vote_idx.end(),
      "Account must create a comment vote before claiming acivity reward." );
   FC_ASSERT( recent_view_itr != recent_view_idx.end(),
      "Account must create a comment view before claiming acivity reward." );
   
   const comment_vote_object& vote = *recent_vote_itr;
   const comment_view_object& view = *recent_view_itr;

   ilog("Claiming Activity reward for Comment: ${c} ${p} Vote: ${v} View: ${vi}",
      ("c",comment.author)("p",comment.permlink)("v",vote)("vi",view));
   
   FC_ASSERT( uint128_t( comment.net_votes ) >= ( comment_metrics.median_vote_count / 10 ),
      "Referred recent Post should have at least 10% of median number of votes." );
   FC_ASSERT( uint128_t( comment.view_count ) >= ( comment_metrics.median_view_count / 10 ),
      "Referred recent Post should have at least 10% of median number of views." );
   FC_ASSERT( uint128_t( comment.vote_power.value ) >= ( comment_metrics.median_vote_power / 10 ),
      "Referred recent Post should have at least 10% of median vote power." );
   FC_ASSERT( uint128_t( comment.view_power.value ) >= ( comment_metrics.median_view_power / 10 ),
      "Referred recent Post should have at least 10% of median view power." );
   FC_ASSERT( now <= ( comment.created + fc::days(1) ),
      "Recent Post should have been made in the last 24 hours." );
   FC_ASSERT( now <= ( view.created + fc::days(1) ),
      "Most Recent View should have been made in the last 24 hours." );
   FC_ASSERT( now <= ( vote.created + fc::days(1) ),
      "Most Recent Vote should have been made in the last 24 hours." );

   const auto& vote_idx = _db.get_index< producer_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, 1 ) );     // Gets top voted producer of account.
   const producer_object& producer = _db.get_producer( vote_itr->producer );

   _db.claim_activity_reward( account, producer, comment.reward_currency );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }

} } // node::chain