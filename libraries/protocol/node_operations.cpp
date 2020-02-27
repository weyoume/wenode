#include <node/protocol/node_operations.hpp>
#include <fc/io/json.hpp>

#include <locale>

namespace node { namespace protocol {

   bool inline is_asset_type( asset a, asset_symbol_type symbol )
   {
      return a.symbol == symbol;
   }

   //============================//
   // === Account Operations === //
   //============================//



   void account_create_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( registrar );
      validate_account_name( referrer );
      validate_account_name( new_account_name );
      
      if( proxy.size() )
      {
         validate_account_name( proxy );
      }
      if( governance_account.size() )
      {
         validate_account_name( governance_account );
      }
      if( recovery_account.size() )
      {
         validate_account_name( recovery_account );
      }

      FC_ASSERT( is_asset_type( fee, SYMBOL_COIN ), 
         "Account creation fee must be in core asset." );
      FC_ASSERT( is_asset_type( delegation, SYMBOL_COIN ), 
         "Delegation must be in core asset." );
      FC_ASSERT( fee >= asset( 0, SYMBOL_COIN ),
         "Account creation fee cannot be negative" );
      FC_ASSERT( delegation >= asset( 0, SYMBOL_COIN ),
         "Delegation cannot be negative" );

      owner_auth.validate();
      active_auth.validate();
      posting_auth.validate();

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ), 
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ), 
            "JSON Metadata not valid JSON." );
      }
      if( json_private.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json_private ), 
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json_private ), 
            "JSON Metadata not valid JSON." );
      }

      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }

      validate_public_key( secure_public_key );
      validate_public_key( connection_public_key );
      validate_public_key( friend_public_key );
      validate_public_key( companion_public_key );
   }

   void account_update_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( account != TEMP_ACCOUNT, 
         "Cannot update temp account." );

      owner_auth.validate();
      active_auth.validate();
      posting_auth.validate();
      
      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ), 
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ), 
            "JSON Metadata not valid JSON." );
      }
      if( json_private.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json_private ), 
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json_private ), 
            "JSON Metadata not valid JSON." );
      }

      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }

      validate_public_key( secure_public_key );
      validate_public_key( connection_public_key );
      validate_public_key( friend_public_key );
      validate_public_key( companion_public_key );
   }

   void account_business_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      switch( business_type )
      {
         case OPEN_BUSINESS:
         case PUBLIC_BUSINESS:
         case PRIVATE_BUSINESS:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Business Type is invalid." );
         }
      }
      
      FC_ASSERT( officer_vote_threshold > 0, 
         "Officer vote threshold must be greater than 0.");

      validate_public_key( business_public_key );
   }

   void account_profile_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( governance_account );
      validate_public_key( profile_public_key );

      FC_ASSERT( first_name.size() < MAX_STRING_LENGTH,
         "First name is too long." );
      FC_ASSERT( fc::is_utf8( first_name ), 
         "First name is not formatted in UTF8." );

      FC_ASSERT( last_name.size() < MAX_STRING_LENGTH,
         "Last name is too long." );
      FC_ASSERT( fc::is_utf8( last_name ), 
         "Last name is not formatted in UTF8." );

      FC_ASSERT( gender.size() < MAX_STRING_LENGTH,
         "Gender is too long." );
      FC_ASSERT( fc::is_utf8( gender ), 
         "Gender is not formatted in UTF8." );

      FC_ASSERT( date_of_birth.size() < MAX_STRING_LENGTH,
         "Date of Birth is too long." );
      FC_ASSERT( fc::is_utf8( date_of_birth ), 
         "Date of Birth is not formatted in UTF8." );
      
      FC_ASSERT( email.size() < MAX_STRING_LENGTH,
         "Email is too long." );
      FC_ASSERT( fc::is_utf8( email ), 
         "Email is not formatted in UTF8." );

      FC_ASSERT( phone.size() < MAX_STRING_LENGTH,
         "Phone is too long." );
      FC_ASSERT( fc::is_utf8( phone ), 
         "Phone is not formatted in UTF8." );
      
      FC_ASSERT( nationality.size() < MAX_STRING_LENGTH,
         "Nationality is too long." );
      FC_ASSERT( fc::is_utf8( nationality ), 
         "Nationality is not formatted in UTF8." );

      FC_ASSERT( address.size() < MAX_STRING_LENGTH,
         "Address is too long." );
      FC_ASSERT( fc::is_utf8( address ), 
         "Address is not formatted in UTF8." );
   }

   void account_verification_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( verifier_account );
      validate_account_name( verified_account );
      
      FC_ASSERT( shared_image.size() < MAX_STRING_LENGTH,
         "First name is too long." );
      FC_ASSERT( fc::is_utf8( shared_image ), 
         "First name is not formatted in UTF8." );
      FC_ASSERT( shared_image.size() == 46 && shared_image[0] == 'Q' && shared_image[1] == 'm',
         "Image rejected: IPFS string should be 46 characters long and begin with 'Qm'." );
   }

   void account_membership_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( interface );
      switch( membership_type )
      {
         case NONE:
         case STANDARD_MEMBERSHIP:
         case MID_MEMBERSHIP:
         case TOP_MEMBERSHIP:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Membership Type is invalid." );
         }
      }

      FC_ASSERT( account != TEMP_ACCOUNT, 
         "Cannot create membership for temp account." );
      FC_ASSERT( months >= 1 && months <= 120, 
         "Months of membership purchased must be between 1, and 120.", ("m", months ) );
   }

   void account_vote_executive_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( business_account );
      validate_account_name( executive_account );
      switch( role )
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
            FC_ASSERT( false, "Invalid executive officer role" );
         }
         break;
      }
   }

   void account_vote_officer_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( business_account );
      validate_account_name( officer_account );
   }

   void account_member_request_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( business_account );
      FC_ASSERT( message.size() <= MAX_STRING_LENGTH, 
         "Message is too long" );
   }

   void account_member_invite_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( business_account );
      validate_account_name( member );
      FC_ASSERT( account != member, 
         "Account: ${a} cannot invite itself to become a member of a Business account: ${b}.", 
      ("a", member)("b", business_account));
      FC_ASSERT( message.size() <= MAX_STRING_LENGTH,
         "Message is too long." );
      FC_ASSERT( encrypted_business_key.size() <= MAX_STRING_LENGTH,
         "Encrypted key is too long." );
   }

   void account_accept_request_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( business_account );
      validate_account_name( member );
      FC_ASSERT( account != member, 
         "Account: ${a} cannot accept its own join request to a business account: ${b}.", 
         ("a", member)("b", business_account));
   }

   void account_accept_invite_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( business_account );
   }

   void account_remove_member_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( business_account );
      validate_account_name( member );
   }

   void account_update_list_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( listed_account.valid() || listed_asset.valid(),
         "Operation requires either an account or an asset to update list." );
      FC_ASSERT( !( blacklisted && whitelisted ),
         "Operation cannot add item to both blacklist and whitelist." );

      if( listed_account.valid() )
      {
         validate_account_name( *listed_account );
         FC_ASSERT( *listed_account != account,
            "Account cannot add itself to its blacklist or whitelist." );
      }
      if( listed_asset.valid() )
      {
         is_valid_symbol( *listed_asset );
      }
   }

   void account_producer_vote_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( producer );
      FC_ASSERT( vote_rank >= 1 && vote_rank <= 100, 
         "Vote rank must be between zero and one hundred." );
   }

   void account_update_proxy_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      if( proxy.size() )
      {
         validate_account_name( proxy );
      }
      FC_ASSERT( proxy != account, 
         "Cannot proxy to self" );
   }

   void request_account_recovery_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( recovery_account );
      validate_account_name( account_to_recover );
      new_owner_authority.validate();
   }

   void recover_account_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account_to_recover );
      FC_ASSERT( !( new_owner_authority == recent_owner_authority ), 
         "Cannot set new owner authority to the recent owner authority" );
      FC_ASSERT( !new_owner_authority.is_impossible(),
         "New owner authority cannot be impossible" );
      FC_ASSERT( !recent_owner_authority.is_impossible(),
         "Recent owner authority cannot be impossible" );
      FC_ASSERT( new_owner_authority.weight_threshold,
         "New owner authority cannot be trivial" );
      new_owner_authority.validate();
      recent_owner_authority.validate();
   }

   void reset_account_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( reset_account );
      validate_account_name( account_to_reset );
      FC_ASSERT( !new_owner_authority.is_impossible(), 
         "New owner authority cannot be impossible." );
      FC_ASSERT( new_owner_authority.weight_threshold, 
         "New owner authority cannot be trivial." );
      new_owner_authority.validate();
   }

   void set_reset_account_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( new_reset_account );
      FC_ASSERT( days >= 3 && days <= 365, 
         "Reset account delay must be between 3 and 365 days." );
   }

   void change_recovery_account_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account_to_recover );
      validate_account_name( new_recovery_account );
   }

   void decline_voting_rights_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );
   }

   void connection_request_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( requested_account );

      switch( connection_type )
      {
         case CONNECTION:
         case FRIEND:
         case COMPANION:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Connection Type is invalid." );
         }
      }

      FC_ASSERT( account != requested_account, 
         "Account cannot connect with itself." );
      FC_ASSERT( message.size() <= MAX_STRING_LENGTH, 
         "Message is too long." );
   }

   void connection_accept_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( requesting_account );
      FC_ASSERT( account != requesting_account, 
         "Account cannot connect with itself." );
      FC_ASSERT( connection_id.size() <= MAX_STRING_LENGTH,
         "Connection ID is too long." );
      validate_uuidv4( connection_id );

      switch( connection_type )
      {
         case CONNECTION:
         case FRIEND:
         case COMPANION:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Connection Type is invalid." );
         }
      }

      FC_ASSERT( encrypted_key.size() <= MAX_STRING_LENGTH,
         "Encrypted Key is too long." );
   }

   void account_follow_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( follower );
      validate_account_name( following );
      validate_account_name( interface );
      FC_ASSERT( follower != following, 
      "Account cannot follow itself." );
   }

   void tag_follow_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( follower );
      validate_tag_name( tag );
      validate_account_name( interface );
   }

   void activity_reward_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( interface );
      validate_permlink( permlink );
   }


   //===========================//
   // === Network Operations ===//
   //===========================//


   void update_network_officer_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      switch( officer_type )
      {
         case DEVELOPMENT:
         case MARKETING:
         case ADVOCACY:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Officer Type is invalid." );
         }
      }

      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }
      
      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ), 
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ), 
            "JSON Metadata not valid JSON." );
      }
   }

   void network_officer_vote_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( network_officer );
      FC_ASSERT( vote_rank >= 1 && vote_rank <= 100, 
         "Vote rank must be between zero and one hundred." );
   }

   void update_executive_board_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( executive );

      FC_ASSERT( url.size() + details.size() + json.size(),
         "Cannot update Executive board because it does not contain content." );
      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }
      
      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ), 
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ), 
            "JSON Metadata not valid JSON." );
      }

      FC_ASSERT( budget.amount > 0,
         "Budget required." );
      FC_ASSERT( is_valid_symbol( budget.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", budget.symbol) );
      FC_ASSERT( budget.symbol == SYMBOL_CREDIT, 
         "Executive Budget must be in the network credit asset." );
   }

   void executive_board_vote_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( executive_board );
      FC_ASSERT( vote_rank >= 1 && vote_rank <= 100,
         "Vote rank must be between zero and one hundred." );
   }

   void update_governance_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      FC_ASSERT( url.size() + details.size() + json.size(), 
         "Cannot update governance account because it does not contain content." );
      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ), 
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ), 
            "JSON Metadata not valid JSON." );
      }
   }

   void subscribe_governance_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( governance_account );
   }

   void update_supernode_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      FC_ASSERT( url.size() + details.size() + json.size() + node_api_endpoint.size(), 
         "Cannot update supernode because it does not contain sufficient content." );
      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }
      FC_ASSERT( node_api_endpoint.size() < MAX_URL_LENGTH,
         "Node API endpoint is too long." );
      if( node_api_endpoint.size() > 0 )
      {
         validate_url( node_api_endpoint );
      }
      FC_ASSERT( notification_api_endpoint.size() < MAX_URL_LENGTH,
         "Notification API endpoint is too long." );
      if( notification_api_endpoint.size() > 0 )
      {
         validate_url( notification_api_endpoint );
      }
      FC_ASSERT( auth_api_endpoint.size() < MAX_URL_LENGTH,
         "Auth API endpoint is too long." );
      if( auth_api_endpoint.size() > 0 )
      {
         validate_url( auth_api_endpoint );
      }
      FC_ASSERT( ipfs_endpoint.size() < MAX_URL_LENGTH,
         "IPFS endpoint is too long." );
      if( ipfs_endpoint.size() > 0 )
      {
         validate_url( ipfs_endpoint );
      }
      FC_ASSERT( bittorrent_endpoint.size() < MAX_URL_LENGTH,
         "Bittorrent endpoint is too long." );
      if( bittorrent_endpoint.size() > 0 )
      {
         validate_url( bittorrent_endpoint );
      }
      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ), 
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ), 
            "JSON Metadata not valid JSON." );
      }
   }

   void update_interface_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      FC_ASSERT( url.size() + details.size() + json.size(),
         "Cannot update Interface because it does not contain content." );
      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ), 
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ), 
            "JSON Metadata not valid JSON." );
      }
   }

   void update_mediator_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      FC_ASSERT( url.size() + details.size() + json.size(),
         "Cannot update Interface because it does not contain content." );
      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ), 
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ), 
            "JSON Metadata not valid JSON." );
      }

      FC_ASSERT( mediator_bond.symbol == SYMBOL_COIN,
         "Mediation bond must be denominated in core asset." );
      FC_ASSERT( mediator_bond.amount >= BLOCKCHAIN_PRECISION,
         "Mediation bond must be at least 1 unit of core asset." );
   }

   void create_community_enterprise_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( creator );
      FC_ASSERT( enterprise_id.size() < MAX_STRING_LENGTH,
         "Enterprise ID is too long." );
      validate_uuidv4( enterprise_id );
      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }
      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON." );
      }

      FC_ASSERT( begin > GENESIS_TIME,
         "Begin time must be after genesis time." );

      switch( proposal_type )
      {
         case FUNDING:
         {
            FC_ASSERT( beneficiaries.size() >= 1 && beneficiaries.size() <= 100, 
            "Funding Proposal must have between 1 and 100 beneficiaries." );
         }
         break;
         case COMPETITION:
         {

         }
         break;
         case INVESTMENT:
         {
            FC_ASSERT( beneficiaries.size() == 0, 
               "Investment Proposal should not specify account beneficiaries." );
            FC_ASSERT( investment.valid(), 
               "Investment proposal should specify an asset to invest in." );
            FC_ASSERT( is_valid_symbol( *investment ), 
               "Invalid investment symbol." );
         }
         break;
         default:
         {
            FC_ASSERT( false, "Invalid proposal type." );
         }
      }

      for( auto name : beneficiaries )
      {
         validate_account_name( name.first );
         FC_ASSERT( name.second <= PERCENT_100,
            "Beneficiary percent is too large." );
      }

      FC_ASSERT( milestones.size() >= 2, 
         "Proposal must have at least 2 milestones." );

      for( auto mile : milestones )
      {
         FC_ASSERT( mile.first.size() < MAX_STRING_LENGTH,
            "Milestone is too long" );
         FC_ASSERT( mile.second <= PERCENT_100,
            "Milestone percent is too large." );
      }

      FC_ASSERT( duration <= 3650,
         "Duration is too long." );
      FC_ASSERT( duration > 0,
         "Duration must be at least one day." );
      FC_ASSERT( daily_budget.amount > 0,
         "Budget budget must be greater than zero." );
      FC_ASSERT( is_valid_symbol( daily_budget.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", daily_budget.symbol) );
      FC_ASSERT( daily_budget.symbol == SYMBOL_COIN,
         "Daily Budget must be in Core Asset." );
      FC_ASSERT( fee.amount > 0,
         "Fee amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( fee.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", fee.symbol) );
      FC_ASSERT( fee.symbol == SYMBOL_COIN, 
         "Fee must be in Core Asset." );
   }

   void claim_enterprise_milestone_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( creator );
      FC_ASSERT( enterprise_id.size() < MAX_STRING_LENGTH,
         "Enterprise ID is too long." );
      validate_uuidv4( enterprise_id );
      FC_ASSERT( milestone <= PERCENT_100,
         "Milestone percent is too large." );
      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details size is too large." );
   }

   void approve_enterprise_milestone_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( creator );
      FC_ASSERT( enterprise_id.size() < MAX_STRING_LENGTH,
         "Enterprise ID is too long." );
      validate_uuidv4( enterprise_id );
      FC_ASSERT( milestone <= PERCENT_100,
         "Milestone percent is too large." );
      FC_ASSERT( vote_rank >= 1 && vote_rank <= 100,
         "Vote rank must be between zero and one hundred." );
   }

   //=====================================//
   // === Post and Comment Operations === //
   //=====================================//


   void comment_options::validate()const
   {
      FC_ASSERT( percent_liquid <= PERCENT_100, 
         "Percent cannot exceed 100%" );
      FC_ASSERT( max_accepted_payout.symbol == SYMBOL_USD, 
         "Max accepted payout must be in USD" );
      FC_ASSERT( max_accepted_payout.amount.value >= 0, 
         "Cannot accept less than 0 payout" );

      switch( post_type )
      {
         case TEXT_POST:
         case IMAGE_POST:
         case VIDEO_POST:
         case LINK_POST:
         case ARTICLE_POST:
         case AUDIO_POST:
         case FILE_POST:
         case POLL_POST:
         case LIVESTREAM_POST:
         case PRODUCT_POST:
         case LIST_POST:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Comment rejected: Invalid post type." );
         }
      }

      switch( reach )
      {
         case NO_FEED:
         case FOLLOW_FEED:
         case MUTUAL_FEED:
         case CONNECTION_FEED:
         case FRIEND_FEED:
         case COMPANION_FEED:
         case COMMUNITY_FEED:
         case TAG_FEED:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Comment rejected: Invalid feed type." );
         }
      }

      switch( rating )
      {
         case FAMILY:
         case GENERAL:
         case ADULT:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Comment rejected: Invalid rating type." );
         }
      }

      uint32_t sum = 0;

      FC_ASSERT( beneficiaries.size(), 
         "Must specify at least one beneficiary" );
      FC_ASSERT( beneficiaries.size() < 128, 
         "Cannot specify more than 127 beneficiaries." ); // Require size serializtion fits in one byte.

      for( size_t i = 1; i < beneficiaries.size(); i++ )
      {
         validate_account_name( beneficiaries[i].account );
         FC_ASSERT( beneficiaries[i].weight <= PERCENT_100, 
            "Cannot allocate more than 100% of rewards to one account" );
         sum += beneficiaries[i].weight;
         FC_ASSERT( sum <= PERCENT_100, 
            "Cannot allocate more than 100% of rewards to a comment" );
         FC_ASSERT( beneficiaries[i - 1] < beneficiaries[i], 
            "Benficiaries must be specified in sorted order (account ascending)" );
      }
   }

   void comment_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( author );
      validate_account_name( interface );
      validate_permlink( parent_permlink );
      validate_permlink( permlink );
      options.validate();

      if( community.size() )
      {
         validate_community_name( community );
      }

      FC_ASSERT( fc::is_utf8( body ),
         "Body not formatted in UTF8" );
      FC_ASSERT( body.size() + json.size() + title.size() + ipfs.size() + magnet.size(),
         "Cannot update comment because it does not contain content." );
      FC_ASSERT( body.size() < MAX_BODY_SIZE,
         "Comment rejected: Body size is too large." );
      FC_ASSERT( title.size() < MAX_STRING_LENGTH,
         "Comment rejected: Title size is too large." );
      FC_ASSERT( language.size() == 2,
         "Comment rejected: Title size is too large." );

      for( string item : ipfs )
      {
         FC_ASSERT( item.size() == 46 && item[0] == 'Q' && item[1] == 'm',
            "Comment rejected: IPFS string should be 46 characters long and begin with 'Qm'." );
      }

      for( auto item : magnet )
      {
         FC_ASSERT( item.size() < MAX_BODY_SIZE,
            "Comment rejected: Magnet size is too large." );
      }

      for( auto item : tags )
      {
         FC_ASSERT( item.size() < MAX_BODY_SIZE,
            "Comment rejected: Magnet size is too large." );
      }
      
      FC_ASSERT( json.size() < MAX_BODY_SIZE, 
         "Comment rejected: JSON size is too large." );

      FC_ASSERT( fc::is_utf8( json ), 
         "Comment rejected: JSON is not valid UTF8" );

      if( parent_author.size() )
      {
         validate_account_name( parent_author );
      }

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::json::is_valid(json), "JSON Metadata not valid JSON" );
      }

      FC_ASSERT( public_key.size() < MAX_STRING_LENGTH,
         "Comment rejected: Title size is too large." );

      if( comment_price.amount > 0 )
      {
         FC_ASSERT( is_valid_symbol( comment_price.symbol ),
            "Symbol ${symbol} is not a valid symbol", ("symbol", comment_price.symbol) );
      }

      if( premium_price.amount > 0 )
      {
         FC_ASSERT( is_valid_symbol( premium_price.symbol ),
            "Symbol ${symbol} is not a valid symbol", ("symbol", comment_price.symbol) );
      }
   }

   void message_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( sender );
      validate_account_name( recipient );
      FC_ASSERT( message.size(), 
         "Message must include a message string." );
      FC_ASSERT( message.size() < MAX_STRING_LENGTH,
         "Message is too long." );
      FC_ASSERT( uuid.size() < MAX_STRING_LENGTH,
         "UUID is too long." );
   }

   void vote_operation::validate() const
   {  
      validate_account_name( signatory );
      validate_account_name( voter );
      validate_account_name( author );
      validate_account_name( interface );
      FC_ASSERT( abs(weight) <= PERCENT_100, 
         "Weight is not a valid percentage (0 - 10000)" );
      validate_permlink( permlink );
   }

   void view_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( viewer );
      validate_account_name( author );
      validate_account_name( interface );
      validate_account_name( supernode );
      validate_permlink( permlink );
   }

   void share_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( sharer );
      validate_account_name( author );
      validate_account_name( interface );
      if( community.valid() )
      {
         validate_community_name( *community );
      }
      if( tag.valid() )
      {
         validate_tag_name( *tag );
      }
      validate_permlink( permlink );
   }

   void moderation_tag_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( moderator );
      validate_account_name( author );
      validate_permlink( permlink );
      for( auto tag : tags )
      {
         validate_tag_name( tag );
      }

      switch( rating )
      {
         case FAMILY:
         case GENERAL:
         case ADULT:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Invalid rating type." );
         }
      }
      FC_ASSERT( details.size(), 
         "Moderation tag must include details explaining the rule or standard violation." );

      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
   }


   //==========================//
   // === Community Operations === //
   //==========================//


   void community_create_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( founder );
      validate_community_name( name );

      switch( community_privacy )
      {
         case OPEN_PUBLIC_COMMUNITY:
         case GENERAL_PUBLIC_COMMUNITY:
         case EXCLUSIVE_PUBLIC_COMMUNITY:
         case CLOSED_PUBLIC_COMMUNITY:
         case OPEN_PRIVATE_COMMUNITY:
         case GENERAL_PRIVATE_COMMUNITY:
         case EXCLUSIVE_PRIVATE_COMMUNITY:
         case CLOSED_PRIVATE_COMMUNITY:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Invalid community privacy." );
         }
      }

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON." );
      }

      if( json_private.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json_private ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json_private ),
            "JSON Metadata not valid JSON." );
      }

      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }
      FC_ASSERT( community_public_key.size() < MAX_STRING_LENGTH,
         "Community public key is too long." );
   }

   void community_update_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_community_name( community );

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON." );
      }
      
      if( json_private.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json_private ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json_private ),
            "JSON Metadata not valid JSON." );
      }

      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }
      validate_public_key( community_public_key );
   }

   void community_add_mod_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( moderator );
      validate_community_name( community );
   }

   void community_add_admin_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( admin );
      validate_community_name( community );
   }

   void community_vote_mod_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( moderator );
      validate_community_name( community );
      FC_ASSERT( vote_rank >= 1 && vote_rank <= 100,
         "Vote rank must be between zero and one hundred." );
   }

   void community_transfer_ownership_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( new_founder);
      validate_community_name( community );
   }

   void community_join_request_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_community_name( community );
      FC_ASSERT( message.size() < MAX_STRING_LENGTH,
         "Message is too long." );
   }

   void community_join_invite_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( member );
      validate_community_name( community );
      FC_ASSERT( account != member,
      "Account: ${a} cannot invite itself to join a community: ${b} .",
       ("a", member)("b", community));
      FC_ASSERT( message.size() < MAX_STRING_LENGTH,
         "Message is too long." );
      FC_ASSERT( encrypted_community_key.size() < MAX_STRING_LENGTH,
         "Message is too long." );
   }

   void community_join_accept_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( member );
      validate_community_name( community );
      FC_ASSERT( encrypted_community_key.size() < MAX_STRING_LENGTH,
         "Message is too long." );
      FC_ASSERT( account != member, 
         "Account: ${a} cannot accept its own join request to a community: ${b}.", 
      ("a", member)("b", community));
   }

   void community_invite_accept_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_community_name( community );
   }

   void community_remove_member_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( member );
      validate_community_name( community );
   }

   void community_blacklist_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( member );
      validate_community_name( community );
      FC_ASSERT( account != member, 
         "Account: ${a} cannot add or remove itself from the blacklist of community: ${b} .",
         ("a", member)("b", community));
   }

   void community_subscribe_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( interface );
      validate_community_name( community );
   }

   void community_event_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_community_name( community );

      FC_ASSERT( event_name.size() < MAX_STRING_LENGTH,
         "Event Name is too long." );
      FC_ASSERT( fc::is_utf8( event_name ),
         "Event Name is not formatted in UTF8." );

      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ),
         "Details is not formatted in UTF8." );

      FC_ASSERT( location.size() < MAX_STRING_LENGTH,
         "Location is too long." );
      FC_ASSERT( fc::is_utf8( location ),
         "Location is not formatted in UTF8." );

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON." );
      }

      for( auto a : invited )
      {
         validate_account_name( a );
      }

      FC_ASSERT( event_start_time > GENESIS_TIME,
         "Begin time must be after genesis time." );
      FC_ASSERT( event_end_time > GENESIS_TIME,
         "Begin time must be after genesis time." );
      FC_ASSERT( event_end_time > event_start_time,
         "Begin time must be after genesis time." );
   }

   void community_event_attend_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_community_name( community );

      FC_ASSERT( event_name.size() < MAX_STRING_LENGTH,
         "Event Name is too long." );
      FC_ASSERT( fc::is_utf8( event_name ),
         "Event Name is not formatted in UTF8." );
   }



   //================================//
   // === Advertising Operations === //
   //================================//



   void ad_creative_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( author );

      switch( format_type )
      {
         case STANDARD_FORMAT:
         case PREMIUM_FORMAT:
         case PRODUCT_FORMAT:
         case LINK_FORMAT:
         case COMMUNITY_FORMAT:
         case ACCOUNT_FORMAT:
         case ASSET_FORMAT:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Invalid ad format type." );
         }
      }

      FC_ASSERT( creative_id.size(),
         "Creative has no creative ID." );
      FC_ASSERT( creative_id.size() <= MAX_STRING_LENGTH,
         "Creative ID is too long." );
      FC_ASSERT( fc::is_utf8( creative_id ),
         "Creative ID must be UTF-8" );

      validate_uuidv4( creative_id );

      FC_ASSERT( objective.size(),
         "Creative has no objective." );
      FC_ASSERT( objective.size() <= MAX_STRING_LENGTH,
         "Objective is too long." );
      FC_ASSERT( fc::is_utf8( objective ),
         "Objective must be UTF-8" );

      FC_ASSERT( creative.size(),
         "Creative has no content." );
      FC_ASSERT( creative.size() <= MAX_STRING_LENGTH,
         "Creative is too long." );
      FC_ASSERT( fc::is_utf8( creative ),
         "Creative must be UTF-8" );

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON." );
      }
   }


   void ad_campaign_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( interface );
      FC_ASSERT( campaign_id.size() < MAX_STRING_LENGTH,
         "Campaign ID is too long." );
      FC_ASSERT( campaign_id.size() > 0,
         "Campaign has no campaign ID." );
      FC_ASSERT( budget.amount >= 0,
         "Campaign requires a budget greater than or equal to 0." );
         
      validate_uuidv4( campaign_id );

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON." );
      }

      FC_ASSERT( begin > GENESIS_TIME,
         "Begin time must be after genesis time." );
      FC_ASSERT( end > GENESIS_TIME,
         "Begin time must be after genesis time." );
      FC_ASSERT( end > begin,
         "Begin time must be after genesis time." );
      FC_ASSERT( budget.amount > 0,
         "Budget required." );
      FC_ASSERT( is_valid_symbol( budget.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", budget.symbol) );

      for( auto name : agents )
      {
         validate_account_name( name );
      }
   }


   void ad_inventory_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( provider );
      FC_ASSERT( inventory_id.size() > 0,
         "Inventory has no inventory ID." );
      FC_ASSERT( fc::is_utf8( inventory_id ),
         "inventory ID must be UTF-8" );
      FC_ASSERT( min_price.amount > 0,
         "Minimum inventory price must be greater than 0");
      FC_ASSERT( inventory_id.size() < MAX_STRING_LENGTH,
         "Inventory ID is too long." );
      FC_ASSERT( is_valid_symbol( min_price.symbol ),
         "Symbol ${symbol} is not a valid symbol", ( "symbol", min_price.symbol ) );
      FC_ASSERT( inventory > 0,
         "Inventory must be greater than zero." );
      validate_uuidv4( inventory_id );
   
      switch( metric )
      {
         case VIEW_METRIC:
         case VOTE_METRIC:
         case SHARE_METRIC:
         case FOLLOW_METRIC:
         case PURCHASE_METRIC:
         case PREMIUM_METRIC:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Invalid metric type.");
         }
      }
      
      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON." );
      }

      FC_ASSERT( audience_id.size() < MAX_STRING_LENGTH,
         "Audience ID is too long." );
      validate_uuidv4( audience_id );
   }


   void ad_audience_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( audience_id.size(),
         "Audience has no audience ID." );
      FC_ASSERT( audience_id.size() <= MAX_STRING_LENGTH,
         "Audience ID is too long." );
      FC_ASSERT( fc::is_utf8( audience_id ),
         "Audience ID must be UTF-8" );
      FC_ASSERT( audience.size(),
         "Audience must include at least one account." );
      validate_uuidv4( audience_id );

      for( auto name : audience )
      {
         validate_account_name( name );
      }

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON." );
      }
   }


   void ad_bid_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( bidder );
      validate_account_name( account );
      validate_account_name( provider );

      FC_ASSERT( bid_id.size(),
         "bid has no bid ID." );
      FC_ASSERT( bid_id.size() <= MAX_STRING_LENGTH,
         "bid ID is too long." );
      FC_ASSERT( fc::is_utf8( bid_id ),
         "bid ID must be UTF-8" );
      validate_uuidv4( bid_id );

      FC_ASSERT( campaign_id.size(),
         "campaign has no campaign ID." );
      FC_ASSERT( campaign_id.size() <= MAX_STRING_LENGTH,
         "campaign ID is too long." );
      FC_ASSERT( fc::is_utf8( campaign_id ),
         "campaign ID must be UTF-8" );
      validate_uuidv4( campaign_id );

      FC_ASSERT( creative_id.size(),
         "campaign has no creative ID." );
      FC_ASSERT( creative_id.size() <= MAX_STRING_LENGTH,
         "creative ID is too long." );
      FC_ASSERT( fc::is_utf8( creative_id ),
         "creative ID must be UTF-8" );
      validate_uuidv4( creative_id );

      FC_ASSERT( inventory_id.size(),
         "inventory has no inventory ID." );
      FC_ASSERT( inventory_id.size() <= MAX_STRING_LENGTH,
         "inventory ID is too long." );
      FC_ASSERT( fc::is_utf8( inventory_id ),
         "inventory ID must be UTF-8" );
      validate_uuidv4( inventory_id );

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON." );
      }

      FC_ASSERT( bid_price.amount > 0,
         "Minimum bid price must be greater than 0." );
      FC_ASSERT( requested > 0,
         "Inventory requested must be greater than 0." );

      FC_ASSERT( bid_price.amount > 0,
         "Bid price must be greater than zero." );
      FC_ASSERT( is_valid_symbol( bid_price.symbol ),
         "Symbol ${symbol} is not a valid symbol", ( "symbol", bid_price.symbol ) );
      FC_ASSERT( expiration > GENESIS_TIME,
         "Begin time must be after genesis time." );

      for( auto aud : included_audiences )
      {
         FC_ASSERT( aud.size() < MAX_STRING_LENGTH,
            "Audience ID is too long." );
      }

      for( auto aud : excluded_audiences )
      {
         FC_ASSERT( aud.size() < MAX_STRING_LENGTH,
            "Audience ID is too long." );
      }
   }



   //==========================//
   //==== Graph Operations ====//
   //==========================//



   void graph_node_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( interface );
      validate_public_key( node_public_key );

      for( auto n : node_types )
      {
         validate_account_name( n );
      }
      
      FC_ASSERT( node_id.size(),
         "Node has no Node ID." );
      FC_ASSERT( node_id.size() <= MAX_STRING_LENGTH,
         "Node ID is too long." );
      FC_ASSERT( fc::is_utf8( node_id ),
         "Node ID must be UTF-8" );
      validate_uuidv4( node_id );

      FC_ASSERT( name.size() < MAX_STRING_LENGTH,
         "Name is too long." );
      FC_ASSERT( fc::is_utf8( name ),
         "Name is not formatted in UTF8." );

      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ),
         "Details are not formatted in UTF8." );

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON." );
      }

      if( json_private.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json_private ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json_private ),
            "JSON Metadata not valid JSON." );
      }
   }


   void graph_edge_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( from_node_account );
      validate_account_name( to_node_account );
      validate_account_name( interface );
      validate_public_key( edge_public_key );

      for( auto n : edge_types )
      {
         validate_account_name( n );
      }
      
      FC_ASSERT( edge_id.size(),
         "Edge has no Edge ID." );
      FC_ASSERT( edge_id.size() <= MAX_STRING_LENGTH,
         "Edge ID is too long." );
      FC_ASSERT( fc::is_utf8( edge_id ),
         "Edge ID must be UTF-8" );
      validate_uuidv4( edge_id );

      FC_ASSERT( from_node_id.size(),
         "Edge has no Edge ID." );
      FC_ASSERT( from_node_id.size() <= MAX_STRING_LENGTH,
         "Edge ID is too long." );
      FC_ASSERT( fc::is_utf8( from_node_id ),
         "Edge ID must be UTF-8" );
      validate_uuidv4( from_node_id );

      FC_ASSERT( to_node_id.size(),
         "Edge has no Edge ID." );
      FC_ASSERT( to_node_id.size() <= MAX_STRING_LENGTH,
         "Edge ID is too long." );
      FC_ASSERT( fc::is_utf8( to_node_id ),
         "Edge ID must be UTF-8" );
      validate_uuidv4( to_node_id );

      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ),
         "Details are not formatted in UTF8." );

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON." );
      }

      if( json_private.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json_private ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json_private ),
            "JSON Metadata not valid JSON." );
      }
   }

   void graph_node_property_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( interface );
      validate_account_name( node_type );

      switch( graph_privacy )
      {
         case CONNECTION:
         case FRIEND:
         case COMPANION:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Connection Type is invalid." );
         }
      }

      switch( edge_permission )
      {
         case CONNECTION:
         case FRIEND:
         case COMPANION:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Connection Type is invalid." );
         }
      }

      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ),
         "Details are not formatted in UTF8." );

      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );

      for( auto a : attributes )
      {
         FC_ASSERT( a.size() < MAX_URL_LENGTH,
            "Attribute is too long." );
         FC_ASSERT( fc::is_utf8( a ),
            "Attribute is not formatted in UTF8." );
      }
   }

   void graph_edge_property_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( interface );
      validate_account_name( edge_type );

      switch( graph_privacy )
      {
         case CONNECTION:
         case FRIEND:
         case COMPANION:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Connection Type is invalid." );
         }
      }

      for( auto a : from_node_types )
      {
         validate_account_name( a );
      }

      for( auto a : to_node_types )
      {
         validate_account_name( a );
      }

      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ),
         "Details are not formatted in UTF8." );

      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );

      for( auto a : attributes )
      {
         FC_ASSERT( a.size() < MAX_URL_LENGTH,
            "Attribute is too long." );
         FC_ASSERT( fc::is_utf8( a ),
            "Attribute is not formatted in UTF8." );
      }
   }


   //=============================//
   // === Transfer Operations === //
   //=============================//



   void transfer_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( amount.amount > 0,
         "INVALID TRANSFER: NEGATIVE AMOUNT - THEFT NOT PERMITTED." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE,
         "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ),
         "Memo is not UTF8" );
   }


   void transfer_request_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( amount.amount > 0, 
         "INVALID TRANSFER: NEGATIVE AMOUNT - THEFT NOT PERMITTED." );
      FC_ASSERT( is_valid_symbol(amount.symbol), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE,
         "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ), 
         "Memo is not UTF8" );
      FC_ASSERT( request_id.size() < MAX_STRING_LENGTH,
         "Request ID is too long." );
      FC_ASSERT( request_id.size() > 0,
         "Request ID is required." );
      validate_uuidv4( request_id );
      FC_ASSERT( expiration > GENESIS_TIME,
         "Expiration time must be after genesis time." );
   }


   void transfer_accept_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( request_id.size() < MAX_STRING_LENGTH,
         "Request ID is too long." );
      FC_ASSERT( request_id.size() > 0,
         "Request ID is required." );
      validate_uuidv4( request_id );
   }


   void transfer_recurring_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( transfer_id.size() < MAX_STRING_LENGTH,
         "Transfer ID is too long." );
      FC_ASSERT( transfer_id.size() > 0,
         "Transfer ID is required." );
      validate_uuidv4( transfer_id );
      FC_ASSERT( amount.amount > 0,
         "INVALID TRANSFER: NEGATIVE AMOUNT - THEFT NOT PERMITTED." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol ) );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE,
         "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ),
         "Memo is not UTF8" );
      FC_ASSERT( begin > GENESIS_TIME,
         "Begin time must be after genesis time." );
      FC_ASSERT( interval > fc::hours(1),
         "Interval time must be at least one hour." );
      FC_ASSERT( payments > 0,
         "Payments must be greater than 1." );
   }


   void transfer_recurring_request_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( request_id.size() < MAX_STRING_LENGTH,
         "Request ID is too long." );
      FC_ASSERT( request_id.size() > 0,
         "Request ID is required." );
      FC_ASSERT( fc::is_utf8( request_id ),
         "Request ID is not UTF8" );
      validate_uuidv4( request_id );
      FC_ASSERT( amount.amount > 0,
         "INVALID TRANSFER: NEGATIVE AMOUNT - THEFT NOT PERMITTED." );
      FC_ASSERT( is_valid_symbol(amount.symbol),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE,
         "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ),
         "Memo is not UTF8" );
      FC_ASSERT( begin > GENESIS_TIME,
         "Begin time must be after genesis time." );
      FC_ASSERT( interval > fc::minutes(1),
         "Interval time must be at least one minute." );
      FC_ASSERT( expiration > GENESIS_TIME,
         "Expiration time must be after genesis time." );
   }

   void transfer_recurring_accept_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( request_id.size() < MAX_STRING_LENGTH,
         "Request ID is too long." );
      FC_ASSERT( request_id.size() > 0,
         "Request ID is required." );
      FC_ASSERT( fc::is_utf8( request_id ),
         "Request ID is not UTF8" );
      validate_uuidv4( request_id );
   }


   //============================//
   // === Balance Operations === //
   //============================//


   void claim_reward_balance_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      FC_ASSERT( reward.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( reward.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", reward.symbol) );
   }

   void stake_asset_operation::validate() const
   {
      validate_account_name( from );
      if( to != account_name_type() )
      {
         validate_account_name( to );
      } 
      FC_ASSERT( amount.amount > 0,
         "Must transfer a nonzero amount" );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
   }

   void unstake_asset_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( from );
      if ( to != account_name_type() )
      {
         validate_account_name( to );
      }
      FC_ASSERT( is_valid_symbol(amount.symbol),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( amount.amount >= 0,
         "Cannot withdraw negative stake. Account: ${account}, From:${amount}", ("from", from)("amount", amount) );
   }

   void unstake_asset_route_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( 0 < percent && percent <= PERCENT_100,
         "Percent must be valid percent (1 - 10000)" );
   }

   void transfer_to_savings_operation::validate()const 
   {
      validate_account_name( signatory );
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( amount.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol(amount.symbol),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE,
         "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ),
         "Memo is not UTF8" );
   }

   void transfer_from_savings_operation::validate()const 
   {
      validate_account_name( signatory );
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( request_id.size() < MAX_STRING_LENGTH,
         "Request ID is too long." );
      FC_ASSERT( request_id.size() > 0,
         "Request ID is too long." );
      FC_ASSERT( fc::is_utf8( request_id ), 
         "Request ID is not UTF8" );
      validate_uuidv4( request_id );
      FC_ASSERT( amount.amount > 0, 
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE,
         "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ),
         "Memo is not UTF8" );
   }

   void delegate_asset_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( delegator );
      validate_account_name( delegatee );
      FC_ASSERT( delegator != delegatee,
         "You cannot delegate to yourself" );
      FC_ASSERT( amount.amount >= 0,
         "Delegation must be greater than zero." );
      FC_ASSERT( is_valid_symbol(amount.symbol),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
   }


   //================================//
   // === Marketplace Operations === //
   //================================//



   void product_update_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      
      FC_ASSERT( product_name.size() < MAX_STRING_LENGTH,
         "Product name is too long" );
      FC_ASSERT( fc::is_utf8( product_name ),
         "Product Name is not UTF8" );

      switch( sale_type )
      {
         case FIXED_PRICE_SALE:
         case WHOLESALE_PRICE_SALE:
         case OPEN_AUCTION_SALE:
         case REVERSE_AUCTION_SALE:
         case SECRET_AUCTION_SALE:
         {
            break;
         }
         default:
         {
            FC_ASSERT( false, "Connection Type is invalid." );
         }
      }

      for( auto a : product_variants )
      {
         FC_ASSERT( a.size() < MAX_STRING_LENGTH,
            "Product variant is too long" );
         FC_ASSERT( fc::is_utf8( a ),
            "Product variant is not UTF8" );
      }

      for( auto a : details )
      {
         FC_ASSERT( a.first.size() < MAX_STRING_LENGTH,
            "Product variant is too long" );
         FC_ASSERT( fc::is_utf8( a.first ),
            "Product variant is not UTF8" );
         FC_ASSERT( a.second.size() < MAX_STRING_LENGTH,
            "Product details is too long" );
         FC_ASSERT( fc::is_utf8( a.second ),
            "Product details is not UTF8" );
      }

      for( auto a : images )
      {
         FC_ASSERT( a.first.size() < MAX_STRING_LENGTH,
            "Product variant is too long" );
         FC_ASSERT( fc::is_utf8( a.first ),
            "Product variant is not UTF8" );
         FC_ASSERT( a.second.size() < MAX_STRING_LENGTH,
            "Image is too long" );
         FC_ASSERT( fc::is_utf8( a.second ),
            "Image is not UTF8" );
         FC_ASSERT( a.second.size() == 46 && a.second[0] == 'Q' && a.second[1] == 'm',
            "Image IPFS string should be 46 characters long and begin with 'Qm'." );
      }

      for( auto a : product_prices )
      {
         FC_ASSERT( a.first.size() < MAX_STRING_LENGTH,
            "Product variant is too long" );
         FC_ASSERT( fc::is_utf8( a.first ),
            "Product variant is not UTF8" );
         FC_ASSERT( is_valid_symbol( a.second.symbol ),
            "Product Price symbol is not valid symbol" );
         FC_ASSERT( a.second.amount >= 0,
            "Product Price must be positive amount" );
      }

      for( auto a : stock_available )
      {
         FC_ASSERT( a.first.size() < MAX_STRING_LENGTH,
            "Product variant is too long" );
         FC_ASSERT( fc::is_utf8( a.first ),
            "Product variant is not UTF8" );
         FC_ASSERT( a.second >= 0,
            "Stock available must be positive amount" );
      }

      for( auto a : json )
      {
         FC_ASSERT( a.first.size() < MAX_STRING_LENGTH,
            "Product variant is too long" );
         FC_ASSERT( fc::is_utf8( a.first ),
            "Product variant is not UTF8" );
         FC_ASSERT( fc::is_utf8( a.second ),
            "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid( a.second ),
            "JSON Metadata not valid JSON" );
      }

      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long" );
      FC_ASSERT( fc::is_utf8( product_name ),
         "URL is not UTF8" );

      for( auto a : delivery_variants )
      {
         FC_ASSERT( a.size() < MAX_STRING_LENGTH,
            "Delivery variant is too long" );
         FC_ASSERT( fc::is_utf8( a ),
            "Delivery variant is not UTF8" );
      }

      for( auto a : delivery_prices )
      {
         FC_ASSERT( a.first.size() < MAX_STRING_LENGTH,
            "Product variant is too long" );
         FC_ASSERT( fc::is_utf8( a.first ),
            "Product variant is not UTF8" );
         FC_ASSERT( is_valid_symbol( a.second.symbol ),
            "Delivery Price symbol is not valid symbol" );
         FC_ASSERT( a.second.amount >= 0,
            "Delivery Price must be positive amount" );
      }
   }


   void product_purchase_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      
      FC_ASSERT( product_name.size() < MAX_STRING_LENGTH,
         "Product name is too long" );
      FC_ASSERT( fc::is_utf8( product_name ),
         "Product Name is not UTF8" );

      for( auto a : order_size )
      {
         FC_ASSERT( a.first.size() < MAX_STRING_LENGTH,
            "Product variant is too long" );
         FC_ASSERT( fc::is_utf8( a.first ),
            "Product variant is not UTF8" );
         FC_ASSERT( a.second >= 0,
            "Order Size must be positive amount" );
      }

      FC_ASSERT( is_valid_symbol( total_payable.symbol ),
         "Product Price symbol is not valid symbol" );
      FC_ASSERT( total_payable.amount >= 0,
         "Product Price must be positive amount" );

      FC_ASSERT( shipping_address.size() < MAX_STRING_LENGTH,
         "Shipping Address is too long" );
      FC_ASSERT( fc::is_utf8( shipping_address ),
         "Shipping Address is not UTF8" );

      FC_ASSERT( escrow_id.size() < MAX_STRING_LENGTH,
         "Escrow ID is too long." );
      FC_ASSERT( escrow_id.size() > 0,
         "Escrow ID is required." );
      FC_ASSERT( fc::is_utf8( escrow_id ), 
         "Escrow ID is not UTF8" );
      validate_uuidv4( escrow_id );

      FC_ASSERT( acceptance_time > GENESIS_TIME,
         "Acceptance time must be after genesis time." );
      FC_ASSERT( escrow_expiration > GENESIS_TIME,
         "Escrow expiration must be after genesis time." );
      FC_ASSERT( acceptance_time < escrow_expiration,
         "Acceptance time must be before escrow expiration" );

      FC_ASSERT( memo.size() < MAX_MEMO_SIZE,
         "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ),
         "Memo is not UTF8" );

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8(json),
            "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid(json),
            "JSON Metadata not valid JSON" );
      }
   }


   void escrow_transfer_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( from );
      validate_account_name( to );
      
      FC_ASSERT( is_valid_symbol(amount.symbol),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( amount.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( from != to,
         "From and To must not be the same account" );
      FC_ASSERT( acceptance_time > GENESIS_TIME,
         "Acceptance time must be after genesis time." );
      FC_ASSERT( escrow_expiration > GENESIS_TIME,
         "Escrow expiration must be after genesis time." );
      FC_ASSERT( acceptance_time < escrow_expiration,
         "Acceptance time must be before escrow expiration" );

      FC_ASSERT( escrow_id.size() < MAX_STRING_LENGTH,
         "Escrow ID is too long." );
      FC_ASSERT( escrow_id.size() > 0,
         "Escrow ID is required." );
      FC_ASSERT( fc::is_utf8( escrow_id ), 
         "Escrow ID is not UTF8" );
      validate_uuidv4( escrow_id );

      FC_ASSERT( memo.size() < MAX_MEMO_SIZE,
         "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ),
         "Memo is not UTF8" );

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8(json),
            "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid(json),
            "JSON Metadata not valid JSON" );
      }
   }

   void escrow_approve_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( mediator );
      validate_account_name( escrow_from );

      FC_ASSERT( mediator != account,
         "Mediator and Account must not be the same account" );
      FC_ASSERT( mediator != escrow_from,
         "Mediator and Escrow From must not be the same account" );
      
      FC_ASSERT( escrow_id.size() < MAX_STRING_LENGTH,
         "Escrow ID is too long." );
      FC_ASSERT( escrow_id.size() > 0,
         "Escrow ID is required." );
      FC_ASSERT( fc::is_utf8( escrow_id ), 
         "Escrow ID is not UTF8" );
      validate_uuidv4( escrow_id );
   }

   void escrow_dispute_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( escrow_from );
      
      FC_ASSERT( escrow_id.size() < MAX_STRING_LENGTH,
         "Escrow ID is too long." );
      FC_ASSERT( escrow_id.size() > 0,
         "Escrow ID is required." );
      FC_ASSERT( fc::is_utf8( escrow_id ),
         "Escrow ID is not UTF8" );
      validate_uuidv4( escrow_id );
   }

   void escrow_release_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( escrow_from );
      
      FC_ASSERT( release_percent >= 0 && release_percent <= PERCENT_100,
         "Release percent must be between 0 and PERCENT_100." );
         
      FC_ASSERT( escrow_id.size() < MAX_STRING_LENGTH,
         "Escrow ID is too long." );
      FC_ASSERT( escrow_id.size() > 0,
         "Escrow ID is required." );
      FC_ASSERT( fc::is_utf8( escrow_id ),
         "Escrow ID is not UTF8" );
      validate_uuidv4( escrow_id );
   }


   //============================//
   // === Trading Operations === //
   //============================//


   void limit_order_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( interface );
      validate_account_name( owner );

      FC_ASSERT( amount_to_sell.symbol == exchange_rate.base.symbol,
         "Sell asset must be the base of the price" );
      exchange_rate.validate();
      FC_ASSERT( ( amount_to_sell * exchange_rate ).amount > 0,
         "Amount to sell cannot round to 0 when traded" );
      FC_ASSERT( order_id.size() < MAX_STRING_LENGTH,
         "Order ID is too long." );
      FC_ASSERT( order_id.size(),
         "Order ID is required." );
      FC_ASSERT( fc::is_utf8( order_id ),
         "Order ID is not UTF8" );
      validate_uuidv4( order_id );
      FC_ASSERT( expiration > GENESIS_TIME,
         "Expiration time must be greater than genesis time." );
   }

   void margin_order_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( interface );
      validate_account_name( owner );
      
      FC_ASSERT( amount_to_borrow.amount > 0, 
         "Please set a greater than zero amount to borrow and collateral.");
      FC_ASSERT( ( amount_to_borrow * exchange_rate).amount > 0,
         "Amount to sell cannot round to 0 when traded" );
      FC_ASSERT( is_valid_symbol( amount_to_borrow.symbol ),
         "Symbol ${symbol} is not a valid symbol", ( "symbol", amount_to_borrow.symbol ) );
      FC_ASSERT( amount_to_borrow.symbol == exchange_rate.base.symbol,
         "Amount to borrow asset must be the base of the price." );
      FC_ASSERT( collateral.amount > 0,
         "Collateral must be greater than zero." );
      FC_ASSERT( is_valid_symbol( collateral.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", collateral.symbol) );
      FC_ASSERT( order_id.size() < MAX_STRING_LENGTH,
         "Order ID is too long." );
      FC_ASSERT( order_id.size(),
         "Order ID is required." );
      FC_ASSERT( fc::is_utf8( order_id ),
         "Order ID is not UTF8" );
      validate_uuidv4( order_id );
      FC_ASSERT( expiration > GENESIS_TIME,
         "Expiration time must be greater than genesis time." );

      exchange_rate.validate();

      if( stop_loss_price.valid() )
      {
         stop_loss_price->validate();
         FC_ASSERT( *stop_loss_price < exchange_rate,
            "Stop Loss price must be less than the specified exchange rate, or the order will be immediately stop loss liquidated." );
      }
      if( take_profit_price.valid() )
      {
         take_profit_price->validate();
         FC_ASSERT( *take_profit_price > exchange_rate,
            "Target price must be greater than the specified exchange rate, or the order will be immediately take profit liquidated." );
      }
      if( limit_stop_loss_price.valid() )
      {
         limit_stop_loss_price->validate();
         FC_ASSERT( *limit_stop_loss_price < exchange_rate,
            "Limit Stop Loss price must be less than the specified exchange rate, or the order will be immediately stop loss liquidated." );
      }
      if( limit_take_profit_price.valid() )
      {
         limit_take_profit_price->validate();
         FC_ASSERT( *limit_take_profit_price > exchange_rate,
            "Target price must be greater than the specified exchange rate, or the order will be immediately take profit liquidated." );
      }
   }


   void auction_order_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( owner );
      validate_account_name( interface );

      FC_ASSERT( amount_to_sell.symbol == min_exchange_rate.base.symbol,
         "Sell asset must be the base of the price" );
      min_exchange_rate.validate();
      FC_ASSERT( ( amount_to_sell * min_exchange_rate ).amount > 0,
         "Amount to sell cannot round to 0 when traded" );
      FC_ASSERT( order_id.size() < MAX_STRING_LENGTH,
         "Order ID is too long." );
      FC_ASSERT( order_id.size(),
         "Order ID is required." );
      FC_ASSERT( fc::is_utf8( order_id ),
         "Order ID is not UTF8" );
      validate_uuidv4( order_id );
      FC_ASSERT( expiration > GENESIS_TIME,
         "Expiration time must be greater than genesis time." );
   }
   

   void call_order_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( interface );
      validate_account_name( owner );

      FC_ASSERT( is_valid_symbol( collateral.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", collateral.symbol) );
      FC_ASSERT( is_valid_symbol( debt.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", debt.symbol) );

      FC_ASSERT( collateral.amount >= 0,
         "Collateral amount of new debt position should be positive." );
      FC_ASSERT( debt.amount >= 0,
         "Debt amount of new debt position should be positive." );
   }


   void option_order_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( owner );
      validate_account_name( interface );

      FC_ASSERT( amount_to_issue.symbol == strike_price.strike_price.base.symbol,
         "Sell asset must be the base of the strike price" );
      strike_price.validate();
      FC_ASSERT( ( amount_to_issue * strike_price.strike_price ).amount > 0,
         "Amount to issue cannot round to 0 when traded" );
      FC_ASSERT( order_id.size() < MAX_STRING_LENGTH,
         "Order ID is too long." );
      FC_ASSERT( order_id.size(),
         "Order ID is required." );
      FC_ASSERT( fc::is_utf8( order_id ),
         "Order ID is not UTF8" );
      validate_uuidv4( order_id );
   }


   void bid_collateral_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( bidder );

      FC_ASSERT( collateral.amount > 0,
         "Additional Collateral must be greater than zero." );
      FC_ASSERT( is_valid_symbol( collateral.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", collateral.symbol) );
      FC_ASSERT( debt.amount > 0,
         "Debt covered must be greater than zero." );
      FC_ASSERT( is_valid_symbol( debt.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", debt.symbol) );
   }



   //=========================//
   // === Pool Operations === //
   //=========================//



   void liquidity_pool_create_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( first_amount.amount > 0,
         "First Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( first_amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", first_amount.symbol) );
      FC_ASSERT( second_amount.amount > 0,
         "Second Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( second_amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", second_amount.symbol) );
   }

   void liquidity_pool_exchange_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( interface );

      FC_ASSERT( amount.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( is_valid_symbol( receive_asset ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", receive_asset) );

      if( limit_price.valid() )
      {
         limit_price->validate();
      }
   }

   void liquidity_pool_fund_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( is_valid_symbol( pair_asset ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", pair_asset) );
   }

   void liquidity_pool_withdraw_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( is_valid_symbol( receive_asset ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", receive_asset) );
   }

   void credit_pool_collateral_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
   }

   void credit_pool_borrow_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( collateral.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( collateral.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
   }

   void credit_pool_lend_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
   }

   void credit_pool_withdraw_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
   }



   //==========================//
   // === Asset Operations === //
   //==========================//



   void asset_options::validate()const
   {
      FC_ASSERT( max_supply > 0, 
         "Maximum asset supply must be greater than zero." );
      FC_ASSERT( max_supply <= MAX_ASSET_SUPPLY,
         "Max supply is too high." );
      FC_ASSERT( market_fee_percent <= PERCENT_100,
         "Market fee percent must be less than 100%." );
      FC_ASSERT( max_market_fee >= 0 && max_market_fee <= MAX_ASSET_SUPPLY,
         "Max market fee must be between 0 and Max asset supply" );
      FC_ASSERT( stake_intervals >= 0,
         "Stake intervals must be greater than or equal to 0.");
      FC_ASSERT( unstake_intervals >= 0,
         "Unstake intervals must be greater than or equal to 0.");

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ), 
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ), 
            "JSON Metadata not valid JSON." );
      }

      FC_ASSERT( details.size(),
         "Details are required." );
      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }
      
      // There must be no high bits in permissions whose meaning is not known.
      FC_ASSERT( !( issuer_permissions & ~ASSET_ISSUER_PERMISSION_MASK ) );
      // The global_settle flag may never be set (this is a permission only)
      FC_ASSERT( !( flags & global_settle ) );

      if( !whitelist_authorities.empty() || !blacklist_authorities.empty() )
      {
         FC_ASSERT( flags & balance_whitelist );
      }
         
      for( auto item : whitelist_markets )
      {
         FC_ASSERT( blacklist_markets.find(item) == blacklist_markets.end() );
      }
      for( auto item : blacklist_markets )
      {
         FC_ASSERT( whitelist_markets.find(item) == whitelist_markets.end() );
      } 
      FC_ASSERT( minimum_feeds > 0 );
      FC_ASSERT( force_settlement_offset_percent <= PERCENT_100 );
      FC_ASSERT( maximum_force_settlement_volume <= PERCENT_100 );
      FC_ASSERT( feed_lifetime >= MIN_FEED_LIFETIME,
         "Feed lifetime must be greater than network minimum." );
      FC_ASSERT( force_settlement_delay >= MIN_SETTLEMENT_DELAY,
         "Force Settlement delay must be greater than network minimum." );
   }

   void asset_create_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( issuer );

      FC_ASSERT( is_valid_symbol( symbol ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", symbol) );
      FC_ASSERT( is_valid_symbol( coin_liquidity.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", coin_liquidity.symbol) );
      FC_ASSERT( is_valid_symbol( usd_liquidity.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", usd_liquidity.symbol) );
      FC_ASSERT( is_valid_symbol( credit_liquidity.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", credit_liquidity.symbol) );
      FC_ASSERT( credit_liquidity.amount > 0,
         "Credit Liquidity must be greater than zero." );
      FC_ASSERT( coin_liquidity.symbol == SYMBOL_COIN, 
         "Asset must have initial liquidity in the COIN asset." );
      FC_ASSERT( coin_liquidity.amount >= 10 * BLOCKCHAIN_PRECISION, 
         "Asset must have at least 10 COIN asset of initial liquidity." );
      FC_ASSERT( usd_liquidity.symbol == SYMBOL_USD, 
         "Asset must have initial liquidity in the USD asset." );
      FC_ASSERT( usd_liquidity.amount >= 10 * BLOCKCHAIN_PRECISION, 
         "Asset must have at least 10 USD asset of initial liquidity." );

      switch( asset_type )
      {
         case STANDARD_ASSET:
         case CURRENCY_ASSET:
         case EQUITY_ASSET:
         case CREDIT_ASSET:
         case BITASSET_ASSET:
         case GATEWAY_ASSET:
         case UNIQUE_ASSET:
         {
            break;
         }
         case LIQUIDITY_POOL_ASSET:
         {
            FC_ASSERT( false,
               "Cannot directly create a new liquidity pool asset. Please create a liquidity pool between two existing assets." );
         }
         break;
         case CREDIT_POOL_ASSET:
         {
            FC_ASSERT( false, 
               "Cannot directly create a new credit pool asset. Credit pool assets are created in addition to every asset." );
         }
         break;
         case OPTION_ASSET:
         {
            FC_ASSERT( false,
               "Cannot directly create a new option asset. Option assets are issued from an Options market." );
         }
         break;
         case PREDICTION_ASSET:
         {
            FC_ASSERT( false, 
               "Cannot directly create a new prediction asset. Prediction assets are issued from a Prediction market." );
         }
         break;
         default:
         {
            FC_ASSERT( false, "Invalid asset type." );
         }
      }
      
      options.validate();

   }

   void asset_update_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( issuer );

      FC_ASSERT( is_valid_symbol( asset_to_update ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", asset_to_update ) );

      new_options.validate();
   }

   void asset_issue_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( issuer );
      validate_account_name( issue_to_account );
      FC_ASSERT( asset_to_issue.amount.value <= MAX_ASSET_SUPPLY, 
         "Amount to issue must be less than max asset supply." );
      FC_ASSERT( asset_to_issue.amount.value > 0,
         "Amount to issue must be greater than zero." );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE,
         "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ),
         "Memo is not UTF8" );
   }

   void asset_reserve_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( payer );
      FC_ASSERT( amount_to_reserve.amount.value <= MAX_ASSET_SUPPLY,
         "Amount to issue must be less than max asset supply." );
      FC_ASSERT( amount_to_reserve.amount.value > 0,
         "Amount to reserve must be greater than zero." );
   }

   void asset_update_issuer_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( issuer );
      validate_account_name( new_issuer );
      FC_ASSERT( is_valid_symbol( asset_to_update ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", asset_to_update ) );
      FC_ASSERT( issuer != new_issuer, 
         "New issuer must be different from issuer." );
   }

   void asset_update_feed_producers_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( issuer );
      FC_ASSERT( is_valid_symbol( asset_to_update ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", asset_to_update ) );

      for( auto name : new_feed_producers )
      {
         validate_account_name( name );
      }
   }

   void asset_publish_feed_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( publisher );
      FC_ASSERT( is_valid_symbol( symbol ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", symbol) );

      feed.validate();

      FC_ASSERT( !feed.settlement_price.is_null(),
         "Settlement price cannot be null." );
      FC_ASSERT( feed.is_for( symbol ),
         "Price feed must be for symbol." );
   }

   void asset_settle_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      FC_ASSERT( amount.amount >= 0,
         "Amount must be greater than or equal to zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
   }

   void asset_global_settle_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( issuer );
      FC_ASSERT( is_valid_symbol( asset_to_settle ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", asset_to_settle ) );
      FC_ASSERT( asset_to_settle == settle_price.base.symbol,
         "Asset to settle must be the same asset as base fo settlement price." );
   }


   //=====================================//
   // === Block Production Operations === //
   //=====================================//


   void producer_update_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( owner );

      FC_ASSERT( details.size() > 0,
         "producer requires details." );
      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8." );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON." );
      }

      FC_ASSERT( latitude <= 90 && latitude >= -90,
         "Latitude must be between +-90 degrees." );
      FC_ASSERT( longitude <= 180 && longitude >= -180,
         "Latitude must be between +-180 degrees." );
      FC_ASSERT( block_signing_key.size() < MAX_STRING_LENGTH,
         "Block signing key size is too large." );

      props.validate();
   }

   struct proof_of_work_operation_validate_visitor
   {
      typedef void result_type;

      template< typename PowType >
      void operator()( const PowType& pow )const
      {
         pow.validate();
      }
   };

   void proof_of_work_operation::validate()const
   {
      props.validate();
      work.visit( proof_of_work_operation_validate_visitor() );
   };

   struct proof_of_work_operation_get_required_active_visitor
   {
      typedef void result_type;

      proof_of_work_operation_get_required_active_visitor( flat_set< account_name_type >& required_active )
         : _required_active( required_active ) {}

      template< typename PowType >
      void operator()( const PowType& work )const
      {
         _required_active.insert( work.input.miner_account );
      }

      flat_set<account_name_type>& _required_active;
   };

   void proof_of_work_operation::get_required_active_authorities( flat_set<account_name_type>& a )const
   {
      if( !new_owner_key )
      {
         proof_of_work_operation_get_required_active_visitor vtor( a );
         work.visit( vtor );
      }
   }

   void proof_of_work::create( const block_id_type& prev, const account_name_type& account_name, uint64_t n )
   {
      input.miner_account = account_name;
      input.prev_block = prev;
      input.nonce = n;

      auto prv_key = fc::sha256::hash( input );
      auto input = fc::sha256::hash( prv_key );
      auto signature = fc::ecc::private_key::regenerate( prv_key ).sign_compact(input);

      auto sig_hash            = fc::sha256::hash( signature );
      public_key_type recover  = fc::ecc::public_key( signature, sig_hash );

      fc::sha256 work = fc::sha256::hash(std::make_pair(input,recover));
      pow_summary = work.approx_log_32();
   }

   void equihash_proof_of_work::create( const block_id_type& recent_block, const account_name_type& account_name, uint32_t nonce )
   {
      input.miner_account = account_name;
      input.prev_block = recent_block;
      input.nonce = nonce;

      auto seed = fc::sha256::hash( input );
      proof = fc::equihash::proof::hash( EQUIHASH_N, EQUIHASH_K, seed );
      pow_summary = fc::sha256::hash( proof.inputs ).approx_log_32();
   }

   void proof_of_work::validate()const
   {
      validate_account_name( input.miner_account );
      proof_of_work tmp; 
      tmp.create( input.prev_block, input.miner_account, input.nonce );
      FC_ASSERT( pow_summary == tmp.pow_summary, 
         "Reported work does not match calculated work" );
   }

   void equihash_proof_of_work::validate() const
   {
      validate_account_name( input.miner_account );
      auto seed = fc::sha256::hash( input );
      FC_ASSERT( proof.n == EQUIHASH_N,
         "Proof of work 'n' value is incorrect" );
      FC_ASSERT( proof.k == EQUIHASH_K,
         "Proof of work 'k' value is incorrect" );
      FC_ASSERT( proof.seed == seed,
         "Proof of work seed does not match expected seed" );
      FC_ASSERT( proof.is_valid(),
         "Proof of work is not a solution", ("block_id", input.prev_block)("miner_account", input.miner_account)("nonce", input.nonce) );
      FC_ASSERT( pow_summary == fc::sha256::hash( proof.inputs ).approx_log_32() );
   }

   void verify_block_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( producer );
   }

   void commit_block_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( producer );
      FC_ASSERT( commitment_stake.amount >= BLOCKCHAIN_PRECISION,
         "Commitment Stake must be greater than zero." );
      FC_ASSERT( is_valid_symbol( commitment_stake.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", commitment_stake.symbol ) );
      FC_ASSERT( commitment_stake.symbol == SYMBOL_COIN,
         "Commitment Stake must be denominated in the core asset." );
      FC_ASSERT( verifications.size() >= IRREVERSIBLE_THRESHOLD,
         "Insufficient Verifications for commit transaction. Please Include additional transaction IDs." );
   }

   void producer_violation_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( reporter );
   }


   //===========================//
   // === Custom Operations === //
   //===========================//


   void custom_operation::validate() const 
   {
      /// required auth accounts are the ones whose bandwidth is consumed
      FC_ASSERT( required_auths.size() > 0, 
         "At least on account must be specified." );
   }

   void custom_json_operation::validate() const 
   {
      /// required auth accounts are the ones whose bandwidth is consumed
      FC_ASSERT( ( required_auths.size() + required_posting_auths.size() ) > 0,
         "At least on account must be specified." );
      FC_ASSERT( id.size() <= 32,
         "ID is too long." );
      FC_ASSERT( fc::is_utf8(json),
         "JSON Metadata not formatted in UTF8." );
      FC_ASSERT( fc::json::is_valid(json),
         "JSON Metadata not valid JSON." );
   }

} } // node::protocol