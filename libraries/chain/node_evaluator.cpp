#include <node/chain/node_evaluator.hpp>
#include <node/chain/database.hpp>
#include <node/chain/custom_operation_interpreter.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/witness_objects.hpp>
#include <node/chain/block_summary_object.hpp>
#include <cmath>

#include <node/chain/util/reward.hpp>

#ifndef IS_LOW_MEM
#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>

using boost::locale::conv::utf_to_utf;

std::wstring utf8_to_wstring(const std::string& str)
{
    return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
}

std::string wstring_to_utf8(const std::wstring& str)
{
    return utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
}

#endif

#include <fc/uint128.hpp>
#include <fc/utf8.hpp>

#include <limits>

namespace node { namespace chain {
   using fc::uint128_t;


//============================//
// === Account Evaluators === //
//============================//


inline void validate_permlink( const string& permlink )
{
   FC_ASSERT( permlink.size() > MIN_PERMLINK_LENGTH && permlink.size() < MAX_PERMLINK_LENGTH, "Permlink is not a valid size." );

   for( auto c : permlink )
   {
      switch( c )
      {
         case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
         case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
         case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z': case '0':
         case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
         case '-':
            break;
         default:
            FC_ASSERT( false, "Invalid permlink character: ${s}", ("s", std::string() + c ) );
      }
   }
}


struct strcmp_equal
{
   bool operator()( const shared_string& a, const string& b )
   {
      return a.size() == b.size() || std::strcmp( a.c_str(), b.c_str() ) == 0;
   }
};


void account_create_evaluator::do_apply( const account_create_operation& o )
{ try {
   const account_name_type& signed_for = o.registrar;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_general(o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   FC_ASSERT( o.account_type == PERSONA || o.account_type == PROFILE || o.account_type == BUSINESS || o.account_type == ANONYMOUS, 
      "Accounts type invalid." );
   FC_ASSERT( o.account_type != VERIFIED, 
      "Accounts cannot be initialized as verified." );

   // Ensure account name is not already in use. 
   const account_object* account_ptr = _db.find_account( o.new_account_name );
   FC_ASSERT( account_ptr == nullptr, 
      "Account with the name: ${n} already exists.", ("n", o.new_account_name) );
   
   const witness_schedule_object& wso = _db.get_witness_schedule();
   // Ensure fee is paid for new account
   FC_ASSERT( o.fee >= asset( wso.median_props.account_creation_fee.amount, SYMBOL_COIN ), 
      "Insufficient Fee: ${f} required, ${p} provided.", ("f", wso.median_props.account_creation_fee )("p", o.fee) );
   const auto& registrar_balance = _db.get_account_balance( o.registrar, SYMBOL_COIN );
   FC_ASSERT( registrar_balance.get_liquid_balance() >= o.fee, 
      "Insufficient balance to create account.", ( "registrar balance", registrar_balance.liquid_balance )( "required", o.fee ) );

   FC_ASSERT( registrar_balance.staked_balance - registrar_balance.delegated_balance - registrar_balance.to_unstake + registrar_balance.total_unstaked >= o.delegation.amount, "Insufficient Stake to delegate to new account.",
               ( "registrar_balance.staked_balance", registrar_balance.staked_balance )
               ( "registrar_balance.delegated_balance", registrar_balance.delegated_balance )( "required", o.delegation ) );

   auto target_delegation = asset( wso.median_props.account_creation_fee.amount * CREATE_ACCOUNT_DELEGATION_RATIO, SYMBOL_COIN );
   auto current_delegation = asset( o.fee.amount * CREATE_ACCOUNT_DELEGATION_RATIO, SYMBOL_COIN ) + o.delegation;

   FC_ASSERT( current_delegation >= target_delegation, "Insufficient Delegation ${f} required, ${p} provided.",
               ("f", target_delegation )
               ( "p", current_delegation )
               ( "account_creation_fee", wso.median_props.account_creation_fee )
               ( "o.fee", o.fee )
               ( "o.delegation", o.delegation ) );

   // Ensure all referenced accounts exist
   
   const account_object& registrar = _db.get_account( o.registrar );

   if( o.recovery_account.size() )
   {
      const account_object& recovery_account = _db.get_account( o.recovery_account );
   }
   if( o.referrer.size() )
   {
      const account_object& referrer = _db.get_account( o.referrer );
   }
   if( o.proxy.size() )
   {
      const account_object& proxy = _db.get_account( o.proxy );
   }
   if( o.governance_account.size() )
   {
      const governance_account_object& governance_account = _db.get_governance_account( o.governance_account );
      FC_ASSERT( governance_account.active, "Governance account is not active, plase select a different governance account." );
   }
   if( o.account_type == PROFILE )          // Profile account validation
   {
      FC_ASSERT( o.governance_account.size() && o.governance_account == registrar.name, 
         "Profile accounts must be registered by its nominated governance account.");
      FC_ASSERT( governance_account.is_approved, 
         "Governance Accounts must be approved by a threshold of network voting subscriptions and witnesses before creating profile accounts.");
   }
   else if( o.account_type == BUSINESS )    // Business account validation
   {
      FC_ASSERT( o.governance_account.size(),
         "Business accounts must have an initial governance account.");
      FC_ASSERT( o.business_type.valid() && o.officer_vote_threshold.valid(),
         "Business accounts must select business type.");
      FC_ASSERT( *o.business_type == OPEN_BUSINESS || 
         *o.business_opts.business_type == PUBLIC_BUSINESS || 
         *o.business_opts.business_type == PRIVATE_BUSINESS,
         "Business accounts type is invalid.");
      FC_ASSERT( *o.officer_vote_threshold >= 0, 
         "Business account requires an officer threshold greater than or equal to 0.");
   }

   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   time_point now = _db.head_block_time();
   
   for( auto& a : o.owner.account_auths )
   {
      _db.get_account( a.first );
   }

   for( auto& a : o.active.account_auths )
   {
      _db.get_account( a.first );
   }

   for( auto& a : o.posting.account_auths )
   {
      _db.get_account( a.first );
   }
   
   _db.adjust_liquid_balance(registrar.name, -o.fee);
   _db.adjust_delegated_balance(registrar.name, o.delegation);

   const account_object& new_account = _db.create< account_object >( [&]( account_object& acc )
   {
      acc.name = o.new_account_name;
      acc.registrar = registrar.name;
      acc.recovery_account = recovery_account.name;
      acc.referrer = referrer.name;
      acc.proxy = proxy.name;
      acc.account_type = o.account_type;

      if( o.governance_account.size )
      {
         a.reset_account = o.governance_account;
         a.recovery_account = o.governance_account;
         a.reset_account_delay = fc::days(7);
      }

      from_string( acc.json, o.json );
      from_string( acc.json_private, o.json_private );
      from_string( acc.details, o.details );
      from_string( acc.url, o.url );

      acc.secure_public_key = o.secure_public_key;
      acc.connection_public_key = o.connection_public_key;
      acc.friend_public_key = o.friend_public_key;
      acc.companion_public_key = o.companion_public_key;

      acc.created = now;
      acc.last_account_update = now;
      acc.last_vote_time = now;
      acc.last_post = now;
      acc.last_root_post = now;
      acc.last_transfer = now;
      acc.last_owner_proved = now;
      acc.last_active_proved = now;
      acc.last_activity_reward = now;
      acc.last_account_recovery = now;
      acc.membership_expiration = time_point::min();
      acc.mined = false;
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
      vdo.min_delegation_time = now + CREATE_ACCOUNT_DELEGATION_TIME;
   });
   
   _db.create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = o.new_account_name;
      auth.owner = o.owner;
      auth.active = o.active;
      auth.posting = o.posting;
      auth.last_owner_update = fc::time_point::min();
   });

   _db.create<account_following_object>( [&]( account_following_object& afo ) 
   {
      afo.account = o.new_account_name;
      afo.last_follow_update = fc::time_point::min();   
   }); 

   if( o.account_type == BUSINESS )
   {
      _db.create< account_business_object >( [&]( account_business_object& abo )
      {
         abo.account = o.new_account_name;
         abo.business_type = *o.business_type;
         abo.executive_board.CHIEF_EXECUTIVE_OFFICER = o.registrar;
         abo.officer_vote_threshold = *o.officer_vote_threshold;
      });
      _db.create< account_officer_vote_object >( [&]( account_officer_vote_object& aovo )
      {
         aovo.account = o.registrar;
         aovo.business_account = o.new_account_name;
         aovo.officer_account = o.registrar;
      });
      _db.create< account_executive_vote_object >( [&]( account_executive_vote_object& aevo )
      {
         aevo.account = o.registrar;
         aevo.business_account = o.new_account_name;
         aevo.executive_account = o.registrar;
         aevo.role = CHIEF_EXECUTIVE_OFFICER;
      });
   }

   if( o.governance_account.size() )
   {
      _db.create<governance_subscription_object>( [&]( governance_subscription_object& gso ) 
      {
         gso.governance_account = o.governance_account;
         gso.account = o.new_account_name;
      });

      _db.modify( new_account, [&]( account_object& a ) 
      {
         a.governance_subscriptions++;
      });
   }
   if( find_network_officer( o.registrar ) != nullptr )
   {
      _db.create<network_officer_vote_object>( [&]( network_officer_vote_object& novo ) 
      {
         novo.network_officer = o.registrar;
         novo.account = o.new_account_name;
      });

      _db.modify( new_account, [&]( account_object& a ) 
      {
         a.officer_votes++;
      });
   }
   if( find_executive_board( o.registrar ) != nullptr )
   {
      _db.create<executive_board_vote_object>( [&]( executive_board_vote_object& ebvo ) 
      {
         ebvo.network_officer = o.registrar;
         ebvo.account = o.new_account_name;
      });

      _db.modify( new_account, [&]( account_object& a ) 
      {
         a.officer_votes++;
      });
   }
   if( find_witness( o.registrar ) != nullptr )
   {
      _db.create<witness_vote_object>( [&]( witness_vote_object& wvo ) 
      {
         wvo.network_officer = o.registrar;
         wvo.account = o.new_account_name;
         wvo.vote_rank = 1;
      });

      _db.modify( new_account, [&]( account_object& a ) 
      {
         a.officer_votes++;
      });
   }
   
} FC_CAPTURE_AND_RETHROW( (o) ) }


void account_update_evaluator::do_apply( const account_update_operation& o )
{ try {
   FC_ASSERT( o.account != TEMP_ACCOUNT, "Cannot update temp account." );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_chief( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   time_point now = _db.head_block_time();
   if( o.posting ) 
   {
      o.posting->validate();
   }
   const auto& account = _db.get_account( o.account );
   const auto& account_auth = _db.get< account_authority_object, by_account >( o.account );

   if( o.owner )
   {
#ifndef IS_TEST_NET
     FC_ASSERT( now - account_auth.last_owner_update > OWNER_UPDATE_LIMIT, 
      "Owner authority can only be updated once an hour." );
#endif
   
   for( auto a: o.owner->account_auths )
   {
      _db.get_account( a.first );
   }
   
   _db.update_owner_authority( account, *o.owner );
   }

   if( o.active )
   {
      for( auto a: o.active->account_auths )
      {
         _db.get_account( a.first );
      }
   }

   if( o.posting )
   {
      for( auto a: o.posting->account_auths )
      {
         _db.get_account( a.first );
      }
   }

   _db.modify( account, [&]( account_object& acc )
   {
      if( o.secure_public_key != public_key_type() )
      {
         acc.secure_public_key = o.secure_public_key;
      }
      if( o.connection_public_key != public_key_type() )
      {
         acc.connection_public_key = o.connection_public_key;
      }
      if( o.friend_public_key != public_key_type() )
      {
         acc.friend_public_key = o.friend_public_key;
      }
      if( o.companion_public_key != public_key_type() )
      {
         acc.companion_public_key = o.companion_public_key;
      }

      acc.last_account_update = now;

      if ( o.json.size() > 0 )
      {
         from_string( acc.json, o.json );
      }
      if ( o.details.size() > 0 )
      {
         from_string( acc.details, o.details );
      }
      if ( o.url.size() > 0 )
      {
         from_string( acc.url, o.url );
      }
      if ( o.json_private.size() > 0 )
      {
         from_string( acc.json_private, o.json_private );
      }       
   });

   if( o.active || o.posting )
   {
      _db.modify( account_auth, [&]( account_authority_object& auth)
      {
         if( o.active )  
         {  
            auth.active  = *o.active;
         }
         if( o.posting ) 
         {  
            auth.posting = *o.posting;
         }
      });
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void account_membership_evaluator::do_apply( const account_membership_operation& o )
{ try {
   FC_ASSERT( o.account != TEMP_ACCOUNT, 
      "Cannot create membership for temp account." );
   FC_ASSERT( o.months >= 1 && o.months <= 120, 
      "Months of membership purchased must be between 1, and 120.", ("m", o.months ) );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_object& int_account = _db.get_account( o.interface );
   const interface_object& interface = _db.get_interface( o.interface );
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   time_point now = props.time;
   asset liquid = _db.get_liquid_balance( o.account, SYMBOL_COIN );
   asset monthly_fee = asset(0, SYMBOL_USD );

   switch( o.membership_type )
   {
      case NONE:
      {
         break; 
      }
      case STANDARD_MEMBERSHIP:
      {
         monthly_fee = props.membership_base_price;
         break; 
      }
      case MID_MEMBERSHIP:
      {
         monthly_fee = props.membership_mid_price;
         break; 
      }
      case TOP_MEMBERSHIP:
      {
         monthly_fee = props.membership_top_price;
         break; 
      }
      default:
      {
         FC_ASSERT( false, "Membership type Invalid: ${m}.", ("m", o.membership_type ) ); 
         break;
      }
   }

   asset carried_fees = asset(0, SYMBOL_USD );
   fc::microseconds remaining = account.membership_expiration - now;

   switch( account.membership_type )
   {
      case NONE:
      {
         break; 
      }
      case STANDARD_MEMBERSHIP:
      {
         carried_fees = asset( ( props.membership_base_price * remaining.count() ) / fc::days( 30 ).count(), SYMBOL_USD );
         break; 
      }
      case MID_MEMBERSHIP:
      {
         carried_fees = asset( ( props.membership_mid_price * remaining.count() ) / fc::days( 30 ).count(), SYMBOL_USD );
         break; 
      }
      case TOP_MEMBERSHIP:
      {
         carried_fees = asset( ( props.membership_top_price * remaining.count() ) / fc::days( 30 ).count(), SYMBOL_USD );
         break; 
      }
      default:
      {
         FC_ASSERT( false, "Membership type Invalid: ${m}.", ("m", account.membership_type ) );
         break;
      }
   }

   asset total_fees = std::max( asset(0, SYMBOL_USD) , monthly_fee * o.months - carried_fees);   // Applies a discount on new membership if an existing membership is still active.

   FC_ASSERT( liquid >= total_fees, 
      "Account has insufficent liquid balance to pay for the requested membership duration." );

   if( total_fees.amount > 0)
   {
      _db.pay_membership_fees( account, total_fees, int_account );      // Pays splits to interface, premium partners, and network
   }

   _db.modify( account, [&]( account_object& a )
   {
      a.membership = o.membership_type;
      a.membership_expiration = now + fc::days( 30 * o.months );
      a.membership_interface = o.interface;
      if( o.recurring )
      {
         a.recurring_membership = o.months;
      }
      else
      {
         a.recurring_membership = 0;
      } 
   });

   // TODO: Premium content credits, and monthly renewal of credits.

} FC_CAPTURE_AND_RETHROW( (o) ) }


void account_vote_executive_evaluator::do_apply( const account_vote_executive_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_vote_executive( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& voter = _db.get_account( o.account );
   const account_object& executive = _db.get_account( o.executive );
   const account_object& business = _db.get_account( o.business_account );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );
   share_type voting_power = _db.get_equity_voting_power( o.account, bus_acc );  
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();

   FC_ASSERT( voting_power > 0, 
      "Account must hold a balance of voting power in the equity assets of the business account in order to vote for executives." );

   switch(o.role)
   {
      case CHIEF_EXECUTIVE_OFFICER:
      case CHIEF_OPERATING_OFFICER:
      case CHIEF_FINANCIAL_OFFICER:
      case CHIEF_DEVELOPMENT_OFFICER:
      case CHIEF_TECHNOLOGY_OFFICER:
      case CHIEF_SECURITY_OFFICER:
      case CHIEF_GOVERNANCE_OFFICER:
      case CHIEF_MARKETING_OFFICER:
      case CHIEF_DESIGN_OFFICER:
      case CHIEF_ADVOCACY_OFFICER:
      break;
      
      default:
      {
         FC_ASSERT( false, "Invalid executive officer role: ${r} found in business account: ${b}.", ("r", o.role)("b", bus_acc.account));
      }
      break;
   }

   if( o.approved )
   {
      FC_ASSERT( voter.can_vote, 
         "Account has declined its voting rights." );
      FC_ASSERT( officer.active, 
         "Network Officer is inactive, and not accepting approval votes at this time." );
      FC_ASSERT( bus_acc.is_authorized_vote_executive( account.name ), 
         "Account: ${a} must be a member of business: ${b} before voting for an Executive.", ("a", o.account)("b", o.business_account));
      FC_ASSERT( bus_acc.is_officer( executive.name ), 
         "Account: ${a} must be an officer of business: ${b} before being voted as Executive.", ("a", o.executive)("b", o.business_account));
   }
   
   const auto& rank_idx = _db.get_index< account_executive_vote_index >().indices().get< by_account_business_role_rank >();
   const auto& executive_idx = _db.get_index< account_executive_vote_index >().indices().get< by_account_business_role_executive >();
   auto rank_itr = rank_idx.find( boost::make_tuple( voter.name, o.business_account, o.role, o.vote_rank ) ); 
   auto executive_itr = executive_idx.find( boost::make_tuple( voter.name, o.business_account, o.role, o.executive_account ) );

   if( o.approved) // Adding or modifying vote
   {
      if( executive_itr == executive_idx.end() && rank_itr == rank_idx.end() ) // No vote for witness or type rank, create new vote.
      {
         _db.create< account_executive_vote_object>( [&]( account_executive_vote_object& v )
         {
            v.account = voter.name;
            v.vote_rank = o.vote_rank;
            v.executive_account = o.executive_account;
            v.role = o.role;
         });
         
         _db.update_account_executive_votes( voter, bus_acc );
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
            remove( *executive_itr );
         }

         _db.update_account_executive_votes( voter, bus_acc, o.executive_account, o.role, o.vote_rank );
      }
   }
   else  // Removing existing vote
   {
      if( executive_itr != executive_idx.end() )
      {
         remove( *executive_itr );
      }
      else if( rank_itr != rank_idx.end() )
      {
         remove( *rank_itr );
      }
      _db.update_account_executive_votes( voter, bus_acc );
   }

   _db.update_business_account( bus_acc, props );   // updates the voting status of the business account to reflect all voting changes.

} FC_CAPTURE_AND_RETHROW( (o)) }


void account_vote_officer_evaluator::do_apply( const account_vote_officer_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_vote_officer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& voter = _db.get_account( o.account );
   const account_object& officer = _db.get_account( o.officer );
   const account_object& business = _db.get_account( o.business_account );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );
   share_type voting_power = _db.get_equity_voting_power( o.account, bus_acc );
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();

   FC_ASSERT( voting_power > 0, 
      "Account must hold a balance of voting power in the equity assets of the business account in order to vote for officers." );

   if( o.approved )
   {
      FC_ASSERT( voter.can_vote, 
         "Account has declined its voting rights." );
      FC_ASSERT( officer.active, 
         "Network Officer is inactive, and not accepting approval votes at this time." );
      FC_ASSERT( bus_acc.is_authorized_vote_officer( account.name ), 
         "Account: ${a} must be a member of business: ${b} before voting for an Officer.", ("a", o.account)("b", o.business_account));
      FC_ASSERT( bus_acc.is_member( officer.name ), 
         "Account: ${a} must be an officer of business: ${b} before being voted as Officer.", ("a", o.officer_account)("b", o.business_account));
   }

   const auto& rank_idx = _db.get_index< account_officer_vote_index >().indices().get< by_account_business_rank >();
   const auto& officer_idx = _db.get_index< account_officer_vote_index >().indices().get< by_account_business_officer >();
   auto rank_itr = rank_idx.find( boost::make_tuple( voter.name, o.business_account, o.vote_rank ) ); 
   auto officer_itr = officer_idx.find( boost::make_tuple( voter.name, o.business_account, o.officer_account ) );

   if( o.approved)       // Adding or modifying vote
   {
      if( officer_itr == officer_idx.end() && rank_itr == rank_idx.end() ) // No vote for witness or type rank, create new vote.
      {
         _db.create< account_officer_vote_object>( [&]( account_officer_vote_object& v )
         {
            v.account = voter.name;
            v.vote_rank = o.vote_rank;
            v.officer_account = o.officer_account;
         });
         
         _db.update_account_officer_votes( voter, bus_acc );
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
            remove( *officer_itr );
         }

         _db.update_account_officer_votes( voter, bus_acc, o.officer_account, o.role, o.vote_rank );
      }
   }
   else  // Removing existing vote
   {
      if( officer_itr != officer_idx.end() )
      {
         remove( *officer_itr );
      }
      else if( rank_itr != rank_idx.end() )
      {
         remove( *rank_itr );
      }
      _db.update_account_officer_votes( voter, bus_acc );
   }

   _db.update_business_account( bus_acc, props );   // updates the voting status of the business account to reflect all voting changes.

} FC_CAPTURE_AND_RETHROW( (o)) }


void account_member_request_evaluator::do_apply( const account_member_request_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_request( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   FC_ASSERT( o.message.size() <= MAX_STRING_LENGTH, "Message is too long" );
   const account_object& account = _db.get_account( o.account );
   const account_object& business = _db.get_account( o.business_account );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );
   FC_ASSERT( bus_acc.business_type != PRIVATE_BUSINESS, 
      "Account: ${a} cannot request to join a Private Business: ${b} Membership is by invitation only.", ("a", o.account)("b", o.business_account));

   FC_ASSERT( !bus_acc.is_member( account.name ), 
      "Account: ${a} is already a member of the business: ${b}.", ("a", o.account)("b", o.business_account)); 
   FC_ASSERT( bus_acc.is_authorised_request( account.name ), 
      "Account: ${a} is not authorised to request to join the business account: ${b}.", ("a", o.account)("b", o.business_account));
   time_point now = _db.head_block_time();
   const auto& req_idx = _db.get_index< account_member_request_index >().indices().get< by_account_business >();
   auto itr = req_idx.find( std::make_tuple( o.account, o.business_account ) );

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
} FC_CAPTURE_AND_RETHROW( (o)) }


void account_member_invite_evaluator::do_apply( const account_member_invite_operation& o )
{ try {
   FC_ASSERT( o.account != o.member, 
      "Account: ${a} cannot invite itself to become a member of a Business account: ${b} .", ("a", o.member)("b", o.business_account));
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_invite( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   FC_ASSERT( o.message.size() <= MAX_STRING_LENGTH, 
      "Message is too long" );
   FC_ASSERT( o.encrypted_business_key.size() <= MAX_STRING_LENGTH, 
      "Message is too long" );

   const account_object& account = _db.get_account( o.account );
   const account_object& member = _db.get_account( o.member);
   const account_object& business = _db.get_account( o.business_account );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );

   FC_ASSERT( !bus_acc.is_member( member.name ), 
      "Account: ${a} is already a member of the Business Account: ${b}.", ("a", o.member)("b", o.business_account));
   FC_ASSERT( bus_acc.is_authorized_invite( account.name ), 
      "Account: ${a} is not authorised to send Business account: ${b} membership invitations.", ("a", o.account)("b", o.business_account));
   
   time_point now = _db.head_block_time();
   const auto& inv_idx = _db.get_index< account_member_invite_index >().indices().get< by_member_business >();
   const auto& key_idx = _db.get_index< account_member_key_index >().indices().get< by_member_business >();
   auto itr = inv_idx.find( std::make_tuple( o.member, o.business_account ) );

   if( itr == req_idx.end())    // Invite does not exist yet
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
         from_string( amko.message, o.encrypted_business_key );
      });
   }
   else     // Invite exists and is being deleted.
   {
      FC_ASSERT( !o.invited, "Invite already exists, Invited should be set to false to remove existing Invitation." );
      auto key_itr = key_idx.find( std::make_tuple( o.member, o.business_account ) );
      if( key_itr != key_idx.end() )
      {
         _db.remove( *key_itr );  // Remove the account key 
      }
      _db.remove( *itr );     // Remove the invitation
      
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void account_accept_request_evaluator::do_apply( const account_accept_request_operation& o )
{ try {
   FC_ASSERT( o.account != o.member, 
      "Account: ${a} cannot accept its own join request to a business account: ${b}.", ("a", o.member)("b", o.business_account));
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_invite( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account ); 
   const account_object& member = _db.get_account( o.member );
   const account_object& business = _db.get_account( o.business_account );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );
   FC_ASSERT( !bus_acc.is_member( member.name ), 
      "Account: ${a} is already a member of the business account: ${b}.", ("a", o.member)("b", o.business_account));
   FC_ASSERT( bus_acc.is_authorized_invite( account.name ), 
      "Account: ${a} is not authorized to accept membership requests to the business account: ${b}.", ("a", o.account)("b", o.business_account ) );

   const auto& req_idx = _db.get_index< account_member_request_index >().indices().get< by_member_business >();
   auto itr = req_idx.find( std::make_tuple( o.member, o.business_account ) );

   FC_ASSERT( itr != req_idx.end(), 
      "Business account membership request does not exist." );    // Ensure Request exists

   if(o.accepted)   // Accepting the request, skipped if rejecting
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
         from_string( amko.message, o.encrypted_business_key );    // Create private key for new member.
      });
   }
   _db.remove( *itr );
} FC_CAPTURE_AND_RETHROW( (o)) }


void account_accept_invite_evaluator::do_apply( const account_accept_invite_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account ); 
   const account_object& business = _db.get_account( o.business_account );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );
   FC_ASSERT( !bus_acc.is_member( account.name ), 
      "Account: ${a} is already a member of the business account: ${b}.", ("a", o.account)("b", o.business_account));

   const auto& inv_idx = _db.get_index< account_member_invite_index >().indices().get< by_member_business >();
   auto itr = inv_idx.find( std::make_tuple( o.account, o.business_account ) );
   
   FC_ASSERT( itr != inv_idx.end(), 
      "Business account membership invitation does not exist." );        // Ensure Invitation exists

   const account_member_invite_object& invite = *itr;

   FC_ASSERT( bus_acc.is_authorized_invite( invite.account ), 
      "Account: ${a} is no longer authorised to send business account: ${b} membership invitations.", 
         ("a", invite.account)("b", o.business_account));  // Ensure inviting account is still authorised to send invitations
   
   const auto& key_idx = _db.get_index< account_member_key_index >().indices().get< by_member_business >();
   auto key_itr = key_idx.find( std::make_tuple( invite.member, o.business_account ) );

   FC_ASSERT( key_itr != key_idx.end(), 
      "Business account key for invited account does not exist." );

   if(o.accepted)   // Accepting the request, skipped if rejecting
   {
      _db.modify( bus_acc , [&]( account_business_object& abo )
      {
         abo.members.insert( invite.member.name );
      });
   }
   _db.remove( invite );
} FC_CAPTURE_AND_RETHROW( (o)) }


void account_remove_member_evaluator::do_apply( const account_remove_member_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_blacklist( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account ); 
   const account_object& member_acc = _db.get_account( o.member );
   const account_object& business = _db.get_account( o.business_account );
   const account_business_object& bus_acc = _db.get_account_business( o.business_account );

   FC_ASSERT( bus_acc.is_member( member_acc.name ), 
      "Account: ${a} is not a member of business: ${b}.", ("a", o.member)("b", o.business_account));
   
   FC_ASSERT( !bus_acc.is_executive( member_acc.name ), 
      "Account: ${a} cannot be removed while an executive of business: ${b}.", ("a", o.member)("b", o.business_account));

   if( account.name != member_acc.name )     // Account can remove itself from  membership.  
   {
      FC_ASSERT( bus_acc.is_authorized_blacklist( o.account ), 
         "Account: ${a} is not authorised to remove accounts from the business: ${b}.", ("a", o.account)("b", o.business_account)); 
   }

   const auto& key_idx = _db.get_index< account_member_key_index >().indices().get< by_member_business >();
   auto key_itr = key_idx.find( std::make_tuple( o.member, o.business_account ) );
   
   _db.modify( bus_acc, [&]( account_business_object& abo )
   {
      abo.members.erase(member_acc.name);
      abo.officers.erase(member_acc.name);
   });
   if( key_itr != key_idx.end() )
   {
      const account_member_key_object& key = *key_itr;
      _db.remove( key );
   }
   
} FC_CAPTURE_AND_RETHROW( (o)) }


void account_update_list_evaluator::do_apply( const account_update_list_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_blacklist( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   FC_ASSERT( o.listed_account.is_valid() || o.listed_asset.is_valid(),
      "Operation requires either an account or an asset to update list." );
   
   const account_name_type account_name;
   const asset_symbol_type asset_sym;

   if( o.listed_account.is_valid() )
   {
      FC_ASSERT( o.account != *o.listed_account, 
         "Account: ${a} cannot add or remove itself from its own blacklist or whitelist." );
         account_name = *o.listed_account;
   }
   if( o.listed_asset.is_valid() )
   {
      const asset_object& asset_obj = _db.get_asset( *o.listed_asset );
      FC_ASSERT( asset_obj.issuer != o.account, 
         "Account: ${a} cannot add an asset it is the issuer of to its own blacklist or whitelist." );
      asset_sym = *o.listed_asset;
   }
   
   const account_object& account = _db.get_account( o.account ); 
   const account_permission_object& perm = _db.get_account_permissions( o.account );
   if( account.account_type == BUSINESS )
   {
      const account_business_object& bus_acc = _db.get_account_business( o.account ); 
      FC_ASSERT( !bus_acc.is_member( account_name ), 
      "Account: ${a} cannot be blacklisted while a member of business: ${b}. Remove them first.", ("a", account_name)("b", o.account));
      FC_ASSERT( !bus_acc.is_officer( account_name ), 
         "Account: ${a} cannot be blacklisted while a officer of business: ${b}. Remove them first.", ("a", account_name)("b", o.account));
      FC_ASSERT( !bus_acc.is_executive( account_name ), 
         "Account: ${a} cannot be blacklisted while an executive of business: ${b}. Remove them first.", ("a", account_name)("b", o.account));
   }
   
   _db.modify( perm, [&]( account_permission_object& apo )
   {
      if(o.blacklisted)
      {
         if( account_name.size() )
         {
            apo.blacklist.insert(account_name);
            apo.whitelist.erase(account_name);
         }
         if( asset_sym.size() )
         {
            apo.blacklist.insert(asset_sym);
            apo.whitelist.erase(asset_sym);
         }
      }
      else if(o.whitelisted)
      {
         if( account_name.size() )
         {
            apo.whitelist.insert(account_name);
            apo.blacklist.erase(account_name);
         }
         if( asset_sym.size() )
         {
            apo.whitelist.insert(asset_sym);
            apo.blacklist.erase(asset_sym);
         }
      }
      else
      {
         if( account_name.size() )
         {
            apo.whitelist.erase(account_name);
            apo.blacklist.erase(account_name);
         }
         if( asset_sym.size() )
         {
            apo.whitelist.erase(asset_sym);
            apo.blacklist.erase(asset_sym);
         }
      }
   });
} FC_CAPTURE_AND_RETHROW( (o)) }


void account_witness_vote_evaluator::do_apply( const account_witness_vote_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& voter = _db.get_account( o.account );
   const witness_object& witness = _db.get_witness( o.witness );
   FC_ASSERT( voter.proxy.size() == 0, 
      "A proxy is currently set, please clear the proxy before voting for a witness." );

   if( o.approved )
   {
      FC_ASSERT( voter.can_vote, 
         "Account has declined its voting rights." );
      FC_ASSERT( witness.active, 
         "Witness is inactive, and not accepting approval votes at this time." );
   }

   const witness_schedule_object& wso = _db.get_witness_schedule();
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   
   const auto& account_rank_idx = _db.get_index< witness_vote_index >().indices().get< by_account_rank >();
   const auto& account_witness_idx = _db.get_index< witness_vote_index >().indices().get< by_account_witness >();
   auto rank_itr = account_rank_idx.find( boost::make_tuple( voter.name, o.vote_rank) );   // vote at rank number
   auto witness_itr = account_witness_idx.find( boost::make_tuple( voter.name, witness.name ) );    // vote for specified witness

   if( o.approved) // Adding or modifying vote
   {
      if( witness_itr == account_witness_idx.end() && rank_itr == account_rank_idx.end() ) // No vote for witness or rank
      {
         FC_ASSERT( voter.witnesses_voted_for < MAX_ACC_WITNESS_VOTES, 
            "Account has voted for too many witnesses." );

         _db.create<witness_vote_object>( [&]( witness_vote_object& v ) 
         {
            v.witness = witness.owner;
            v.account = voter.name;
            v.vote_rank = o.vote_rank;
         });
         
         _db.update_witness_votes( voter );
      }
      else
      {
         if( witness_itr != account_witness_idx.end() && rank_itr != account_rank_idx.end() )
         {
            FC_ASSERT( witness_itr->witness != rank_itr->witness, 
               "Vote at rank is already specified witness." );
         }
         
         if( witness_itr != account_witness_idx.end() )
         {
            remove( *witness_itr );
         }

         _db.update_witness_votes( voter, vote.witness, o.vote_rank );   // Remove existing witness vote, and add at new rank. 
      }
   }
   else  // Removing existing vote
   {
      if( witness_itr != account_witness_idx.end() )
      {
         remove( *witness_itr );
      }
      else if( rank_itr != account_rank_idx.end() )
      {
         remove( *rank_itr );
      }
      _db.update_witness_votes( voter );
   }

   _db.update_witness( witness, wso, props);    // update the voting state of the witness

} FC_CAPTURE_AND_RETHROW( (o) ) }


void account_update_proxy_evaluator::do_apply( const account_update_proxy_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const auto& account = _db.get_account( o.account );
   
   FC_ASSERT( account.proxy != o.proxy, 
      "Proxy must change." );
   FC_ASSERT( account.can_vote, 
      "Account has declined the ability to vote and cannot proxy votes." );

   if( o.proxy.size() ) 
   {
      const auto& new_proxy = _db.get_account( o.proxy );
      flat_set<account_id_type> proxy_chain( { account.id, new_proxy.id } );
      proxy_chain.reserve( MAX_PROXY_RECURSION_DEPTH + 1 );

      /// check for proxy loops and fail to update the proxy if it would create a loop
      auto cprox = &new_proxy;
      while( cprox->proxy.size() != 0 ) 
      {
         const auto next_proxy = _db.get_account( cprox->proxy );
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
      const account_object& old_proxy = get_account( account.proxy );       
      _db.modify( account, [&]( account_object& a ) 
      {
         a.proxy = o.proxy;   // we are clearing the proxy which means we simply update the accounts
      });
      _db.modify( old_proxy, [&]( account_object& a ) 
      {
         a.proxied.erase( o.account );   // Remove name from old proxy
      });
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void request_account_recovery_evaluator::do_apply( const request_account_recovery_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_executive( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account_to_recover = _db.get_account( o.account_to_recover );
   const account_object& account = _db.get_account( o.recovery_account );
   time_point now = _db.head_block_time();

   if ( account_to_recover.recovery_account.length() )   // Make sure recovery matches expected recovery account
      FC_ASSERT( account_to_recover.recovery_account == o.recovery_account, 
         "Cannot recover an account that does not specify the given recovery account." );
   else                                                  // Empty string recovery account defaults to top witness
      FC_ASSERT( _db.get_index< witness_index >().indices().get< by_voting_power >().begin()->owner == o.recovery_account, 
         "Top witness must recover an account with no recovery partner." );

   const auto& recovery_request_idx = _db.get_index< account_recovery_request_index >().indices().get< by_account >();
   auto request = recovery_request_idx.find( o.account_to_recover );

   if( request == recovery_request_idx.end() ) // New Request
   {
      FC_ASSERT( !o.new_owner_authority.is_impossible(), 
         "Cannot recover using an impossible authority." );
      FC_ASSERT( o.new_owner_authority.weight_threshold, 
         "Cannot recover using an open authority." );

      // Check accounts in the new authority exist
      
      for( auto& a : o.new_owner_authority.account_auths )
      {
         _db.get_account( a.first );
      }

      _db.create< account_recovery_request_object >( [&]( account_recovery_request_object& req )
      {
         req.account_to_recover = o.account_to_recover;
         req.new_owner_authority = o.new_owner_authority;
         req.expires = now + ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD;
      });
   }
   else if( o.new_owner_authority.weight_threshold == 0 ) // Cancel Request if authority is open
   {
      _db.remove( *request );
   }
   else // Change Request
   {
      FC_ASSERT( !o.new_owner_authority.is_impossible(), 
         "Cannot recover using an impossible authority." );

      // Check accounts in the new authority exist
      for( auto& a : o.new_owner_authority.account_auths )
      {
         _db.get_account( a.first );
      }
      
      _db.modify( *request, [&]( account_recovery_request_object& req )
      {
         req.new_owner_authority = o.new_owner_authority;
         req.expires = now + ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD;
      });
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void recover_account_evaluator::do_apply( const recover_account_operation& o )
{ try {
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

   FC_ASSERT( found, "Recent authority not found in authority history." );

   _db.remove( *request ); // Remove first, update_owner_authority may invalidate iterator
   _db.update_owner_authority( account, o.new_owner_authority );
   _db.modify( account, [&]( account_object& a )
   {
      a.last_account_recovery = now;
   });
} FC_CAPTURE_AND_RETHROW( (o) ) }


void reset_account_evaluator::do_apply( const reset_account_operation& o )
{ try {
   const account_name_type& signed_for = o.account_to_reset;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_executive( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account_to_reset );
   const account_object& reset_account = _db.get_account( o.reset_account );
   time_point now = _db.head_block_time();

   FC_ASSERT( ( now - account.last_post ) > account.reset_account_delay, "Account must be inactive to be reset." );
   FC_ASSERT( ( now - account.last_root_post ) > account.reset_account_delay, "Account must be inactive to be reset." );
   FC_ASSERT( ( now - account.last_vote_time ) > account.reset_account_delay, "Account must be inactive to be reset." );
   FC_ASSERT( ( now - account.last_active_proved ) > account.reset_account_delay, "Account must be inactive to be reset." );
   FC_ASSERT( ( now - account.last_owner_proved ) > account.reset_account_delay, "Account must be inactive to be reset." );

   FC_ASSERT( account.reset_account == o.reset_account, "Reset account does not match reset account on account." );

   _db.update_owner_authority( account, o.new_owner_authority );
} FC_CAPTURE_AND_RETHROW( (o) ) }


void set_reset_account_evaluator::do_apply( const set_reset_account_operation& op )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_executive( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_object& reset_account = _db.get_account( o.reset_account );

   FC_ASSERT( account.reset_account == o.current_reset_account, "Current reset account does not match reset account on account." );
   FC_ASSERT( account.reset_account != o.reset_account, "Reset account must change." );
   FC_ASSERT( o.reset_account_delay >= 3, "Reset account delay must be at least 3 days." );

   _db.modify( account, [&]( account_object& a )
   {
      a.reset_account = o.reset_account;
      a.reset_account_delay = fc::days(o.days);
   });
} FC_CAPTURE_AND_RETHROW( (o) ) }


void change_recovery_account_evaluator::do_apply( const change_recovery_account_operation& o )
{ try {
   const account_name_type& signed_for = o.account_to_recover;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_executive( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.new_recovery_account ); 
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
} FC_CAPTURE_AND_RETHROW( (o) ) }


void decline_voting_rights_evaluator::do_apply( const decline_voting_rights_operation& o )
{
   FC_ASSERT(o.decline = true, 
      "Did not decline voting rights.");
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_chief( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );

   _db.modify( account [&](account_object acc) 
   {
      acc.can_vote = false;
   });
}


void connection_request_evaluator::do_apply( const connection_request_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account(o.account);
   const account_object& req_account = _db.get_account(o.requested_account);
   FC_ASSERT( o.account != o.requested_account, "Account cannot connect with itself" );
   FC_ASSERT( o.message.size() <= MAX_STRING_LENGTH, "Message is too long" );
   time_point now = _db.head_block_time();

   const auto& req_idx = _db.get_index< connection_request_index >().indices().get< by_account_req >();
   auto req_itr = req_idx.find( boost::make_tuple( account.id, req_account.id ) );

   const account_name_type& account_a_name;
   const account_name_type& account_b_name;

   if(account.id < req_account.id)  // Connection objects are sorted with lowest ID is account A. 
   {
      account_a_name = account.name;
      account_b_name = req_account.name;
   }
   else
   {
      account_b_name = account.name;
      account_a_name = req_account.name;
   }

   const auto& connection_idx = _db.get_index< connection_index >().indices().get< by_accounts >();
   auto con_itr = connection_idx.find( boost::make_tuple( account_a_name, account_b_name ) );

   if( req_itr == req_idx.end() ) // New connection request 
   {
      FC_ASSERT( o.requested, "Request doesn't exist, user must select to request connection with the account." );
      if(con_itr == con_idx.end() )  // No existing connection object
      { 
         FC_ASSERT( o.connection_type == CONNECTION, "First connection request must be of standard Connection type before elevation to higher levels." );

         _db.create<connection_request_object>( [&]( connection_request_object& cro ) 
         {
            cro.requested_account = req_account.id;
            cro.account = account.id;
            cro.connection_type = o.connection_type;
            from_string( cro.message, o.message );
            cro.expire = now + CONNECTION_REQUEST_DURATION;
         });
      }
      else // Connection object found, requesting type change
      {
         const connection_object& connection_obj = *con_itr;
         FC_ASSERT( o.connection_type != connection_obj.connection_type, "Connection of this type already exists, should request a type increase." );
         if( o.connection_type == FRIEND ) 
         {
            FC_ASSERT( now > CONNECTION_REQUEST_DURATION + connection_obj.created, "Friend Connection must wait one week from first connection." );
         }
         else if( o.connection_type == COMPANION ) 
         {
            FC_ASSERT( connection_obj.connection_type == FRIEND, "Companion connection must follow a friend connection." );
            FC_ASSERT( now > CONNECTION_REQUEST_DURATION + connection_obj.created, "Companion Connection must wait one week from Friend connection." );
         }

         _db.create<connection_request_object>( [&]( connection_request_object& cro ) 
         {
            cro.requested_account = req_account.id;
            cro.account = account.id;
            cro.connection_type = o.connection_type;
            from_string( cro.message, o.message );
            cro.expire = now + CONNECTION_REQUEST_DURATION;
         });
      }
   } 
   else // Request exists and is being cancelled.
   { 
      FC_ASSERT( !o.requested, "Connection currently exists, set request to false to cancel." );
 
      _db.remove( *req_itr );
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void connection_accept_evaluator::do_apply( const connection_accept_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account(o.account);
   const account_object& req_account = _db.get_account(o.requesting_account);
   time_point now = _db.head_block_time();
   FC_ASSERT( o.account != o.requesting_account, "Account cannot connect with itself" );
   FC_ASSERT( o.connection_id.size() <= MAX_STRING_LENGTH, "Connection ID is too long" );
   FC_ASSERT( o.encrypted_key.size() <= MAX_STRING_LENGTH, "URL is too long" );

   const account_name_type& account_a_name;
   const account_name_type& account_b_name;

   if(account.id < req_account.id)  // Connection objects are sorted with lowest ID is account A. 
   {
      account_a_name = account.name;
      account_b_name = req_account.name;
   }
   else
   {
      account_b_name = account.name;
      account_a_name = req_account.name;
   }

   const auto& connection_idx = _db.get_index< connection_index >().indices().get< by_accounts >();
   auto con_itr = connection_idx.find( boost::make_tuple( account_a_name, account_b_name, o.connection_type ) );

   const auto& req_idx = _db.get_index< connection_request_index >().indices().get< by_account_req >();
   auto req_itr = req_idx.find( boost::make_tuple( req_account.id, account.id ) );

   const account_following_object& a_following_set = _db.get_account_following( account_a_name );
   const account_following_object& b_following_set = _db.get_account_following( account_b_name );

   if(con_itr == con_idx.end() )  // No existing connection object of that type, creating new connection.
   {
      FC_ASSERT( o.connected, 
         "Connection doesn't exist, must select to connect with account" ); 
      FC_ASSERT( req_itr != req_idx.end(), 
         "Connection Request doesn't exist to accept." );
      const connection_request_object& request = *req_itr;
      FC_ASSERT( o.connection_type == request.connection_type, 
         "Connection request must be of the same level as acceptance" );

      _db.create<connection_object>( [&]( connection_object& co ) 
      {
         co.account_a = account_a_name;
         co.account_b = account_b_name;

         if(account_a_name == account.name) // We're account A
         {
            from_string( co.encrypted_key_a, o.encrypted_key );
         } 
         else // We're account B
         {
            from_string( co.encrypted_key_b, o.encrypted_key );
         }
         co.connection_type = o.connection_type;
         from_string( co.connection_id, o.connection_id );
         co.created = now;
      });

      _db.modify( a_following_set, [&]( const account_following_object& afo )
      {
         if(o.connection_type == CONNECTION )
         {
            afo.connections.insert(account_b_name);
         }
         else if(o.connection_type == FRIEND )
         {
            afo.friends.insert(account_b_name);
         }
         else if(o.connection_type == COMPANION )
         {
            afo.companions.insert(account_b_name);
         }
         afo.last_follow_update = now;
      });

      _db.modify( b_following_set, [&]( const account_following_object& afo )
      {
         if(o.connection_type == CONNECTION )
         {
            afo.connections.insert(account_a_name);
         }
         else if(o.connection_type == FRIEND )
         {
            afo.friends.insert(account_a_name);
         }
         else if(o.connection_type == COMPANION )
         {
            afo.companions.insert(account_a_name);
         }
         afo.last_follow_update = now;
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
            co.account_a = account_a_name;
            co.account_b = account_b_name;

            if(account_a_name == account.name)    // We're account A
            {
               co.encrypted_key_a = o.encrypted_key;
            } 
            else     // We're account B
            {
               co.encrypted_key_b = o.encrypted_key;
            }
         }); 
      }
      else  // Connection object is found, and is being unconnected. 
      {
         _db.modify( a_following_set, [&]( const account_following_object& afo )
         {
            if(o.connection_type == CONNECTION )
            {
               afo.connections.erase(account_b_name);
            }
            else if(o.connection_type == FRIEND )
            {
               afo.friends.erase(account_b_name);
            }
            else if(o.connection_type == COMPANION )
            {
               afo.companions.erase(account_b_name);
            }
            afo.last_follow_update = now;
         });

         _db.modify( b_following_set, [&]( const account_following_object& afo )
         {
            if(o.connection_type == CONNECTION )
            {
               afo.connections.erase(account_a_name);
            }
            else if(o.connection_type == FRIEND )
            {
               afo.friends.erase(account_a_name);
            }
            else if(o.connection_type == COMPANION )
            {
               afo.companions.erase(account_a_name);
            }
            afo.last_follow_update = now;
         });

         _db.remove( connection_obj );
      } 
   }

   _db.update_account_in_feed( o.account, o.requesting_account );
   _db.update_account_in_feed( o.requesting_account, o.account );

} FC_CAPTURE_AND_RETHROW( (o) ) }

/**
 * Enables an account to follow another account by adding it to
 * the account's following object. 
 * Additionally allows for filtering accounts from interfaces.
 */
void account_follow_evaluator::do_apply( const account_follow_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& follower = _db.get_account( o.follower );
   const account_object& following = _db.get_account( o.following );
   const account_following_object& follower_set = _db.find_account_following( o.follower );
   const account_following_object& following_set = _db.find_account_following( o.following );
   time_point now = _db.head_block_time();

   FC_ASSERT( o.follower != o.following, 
      "Account cannot follow itself" );

   if( o.added )
   {
      if( o.followed )    // Creating new follow relation
      {
         _db.modify( follower_set, [&]( account_following_object& afo ) 
         {
            afo.add_following( following.name );
            afo.last_update = now;
         });
         
         _db.modify( following_set, [&]( account_following_object& afo ) 
         {
            afo.add_follower( follower.name );
            afo.last_update = now;
         });

         _db.modify( following, [&]( account_object& a ) 
         {
            a.follower_count++;
         }); 
      }  
      else    // Creating new filter relation
      {
         FC_ASSERT( !follower_set.is_following( following.name ),
            "Cannot filter an account that you follow, unfollow first." );

         _db.modify( follower_set, [&]( account_following_object& afo ) 
         {
            afo.add_filtered( following.name );
            afo.last_update = now;
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
            afo.last_update = now;
         });
         
         _db.modify( following_set, [&]( account_following_object& afo ) 
         {
            afo.remove_follower( follower.name );
            afo.last_update = now;
         });

         _db.modify( following, [&]( account_object& a ) 
         {
            a.follower_count--;
         });
      }  
      else        // Unfiltering
      {
         FC_ASSERT( follower_set.is_filtered( following.name ),
            "Cannot unfilter an account that you do not filter." );

         _db.modify( follower_set, [&]( account_following_object& afo ) 
         {
            afo.remove_filtered( following.name );
            afo.last_update = now;
         });
      }
   }

   _db.update_account_in_feed( o.follower, o.following );
   _db.update_account_in_feed( o.following, o.follower );
   
} FC_CAPTURE_AND_RETHROW( (o) ) }



void tag_follow_evaluator::do_apply( const tag_follow_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& follower = _db.get_account( o.follower );
   const account_following_object& follower_set = _db.find_account_following( o.follower );
   const tag_following_object& tag_set_ptr = _db.find_tag_following( o.tag );
   time_point now = _db.head_block_time();

   if( tag_set_ptr != nullptr )    // Tag follow already exists
   {
      if( o.added )
      {
         if( o.followed )    // Creating new follow relation
         {
            _db.modify( follower_set, [&]( account_following_object& afo ) 
            {
               afo.add_following( o.tag );
               afo.last_update = now;
            });
            
            _db.modify( *tag_set_ptr, [&]( tag_following_object& tfo ) 
            {
               tfo.add_follower( follower.name );
               tfo.last_update = now;
            });

         }  
         else    // Creating new filter relation
         {
            FC_ASSERT( !follower_set.is_following( o.tag ),
               "Cannot filter a tag that you follow, unfollow first." );

            _db.modify( follower_set, [&]( account_following_object& afo ) 
            {
               afo.add_filtered( o.tag );
               tfo.last_update = now;
            });
         }
      }
      else
      {
         if( o.followed )    // Unfollowing the tag
         {
            FC_ASSERT( follower_set.is_following( o.tag ),
               "Cannot unfollow a tag that you do not follow." );

            _db.modify( follower_set, [&]( account_following_object& afo ) 
            {
               afo.remove_following( o.tag );
               tfo.last_update = now;
            });
            
            _db.modify( *tag_set_ptr, [&]( tag_following_object& tfo ) 
            {
               afo.remove_follower( follower.name );
               tfo.last_update = now;
            });
         }  
         else        // Unfiltering
         {
            FC_ASSERT( follower_set.is_filtered( o.tag ),
               "Cannot unfilter a tag that you do not filter." );

            _db.modify( follower_set, [&]( account_following_object& afo ) 
            {
               afo.remove_filtered( o.tag );
            });
         }
      }
   }

   _db.update_tag_in_feed( o.follower, o.tag );
   
} FC_CAPTURE_AND_RETHROW( (o) ) }



void activity_reward_evaluator::do_apply( const activity_reward_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_officer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   time_point now = _db.head_block_time();
   const account_object& account = _db.get_account(o.account);
   FC_ASSERT( account.witnesses_voted_for >= MIN_ACTIVITY_WITNESSES, 
      "Account must have at least 10 witness votes to claim activity reward.");
   const account_object& balance = _db.get_account_balance(o.account, SYMBOL_EQUITY);
   FC_ASSERT( balance.staked_balance >= asset(1 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY), 
      "Account must have at least one equity asset to claim activity reward");
   const comment_metrics_object& comment_metrics = _db.get_comment_metrics();
   
   const comment_object& comment = _db.get_comment(o.account, o.permlink);
   const comment_view_object& view = _db.get_comment_view(o.account, o.view_id);
   const comment_vote_object& vote = _db.get_comment_vote(o.account, o.vote_id);
   FC_ASSERT(comment.net_votes >= comment_metrics.median_vote_count, 
      "Referred recent Post should have at least the median number of votes for activity reward");
   FC_ASSERT(now < acccount.last_activity_reward + fc::days(1), 
      "Can only claim activity reward once per 24 hours");
   FC_ASSERT(now < comment.created + fc::days(1), 
      "Referred Recent Post should have been made in the last 24 hours");
   FC_ASSERT(now < view.created + fc::days(1), 
      "Referred Recent View should have been made in the last 24 hours");
   FC_ASSERT(now < vote.created + fc::days(1), 
      "Referred Recent Vote should have been made in the last 24 hours");

   const auto& vote_idx = _db.get_index< witness_vote_index >().indices().get< by_account_witness >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, 1 ) ); // Gets top voted witness of account.

   const witness_object& witness = get_witness( vote_itr->witness );

   _db.claim_activity_reward( account, witness );

} FC_CAPTURE_AND_RETHROW( (o) ) }


//============================//
// === Network Evaluators === //
//============================//


/**
 * Creates or updates a network officer for a member account, 
 * Network officers can earn from the reward pools for each type
 * by providing services to the protocol, and receiving votes from
 * network stakeholders. The top 50 members of each pool earn a proportional share of
 * the reward available each day.
 */
void update_network_officer_evaluator::do_apply( const update_network_officer_operation& o )
{ try {
   FC_ASSERT( o.url.size() + o.details.size() + o.json.size(), "Cannot update Network officer because it does not contain content." );
   FC_ASSERT( o.url.size() <= MAX_URL_LENGTH, "URL is too long" );
   FC_ASSERT( o.details.size() <= MAX_STRING_LENGTH, "Details are too long" );
   if( o.json.size()) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );
   }
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   time_point now = _db.head_block_time();
   const account_object& account = _db.get_account(o.account);
   FC_ASSERT(account.membership != NONE, "Account must be a member to create a network officer.");   

   const network_officer_object* net_off_ptr = _db.find_network_officer( o.account );

   if( net_off_ptr != nullptr ) // updating existing network officer
   { 
      _db.modify( net_off_ptr*, [&]( network_officer_object& noo )
      {
         noo.officer_type = o.officer_type;    // Selects development, marketing or advocacy
         if( o.url.size())
         {
            from_string( noo.url, o.url );
         }
         if( o.details.size())
         {
            from_string( noo.details, o.details );
         }
         if( o.json.size())
         {
            from_string( noo.json, o.json );
         }
         noo.active = o.active;
      });
   }
   else  // create new network officer
   {
      _db.create< network_officer_object >( [&]( network_officer_object& noo )
      {
         noo.account = o.account;
         noo.officer_type = o.officer_type;    // Selects development, marketing or advocacy
         if( o.url.size())
         {
            from_string( noo.url, o.url );
         }
         if( o.details.size())
         {
            from_string( noo.details, o.details );
         }
         if( o.json.size())
         {
            from_string( noo.json, o.json );
         }
         noo.created = now;
         noo.active = true;
      });
   } 
} FC_CAPTURE_AND_RETHROW( (o) ) }


void network_officer_vote_evaluator::do_apply( const network_officer_vote_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& voter = _db.get_account( o.account );
   const account_object& officer_account = _db.get_network_officer( o.network_officer );
   const network_officer_object officer = _db.get_network_officer( o.network_officer );
   const witness_schedule_object& wso = _db.get_witness_schedule();
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();

   if( o.approved )
   {
      FC_ASSERT( voter.can_vote, 
         "Account has declined its voting rights." );
      FC_ASSERT( officer.active, 
         "Network Officer is inactive, and not accepting approval votes at this time." );
   }
   
   const auto& account_type_rank_idx = _db.get_index< network_officer_vote_index >().indices().get< by_account_type_rank >();
   const auto& account_officer_idx = _db.get_index< network_officer_vote_index >().indices().get< by_account_officer >();
   auto type_rank_itr = account_rank_idx.find( boost::make_tuple( voter.name, officer.officer_type, o.vote_rank) );   // vote at type and rank number
   auto account_officer_itr = account_officer_idx.find( boost::make_tuple( voter.name, officer.name ) );    // vote for specified witness

   if( o.approved) // Adding or modifying vote
   {
      if( account_officer_itr == account_officer_idx.end() && type_rank_itr == type_rank_idx.end() ) // No vote for witness or type rank, create new vote.
      {
         FC_ASSERT( voter.officer_votes < MAX_OFFICER_VOTES, 
            "Account has voted for too many network officers." );

         _db.create< network_officer_vote_object>( [&]( network_officer_vote_object& v )
         {
            v.network_officer = officer.account;
            v.officer_type = officer.officer_type;
            v.account = voter.name;
            v.vote_rank = o.vote_rank;
         });
         
         _db.update_network_officer_votes( voter );
      }
      else
      {
         if( account_officer_itr != account_officer_idx.end() && type_rank_itr != type_rank_idx.end() )
         {
            FC_ASSERT( account_officer_itr->network_officer != type_rank_itr->network_officer, 
               "Vote at rank is already specified network officer." );
         }
         
         if( account_officer_itr != account_officer_idx.end() )
         {
            remove( *account_officer_itr );
         }

         _db.update_network_officer_votes( voter, vote.network_officer, officer.officer_type, o.vote_rank );   // Remove existing officer vote, and add at new rank. 
      }
   }
   else  // Removing existing vote
   {
      if( account_officer_itr != account_officer_idx.end() )
      {
         remove( *account_officer_itr );
      }
      else if( type_rank_itr != type_rank_idx.end() )
      {
         remove( *type_rank_itr );
      }
      _db.update_network_officer_votes( voter );
   }

   _db.update_network_officer( officer, wso, props );

} FC_CAPTURE_AND_RETHROW( (o) ) }

 /**
 * Creates or updates an executive board object for a team of developers and employees.
 * Executive boards enable the issuance of network credit asset for operating a
 * multifaceted development and marketing team for the protocol.
 * Executive boards must:
 * 1 - Hold a minimum balance of 100 Equity assets.
 * 2 - Operate active supernode with at least 100 daily active users.
 * 3 - Operate active interface account with at least 100 daily active users.
 * 4 - Operate an active governance account with at least 100 subscribers.
 * 5 - Have at least 3 members or officers that are top 50 network officers, 1 from each role.
 * 6 - Have at least one member or officer that is an active top 50 witness.
 * 
 * Executive boards receive their budget payments when:
 * 1 - They are approved by at least 40 Accounts, with at least 20% of total voting power.
 * 2 - They are approved by at least 10 Witnesses, with at least 20% of total witness voting power.
 * 3 - The Current price of the credit asset is greater than $0.90 USD.
 * 4 - They are in the top 5 voted executive boards. 
 */
void update_executive_board_evaluator::do_apply( const update_executive_board_operation& o )
{ try {
   FC_ASSERT( o.url.size() + o.details.size() + o.json.size(), 
      "Cannot update Executive Board because it does not contain content." );
   FC_ASSERT( o.url.size() <= MAX_URL_LENGTH, 
      "URL is too long" );
   FC_ASSERT( o.details.size() <= MAX_STRING_LENGTH, 
      "Details are too long" );
   FC_ASSERT( o.budget.symbol == SYMBOL_CREDIT , 
      "Executive Budget must be in the network credit asset." );
   if( o.json.size()) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );
   }
   const account_name_type& signed_for = o.account;
   const account_business_object& b = _db.get_account_business( signed_for );
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
   const account_object& account = _db.get_account( o.account );
   const account_object& executive = _db.get_account( o.executive );
   const 
   FC_ASSERT( o.executive == o.account || b.is_authorized_network( o.account ),
      "Account is not authorized to update Executive Board." );
   const governance_account_object& gov_account = _db.get_governance_account( o.executive );
   const interface_object& interface = _db.get_interface( o.executive );
   const supernode_object& supernode = _db.get_supernode( o.executive );
   asset stake = _db.get_staked_balance( account.name, SYMBOL_COIN );

   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   FC_ASSERT( o.budget.amount <= props.max_exec_budget.amount, 
      "Executive board Budget is too high. Please reduce budget." );
   const witness_schedule_object& wso = _db.get_witness_schedule();
   time_point now = props.time;
     
   const executive_board_object* exec_ptr = _db.find_executive_board( o.executive );

   FC_ASSERT( b.officers.size(), "Executive Board requires at least one officer." );

   const executive_officer_set& officers = b.officers;
   FC_ASSERT( officers.CHIEF_EXECUTIVE_OFFICER.size(),
      "Executive board requires chief executive officer.");

   if( exec_ptr != nullptr ) // Updating existing Executive board
   { 
      _db.modify( exec_ptr*, [&]( executive_board_object& ebo )
      {
         ebo.budget = o.budget;
         if( o.url.size())
         {
            from_string( ebo.url, o.url );
         }
         if( o.details.size())
         {
            from_string( ebo.details, o.details );
         }
         if( o.json.size())
         {
            from_string( ebo.json, o.json );
         }
         ebo.active = o.active;
      });
   }
   else  // create new executive board
   {
      // Perform Executive board eligibility checks when creating new board.
      FC_ASSERT( o.active, 
         "Executive board does not exist for this account, set active to true.");
      FC_ASSERT( supernode.active && interface.active && gov_account.active, 
         "Executive board must have an active interface, supernode, and governance account");
      FC_ASSERT( executive.membership == TOP_MEMBERSHIP, 
         "Account must be a top level member to create an Executive Board.");  
      FC_ASSERT( stake.amount >= 100*BLOCKCHAIN_PRECISION, 
         "Must have a minimum stake of 100 Core assets.");
      FC_ASSERT( gov_account.subscriber_count >= 100, 
         "Interface requires 100 daily active users to create Executive board.");
      FC_ASSERT( interface.daily_active_users >= 100 * PERCENT_100, 
         "Interface requires 100 daily active users to create Executive board.");
      FC_ASSERT( supernode.daily_active_users >= 100 * PERCENT_100, 
         "Supernode requires 100 daily active users to create Executive board.");
      
      bool wit_check = false;
      bool dev_check = false;
      bool mar_check = false;
      bool adv_check = false;

      if( wso.is_top_witness( executive.name ))
      {
         wit_check = true;
      }
      const network_officer_object* off_ptr = _db.find_network_officer( executive.name );
      if( off_ptr != nullptr )
      {
         const network_officer_object& officer = *off_ptr;
         switch( officer.officer_type)
         {
            case DEVELOPMENT: 
            {
               dev_check = true;
               continue;
            }
            case MARKETING: 
            {
               mar_check = true;
               continue;
            }
            case ADVOCACY: 
            {
               adv_check = true;
               continue;
            } 
         }
      }

      for( auto name_role : b.executives )
      {
         if( wso.is_top_witness( name_role.first ))
         {
            wit_check = true;
         }
         const account_object& off_account = _db.get_account( name_role.first );
         const network_officer_object* off_ptr = _db.find_network_officer( name_role.first );
         if( off_ptr != nullptr )
         {
            const network_officer_object& officer = *off_ptr;
            switch( officer.officer_type)
            {
               case DEVELOPMENT: 
               {
                  dev_check = true;
                  continue;
               }
               case MARKETING: 
               {
                  mar_check = true;
                  continue;
               }
               case ADVOCACY: 
               {
                  adv_check = true;
                  continue;
               } 
            }
         }  
      }

      for( auto name : b.officers )
      {
         if( wso.is_top_witness( name ))
         {
            wit_check = true;
         }
         const account_object& off_account = _db.get_account( name );
         const network_officer_object* off_ptr = _db.find_network_officer( name );
         if( off_ptr != nullptr )
         {
            const network_officer_object& officer = *off_ptr;
            switch( officer.officer_type)
            {
               case DEVELOPMENT: 
               {
                  dev_check = true;
                  continue;
               }
               case MARKETING: 
               {
                  mar_check = true;
                  continue;
               }
               case ADVOCACY: 
               {
                  adv_check = true;
                  continue;
               } 
            }
         } 
      }

      FC_ASSERT( wit_check && dev_check && mar_check && adv_check, 
         "Executive Board must contain at least one witness, developer, marketer, and advocate.");

      _db.create< executive_board_object >( [&]( executive_board_object& ebo )
      {
         ebo.account = o.executive;
         ebo.budget = o.budget;
         if( o.url.size())
         {
            from_string( ebo.url, o.url );
         }
         if( o.details.size())
         {
            from_string( ebo.details, o.details );
         }
         if( o.json.size())
         {
            from_string( ebo.json, o.json );
         }
         ebo.active = true;
         noo.created = now;
      });
   } 
} FC_CAPTURE_AND_RETHROW( (o) ) }


void executive_board_vote_evaluator::do_apply( const executive_board_vote_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& voter = _db.get_account( o.account );
   const executive_board_object& executive = _db.get_executive_board( o.executive_board );
   const account_object& executive_account = _db.get_account( o.executive_board );
   const witness_schedule_object& wso = _db.get_witness_schedule();
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();

   if( o.approved )
   {
      FC_ASSERT( voter.can_vote, 
         "Account has declined its voting rights." );
      FC_ASSERT( executive.active, 
         "Executive board is inactive, and not accepting approval votes at this time." );
   }
   
   const auto& account_rank_idx = _db.get_index< executive_board_vote_index >().indices().get< by_account_rank >();
   const auto& account_executive_idx = _db.get_index< executive_board_vote_index >().indices().get< by_account_executive >();
   auto account_rank_itr = account_rank_idx.find( boost::make_tuple( voter.name, o.vote_rank ) );   // vote at rank number
   auto account_executive_itr = account_executive_idx.find( boost::make_tuple( voter.name, executive.account ) );    // vote for specified executive board

   if( o.approved )      // Adding or modifying vote
   {
      if( account_executive_itr == account_executive_idx.end() && account_rank_itr == account_rank_idx.end() ) // No vote for executive or rank, create new vote.
      {
         FC_ASSERT( voter.executive_votes < MAX_EXEC_VOTES, 
            "Account has voted for too many Executive Boards." );

         _db.create< executive_board_object>( [&]( executive_board_object& v )
         {
            v.executive_board = executive.account;
            v.account = voter.name;
            v.vote_rank = o.vote_rank;
         });
         
         _db.update_executive_board_votes( voter );
      }
      else
      {
         if( account_executive_itr != account_executive_idx.end() && account_rank_itr != account_rank_idx.end() )
         {
            FC_ASSERT( account_executive_itr->executive_board != account_rank_itr->executive_board, 
               "Vote at rank is already specified Executive Board." );
         }
         
         if( account_executive_itr != account_executive_idx.end() )
         {
            remove( *account_executive_itr );
         }

         _db.update_executive_board_votes( voter, vote.executive_board, o.vote_rank );   // Remove existing officer vote, and add at new rank. 
      }
   }
   else       // Removing existing vote
   {
      if( account_executive_itr != account_executive_idx.end() )
      {
         remove( *account_executive_itr );
      }
      else if( account_rank_itr != account_rank_idx.end() )
      {
         remove( *account_rank_itr );
      }
      _db.update_executive_board_votes( voter );
   }

   _db.update_executive_board( executive, wso, props );

} FC_CAPTURE_AND_RETHROW( (o) ) }


void update_governance_evaluator::do_apply( const update_governance_operation& o )
{ try {
   FC_ASSERT( o.url.size() + o.details.size() + o.json.size(), "Cannot update governance account because it does not contain content." );
   FC_ASSERT( o.url.size() <= MAX_URL_LENGTH, "URL is too long" );
   FC_ASSERT( o.details.size() <= MAX_STRING_LENGTH, "Details are too long" );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_governance( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   time_point now = _db.head_block_time();
   if( o.json.size()) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );
   }
   const account_object& account = _db.get_account(o.account);
   
   FC_ASSERT(account.membership == TOP_MEMBERSHIP, "Account must be a Top level member to create governance account");   

   const governance_account_object* gov_acc_ptr = _db.find_governance_account( o.account );

   if( gov_acc_ptr != nullptr ) // updating existing governance account
   { 
      _db.modify( gov_acc_ptr*, [&]( governance_account_object& gao )
      {
         if( o.url.size())
         {
            from_string( gao.url, o.url );
         }
         if( o.details.size())
         {
            from_string( gao.details, o.details );
         }
         if( o.json.size())
         {
            from_string( gao.json, o.json );
         }
         gao.active = o.active;
      });
   }
   else  // create new governance account
   {
      _db.create< governance_account_object >( [&]( governance_account_object& gao )
      {
         gao.account = o.account;
         if( o.url.size())
         {
            from_string( gao.url, o.url );
         }
         if( o.details.size())
         {
            from_string( gao.details, o.details );
         }
         if( o.json.size())
         {
            from_string( gao.json, o.json );
         }
         gao.created = now;
         gao.active = true;
      });
   } 
} FC_CAPTURE_AND_RETHROW( (o) ) }


void subscribe_governance_evaluator::do_apply( const subscribe_governance_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_governance( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const governance_account_object& gov_account = _db.get_governance_account( o.goveranance_account );
   share_type voting_power = _db.get_voting_power( account );
   const auto& gov_idx = _db.get_index< governance_subscription_index >().indices().get< by_account_governance >();
   auto itr = gov_idx.find( boost::make_tuple( account.name, gov_account.account ) );

   if( itr == gov_idx.end() )    // Account is new subscriber
   { 
      FC_ASSERT( o.subscribe, "subscription doesn't exist, user must select to subscribe to the account." );
      FC_ASSERT( account.governance_subscriptions < MAX_GOV_ACCOUNTS, "Account has too many governance subscriptions." );

      _db.create<governance_subscription_object>( [&]( governance_subscription_object& gso ) 
      {
         gso.governance_account = gov_account.owner;
         gso.account = account.name;
      });
   
      _db.modify( account, [&]( account_object& a ) 
      {
         a.governance_subscriptions++;
      });

   } 
   else  // Account is already subscribed and is unsubscribing
   { 
      FC_ASSERT( !o.subscribe, "Subscription currently exists, user must select to unsubscribe." );
       
      _db.modify( account, [&]( account_object& a ) 
      {
         a.governance_subscriptions--;
      });
      _db.remove( *itr );
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void update_interface_evaluator::do_apply( const update_interface_operation& o )
{ try {
   FC_ASSERT( o.url.size() + o.details.size() + o.json.size(), "Cannot update Interface because it does not contain content." );
   FC_ASSERT( o.url.size() <= MAX_URL_LENGTH, "URL is too long" );
   FC_ASSERT( o.details.size() <= MAX_STRING_LENGTH, "Details are too long" );
   if( o.json.size()) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );
   }
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const dynamic_global_property_object props = _db.get_dynamic_global_properties();
   time_point now = props.time;
   FC_ASSERT(account.membership != NONE, 
      "Account must be a member to create an Interface.");   

   const interface_object* int_ptr = _db.find_interface( o.account );

   if( int_ptr != nullptr ) // updating existing interface
   { 
      _db.modify( int_ptr*, [&]( interface_object& i )
      { 
         if( o.url.size())
         {
            from_string( i.url, o.url );
         }
         if( o.details.size())
         {
            from_string( i.details, o.details );
         }
         if( o.json.size())
         {
            from_string( i.json, o.json );
         }
         i.active = o.active;
      });
   }
   else  // create new interface
   {
      _db.create< interface_object >( [&]( interface_object& i )
      {
         i.account = o.account;
         if( o.url.size())
         {
            from_string( i.url, o.url );
         }
         if( o.details.size())
         {
            from_string( i.details, o.details );
         }
         if( o.json.size())
         {
            from_string( i.json, o.json );
         }
         i.active = true;
         i.created = now;
         i.last_user_update = now;
      });
   } 
} FC_CAPTURE_AND_RETHROW( (o) ) }


void update_supernode_evaluator::do_apply( const update_supernode_operation& o )
{ try {
   FC_ASSERT( o.url.size() + o.details.size() + o.json.size(), "Cannot update supernode because it does not contain content." );
   FC_ASSERT( o.url.size() <= MAX_URL_LENGTH, "URL is too long" );
   FC_ASSERT( o.node_api_endpoint.size() <= MAX_URL_LENGTH, "Node API URL is too long" );
   FC_ASSERT( o.auth_api_endpoint.size() <= MAX_URL_LENGTH, "Auth API URL is too long" );
   FC_ASSERT( o.notification_api_endpoint.size() <= MAX_URL_LENGTH, "Notification API URL is too long" );
   FC_ASSERT( o.ipfs_endpoint.size() <= MAX_URL_LENGTH, "IPFS URL is too long" );
   FC_ASSERT( o.bittorrent_endpoint.size() <= MAX_URL_LENGTH, "Bittorrent URL is too long" );
   FC_ASSERT( o.details.size() <= MAX_STRING_LENGTH, "Details are too long" );
   if( o.json.size()) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );
   }
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const dynamic_global_property_object props = _db.get_dynamic_global_properties();
   time_point now = props.time;
   const account_object& account = _db.get_account( o.account );  
   const supernode_object* sup_ptr = _db.find_supernode( o.account );

   if( sup_ptr != nullptr ) // updating existing supernode
   { 
      _db.modify( sup_ptr*, [&]( supernode_object& s )
      { 
         if( o.url.size())
         {
            from_string( s.url, o.url );
         }
         if( o.ipfs_endpoint.size())
         {
            from_string( s.ipfs_endpoint, o.ipfs_endpoint );
         }
         if( o.node_api_endpoint.size())
         {
            from_string( s.node_api_endpoint, o.node_api_endpoint );
         }
         if( o.notification_api_endpoint.size())
         {
            from_string( s.notification_api_endpoint, o.notification_api_endpoint );
         }
         if( o.auth_api_endpoint.size())
         {
            from_string( s.auth_api_endpoint, o.auth_api_endpoint );
         }
         if( o.bittorrent_endpoint.size())
         {
            from_string( s.bittorrent_endpoint, o.bittorrent_endpoint );
         }
         if( o.details.size())
         {
            from_string( s.details, o.details );
         }
         if( o.json.size())
         {
            from_string( s.json, o.json );
         }
         s.active = o.active;
         if( s.active == false && o.active == true )   // Reactivating Supernode. 
         {
            s.last_activation_time = now;
         }
      });
   }
   else  // create new supernode
   {
      _db.create< supernode_object >( [&]( supernode_object& s )
      {
         s.account = o.account;
         if( o.url.size())
         {
            from_string( s.url, o.url );
         }
         if( o.ipfs_endpoint.size())
         {
            from_string( s.ipfs_endpoint, o.ipfs_endpoint );
         }
         if( o.node_api_endpoint.size())
         {
            from_string( s.node_api_endpoint, o.node_api_endpoint );
         }
         if( o.notification_api_endpoint.size())
         {
            from_string( s.notification_api_endpoint, o.notification_api_endpoint );
         }
         if( o.auth_api_endpoint.size())
         {
            from_string( s.auth_api_endpoint, o.auth_api_endpoint );
         }
         if( o.bittorrent_endpoint.size())
         {
            from_string( s.bittorrent_endpoint, o.bittorrent_endpoint );
         }
         if( o.details.size())
         {
            from_string( s.details, o.details );
         }
         if( o.json.size())
         {
            from_string( s.json, o.json );
         }
         s.active = true;
         s.created = now;
         s.last_view_weight_update = now;
         s.last_user_update = now;
         s.last_activation_time = now;
      });
   } 
} FC_CAPTURE_AND_RETHROW( (o) ) }


void create_community_enterprise_evaluator::do_apply( const create_community_enterprise_operation& o )
{ try {
   FC_ASSERT( o.url.size() + o.details.size() + o.json.size(), 
      "Cannot update supernode because it does not contain content." );
   FC_ASSERT( o.url.size() <= MAX_URL_LENGTH, 
      "URL is too long." );
   FC_ASSERT( o.enterprise_id.size() <= MAX_STRING_LENGTH, 
      "Enterpise ID is too long." );
   FC_ASSERT( o.details.size() <= MAX_STRING_LENGTH, 
      "Details are too long." );
   FC_ASSERT( o.daily_budget.symbol == SYMBOL_COIN, 
      "Daily Budget must be in Core Asset." );
   FC_ASSERT( o.daily_budget.amount > 0, 
      "Daily Budget must specify a budget amount greater than 0." );
   FC_ASSERT( o.duration > 0, 
      "Community Enterprise must last for at least one day." );
   const account_name_type& signed_for = o.creator;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   share_type daily_budget_total = ( BLOCK_REWARD.amount * BLOCKS_PER_DAY * COMMUNITY_FUND_PERCENT) / PERCENT_100 ;
   FC_ASSERT( o.daily_budget.amount < daily_budget_total, 
      "Daily Budget must specify a budget less than the total amount of funds available per day." );
   if( o.json.size()) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), 
         "JSON Metadata must be UTF-8" );
   }
   FC_ASSERT( o.milestones.size() >= 2, 
         "Proposal must have at least 2 milestones." );
   uint16_t milestone_sum = 0;
   for( auto milestone : o.milestones )
   {
      milestone_sum += milestone.second;
   }
   FC_ASSERT( milestone_sum == PERCENT_100, 
      "Milestone Sum must equal 100 percent." );
   
   if( o.proposal_type == FUNDING )
   {
      FC_ASSERT( o.beneficiaries.size() >= 1, 
         "Funding Proposal must have at least 1 beneficiary." );
      uint16_t beneficiary_sum = 0;
      for( auto beneficiary : o.beneficiaries )
      {
         beneficiary_sum += beneficiary.second;
      }
      FC_ASSERT( beneficiary_sum == PERCENT_100, 
         "Beneficiary Sum must equal 100 percent." );
   }
   else if( o.proposal_type == INVESTMENT )
   {
      FC_ASSERT( o.beneficiaries.size() == 0, 
         "Investment Proposal should not specify account beneficiaries." );
      FC_ASSERT( o.investment.valid(), 
         "Investment proposal should specify an asset to invest in." );
      asset_symbol_type inv_asset = *o.investment;
      const asset_object& asset = _db.get_asset( inv_asset );
   }

   const dynamic_global_property_object props = _db.get_dynamic_global_properties();
   time_point now = props.time;
   const account_object& account = _db.get_account( o.creator );
   const community_enterprise_object* ent_ptr = _db.find_community_enterprise( account.name, o.enterprise_id );

   if( ent_ptr != nullptr ) // Updating or removing existing proposal 
   { 
      const community_enterprise_object& enterprise = *ent_ptr;
      if (enterprise.approved_milestones == -1 )
      {
         FC_ASSERT( o.begin  > ( enterprise.created + fc::days(7) ), 
            "Begin time must be at least 7 days after creation." );
         
         uint16_t milestone_sum = 0;
         for( auto milestone : o.milestones )
         {
            milestone_sum += milestone.second;
         }
         FC_ASSERT( milestone_sum == PERCENT_100, 
            "Milestone Sum must equal 100 percent." );

         if( o.proposal_type == COMPETITION )
         {
            FC_ASSERT( o.beneficiaries.size() == 0, 
               "Competition Proposal must not specifiy winner beneficiaries before starting new proposal." );
         }
      }
      _db.modify( ent_ptr*, [&]( community_enterprise_object& ceo )
      { 
         if( o.url.size())
         {
            from_string( ceo.url, o.url );
         }
         if( o.details.size())
         {
            from_string( ceo.details, o.details );
         }
         if( o.json.size())
         {
            from_string( ceo.json, o.json );
         }
         ceo.active = o.active;
         if( ceo.approved_milestones == -1 )     // Proposal not yet accepted, able to modify. 
         {
            ceo.proposal_type = o.proposal_type;
            ceo.beneficiaries = o.beneficiaries;
            ceo.milestones = o.milestones;
            ceo.begin = o.begin;
            ceo.duration = o.duration;
            ceo.end = o.begin + fc::days( o.duration );
         }
      });
   }
   else  // Create new community enterprise proposal
   {
      FC_ASSERT( o.begin  > (now + fc::days(7)), 
         "Begin time must be at least 7 days in the future." );
      
      if( o.proposal_type == COMPETITION )
      {
         FC_ASSERT( o.beneficiaries.size() == 0, 
            "Competition Proposal must not specifiy winner beneficiaries before starting new proposal." );
      }

      FC_ASSERT( o.fee.symbol == SYMBOL_COIN && o.fee.amount >= BLOCKCHAIN_PRECISION, 
         "Proposal Requires a fee of 1 Core asset.");
      asset liquid = _db.get_liquid_balance( o.creator, SYMBOL_COIN );
      FC_ASSERT( liquid >= o.fee,
         "Creator has insufficient balance to pay community enterprise proposal fee." );
      _db.adjust_liquid_balance( o.creator, -o.fee );
      const reward_fund_object& reward_fund = _db.get_reward_fund();
      _db.modify( reward_fund, [&]( reward_fund_object& rfo )
      { 
         rfo.adjust_community_fund_balance( o.fee );
      });
      _db.adjust_pending_supply( o.fee );

      _db.create< community_enterprise_object >( [&]( community_enterprise_object& ceo )
      {
         ceo.creator = o.creator;
         from_string( ceo.enterprise_id, o.enterprise_id );
         ceo.proposal_type = o.proposal_type;
         ceo.beneficiaries = o.beneficiaries;
         ceo.milestones = o.milestones;
         ceo.begin = o.begin;
         ceo.end = o.begin + fc::days( o.duration );
         ceo.expiration = ceo.end + fc::days(365);
         ceo.duration = o.duration;
         ceo.daily_budget = o.daily_budget;
         if( o.investment.valid() )
         {
            ceo.investment = *o.investment;
         }
         if( o.url.size())
         {
            from_string( ceo.url, o.url );
         }
         if( o.details.size())
         {
            from_string( ceo.details, o.details );
         }
         if( o.json.size())
         {
            from_string( ceo.json, o.json );
         }
         ceo.active = true;
         ceo.created = now;
      });
   } 
} FC_CAPTURE_AND_RETHROW( (o) ) }


void claim_enterprise_milestone_evaluator::do_apply( const claim_enterprise_milestone_operation& o )
{ try {
   FC_ASSERT( o.enterprise_id.size() <= MAX_STRING_LENGTH, 
      "Enterpise ID is too long." );
   FC_ASSERT( o.details.size() <= MAX_STRING_LENGTH, 
      "Details are too long." );
   const account_name_type& signed_for = o.creator;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& creator = _db.get_account( o.creator );
   
   const community_enterprise_object& enterprise = _db.get_community_enterprise( o.creator, o.enterprise_id );
   FC_ASSERT( o.milestone > enterprise.approved_milestones,
      "Claimed milestone has already been approved." );
   FC_ASSERT( o.milestone != enterprise.claimed_milestones,
      "This operaiton would not change the claimed milestone." );

   const dynamic_global_property_object props = _db.get_dynamic_global_properties();
   const witness_schedule_object& witness_schedule = get_witness_schedule();
   time_point now = props.time;
   shared_string new_history;
   from_string( new_history, o.details );

   _db.modify( enterprise, [&]( community_enterprise_object& ceo )
   { 
      if( new_history.size())
      {
         ceo.milestone_history.push_back( new_history );
      }
      ceo.claimed_milestones = o.milestone;
   });

   _db.update_enterprise( enterprise, witness_schedule, props );
   
} FC_CAPTURE_AND_RETHROW( (o) ) }


void approve_enterprise_milestone_evaluator::do_apply( const approve_enterprise_milestone_operation& o )
{ try {
   FC_ASSERT( o.enterprise_id.size() <= MAX_STRING_LENGTH, 
      "Enterpise ID is too long." );
   FC_ASSERT( o.details.size() <= MAX_STRING_LENGTH, 
      "Details are too long." );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_object& creator = _db.get_account( o.creator );
   const community_enterprise_object& enterprise = _db.get_community_enterprise( o.creator, o.enterprise_id );
   FC_ASSERT( o.milestone <= enterprise.claimed_milestones,
      "Cannot approve milestone that has not yet been claimed by the enterprise creator." );
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   const witness_schedule_object& witness_schedule = get_witness_schedule();
   time_point now = props.time;
   const enterprise_approval_object* approval_ptr = _db.find_enterprise_approval( creator.name, o.enterprise_id, account.name );

   if( approval_ptr != nullptr )      // Updating or removing existing approval
   { 
      const enterprise_approval_object& approval = *approval_ptr;
      FC_ASSERT( o.milestone > approval.milestone, 
         "Update to enterprise milestone approval must support a greater milestone level." );

      if( o.approved )   // Modifying approval details.
      {
         _db.modify( approval, [&]( enterprise_approval_object& emao )
         { 
            emao.milestone = o.milestone;
            emao.last_updated = now;
         });
      }
      else
      {
         _db.remove( approval );
      }
   }
   else  // Create new enterprise milestone approval.
   {
      _db.create< enterprise_approval_object >( [&]( enterprise_approval_object& emao )
      {
         emao.account = o.account;
         emao.creator = o.creator;
         emao.enterprise_id = o.enterprise_id;
         emao.milestone = o.milestone;
         emao.created = now;
      });
   }

   _db.update_enterprise( enterprise, witness_schedule, props );

} FC_CAPTURE_AND_RETHROW( (o) ) }



//=====================================//
// === Post and Comment Evaluators === //
//=====================================//


/**
 * Creates a new comment object, or edits an existing object with new details.
 * Comments made within a board must be created in a board that the user has the 
 * permission to author a post in. 
*/
void comment_evaluator::do_apply( const comment_operation& o )
{ try {
   const account_name_type& signed_for = o.author;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   time_point now = props.time;
   const auto& by_permlink_idx = _db.get_index< comment_index >().indices().get< by_permlink >();
   auto itr = by_permlink_idx.find( boost::make_tuple( o.author, o.permlink ) );
   const account_object& auth = _db.get_account( o.author );
   const interface_object& interface = _db.get_interface( o.interface );
   const board_object* board_ptr = nullptr;

   if( o.board )     // Board validity and permissioning checks
   {
      board_ptr = _db.find_board( o.board );

      FC_ASSERT( board_ptr != nullptr, 
         "Board Name: ${b} not found.", ("b", o.board ));
      const board_object& board = *board_ptr;
      feed_types board_feed_type = BOARD_FEED;
      const board_member_object& board_member = _db.get_board_member( board.name );

      if( comment.parent == ROOT_POST_PARENT )
      {
         FC_ASSERT( board_member.is_authorized_author(o.author, board), 
            "User ${u} is not authorized to post in the board ${b}.",("b", board.name)("u", auth.name));
      }
      else
      {
         FC_ASSERT( board_member.is_authorized_interact( o.author ), 
            "User ${u} is not authorized to interact with posts in the board ${b}.",("b", board.name)("u", auth.name));
      }
      
      switch( board.board_type )
      {
         case BOARD: 
            break;
         case GROUP:
         {
            board_feed_type = GROUP_FEED;
         }
         break;
         case EVENT:
         {
            board_feed_type = EVENT_FEED;
         }
         break;
         case STORE:
         {
            board_feed_type = STORE_FEED;
         }
         break;
         default:
         {
            FC_ASSERT( false, "Invalid board type.");
         }
      }
      
      switch( board.board_privacy )
      {
         case OPEN_BOARD:
         case PUBLIC_BOARD:
         {
            FC_ASSERT( !o.privacy && o.public_key == public_key_type(), 
               "Posts in Open and Public boards should not be encrypted." );
         }
         case PRIVATE_BOARD:
         case EXCLUSIVE_BOARD:
         {
            FC_ASSERT( o.privacy && o.public_key == board.board_public_key, 
               "Posts in Private and Exclusive Boards must be encrypted with the board public key.");
            FC_ASSERT( o.reach == board_feed_type, 
               "Posts in Private and Exclusive Boards should have reach limited to only board level subscribers.");
         }
         break;
         default:
         {
            FC_ASSERT( false, "Invalid Board Privacy type." );
         }
         break;
      }
   }

   switch( o.reach )
   {
      case TAG_FEED:
      case FOLLOW_FEED:
      case MUTUAL_FEED:
      {
         FC_ASSERT( !o.privacy && o.public_key == public_key_type(), 
            "Follow, Mutual and Tag level posts should not be encrypted." );
      }
      break;
      case CONNECTION_FEED:
      {
         FC_ASSERT( o.privacy && o.public_key == account.connection_public_key, 
            "Connection level posts must be encrypted with the account's Connection public key." );
      }
      break;
      case FRIEND_FEED:
      {
         FC_ASSERT( o.privacy && o.public_key == account.friend_public_key, 
            "Connection level posts must be encrypted with the account's Friend public key.");
      }
      break;
      case COMPANION_FEED:
      {
         FC_ASSERT( o.privacy && o.public_key == account.companion_public_key, 
            "Connection level posts must be encrypted with the account's Companion public key.");
      }
      break;
      case BOARD_FEED:
      case GROUP_FEED:
      case EVENT_FEED:
      case STORE_FEED:
      {
         FC_ASSERT( board_ptr != nullptr, 
            "Board level posts must be made within a valid board.");
         FC_ASSERT( o.privacy && o.public_key == board_ptr->board_public_key, 
            "Board level posts must be encrypted with the board public key.");
      }
      break;
      case NO_FEED:
      break;
      default:
      {
         FC_ASSERT( false, "Invalid Post Reach Type." );
      }
   }
   
   uint128_t reward = 0;
   uint128_t weight = 0;
   uint128_t max_weight = 0;
   uint128_t new_comment_power = auth.comment_power;

   comment_id_type id;

   const comment_object* parent = nullptr;
   if( o.parent_author != ROOT_POST_PARENT )
   {
      parent = &_db.get_comment( o.parent_author, o.parent_permlink );
      FC_ASSERT( parent->depth < MAX_COMMENT_DEPTH, 
         "Comment is nested ${x} posts deep, maximum depth is ${y}.", ("x",parent->depth)("y",MAX_COMMENT_DEPTH) );
   }

   if( o.json.size() ) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );
   }
      
   if ( itr == by_permlink_idx.end() )                // Post does not yet exist, creating new post
   {
      if( o.parent_author == ROOT_POST_PARENT )       // Post is a new root post
      {
         FC_ASSERT( ( now - auth.last_root_post ) > MIN_ROOT_COMMENT_INTERVAL, 
            "You may only post once every 60 seconds.", ("now",now)("last_root_post", auth.last_root_post) );
         
      }    
      else         // Post is a new comment
      {
         const comment_object& root = _db.get( parent->root_comment );       // If root post, gets the posts own object.

         FC_ASSERT( root.allow_replies, 
            "The parent comment has disabled replies." );
         FC_ASSERT( ( now - auth.last_post) > MIN_REPLY_INTERVAL, 
            "You may only comment once every 15 seconds.", ("now",now)("auth.last_post",auth.last_post) );
         
         FC_ASSERT( parent->comment_paid( o.author ), 
            "User ${u} has not paid the required comment price: ${p} to reply to this post ${c}.",
            ("c", parent->permlink)("u", auth.name)("p", parent->comment_price));

         const reward_fund_object& reward_fund = _db.get_reward_fund();
         auto curve = reward_fund.curation_reward_curve;
         int64_t elapsed_seconds = ( now - auth.last_post ).to_seconds();
         int16_t regenerated_power = (PERCENT_100 * elapsed_seconds) / props.comment_recharge_time.to_seconds();
         int16_t current_power = std::min( int64_t( auth.comment_power + regenerated_power), int64_t(PERCENT_100) );
         FC_ASSERT( current_power > 0, 
            "Account currently does not have any commenting power." );
         share_type voting_power = _db.get_voting_power( auth.name );
         int16_t max_comment_denom = props.comment_reserve_rate * ( props.comment_recharge_time.count() / fc::days(1).count );  // Weights the viewing power with the network reserve ratio and recharge time
         FC_ASSERT( max_comment_denom > 0 );
         int16_t used_power = (current_power + max_comment_denom - 1) / max_comment_denom;
         new_comment_power = current_power - used_power;
         FC_ASSERT( used_power <= current_power, 
            "Account does not have enough power to comment." );
         
         reward = ( voting_power * used_power) / PERCENT_100;
         
         uint128_t old_power = std::max( root.comment_power, uint128_t(0) );  // Record commentvalue before applying comment

         _db.modify( root, [&]( comment_object& c )
         {
            c.net_reward += reward;
            c.comment_power += reward;
         });

         uint128_t new_power = std::max( root.comment_power, uint128_t(0));   // record new net reward after applying comment
         bool curation_reward_eligible = reward > 0 && root.cashout_time != fc::time_point::maximum() && root.allow_curation_rewards;
         uint16_t curation_reward_percent = reward_fund.curation_reward_percent;
            
         if( curation_reward_eligible )
         {
            uint128_t old_weight = util::evaluate_reward_curve( old_power, curve, reward_fund.content_constant );
            uint128_t new_weight = util::evaluate_reward_curve( new_power, curve, reward_fund.content_constant );       
            uint128_t max_comment_weight = new_weight - old_weight;   // Gets the difference in content reward weight before and after the comment occurs.

            _db.modify( root, [&]( comment_object& c )
            {
               c.total_comment_weight += max_comment_weight;
            });

            uint128_t curation_auction_decay_time = props.curation_auction_decay_time.to_seconds();
            uint128_t w = max_comment_weight;
            uint128_t delta_t = std::min( uint128_t((cv.last_update - comment.created).to_seconds()), curation_auction_decay_time ); 

            w *= delta_t;
            w /= curation_auction_decay_time;                     // Discount weight linearly by time for early comments in the first 10 minutes.

            double curation_decay = props.comment_curation_decay;      // Number of comments for half life of curation reward decay.
            double comment_discount_rate = std::max(( double( comment.children ) / curation_decay), double(0));
            double comment_discount = std::pow(0.5, comment_discount_rate );     // Raises 0.5 to a fractional power for each 100 comments added
            double comment_discount_percent = comment_discount * double(PERCENT_100);
            FC_ASSERT(comment_discount_percent >= 0, "Vote discount percent should not become negative");
            w *= uint128_t(comment_discount_percent);
            w /= PERCENT_100;      // Discount weight exponentially for each successive comment on the post, decaying by 50% per 100 comments.
            weight = w;
            max_weight = max_comment_weight;
         } 
      }

      uint64_t post_bandwidth = auth.post_bandwidth;

      _db.modify( auth, [&]( account_object& a ) 
      {
         if( o.parent_author == ROOT_POST_PARENT )
         {
            a.last_root_post = now;
            a.post_bandwidth = uint32_t( post_bandwidth );
         }
         else
         {
            a.comment_power = new_comment_power;
         }
         a.last_post = now;
         a.post_count++;
      });

      const auto& new_comment = _db.create< comment_object >( [&]( comment_object& com )
      {
         validate_permlink( o.parent_permlink );
         validate_permlink( o.permlink );
         
         com.author = o.author;
         com.rating = o.rating;
         com.board = o.board;
         com.privacy = o.privacy;
         com.reach = o.reach;
         com.post_type = o.post_type;
         com.author_reputation = auth.author_reputation;
         com.comment_price = o.comment_price;
         com.premium_price = o.premium_price;
         from_string( com.public_key, o.public_key);
         from_string( com.ipfs, o.ipfs );
         from_string( com.magnet, o.magnet );
         from_string( com.language, o.language );
         from_string( com.permlink, o.permlink );
         from_string( com.body, o.body );
         from_string( com.json, o.json );
         com.last_update = now;
         com.created = com.last_update;
         com.active = com.last_update;
         com.last_payout = fc::time_point::min();
         com.max_cashout_time = fc::time_point::maximum();
         com.cashout_time = com.created + props.content_reward_interval;

         if ( o.parent_author == ROOT_POST_PARENT )     // New Root post
         {
            com.parent_author = "";
            from_string( com.parent_permlink, o.parent_permlink );
            from_string( com.category, o.parent_permlink );
            com.root_comment = com.id;
         }
         else       // New comment
         {
            com.parent_author = parent->author;
            com.parent_permlink = parent->permlink;
            com.depth = parent->depth + 1;
            com.category = parent->category;
            com.root_comment = parent->root_comment;
            com.reward = reward;
            com.weight = weight;
            com.max_weight = max_weight;
         }
      });

      id = new_comment.id;

      while( parent != nullptr )                 // Increments the children counter on all ancestor comments, and bumps active time.
      {
         _db.modify( *parent, [&]( comment_object& p )
         {
            p.children++;
            p.active = now;
         });

         if( parent->parent_author != ROOT_POST_PARENT )
         {
            parent = &_db.get_comment( parent->parent_author, parent->parent_permlink );
         }
         else
         {
            parent = nullptr;
         }
      }

      // Create feed and blog objects for author account's followers and connections, board followers, and tag followers. 
      _db.add_comment_to_feeds( new_comment );

      if( board_ptr != nullptr )
      {
         _db.modify( *board_ptr, [&]( board_object& bo )
         {
            if ( o.parent_author == ROOT_POST_PARENT )
            {
               bo.post_count++;
               bo.last_root_post = now;
            }
            else 
            {
               bo.comment_count++;
            }
            bo.last_post = now;
         });
      }  
   }
   else           // Post found, editing or deleting existing post. 
   {
      const comment_object& comment = *itr;

      if( !o.deleted )     // Editing post
      {
         feed_types old_reach = comment.reach;

         _db.modify( comment, [&]( comment_object& com )
         {
            com.last_update = now;
            com.active = now;
            com.rating = o.rating;
            com.board = o.board;
            com.privacy = o.privacy;
            com.reach = o.reach;

            strcmp_equal equal;

            if( !parent )
            {
               FC_ASSERT( com.parent_author == account_name_type(), 
                  "The parent of a comment cannot change." );
               FC_ASSERT( equal( com.parent_permlink, o.parent_permlink ), 
                  "The permlink of a comment cannot change." );
            }
            else
            {
               FC_ASSERT( com.parent_author == o.parent_author, 
                  "The parent of a comment cannot change." );
               FC_ASSERT( equal( com.parent_permlink, o.parent_permlink ), 
                  "The permlink of a comment cannot change." );
            }       
            if( o.json.size() && fc::is_utf8( o.json ) )
            {
               from_string( com.json, o.json );  
            }
            if( o.ipfs.size() ) 
            {
               from_string( com.ipfs, o.ipfs);
            } 
            if( o.magnet.size() ) 
            {
               from_string( com.magnet, o.magnet);
            } 
            if( o.language.size() ) 
            {
               from_string( com.language, o.language);
            }
            if( o.public_key.size() ) 
            {
               from_string( com.public_key, o.public_key);
            }
            if( o.body.size() ) 
            { try {
                  diff_match_patch<std::wstring> dmp;
                  auto patch = dmp.patch_fromText( utf8_to_wstring(o.body) );
                  if( patch.size() ) 
                  {
                     auto result = dmp.patch_apply( patch, utf8_to_wstring( to_string( com.body ) ) );
                     auto patched_body = wstring_to_utf8(result.first);
                     if( !fc::is_utf8( patched_body ) ) 
                     {
                        idump(("invalid utf8")(patched_body));
                        from_string( com.body, fc::prune_invalid_utf8(patched_body) );
                     } 
                     else 
                     { 
                        from_string( com.body, patched_body ); 
                     }
                  }
                  else 
                  { // replace
                     from_string( com.body, o.body );
                  }
               } 
               catch ( ... ) 
               {
                  from_string( com.body, o.body );
               }
            }
         });

         if( comment.reach != old_reach )    // If reach has changed, recreate feed objects for author account's followers and connections.
         {
            _db.clear_comment_feeds( comment );
            _db.add_comment_to_feeds( comment );
         } 
      } 
      else
      {
         _db.modify( comment, [&]( comment_object& com )
         {
            com.deleted = true;               // deletes comment, nullifying all possible information.
            com.last_update = fc::time_point::min();
            com.active = fc::time_point::min();
            com.rating = EXPLICIT;
            com.board = board_name_type();
            com.privacy = true;
            com.reach = NO_FEED;
   
            from_string( com.json, string() );  
            from_string( com.ipfs, string() );
            from_string( com.magnet, string() );
            from_string( com.language, string() );
            from_string( com.public_key, string() );
            from_string( com.body, string() );
         });
        
         _db.clear_comment_feeds( comment );
      }
   } // end EDIT case
} FC_CAPTURE_AND_RETHROW( (o) ) }


struct comment_options_extension_visitor
{
   comment_options_extension_visitor( const comment_object& c, database& db ) : _c( c ), _db( db ) {}
   typedef void result_type;
   const comment_object& _c;
   database& _db;
   void operator()( const comment_payout_beneficiaries& cpb ) const
   {
      FC_ASSERT( _c.beneficiaries.size() == 0, "Comment already has beneficiaries specified." );
      FC_ASSERT( _c.net_reward == 0, "Comment must not have been voted on before specifying beneficiaries." );

      _db.modify( _c, [&]( comment_object& c )
      {
         for( auto& b : cpb.beneficiaries )
         {
            auto acc = _db.find< account_object, by_name >( b.account );
            FC_ASSERT( acc != nullptr, "Beneficiary \"${a}\" must exist.", ("a", b.account) );
            c.beneficiaries.push_back( b );
         }
      });
   }
};


void comment_options_evaluator::do_apply( const comment_options_operation& o )
{
   const account_name_type& signed_for = o.author;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& auth = _db.get_account( o.author );
   const auto& comment = _db.get_comment( o.author, o.permlink );

   if( !o.allow_curation_rewards || !o.allow_votes || o.max_accepted_payout < comment.max_accepted_payout )
   {
      FC_ASSERT( comment.net_reward == 0, 
         "One of the included comment options requires the comment to have no reward allocated to it." );
   }
   
   FC_ASSERT( comment.allow_curation_rewards >= o.allow_curation_rewards, 
      "Curation rewards cannot be re-enabled." );
   FC_ASSERT( comment.allow_votes >= o.allow_votes, 
      "Voting cannot be re-enabled." );
   FC_ASSERT( comment.max_accepted_payout >= o.max_accepted_payout, 
      "A comment cannot accept a greater payout." );
   FC_ASSERT( comment.percent_liquid >= o.percent_liquid, 
      "A comment cannot accept a greater percent USD." );

   _db.modify( comment, [&]( comment_object& c ) 
   {
      c.max_accepted_payout = o.max_accepted_payout;
      c.percent_liquid = o.percent_liquid;
      c.allow_votes = o.allow_votes;
      c.allow_curation_rewards = o.allow_curation_rewards;
   });

   for( auto& e : o.extensions )
   {
      e.visit( comment_options_extension_visitor( comment, _db ) );
   }
}


void message_evaluator::do_apply( const message_operation& o )
{ try {
   const account_name_type& signed_for = o.sender;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   FC_ASSERT( o.message.size(), "Message must include a message string." );
   FC_ASSERT( o.message.size() <= MAX_STRING_LENGTH, "Message is too long." );
   FC_ASSERT( fc::is_utf8( o.message ), "JSON Metadata must be UTF-8" );
   if( o.json.size()) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );
   }
   const account_object& sender = _db.get_account( o.sender );
   const account_object& recipient = _db.get_account( o.recipient );
   time_point now = _db.head_block_time();
   
   const account_name_type& account_a_name;
   const account_name_type& account_b_name;

   if(sender.id < recipient.id)        // Connection objects are sorted with lowest ID is account A. 
   {
      account_a_name = sender.name;
      account_b_name = recipient.name;
   }
   else
   {
      account_b_name = sender.name;
      account_a_name = recipient.name;
   }

   const auto& connection_idx = _db.get_index< connection_index >().indices().get< by_accounts >();
   auto con_itr = connection_idx.find( boost::make_tuple( account_a_name, account_b_name, CONNECTION ) );

   FC_ASSERT( con_itr != connection_idx.end(), 
      "Cannot send message: No Connection between Account: ${a} and Account: ${b}", ("a", account_a_name)("b", account_b_name) );

   const auto& message_idx = _db.get_index< message_index >().indices().get< by_sender_uuid >();
   auto message_itr = message_idx.find( boost::make_tuple( sender.name, o.uuid) );

   if( message_itr == message_idx.end() )         // Message uuid does not exist, creating new message
   {
      _db.create< message_object >( [&]( message_object& mo )
      {
         mo.sender = sender.name;
         mo.recipient = recipient.name;
         mo.sender_public_key = sender.secure_public_key;
         mo.recipient_public_key = recipient.secure_public_key;
         from_string( mo.message, o.message);
         from_string( mo.uuid, o.uuid);
         mo.time = now;
         mo.last_updated = now;
      });
   }
   else
   {
      _db.modify( *message_itr, [&]( message_object& mo )
      {
         from_string( mo.message, o.message);
         mo.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void vote_evaluator::do_apply( const vote_operation& o )
{ try {
   const account_name_type& signed_for = o.voter;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const comment_object& comment = _db.get_comment( o.author, o.permlink );
   const account_object& voter = _db.get_account( o.voter );
   time_point now = _db.head_block_time();
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   FC_ASSERT( voter.can_vote, 
      "Voter has declined their voting rights." );
   FC_ASSERT( comment.allow_votes, 
      "Votes are not allowed on the comment." );

   const board_object* board_ptr = nullptr; 
   if( comment.board )
   {
      board_ptr = _db.find_board( comment.board );      
      const board_member_object& board_member = _db.get_board_member( comment.board );       
      FC_ASSERT( board_member.is_authorized_interact( voter.name ), 
         "User ${u} is not authorized to interact with posts in the board ${b}.",("b", board.name)("u", voter.name));
   }

   const reward_fund_object& reward_fund = _db.get_reward_fund();
   auto curve = reward_fund.curation_reward_curve;
   
   const auto& comment_vote_idx = _db.get_index< comment_vote_index >().indices().get< by_comment_voter >();
   auto itr = comment_vote_idx.find( std::make_tuple( comment.id, voter.id ) );

   int64_t elapsed_seconds = (now - voter.last_vote_time).to_seconds();
   FC_ASSERT( elapsed_seconds >= MIN_VOTE_INTERVAL_SEC, 
      "Can only vote once every ${s} seconds.", ("s", MIN_VOTE_INTERVAL_SEC) );
   int16_t regenerated_power = (PERCENT_100 * elapsed_seconds) / props.vote_recharge_time.to_seconds();
   int16_t current_power = std::min( int64_t(voter.voting_power + regenerated_power), int64_t(PERCENT_100) );
   FC_ASSERT( current_power > 0, 
      "Account currently does not have voting power." );
   int16_t abs_weight = abs(o.weight);
   int16_t used_power = (current_power * abs_weight) / PERCENT_100;

   int16_t max_vote_denom = props.vote_power_reserve_rate * (props.vote_recharge_time.count() / fc::days(1).count);
   FC_ASSERT( max_vote_denom > 0 );
   used_power = (used_power + max_vote_denom - 1) / max_vote_denom;
   FC_ASSERT( used_power <= current_power, 
      "Account does not have enough power to vote." );

   uint128_t voting_power = _db.get_voting_power( voter ).value;    // Gets the user's voting power from their Equity and Staked coin balances
   uint128_t abs_reward = (voting_power * used_power) / PERCENT_100;
   FC_ASSERT( abs_reward > VOTE_DUST_THRESHOLD || o.weight == 0, 
      "Voting weight is too small, please accumulate more voting power." );
   uint128_t reward = o.weight < 0 ? -abs_reward : abs_reward; // Determines the sign of abs_reward for upvote and downvote

   const comment_object& root = _db.get( comment.root_comment );

   if( itr == comment_vote_idx.end() )   // New vote is being added to emtpy index
   {
      FC_ASSERT( o.weight != 0, 
         "Vote weight cannot be 0 for the first vote.");
      FC_ASSERT( abs_reward > 0, 
         "Cannot vote with 0 reward." );

      _db.modify( voter, [&]( account_object& a )
      {
         a.voting_power = current_power - used_power;
         a.last_vote_time = now;
      });

      uint128_t old_power = std::max( comment.vote_power, uint128_t(0));

      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_reward += reward;
         c.vote_power += reward;
         if( reward > 0 )
         {
            c.net_votes++;
         }   
         else
         {
            c.net_votes--;
         } 
      });

      uint128_t new_power = std::max( comment.vote_power, uint128_t(0));

      /** this verifies uniqueness of voter
       *
       *  cv.weight / c.total_vote_weight ==> % of reward increase that is accounted for by the vote
       *
       *  W(R) = B * R / ( R + 2S )
       *  W(R) is bounded above by B. B is fixed at 2^64 - 1, so all weights fit in a 64 bit integer.
       *
       *  The equation for an individual vote is:
       *    W(R_N) - W(R_N-1), which is the delta increase of proportional weight
       *
       *  c.total_vote_weight =
       *    W(R_1) - W(R_0) +
       *    W(R_2) - W(R_1) + ...
       *    W(R_N) - W(R_N-1) = W(R_N) - W(R_0)
       *
       *  Since W(R_0) = 0, c.total_vote_weight is also bounded above by B and will always fit in a 64 bit integer.
       */

      _db.create<comment_vote_object>( [&]( comment_vote_object& cv )
      {
         cv.voter = voter.name;
         cv.comment = comment.id;
         cv.reward = reward;
         cv.vote_percent = o.weight;
         cv.last_update = now;

         bool curation_reward_eligible = reward > 0 && comment.cashout_time != fc::time_point::maximum() && comment.allow_curation_rewards;
         
         if( curation_reward_eligible )
         {
            uint128_t old_weight = util::evaluate_reward_curve( old_power, curve, reward_fund.content_constant );
            uint128_t new_weight = util::evaluate_reward_curve( new_power, curve, reward_fund.content_constant );
            uint128_t max_vote_weight = new_weight - old_weight;

            _db.modify( comment, [&]( comment_object& c )
            {
               c.total_vote_weight += max_vote_weight;       // Increase reward weight for curation rewards by maximum
            });

            uint128_t curation_auction_decay_time = props.curation_auction_decay_time.to_seconds();
            uint128_t w = max_vote_weight;
            uint128_t delta_t = std::min( uint128_t(( now - comment.created ).to_seconds()), curation_auction_decay_time ); 

            w *= delta_t;
            w /= curation_auction_decay_time;       // Discount weight linearly by time for early votes in the first 10 minutes

            double curation_decay = props.vote_curation_decay;
            double vote_discount_rate = std::max(( double(comment.net_votes) / curation_decay), double(0));
            double vote_discount = std::pow(0.5, vote_discount_rate );     // Raises 0.5 to a fractional power for each 100 net_votes added
            double vote_discount_percent = double(vote_discount) * double(PERCENT_100);
            FC_ASSERT( vote_discount_percent >= 0, "Vote discount should not become negative");
            w *= uint128_t(vote_discount_percent);
            w /= PERCENT_100; // Discount weight exponentially for each successive vote on the post, decaying by 50% per 100 votes.
            cv.max_weight = max_vote_weight;
            cv.weight = w;
         }
         else
         {
            cv.weight = 0;
         }
      });

      if( board_ptr != nullptr )
      {
         _db.modify( *board_ptr, [&]( board_object& bo )
         {
            if( reward > 0 )
            {
               bo.vote_count++;
            }
            else 
            {
               bo.vote_count--;
            }
         });
      }
   }
   else  // Vote is being altered from a previous vote
   {
      FC_ASSERT( itr->num_changes < MAX_VOTE_CHANGES, 
         "Voter has used the maximum number of vote changes on this comment." );
      FC_ASSERT( itr->vote_percent != o.weight, 
         "You have already voted in a similar way." );

      if( itr->reward < reward )
      {
         FC_ASSERT( now < comment.cashout_time - UPVOTE_LOCKOUT_TIME, "Cannot increase payout within last twelve hours before payout." );
      }

      _db.modify( voter, [&]( account_object& a )
      {
         a.voting_power = current_power - used_power;     // Decrement Voting power by amount used
         a.last_vote_time = now;                          // Update last vote time
      });

      uint128_t old_power = std::max(comment.vote_power, uint128_t(0));  // Record net reward before new vote is applied

      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_reward -= itr->reward;
         c.net_reward += reward;
         c.vote_power -= itr->reward;
         c.vote_power += reward;
         c.total_vote_weight -= itr->max_weight;     // deduct votes previous total weight contribution

         if( reward > 0 && itr->reward < 0 ) // Remove downvote and add upvote
            c.net_votes += 2;
         else if( reward > 0 && itr->reward == 0 ) // Add upvote from neutral vote
            c.net_votes += 1;
         else if( reward == 0 && itr->reward < 0 ) // Remove downvote and leave neutral
            c.net_votes += 1;
         else if( reward == 0 && itr->reward > 0 )  // Remove upvote and leave neutral
            c.net_votes -= 1;
         else if( reward < 0 && itr->reward == 0 ) // Add downvote from neutral vote
            c.net_votes -= 1;
         else if( reward < 0 && itr->reward > 0 ) // Remove Upvote and add downvote
            c.net_votes -= 2;
      });

      uint128_t new_power = std::max( comment.vote_power, uint128_t(0));    // Record net reward after new vote is applied

      _db.modify( *itr, [&]( comment_vote_object& cv )
      {
         cv.reward = reward;
         cv.vote_percent = o.weight;
         cv.last_update = now;
         cv.num_changes += 1;

         bool curation_reward_eligible = reward > 0 && comment.cashout_time != fc::time_point::maximum() && comment.allow_curation_rewards;
         
         if( curation_reward_eligible )
         {
            uint128_t old_weight = util::evaluate_reward_curve( old_power, curve, reward_fund.content_constant );
            uint128_t new_weight = util::evaluate_reward_curve( new_power, curve, reward_fund.content_constant );
            uint128_t max_vote_weight = new_weight - old_weight;

            _db.modify( comment, [&]( comment_object& c )
            {
               c.total_vote_weight += max_vote_weight;       // Increase reward weight for curation rewards by maximum
            });

            uint128_t curation_auction_decay_time = props.curation_auction_decay_time.to_seconds();
            uint128_t w = max_vote_weight;
            uint128_t delta_t = std::min( uint128_t(( now - comment.created ).to_seconds()), curation_auction_decay_time ); 

            w *= delta_t;
            w /= curation_auction_decay_time;       // Discount weight linearly by time for early votes in the first 10 minutes

            double curation_decay = props.vote_curation_decay;
            double vote_discount_rate = std::max(( double(comment.net_votes) / curation_decay), double(0));
            double vote_discount = std::pow(0.5, vote_discount_rate );     // Raises 0.5 to a fractional power for each 100 net_votes added
            double vote_discount_percent = double(vote_discount) * double(PERCENT_100);
            FC_ASSERT( vote_discount_percent >= 0, "Vote discount should not become negative");
            w *= uint128_t(vote_discount_percent);
            w /= PERCENT_100; // Discount weight exponentially for each successive vote on the post, decaying by 50% per 100 votes.
            cv.max_weight = max_vote_weight;
            cv.weight = w;
         }
         else
         {
            cv.weight = 0;
            cv.max_weight = 0;
         }
      });
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void view_evaluator::do_apply( const view_operation& o )
{ try {
   const account_name_type& signed_for = o.viewer;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const comment_object& comment = _db.get_comment( o.author, o.permlink );
   const account_object& viewer = _db.get_account( o.viewer );
   uint128_t voting_power = _db.get_voting_power( viewer ).value;         // Gets the user's voting power from their Equity and Staked coin balances to weight the view.
   FC_ASSERT( comment.allow_views, "Views are not allowed on the comment." );
   FC_ASSERT( viewer.can_vote, "Viewer has declined their voting rights." );
   const board_object* board_ptr = nullptr; 
   if( comment.board )
   {
      board_ptr = _db.find_board( comment.board );      
      const board_member_object& board_member = _db.get_board_member( comment.board );       
      FC_ASSERT( board_member.is_authorized_interact( viewer.name ), 
         "User ${u} is not authorized to interact with posts in the board ${b}.",("b", board.name)("u", viewer.name));
   }
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   const reward_fund_object& reward_fund = _db.get_reward_fund();
   auto curve = reward_fund.curation_reward_curve;
   
   const supernode_object& supernode = _db.get_supernode( o.supernode );
   const interface_object& interface = _db.get_interface( o.interface );

   time_point now = _db.head_block_time();

   const auto& comment_view_idx = _db.get_index< comment_view_index >().indices().get< by_comment_viewer >();
   auto itr = comment_view_idx.find( std::make_tuple( comment.id, viewer.name ) );
   int64_t elapsed_seconds = (now - viewer.last_view_time).to_seconds();

   FC_ASSERT( elapsed_seconds >= MIN_VIEW_INTERVAL_SEC, 
      "Can only view once every ${s} seconds.", ("s", MIN_VIEW_INTERVAL_SEC) );
   int16_t regenerated_power = (PERCENT_100 * elapsed_seconds) / props.view_recharge_time.to_seconds();
   int16_t current_power = std::min( int64_t(viewer.viewing_power + regenerated_power), int64_t(PERCENT_100) );

   FC_ASSERT( current_power > 0, 
      "Account currently does not have any viewing power." );

   int16_t max_view_denom = props.view_reserve_rate * (props.view_recharge_time.count() / fc::days(1).count);    // Weights the viewing power with the network reserve ratio and recharge time
   FC_ASSERT( max_view_denom > 0 );
   int16_t used_power = (current_power + max_view_denom - 1) / max_view_denom;
   FC_ASSERT( used_power <= current_power, "Account does not have enough power to view." );
   
   uint128_t reward = (voting_power * used_power) / PERCENT_100;
   const comment_object& root = _db.get( comment.root_comment );       // If root post, gets the posts own object.

   if( itr == comment_view_idx.end() )   // New view is being added 
   {
      FC_ASSERT( reward > 0, "Cannot claim view with 0 reward." );
      FC_ASSERT( o.viewed, "Viewed must be set to true to create new view, View does not exist to remove." );

      const auto& supernode_view_idx = _db.get_index< comment_view_index >().indices().get< by_supernode_viewer >();
      const auto& interface_view_idx = _db.get_index< comment_view_index >().indices().get< by_interface_viewer >();

      auto supernode_view_itr = comment_view_idx.lower_bound( std::make_tuple( supernode.account, viewer.name ) );
      auto interface_view_itr = comment_view_idx.lower_bound( std::make_tuple( interface.account, viewer.name ) );
      share_type vp = voting_power.to_int64();

      if( supernode_view_itr == supernode_view_idx.end() )   // No view exists
      {
         _db.adjust_view_weight( supernode, vp );    // Adds voting power to the supernode view weight once per day per user. 
      }
      else if( ( supernode_view_itr->created + days(1) ) < now )     // Latest view is more than 1 day old
      {
         _db.adjust_view_weight( supernode, vp );    // Adds voting power to the supernode view weight once per day per user. 
      }
      if( interface_view_itr == interface_view_idx.end() )   // No view exists
      {
         _db.adjust_interface_users( interface );
      }
      else if( ( interface_view_itr->created + days(1) ) < now )    // Latest View is more than 1 day old
      {
         _db.adjust_interface_users( interface );
      }

      _db.modify( viewer, [&]( account_object& a )
      {
         a.viewing_power = current_power - used_power;
         a.last_view_time = now;
      });

      uint128_t old_power = std::max(comment.view_power, uint128_t(0));  // Record reward value before applying view transaction

      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_reward += reward;
         c.view_power += reward;
         c.view_count++;
      });

      uint128_t new_power = std::max( comment.view_power, uint128_t(0));   // record new net reward after viewing

      if( board_ptr != nullptr )
      {
         _db.modify( *board_ptr, [&]( board_object& bo )
         {
            bo.view_count++;
         });
      }

      share_type max_view_weight = 0;

      _db.create<comment_view_object>( [&]( comment_view_object& cv )
      {
         cv.viewer = viewer.name;
         cv.comment = comment.id;
         cv.interface = interface.account;
         cv.supernode = supernode.account;
         cv.reward = reward;
         cv.created = now;

         bool curation_reward_eligible = reward > 0 && comment.cashout != fc::time_point::maximum() && comment.allow_curation_rewards;
            
         if( curation_reward_eligible )
         {
            uint128_t old_weight = util::evaluate_reward_curve( old_power, curve, reward_fund.content_constant );
            uint128_t new_weight = util::evaluate_reward_curve( new_power, curve, reward_fund.content_constant );
            uint128_t max_view_weight = new_weight - old_weight;  // Gets the difference in content reward weight before and after the view occurs.

            _db.modify( comment, [&]( comment_object& c )
            {
               c.total_view_weight += max_view_weight;
            });

            uint128_t curation_auction_decay_time = props.curation_auction_decay_time.to_seconds();
            uint128_t w = max_view_weight;
            uint128_t delta_t = std::min( uint128_t(( now - comment.created).to_seconds()), curation_auction_decay_time );

            w *= delta_t;
            w /= curation_auction_decay_time;                     // Discount weight linearly by time for early views in the first 10 minutes.

            double curation_decay = props.view_curation_decay;      // Number of views for half life of curation reward decay.
            double view_discount_rate = std::max(( double(comment.view_count) / curation_decay), double(0));
            double view_discount = std::pow(0.5, view_discount_rate );     // Raises 0.5 to a fractional power for each 1000 views added
            uint64_t view_discount_percent = double(view_discount)*double(PERCENT_100);
            FC_ASSERT(view_discount_percent >= 0, "View discount should not become negative");
            w *= view_discount_percent;
            w /= PERCENT_100;      // Discount weight exponentially for each successive view on the post, decaying by 50% per 1000 views.
            cv.weight = w;
            cv.max_weight = max_view_weight;
         }
         else
         {
            cv.weight = 0;
            cv.max_weight = 0;
         }
      });
   }
   else  // View is being removed
   {
      FC_ASSERT( !o.viewed, 
         "Must select view = false to remove existing view" );
      
      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_reward -= itr->reward;
         c.view_power -= itr->reward;
         c.total_view_weight -= itr->max_weight;
         c.view_count--;
      });

      _db.modify( board, [&]( board_object& bo )
      {
         bo.view_count--;  
      });

      _db.remove( *itr );
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void share_evaluator::do_apply( const share_operation& o )
{ try {
   const account_name_type& signed_for = o.sharer;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const comment_object& comment = _db.get_comment( o.author, o.permlink );
   FC_ASSERT( comment.parent_author.size() == 0, 
      "Only top level posts can be shared." );
   FC_ASSERT( comment.allow_shares, 
      "shares are not allowed on the comment." );
   const account_object& sharer = _db.get_account( o.sharer );
   FC_ASSERT( sharer.can_vote,  
      "sharer has declined their voting rights." );
   const board_object* board_ptr = nullptr; 
   if( comment.board )
   {
      board_ptr = _db.find_board( comment.board );      
      const board_member_object& board_member = _db.get_board_member( comment.board );       
      FC_ASSERT( board_member.is_authorized_interact( sharer.name ), 
         "User ${u} is not authorized to interact with posts in the board ${b}.",("b", board.name)("u", sharer.name));
   }
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   const reward_fund_object& reward_fund = _db.get_reward_fund();
   auto curve = reward_fund.curation_reward_curve;
   time_point now = props.time;
   
   const auto& comment_share_idx = _db.get_index< comment_share_index >().indices().get< by_comment_sharer >();
   auto itr = comment_share_idx.find( std::make_tuple( comment.id, sharer.name ) );

   const auto& blog_idx = _db.get_index< blog_index >().indices().get< by_comment_account >();
   const blog_itr = blog_idx.find( boost::make_tuple( comment.id, sharer.name ) );

   int64_t elapsed_seconds = (now - sharer.last_feed_time).to_seconds();
   FC_ASSERT( elapsed_seconds >= MIN_SHARE_INTERVAL_SEC, 
      "Can only share once every ${s} seconds.", ("s", MIN_SHARE_INTERVAL_SEC) );
   int16_t regenerated_power = (PERCENT_100 * elapsed_seconds) / props.share_recharge_time.to_seconds();
   int16_t current_power = std::min( int64_t(sharer.sharing_power + regenerated_power), int64_t(PERCENT_100) );
   FC_ASSERT( current_power > 0, 
      "Account currently does not have any sharing power." );

   int16_t max_share_denom = props.share_reserve_rate * (props.share_recharge_time.count() / fc::days(1).count);    // Weights the sharing power with the network reserve ratio and recharge time
   FC_ASSERT( max_share_denom > 0 );
   int16_t used_power = (current_power + max_share_denom - 1) / max_share_denom;
   FC_ASSERT( used_power <= current_power,   
      "Account does not have enough power to share." );
   uint128_t voting_power = _db.get_voting_power( sharer ).value;         // Gets the user's voting power from their Equity and Staked coin balances to weight the share.
   uint128_t reward = (voting_power * used_power) / PERCENT_100;
   const comment_object& root = _db.get( comment.root_comment );       // If root post, gets the posts own object.

   if( itr == comment_share_idx.end() )   // New share is being added to emtpy index
   {
      FC_ASSERT( reward > 0, 
         "Cannot claim share with 0 reward." );
      FC_ASSERT( o.shared , 
         "Shared must be set to true to create new share, share does not exist to remove." );

      _db.modify( sharer, [&]( account_object& a )
      {
         a.share_power = current_power - used_power;
         a.last_feed_time = now;
      });

      uint128_t old_power = std::max(comment.share_power, uint128_t(0));  // Record reward value before applying share transaction

      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_reward += reward;
         c.share_power += reward;
         c.share_count++;
      });

      if( board_ptr != nullptr )
      {
         _db.modify( *board_ptr, [&]( board_object& bo )
         {
            bo.share_count++;
         });
      }

      uint128_t new_power = std::max( comment.share_power, uint128_t(0));   // record new net reward after sharing

      // Create comment share object for tracking share.
      _db.create<comment_share_object>( [&]( comment_share_object& cs )    
      {
         cs.sharer = sharer.name;
         cs.comment = comment.id;
         cs.reward = reward;
         cs.created = now;

         bool curation_reward_eligible = reward > 0 && comment.cashout != fc::time_point::maximum() && comment.allow_curation_rewards;
  
         if( curation_reward_eligible )
         {
            uint128_t old_weight = util::evaluate_reward_curve( old_power, curve, reward_fund.content_constant );
            uint128_t new_weight = util::evaluate_reward_curve( new_power, curve, reward_fund.content_constant );
            uint128_t max_share_weight = new_weight - old_weight;  // Gets the difference in content reward weight before and after the share occurs.

            _db.modify( comment, [&]( comment_object& c )
            {
               c.total_share_weight += max_share_weight;
            });

            uint128_t curation_auction_decay_time = props.curation_auction_decay_time.to_seconds();
            uint128_t w = max_share_weight;
            uint128_t delta_t = std::min( uint128_t(( now - comment.created).to_seconds()), curation_auction_decay_time ); 

            w *= delta_t;
            w /= curation_auction_decay_time;   // Discount weight linearly by time for early shares in the first 10 minutes.

            double curation_decay = props.share_curation_decay;      // Number of shares for half life of curation reward decay.
            double share_discount_rate = std::max(( double(comment.share_count) / curation_decay), double(0));
            double share_discount = std::pow(0.5, share_discount_rate );     // Raises 0.5 to a fractional power for each 1000 shares added
            double share_discount_percent = double(share_discount) * double(PERCENT_100);
            FC_ASSERT(share_discount_percent >= 0, "Share discount should not become negative");
            w *= uint128_t(share_discount_percent);
            w /= PERCENT_100;      // Discount weight exponentially for each successive share on the post, decaying by 50% per 50 shares.
            cs.weight = w;
            cs.max_weight = max_share_weight;
         }
         else
         {
            cs.weight = 0;
            cs.max_weight = 0;
         }
      });
      
      // Create blog and feed objects for sharer account's followers and connections. 
      _db.share_comment_to_feeds( sharer.name, comment ); 


      if( o.board.is_valid() )
      {
         const board_member_object& board_member = _db.get_board_member( o.board );       
         FC_ASSERT( board_member.is_authorized_interact( sharer.name ), 
            "User ${u} is not authorized to interact with posts in the board ${b}.",
            ("b", board.name)("u", sharer.name));

         _db.share_comment_to_board( sharer.name, *o.board, comment );
      }

      if( o.tag.is_valid() )
      {
         _db.share_comment_to_board( sharer.name, *o.tag, comment );
      }
      
   }
   else  // share is being removed
   {
      FC_ASSERT( !o.shared, 
         "Must select shared = false to remove existing share" );
      
      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_reward -= itr->reward;
         c.share_power -= itr->reward;
         c.total_share_weight -= itr->max_weight;
         c.share_count--;
      });

      if( board_ptr != nullptr )
      {
         _db.modify( *board_ptr, [&]( board_object& bo )
         {
            bo.share_count--;
         });
      }

      // Remove all blog and feed objects that the account has created for the original share operation. 
      _db.remove_shared_comment( sharer.name, comment );  // TODO

      _db.remove( *itr );
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


/**
 * Moderation tags enable interface providers to co-ordinate moderation efforts
 * on-chain and provides a method for discretion to be provided
 * to displaying content, based on the governance addresses subscribed to by the 
 * viewing user.
 */
void moderation_tag_evaluator::do_apply( const moderation_tag_operation& o )
{ try {
   FC_ASSERT( fc::is_utf8( o.details ), "Details must be UTF-8" );
   FC_ASSERT( o.details.size(), 
      "Moderation tag must include details explaining the rule or standard violation." );
   FC_ASSERT( o.details.size() <= MAX_STRING_LENGTH, "Details are too long." );
   const account_name_type& signed_for = o.moderator;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_governance( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& moderator = _db.get_account( o.moderator );
   const account_object& author = _db.get_account( o.author );
   const comment_object& comment = _db.get_comment( o.author, o.permlink );
   const governance_account_object* gov_ptr = _db.find_governance_account( o.moderator );
   const board_object* board_ptr = _db.find_board( comment.board );

   if( board_ptr != nullptr || gov_ptr != nullptr )
   {
      const board_member_object* board_member_ptr = _db.find_board_member( comment.board );
      FC_ASSERT( board_member_ptr != nullptr || gov_ptr != nullptr, 
         "Account must be a board moderator or governance account to create moderation tag." );

      if( board_member_ptr == nullptr )     // no board, must be governance account
      {
         FC_ASSERT( gov_ptr != nullptr, 
            "Account must be a governance account to create moderation tag." );
         FC_ASSERT( gov_ptr->account == o.moderator, 
            "Account must be a governance account to create moderation tag." );
      }
      else if( gov_ptr == nullptr )         // not governance account, must be moderator
      {
         FC_ASSERT( board_member_ptr != nullptr, 
            "Account must be a board moderator to create moderation tag." );
         FC_ASSERT( board_member_ptr->is_moderator( o.moderator ), 
            "Account must be a board moderator to create moderation tag." );
      }
      else
      {
         FC_ASSERT( board_member_ptr->is_moderator( o.moderator ) || gov_ptr->account == o.moderator, 
            "Account must be a board moderator or governance account to create moderation tag." );
      } 
   }
  
   FC_ASSERT( o.rating >= comment.rating,  
      "Moderation Tag rating ${n} should be equal to or greater than author's rating ${r}.", ("n", o.rating)("r", comment.rating) );
   const auto& mod_idx = _db.get_index< moderation_tag_index >().indices().get< by_moderator_comment >();
   auto itr = mod_idx.find( boost::make_tuple( o.moderator, comment.id ) );
   time_point now = _db.head_block_time();

   if( itr != mod_idx.end() )     // Creating new moderation tag.
   {
      FC_ASSERT( o.applied, 
         "Moderation tag does not exist, Applied should be set to true to create new moderation tag.");

      _db.create< moderation_tag_object >( [&]( moderation_tag_object& mto )
      {
         mto.moderator = moderator.name;
         mto.comment = comment.id;
         mto.board = o.board;
         mto.tags = o.tags;
         mto.rating = o.rating;
         mto.details = o.details;
         mto.filter = o.filter;
         mto.last_update = now;
         mto.created = now;  
      });
   }
   else    
   {
      if( o.applied )  // Editing existing moderation tag
      {
         _db.modify( *itr, [&]( moderation_tag_object& mto )
         {
            mto.tags = o.tags;
            mto.rating = o.rating;
            mto.details = o.details;
            mto.filter = o.filter;
            mto.last_update = now;
         });
      }
      else    // deleting moderation tag
      {
         _db.remove( *itr );
      }
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


//==========================//
// === Board Evaluators === //
//==========================//


void board_create_evaluator::do_apply( const board_create_operation& o )
{ try {
   const account_name_type& signed_for = o.founder;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& founder = _db.get_account( o.founder ); 
   time_point now = _db.head_block_time();
   FC_ASSERT( now > founder.last_board_created + MIN_BOARD_CREATE_INTERVAL, 
      "Founders can only create one board per day." );
   const board_object* board_ptr = _db.find_board( o.name ); 
   FC_ASSERT( board_ptr == nullptr, 
      "Board with the name: ${n} already exists.", ("n", o.name) );

   _db.create< board_object >( [&]( board_object& bo )
   {
      bo.name = o.name;
      bo.founder = founder.name;
      bo.board_type = o.board_type;
      bo.board_privacy = o.board_privacy;
      from_string( bo.json, o.json );
      from_string( bo.json_private, o.json_private );
      from_string( bo.details, o.details );
      from_string( bo.url, o.url );
      bo.board_public_key = o.board_public_key;
      bo.created = now;
      bo.last_board_update = now;
      bo.last_post = now;
      bo.last_root_post = now;
   });

   const board_member_object& new_board_member = _db.create< board_member_object >( [&]( board_member_object& bmo )
   {
      bmo.name = o.name;
      bmo.founder = founder.name;
      bmo.subscribers.insert(founder.name);
      bmo.members.insert(founder.name);
      bmo.moderators.insert(founder.name);
      bmo.administrators.insert(founder.name);
   });

   _db.create< board_moderator_vote_object>( [&]( board_moderator_vote_object& v )
   {
      v.moderator = founder.name;
      v.account = founder.name;
      v.board = o.name;
      v.vote_rank = 1;
   });

   _db.update_board_moderators( new_board_member );

} FC_CAPTURE_AND_RETHROW( (o)) }


void board_update_evaluator::do_apply( const board_update_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const board_object& board = _db.get_board( o.board ); 
   const account_object& account = _db.get_account( o.account );
   time_point now = _db.head_block_time();
   FC_ASSERT( now > board.last_board_update + MIN_BOARD_UPDATE_INTERVAL, 
      "Boards can only be updated once per 10 minutes." );
   const board_member_object& board_member = _db.get_board_member( o.board );
   FC_ASSERT( board_member.is_administrator(account.name), 
      "Only administrators of the board can update it.");

   if( o.board_privacy == OPEN_BOARD )
   {
      FC_ASSERT( board.board_privacy != PUBLIC_BOARD || board.board_privacy != PRIVATE_BOARD || board.board_privacy != EXCLUSIVE_BOARD,
         "Cannot lower the privacy setting of a board, only increase it." );
   }
   else if( o.board_privacy == PUBLIC_BOARD )
   {
      FC_ASSERT( board.board_privacy != PRIVATE_BOARD || board.board_privacy != EXCLUSIVE_BOARD,
         "Cannot lower the privacy setting of a board, only increase it." );
   }
   else if( o.board_privacy == PRIVATE_BOARD )
   {
      FC_ASSERT( board.board_privacy != EXCLUSIVE_BOARD,
         "Cannot lower the privacy setting of a board, only increase it." );
   }

   _db.modify(board, [&]( board_object& bo)
   {
      bo.board_type = o.board_type;
      bo.board_privacy = o.board_privacy;
      from_string( bo.json, o.json );
      from_string( bo.json_private, o.json_private );
      from_string( bo.details, o.details );
      from_string( bo.url, o.url );
      bo.board_public_key = o.board_public_key;
      bo.last_board_update = now;
   });
} FC_CAPTURE_AND_RETHROW( (o)) }


void board_vote_mod_evaluator::do_apply( const board_vote_mod_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& voter = _db.get_account( o.account );
   const account_object& moderator_account = _db.get_account( o.moderator );
   const board_object& board = _db.get_board( o.board );
   const board_member_object& board_member = _db.get_board_member( o.board );
   const witness_schedule_object& wso = _db.get_witness_schedule();
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();

   if( o.added )
   {
      FC_ASSERT( voter.can_vote, 
         "Account has declined its voting rights." );
      FC_ASSERT( board_member.is_member( o.account ),
         "Account: ${a} must be a member before voting for a moderator of Board: ${b}.", ("a", o.account)("b", o.board));
      FC_ASSERT( board_member.is_moderator( o.moderator ),
         "Account: ${a} must be a moderator of Board: ${b}.", ("a", o.moderator)("b", o.board));
   }
   
   const auto& account_board_rank_idx = _db.get_index< executive_board_vote_index >().indices().get< by_account_board_rank >();
   const auto& account_board_moderator_idx = _db.get_index< executive_board_vote_index >().indices().get< by_account_board_moderator >();
   auto account_board_rank_itr = account_board_rank_idx.find( boost::make_tuple( o.account, o.board, o.vote_rank ) );   // vote at rank number
   auto account_board_moderator_itr = account_board_moderator_idx.find( boost::make_tuple( o.account, o.board, o.moderator ) );    // vote for moderator in board

   if( o.added )      // Adding or modifying vote
   {
      if( account_board_moderator_itr == account_board_moderator_idx.end() && 
         account_board_rank_itr == account_board_rank_idx.end() )       // No vote for executive or rank, create new vote.
      {
         _db.create< board_moderator_vote_object>( [&]( board_moderator_vote_object& v )
         {
            v.moderator = o.moderator;
            v.account = o.account;
            v.board = o.board;
            v.vote_rank = o.vote_rank;
         });
         
         _db.update_board_moderator_votes( voter, o.board );
      }
      else
      {
         if( account_board_moderator_itr != account_board_moderator_idx.end() && account_board_rank_itr != account_board_rank_idx.end() )
         {
            FC_ASSERT( account_board_moderator_itr->executive_board != account_board_rank_itr->executive_board, 
               "Vote at rank is already for the specified moderator." );
         }
         
         if( account_board_moderator_itr != account_board_moderator_idx.end() )
         {
            remove( *account_board_moderator_itr );
         }

         _db.update_board_moderator_votes( voter, o.board, o.moderator, o.vote_rank );   // Remove existing moderator vote, and add at new rank. 
      }
   }
   else       // Removing existing vote
   {
      if( account_board_moderator_itr != account_board_moderator_idx.end() )
      {
         remove( *account_board_moderator_itr );
      }
      else if( account_board_rank_itr != account_board_rank_idx.end() )
      {
         remove( *account_board_rank_itr );
      }
      _db.update_board_moderator_votes( voter, o.board );
   }

   _db.update_board_moderators( board_member );

} FC_CAPTURE_AND_RETHROW( (o)) }


void board_add_mod_evaluator::do_apply( const board_add_mod_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_governance( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_object& moderator = _db.get_account( o.moderator );
   const board_object& board = _db.get_board( o.board ); 
   const board_member_object& board_member = _db.get_board_member( o.board );
   FC_ASSERT( board_member.is_member(moderator.name), 
      "Account: ${a} must be a member before promotion to moderator of Board: ${b}.", ("a", o.moderator)("b", o.board));
   
   if( o.added || account.name != admin.name )     // Account can remove itself from board administrators.  
   {
      FC_ASSERT( board_member.is_administrator(account.name), 
         "Account: ${a} is not an administrator of the Board: ${b} and cannot add or remove moderators.", ("a", o.account)("b", o.board));
   }

   if( o.added )
   {
      FC_ASSERT( !board_member.is_moderator(moderator.name), 
         "Account: ${a} is already a moderator of Board: ${b}.", ("a", o.moderator)("b", o.board));
      FC_ASSERT( !board_member.is_administrator(moderator.name), 
         "Account: ${a} is already a administrator of Board: ${b}.", ("a", o.moderator)("b", o.board));
      FC_ASSERT( board_member.founder != moderator.name, 
         "Account: ${a} is already the Founder of Board: ${b}.", ("a", o.moderator)("b", o.board));
   }
   else
   {
      FC_ASSERT( board_member.is_moderator(moderator.name), 
         "Account: ${a} is not a moderator of Board: ${b}.", ("a", o.moderator)("b", o.board));
      FC_ASSERT( !board_member.is_administrator(moderator.name), 
         "Account: ${a} cannot be removed from moderators while an administrator of Board: ${b}.", ("a", o.moderator)("b", o.board));
      FC_ASSERT( board_member.founder != moderator.name ), 
         "Account: ${a} cannot be removed while the founder of Board: ${b}.", ("a", o.moderator)("b", o.board));
   }

   _db.modify(board_member, [&]( board_member_object& bmo)
   {
      if(o.added)
      {
         bmo.moderators.insert(moderator.name);
      }
      else 
      {
         bmo.moderators.erase(moderator.name);
      }
      
   });
} FC_CAPTURE_AND_RETHROW( (o)) }


void board_add_admin_evaluator::do_apply( const board_add_admin_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_governance( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_object& administrator = _db.get_account( o.admin ); 
   const board_object& board = _db.get_board( o.board ); 
   const board_member_object& board_member = _db.get_board_member( o.board );

   FC_ASSERT( board_member.is_member(administrator.name), 
      "Account: ${a} must be a member before promotion to administrator of Board: ${b}.", ("a", o.admin)("b", o.board));
   FC_ASSERT( board_member.is_moderator(administrator.name), 
      "Account: ${a} must be a moderator before promotion to administrator of Board: ${b}.", ("a", o.admin)("b", o.board));

   if( o.added || account.name != administrator.name )     // Account can remove itself from board administrators.  
   {
      FC_ASSERT( board_member.founder == account.name), 
         "Only the Founder: ${f} of the board can add or remove administrators.", ("f", board_member.founder));
   }
   if(o.added)
   {
      FC_ASSERT( !board_member.is_administrator(administrator.name), 
         "Account: ${a} is already an administrator of Board: ${b}.", ("a", o.admin)("b", o.board));
      FC_ASSERT( board_member.founder != administrator.name, 
         "Account: ${a} is already the Founder of Board: ${b}.", ("a", o.admin)("b", o.board));
   }
   else
   {
      FC_ASSERT( board_member.is_administrator(administrator.name), 
         "Account: ${a} is not an administrator of Board: ${b}.", ("a", o.admin)("b", o.board));
      FC_ASSERT( board_member.founder != administrator.name ), 
         "Account: ${a} cannot be removed as administrator while the Founder of Board: ${b}.", ("a", o.moderator)("b", o.board));
   }
   
   _db.modify(board_member, [&]( board_member_object& bmo)
   {
      if(o.added)
      {
         bmo.administrators.insert(administrator.name);
      }
      else 
      {
         bmo.administrators.erase(administrator.name);
      }
   });
} FC_CAPTURE_AND_RETHROW( (o)) }


void board_transfer_ownership_evaluator::do_apply( const board_transfer_ownership_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_chief( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const board_object& board = _db.get_board( o.board ); 
   const account_object& account = _db.get_account( o.account ); 
   const account_object& new_founder = _db.get_account( o.new_founder );
   time_point now = _db.head_block_time();
   const board_member_object& board_member = _db.get_board_member( o.board );
   FC_ASSERT( board.founder == account.name), "Only the founder of the board can transfer ownership.");
   FC_ASSERT( board_member.founder == account.name), "Only the founder of the board can transfer ownership.");

   _db.modify(board, [&]( board_object& bo)
   {
      bo.founder = o.new_founder;
   });

   _db.modify(board_member, [&]( board_member_object& bmo)
   {
      bmo.founder = o.new_founder;
   });

} FC_CAPTURE_AND_RETHROW( (o)) }


void board_join_request_evaluator::do_apply( const board_join_request_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account ); 
   const board_object& board = _db.get_board( o.board ); 
   FC_ASSERT( board.board_privacy != EXCLUSIVE_BOARD, 
      "Account: ${a} cannot request to join an Exclusive board: ${b} Membership is by invitation only.", ("a", o.account)("b", o.board));
   const board_member_object& board_member = _db.get_board_member( o.board ); 
   FC_ASSERT( !board_member.is_member( account.name ), 
      "Account: ${a} is already a member of the board: ${b}.", ("a", o.account)("b", o.board)); 
   FC_ASSERT( board_member.is_authorised_request( account.name ), 
      "Account: ${a} is not authorised to request to join the board: ${b}.", ("a", o.account)("b", o.board));
   time_point now = _db.head_block_time();
   const auto& req_idx = _db.get_index< board_join_request_index >().indices().get< by_account_board >();
   auto itr = req_idx.find( std::make_tuple( o.account, o.board ) );

   if( itr == req_idx.end())    // Request does not exist yet
   {
      FC_ASSERT( o.requested, "Board join request does not exist, requested should be set to true." );
      _db.create< board_join_request_object >( [&]( board_join_request_object& bjro )
      {
         bjro.account = account.name;
         bjro.board = board.name;
         from_string( bjro.message, o.message );
         bjro.expiration = now + CONNECTION_REQUEST_DURATION;
      });
   }
   else     // Request exists and is being deleted
   {
      FC_ASSERT( !o.requested, "Request already exists, Requested should be set to false to remove existing request." );
      _db.remove( *itr );
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void board_join_invite_evaluator::do_apply( const board_join_invite_operation& o )
{ try {
   FC_ASSERT( o.account != o.member, "Account: ${a} cannot invite itself to join a board: ${b} .", ("a", o.member)("b", o.board));
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account ); 
   const account_object& member = _db.get_account( o.member ); 
   const board_object& board = _db.get_board( o.board );
   const board_member_object& board_member = _db.get_board_member( o.board ); 
   FC_ASSERT( !board_member.is_member( member.name ), 
      "Account: ${a} is already a member of the board: ${b}.", ("a", o.member)("b", o.board));
   FC_ASSERT( board_member.is_authorized_invite( account.name, board ), 
      "Account: ${a} is not authorised to send board: ${b} join invitations.", ("a", o.account)("b", o.board));
   
   time_point now = _db.head_block_time();
   const auto& key_idx = _db.get_index< board_member_key_index >().indices().get< by_member_board >();
   const auto& inv_idx = _db.get_index< board_join_invite_index >().indices().get< by_member_board >();
   auto inv_itr = inv_idx.find( std::make_tuple( o.member, o.board ) );

   if( inv_itr == inv_idx.end())    // Invite does not exist yet
   {
      FC_ASSERT( o.invited, "Board invite request does not exist, invited should be set to true." );
      _db.create< board_join_invite_object >( [&]( board_join_invite_object& bjio )
      {
         bjio.account = account.name;
         bjio.member = member.name;
         bjio.board = board.name;
         from_string( bjio.message, o.message );
         bjio.expiration = now + CONNECTION_REQUEST_DURATION;
      });
      _db.create< board_member_key_object >( [&]( board_member_key_object& bmko )
      {
         bmko.account = account.name;
         bmko.member = member.name;
         bmko.board = o.board;
         from_string( bmko.encrypted_board_key, o.encrypted_board_key );
      });
   }
   else     // Invite exists and is being deleted.
   {
      FC_ASSERT( !o.invited, "Invite already exists, Invited should be set to false to remove existing Invitation." );
      
      _db.remove( *inv_itr );
      auto key_itr = key_idx.find( std::make_tuple( o.member, o.board ) );
      _db.remove( *key_itr );
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void board_join_accept_evaluator::do_apply( const board_join_accept_operation& o )
{ try {
   FC_ASSERT( o.account != o.member, "Account: ${a} cannot accept its own join request to a board: ${b}.", ("a", o.member)("b", o.board));
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account ); 
   const account_object& member = _db.get_account( o.member );
   const board_object& board = _db.get_board( o.board );
   const board_member_object& board_member = _db.get_board_member( o.board );
   FC_ASSERT( !board_member.is_member( member.name ), "Account: ${a} is already a member of the board: ${b}.", ("a", o.member)("b", o.board));
   FC_ASSERT( board_member.is_authorized_invite(account.name, board), 
      "Account: ${a} is not authorized to accept join requests to the board: ${b}.", ("a", o.account)("b", o.board));    // Ensure Account is a moderator.

   const auto& req_idx = _db.get_index< board_join_request_index >().indices().get< by_account_board >();
   auto req_itr = req_idx.find( std::make_tuple( o.member, o.board ) );
   auto key_itr = key_idx.find( std::make_tuple( o.member, o.board ) );

   FC_ASSERT( req_itr != req_idx.end(), "Board join request does not exist.");    // Ensure Request exists

   if(o.accepted)   // Accepting the request, skipped if rejecting
   {
      _db.modify(board_member, [&]( board_member_object& bmo)
      {
         bmo.members.insert(member.name);
      });
      _db.create< board_member_key_object >( [&]( board_member_key_object& bmko )
      {
         bmko.account = account.name;
         bmko.member = member.name;
         bmko.board = o.board;
         from_string( bmko.encrypted_board_key, o.encrypted_board_key );
      });
   }
   _db.remove( *req_itr );
} FC_CAPTURE_AND_RETHROW( (o)) }


void board_invite_accept_evaluator::do_apply( const board_invite_accept_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const board_object& board = _db.get_board( o.board );
   const board_member_object& board_member = _db.get_board_member( o.board );
   FC_ASSERT( !board_member.is_member( account.name ), "Account: ${a} is already a member of the board: ${b}.", ("a", o.account)("b", o.board));
   const auto& inv_idx = _db.get_index< board_join_invite_index >().indices().get< by_member_board >();
   auto itr = inv_idx.find( std::make_tuple( o.account, o.board ) );
   FC_ASSERT( itr != inv_idx.end(), "Board join invitation does not exist.");   // Ensure Invitation exists
   const board_join_invite_object& invite = *itr;

   FC_ASSERT( board_member.is_authorized_invite( invite.account, board ), 
      "Account: ${a} is no longer authorised to send board: ${b} join invitations.", ("a", invite.account)("b", o.board));  // Ensure inviting account is still authorised to send invitations
   
   if(o.accepted)   // Accepting the request, skipped if rejecting
   {
      _db.modify(board_member, [&]( board_member_object& bmo )
      {
         bmo.members.insert(account.name);
      });
   }
   _db.remove( invite );
} FC_CAPTURE_AND_RETHROW( (o)) }


void board_remove_member_evaluator::do_apply( const board_remove_member_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_governance( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_object& member = _db.get_account( o.member );
   const board_object& board = _db.get_board( o.board );
   const board_member_object& board_member = _db.get_board_member( o.board );

   FC_ASSERT( board_member.is_member( member.name ), "Account: ${a} is not a member of board: ${b}.", ("a", o.member)("b", o.board));
   FC_ASSERT( !board_member.is_moderator( member.name ), "Account: ${a} cannot be removed while a moderator of board: ${b}.", ("a", o.member)("b", o.board));
   FC_ASSERT( !board_member.is_administrator( member.name ), "Account: ${a} cannot be removed while an administrator of board: ${b}.", ("a", o.member)("b", o.board));
   FC_ASSERT( board_member.founder != member.name ), "Account: ${a} cannot be removed while the founder of board: ${b}.", ("a", o.member)("b", o.board));

   const auto& key_idx = _db.get_index< board_member_key_index >().indices().get< by_member_board >();

   if( account.name != member.name )     // Account can remove itself from board membership.  
   {
      FC_ASSERT( board_member.is_authorized_blacklist( o.account, board ), 
         "Account: ${a} is not authorised to remove accounts from board: ${b}.", ("a", o.account)("b", o.board)); 
   }
   
   _db.modify(board_member, [&]( board_member_object& bmo)
   {
      bmo.members.erase( member.name );
   });

   auto key_itr = key_idx.find( std::make_tuple( o.member, o.board ) );
   if( key_itr != key_idx.end() )
   {
      const board_member_key_object& key = *key_itr;
      _db.remove( key );
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void board_blacklist_evaluator::do_apply( const board_blacklist_operation& o )
{ try {
   FC_ASSERT( o.account != o.member, "Account: ${a} cannot add or remove itself from the blacklist of board: ${b} .", ("a", o.member)("b", o.board));
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_governance( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_object& member = _db.get_account( o.member );
   const board_object& board = _db.get_board( o.board );
   const board_member_object& board_member = _db.get_board_member( o.board );
   
   FC_ASSERT( board_member.is_authorized_blacklist( o.account, board ), 
      "Account: ${a} is not authorised to add or remove accounts from the blacklist of board: ${b}.", ("a", o.account)("b", o.board)); 
   
   FC_ASSERT( !board_member.is_member( member.name ), "Account: ${a} cannot be blacklisted while a member of board: ${b}. Remove them first.", ("a", o.member)("b", o.board));
   FC_ASSERT( !board_member.is_moderator( member.name ), "Account: ${a} cannot be blacklisted while a moderator of board: ${b}. Remove them first.", ("a", o.member)("b", o.board));
   FC_ASSERT( !board_member.is_administrator( member.name ), "Account: ${a} cannot be blacklisted while an administrator of board: ${b}. Remove them first.", ("a", o.member)("b", o.board));
   FC_ASSERT( board_member.founder != member.name ), "Account: ${a} cannot be blacklisted while the founder of board: ${b}.", ("a", o.member)("b", o.board));

   _db.modify(board_member, [&]( board_member_object& bmo)
   {
      if(o.blacklisted)
      {
         bmo.blacklist.insert(member.name);
      }
      else
      {
         bmo.blacklist.erase(member.name);
      }
   });
} FC_CAPTURE_AND_RETHROW( (o)) }


void board_subscribe_evaluator::do_apply( const board_subscribe_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_following_object& account_following = _db.get_account_following( o.account );
   const board_object& board = _db.get_board( o.board );
   const board_member_object& board_member = _db.get_board_member( o.board );
   time_point now = _db.head_block_time();

   if( o.subscribed )   // Adding subscription 
   {
      FC_ASSERT( board_member.is_authorized_interact( account.name ), 
         "Account: ${a} is not authorized to subscribe to the board ${b}. Become a member first.",("a", account.name)("b", o.board));
      FC_ASSERT( !board_member.is_subscriber( account.name ), 
         "Account: ${a} is already subscribed to the board ${b}.",("a", account.name)("b", o.board));
   }
   else     // Removing subscription
   {
      FC_ASSERT( board_member.is_subscriber( account.name ), 
         "Account: ${a} is not subscribed to the board ${b}.",("a", account.name)("b", o.board));
   }

   if( o.added )
   {
      if( o.subscribed )     // Add subscriber
      {
         _db.modify( board_member, [&]( board_member_object& bmo )
         {
            bmo.add_subscriber( account.name );
            bmo.last_update = now; 
         });

         _db.modify( account_following, [&]( account_following_object& afo )
         {
            afo.add_following( board.name );
            afo.last_update = now; 
         });

         _db.add_board_to_feed( account.name, board.name );
      }
      else        // Add filter
      {
         _db.modify( account_following, [&]( account_following_object& afo )
         {
            afo.add_filtered( board.name );
            afo.last_update = now; 
         });
      }
   }
   else
   {
      if( o.subscribed )     // Remove subscriber
      {
         _db.modify( board_member, [&]( board_member_object& bmo )
         {
            bmo.remove_subscriber( account.name );
            bmo.last_update = now; 
         });

         _db.modify( account_following, [&]( account_following_object& afo )
         {
            afo.remove_following( board.name );
            afo.last_update = now; 
         });

         _db.remove_board_from_feed( account.name, board.name );
      }
      else        // Remove filter
      {
         _db.modify( account_following, [&]( account_following_object& afo )
         {
            afo.remove_filtered( board.name );
            afo.last_update = now; 
         });
      }
   }
   // Add new feed objects or remove old feed objects for board in account's feed.
   _db.update_board_in_feed( o.account, o.board );

} FC_CAPTURE_AND_RETHROW( (o)) }


//================================//
// === Advertising Evaluators === //
//================================//


void ad_creative_evaluator::do_apply( const ad_creative_operation& o )
{ try {
   FC_ASSERT( o.creative_id.size(), "Creative has no creative ID." );
   FC_ASSERT( o.creative_id.size() <= MAX_STRING_LENGTH, "Creative ID is too long." );
   FC_ASSERT( fc::is_utf8( o.creative_id ), "Creative ID must be UTF-8" );

   FC_ASSERT( o.objective.size(), "Creative has no objective." );
   FC_ASSERT( o.objective.size() <= MAX_STRING_LENGTH, "Objective is too long." );
   FC_ASSERT( fc::is_utf8( o.objective ), "Objective must be UTF-8" );

   FC_ASSERT( o.creative.size(), "Creative has no content." );
   FC_ASSERT( o.creative.size() <= MAX_STRING_LENGTH, "Creative is too long." );
   FC_ASSERT( fc::is_utf8( o.creative ), "Creative must be UTF-8" );

   if( o.json.size() ) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );
   }
   
   const account_name_type& signed_for = o.author;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   time_point now = _db.head_block_time();
   const account_object& author = _db.get_account( o.author );

   const auto& creative_idx = _db.get_index< ad_creative_index >().indices().get< by_creative_id >();
   auto creative_itr = creative_idx.find( boost::make_tuple( author.name, o.creative_id ) );

   if( creative_itr == creative_idx.end() )    // Ad creative does not exist
   {
      _db.create< ad_creative_object >( [&]( ad_creative_object& aco )
      {
         aco.author = author.name;
         aco.format_type = o.format_type;
         from_string( aco.creative_id, o.creative_id );
         from_string( aco.objective, o.objective );
         from_string( aco.creative, o.creative );
         from_string( aco.json, o.json );
         aco.active = o.active;
         aco.created = now;
         aco.last_updated = now;
      });

   }
   else  // Creative exists, editing
   {
      _db.modify( *creative_itr, [&]( ad_creative_object& aco )
      {
         aco.format_type = o.format_type;
         from_string( aco.objective, o.objective );
         from_string( aco.creative, o.creative );
         from_string( aco.json, o.json );
         aco.active = o.active;
         aco.last_updated = now;
      }); 
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void ad_campaign_evaluator::do_apply( const ad_campaign_operation& o )
{ try {
   FC_ASSERT( o.campaign_id.size(), "Campaign has no campaign ID." );
   FC_ASSERT( o.campaign_id.size() <= MAX_STRING_LENGTH, "Campaign ID is too long." );
   FC_ASSERT( fc::is_utf8( o.campaign_id ), "Campaign ID must be UTF-8" );
   FC_ASSERT( o.budget.amount >= 0 , "Campaign requires a budget greater than or equal to 0.");
   
   if( o.json.size() ) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );
   }
   
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   for( auto agent : o.agents )   // Ensure all agent accounts exist
   {
      const account_object& agent = _db.get_account( agent );
   }

   time_point now = _db.head_block_time();

   const auto& campaign_idx = _db.get_index< ad_campaign_index >().indices().get< by_campaign_id >();
   auto campaign_itr = campaign_idx.find( boost::make_tuple( author.name, o.campaign_id ) );

   const asset_dynamic_data_object& asset_dyn_data = get_dynamic_data( o.budget.symbol );

   if( campaign_itr == campaign_idx.end() )    // Ad campaign does not exist
   {
      FC_ASSERT( o.budget.amount > BLOCKCHAIN_PRECISION, "New Campaign requires a minimum budget of 1 Asset");
      _db.adjust_liquid_balance( o.account, -o.budget );

      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_pending_supply( o.budget );
      });

      _db.create< ad_campaign_object >( [&]( ad_campaign_object& aco )
      {
         aco.account = account.name;
         from_string( aco.campaign_id, o.campaign_id );
         from_string( aco.json, o.json );
         aco.budget = o.budget;
         aco.total_bids = asset(0, o.budget.symbol );
         aco.begin = o.begin;
         aco.end = o.end;
         aco.agents = o.agents;
         aco.agents.insert( account.name );
         aco.created = now;
         aco.last_updated = now;
         aco.active = o.active;
      });
   }
   else  // campaign exists, editing
   {
      const ad_campaign_object& campaign = *campaign_itr;
      FC_ASSERT( campaign.budget.symbol == o.budget.symbol, 
         "Budget asset must be the same as the campigns existing budget asset.");
      FC_ASSERT( o.budget * AD_RESERVE_RATIO >= campaign.total_bids, 
         "New Budget cannot bring campaign below the ad reserve ratio on its outstanding total bids. Cancel bids, or increase budget.");

      asset delta =  campaign.budget - o.budget;

      _db.adjust_liquid_balance( o.account, delta );  // Deduct balance by new additional budget amount, or refund difference. 

      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_pending_supply( -delta );
      });

      _db.modify( campaign, [&]( ad_campaign_object& aco )
      {
         from_string( aco.json, o.json );
         aco.budget = o.budget;
         aco.begin = o.begin;
         aco.end = o.end;
         aco.agents = o.agents;
         aco.last_updated = now;
         aco.active = o.active;
      }); 
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void ad_inventory_evaluator::do_apply( const ad_inventory_operation& o )
{ try {
   FC_ASSERT( o.inventory_id.size(), "inventory has no inventory ID." );
   FC_ASSERT( o.inventory_id.size() <= MAX_STRING_LENGTH, "inventory ID is too long." );
   FC_ASSERT( fc::is_utf8( o.inventory_id ), "inventory ID must be UTF-8" );
   FC_ASSERT( o.min_price.amount > 0 , "Minimum inventory price must be greater than 0");
   FC_ASSERT( o.inventory > 0 , "Inventory offering must be greater than 0");
   
   if( o.json.size() ) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );
   }
   
   const account_name_type& signed_for = o.provider;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const interface_object& interface = _db.get_interface( o.provider );
   const account_object& provider = _db.get_account( o.provider );
   for( auto audience : o.audience )   // Ensure all audiences exist
   {
      const ad_audience_object& audience = _db.get_ad_audience( provider.name, audience );
   }

   for( auto agent : o.agents )   // Ensure all agent accounts exist
   {
      const account_object& agent = _db.get_account( agent );
   }

   time_point now = _db.head_block_time();

   const auto& inventory_idx = _db.get_index< ad_inventory_index >().indices().get< by_inventory_id >();
   auto inventory_itr = inventory_idx.find( boost::make_tuple( author.name, o.inventory_id ) );

   if( inventory_itr == inventory_idx.end() )    // Ad inventory does not exist
   {
      _db.create< ad_inventory_object >( [&]( ad_inventory_object& aio )
      {
         aio.provider = provider.name;
         from_string( aio.inventory_id, o.inventory_id );
         from_string( aio.json, o.json );
         aio.metric = o.metric;
         aio.min_price = o.min_price;
         aio.inventory = o.inventory;
         aio.remaining = o.inventory;
         aio.audience = o.audience;
         aio.agents = o.agents;
         aio.agents.insert( provider.name );
         aio.created = now;
         aio.last_updated = now;
         aio.active = o.active;
      });
   }
   else  // inventory exists, editing
   {
      const ad_inventory_object& inventory = *inventory_itr;

      uint32_t prev_inventory = inventory.inventory;
      uint32_t prev_remaining = inventory.remaining;
      uint32_t delta = o.inventory - prev_inventory;
      uint32_t new_remaining = prev_remaining + delta;

      FC_ASSERT( new_remaining >= 0), "Cannot lower remaining inventory to below 0.";

      _db.modify( inventory, [&]( ad_inventory_object& aio )
      {
         from_string( aio.json, o.json );
         aio.min_price = o.min_price;
         aio.inventory = o.inventory;
         aio.remaining = new_remaining;
         aio.audience = o.audience;
         aio.agents = o.agents;
         aio.last_updated = now;
         aio.active = o.active;
      }); 
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void ad_audience_evaluator::do_apply( const ad_audience_operation& o )
{ try {
   FC_ASSERT( o.audience_id.size(), "audience has no audience ID." );
   FC_ASSERT( o.audience_id.size() <= MAX_STRING_LENGTH, "audience ID is too long." );
   FC_ASSERT( fc::is_utf8( o.audience_id ), "audience ID must be UTF-8" );
   FC_ASSERT( o.audience.size() , "Audience must include at least one account.");
   
   if( o.json.size() ) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );
   }
   const account_object& account = _db.get_account( o.account );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_general( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   for( auto account : o.audience )   // Ensure all audience member accounts exist
   {
      const account_object& member = _db.get_account( account );
   }

   time_point now = _db.head_block_time();

   const auto& audience_idx = _db.get_index< ad_audience_index >().indices().get< by_audience_id >();
   auto audience_itr = audience_idx.find( boost::make_tuple( author.name, o.audience_id ) );

   if( audience_itr == audience_idx.end() )    // Ad audience does not exist
   {
      _db.create< ad_audience_object >( [&]( ad_audience_object& aao )
      {
         aao.account = account.name;
         from_string( aao.audience_id, o.audience_id );
         from_string( aao.json, o.json );
         aao.audience = o.audience;
         aao.created = now;
         aao.last_updated = now;
         aao.active = o.active;
      });
   }
   else  // audience exists, editing
   {
      const ad_audience_object& audience = *audience_itr;

      _db.modify( audience, [&]( ad_audience_object& aao )
      {
         from_string( aao.json, o.json );
         aao.audience = o.audience;
         aao.last_updated = now;
         aao.active = o.active;
      }); 
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void ad_bid_evaluator::do_apply( const ad_bid_operation& o )
{ try {
   FC_ASSERT( o.bid_id.size(), "bid has no bid ID." );
   FC_ASSERT( o.bid_id.size() <= MAX_STRING_LENGTH, "bid ID is too long." );
   FC_ASSERT( fc::is_utf8( o.bid_id ), "bid ID must be UTF-8" );

   FC_ASSERT( o.campaign_id.size(), "campaign has no campaign ID." );
   FC_ASSERT( o.campaign_id.size() <= MAX_STRING_LENGTH, "campaign ID is too long." );
   FC_ASSERT( fc::is_utf8( o.campaign_id ), "campaign ID must be UTF-8" );

   FC_ASSERT( o.inventory_id.size(), "inventory has no inventory ID." );
   FC_ASSERT( o.inventory_id.size() <= MAX_STRING_LENGTH, "inventory ID is too long." );
   FC_ASSERT( fc::is_utf8( o.inventory_id ), "inventory ID must be UTF-8" );

   if( o.json.size() ) 
   {
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );
   }

   FC_ASSERT( o.bid_price.amount > 0 , "Minimum bid price must be greater than 0.");
   FC_ASSERT( o.requested > 0 , "Inventory request must be greater than 0." );
   const account_name_type& signed_for = o.bidder;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& bidder = _db.get_account( o.bidder );
   const account_object& account = _db.get_account( o.account );
   const account_object& provider = _db.get_account( o.provider );
   const ad_campaign_object& campaign = _db.get_ad_campaign( account.name, o.campaign_id );
   const ad_inventory_object& inventory = _db.get_ad_inventory( provider.name, o.inventory_id );

   time_point now = _db.head_block_time();

   FC_ASSERT( now < o.expiration, 
      "Bid expiration time has to be in the future." );

   FC_ASSERT( campaign.active, 
   "Campaign must be set to active to create a bid.");
   FC_ASSERT( now > campaign.begin, 
      "Campaign has not begun yet, please wait until the beginning time to create bids.");
   FC_ASSERT( now < campaign.end, 
      "Campaign has expired.");
   
   FC_ASSERT( inventory.active, 
      "Inventory must be set to active to create a bid.");
   FC_ASSERT( now < inventory.expiration, 
      "Inventory offering has expired.");
   FC_ASSERT( inventory.remaining >= o.requested, 
      "Bid request exceeds the inventory remaining from the provider.");
   FC_ASSERT( o.bid_price >= inventory.min_price, 
      "Bid price per metric must be greater than the inventory minimum price.");

   asset_symbol_type budget_asset = campaign.budget.symbol;
   asset_symbol_type inventory_asset = inventory.min_price.symbol;
   FC_ASSERT( budget_asset == inventory_asset, "Campaign's budget asset must match the inventory minimum price asset." );

   asset new_total_bids = campaign.total_bids + o.bid_price * o.requested;
   FC_ASSERT( campaign.budget * AD_RESERVE_RATIO >=  new_total_bids, 
      "Total Bids must not exceed ${r} times the budget of the campaign, increase the budget balance of the campaign.", ("r", AD_RESERVE_RATIO) );

   FC_ASSERT( campaign.is_agent( bidder.name ), 
      "Bidder must be a designated agent of the campaign to create a bid on its behalf.");

   FC_ASSERT( o.included_audiences.size(), "Bid must contain at least one audience." );

   flat_set<account_name_type> combined_audience;
   string audience_id;

   if(included_audiences.size() > 1 || excluded_audiences.size() )
   {
      for( auto audience_id : o.included_audiences )   // Ensure all includec audience objects exist
      {
         const ad_audience_object& aud = _db.get_ad_audience( bidder.name, audience_id );
         FC_ASSERT( aud.active, "Bid contains an inactive audience.");
         for( account_name_type name : aud.audience )
         {
            combined_audience.insert( name );
         }
      }
      for( auto audience_id : o.excluded_audiences )   // Ensure all excluded audience objects exist
      {
         const ad_audience_object& aud = _db.get_ad_audience( bidder.name, audience_id );
         FC_ASSERT( aud.active, "Bid contains an inactive audience.");
         for( account_name_type name : aud.audience )
         {
            combined_audience.erase( name );
         }
      }
      const auto& audience_idx = _db.get_index< ad_audience_index >().indices().get< by_audience_id >();
      auto audience_itr = audience_idx.find( boost::make_tuple( bidder.name, o.bid_id ) );
      audience_id =  o.bid_id;

      if( audience_itr == audience_idx.end() )    // Combined Ad audience for this bid_id does not exist, creating new one
      {
         _db.create< ad_audience_object >( [&]( ad_audience_object& aao )
         {
            aao.account = bidder.name;
            from_string( aao.audience_id, audience_id );
            from_string( aao.json, o.json );
            aao.audience = combined_audience;
            aao.created = now;
            aao.last_updated = now;
         });
         
      }
      else    // Bid audience already exists for this bid id, editing
      {
         const ad_audience_object& audience = *audience_itr;
         _db.modify( audience, [&]( ad_audience_object& aao )
         {
            from_string( aao.json, o.json );
            aao.audience = combined_audience;
            aao.last_updated = now;
         }); 
      }
   }
   else
   {
      audience_id = o.included_audiences[0];
   }

   const auto& bid_idx = _db.get_index< ad_bid_index >().indices().get< by_bid_id >();
   auto bid_itr = bid_idx.find( boost::make_tuple( bidder.name, o.bid_id ) );

   if( bid_itr == bid_idx.end() )    // Ad bid does not exist, creating new bid.
   {
      FC_ASSERT( o.active, "Bid does not exist, set active to true." );
      _db.create< ad_bid_object >( [&]( ad_bid_object& abo )
      {
         abo.bidder = bidder.name;
         from_string( abo.bid_id, o.bid_id );
         abo.account = account.name;
         from_string( abo.campaign_id, o.campaign_id );
         from_string( abo.creative_id, o.creative_id );
         abo.provider = provider.name;
         from_string( abo.inventory_id, o.inventory_id );
         from_string( abo.audience_id, audience_id );
         abo.bid_price = o.bid_price;
         abo.requested = o.requested;
         abo.remaining = o.requested;
         abo.created = now;
         abo.last_updated = now;
         abo.expiration = o.expiration;
      });

      _db.modify( campaign, [&]( ad_campaign_object& aco )
      {
         aco.total_bids = new_total_bids;
      }); 
   }
   else  
   {
      const ad_bid_object& bid = *bid_itr;
      uint32_t prev_requested = bid.requested;
      uint32_t prev_remaining = bid.remaining;
      asset prev_price = bid.bid_price;

      if(o.active)    // Bid exists and is being edited.
      {
         uint32_t delta_req = o.requested - prev_requested;
         asset delta_price = o.bid_price - prev_price;
         uint32_t new_remaining = prev_remaining + delta_req;
         FC_ASSERT( new_remaining >= 0, "New inventory requested remaining must be equal to or greater than zero.");
         
         asset delta_total_bids = prev_remaining * delta_price + delta_req * o.bid_price;

         _db.modify( bid, [&]( ad_bid_object& abo )
         {
            abo.bid_price = o.bid_price;
            abo.requested = o.requested;
            abo.remaining = new_remaining;
            abo.last_updated = now;
            abo.expiration = o.expiration;
         }); 

         if( delta_total_bids != asset(0, inventory_asset ))
         {
            _db.modify( campaign, [&]( ad_campaign_object& aco )
            {
               aco.total_bids += delta_total_bids;
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

         _db.remove( bid );
      }  
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void ad_deliver_evaluator::do_apply( const ad_deliver_operation& o )
{ try {
   FC_ASSERT( o.bid_id.size(), "Delivery has no bid ID." );
   FC_ASSERT( o.bid_id.size() <= MAX_STRING_LENGTH, "bid ID is too long." );
   FC_ASSERT( fc::is_utf8( o.bid_id ), "bid ID must be UTF-8" );

   FC_ASSERT( o.delivery_price.amount > 0 , "Delivery price must be greater than 0.");
   FC_ASSERT( o.delivered > 0 , "Delivery amount must be greater than 0." );
   
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_business_object& b = _db.get_account_business( signed_for );
      const account_object& signatory = _db.get_account( o.signatory );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_object& bidder = _db.get_account( o.bidder );
   const ad_bid_object& bid = _db.get_ad_bid( bidder.name, o.bid_id );
   const account_object& advertiser = _db.get_account( bid.account );
   const account_object& provider = _db.get_account( bid.provider );
   const ad_campaign_object& campaign = _db.get_ad_campaign( advertiser.name, bid.campaign_id );
   const ad_inventory_object& inventory = _db.get_ad_inventory( provider.name, bid.inventory_id );
   const ad_audience_object& audience = _db.get_ad_audience( bidder.name , bid.audience_id );
   const ad_creative_object& creative = _db.get_ad_creative( account.name, bid.creative_id );
   const account_object& demand = _db.get_account( campaign.interface );
   
   FC_ASSERT( inventory.is_agent( account.name ), 
      "Delivering account must be a designated agent of the Inventory.");
   FC_ASSERT( campaign.is_agent( bidder.name ), 
      "Bidder account is no longer an agent of the campaign, bid is invalid for delivery.");

   time_point now = _db.head_block_time();
   asset_symbol_type bid_asset = bid.bid_price.symbol;
   asset_symbol_type delivery_asset = o.delivery_price.symbol;

   FC_ASSERT( bid_asset == delivery_asset,
      "Delivery price asset must match the bid price asset." );
   FC_ASSERT( o.delivery_price <= bid.bid_price, 
      "Delivery Price must be equal to or less than Bid price");
   FC_ASSERT( o.delivered <= bid.remaining, 
      "Delivery size must be equal to or less than the bid's remaining inventory request.");
   FC_ASSERT( now < bid.expiration, 
      "Bid has expired.");

   FC_ASSERT( campaign.active,
      "Campaign must be set to active to deliver inventory.");
   FC_ASSERT( now > campaign.begin,
      "Campaign has not begun yet, please wait until the beginning time to create bids.");
   FC_ASSERT( now < campaign.end,
      "Campaign has expired.");
   
   FC_ASSERT( inventory.active, 
      "Inventory must be set to active to create a bid.");
   FC_ASSERT( now < inventory.expiration, 
      "Inventory offering has expired.");
   FC_ASSERT( inventory.remaining >= o.delivered, 
      "Delivery amount exceeds the inventory remaining from the provider. Please increase the inventory provided.");

   asset delivery_value = o.delivery_price * o.delivered;

   FC_ASSERT( campaign.budget >=  delivery_value, 
      "Campaign does not have sufficient budget to pay the delivery." );

   uint32_t valid_inventory_count = 0;
   metric_types metric = inventory.metric;
   flat_set<account_object&> inventory_accounts;
   
   for( transaction_id_type txn : o.transactions )     // Validates the inventory transactions in the delivery
   {
      annotated_signed_transaction metric_txn = _db.get_transaction( txn );
      FC_ASSERT( metric_txn.operations.size(), "Transaction ID ${t} has no operations.", ("t", txn) );
      operation& op = metric_txn.operations[0];

      switch(metric)
      {
         case VIEW_METRIC:
         {
            if( op.which() == operation::tag< view_operation >::value )
            {
               view_operation& view_op = op.get< view_operation >();
               FC_ASSERT( audience.is_audience( view_op.viewer ), "Account is not within the bid audience." );
               FC_ASSERT( view_op.author == creative.author, "View Transaction author is not the creative's objective author." );
               FC_ASSERT( view_op.permlink == creative.objective, "View Transaction permlink is not the creative's objective permlink." );
               FC_ASSERT( view_op.interface == inventory.provider, "View Transaction interface is not the inventory provider." );
               const account_object& acc = _db.get_account( view_op.viewer );
               FC_ASSERT( acc.membership == NONE, "Account: ${a} is a member, and should not receive advertising.",("a", acc.name ) );
               inventory_accounts.insert( acc );
               valid_inventory_count++;
            }
            break;
         }
         case VOTE_METRIC:
         {
            if( op.which() == operation::tag< vote_operation >::value )
            {
               vote_operation& vote_op = op.get< vote_operation >();
               FC_ASSERT( audience.is_audience( vote_op.voter ), "Account is not within the bid audience." );
               FC_ASSERT( vote_op.author == creative.author, "Vote Transaction author is not the creative's objective author." );
               FC_ASSERT( vote_op.permlink == creative.objective, "vote Transaction permlink is not the creative's objective permlink." );
               FC_ASSERT( vote_op.interface == inventory.provider, "vote Transaction interface is not the inventory provider." );
               const account_object& acc = _db.get_account( vote_op.voter );
               FC_ASSERT( acc.membership == NONE, "Account: ${a} is a member, and should not receive advertising.",("a", acc.name ) );
               inventory_accounts.insert( acc );
               valid_inventory_count++;
            }
            break;
         }
         case SHARE_METRIC:
         {
            if( op.which() == operation::tag< share_operation >::value )
            {
               share_operation& share_op = op.get< share_operation >();
               FC_ASSERT( audience.is_audience( share_op.sharer ), "Account is not within the bid audience." );
               FC_ASSERT( share_op.author == creative.author, "Share Transaction author is not the creative's objective author." );
               FC_ASSERT( share_op.permlink == creative.objective, "Share Transaction permlink is not the creative's objective permlink." );
               FC_ASSERT( share_op.interface == inventory.provider, "Share Transaction interface is not the inventory provider." );
               const account_object& acc = _db.get_account( share_op.sharer );
               FC_ASSERT( acc.membership == NONE, "Account: ${a} is a member, and should not receive advertising.",("a", acc.name ) );
               inventory_accounts.insert( acc );
               valid_inventory_count++;
            }
            break;
         } 
         case FOLLOW_METRIC:
         {
            if( op.which() == operation::tag< account_follow_operation >::value )
            {
               account_follow_operation& follow_op = op.get< account_follow_operation >();
               FC_ASSERT( audience.is_audience( follow_op.follower ), "Account is not within the bid audience." );
               FC_ASSERT( follow_op.following == creative.author, "Follow Transaction following is not the creative's objective author." );
               FC_ASSERT( follow_op.details == creative.objective, "Follow Transaction details is not the creative's objective details." );
               FC_ASSERT( follow_op.interface == inventory.provider, "Follow Transaction interface is not the inventory provider." );
               const account_object& acc = _db.get_account( follow_op.follower );
               FC_ASSERT( acc.membership == NONE, "Account: ${a} is a member, and should not receive advertising.",("a", acc.name ) );
               inventory_accounts.insert( acc );
               valid_inventory_count++;
            }
            break;
         } // TODO purchase, premium.
         case PREMIUM_METRIC:
         {

         }
         case PURCHASE_METRIC:
         {

         }
      }
   }

   FC_ASSERT( valid_inventory_count >= o.delivered, 
      "Transaction does not deliver the corect amount of valid inventory transactions." );

   _db.modify( campaign, [&]( ad_campaign_object& aco )
   {
      aco.budget -= delivery_value;
      aco.total_bids -= delivery_value;
      aco.last_updated = now;
   });

   _db.modify( inventory, [&]( ad_inventory_object& aio )
   {
      aio.remaining -= o.delivered;
      aio.last_updated = now;
   });

   _db.modify( bid, [&]( ad_bid_object& abo )
   {
      abo.remaining -= o.delivered;
      abo.last_updated = now;
   });

   _db.pay_advertising_delivery( provider, demand, bidder, account, inventory_accounts, delivery_value );

   if( bid.remaining == 0 )
   {
      _db.remove( bid );
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


//=============================//
// === Transfer Evaluators === //
//=============================//



/**
 * Transfers an amount of a liquid asset balance from one account to another
 * TODO: Asset white and blacklist checking, and flag checking
 */
void transfer_evaluator::do_apply( const transfer_operation& o )
{ try {
   const account_name_type& signed_for = o.from;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& from_account = _db.get_account(o.from);
   const account_object& to_account = _db.get_account(o.to);
  
   asset liquid = _db.get_liquid_balance( from_account.name, o.amount.symbol );

   FC_ASSERT( liquid >= o.amount, 
      "Account does not have sufficient funds for transfer." );

   time_point now = _db.head_block_time();
    const asset_object& asset = _db.get_asset(o.amount.symbol);

   const account_permission_object& to_account_permissions = _db.get_account_permissions(o.to);
   FC_ASSERT( to_account_permissions.is_authorized_asset( asset ), 
      "Transfer is not authorized, due to recipient account's asset permisssions" );

   const account_permission_object& from_account_permissions = _db.get_account_permissions(o.from);
   FC_ASSERT( from_account_permissions.is_authorized_asset( asset ), 
      "Transfer is not authorized, due to sender account's asset permisssions" );

   if( from_account.active_challenged )
   {
      _db.modify( from_account, [&]( account_object& a )
      {
         a.last_transfer = now;
      });
   }

   

   vector<string> part; 
   part.reserve(4);
   auto path = o.memo;
   boost::split( part, path, boost::is_any_of("/") );
   if( part.size() > 1 && part[0].size() && part[0][0] == '@' )      // find memo reference to comment, add transfer to payments received.
   {
      auto acnt = part[0].substr(1);
      auto perm = part[1];

      auto comment_ptr = _db.find_comment( acnt, perm );
      if( comment_ptr != nullptr )
      {
         const comment_object& comment = *comment_ptr;
         
         _db.modify( comment, [&]( comment_object& co )
         {
            if( co.payments_received[ o.from ].size() )
            {
               if( co.payments_received[ o.from ][ o.amount.symbol ].size() )
               {
                  co.payments_received[ o.from ][ o.amount.symbol ] += o.amount;
               }
               else
               {
                  co.payments_received[ o.from ][ o.amount.symbol ] = o.amount;
               }
            }
            else
            {
               co.payments_received[ o.from ][ o.amount.symbol ] = o.amount;
            }
         });
      }
   }

   _db.adjust_liquid_balance( from_account.name, -o.amount );
   _db.adjust_liquid_balance( to_account.name, o.amount );

} FC_CAPTURE_AND_RETHROW( (o)) }


//  TODO: Limit transaction requests to connected accounts. Add limitations based on amounts and connection status.
void transfer_request_evaluator::do_apply( const transfer_request_operation& o )
{ try {
   const account_name_type& signed_for = o.to;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& from_account = _db.get_account(o.from);
   const account_object& to_account = _db.get_account(o.to);
   const asset_object& asset = _db.get_asset(o.amount.symbol);
   time_point now = _db.head_block_time();
   const account_permission_object& to_account_permissions = _db.get_account_permissions(o.to);
   FC_ASSERT( to_account_permissions.is_authorized_asset( asset ), "Transfer is not authorized, due to recipient account's asset permisssions" );
   const account_permission_object& from_account_permissions = _db.get_account_permissions(o.from);
   FC_ASSERT( from_account_permissions.is_authorized_asset( asset ), "Transfer is not authorized, due to sender account's asset permisssions" );
   FC_ASSERT( _db.get_liquid_balance( from_account.name, o.amount.symbol ) >= o.amount, "Account does not have sufficient funds for transfer." );

   const auto& req_idx = _db.get_index< transfer_request_index >().indices().get< by_request_id >();
   auto req_itr = req_idx.find( boost::make_tuple( to_account.name, o.request_id ) );

   if( req_itr == req_idx.end() )    // Transfer request does not exist, creating new request.
   {
      FC_ASSERT( o.requested , "Transfer request does not exist to cancel, set requested to true.");
      _db.create< transfer_request_object >( [&]( transfer_request_object& tro )
      {
         tro.to = to_account.name;
         tro.from = from_account.name;
         from_string( tro.memo, o.memo );
         from_string( tro.request_id, o.request_id );
         tro.amount = o.amount;
         tro.expiration = now + TRANSFER_REQUEST_DURATION;
      });
   }
   else
   {
      if( o.requested)
      {
         _db.modify( *req_itr, [&]( transfer_request_object& tro )
         {
            from_string( tro.memo, o.memo );
            tro.amount = o.amount;
         });
      }
      else
      {
         _db.remove( *req_itr );
      }
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void transfer_accept_evaluator::do_apply( const transfer_accept_operation& o )
{ try {
   const account_name_type& signed_for = o.from;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& from_account = _db.get_account(o.from);
   const account_object& to_account = _db.get_account(o.to);
   const asset_object& asset = _db.get_asset(o.amount.symbol);
   time_point now = _db.head_block_time();

   const account_permission_object& to_account_permissions = _db.get_account_permissions(o.to);
   FC_ASSERT( to_account_permissions.is_authorized_asset( asset ), "Transfer is not authorized, due to recipient account's asset permisssions" );

   const account_permission_object& from_account_permissions = _db.get_account_permissions(o.from);
   FC_ASSERT( from_account_permissions.is_authorized_asset( asset ), "Transfer is not authorized, due to sender account's asset permisssions" );

   FC_ASSERT( _db.get_liquid_balance( from_account.name, o.amount.symbol ) >= o.amount, "Account does not have sufficient funds for transfer." );

   const transfer_request_object& request = _db.get_transfer_request( to_account.name, o.request_id );
   FC_ASSERT( now < request.expiration, "Request has expired." ); 

   if( o.accepted )
   {
      FC_ASSERT( _db.get_liquid_balance( from_account.name, request.amount.symbol ) >= request.amount, 
         "Account does not have sufficient funds for transfer." );
      _db.adjust_liquid_balance( request.to, -request.amount );
      _db.adjust_liquid_balance( request.name, request.amount );
   }
   else
   {
      _db.remove( request );
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void transfer_recurring_evaluator::do_apply( const transfer_recurring_operation& o )
{ try {
   const account_name_type& signed_for = o.from;
   if( o.signatory != o.get_creator_name() )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& from_account = _db.get_account(o.from);
   const account_object& to_account = _db.get_account(o.to);
   const asset_object& asset = _db.get_asset(o.amount.symbol);
   time_point now = _db.head_block_time();

   const account_permission_object& to_account_permissions = _db.get_account_permissions(o.to);
   FC_ASSERT( to_account_permissions.is_authorized_asset( asset ), "Transfer is not authorized, due to recipient account's asset permisssions" );

   const account_permission_object& from_account_permissions = _db.get_account_permissions(o.from);
   FC_ASSERT( from_account_permissions.is_authorized_asset( asset ), "Transfer is not authorized, due to sender account's asset permisssions" );

   FC_ASSERT( _db.get_liquid_balance( from_account.name, o.amount.symbol ) >= o.amount, "Account does not have sufficient funds for first transfer." );

   const auto& recurring_idx = _db.get_index< transfer_recurring_index >().indices().get< by_transfer_id >();
   auto recurring_itr = recurring_idx.find( boost::make_tuple( from_account.name, o.transfer_id ) );

   if( recurring_itr == recurring_idx.end() )    // Recurring transfer does not exist, creating new recurring transfer.
   {
      FC_ASSERT( o.active , "Recurring transfer does not exist to cancel, set active to true.");
      _db.create< transfer_recurring_object >( [&]( transfer_recurring_object& tro )
      {
         tro.to = to_account.name;
         tro.from = from_account.name;
         from_string( tro.memo, o.memo );
         from_string( tro.transfer_id, o.transfer_id );                 
         tro.amount = o.amount;
         tro.begin = o.begin;
         tro.end = o.end;
         tro.interval = o.interval;
         tro.next_transfer = o.begin;
      });
   }
   else
   {
      if( o.active)
      {
         _db.modify( *recurring_itr, [&]( transfer_recurring_object& tro )
         {
            from_string( tro.memo, o.memo );
            tro.amount = o.amount;
            tro.begin = o.begin;
            tro.end = o.end;
            tro.interval = o.interval;
         });
      }
      else
      {
         _db.remove( *recurring_itr );
      }
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


// Limitations on daily recurring transfer spend based on connection status. 
void transfer_recurring_request_evaluator::do_apply( const transfer_recurring_request_operation& o )
{ try {
   const account_object& from_account = _db.get_account(o.from);
   const account_object& to_account = _db.get_account(o.to);
   const account_name_type& signed_for = o.to;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const asset_object& asset = _db.get_asset(o.amount.symbol);
   time_point now = _db.head_block_time();

   const account_permission_object& to_account_permissions = _db.get_account_permissions(o.to);
   FC_ASSERT( to_account_permissions.is_authorized_asset( asset ), 
      "Transfer is not authorized, due to recipient account's asset permisssions" );

   const account_permission_object& from_account_permissions = _db.get_account_permissions(o.from);
   FC_ASSERT( from_account_permissions.is_authorized_asset( asset ), 
      "Transfer is not authorized, due to sender account's asset permisssions" );

   FC_ASSERT( _db.get_liquid_balance( from_account.name, o.amount.symbol ) >= o.amount, 
      "Account does not have sufficient funds for first transfer." );

   const auto& req_idx = _db.get_index< transfer_recurring_request_index >().indices().get< by_request_id >();
   auto req_itr = req_idx.find( boost::make_tuple( to_account.name, o.request_id ) );

   if( req_itr == req_idx.end() )    // Recurring transfer request does not exist, creating new recurring transfer request.
   {
      FC_ASSERT( o.requested , "Recurring transfer request does not exist to cancel, set active to true.");
      _db.create< transfer_recurring_request_object >( [&]( transfer_recurring_request_object& trro )
      {
         trro.to = to_account.name;
         trro.from = from_account.name;
         from_string( trro.memo, o.memo );
         from_string( trro.request_id, o.request_id );                 
         trro.amount = o.amount;
         trro.begin = o.begin;
         trro.end = o.end;
         trro.interval = o.interval;
         trro.expiration = now + TRANSFER_REQUEST_DURATION;
      });
   }
   else
   {
      if( o.requested)
      {
         _db.modify( *req_itr, [&]( transfer_recurring_request_object& trro )
         {
            from_string( trro.memo, o.memo );
            trro.amount = o.amount;
            trro.begin = o.begin;
            trro.end = o.end;
            trro.interval = o.interval;
         });
      }
      else
      {
         _db.remove( *req_itr );
      }
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


void transfer_recurring_accept_evaluator::do_apply( const transfer_recurring_accept_operation& o )
{ try {
   const account_object& from_account = _db.get_account( o.from );
   const account_name_type& signed_for = o.from;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& to_account = _db.get_account( o.to );
   const transfer_recurring_request_object& request = _db.get_transfer_recurring_request( to_account.name, o.request_id );
   const asset_object& asset = _db.get_asset( request.amount.symbol );
   time_point now = _db.head_block_time();
   const account_permission_object& to_account_permissions = _db.get_account_permissions( o.to );
   FC_ASSERT( to_account_permissions.is_authorized_asset( asset ), "Transfer is not authorized, due to recipient account's asset permisssions" );
   const account_permission_object& from_account_permissions = _db.get_account_permissions( o.from );
   FC_ASSERT( from_account_permissions.is_authorized_asset( asset ), "Transfer is not authorized, due to sender account's asset permisssions" );
   FC_ASSERT( _db.get_liquid_balance( request.from, request.amount.symbol ) >= request.amount, "Account does not have sufficient funds for first transfer." );

   const auto& recurring_idx = _db.get_index< transfer_recurring_index >().indices().get< by_transfer_id >();
   auto recurring_itr = recurring_idx.find( boost::make_tuple( from_account.name, o.request_id ) );   // Request ID is used as new transfer ID when accepted. 

   FC_ASSERT( recurring_itr == recurring_idx.end(), 
      "Recurring transfer with this ID already exists, cannot accept again." );   // Recurring transfer does not exist.
   
   if( o.accepted )   // Accepting transfer by creating new transfer.
   {
      _db.create< transfer_recurring_object >( [&]( transfer_recurring_object& tro )
      {
         tro.to = request.to;
         tro.from = request.from;
         from_string( tro.memo, request.memo );
         from_string( tro.transfer_id, request.request_id );                 
         tro.amount = request.amount;
         tro.begin = request.begin;
         tro.end = request.end;
         tro.interval = request.interval;
         tro.next_transfer = request.begin;
      });
   }
   else    // Rejecting recurring transfer request.
   {
      _db.remove( request );
   }
} FC_CAPTURE_AND_RETHROW( (o)) }


//============================//
// === Balance Evaluators === //
//============================//


void claim_reward_balance_evaluator::do_apply( const claim_reward_balance_operation& o )
{
   const account_balance_object& account_balance = _db.get_account_balance( o.account, o.reward.symbol );
   const account_object& acc = _db.get_account( o.account);
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_officer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   FC_ASSERT( o.reward.amount <= account_balance.reward_balance, 
      "Cannot claim requested reward assets. Claimed: ${c} Available: ${a}", ("c", o.reward.amount)("a", account_balance.reward_balance) );

   asset shared_reward = asset( 0, o.reward.symbol );

   if( acc.revenue_share )     // Distributes revenue from rewards to listed equity and credit assets for business accounts. 
   {
      for( auto eq : acc.equity_revenue_shares )
      {
         const asset_equity_data_object& equity = get_equity_data( eq.first );
         asset share = ( o.reward * eq.second ) / PERCENT_100;
         shared_reward += share;
         _db.modify( equity, [&]( asset_equity_data_object& aedo ) 
         {
            aedo.adjust_pool( share );
         }); 
      }
      for( auto cr : acc.credit_revenue_shares )
      {
         const asset_credit_data_object& credit = get_credit_data( cr.first );
         asset share = ( o.reward * cr.second ) / PERCENT_100;
         shared_reward += share;
         _db.modify( credit, [&]( asset_credit_data_object& acdo ) 
         {
            acdo.adjust_pool( share );
         }); 
      }
   }

   const asset& reward = o.reward - shared_reward;
   const asset& staked_reward = ( reward * REWARD_STAKED_PERCENT ) / PERCENT_100;
   const asset& liquid_reward = reward - staked_reward;

   _db.adjust_reward_balance( acc, -o.reward );
   _db.adjust_liquid_balance( acc, liquid_reward );
   _db.adjust_staked_balance( acc, staked_reward );

   _db.modify( acc, [&]( account_object& a ) 
   {
      a.total_rewards += reward.amount;
   }); 
}


void stake_asset_evaluator::do_apply( const stake_asset_operation& o )
{
   const account_object& from_account = _db.get_account(o.from);
   const account_name_type& signed_for = o.from;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& to_account = o.to.size() ? _db.get_account(o.to) : from_account;
   const account_balance_object& account_balance = _db.get_account_balance( from_account.name, o.amount.symbol );
   const asset_object& asset_object = _db.get_asset( o.amount.symbol );
   time_point now = _db.head_block_time();

   FC_ASSERT( _db.get_liquid_balance( from_account, o.amount.symbol) >= o.amount, "Account does not have sufficient liquid balance for stake operation." );

   if( o.amount.amount == 0 )
   {
      FC_ASSERT( account_balance.stake_rate != 0, "This operation would not change the stake rate." );

      _db.modify( account_balance, [&]( account_balance_object& abo ) 
      {
         abo.stake_rate = 0;
         abo.next_stake_time = time_point::maximum();
         abo.to_stake = 0;
         abo.total_staked = 0;

         abo.unstake_rate = 0;
         abo.next_unstake_time = time_point::maximum();
         abo.to_unstake = 0;
         abo.total_unstaked = 0;
      });
   }
   else
   {
      if( asset_object.stake_intervals == 0 )
      {
         _db.adjust_liquid_balance( from_account, -o.amount );
         _db.adjust_staked_balance( to_account, o.amount );
      }
      else if( asset_object.stake_intervals >= 1 )
      {
         share_type new_stake_rate = ( o.amount.amount / asset_object.stake_intervals );
         FC_ASSERT( account_balance.stake_rate != new_stake_rate, "This operation would not change the stake rate." );
         _db.modify( account_balance, [&]( account_balance_object& abo )
         {
            abo.stake_rate = new_stake_rate;
            abo.next_stake_time = now + fc::seconds(STAKE_WITHDRAW_INTERVAL_SECONDS);
            abo.to_stake = o.amount.amount;
            abo.total_staked = 0;

            abo.unstake_rate = 0;
            abo.next_unstake_time = time_point::maximum();
            abo.to_unstake = 0;
            abo.total_unstaked = 0;
         });
      } 
   }
}


void unstake_asset_evaluator::do_apply( const unstake_asset_operation& o )
{
   const account_object& from_account = _db.get_account(o.from);
   const account_name_type& signed_for = o.from;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& to_account = o.to.size() ? _db.get_account(o.to) : from_account;
   const account_balance_object& account_balance = _db.get_account_balance( from_account.name, o.amount.symbol );
   const asset_object& asset_object = _db.get_asset( o.amount.symbol );
   asset stake = _db.get_staked_balance( o.account, o.amount.symbol);
   time_point now = _db.head_block_time();

   FC_ASSERT( stake >= o.amount, "Account does not have sufficient staked balance for unstaking." );

   if( !account.mined && o.amount.symbol == SYMBOL_COIN)
   {
      const auto& props = _db.get_dynamic_global_properties();
      const witness_schedule_object& wso = _db.get_witness_schedule();
      asset min_stake = asset(wso.median_props.account_creation_fee * 10, SYMBOL_COIN);
      FC_ASSERT( stake >= min_stake, "Account registered by another account requires 10x account creation before it can be powered down." );
   }

   if( o.amount.amount == 0 )
   {
      FC_ASSERT( account_balance.unstake_rate != 0, "This operation would not change the unstake rate." );

      _db.modify( account_balance, [&]( account_balance_object& abo ) 
      {
         abo.stake_rate = 0;
         abo.next_stake_time = time_point::maximum();
         abo.to_stake = 0;
         abo.total_staked = 0;

         abo.unstake_rate = 0;
         abo.next_unstake_time = time_point::maximum();
         abo.to_unstake = 0;
         abo.total_unstaked = 0;
      });
   }
   else
   {
      if( asset_object.unstake_intervals == 0 )
      {
         _db.adjust_staked_balance( from_account, -o.amount );
         _db.adjust_liquid_balance( to_account, o.amount );
      }
      else if( asset_object.unstake_intervals >= 1 )
      {
         share_type new_unstake_rate = ( o.amount.amount / asset_object.unstake_intervals );
         FC_ASSERT( account_balance.unstake_rate != new_unstake_rate, "This operation would not change the unstake rate." );
         _db.modify( account_balance, [&]( account_balance_object& abo )
         {
            abo.stake_rate = 0;
            abo.next_stake_time = time_point::maximum();
            abo.to_stake = 0;
            abo.total_staked = 0;

            abo.unstake_rate = new_unstake_rate;
            abo.next_unstake_time = now + fc::seconds(STAKE_WITHDRAW_INTERVAL_SECONDS);
            abo.to_unstake = o.amount.amount;
            abo.total_unstaked = 0;
         });
      }
   }
}


void unstake_asset_route_evaluator::do_apply( const unstake_asset_route_operation& o )
{ try {
   const auto& from_account = _db.get_account( o.from_account );
   const account_name_type& signed_for = o.from_account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const auto& to_account = _db.get_account( o.to_account );
   const auto& wd_idx = _db.get_index< unstake_asset_route_index >().indices().get< by_withdraw_route >();
   auto itr = wd_idx.find( boost::make_tuple( from_account.name, to_account.name ) );

   if( itr == wd_idx.end() )
   {
      FC_ASSERT( o.percent != 0, "Cannot create a 0% destination." );
      FC_ASSERT( from_account.withdraw_routes < MAX_WITHDRAW_ROUTES, "Account already has the maximum number of routes." );

      _db.create< unstake_asset_route_object >( [&]( unstake_asset_route_object& wvdo )
      {
         wvdo.from_account = from_account.name;
         wvdo.to_account = to_account.name;
         wvdo.percent = o.percent;
         wvdo.auto_stake = o.auto_stake;
      });

      _db.modify( from_account, [&]( account_object& a )
      {
         a.withdraw_routes++;
      });
   }
   else if( o.percent == 0 )
   {
      _db.remove( *itr );

      _db.modify( from_account, [&]( account_object& a )
      {
         a.withdraw_routes--;
      });
   }
   else
   {
      _db.modify( *itr, [&]( unstake_asset_route_object& wvdo )
      {
         wvdo.from_account = from_account.name;
         wvdo.to_account = to_account.name;
         wvdo.percent = o.percent;
         wvdo.auto_stake = o.auto_stake;
      });
   }

   itr = wd_idx.upper_bound( boost::make_tuple( from_account.name, account_name_type() ) );
   uint16_t total_percent = 0;

   while( itr->from_account == from_account.name && itr != wd_idx.end() )
   {
      total_percent += itr->percent;
      ++itr;
   }

   FC_ASSERT( total_percent <= PERCENT_100, "More than 100% of unstake operations allocated to destinations." );
} FC_CAPTURE_AND_RETHROW() }


void transfer_to_savings_evaluator::do_apply( const transfer_to_savings_operation& o )
{
   const auto& from = _db.get_account( o.from );
   const account_name_type& signed_for = o.from;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const auto& to   = _db.get_account(o.to);
   FC_ASSERT( _db.get_liquid_balance( from, o.amount.symbol ) >= o.amount, "Account does not have sufficient funds to transfer to savings." );

   _db.adjust_liquid_balance( from, -o.amount );
   _db.adjust_savings_balance( to, o.amount );
}


void transfer_from_savings_evaluator::do_apply( const transfer_from_savings_operation& o )
{
   const account_object& from = _db.get_account( o.from );
   const account_name_type& signed_for = o.from;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   _db.get_account(o.to); // Verify to account exists
   time_point now = _db.head_block_time();
   FC_ASSERT( from.savings_withdraw_requests < SAVINGS_WITHDRAW_REQUEST_LIMIT, "Account has reached limit for pending withdraw requests." );

   FC_ASSERT( _db.get_savings_balance( from, o.amount.symbol ) >= o.amount );
   _db.adjust_savings_balance( from, -o.amount );
   _db.create<savings_withdraw_object>( [&]( savings_withdraw_object& s ) 
   {
      s.from   = o.from;
      s.to     = o.to;
      s.amount = o.amount;
#ifndef IS_LOW_MEM
      from_string( s.memo, o.memo );
#endif
      s.request_id = o.request_id;
      s.complete = now + SAVINGS_WITHDRAW_TIME;
   });

   _db.modify( from, [&]( account_object& a )
   {
      a.savings_withdraw_requests++;
   });
}

void cancel_transfer_from_savings_evaluator::do_apply( const cancel_transfer_from_savings_operation& o )
{
   const auto& swo = _db.get_savings_withdraw( o.from, o.request_id );
   const account_name_type& signed_for = o.from;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   _db.adjust_savings_balance( _db.get_account( swo.from ), swo.amount );
   _db.remove( swo );

   const auto& from = _db.get_account( o.from );
   _db.modify( from, [&]( account_object& a )
   {
      a.savings_withdraw_requests--;
   });
}


void delegate_asset_evaluator::do_apply( const delegate_asset_operation& o )
{
   const auto& delegator_balance = _db.get_account_balance( o.delegator, o.amount.symbol );
   const account_name_type& signed_for = o.delegator;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const auto& delegatee_balance = _db.get_account_balance( o.delegatee, o.amount.symbol );

   const auto& delegator_account = _db.get_account( o.delegator );
   const auto& delegatee_account = _db.get_account( o.delegatee );
   time_point now = _db.head_block_time();
   auto delegation = _db.find< asset_delegation_object, by_delegation >( boost::make_tuple( o.delegator, o.delegatee ) );

   asset available_stake = delegator_balance.get_staked_balance() - delegator_balance.get_delegated_balance() - asset( delegator_balance.to_unstake - delegator_balance.total_unstaked, o.amount.symbol );

   const auto& wso = _db.get_witness_schedule();
   const auto& gpo = _db.get_dynamic_global_properties();

   if( delegation == nullptr ) // If delegation doesn't exist, create it
   {
      FC_ASSERT( available_stake >= o.amount, "Delegation Error: Account does not have enough stake to delegate." );

      _db.create< asset_delegation_object >( [&]( asset_delegation_object& ado ) 
      {
         ado.delegator = o.delegator;
         ado.delegatee = o.delegatee;
         ado.amount = o.amount;
         ado.min_delegation_time = now;
      }); 

      _db.adjust_delegated_balance( delegator_account, o.amount); // increase delegated balance of delegator account
      _db.adjust_receiving_balance( delegatee_account, o.amount); // increase receiving balance of delegatee account
   }
   else if( o.amount >= delegation->amount ) // Else if the delegation is increasing
   {
      asset delta = o.amount - delegation->amount;

      FC_ASSERT( available_stake >= o.amount - delegation->amount, "Delegation Error: Account does not have enough stake to delegate." );

      _db.adjust_delegated_balance( delegator_account, delta); // increase delegated balance of delegator account
      _db.adjust_receiving_balance( delegatee_account, delta); // increase receiving balance of delegatee account

      _db.modify( *delegation, [&]( asset_delegation_object& ado )
      {
         ado.amount = o.amount;
      });
   }
   else // Else the delegation is decreasing (delegation->asset > o.amount)
   {
      asset delta = delegation->amount - o.amount;

      if( o.amount.amount == 0 )
      {
         FC_ASSERT( delegation->amount.amount > 0, "Delegation error: Delegation would be set to zero, but it is already zero");
      }

      _db.create< asset_delegation_expiration_object >( [&]( asset_delegation_expiration_object& obj )
      {
         obj.delegator = o.delegator;
         obj.amount = delta;
         obj.expiration = std::max( now + CONTENT_REWARD_INTERVAL, delegation->min_delegation_time );
      });

      _db.adjust_delegated_balance( delegator_account, -delta); // decrease delegated balance of delegator account
      _db.adjust_receiving_balance( delegatee_account, -delta); // decrease receiving balance of delegatee account

      if( o.amount.amount > 0 )
      {
         _db.modify( *delegation, [&]( asset_delegation_object& ado )
         {
            ado.amount = o.amount;
         });
      }
      else
      {
         _db.remove( *delegation );
      }
   }
}


//===========================//
// === Escrow Evaluators === //
//===========================//


void escrow_transfer_evaluator::do_apply( const escrow_transfer_operation& o )
{ try {
   FC_ASSERT( o.amount.symbol == o.fee.symbol, "Fee Asset must be identical to escrow payment asset.");
   time_point now = _db.head_block_time();
   FC_ASSERT( o.ratification_deadline > now, "The escrow ratification deadline must be after head block time." );
   FC_ASSERT( o.escrow_expiration > now, "The escrow expiration must be after head block time." );

   const auto& from_account_balance = _db.get_account_balance(o.from, o.amount.symbol);
   
   const account_object& to_account = _db.get_account(o.to);
   const account_object& from_account = _db.get_account(o.from);
   const account_name_type& signed_for = o.from;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& agent_account = _db.get_account(o.agent);

   asset escrow_total = o.amount + o.fee;
   
   FC_ASSERT( from_account_balance.liquid_balance >= escrow_total.amount, 
      "Account cannot cover costs of escrow. Required: ${r} Available: ${a}", ("r",escrow_total)("a",from_account_balance.liquid_balance) );

   _db.adjust_liquid_balance( from_account, -escrow_total );

   _db.create<escrow_object>([&]( escrow_object& esc )
   {
      esc.escrow_id              = o.escrow_id;
      esc.from                   = o.from;
      esc.to                     = o.to;
      esc.agent                  = o.agent;
      esc.ratification_deadline  = o.ratification_deadline;
      esc.escrow_expiration      = o.escrow_expiration;
      esc.balance                = o.amount;
      esc.pending_fee            = o.fee;
   });
} FC_CAPTURE_AND_RETHROW( (o) ) }

void escrow_approve_evaluator::do_apply( const escrow_approve_operation& o )
{ try {
   const auto& escrow = _db.get_escrow( o.from, o.escrow_id );
   time_point now = _db.head_block_time();
   FC_ASSERT( escrow.to == o.to, "Operation 'to' (${o}) does not match escrow 'to' (${e}).", ("o", o.to)("e", escrow.to) );
   FC_ASSERT( escrow.agent == o.agent, "Operation 'agent' (${a}) does not match escrow 'agent' (${e}).", ("o", o.agent)("e", escrow.agent) );
   FC_ASSERT( escrow.ratification_deadline >= now, "The escrow ratification deadline has passed. Escrow can no longer be ratified." );
   const account_name_type& signed_for = o.who;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   bool reject_escrow = !o.approve;

   if( o.who == o.to )
   {
      FC_ASSERT( !escrow.to_approved, "Account 'to' (${t}) has already approved the escrow.", ("t", o.to) );

      if( !reject_escrow )
      {
         _db.modify( escrow, [&]( escrow_object& esc )
         {
            esc.to_approved = true;
         });
      }
   }
   if( o.who == o.agent )
   {
      FC_ASSERT( !escrow.agent_approved, "Account 'agent' (${a}) has already approved the escrow.", ("a", o.agent) );

      if( !reject_escrow )
      {
         _db.modify( escrow, [&]( escrow_object& esc )
         {
            esc.agent_approved = true;
         });
      }
   }

   if( reject_escrow )
   {
      const auto& from_account = _db.get_account( o.from );
      _db.adjust_liquid_balance( from_account, escrow.balance );
      _db.adjust_liquid_balance( from_account, escrow.pending_fee );

      _db.remove( escrow );
   }
   else if( escrow.to_approved && escrow.agent_approved )
   {
      const auto& agent_account = _db.get_account( o.agent );
      _db.adjust_liquid_balance( agent_account, escrow.pending_fee );

      _db.modify( escrow, [&]( escrow_object& esc )
      {
         esc.pending_fee.amount = 0;
      });
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void escrow_dispute_evaluator::do_apply( const escrow_dispute_operation& o )
{ try {
   const account_object& from = _db.get_account( o.from ); // Verify from account exists
   const account_object& who = _db.get_account( o.who ); // Verify who account exists

   const account_name_type& signed_for = o.who;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();

   const auto& e = _db.get_escrow( o.from, o.escrow_id );
   FC_ASSERT( now < e.escrow_expiration, "Disputing the escrow must happen before expiration." );
   FC_ASSERT( e.to_approved && e.agent_approved, "The escrow must be approved by all parties before a dispute can be raised." );
   FC_ASSERT( !e.disputed, "The escrow is already under dispute." );
   FC_ASSERT( e.to == o.to, "Operation 'to' (${o}) does not match escrow 'to' (${e}).", ("o", o.to)("e", e.to) );
   FC_ASSERT( e.agent == o.agent, "Operation 'agent' (${a}) does not match escrow 'agent' (${e}).", ("o", o.agent)("e", e.agent) );

   _db.modify( e, [&]( escrow_object& esc )
   {
      esc.disputed = true;
   });
} FC_CAPTURE_AND_RETHROW( (o) ) }


void escrow_release_evaluator::do_apply( const escrow_release_operation& o )
{ try {
   _db.get_account(o.from); // Verify from account exists
   const account_object& who = _db.get_account( o.who ); // Verify who account exists

   const account_name_type& signed_for = o.who;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const auto& receiver_account = _db.get_account(o.receiver);
   time_point now = _db.head_block_time();

   const auto& e = _db.get_escrow( o.from, o.escrow_id );
   FC_ASSERT( e.to_approved && e.agent_approved, "Funds cannot be released prior to escrow approval by all accounts." );
   FC_ASSERT( e.balance >= o.amount, "Release amount exceeds escrow balance. Amount: ${a}, Balance: ${b}", ("a", o.amount)("b", e.balance) );
   FC_ASSERT( e.to == o.to, "Operation 'to' (${o}) does not match escrow 'to' (${e}).", ("o", o.to)("e", e.to) );
   FC_ASSERT( e.agent == o.agent, "Operation 'agent' (${a}) does not match escrow 'agent' (${e}).", ("o", o.agent)("e", e.agent) );
   FC_ASSERT( o.receiver == e.from || o.receiver == e.to, "Funds must be released to 'from' (${f}) or 'to' (${t})", ("f", e.from)("t", e.to) );
   
   // If there is a dispute regardless of expiration, the agent can release funds to either party
   if( e.disputed )
   {
      FC_ASSERT( o.who == e.agent, "Only 'agent' (${a}) can release funds in a disputed escrow.", ("a", e.agent) );
   }
   else
   {
      FC_ASSERT( o.who == e.from || o.who == e.to, "Only 'from' (${f}) and 'to' (${t}) can release funds from a non-disputed escrow", ("f", e.from)("t", e.to) );

      if( e.escrow_expiration > now )
      {
         // If there is no dispute and escrow has not expired, either party can release funds to the other.
         if( o.who == e.from )
         {
            FC_ASSERT( o.receiver == e.to, "Only 'from' (${f}) can release funds to 'to' (${t}).", ("f", e.from)("t", e.to) );
         }
         else if( o.who == e.to )
         {
            FC_ASSERT( o.receiver == e.from, "Only 'to' (${t}) can release funds to 'from' (${t}).", ("f", e.from)("t", e.to) );
         }
      }
   }
   // If escrow expires and there is no dispute, either party can release funds to either party.

   _db.adjust_liquid_balance( receiver_account, o.amount );

   _db.modify( e, [&]( escrow_object& esc )
   {
      esc.balance -= o.amount;
   });

   if( e.balance.amount == 0 )
   {
      _db.remove( e );
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


//============================//
// === Trading Evaluators === //
//============================//


void limit_order_create_evaluator::do_apply( const limit_order_create_operation& o )
{ try {
   time_point now = _db.head_block_time();
   FC_ASSERT( o.expiration > now, "Limit order has to expire after head block time." );

   const account_object& owner = _db.get_account( o.owner );
   const account_name_type& signed_for = o.owner;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& interface = _db.get_account( o.interface );
   const interface_object& inter = _db.get_interface( o.interface );
   asset liquid = _db.get_liquid_balance( owner.name, o.amount_to_sell.symbol );
   FC_ASSERT( liquid >= o.amount_to_sell, "Account does not have sufficient funds for limit order." );
   _db.adjust_liquid_balance( owner.name, -o.amount_to_sell );

   const limit_order_object& order = _db.create<limit_order_object>( [&]( limit_order_object& obj )
   {
      obj.seller = o.owner;
      obj.order_id = o.order_id;
      obj.for_sale = o.amount_to_sell.amount;
      obj.sell_price = o.exchange_rate;
      obj.expiration = o.expiration;
      obj.interface = o.interface;
      obj.created = now;
   });

   bool filled = _db.apply_order( order );

   if( o.fill_or_kill ) 
   {
      FC_ASSERT( filled, "Cancelling order because it was not filled." );
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void limit_order_cancel_evaluator::do_apply( const limit_order_cancel_operation& o )
{
   const account_name_type& signed_for = o.owner;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   _db.cancel_order( _db.get_limit_order( o.owner, o.order_id ) );
}


void margin_order_create_evaluator::do_apply( const margin_order_create_operation& o )
{ try {
   FC_ASSERT( o.amount_to_borrow.amount > 0 && o.collateral.amount > 0, 
      "Please set a greater than zero amount to borrow and collateral.");
   const dynamic_global_property_object props = _db.get_dynamic_global_properties();
   time_point now = props.time;
   FC_ASSERT( o.expiration > now, 
      "Margin order has to expire after head block time." );

   FC_ASSERT( o.exchange_rate.base == amount_to_borrow.symbol, 
      "Margin order exchange rate should have debt asset as base of price." );
   
   const account_object& owner = _db.get_account( o.owner );

   const account_name_type& signed_for = o.owner;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   FC_ASSERT( owner.loan_default_balance == 0, 
      "Account has an outstanding loan default balance. Please expend network credit collateral to recoup losses before opening a new loan." );

   const account_object& interface = _db.get_account( o.interface );
   const interface_object& inter = _db.get_interface( o.interface );

   const asset_object& debt_asset = _db.get_asset( o.amount_to_borrow.symbol );
   const asset_object& collateral_asset = _db.get_asset( o.collateral.symbol );
   const asset_object& position_asset = _db.get_asset( o.exchange_rate.quote.symbol );

   const credit_collateral_object& collateral = _db.get_collateral( o.account, o.collateral.symbol );

   FC_ASSERT( collateral.collateral.amount >= o.collateral.amount, 
      "Insufficient collateral balance in this asset to vest the amount requested in the loan. Please increase collateral.");

   const asset_credit_pool_object& pool = _db.get_credit_pool( o.amount_to_borrow.symbol, false );
   asset min_collateral = ( o.amount_to_borrow * props.margin_open_ratio ) / PERCENT_100;        // Min margin collateral equal to 20% of debt value.
   share_type collateralization;

   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( collateral_asset.symbol != debt_asset.symbol )
   { 
      if( debt_asset.id < collateral_asset.id )
      {
         symbol_a = debt_asset.symbol;
         symbol_b = collateral_asset.symbol;
      }
      else
      {
         symbol_b = debt_asset.symbol;
         symbol_a = collateral_asset.symbol;
      }

      const asset_liquidity_pool_object& debt_collateral_liq_pool = _db.get_liquidity_pool( symbol_a, symbol_b );    
      price median_price = debt_collateral_liq_pool.hour_median_price;     // Get the asset pair's liquidity pool median price for calculations.
      min_collateral = min_collateral * median_price;
      collateralization = ( o.collateral * median_price ).amount * PERCENT_100 / o.amount_to_borrow.amount;
   }
   else
   {
      collateralization = ( o.collateral.amount * PERCENT_100 ) / o.amount_to_borrow.amount;
   }
   
   asset position = o.amount_to_borrow * o.exchange_rate;

   FC_ASSERT( o.collateral.amount >= min_collateral.amount, 
      "Collateral is insufficient to support a margin loan of this size. Please increase collateral.");
   FC_ASSERT( _db.margin_check( o.amount_to_borrow, position, o.collateral, pool ), 
      "Margin loan with provided collateral, position, and debt is not viable with current asset liquidity conditions. Please lower debt." );
   FC_ASSERT( pool.base_balance.amount >= o.amount_to_borrow.amount, 
      "Insufficient Available assets to borrow from credit pool. Please lower debt." );
   FC_ASSERT( o.stop_loss_price < o.exchange_rate,
      "Stop price must be less than the specified exchange rate, or the order will be immediately stop loss liquidated." );
   FC_ASSERT( o.take_profit_price > o.exchange_rate,
      "Target price must be greater than the specified exchange rate, or the order will be immediately take profit liquidated." );
   FC_ASSERT( o.limit_stop_loss_price < o.exchange_rate,
      "Stop price must be less than the specified exchange rate, or the order will be immediately stop loss liquidated." );
   FC_ASSERT( o.limit_take_profit_price > o.exchange_rate,
      "Target price must be greater than the specified exchange rate, or the order will be immediately take profit liquidated." );
   
   // Decrement from pledged collateral object balance.
   _db.modify( collateral, [&]( credit_collateral_object& cco )   
   {
      cco.collateral -= o.collateral;
   });

   // Transfer borrowed amount from the base balance to the borrowed balance of the credit pool.
   _db.modify( pool, [&]( asset_credit_pool_object& acpo )
   {
      acpo.borrowed_balance += o.amount_to_borrow;
      acpo.base_balance -= o.amount_to_borrow;
   });

   const margin_order_object& order = _db.create<margin_order_object>( [&]( margin_order_object& moo )
   {
      moo.owner = o.owner;
      moo.order_id = o.order_id;
      moo.sell_price = o.exchange_rate;
      moo.debt = o.amount_to_borrow;
      moo.debt_balance = o.amount_to_borrow;
      moo.interest = asset( 0, o.amount_to_borrow.symbol);
      moo.position = position;
      moo.position_filled = asset( 0, position.symbol );
      moo.collateralization = collateralization;
      moo.interface = o.interface;
      moo.expiration = o.expiration;
      moo.stop_loss_price = o.stop_loss_price;
      moo.take_profit_price = o.take_profit_price;
      moo.limit_stop_loss_price = o.limit_stop_loss_price;
      moo.limit_take_profit_price = o.limit_take_profit_price;
      moo.liquidating = false;
      moo.created = now;
      moo.last_updated = now;
      moo.unrealized_value = 0;
   });

   bool filled = _db.apply_order( order );

   if( o.fill_or_kill ) 
   {
      FC_ASSERT( filled, "Cancelling order because it was not filled." );
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void margin_order_close_evaluator::do_apply( const margin_order_close_operation& o )
{
   const margin_order_object& margin = _db.get_margin_order( o.owner, o.order_id );
   const account_object& owner = _db.get_account( o.owner );

   const account_name_type& signed_for = o.owner;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   if( o.force_close )
   {
      _db.close_margin_order( margin );      // Liquidate the remaining order into the liquidity pool at current price and close loan.
   }
   else
   {
      FC_ASSERT( o.exchange_rate.is_valid(), 
         "Must provide exchange rate if not force closing order.");
      price new_sell_price = *o.exchange_rate;
      FC_ASSERT( new_sell_price.base.symbol == margin.sell_price.quote.symbol && 
         new_sell_price.quote.symbol == margin.sell_price.base.symbol,
         "Margin close exchange rate must have inverse price pair to order sell price pair." );

      _db.modify( margin, [&]( margin_order_object& moo )
      {
         moo.liquidating = true;
         moo.sell_price = new_sell_price;      // Invert price and insert into the book as sell position order. 
      });
      bool filled = _db.apply_order( margin );
      if( o.fill_or_kill ) 
      {
         FC_ASSERT( filled, "Cancelling close order because it was not filled." );
      }
   }
}


void call_order_update_evaluator::do_apply(const call_order_update_operation& o)
{ try {
   const account_object& paying_account = _db.get_account(o.funding_account);
   const account_name_type& signed_for = o.funding_account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   const asset_object& debt_asset = _db.get_asset(o.delta_debt.symbol);

   FC_ASSERT( debt_asset.is_market_issued(), 
      "Asset ${sym} is not a collateralized asset.", ("sym", debt_asset.symbol) );

   const asset_dynamic_data_object& debt_dynamic_data = _db.get_dynamic_data(o.delta_debt.symbol);

   FC_ASSERT( debt_dynamic_data.total_supply + o.delta_debt.amount <= debt_asset.options.max_supply,
      "Borrowing this quantity would exceed the assets MAX_SUPPLY" );
   FC_ASSERT( debt_dynamic_data.total_supply + o.delta_debt.amount >= 0, 
      "This transaction would bring current supply below zero.");

   const asset_bitasset_data_object& debt_bitasset_data  = _db.get_bitasset_data(debt_asset.symbol);

   // if there is a settlement for this asset, then no further margin positions may be taken and
   // all existing margin positions should have been closed via database::globally_settle_asset
   FC_ASSERT( !debt_bitasset_data.has_settlement(), "Cannot update debt position when the asset has been globally settled" );

   FC_ASSERT( o.delta_collateral.symbol == debt_bitasset_data.backing_asset, "Collateral asset type should be same as backing asset of debt asset" );

   FC_ASSERT( !debt_bitasset_data.current_feed.settlement_price.is_null(), 
         "Cannot borrow asset with no price feed." );
   
   if( o.delta_debt.amount != 0 )
   {
      _db.adjust_liquid_balance( o.funding_account, o.delta_debt );
   }

   if( o.delta_collateral.amount != 0 )
   {
      _db.adjust_liquid_balance( o.funding_account, -o.delta_collateral );
   }

   auto& call_idx = _db.get_index< call_order_index >().indices().get< by_account >();

   auto itr = call_idx.find( boost::make_tuple(o.funding_account, o.delta_debt.symbol) );

   const call_order_object& call_object = _db.get_call_order(o.funding_account, o.delta_debt.symbol);

   const call_order_object* call_obj_ptr = _db.find_call_order(o.funding_account, o.delta_debt.symbol);

   optional<price> old_collateralization;
   optional<share_type> old_debt;

   if( call_obj_ptr = nullptr ) // creating new debt position
   {
      FC_ASSERT( o.delta_collateral.amount > 0, "Delta collateral amount of new debt position should be positive" );
      FC_ASSERT( o.delta_debt.amount > 0, "Delta debt amount of new debt position should be positive" );

      _db.create<call_order_object>( [&]( call_order_object& coo )
      {
         coo.borrower = o.funding_account;
         coo.collateral = o.delta_collateral;
         coo.debt = o.delta_debt;
         coo.call_price = price( asset( 1, o.delta_collateral.symbol ), asset( 1, o.delta_debt.symbol ) );
         coo.target_collateral_ratio = *o.target_collateral_ratio;
      });
   }
   else // updating existing debt position
   {
      auto new_collateral = call_object.collateral + o.delta_collateral;
      auto new_debt = call_object.debt + o.delta_debt;
   
      if( new_debt.amount == 0 )
      {
         FC_ASSERT( new_collateral.amount == 0, "Should claim all collateral when closing debt position" );
         _db.remove( call_object );
      }

      FC_ASSERT( new_collateral.amount > 0 && new_debt.amount > 0, "Both collateral and debt should be positive after updated a debt position if not to close it" );

      price old_collateralization = call_object.collateralization();
      const asset& old_debt = call_object.debt;

      _db.modify( call_object, [&]( call_order_object& coo )
      {
         coo.collateral = new_collateral;
         coo.debt       = new_debt;
         coo.target_collateral_ratio = *o.target_collateral_ratio;
      });
   }

   if( _db.check_call_orders( debt_asset, false, false, &debt_bitasset_data ) ) // Don't allow black swan, not for new limit order
   {
      call_obj_ptr = _db.find_call_order(o.funding_account, o.delta_debt.symbol);
      FC_ASSERT(!call_obj_ptr, "Updating call order would trigger a margin call that cannot be fully filled" );
   }
   else
   {
      call_obj_ptr = _db.find_call_order(o.funding_account, o.delta_debt.symbol);  // No black swan event has occurred
      FC_ASSERT( call_obj_ptr, "No margin call was executed and yet the call object was deleted" );
      
      // We didn't fill any call orders. This may be because we
      // aren't in margin call territory, or it may be because there
      // were no matching orders. In the latter case,
      // if collateral ratio is not increased or debt is increased, we throw.
      // be here, we know no margin call was executed,
      // so call_obj's collateral ratio should be set only by op
      FC_ASSERT( ( call_object.collateralization() > debt_bitasset_data.current_maintenance_collateralization )
                  || ( old_collateralization.valid() && call_object.debt.amount <= *old_debt && call_object.collateralization() > *old_collateralization ),
         "Can only increase collateral ratio without increasing debt if would trigger a margin call that cannot be fully filled",
         ("old_debt", *old_debt)
         ("new_debt", call_object.debt)
         ("old_collateralization", *old_collateralization)
         ("new_collateralization", call_object.collateralization() )
         );
   }
   
} FC_CAPTURE_AND_RETHROW( (o) ) }


void bid_collateral_evaluator::do_apply(const bid_collateral_operation& o)
{ try {
   const account_object& paying_account = _db.get_account(o.bidder);
   const account_name_type& signed_for = o.bidder;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& debt_asset = _db.get_asset(o.debt_covered.symbol);
   FC_ASSERT( debt_asset.is_market_issued(), "Unable to cover ${sym} as it is not a collateralized asset.", ("sym", debt_asset.symbol) );
   const asset_bitasset_data_object& bitasset_data = _db.get_bitasset_data(debt_asset.symbol);
   FC_ASSERT( bitasset_data.has_settlement() );
   FC_ASSERT( o.additional_collateral.symbol == bitasset_data.backing_asset );

   if( o.additional_collateral.amount > 0 )
   {
      FC_ASSERT( _db.get_liquid_balance(paying_account, bitasset_data.backing_asset) >= o.additional_collateral,
                 "Cannot bid ${c} collateral when payer only has ${b}", ("c", o.additional_collateral.amount)
                 ("b", _db.get_liquid_balance(paying_account, bitasset_data.backing_asset) ));
   }

   const auto& bids = _db.get_index< collateral_bid_index >();
   const auto& index = bids.indices().get<by_account>();
   const auto& bid = index.find( boost::make_tuple( o.debt_covered.symbol, o.bidder ) );
   const collateral_bid_object* bid_ptr;

   if( bid != index.end() )
   {
      bid_ptr = &(*bid);
   }
      
   else 
   {
      FC_ASSERT( o.debt_covered.amount > 0, "Can't find bid to cancel?!");
   }
       
   if( bid_ptr )
      _db.cancel_bid( *bid_ptr, false );

   if( o.debt_covered.amount == 0 )

   _db.adjust_liquid_balance( o.bidder, -o.additional_collateral );

   _db.create<collateral_bid_object>([&]( collateral_bid_object& bid ) 
   {
      bid.bidder = o.bidder;
      bid.inv_swan_price = o.additional_collateral / o.debt_covered;
   });

} FC_CAPTURE_AND_RETHROW( (o) ) }


//=========================//
// === Pool Evaluators === //
//=========================//


void liquidity_pool_create_evaluator::do_apply( const liquidity_pool_create_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const asset_object& first_asset = _db.get_asset( o.first_amount.symbol );
   const asset_object& second_asset = _db.get_asset( o.second_amount.symbol );

   FC_ASSERT( first_asset.asset_type != LIQUIDITY_POOL_ASSET && second_asset.asset_type != LIQUIDITY_POOL_ASSET , 
      "Cannot make a liquidity pool asset with a liquidity pool asset as a component." );

   asset amount_a;
   asset amount_b;

   if( first_asset.id < second_asset.id )
   {
      amount_a = o.first_asset;
      amount_b = o.second_amount;
   }
   else
   {
      amount_b = o.first_asset;
      amount_a = o.second_amount;
   }

   asset_symbol_type liquidity_asset_symbol = LIQUITY_ASSET_PREFIX+amount_a.symbol+"."+amount_b.symbol;

   share_type max = std::max( amount_a.amount, amount_b.amount );
   const auto& liq_idx = _db.get_index< asset_liquidity_pool_index >().indices().get< by_asset_pair >();
   auto liq_itr = liq_idx.find( boost::make_tuple( amount_a.symbol, amount_b.symbol ) ); 

   FC_ASSERT( liq_itr == liq_idx.end(), 
      "Asset liquidity pair already exists. Use an exchange or fund operation.");
   
   _db.create<asset_object>( [&]( asset_object& a ) 
   {
      a.issuer = o.account;
      a.symbol = liquidity_asset_symbol;
      a.asset_type = LIQUIDITY_POOL_ASSET;
   });

   _db.create<asset_dynamic_data_object>( [&]( asset_dynamic_data_object& a ) 
   {
      a.symbol = liquidity_asset_symbol;                 
      a.total_supply = 0;            
      a.liquid_supply = 0; 
      a.staked_supply = 0; 
      a.reward_supply = 0; 
      a.savings_supply = 0;   
      a.delegated_supply = 0;
      a.receiving_supply = 0; 
      a.pending_supply = 0; 
      a.confidential_supply = 0;  
      a.accumulated_fees = 0;
      a.fee_pool = 0;
   });
      
   _db.create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& alpo )
   {   
      alpo.issuer = o.account;
      alpo.balance_a = amount_a;
      alpo.balance_b = amount_b;
      alpo.symbol_a = amount_a.symbol;
      alpo.symbol_b = amount_b.symbol;

      alpo.hour_median_price = price(alpo.balance_a, alpo.balance_b);
      alpo.day_median_price = price(alpo.balance_a, alpo.balance_b);
      alpo.price_history.push_back( price(alpo.balance_a, alpo.balance_b) );
      alpo.symbol_liquid = liquidity_asset_symbol;
      alpo.balance_liquid = max;
   });

   _db.adjust_liquid_balance( o.account, asset( max, liquidity_asset_symbol ));
} FC_CAPTURE_AND_RETHROW( (o) ) }


void liquidity_pool_exchange_evaluator::do_apply( const liquidity_pool_exchange_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const asset_object& first_asset = _db.get_asset( o.amount.symbol );
   const asset_object& second_asset = _db.get_asset( o.receive_asset );
   const account_object int_account = _db.get_account( o.interface );
   const interface_object interface = _db.get_interface( o.interface );

   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( first_asset.id < second_asset.id )
   {
      symbol_a = o.first_asset.symbol;
      symbol_b = o.second_asset.symbol;
   }
   else
   {
      symbol_b = o.first_asset.symbol;
      symbol_a = o.second_asset.symbol;
   }

   const asset_liquidity_pool_object& liquidity_pool = _db.get_liquidity_pool( symbol_a, symbol_b );

   if( o.acquire )
   {
      _db.liquid_acquire( o.amount, account, liquidity_pool, int_account );
   }
   else if( o.limit_price.is_valid() )
   {
      FC_ASSERT( *o.limit_price < liquidity_pool.base_price( *o.limit_price.base.symbol ), 
         "Limit price must be lower than current liquidity pool exchange price.");
      _db.liquid_limit_exchange( o.amount, *o.limit_price, account, liquidity_pool, int_account );
   }
   else
   {
      _db.liquid_exchange( o.amount, account, liquidity_pool, int_account );
   }
 
} FC_CAPTURE_AND_RETHROW( (o) ) }


void liquidity_pool_fund_evaluator::do_apply( const liquidity_pool_fund_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const asset_object& first_asset = _db.get_asset( o.amount.symbol );
   const asset_object& second_asset = _db.get_asset( o.pair_asset );

   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( first_asset.id < second_asset.id )
   {
      symbol_a = o.first_asset.symbol;
      symbol_b = o.second_asset.symbol;
   }
   else
   {
      symbol_b = o.first_asset.symbol;
      symbol_a = o.second_asset.symbol;
   }

   const asset_liquidity_pool_object& liquidity_pool = _db.get_liquidity_pool( symbol_a, symbol_b );

   _db.liquid_fund( o.amount, account, liquidity_pool );

} FC_CAPTURE_AND_RETHROW( (o) ) }


void liquidity_pool_withdraw_evaluator::do_apply( const liquidity_pool_withdraw_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const asset_object& first_asset = _db.get_asset( o.amount.symbol );
   const asset_object& second_asset = _db.get_asset( o.receive_asset );

   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( first_asset.id < second_asset.id )
   {
      symbol_a = o.first_asset.symbol;
      symbol_b = o.second_asset.symbol;
   }
   else
   {
      symbol_b = o.first_asset.symbol;
      symbol_a = o.second_asset.symbol;
   }

   const asset_liquidity_pool_object& liquidity_pool = _db.get_liquidity_pool( symbol_a, symbol_b );

   _db.liquid_withdraw( o.amount, o.receive_asset, account, liquidity_pool );

} FC_CAPTURE_AND_RETHROW( (o) ) }


void credit_pool_collateral_evaluator::do_apply( const credit_pool_collateral_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const asset_object& collateral_asset = _db.get_asset( o.amount.symbol );
   const asset& liquid = _db.get_liquid_balance( o.account.name, o.amount.symbol );
   asset loan_default_balance = account.loan_default_balance;
   const auto& col_idx = _db.get_index< credit_collateral_index >().indices().get< by_owner_symbol >();
   auto col_itr = col_idx.find( boost::make_tuple( account.name, amount.symbol ) ); 
   asset default_paid = asset(0, o.amount.symbol);

   if( col_itr == col_idx.end() )    // Collateral object does not exist.
   {
      asset delta = o.amount;
      if (loan_default_balance.amount > 0 && o.amount.symbol == SYMBOL_CREDIT )
      {
         default_paid = asset( std::min( delta.amount , loan_default_balance.amount ), delta.symbol );
         delta -= default_paid; 

         _db.modify( account, [&]( account_object& a )
         {
            a.loan_default_balance -= default_paid;
         });
      }
      FC_ASSERT( liquid.amount >= o.amount.amount, 
         "Insufficient liquid balance to collateralize the amount requested.");
      _db.adjust_liquid_balance( o.account, -o.amount);
      _db.adjust_pending_supply( o.amount );

      _db.create<credit_collateral_object>( [&]( credit_collateral_object& cco ) 
      {
         cco.owner = account.name;
         cco.symbol = collateral_asset.symbol;
         cco.collateral = delta;
      });
   }
   else    // Collateral position exists and is being updated. 
   {
      const credit_collateral_object& collateral = *col_itr;
      asset old_collateral = collateral.collateral;
      asset delta = o.amount - old_collateral; 

      if (loan_default_balance.amount > 0 && delta.amount > 0 && o.amount.symbol == SYMBOL_CREDIT )
      {
         default_paid = asset( std::min( delta.amount , loan_default_balance.amount ), delta.symbol ); 

         _db.modify( account, [&]( account_object& a )
         {
            a.loan_default_balance -= default_paid;
         });
      }

      FC_ASSERT( delta.amount != 0 , 
         "Operation would not change collateral position in this asset.");
      FC_ASSERT( liquid.amount >= delta.amount , 
         "Insufficient liquid balance to increase collateral position to the amount requested.");

      _db.adjust_liquid_balance( o.account, -delta );
      _db.adjust_pending_supply( delta );

      _db.modify( collateral, [&]( credit_collateral_object& cco )
      {
         cco.collateral = o.amount - default_paid;
      });

      if( collateral.collateral.amount == 0 )
      {
         _db.remove( collateral );
      }
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void credit_pool_borrow_evaluator::do_apply( const credit_pool_borrow_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const asset_object& debt_asset = _db.get_asset( o.amount.symbol );
   const asset_object& collateral_asset = _db.get_asset( o.collateral.symbol );
   const credit_collateral_object& collateral = _db.get_collateral( o.account, o.collateral.symbol );
   const asset_credit_pool_object& pool = _db.get_credit_pool( o.amount.symbol, false );
   const asset& liquid = _db.get_liquid_balance( o.account.name, o.amount.symbol );
   const dynamic_global_property_object props = _db.get_dynamic_global_properties();
   asset min_collateral = ( o.amount * props.credit_open_ratio ) / PERCENT_100;        // Min collateral equal to 125% of debt value
   asset max_debt = ( o.collateral * PERCENT_100 ) / props.credit_liquidation_ratio;   // Max debt before liquidation equal to aprox 90% of collateral value. 
   time_point now = _db.head_block_time();
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( collateral_asset.symbol != debt_asset.symbol )
   { 
      if( debt_asset.id < collateral_asset.id )
      {
         symbol_a = debt_asset.symbol;
         symbol_b = collateral_asset.symbol;
      }
      else
      {
         symbol_b = debt_asset.symbol;
         symbol_a = collateral_asset.symbol;
      }

      const asset_liquidity_pool_object& liquidity_pool = _db.get_liquidity_pool( symbol_a, symbol_b );    
      price median_price = liquidity_pool.hour_median_price;     // Get the asset pair's liquidity pool median price for calculations.
      min_collateral = min_collateral * median_price;
      max_debt = max_debt * median_price;
   }

   FC_ASSERT( o.collateral.amount >= min_collateral.amount , 
      "Collateral is insufficient to support a loan of this size.");

   const auto& loan_idx = _db.get_index< credit_loan_index >().indices().get< by_owner_id >();
   auto loan_itr = loan_idx.find( boost::make_tuple( account.name, o.loan_id ) ); 

   if( loan_itr == loan_idx.end() )    // Credit loan object with this ID does not exist.
   {
      FC_ASSERT( account.loan_default_balance == 0, 
         "Account has an outstanding loan default balance. Please expend network credit collateral to recoup losses before opening a new loan." );
      FC_ASSERT( _db.credit_check( o.amount, o.collateral, pool), 
         "New loan with provided collateral and debt is not viable with current asset liquidity conditions. Please lower debt." );
      FC_ASSERT( o.amount.amount != 0 && o.collateral.amount != 0, 
         "Loan does not exist to close out. Please set non-zero amount and collateral.");
      FC_ASSERT( collateral.collateral.amount >= o.collateral.amount, 
         "Insufficient collateral balance in this asset to vest the amount requested in the loan. Please increase collateral.");
      FC_ASSERT( pool.base_balance.amount >= o.amount.amount, 
         "Insufficient Available asset to borrow from credit pool. Please lower debt." );

      // Create new loan object for the account
      _db.create<credit_loan_object>( [&]( credit_loan_object& clo )    
      {
         clo.owner = account.name;
         clo.debt = o.amount;
         clo.interest = asset( 0, o.amount.symbol );
         clo.collateral = o.collateral;

         if( o.amount.symbol != o.collateral.symbol )
         {
            clo.loan_price = price( o.collateral, o.amount);
            clo.liquidation_price = price( o.collateral, max_debt );
         }
      
         clo.created = now;
         clo.last_updated = now;
      });

      // Decrement from pledged collateral object balance.
      _db.modify( collateral, [&]( credit_collateral_object& cco )
      {
         cco.collateral -= o.collateral;
      });

      // Transfer borrowed amount from the base balance to the borrowed balance of the credit pool. 
      _db.modify( pool, [&]( asset_credit_pool_object& acpo )
      {
         acpo.borrowed_balance += o.amount;
         acpo.base_balance -= o.amount;
      });

      _db.adjust_pending_supply( -o.amount );
      _db.adjust_liquid_balance( o.account, o.amount);     // Withdraw loaned balance from pending supply within credit pool base balance. 
   }
   else    // Loan object exists and is being updated or closed out. 
   {
      const credit_loan_object& loan = *loan_itr;
      asset old_collateral = loan.collateral;
      asset delta_collateral = o.collateral - old_collateral; 
      asset old_debt = loan.debt;
      asset delta_debt = o.amount - old_debt; 

      FC_ASSERT( delta_collateral.amount != 0 || delta_debt.amount != 0, 
         "Operation would not change collateral or debt position in the loan.");
      FC_ASSERT( collateral.collateral.amount >= delta_collateral.amount , 
         "Insufficient collateral balance in this asset to vest the amount requested in the loan.");
      FC_ASSERT( liquid.amount >= -delta_debt.amount , 
         "Insufficient liquid balance in this asset to repay the amount requested.");

      share_type interest_rate = pool.interest_rate(props.credit_min_interest, props.credit_variable_interest);   // Calulate pool's interest rate

      share_type interest_accrued = ( loan.debt.amount * interest_rate * ( now - loan.last_updated ).to_seconds() ) / ( PERCENT_100 * fc::days(365).to_seconds() ) ;
      asset interest_asset = asset( interest_accrued, loan.debt.symbol );   // Accrue interest on debt balance

      if( o.amount.amount == 0 || o.collateral.amount == 0 )   // Closing out the loan, ensure both amount and collateral are zero if one is zero. 
      {
         FC_ASSERT( o.amount.amount == 0 && o.collateral.amount == 0, 
            "Both collateral and amount must be set to zero to close out loan.");
         asset closing_debt = old_debt + interest_asset;

         _db.adjust_liquid_balance( o.account, -closing_debt );    // Return debt to the pending supply of the credit pool 
         _db.adjust_pending_supply( closing_debt );

         // Return collateral to the account's collateral balance. 
         _db.modify( collateral, [&]( credit_collateral_object& cco )
         {
            cco.collateral += old_collateral;
         });

         // Transfer the borrowed balance back to the base balance, with interest. 
         _db.modify( pool, [&]( asset_credit_pool_object& acpo )
         {
            acpo.borrowed_balance -= old_debt;
            acpo.base_balance += closing_debt;
         });

         _db.remove( loan );
      }
      else      // modifying the loan or repaying partially. 
      {
         FC_ASSERT( delta_debt < 0 || account.loan_default_balance == 0, 
            "Account has an outstanding loan default balance. Please expend network credits to recoup losses before increasing debt." );
         FC_ASSERT( _db.credit_check( o.amount, o.collateral, pool), 
            "New loan with provided collateral and debt is not viable with current asset liquidity conditions. Please lower debt." );

         asset new_debt = o.amount + interest_asset;

         // Update to new collateral amount
         _db.modify( collateral, [&]( credit_collateral_object& cco )
         {
            cco.collateral -= delta_collateral;
         });

         // Update borrowed balance
         _db.modify( pool, [&]( asset_credit_pool_object& acpo )
         {
            acpo.borrowed_balance += delta_debt;
            acpo.base_balance -= delta_debt; 
         });

         // Update to new loan parameters
         _db.modify( loan, [&]( credit_loan_object& clo )
         {
            clo.debt = new_debt;
            clo.collateral = o.collateral;
            if( loan.debt.symbol != loan.collateral.symbol )
            {
               clo.loan_price = price( o.collateral, o.amount);
               clo.liquidation_price = price( o.collateral, max_debt );
            }
            clo.last_updated = now;
         });

         _db.adjust_liquid_balance( o.account, delta_debt );    // Shift newly borrowed or repaid amount with pending supply. 
         _db.adjust_pending_supply( -delta_debt );
      }
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void credit_pool_lend_evaluator::do_apply( const credit_pool_lend_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& asset = _db.get_asset( o.amount.symbol );
   const asset_credit_pool_object& credit_pool = _db.get_credit_pool( asset.symbol, false );

   _db.credit_lend( o.amount, account, credit_pool );

} FC_CAPTURE_AND_RETHROW( (o) ) }


void credit_pool_withdraw_evaluator::do_apply( const credit_pool_withdraw_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& asset = _db.get_asset( o.amount.symbol );
   const asset_credit_pool_object& credit_pool = _db.get_credit_pool( asset.symbol, true );

   _db.credit_withdraw( o.amount, account, credit_pool );
   
} FC_CAPTURE_AND_RETHROW( (o) ) }


//==========================//
// === Asset Evaluators === //
//==========================//


// The following Asset Evaluators are derived primarily from the Bitshares core codebase with much appreciation.
// https://bitshares.org
// https://github.com/bitshares/bitshares-core

/**
 * Creates a new asset object, in addition to its dynamic data object, 
 * and bitasset object if it is a bitasset.
 * Creates the liquitity pool between the new asset and the core asset.
 * Creates the credit pool for the new asset.
 */
/**
 * ASSET TYPES
 * 
   CURRENCY_ASSET,         // Cryptocurrency that is issued by the network, starts from zero supply, issuing account is the null account, cannot be issued by any accounts. 
   STANDARD_ASSET,         // Regular asset, can be transferred and staked, saved, and delegated.
   EQUITY_ASSET,           // Asset issued by a business account that distributes a dividend from incoming revenue, and has voting power over a business accounts transactions. 
   CREDIT_ASSET,           // Asset issued by a business account that is backed by repayments up to a face value, and interest payments.
   BITASSET_ASSET,         // Asset based by collateral and track the value of an external asset.
   LIQUIDITY_POOL_ASSET,   // Liquidity pool asset that is backed by the deposits of an asset pair's liquidity pool and earns trading fees. 
   CREDIT_POOL_ASSET,      // Credit pool asset that is backed by deposits of the base asset, used for borrowing funds from the pool, used as collateral to borrow base asset.
   OPTION_ASSET,           // Asset that enables the execution of a trade at a specific price until an expiration date. 
   PREDICTION_ASSET,       // Asset backed by an underlying collateral claim, on the condition that a prediction market resolve in a particular outcome.
   GATEWAY_ASSET,          // Asset backed by deposits with an exchange counterparty of another asset or currency. 
   UNIQUE_ASSET,           // Asset with a supply of one, contains metadata relating to the ownership of a unique non-fungible asset. 
*/
void asset_create_evaluator::do_apply( const asset_create_operation& o )
{ try {

   FC_ASSERT( o.asset_type != LIQUIDITY_POOL_ASSET, 
      "Cannot directly create a new liquidity pool asset. Please create a liquidity pool between two existing assets.");
   FC_ASSERT( o.asset_type != CREDIT_POOL_ASSET, 
      "Cannot directly create a new credit pool asset. Credit pool assets are created in addition to every asset.");
   FC_ASSERT( o.asset_type != PREDICTION_ASSET, 
      "Cannot directly create a new prediction asset. Prediction assets are issued from a Prediction market.");
   FC_ASSERT( o.asset_type != OPTION_ASSET,
      "Cannot directly create a new option asset. Option assets are issued from an Options market.");
   auto& asset_indx =_db.get_index< asset_index >().indices().get< by_symbol >();
   auto asset_symbol_itr = asset_indx.find( o.symbol );
   FC_ASSERT( asset_symbol_itr == asset_indx.end(), 
      "Asset with this symbol already exists, please choose a new symbol." );
   
   const auto& gprops =_db.get_dynamic_global_properties();
   time_point now = gprops.time;
   const account_object& account = _db.get_account( o.issuer );
   const account_name_type& signed_for = o.issuer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   account_name_type issuer_account_name = account.name;

   FC_ASSERT( o.coin_liquidity.symbol == SYMBOL_COIN, 
      "Asset must have initial liquidity in the COIN asset." );
   FC_ASSERT( o.coin_liquidity.amount >= 10 * BLOCKCHAIN_PRECISION, 
      "Asset must have at least 10 COIN asset of initial liquidity." );
   FC_ASSERT( o.usd_liquidity.symbol == SYMBOL_USD, 
      "Asset must have initial liquidity in the USD asset." );
   FC_ASSERT( o.usd_liquidity.amount >= 10 * BLOCKCHAIN_PRECISION, 
      "Asset must have at least 10 USD asset of initial liquidity." );

   FC_ASSERT( o.common_options.whitelist_authorities.size() <= gprops.maximum_asset_whitelist_authorities );
   FC_ASSERT( o.common_options.blacklist_authorities.size() <= gprops.maximum_asset_whitelist_authorities );

   FC_ASSERT( o.common_options.stake_intervals <= gprops.max_stake_intervals && o.common_options.stake_intervals >= 0, 
      "Asset stake intervals outside of acceptable limits." );
   FC_ASSERT( o.common_options.unstake_asset <= gprops.max_unstake_intervals && o.common_options.unstake_intervals >= 0, 
      "Asset unstake intervals outside of acceptable limits." );

   // Check that all authorities do exist
   for( auto account : o.common_options.whitelist_authorities )
     _db.get_account(account);
   for( auto account : o.common_options.blacklist_authorities )
     _db.get_account(account);

   string asset_string = o.symbol;

   auto dotpos = asset_string.find( '.' );
   if( dotpos != std::string::npos )
   {
      auto prefix = asset_string.substr( 0, dotpos );
      auto asset_symbol_itr = asset_indx.find( prefix );

      FC_ASSERT( asset_symbol_itr != asset_indx.end(), "Asset ${s} may only be created by issuer of ${p}, but ${p} has not been registered",
                  ("s",o.symbol)("p",prefix) );

      FC_ASSERT( asset_symbol_itr->issuer == o.issuer, "Asset ${s} may only be created by issuer of ${p}, ${i}",
                  ("s",o.symbol)("p",prefix)("i", o.issuer) );
   }

   // Asset specific requirements

   switch( o.asset_type )
   {
      case STANDARD_ASSET:
      {
         // No specific checks
      }
      break;
      case CURRENCY_ASSET:
      {
         issuer_account_name = NULL_ACCOUNT;
         FC_ASSERT( o.currency_opts.valid(), 
            "Currency Asset requires valid currency options." );
      }
      break;
      case EQUITY_ASSET:
      {
         FC_ASSERT( account.account_type == BUSINESS, 
            "Equity Asset can only be issued by a business account." );
         FC_ASSERT( o.equity_opts.valid(), 
            "Equity Asset requires valid equity options." );
         const asset_object& dividend_asset = _db.get_asset(o.equity_opts->dividend_asset);
         uint16_t revenue_share_sum = o.equity_opts->dividend_share_percent;
         for( auto share : account.equity_revenue_shares )
         {
            revenue_share_sum += share.second;
         }
         for( auto share : account.credit_revenue_shares )
         {
            revenue_share_sum += share.second;
         }
         FC_ASSERT( revenue_share_sum <= 50 * PERCENT_1, 
            "Cannot share more than 50 percent of account revenue." );
      }
      break;
      case CREDIT_ASSET:
      {
         FC_ASSERT( account.account_type == BUSINESS, 
            "Credit Asset can only be issued by a business account." );
         FC_ASSERT( o.credit_opts.valid(), 
            "Credit Asset requires valid credit options." );
         const asset_object& buyback_asset = _db.get_asset(o.credit_opts->buyback_asset); 
         uint16_t revenue_share_sum = o.credit_opts->buyback_share_percent;

         for( auto share : account.equity_revenue_shares )
         {
            revenue_share_sum += share.second;
         }
         for( auto share : account.credit_revenue_shares )
         {
            revenue_share_sum += share.second;
         }
         FC_ASSERT( revenue_share_sum <= 50 * PERCENT_1, 
            "Cannot share more than 50 percent of account revenue." ); 
      }
      break;
      case BITASSET_ASSET:
      {
         FC_ASSERT( o.bitasset_opts.valid(), 
            "Bitasset must contain valid bitasset options." );
         const asset_object& backing_asset = _db.get_asset(o.bitasset_opts->short_backing_asset);
         if( backing_asset.is_market_issued() )
         {
            const asset_bitasset_data_object& backing_bitasset_data = _db.get_bitasset_data(backing_asset.symbol);
            const asset_object& backing_backing_asset = _db.get_asset(backing_bitasset_data.backing_asset);

            FC_ASSERT( !backing_backing_asset.is_market_issued(), 
               "May not create a bitasset backed by a bitasset backed by a bitasset." );
            FC_ASSERT( backing_backing_asset.symbol == SYMBOL_COIN, 
               "Backing asset should be the core asset.");
         } 
         else 
         {
            FC_ASSERT( backing_asset.symbol == SYMBOL_COIN, "Backing asset should be the core asset.");
         }
            
         FC_ASSERT( fc::microseconds(o.bitasset_opts->feed_lifetime) > MIN_FEED_LIFETIME, 
            "Feed lifetime must be greater than network minimum.");
         FC_ASSERT( fc::microseconds(o.bitasset_opts->force_settlement_delay) > MIN_SETTLEMENT_DELAY, 
            "Force Settlement delay must be greater than network minimum." );
      }
      break;
      case GATEWAY_ASSET:
      {
         FC_ASSERT( account.account_type == BUSINESS, 
            "Gateway Asset can only be issued by a business account." );
         FC_ASSERT( o.gateway_opts.valid(), 
            "Gateway Asset requires valid gateway options." );
      }
      break;
      case UNIQUE_ASSET:
      {
         FC_ASSERT( o.common_options.max_supply == BLOCKCHAIN_PRECISION, 
            "Unique assets must have a maximum supply of exactly one unit." );
      }
      break; 
   }

   // Create the asset object 
   _db.create<asset_object>( [&]( asset_object& a ) 
   {
      a.issuer = account.name;
      a.symbol = o.symbol;
      a.asset_type = o.asset_type;
      a.options = o.common_options;
      a.created = now;
   });

   _db.create<asset_dynamic_data_object>( [&]( asset_dynamic_data_object& a ) 
   {
      a.issuer = account.name;
      a.symbol = o.symbol;                
   });

   if( o.bitasset_opts.valid() ) 
   {
      _db.create<asset_bitasset_data_object>( [&]( asset_bitasset_data_object& a ) 
      {
         a.symbol = o.symbol;
         a.options = o.bitasset_opts;
         a.backing_asset = o.bitasset_opts->short_backing_asset;
      });
   }

   if( o.equity_opts.valid() )
   {
      _db.create<asset_equity_data_object>( [&]( asset_equity_data_object& a ) 
      {
         a.symbol = o.symbol;
         a.business_account = o.issuer;
         a.options = *o.equity_opts;
         a.dividend_asset = o.equity_opts->dividend_asset;
         a.dividend_pool = asset( 0, a.dividend_asset );
         a.last_updated = now;
         a.next_dividend = now + o.equity_opts->dividend_interval;
      });
      _db.modify( account, [&o](account_object& a) 
      {
         a.revenue_share = true;
         a.equity_revenue_shares[ o.symbol ] = o.equity_opts->dividend_share_percent;
      });
   }

   if( o.credit_opts.valid() )
   {
      _db.create<asset_credit_data_object>( [&]( asset_credit_data_object& a ) 
      {
         a.symbol = o.symbol;
         a.business_account = o.issuer;
         a.options = *o.credit_opts;
         a.buyback_asset = o.credit_opts->buyback_asset;
         a.buyback_pool = asset( 0, a.buyback_asset );
         a.last_updated = now;
         a.next_buyback = now + o.credit_opts->buyback_interval;
      });
      _db.modify( account, [&o](account_object& a) 
      {
         a.revenue_share = true;
         a.credit_revenue_shares[ o.symbol ] = o.equity_opts->dividend_share_percent;
      });
   }

   // Create the core liquidity pool for the new asset.
   
   const asset_object& core_asset = _db.get_asset( SYMBOL_COIN );
   const asset_object& new_asset = _db.get_asset( o.symbol );
   asset_symbol_type core_liq_symbol = LIQUITY_ASSET_PREFIX+core_asset.symbol+"."+new_asset.symbol;
   
   _db.create<asset_object>( [&]( asset_object& a ) 
   {
      a.issuer = account.name;
      a.symbol = core_liq_symbol;
      a.asset_type = LIQUIDITY_POOL_ASSET;
   });

   _db.create<asset_dynamic_data_object>( [&]( asset_dynamic_data_object& a ) 
   {
      a.issuer = account.name;
      a.symbol = core_liq_symbol;                
   });

   asset init_new_asset = asset(o.coin_liquidity.amount, new_asset.symbol );              // Creates initial new asset supply equivalent to core liquidity. 
   asset init_liquid_asset = asset(o.coin_liquidity.amount, core_liq_symbol);             // Creates equivalent supply of the liquidity pool asset for liquidity injection.
      
   _db.create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
   {   
      a.issuer = account.name;
      a.symbol_a = core_asset.symbol;
      a.symbol_b = new_asset.symbol;
      a.balance_a = o.coin_liquidity;
      a.balance_b = init_new_asset;
      a.hour_median_price = price(a.balance_a, a.balance_b);
      a.day_median_price = price(a.balance_a, a.balance_b);
      a.price_history.push_back( price(a.balance_a, a.balance_b) );
      a.balance_liquid = init_liquid_asset;
   });

   _db.adjust_pending_supply( init_new_asset );
   _db.adjust_liquid_balance( o.issuer, init_liquid_asset );


   // Create the USD liquidity pool for the new asset.
   
   const asset_object& usd_asset = _db.get_asset( SYMBOL_USD );
   const asset_object& new_asset = _db.get_asset( o.symbol );
   asset_symbol_type usd_liq_symbol = LIQUITY_ASSET_PREFIX+usd_asset.symbol+"."+new_asset.symbol;
   
   _db.create<asset_object>( [&]( asset_object& a ) 
   {
      a.issuer = account.name;
      a.symbol = usd_liq_symbol;
      a.asset_type = LIQUIDITY_POOL_ASSET;
   });

   _db.create<asset_dynamic_data_object>( [&]( asset_dynamic_data_object& a ) 
   {
      a.issuer = account.name;
      a.symbol = usd_liq_symbol;                
   });

   asset init_new_asset = asset(o.usd_liquidity.amount, new_asset.symbol );      // Creates initial new asset supply equivalent to core liquidity. 
   asset init_liquid_asset = asset(o.usd_liquidity.amount, usd_liq_symbol);      // Creates equivalent supply of the liquidity pool asset for liquidity injection.
      
   _db.create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
   {   
      a.issuer = account.name;
      a.symbol_a = usd_asset.symbol;
      a.symbol_b = new_asset.symbol;
      a.balance_a = o.usd_liquidity;
      a.balance_b = init_new_asset;
      a.hour_median_price = price(a.balance_a, a.balance_b);
      a.day_median_price = price(a.balance_a, a.balance_b);
      a.price_history.push_back( price(a.balance_a, a.balance_b) );
      a.balance_liquid = init_liquid_asset;
   });

   _db.adjust_pending_supply( init_new_asset );
   _db.adjust_liquid_balance( o.issuer, init_liquid_asset );
   
   // Create the asset credit pool for the new asset. 

   asset_symbol_type credit_asset_symbol = CREDIT_ASSET_PREFIX+new_asset.symbol;
   
   _db.create<asset_object>( [&]( asset_object& a ) 
   {
      a.issuer = account.name;
      a.symbol = credit_asset_symbol;
      a.asset_type = CREDIT_POOL_ASSET;
   });

   _db.create<asset_dynamic_data_object>( [&]( asset_dynamic_data_object& a ) 
   {
      a.issuer = account.name;
      a.symbol = credit_asset_symbol;                 
   });

   asset init_lent_asset = asset(o.credit_liquidity.amount, new_asset.symbol );                // Creates and lends equivalent new assets to the credit pool.
   asset init_credit_asset = asset(o.credit_liquidity.amount * 100, credit_asset_symbol);      // Creates equivalent credit pool assets and passes to issuer. 
   price init_credit_price = price( init_lent_asset, init_credit_asset );                      // Starts the initial credit asset exchange rate at 100:1.

   _db.create<asset_credit_pool_object>( [&]( asset_credit_pool_object& a ) 
   {
      a.issuer = account.name;
      a.base_symbol = o.symbol;   
      a.credit_symbol = credit_asset_symbol; 
      a.base_balance = init_lent_asset;
      a.borrowed_balance = asset( 0, new_asset.symbol );
      a.credit_balance = init_credit_asset;   
      a.last_price = init_credit_price;     // Initializes to a ratio of 100:1; 
   });
   
   _db.adjust_pending_supply( init_lent_asset );               
   _db.adjust_liquid_balance( o.issuer, init_credit_asset );    

} FC_CAPTURE_AND_RETHROW( (o) ) } 


void asset_update_evaluator::do_apply(const asset_update_operation& o)
{ try {
   const account_name_type& issuer = _db.get_account( o.issuer );
   const account_name_type& signed_for = o.issuer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   if( o.new_issuer.is_valid() )
   {
      const account_name_type& new_issuer = _db.get_account( *o.new_issuer );
   }

   const asset_object& asset = _db.get_asset(o.asset_to_update);
   auto asset_copy = asset;
   asset_copy.options = o.new_options;
   asset_copy.validate();
   const asset_dynamic_data_object& dyn_data = _db.get_dynamic_data(o.asset_to_update);

   if( (dyn_data.total_supply != 0) )
   {
      // new issuer_permissions must be subset of old issuer permissions
      FC_ASSERT(!(o.new_options.issuer_permissions & ~asset.options.issuer_permissions), "Cannot reinstate previously revoked issuer permissions on an asset.");
   }

   // changed flags must be subset of old issuer permissions
   FC_ASSERT(!((o.new_options.flags ^ asset.options.flags) & ~asset.options.issuer_permissions), "Flag change is forbidden by issuer permissions");
   FC_ASSERT( o.issuer == asset.issuer, "Incorrect issuer for asset! (${o.issuer} != ${a.issuer})",("o.issuer", o.issuer)("asset.issuer", asset.issuer) );

   const dynamic_global_property_object& gprops = _db.get_dynamic_global_properties();

   FC_ASSERT( o.new_options.whitelist_authorities.size() <= gprops.maximum_asset_whitelist_authorities );
   for( auto account : o.new_options.whitelist_authorities )
      _db.get_account(account);
   FC_ASSERT( o.new_options.blacklist_authorities.size() <= gprops.maximum_asset_whitelist_authorities );
   for( auto account : o.new_options.blacklist_authorities )
      _db.get_account(account);

   // If we are now disabling force settlements, cancel all open force settlement orders
   if( (o.new_options.flags & disable_force_settle) && asset.can_force_settle() )
   {
      const auto& idx = _db.get_index< force_settlement_index >().indices().get< by_expiration >();
      // Re-initialize itr every loop as objects are being removed each time.
      for( auto itr = idx.lower_bound(o.asset_to_update); 
      itr != idx.end() && itr->settlement_asset_symbol() == o.asset_to_update; 
      itr = idx.lower_bound(o.asset_to_update) )
         _db.cancel_settle_order(*itr, true);
   }

   // For market-issued assets, if core change rate changed, update flag in bitasset data
   if( asset.is_market_issued() && asset.options.core_exchange_rate != o.new_options.core_exchange_rate )
   {
      const asset_bitasset_data_object& bitasset = _db.get_bitasset_data(o.asset_to_update);
      if( !bitasset.asset_cer_updated )
      {
         _db.modify( bitasset, [](asset_bitasset_data_object& b)
         {
            b.asset_cer_updated = true;
         });
      }
   }

   _db.modify(asset, [&o](asset_object& a) 
   {
      if( o.new_issuer ) 
      {
         a.issuer = *o.new_issuer;
      }
      a.options = o.new_options;
   });

} FC_CAPTURE_AND_RETHROW( (o) ) }


void asset_issue_evaluator::do_apply( const asset_issue_operation& o )
{ try {
   const account_name_type& issuer = _db.get_account( o.issuer );
   const account_name_type& signed_for = o.issuer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const asset_object& asset = _db.get_asset(o.asset_to_issue.symbol);
   FC_ASSERT( o.issuer == asset.issuer, 
      "Only the asset issuer can issue new units of an asset." );
   FC_ASSERT( !asset.is_market_issued(), 
      "Cannot manually issue ${sym} because it is a market-issued asset. They must be issued in response to an asset input.", ("sym", asset.symbol) );

   const account_object& to_account = _db.get_account(o.issue_to_account);
   const account_permission_object& to_account_permissions = _db.get_account_permissions(o.issue_to_account);
   FC_ASSERT( to_account_permissions.is_authorized_asset( asset ) , 
      "The recipient account is not authoirzed to hold the asset being issued.");

   const asset_dynamic_data_object& asset_dyn_data = _db.get_dynamic_data(asset.symbol);

   FC_ASSERT( (asset_dyn_data.total_supply + o.asset_to_issue.amount) <= asset.options.max_supply,
      "Issuing this amount would exceed the asset's maximum supply. Please raise the maximum supply, or reduce issuance amount." );

   _db.adjust_liquid_balance( to_account, o.asset_to_issue );

} FC_CAPTURE_AND_RETHROW( (o) ) }


void asset_reserve_evaluator::do_apply( const asset_reserve_operation& o )
{ try {
   const asset_object& asset = _db.get_asset(o.amount_to_reserve.symbol);

   FC_ASSERT(!asset.is_market_issued(), 
      "Cannot reserve ${sym} because it is a market-issued asset",  );

   const account_object& from_account = _db.get_account(o.payer);
   const account_name_type& signed_for = o.payer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_permission_object& from_account_permissions = _db.get_account_permissions(o.payer);
   FC_ASSERT( from_account_permissions.is_authorized_asset( asset ),
      "The recipient account is not authorized to reserve the asset.");

   const auto& asset_dyn_data = _db.get_dynamic_data(asset.symbol);
   FC_ASSERT( (asset_dyn_data.total_supply - o.amount_to_reserve.amount) >= 0,
      "Cannot reserve more of an asset than its current total supply.");

   _db.adjust_liquid_balance( from_account, -o.amount_to_reserve );

} FC_CAPTURE_AND_RETHROW( (o) ) }


void asset_claim_fees_evaluator::do_apply( const asset_claim_fees_operation& o )
{ try {
   const asset_object& asset = _db.get_asset( o.amount_to_claim.symbol );
   const asset_dynamic_data_object& dyn_data = _db.get_dynamic_data( o.amount_to_claim.symbol );
   const account_name_type& issuer = _db.get_account( o.issuer );
   const account_name_type& signed_for = o.issuer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   FC_ASSERT( asset.issuer == o.issuer, 
      "An Asset's fees may only be claimed by it's issuing account." );
   
   FC_ASSERT( o.amount_to_claim.amount <= dyn_data.accumulated_fees, 
      "Cannot claim more fees than have accumulated: ${a}. Please lower the claim amount", ("a", dyn_data.accumulated_fees) );

   _db.modify( dyn_data, [&]( asset_dynamic_data_object& addo  ) 
   {
     addo.adjust_accumulated_fees( -o.amount_to_claim );
   });

   _db.adjust_liquid_balance( o.issuer, o.amount_to_claim );
} FC_CAPTURE_AND_RETHROW( (o) ) }


void asset_claim_pool_evaluator::do_apply( const asset_claim_pool_operation& o )
{ try {
   const account_name_type& issuer = _db.get_account( o.issuer );
   const account_name_type& signed_for = o.issuer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const asset_object& asset = _db.get_asset( o.symbol );
   const asset_dynamic_data_object& dyn_data = _db.get_dynamic_data( o.symbol );
   
   FC_ASSERT( o.amount_to_claim.symbol == SYMBOL_COIN, 
      "Amount to claim from fee pool must be denominated in the core asset." );

   FC_ASSERT( asset.issuer == o.issuer, 
      "An Asset's fees may only be claimed by it's issuing account." );
   
   FC_ASSERT( o.amount_to_claim.amount <= dyn_data.fee_pool, 
      "Cannot claim more fees than have accumulated: ${a}. Please lower the claim amount", ("a", dyn_data.accumulated_fees) );

   _db.modify( dyn_data, [&o]( asset_dynamic_data_object& addo  ) 
   {
      addo.adjust_fee_pool( -o.amount_to_claim);
   });

   _db.adjust_liquid_balance( o.issuer, o.amount_to_claim );
} FC_CAPTURE_AND_RETHROW( (o) ) }


void asset_fund_fee_pool_evaluator::do_apply(const asset_fund_fee_pool_operation& o)
{ try {
   const account_name_type& signed_for = o.from_account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const asset_object& asset = _db.get_asset(o.symbol);
   const account_object& from_account = _db.get_account(o.from_account);

   FC_ASSERT(o.pool_amount.symbol == SYMBOL_COIN,
      "Amount to claim from fee pool must be denominated in the core asset." );

   const asset_dynamic_data_object& dyn_data = _db.get_dynamic_data(o.symbol);

   _db.adjust_liquid_balance(from_account, -o.pool_amount);

   _db.modify( dyn_data, [&]( asset_dynamic_data_object& addo ) 
   {
      addo.adjust_fee_pool( o.pool_amount );
   });

} FC_CAPTURE_AND_RETHROW( (o) ) }


static void validate_new_issuer(database& _db, const asset_object& a, const account_name_type& new_issuer )
{ try {
   const account_object* new_issuer_account_ptr = _db.find_account(new_issuer);
   FC_ASSERT(new_issuer_account_ptr != nullptr);
   const account_object& new_issuer_account = *new_issuer_account_ptr;

   if( a.is_market_issued() && new_issuer == COMMITTEE_ACCOUNT )
   {
      const asset_bitasset_data_object& bitasset_data = _db.get_bitasset_data(a.symbol);
      const asset_object& backing_asset = _db.get_asset(bitasset_data.backing_asset);
      if( backing_asset.is_market_issued() )
      {
         const asset_bitasset_data_object& backing_bitasset_data = _db.get_bitasset_data(backing_asset.symbol);
         const asset_object& backing_backing_asset = _db.get_asset(backing_bitasset_data.symbol);
         FC_ASSERT( backing_backing_asset.symbol == SYMBOL_COIN, "May not create a blockchain-controlled market asset which is not backed by CORE.");
      } 
      else
      {
         FC_ASSERT( backing_asset.symbol == SYMBOL_COIN, "May not create a blockchain-controlled market asset which is not backed by CORE.");
      }   
   }
} FC_CAPTURE_AND_RETHROW( (a)(new_issuer) ) }


void asset_update_issuer_evaluator::do_apply(const asset_update_issuer_operation& o)
{ try {
   const account_name_type& signed_for = o.issuer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_chief( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& asset = _db.get_asset(o.asset_to_update);
   validate_new_issuer(_db, asset, o.new_issuer );

   FC_ASSERT( o.issuer == asset.issuer, "Invalid issuer for asset (${o.issuer} != ${a.issuer})",
      ("o.issuer", o.issuer)("asset.issuer", asset.issuer) );

   _db.modify( asset, [&](asset_object& a) 
   {
      a.issuer = o.new_issuer;
   });

} FC_CAPTURE_AND_RETHROW( (o) ) }


/*
 * Loop through assets, looking for ones that are backed by the asset being changed. When found,
 * perform checks to verify validity.
 *
 * @param op the bitasset update operation being performed
 * @param new_backing_asset
 */
void check_children_of_bitasset(database& db, const asset_update_bitasset_operation& o, const asset_object& new_backing_asset)
{
   if ( new_backing_asset.symbol != SYMBOL_COIN ) //  If new backing asset is CORE, no child bitassets to review. 
   {
      const auto& idx = db.get_index< asset_bitasset_data_index >().indices().get< by_short_backing_asset >();
      auto backed_range = idx.equal_range(o.asset_to_update);
      std::for_each( backed_range.first, backed_range.second, // loop through all assets that have this asset as a backing asset
         [&new_backing_asset, &o, &db](const asset_bitasset_data_object& bitasset_data)
         {
            const auto& child = db.get_bitasset_data(bitasset_data.symbol);

            FC_ASSERT( child.symbol != o.new_options.short_backing_asset,
                  "The BitAsset would be invalidated by changing this backing asset ('A' backed by 'B' backed by 'A')." );
         } ); 
   }
} 


/*******
 * @brief Apply requested changes to bitasset options
 *
 * This applies the requested changes to the bitasset object. It also cleans up the
 * releated feeds
 *
 * @param op the requested operation
 * @param db the database
 * @param bdo the actual database object
 * @param asset_to_update the asset_object related to this bitasset_data_object
 * @returns true if the feed price is changed
 */
bool update_bitasset_object_options( const asset_update_bitasset_operation& o, database& db, asset_bitasset_data_object& bdo, const asset_object& asset_to_update )
{
   const fc::time_point next_maint_time = db.get_dynamic_global_properties().next_maintenance_time;

   // If the minimum number of feeds to calculate a median has changed,  recalculate the median
   bool should_update_feeds = false;
   const bitasset_options& options = *bdo.options;

   if( o.new_options.minimum_feeds != options.minimum_feeds )
      should_update_feeds = true;

   // call update_median_feeds if the feed_lifetime changed
   if( o.new_options.feed_lifetime != options.feed_lifetime )
   {
      should_update_feeds = true;
   }

   // feeds must be reset if the backing asset is changed
   bool backing_asset_changed = false;
   bool is_witness_or_committee_fed = false;
   if( o.new_options.short_backing_asset != bdo.backing_asset )
   {
      backing_asset_changed = true;
      should_update_feeds = true;
      if( asset_to_update.options.flags & ( witness_fed_asset | committee_fed_asset ) )
         is_witness_or_committee_fed = true;
   }

   bdo.options = o.new_options;

   if( backing_asset_changed ) // Reset price feeds if modifying backing asset
   {
      if( is_witness_or_committee_fed )
      {
         bdo.feeds.clear();
      }
      else
      {
         for( auto& current_feed : bdo.feeds )
         {
            current_feed.second.second.settlement_price = price();  // Zero all outstanding price feeds
         }
      }
   }

   if( should_update_feeds ) // Call check_call_orders if the price feed changes
   {
      const auto old_feed = bdo.current_feed;
      bdo.update_median_feeds( db.head_block_time(), next_maint_time );
      return ( !( old_feed == bdo.current_feed ) ); 
   }

   return false;
}


void asset_update_bitasset_evaluator::do_apply(const asset_update_bitasset_operation& o)
{ try {
   const account_name_type& signed_for = o.issuer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& asset_obj = _db.get_asset( o.asset_to_update );

   FC_ASSERT( asset_obj.is_market_issued(), 
      "Cannot update BitAsset-specific settings on a non-BitAsset." );
   FC_ASSERT( o.issuer == asset_obj.issuer, 
      "Only asset issuer can update bitasset_data of the asset." );

   const asset_object& dyn_data = _db.get_dynamic_data( o.asset_to_update );
   const asset_bitasset_data_object& current_bitasset_data = _db.get_bitasset_data(asset_obj.symbol);

   FC_ASSERT( !current_bitasset_data.has_settlement(), 
      "Cannot update a bitasset after a global settlement has executed" );
   
   if( o.new_options.short_backing_asset != current_bitasset_data.options.short_backing_asset )   // Are we changing the backing asset?
   {
      FC_ASSERT( dyn_data.get_total_supply().amount == 0,
         "Cannot update a bitasset's backing asset if there is already a current supply. Please globally settle first." );

      const asset_object& new_backing_asset = _db.get_asset(o.new_options.short_backing_asset); // Check if the new backing asset exists

      FC_ASSERT( o.new_options.short_backing_asset != asset_obj.symbol,
         "Cannot update a bitasset to be backed by itself. Please select a different backing asset." );

      if( asset_obj.issuer == COMMITTEE_ACCOUNT )
      {
         if( new_backing_asset.is_market_issued() )
         {
            const asset_bitasset_data_object& backing_bitasset_data = _db.get_bitasset_data(new_backing_asset.symbol);
            FC_ASSERT( backing_bitasset_data.options.short_backing_asset == SYMBOL_COIN,
               "Commitee may not modify a blockchain-controlled market asset to be backed by an asset which is not backed by core asset." );

            check_children_of_bitasset( _db, o, new_backing_asset );  // Ensure backing asset does not cause recursive backing.
         }
         else
         {
            FC_ASSERT( new_backing_asset.symbol == SYMBOL_COIN,
               "May not modify a blockchain-controlled market asset to be backed by an asset which is not market issued asset nor core asset." );
         }
      }
      else // not a committee issued asset
      {
         // If we're changing to a backing_asset that is not CORE, we need to look at any
         // asset ( "CHILD" ) that has this one as a backing asset. If CHILD is committee-owned,
         // the change is not allowed. If CHILD is user-owned, then this asset's backing
         // asset must be either CORE or a UIA.
         if ( new_backing_asset.symbol != SYMBOL_COIN ) // not backed by CORE
         {
            check_children_of_bitasset( _db, o, new_backing_asset );   // Checks for recursive backing assets
         }
      }

      // Check if the new backing asset is itself backed by something. It must be CORE or a UIA.
      if ( new_backing_asset.is_market_issued() )
      {
         const asset_bitasset_data_object& backing_bitasset_data = _db.get_bitasset_data(new_backing_asset.symbol);
         const asset_object& backing_backing_asset = _db.get_asset(backing_bitasset_data.options.short_backing_asset);

         FC_ASSERT( (backing_backing_asset.symbol == SYMBOL_COIN || !backing_backing_asset.is_market_issued()),
            "A BitAsset cannot be backed by a BitAsset that itself is backed by a BitAsset.");
      }
   }
   
   FC_ASSERT( o.new_options.feed_lifetime > BLOCK_INTERVAL,
      "Feed lifetime must exceed block interval." );
   FC_ASSERT( o.new_options.force_settlement_delay > BLOCK_INTERVAL,
      "Force settlement delay must exceed block interval." );

   bool to_check_call_orders = false;

   _db.modify( current_bitasset_data, [&o, &asset_obj, &to_check_call_orders, &_db] ( asset_bitasset_data_object& bdo )
   {
      to_check_call_orders = update_bitasset_object_options( o, _db, bdo, asset_obj );
   });

   if( to_check_call_orders )     // Process margin calls, allow black swan, not for a new limit order
   {
      _db.check_call_orders( asset_obj, true, false );
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void asset_update_feed_producers_evaluator::do_apply(const asset_update_feed_producers_operation& o)
{ try {
   const account_name_type& signed_for = o.issuer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const chain_properties& cprops = _db.chain_properties();
   time_point now = _db.head_block_time();
   time_point next_maint_time = _db.next_maintenance_time();

   FC_ASSERT( o.new_feed_producers.size() <= cprops.maximum_asset_feed_publishers,
      "Cannot specify more feed producers than maximum allowed" );

   const asset_object& a = _db.get_asset( o.asset_to_update);

   FC_ASSERT(a.is_market_issued(), 
      "Cannot update feed producers on a non-BitAsset.");
   FC_ASSERT(!(a.options.flags & committee_fed_asset), 
      "Cannot set feed producers on a committee-fed asset.");
   FC_ASSERT(!(a.options.flags & witness_fed_asset), 
      "Cannot set feed producers on a witness-fed asset.");
   FC_ASSERT( a.issuer == o.issuer, 
      "Only asset issuer can update feed producers of an asset" );
   
   for( account_name_type name : o.new_feed_producers )     // Make sure all producers exist. Check these after asset because account lookup is more expensive.
   {
      _db.get_account( name );
   }
      
   const asset_bitasset_data_object& bitasset_to_update = _db.get_bitasset_data(a.symbol);

   _db.modify( bitasset_to_update, [&o,now,next_maint_time](asset_bitasset_data_object& abdo) 
   {
      for( auto itr = abdo.feeds.begin(); itr != abdo.feeds.end(); )    // Remove any old publishers who are no longer publishers.
      {
         if( !o.new_feed_producers.count(itr->first) )
         {
            itr = abdo.feeds.erase(itr);     // Resets iterator to the new feeds itr with the name's key removed.
         }
         else
         {
            ++itr;
         }
            
      }
      for( const account_name_type name : o.new_feed_producers )    // Now, add map keys for any new publishers.
      {
         abdo.feeds[name];
      }
      abdo.update_median_feeds( now, next_maint_time );
   });
   
   _db.check_call_orders( a, true, false );     // Process margin calls, allow black swan, not for a new limit order

} FC_CAPTURE_AND_RETHROW( (o) ) }


void asset_publish_feed_evaluator::do_apply(const asset_publish_feed_operation& o)
{ try {
   const account_name_type& signed_for = o.publisher;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const asset_object& base = _db.get_asset( o.symbol );     // Verify that this feed is for a market-issued asset and that asset is backed by the base.

   time_point now = _db.head_block_time();
   time_point next_maint_time = _db.next_maintenance_time();
   
   FC_ASSERT( base.is_market_issued(), 
      "Can only publish price feeds for market-issued assets" );

   const asset_bitasset_data_object& bitasset = _db.get_bitasset_data( base.symbol);

   // The settlement price must be quoted in terms of the backing asset.
   FC_ASSERT( o.feed.settlement_price.quote.symbol == bitasset.options.short_backing_asset,
      "Quote asset type in settlement price should be same as backing asset of this asset" );

   if( !o.feed.core_exchange_rate.is_null() )
   {
      FC_ASSERT( o.feed.core_exchange_rate.quote.symbol == SYMBOL_COIN,
         "Quote asset in core exchange rate should be CORE asset" );
   }
   
   if( base.options.flags & witness_fed_asset )     // Verify that the publisher is authoritative to publish a feed.
   {
      FC_ASSERT( _db.get_account(WITNESS_ACCOUNT).active.account_auths.count(o.publisher),
         "Only active witnesses are allowed to publish price feeds for this asset" );
   }
   else if( base.options.flags & committee_fed_asset )
   {
      FC_ASSERT( _db.get_account(COMMITTEE_ACCOUNT).active.account_auths.count(o.publisher),
         "Only active committee members are allowed to publish price feeds for this asset" );
   }
   else
   {
      FC_ASSERT( bitasset.feeds.count(o.publisher),
         "The account is not in the set of allowed price feed producers of this asset" );
   }

   price_feed old_feed =  bitasset.current_feed;    // Store medians for this asset.
   
   _db.modify( bitasset , [&o,now,next_maint_time](asset_bitasset_data_object& abdo) 
   {
      abdo.feeds[o.publisher] = make_pair( now, o.feed );
      abdo.update_median_feeds( now, next_maint_time );
   });

   if( !(old_feed == bitasset.current_feed) )
   {
      // Check whether need to revive the asset and proceed if need
      if( bitasset.has_settlement() && !bitasset.current_feed.settlement_price.is_null() ) // has globally settled and has a valid feed
      {
         bool should_revive = false;
         const auto& mia_dyn = _db.get_dynamic_data(base.symbol);
         if( mia_dyn.current_supply == 0 ) // If current supply is zero, revive the asset
         {
            should_revive = true;
         }
         else // if current supply is not zero, when collateral ratio of settlement fund is greater than MCR, revive the asset.
         {
            // calculate collateralization and compare to maintenance_collateralization
            if( price( asset( bitasset.settlement_fund, bitasset.options.short_backing_asset ), asset( mia_dyn.current_supply, o.symbol ) ) > bitasset.current_maintenance_collateralization )
            {
               should_revive = true;
            }
               
         }
         if( should_revive )
         {
            d.revive_bitasset(base);
         } 
      }
      _db.check_call_orders( base, true, false );    // Process margin calls, allow black swan, not for a new limit order
   }
} FC_CAPTURE_AND_RETHROW((o)) }


void asset_settle_evaluator::do_apply(const asset_settle_operation& o)
{ try {
   const account_name_type& signed_for = o.account;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const asset_object& asset_to_settle = _db.get_asset( o.amount.symbol ));
   const asset_dynamic_data_object& mia_dyn = _db.get_dynamic_data( o.amount.symbol );
   time_point now = _db.head_block_time();

   FC_ASSERT(asset_to_settle.is_market_issued(), 
      "Cannot settle a non-market issued asset.");

   const asset_bitasset_data_object& bitasset = _db.get_bitasset_data( o.amount.symbol ));

   FC_ASSERT(asset_to_settle.can_force_settle() || bitasset.has_settlement(),
      "Asset must be able to be force settled, or have a global settlement price to settle." );

   asset liquid = _db.get_liquid_balance(o.account, asset_to_settle.symbol);
   
   FC_ASSERT(liquid >= o.amount, 
      "Account does not have enough of the asset to settle the requested amount.");

   if( bitasset.has_settlement() )   // Asset has been globally settled. 
   {
      asset settled_amount = o.amount * bitasset.settlement_price;         // Round down, in favor of global settlement fund.

      if( o.amount == mia_dyn.get_total_supply() )             // Settling the entire asset remaining supply. 
      {
         settled_amount.amount = bitasset.settlement_fund;        // Set the settled amount to the exact remaining supply. 
      }
      else
      {
         FC_ASSERT( settled_amount.amount <= bitasset.settlement_fund,
            "Settled amount should be less than or equal to settlement fund." ); // should be strictly < except for PM with zero outcome
      }
         
      FC_ASSERT( settled_amount.amount != 0, 
         "Rounding issue: Settlement amount should not be zero." );
      
      asset pays = o.amount;
      if( o.amount != mia_dyn.get_total_supply() && settled_amount.amount != 0 )
      {
         pays = settled_amount.multiply_and_round_up( bitasset.settlement_price );
      }

      _db.adjust_liquid_balance( o.account, -pays );

      if( settled_amount.amount > 0 )     // Transfer from global settlement fund to account. 
      {
         _db.modify( bitasset, [&]( asset_bitasset_data_object& adbo )   
         {
            abdo.settlement_fund -= settled_amount.amount;
         });

         _db.adjust_liquid_balance( o.account, settled_amount );
         _db.adjust_pending_supply( -settled_amount );
      }
   }
   else    // Not globally settled, creating force settlement. 
   {
      _db.adjust_balance( o.account, -o.amount );

      time_point settlement_time = now + bitasset.options.force_settlement_delay;

      _db.create<force_settlement_object>([&](force_settlement_object& fso) 
      {
         fso.owner = o.account;
         fso.balance = o.amount;
         fso.settlement_date = settlement_time;
      });
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void asset_global_settle_evaluator::do_apply(const asset_global_settle_operation& o)
{ try {
   const account_name_type& signed_for = o.issuer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& asset_to_settle = _db.get_asset(o.asset_to_settle);
   const asset_dynamic_data_object& mia_dyn = _db.get_dynamic_data( o.amount.symbol );

   FC_ASSERT( asset_to_settle.is_market_issued(), 
      "Can only globally settle market-issued assets." );
   FC_ASSERT( asset_to_settle.can_global_settle(), 
      "The global_settle permission of this asset is disabled." );
   FC_ASSERT( asset_to_settle.issuer == o.issuer, 
      "Only asset issuer can globally settle an asset." );
   FC_ASSERT( mia_dyn.get_total_supply().amount > 0, 
      "Cannot globally settle an asset with zero supply." );

   const asset_bitasset_data_object& bitasset = _db.get_bitasset_data( o.asset_to_settle );

   // If there is a settlement for this asset, then no further global settlement may be taken.

   FC_ASSERT( !bitasset.has_settlement(), 
      "This asset has already been globally settled, cannot global settle again." );

   const auto& idx = _db.get_index<call_order_index>().indices().get<by_collateral>();

   FC_ASSERT( !idx.empty(), 
      "Critical error: No debt positions found on entire network. Bitasset supply should not exist." );

   auto itr = idx.lower_bound( price::min( bitasset.options.short_backing_asset, o.asset_to_settle ) );

   FC_ASSERT( itr != idx.end() && itr->debt_type() == o.asset_to_settle, 
      "Critical error: No debt positions found for the asset being settled. Bitasset supply should not exist." );

   const call_order_object& least_collateralized_short = *itr;

   FC_ASSERT(least_collateralized_short.debt * o.settle_price <= least_collateralized_short.collateral,
      "Cannot force settle at supplied price: Least collateralized short lacks sufficient collateral to settle. Please lower the price");

   _db.globally_settle_asset( asset_to_settle, o.settle_price );

} FC_CAPTURE_AND_RETHROW( (o) ) }


//==================================//
// === Block Producer Evaluators ===//
//==================================//


void witness_update_evaluator::do_apply( const witness_update_operation& o )
{ try {
   const account_object& owner = _db.get_account( o.owner ); // verify owner exists
   const account_name_type& signed_for = o.owner;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   FC_ASSERT( o.url.size() <= MAX_URL_LENGTH, 
      "URL is too long" );

   FC_ASSERT( o.details.size() <= MAX_STRING_LENGTH, 
      "Details are too long" );

   FC_ASSERT( o.props.account_creation_fee.symbol == SYMBOL_COIN,
      "Acccount creation fee must be in the core asset." );
   
   time_point now = _db.head_block_time();
   
   const auto& by_witness_name_idx = _db.get_index< witness_index >().indices().get< by_name >();

   auto wit_itr = by_witness_name_idx.find( o.owner );

   if( wit_itr != by_witness_name_idx.end() )
   {
      _db.modify( *wit_itr, [&]( witness_object& w ) 
      {
         if ( o.json.size() > 0 )
         {
            from_string( w.json, o.json );
         }
         if ( o.details.size() > 0 )
         {
            from_string( w.details, o.details );
         }
         if ( o.url.size() > 0 )
         {
            from_string( w.url, o.url );
         }

         w.signing_key        = o.block_signing_key;

         w.props              = o.props;
      });
   }
   else
   {
      _db.create< witness_object >( [&]( witness_object& w ) 
      {
         w.owner              = o.owner;

         if ( o.json.size() > 0 )
         {
            from_string( w.json, o.json );
         }
         if ( o.details.size() > 0 )
         {
            from_string( w.details, o.details );
         }
         if ( o.url.size() > 0 )
         {
            from_string( w.url, o.url );
         }

         w.signing_key        = o.block_signing_key;

         w.created            = now;

         w.props              = o.props;
      });
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void proof_of_work_evaluator::do_apply( const proof_of_work_operation& o )
{ try {
   const auto& work = o.work.get< equihash_proof_of_work >();
   uint32_t target_pow = _db.pow_difficulty();

   FC_ASSERT( work.prev_block == _db.head_block_id(), 
      "Proof of Work op not for last block." );
   auto recent_block_num = protocol::block_header::num_from_id( work.input.prev_block );

   FC_ASSERT( recent_block_num > dgp.last_irreversible_block_num,
      "Proof of Work done for block older than last irreversible block number." );

   FC_ASSERT( work.pow_summary < target_pow, 
      "Insufficient work difficulty. Work: ${w}, Target: ${t} .", ("w",work.pow_summary)("t", target_pow) );

   const dynamic_global_property_object& dgp = _db.get_dynamic_global_properties();
   time_point now = dgp.time;
   const witness_schedule_object& witness_schedule = get_witness_schedule();
   fc::microseconds decay_rate = witness_schedule.pow_decay_time;               // Averaging window of the targetting adjustment
   account_name_type worker_account = work.input.worker_account;
   
   FC_ASSERT( o.props.maximum_block_size >= MIN_BLOCK_SIZE_LIMIT * 2, "Voted maximum block size is too small." );

   uint32_t work_difficulty = ( 1 << 30 ) / work.pow_summary; 

   const auto& accounts_by_name = _db.get_index<account_index>().indices().get<by_name>();
   auto itr = accounts_by_name.find( worker_account );

   if(itr == accounts_by_name.end())
   {
      FC_ASSERT( o.new_owner_key.valid(), "New owner key is not valid." );

      public_key_type ok = *o.new_owner_key;

      _db.create< account_object >( [&]( account_object& acc )
      {
         acc.registrar = NULL_ACCOUNT;
         acc.name = worker_account;
         acc.recovery_account = "";          // Highest voted witness at time of recovery.

         acc.secure_public_key = ok;
         acc.connection_public_key = ok;
         acc.friend_public_key = ok;
         acc.companion_public_key = ok;

         acc.created = now;
         acc.last_account_update = now;
         acc.last_vote_time = now;
         acc.last_post = now;
         acc.last_root_post = now;
         acc.last_transfer = now;
         acc.last_owner_proved = now;
         acc.last_active_proved = now;
         acc.last_activity_reward = now;
         acc.last_account_recovery = now;
         acc.mined = true;
      });

      _db.create< account_authority_object >( [&]( account_authority_object& auth )
      {
         auth.account = worker_account;
         auth.owner = authority( 1, ok, 1);
         auth.active = auth.owner;
         auth.posting = auth.owner;
      });

      _db.create<witness_object>( [&]( witness_object& w )
      {
         w.owner             = worker_account;
         w.props             = o.props;
         w.signing_key       = ok;
         w.mining_count      = 1;
         w.mining_power      = BLOCKCHAIN_PRECISION;
         w.last_mining_update     = now;
      });
   }
   else
   {
      FC_ASSERT( !o.new_owner_key.valid(), 
         "Cannot specify an owner key unless creating account." );

      const witness_object* cur_witness = _db.find_witness( worker_account );

      FC_ASSERT( cur_witness, 
         "Witness must be created for existing account before mining.");

      // Decay and increment mining power for the miner. 
      _db.modify( *cur_witness, [&]( witness_object& w )
      {
         w.mining_count++;
         w.props            = o.props;
         w.mining_power     -= ( w.mining_power * ( now - w.last_mining_update ).to_seconds() ) / decay_rate.to_seconds();
         w.mining_power     += BLOCKCHAIN_PRECISION;
         w.last_mining_update    = now;
      });
   }

   _db.modify( dgp, [&]( dynamic_global_property_object& p )
   {
      p.total_pow + work_difficulty;
   });

   _db.claim_proof_of_work_reward( worker_account );

} FC_CAPTURE_AND_RETHROW( (o) ) }


void verify_block_evaluator::do_apply( const verify_block_operation& o )
{ try {
   const account_object& producer = _db.get_account( o.producer ); 
   const witness_object& witness = _db.get_witness( o.producer );
   const account_name_type& signed_for = o.producer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   auto recent_block_num = protocol::block_header::num_from_id( o.block_id );

   FC_ASSERT( recent_block_num == o.height, 
      "Block with this ID not found at this height." );

   FC_ASSERT( recent_block_num > props.last_irreversible_block_num,
      "Verify Block done for block older than last irreversible block number." );

   FC_ASSERT( recent_block_num > props.last_committed_block_num,
      "Verify Block done for block older than last committed block number." );

   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   time_point now = props.time;
   const witness_schedule_object& witness_schedule = get_witness_schedule();
   
   vector<account_name_type> top_witnesses = witness_schedule.top_witnesses;
   vector<account_name_type> top_miners = witness_schedule.top_miners;

   FC_ASSERT( std::find( top_witnesses.begin(), top_witnesses.end(), witness.name) != top_witnesses.end() ||
      std::find( top_miners.begin(), top_miners.end(), witness.name) != top_miners.end(),
      "Producer must be a top witness or miner to publish a block verification." );

   const auto& valid_idx = get_index< block_validation_index >().indices().get< by_producer_height >();
   auto valid_itr = valid_idx.find( boost::make_tuple( witness.name, o.height ) );
   
   if( valid_itr == valid_idx.end() )     // New verification object at this height.
   {
      _db.create<block_validation_object>( [&]( block_validation_object& bvo )
      {
         bvo.producer = producer.name;
         bvo.block_id = o.block_id;
         bvo.height = o.height;
         bvo.created  = now;
         bvo.committed = false;
      });
   }
   else   // Existing verifcation exists, Changing uncommitted block_id
   {
      const block_validation_object& val = *valid_itr;

      FC_ASSERT( val.block_id != o.block_id, 
         "Operation must change an uncommitted block id.");

      FC_ASSERT( val.committed == false, 
         "CANNOT ALTER COMMITED VALIDATION. PRODUCER: ${p) BLOCK_ID1: ${o} BLOCK_ID2: ${p} HEIGHT: ${h} PRODUCER VIOLATION DETECTED.", 
            ("producer", o.producer)("o", val.block_id)("b", o.block_id)("h", o.height) );

      _db.modify( val, [&]( block_validation_object& bvo )
      {
         bvo.block_id = o.block_id;
      });
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void commit_block_evaluator::do_apply( const commit_block_operation& o )
{ try {
   const account_object& producer = _db.get_account( o.producer );
   const witness_object& witness = _db.get_witness( o.producer );
   const account_name_type& signed_for = o.producer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   auto recent_block_num = protocol::block_header::num_from_id( o.block_id );

   FC_ASSERT( recent_block_num == o.height, 
      "Block with this ID not found at this height." );

   FC_ASSERT( recent_block_num > props.last_irreversible_block_num,
      "Commit Block done for block older than last irreversible block number." );

   FC_ASSERT( recent_block_num > props.last_committed_block_num,
      "Commit Block done for block older than last irreversible block number." );

   FC_ASSERT( verifications.size() >= IRREVERSIBLE_THRESHOLD, 
      "Insufficient Verifications for commit transaction. Please await further verifications from block producers." );

   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   time_point now = props.time;
   const witness_schedule_object& witness_schedule = get_witness_schedule();
   
   vector<account_name_type> top_witnesses = witness_schedule.top_witnesses;
   vector<account_name_type> top_miners = witness_schedule.top_miners;

   FC_ASSERT( witness_schedule.is_top_producer( o.producer ),
      "Producer must be a top witness or miner to publish a block commit." );

   const auto& valid_idx = get_index< block_validation_index >().indices().get< by_producer_height >();
   auto valid_itr = valid_idx.find( boost::make_tuple( witness.name, o.height ) );

   FC_ASSERT( valid_itr != valid_idx.end(), 
      "Cannot commit a block height which the producer has not yet verified.");
   
   const block_validation_object& val = *valid_itr;

   FC_ASSERT( val.block_id == o.block_id, 
      "Different block ID than the verification ID, please update verification.");

   asset stake = _db.get_staked_balance( producer.name, SYMBOL_COIN );

   FC_ASSERT( stake >= o.stake, 
      "Producer has insufficient staked balance to stake the provided amount.");

   flat_set<account_name_type> verifiers;

   for( transaction_id_type txn : o.verifications )  // Ensure that all included verification transactions are valid.
   {
      annotated_signed_transaction verify_txn = _db.get_transaction( txn );
      FC_ASSERT( verify_txn.operations.size(), "Transaction ID ${t} has no operations.", ("t", txn) );
      operation& op = verify_txn.operations[0];

      FC_ASSERT( op.which() == operation::tag< verify_block_operation >::value , 
         "Transaction is not a verification transaction. ");
      
      const verify_block_operation& verify_op = op.get< verify_block_operation >();

      verify_op.validate();

      FC_ASSERT( verify_op.block_id == o.block_id, 
         "Verifying transaction is not for this block ID." );
      FC_ASSERT( verify_op.height == o.height, 
         "Verification transaction is not for this height." );

      const account_object& verify_acc = _db.get_account( verify_op.producer );
      const witness_object& verify_wit = _db.get_witness( verify_op.producer );

      FC_ASSERT( witness_schedule.is_top_producer( verify_wit.name ), 
         "Verifying Account is not a top producer." );
   
      verifiers.insert( verify_wit.name );
   }

   FC_ASSERT( verifiers.size() >= IRREVERSIBLE_THRESHOLD, 
      "Insufficient Unique Concurrent Valid Verifications for commit transaction. Please await further verifications from block producers." );

   _db.modify( val, [&]( block_validation_object& bvo )
   {
      bvo.committed = true;                // Verification cannot be altered after committed. 
      bvo.commit_time = now;               // Fastest by commit time are eligible for validation reward.
      bvo.stake = o.stake;                 // Reward is weighted by stake committed. 
      bvo.verifications = o.verifications;
      bvo.verifers = verifiers;
   });

   _db.modify( witness, [&]( witness_object& w )
   {
      w.last_commit_height = o.height;
      w.last_commit_id = o.block_id;
   });
   
} FC_CAPTURE_AND_RETHROW( (o) ) }


void producer_violation_evaluator::do_apply( const producer_violation_operation& o )
{ try {
   const account_object& producer = _db.get_account( o.producer ); 
   const witness_object& witness = db.get_witness( o.producer );
   const account_name_type& signed_for = o.producer;
   if( o.signatory != signed_for )
   {
      const account_object& signatory = _db.get_account( o.signatory );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   auto recent_block_num = protocol::block_header::num_from_id( o.block_id );

   FC_ASSERT( recent_block_num > props.last_irreversible_block_num,
      "Verify Block done for block older than last irreversible block number." );

   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   time_point now = props.time;
   const witness_schedule_object& witness_schedule = get_witness_schedule();
   
} FC_CAPTURE_AND_RETHROW( (o) ) }



//===========================//
// === Custom Evaluators === //
//===========================//



void custom_evaluator::do_apply( const custom_operation& o )
{ try {
   if( _db.is_producing() )
   {
      FC_ASSERT( o.data.size() <= 8192, "custom_operation data must be less than 8k" );
   }  
} FC_CAPTURE_AND_RETHROW( (o) ) }


void custom_json_evaluator::do_apply( const custom_json_operation& o )
{
   if( _db.is_producing() )
   {
      FC_ASSERT( o.json.length() <= 8192, "customJson_operation json must be less than 8k" );
   }
      
   std::shared_ptr< custom_operation_interpreter > eval = _db.get_custom_json_evaluator( o.id );
   if( !eval )
   {
      return;
   }

   try
   {
      eval->apply( o );
   }
   catch( const fc::exception& e )
   {
      if( _db.is_producing() )
      {
         throw e;
      }  
   }
   catch(...)
   {
      elog( "Unexpected exception applying custom json evaluator." );
   }
}


} } // node::chain