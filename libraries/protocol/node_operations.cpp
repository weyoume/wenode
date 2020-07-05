#include <node/protocol/node_operations.hpp>
#include <fc/crypto/base58.hpp>
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

      if( recovery_account.size() )
      {
         validate_account_name( recovery_account );
      }

      if( reset_account.size() )
      {
         validate_account_name( reset_account );
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

      FC_ASSERT( !owner_auth.is_impossible(),
         "Owner Authority is impossible." );
      FC_ASSERT( !active_auth.is_impossible(),
         "Active Authority is impossible." );
      FC_ASSERT( !posting_auth.is_impossible(),
         "Posting Authority is impossible." );

      if( details.size() > 0 )
      {
         FC_ASSERT( details.size() < MAX_STRING_LENGTH,
            "Details are too long." );
         FC_ASSERT( fc::is_utf8( details ), 
            "Details are not formatted in UTF8." );
      }

      if( url.size() > 0 )
      {
         FC_ASSERT( url.size() < MAX_URL_LENGTH,
            "URL is too long." );
         FC_ASSERT( fc::is_utf8( url ),
            "URL is not formatted in UTF8." );
         validate_url( url );
      }
      
      if( image.size() > 0 )
      {
         FC_ASSERT( image.size() < MAX_STRING_LENGTH,
         "Image is too long." );
         FC_ASSERT( fc::is_utf8( image ), 
            "Image is not formatted in UTF8." );
         FC_ASSERT( image.size() == 46 && image[0] == 'Q' && image[1] == 'm',
            "Image rejected: IPFS string should be 46 characters long and begin with 'Qm'." );
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
      }

      if( first_name.size() > 0 )
      {
         FC_ASSERT( first_name.size() < MAX_STRING_LENGTH,
            "First name is too long." );
         FC_ASSERT( fc::is_utf8( first_name ), 
            "First name is not formatted in UTF8." );
      }

      if( last_name.size() > 0 )
      {
         FC_ASSERT( last_name.size() < MAX_STRING_LENGTH,
            "Last name is too long." );
         FC_ASSERT( fc::is_utf8( last_name ), 
            "Last name is not formatted in UTF8." );
      }

      if( gender.size() > 0 )
      {
         FC_ASSERT( gender.size() < MAX_STRING_LENGTH,
            "Gender is too long." );
         FC_ASSERT( fc::is_utf8( gender ), 
            "Gender is not formatted in UTF8." );
      }

      if( date_of_birth.size() > 0 )
      {
         FC_ASSERT( date_of_birth.size() < MAX_STRING_LENGTH,
            "Date of Birth is too long." );
         FC_ASSERT( fc::is_utf8( date_of_birth ), 
            "Date of Birth is not formatted in UTF8." );
      }

      if( email.size() > 0 )
      {
         FC_ASSERT( email.size() < MAX_STRING_LENGTH,
            "Email is too long." );
         FC_ASSERT( fc::is_utf8( email ), 
            "Email is not formatted in UTF8." );
      }

      if( phone.size() > 0 )
      {
         FC_ASSERT( phone.size() < MAX_STRING_LENGTH,
            "Phone is too long." );
         FC_ASSERT( fc::is_utf8( phone ), 
            "Phone is not formatted in UTF8." );
      }

      if( nationality.size() > 0 )
      { 
         FC_ASSERT( nationality.size() < MAX_STRING_LENGTH,
            "Nationality is too long." );
         FC_ASSERT( fc::is_utf8( nationality ), 
            "Nationality is not formatted in UTF8." );
      }

      FC_ASSERT( secure_public_key.size() < MAX_URL_LENGTH,
         "Secure Public key is too long." );
      validate_public_key( secure_public_key );

      FC_ASSERT( connection_public_key.size() < MAX_URL_LENGTH,
         "Connection Public key is too long." );
      validate_public_key( connection_public_key );

      FC_ASSERT( friend_public_key.size() < MAX_URL_LENGTH,
         "Friend Public key is too long." );
      validate_public_key( friend_public_key );

      FC_ASSERT( companion_public_key.size() < MAX_URL_LENGTH,
         "Companion Public key is too long." );
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

      FC_ASSERT( !owner_auth.is_impossible(),
         "Owner Authority is impossible." );
      FC_ASSERT( !active_auth.is_impossible(),
         "Active Authority is impossible." );
      FC_ASSERT( !posting_auth.is_impossible(),
         "Posting Authority is impossible." );
      
      if( details.size() > 0 )
      {
         FC_ASSERT( details.size() < MAX_STRING_LENGTH,
            "Details are too long." );
         FC_ASSERT( fc::is_utf8( details ), 
            "Details are not formatted in UTF8." );
      }

      if( url.size() > 0 )
      {
         FC_ASSERT( url.size() < MAX_URL_LENGTH,
            "URL is too long." );
         FC_ASSERT( fc::is_utf8( url ),
            "URL is not formatted in UTF8." );
         validate_url( url );
      }
      
      if( image.size() > 0 )
      {
         FC_ASSERT( image.size() < MAX_STRING_LENGTH,
         "Image is too long." );
         FC_ASSERT( fc::is_utf8( image ), 
            "Image is not formatted in UTF8." );
         FC_ASSERT( image.size() == 46 && image[0] == 'Q' && image[1] == 'm',
            "Image rejected: IPFS string should be 46 characters long and begin with 'Qm'." );
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
      }

      if( first_name.size() > 0 )
      {
         FC_ASSERT( first_name.size() < MAX_STRING_LENGTH,
            "First name is too long." );
         FC_ASSERT( fc::is_utf8( first_name ), 
            "First name is not formatted in UTF8." );
      }

      if( last_name.size() > 0 )
      {
         FC_ASSERT( last_name.size() < MAX_STRING_LENGTH,
            "Last name is too long." );
         FC_ASSERT( fc::is_utf8( last_name ), 
            "Last name is not formatted in UTF8." );
      }

      if( gender.size() > 0 )
      {
         FC_ASSERT( gender.size() < MAX_STRING_LENGTH,
            "Gender is too long." );
         FC_ASSERT( fc::is_utf8( gender ), 
            "Gender is not formatted in UTF8." );
      }

      if( date_of_birth.size() > 0 )
      {
         FC_ASSERT( date_of_birth.size() < MAX_STRING_LENGTH,
            "Date of Birth is too long." );
         FC_ASSERT( fc::is_utf8( date_of_birth ), 
            "Date of Birth is not formatted in UTF8." );
      }

      if( email.size() > 0 )
      {
         FC_ASSERT( email.size() < MAX_STRING_LENGTH,
            "Email is too long." );
         FC_ASSERT( fc::is_utf8( email ), 
            "Email is not formatted in UTF8." );
      }

      if( phone.size() > 0 )
      {
         FC_ASSERT( phone.size() < MAX_STRING_LENGTH,
            "Phone is too long." );
         FC_ASSERT( fc::is_utf8( phone ), 
            "Phone is not formatted in UTF8." );
      }

      if( nationality.size() > 0 )
      { 
         FC_ASSERT( nationality.size() < MAX_STRING_LENGTH,
            "Nationality is too long." );
         FC_ASSERT( fc::is_utf8( nationality ), 
            "Nationality is not formatted in UTF8." );
      }

      if( secure_public_key.size() )
      {
         FC_ASSERT( secure_public_key.size() < MAX_URL_LENGTH,
            "Secure Public key is too long." );
         validate_public_key( secure_public_key );
      }
      
      if( connection_public_key.size() )
      {
         FC_ASSERT( connection_public_key.size() < MAX_URL_LENGTH,
            "Connection Public key is too long." );
         validate_public_key( connection_public_key );
      }

      if( friend_public_key.size() )
      {
         FC_ASSERT( friend_public_key.size() < MAX_URL_LENGTH,
            "Friend ublic key is too long." );
         validate_public_key( friend_public_key );
      }

      if( companion_public_key.size() )
      {
         FC_ASSERT( companion_public_key.size() < MAX_URL_LENGTH,
            "Companion Public key is too long." );
         validate_public_key( companion_public_key );
      }
   }


   void account_verification_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( verifier_account );
      validate_account_name( verified_account );
      
      FC_ASSERT( shared_image.size() < MAX_STRING_LENGTH,
         "Shared Image is too long." );
      FC_ASSERT( fc::is_utf8( shared_image ), 
         "Shared Image is not formatted in UTF8." );
      FC_ASSERT( shared_image.size() == 46 && shared_image[0] == 'Q' && shared_image[1] == 'm',
         "Image rejected: IPFS string should be 46 characters long and begin with 'Qm'." );
   }

   void account_business_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( init_ceo_account );

      FC_ASSERT( business_type.size() < MAX_URL_LENGTH,
         "Business Type is invalid." );
      FC_ASSERT( fc::is_utf8( business_type ),
         "Business Type is invalid." );
      FC_ASSERT( officer_vote_threshold > 0, 
         "Officer vote threshold must be greater than 0.");
      FC_ASSERT( business_public_key.size() < MAX_URL_LENGTH,
         "Business Public key is too long." );
      validate_public_key( business_public_key );
   }

   void account_membership_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      if( interface.size() )
      {
         validate_account_name( interface );
      }

      FC_ASSERT( membership_type.size() < MAX_URL_LENGTH,
         "Membership Type is invalid." );
      FC_ASSERT( fc::is_utf8( membership_type ),
         "Membership Type is invalid." );
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

      FC_ASSERT( role.size() < MAX_URL_LENGTH,
         "Role Type is invalid." );
      FC_ASSERT( fc::is_utf8( role ),
         "Role Type is invalid." );
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
         FC_ASSERT( is_valid_symbol( *listed_asset ) );
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

      FC_ASSERT( connection_type.size() < MAX_URL_LENGTH,
         "Connection Type is invalid." );
      FC_ASSERT( fc::is_utf8( connection_type ),
         "Connection Type is invalid." );

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

      FC_ASSERT( connection_type.size() < MAX_URL_LENGTH,
         "Connection Type is invalid." );
      FC_ASSERT( fc::is_utf8( connection_type ),
         "Connection Type is invalid." );

      FC_ASSERT( encrypted_key.size() <= MAX_STRING_LENGTH,
         "Encrypted Key is too long." );
   }

   void account_follow_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( follower );
      validate_account_name( following );
      if( interface.size() )
      {
         validate_account_name( interface );
      }
      FC_ASSERT( follower != following, 
      "Account cannot follow itself." );
   }

   void tag_follow_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( follower );
      validate_tag_name( tag );
      if( interface.size() )
      {
         validate_account_name( interface );
      }
   }

   void activity_reward_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      if( interface.size() )
      {
         validate_account_name( interface );
      }
      validate_permlink( permlink );

      FC_ASSERT( view_id >= 0,
         "View ID cannot be negative." );
      FC_ASSERT( vote_id >= 0,
         "Vote ID cannot be negative." );
   }


   //===========================//
   // === Network Operations ===//
   //===========================//


   void update_network_officer_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( officer_type.size() < MAX_URL_LENGTH,
         "Officer Type is invalid." );
      FC_ASSERT( fc::is_utf8( officer_type ),
         "Officer Type is invalid." );
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

      for( auto name : beneficiaries )
      {
         validate_account_name( name.first );
         FC_ASSERT( name.second <= PERCENT_100,
            "Beneficiary percent is too large." );
      }

      FC_ASSERT( milestone_shares.size() >= 2, 
         "Proposal must have at least 2 milestones." );

      for( auto mile : milestone_shares )
      {
         FC_ASSERT( mile <= PERCENT_100,
            "Milestone share percent is too large." );
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
      FC_ASSERT( rating >= 1 && rating <= 10, 
         "Post Rating level should be between 1 and 10" );
      FC_ASSERT( post_type.size() < MAX_URL_LENGTH,
         "Post Type is invalid." );
      FC_ASSERT( fc::is_utf8( post_type ),
         "Post Type is invalid." );
      FC_ASSERT( reach.size() < MAX_URL_LENGTH,
         "Post Type is invalid." );
      FC_ASSERT( fc::is_utf8( reach ),
         "Post Type is invalid." );

      uint32_t sum = 0;

      FC_ASSERT( beneficiaries.size() < 128, 
         "Cannot specify more than 127 beneficiaries." ); // Require size serialization fits in one byte.

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
      validate_permlink( parent_permlink );
      validate_permlink( permlink );

      if( title.size() )
      {
         FC_ASSERT( title.size() < MAX_URL_LENGTH,
         "Title is too long." );
         FC_ASSERT( fc::is_utf8( title ),
         "Title is not formatted in UTF8." );
      }

      options.validate();

      if( community.size() )
      {
         validate_community_name( community );
      }

      if( interface.size() )
      {
         validate_account_name( interface );
      }

      FC_ASSERT( fc::is_utf8( body ),
         "Body not formatted in UTF8" );
      FC_ASSERT( body.size() + json.size() + title.size() + ipfs.size() + magnet.size() + url.size(),
         "Cannot update comment because it does not contain content." );
      FC_ASSERT( body.size() < MAX_BODY_SIZE,
         "Comment rejected: Body size is too large." );
      FC_ASSERT( language.size() == 2,
         "Comment rejected: Language should be two characters using ISO 639-1." );

      if( ipfs.size() )
      {
         FC_ASSERT( ipfs.size() == 46 && ipfs[0] == 'Q' && ipfs[1] == 'm',
            "Comment rejected: IPFS string should be 46 characters long and begin with 'Qm'." );
         FC_ASSERT( fc::is_utf8( ipfs ), 
         "Comment rejected: JSON is not valid UTF8" );
      }

      if( magnet.size() )
      {
         FC_ASSERT( magnet.size() < MAX_STRING_LENGTH,
            "Comment rejected: Magnet size is too large." );
      }
         
      if( url.size() )
      {
         FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
         FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
         validate_url( url );
      }

      for( tag_name_type t : tags )
      {
         FC_ASSERT( t.size() < MAX_URL_LENGTH,
            "Tag: ${t} is too long.",("t", t) );
         validate_tag_name( t );
      }

      if( parent_author.size() )
      {
         validate_account_name( parent_author );
      }

      if( json.size() )
      {
         FC_ASSERT( json.size() < MAX_BODY_SIZE, 
         "Comment rejected: JSON size is too large." );
         FC_ASSERT( fc::is_utf8( json ), 
         "Comment rejected: JSON is not valid UTF8" );
         FC_ASSERT( fc::json::is_valid(json), 
         "JSON Metadata not valid JSON" );
      }

      if( public_key.size() )
      {
         FC_ASSERT( public_key.size() < MAX_URL_LENGTH,
         "Comment Public key is too long." );
         validate_public_key( public_key );
      }

      FC_ASSERT( latitude <= 90 && latitude >= -90,
         "Latitude must be between +-90 degrees." );
      FC_ASSERT( longitude <= 180 && longitude >= -180,
         "Latitude must be between +-180 degrees." );
      FC_ASSERT( comment_price.amount >= 0,
         "Comment price cannot be negative." );
      FC_ASSERT( reply_price.amount >= 0,
         "Reply price cannot be negative." );
      FC_ASSERT( premium_price.amount >= 0,
         "Premium price cannot be negative." );

      if( comment_price.amount > 0 )
      {
         FC_ASSERT( is_valid_symbol( comment_price.symbol ),
            "Symbol ${symbol} is not a valid symbol", ("symbol", comment_price.symbol) );
      }
      if( reply_price.amount > 0 )
      {
         FC_ASSERT( is_valid_symbol( reply_price.symbol ),
            "Symbol ${symbol} is not a valid symbol", ("symbol", reply_price.symbol) );
      }
      if( premium_price.amount > 0 )
      {
         FC_ASSERT( is_valid_symbol( premium_price.symbol ),
            "Symbol ${symbol} is not a valid symbol", ("symbol", premium_price.symbol) );
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
      FC_ASSERT( uuid.size() < MAX_URL_LENGTH,
         "UUID is too long." );
      validate_uuidv4( uuid );
   }

   void vote_operation::validate() const
   {  
      validate_account_name( signatory );
      validate_account_name( voter );
      validate_account_name( author );

      if( interface.size() )
      {
         validate_account_name( interface );
      }

      FC_ASSERT( abs(weight) <= PERCENT_100, 
         "Weight is not a valid percentage (0 - 10000)" );
      validate_permlink( permlink );
   }

   void view_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( viewer );
      validate_account_name( author );

      if( interface.size() )
      {
         validate_account_name( interface );
      }

      validate_account_name( supernode );
      validate_permlink( permlink );
   }

   void share_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( sharer );
      validate_account_name( author );

      if( interface.size() )
      {
         validate_account_name( interface );
      }

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

      for( tag_name_type t : tags )
      {
         validate_tag_name( t );
      }

      FC_ASSERT( rating >= 1 && rating <= 10,
         "Post Rating level should be between 1 and 10" );
      FC_ASSERT( details.size(), 
         "Moderation tag must include details explaining the rule or standard violation." );
      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
   }

   void list_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( creator );

      FC_ASSERT( list_id.size(),
         "List has no list ID." );
      FC_ASSERT( list_id.size() <= MAX_STRING_LENGTH,
         "List ID is too long." );
      FC_ASSERT( fc::is_utf8( list_id ),
         "List ID must be UTF-8" );

      validate_uuidv4( list_id );

      FC_ASSERT( name.size(),
         "List has no name." );
      FC_ASSERT( name.size() <= MAX_STRING_LENGTH,
         "Name is too long." );
      FC_ASSERT( fc::is_utf8( name ),
         "Name must be UTF-8" );
      
      for( int64_t a : accounts )
      {
         FC_ASSERT( a >= 0, 
            "IDs must be greater than or equal to 0.");
      }
      for( int64_t a : comments )
      {
         FC_ASSERT( a >= 0, 
            "IDs must be greater than or equal to 0.");
      }
      for( int64_t a : communities )
      {
         FC_ASSERT( a >= 0, 
            "IDs must be greater than or equal to 0.");
      }
      for( int64_t a : assets )
      {
         FC_ASSERT( a >= 0, 
            "IDs must be greater than or equal to 0.");
      }
      for( int64_t a : products )
      {
         FC_ASSERT( a >= 0, 
            "IDs must be greater than or equal to 0.");
      }
      for( int64_t a : auctions )
      {
         FC_ASSERT( a >= 0, 
            "IDs must be greater than or equal to 0.");
      }
      for( int64_t a : nodes )
      {
         FC_ASSERT( a >= 0, 
            "IDs must be greater than or equal to 0.");
      }
      for( int64_t a : edges )
      {
         FC_ASSERT( a >= 0, 
            "IDs must be greater than or equal to 0.");
      }
      for( int64_t a : node_types )
      {
         FC_ASSERT( a >= 0, 
            "IDs must be greater than or equal to 0.");
      }
      for( int64_t a : edge_types )
      {
         FC_ASSERT( a >= 0, 
            "IDs must be greater than or equal to 0.");
      }
   }

   void poll_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( creator );

      FC_ASSERT( poll_id.size(),
         "Poll has no list ID." );
      FC_ASSERT( poll_id.size() <= MAX_STRING_LENGTH,
         "Poll ID is too long." );
      FC_ASSERT( fc::is_utf8( poll_id ),
         "Poll ID must be UTF-8" );

      validate_uuidv4( poll_id );

      FC_ASSERT( details.size(),
         "Poll has no name." );
      FC_ASSERT( details.size() <= MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ),
         "Details must be UTF-8" );

      
      for( auto option : poll_options )
      {
         FC_ASSERT( option.size(),
         "Poll has no name." );
         FC_ASSERT( option.size() <= MAX_STRING_LENGTH,
            "Option are too long." );
         FC_ASSERT( fc::is_utf8( option ),
            "Option must be UTF-8" );
      }

      FC_ASSERT( completion_time > GENESIS_TIME,
         "Completion time must be after Genesis time." );
   }

   void poll_vote_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( voter );
      validate_account_name( creator );

      FC_ASSERT( poll_id.size(),
         "Poll has no list ID." );
      FC_ASSERT( poll_id.size() <= MAX_STRING_LENGTH,
         "Poll ID is too long." );
      FC_ASSERT( fc::is_utf8( poll_id ),
         "Poll ID must be UTF-8" );

      validate_uuidv4( poll_id );

      FC_ASSERT( poll_option >= 0,
      "Poll option must be greater than 0." );
   }


   //==============================//
   // === Community Operations === //
   //==============================//


   void community_create_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( founder );
      validate_community_name( name );

      FC_ASSERT( community_privacy.size() < MAX_URL_LENGTH,
         "Community privacy Type is invalid." );
      FC_ASSERT( fc::is_utf8( community_privacy ),
         "Community privacy Type is invalid." );

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
         FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
         FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
         validate_url( url );
      }

      FC_ASSERT( community_public_key.size() < MAX_URL_LENGTH,
         "Community Public key is too long." );
      validate_public_key( community_public_key );
      
      FC_ASSERT( max_rating >= 1 && max_rating <= 9, 
         "Post Max Rating level should be between 1 and 9" );
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
      }

      FC_ASSERT( details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( fc::is_utf8( details ), 
         "Details are not formatted in UTF8." );
      
      if( url.size() > 0 )
      {
         FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
         FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
         validate_url( url );
      }

      FC_ASSERT( community_public_key.size() < MAX_URL_LENGTH,
         "Community public key is too long." );
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
      validate_account_name( new_founder );
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

      if( interface.size() )
      {
         validate_account_name( interface );
      }

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

      FC_ASSERT( !( attending && not_attending ),
         "Cannot select to both attend and not attend the same event." );
   }



   //================================//
   // === Advertising Operations === //
   //================================//



   void ad_creative_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( author );

      FC_ASSERT( format_type.size() < MAX_URL_LENGTH,
         "Format Type is invalid." );
      FC_ASSERT( fc::is_utf8( format_type ),
         "Format Type is invalid." );

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

      if( interface.size() )
      {
         validate_account_name( interface );
      }

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
         "Inventory ID is required." );
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

      FC_ASSERT( metric.size() < MAX_URL_LENGTH,
         "Metric Type is invalid." );
      FC_ASSERT( fc::is_utf8( metric ),
         "Metric Type is invalid." );

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
      validate_account_name( author );
      validate_account_name( provider );

      FC_ASSERT( bid_id.size(),
         "bid has no bid ID." );
      FC_ASSERT( bid_id.size() <= MAX_STRING_LENGTH,
         "bid ID is too long." );
      FC_ASSERT( fc::is_utf8( bid_id ),
         "bid ID must be UTF-8" );
      validate_uuidv4( bid_id );

      FC_ASSERT( campaign_id.size(),
         "Bid has no campaign ID." );
      FC_ASSERT( campaign_id.size() <= MAX_STRING_LENGTH,
         "Bid ID is too long." );
      FC_ASSERT( fc::is_utf8( campaign_id ),
         "Bid ID must be UTF-8" );
      validate_uuidv4( campaign_id );

      FC_ASSERT( creative_id.size(),
         "Bid has no creative ID." );
      FC_ASSERT( creative_id.size() <= MAX_STRING_LENGTH,
         "Bid ID is too long." );
      FC_ASSERT( fc::is_utf8( creative_id ),
         "Bid ID must be UTF-8" );
      validate_uuidv4( creative_id );

      FC_ASSERT( inventory_id.size(),
         "Bid has no inventory ID." );
      FC_ASSERT( inventory_id.size() <= MAX_STRING_LENGTH,
         "Bid ID is too long." );
      FC_ASSERT( fc::is_utf8( inventory_id ),
         "Bid ID must be UTF-8" );
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
      FC_ASSERT( bid_price.symbol == SYMBOL_COIN,
         "Bid Price must be denominated in core asset." );
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

      FC_ASSERT( audience_id.size(),
         "Bid has no audience ID." );
      FC_ASSERT( audience_id.size() <= MAX_STRING_LENGTH,
         "Audience ID is too long." );
      FC_ASSERT( fc::is_utf8( audience_id ),
         "Audience ID must be UTF-8" );
      validate_uuidv4( audience_id );
   }



   //==========================//
   //==== Graph Operations ====//
   //==========================//



   void graph_node_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      if( interface.size() )
      {
         validate_account_name( interface );
      }

      FC_ASSERT( node_public_key.size() < MAX_URL_LENGTH,
         "Community public key is too long." );
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
      }
   }


   void graph_edge_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      validate_account_name( from_node_account );
      validate_account_name( to_node_account );

      if( interface.size() )
      {
         validate_account_name( interface );
      }

      FC_ASSERT( edge_public_key.size() < MAX_URL_LENGTH,
         "Community public key is too long." );
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
      }
   }

   void graph_node_property_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      if( interface.size() )
      {
         validate_account_name( interface );
      }

      validate_account_name( node_type );

      FC_ASSERT( graph_privacy.size() < MAX_URL_LENGTH,
         "Graph Privacy Type is invalid." );
      FC_ASSERT( fc::is_utf8( graph_privacy ),
         "Graph Privacy Type is invalid." );

      FC_ASSERT( edge_permission.size() < MAX_URL_LENGTH,
         "Edge Permission Type is invalid." );
      FC_ASSERT( fc::is_utf8( edge_permission ),
         "Edge Permission Type is invalid." );

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
      }
   }

   void graph_edge_property_operation::validate() const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      if( interface.size() )
      {
         validate_account_name( interface );
      }

      validate_account_name( edge_type );

      FC_ASSERT( graph_privacy.size() < MAX_URL_LENGTH,
         "Graph Privacy Type is invalid." );
      FC_ASSERT( fc::is_utf8( graph_privacy ),
         "Graph Privacy Type is invalid." );

      FC_ASSERT( from_node_types.size() > 0,
         "Graph From node types are required." );

      for( auto a : from_node_types )
      {
         is_valid_symbol( a );
      }

      FC_ASSERT( to_node_types.size() > 0,
         "Graph To node types are required." );

      for( auto a : to_node_types )
      {
         is_valid_symbol( a );
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


   /**
    *  Packs *this then encodes as base58 encoded string.
    */
   stealth_confirmation::operator string()const
   {
      return fc::to_base58( fc::raw::pack( *this ) );
   }
   /**
    * Unpacks from a base58 string
    */
   stealth_confirmation::stealth_confirmation( const std::string& base58 )
   {
      *this = fc::raw::unpack<stealth_confirmation>( fc::from_base58( base58 ) );
   }


   /**
    * Verifies that input commitments - output commitments add up to 0.
    * 
    * Require all inputs and outputs to be sorted by commitment 
    * to prevent duplicate commitments
    * and ensure information is not leaked through ordering of commitments.
    * 
    * This method can be computationally intensive due to the 
    * commitment sum verifcation. This part is done last to avoid 
    * comparing unnecessarilty.
    */
   void transfer_confidential_operation::validate()const
   {
      FC_ASSERT( is_valid_symbol( fee.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", fee.symbol) );

      vector< commitment_type > in(inputs.size());
      vector< commitment_type > out(outputs.size());
      int64_t net_public = fee.amount.value;              // from_amount.value - to_amount.value;

      for( uint32_t i = 0; i < in.size(); ++i )
      {
         in[i] = inputs[i].commitment;
         if( i > 0 )
         {
            FC_ASSERT( in[i-1] < in[i] );
         }
      }

      for( uint32_t i = 0; i < out.size(); ++i )
      {
         out[i] = outputs[i].commitment;
         if( i > 0 ) 
         {
            FC_ASSERT( out[i-1] < out[i] );
         }
         FC_ASSERT( !outputs[i].owner.is_impossible() );
      }

      FC_ASSERT( in.size(),
         "Transfer must have at least one input" );

      if( outputs.size() > 1 )
      {
         for( auto out : outputs )
         {
            auto info = fc::ecc::range_get_info( out.range_proof );
            FC_ASSERT( info.max_value <= MAX_ASSET_SUPPLY );
         }
      }
      FC_ASSERT( fc::ecc::verify_sum( in, out, net_public ), 
         "Input commitments sum is not equal to Output commitments.", ("net_public", net_public) );
   } 


   /**
    * Require all outputs to be sorted.
    * This prevents duplicates AND prevents implementations
    * From accidentally leaking information by how they arrange commitments.
    */
   void transfer_to_confidential_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( from );

      FC_ASSERT( fee.amount >= 0 );
      FC_ASSERT( amount.amount > 0 );

      FC_ASSERT( is_valid_symbol( fee.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", fee.symbol) );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );

      vector< commitment_type > in;
      vector< commitment_type > out(outputs.size());
      int64_t net_public = amount.amount.value;

      for( uint32_t i = 0; i < out.size(); ++i )
      {
         out[i] = outputs[i].commitment;
         if( i > 0 )
         {
            FC_ASSERT( out[i-1] < out[i], "All outputs must be sorted by commitment id" );
         }
         
         FC_ASSERT( !outputs[i].owner.is_impossible(),
            "Balance owner authority cannot be impossible to sign." );
      }
      FC_ASSERT( out.size(), "Transfer must have at least one output" );

      auto public_c = fc::ecc::blind(blinding_factor,net_public);

      FC_ASSERT( fc::ecc::verify_sum( {public_c}, out, 0 ), "", ("net_public",net_public) );

      if( outputs.size() > 1 )
      {
         for( auto out : outputs )
         {
            auto info = fc::ecc::range_get_info( out.range_proof );
            FC_ASSERT( info.max_value <= MAX_ASSET_SUPPLY );
         }
      }
   }


   /**
    * Requiring all inputs to be sorted we also prevent duplicate commitments on the input
    */
   void transfer_from_confidential_operation::validate()const
   {
      validate_account_name( to );

      FC_ASSERT( is_valid_symbol( fee.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", fee.symbol) );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );

      FC_ASSERT( amount.amount > 0 );
      FC_ASSERT( fee.amount >= 0 );
      FC_ASSERT( inputs.size() > 0 );
      FC_ASSERT( amount.symbol == fee.symbol );

      vector< commitment_type >      in(inputs.size());

      vector< commitment_type >      out;

      int64_t                        net_public = fee.amount.value + amount.amount.value;

      out.push_back( fc::ecc::blind( blinding_factor, net_public ) );

      for( uint32_t i = 0; i < in.size(); ++i )
      {
         in[i] = inputs[i].commitment;

         if( i > 0 )
         {
            FC_ASSERT( in[i-1] < in[i], "all inputs must be sorted by commitment id" );
         } 
      }
      FC_ASSERT( in.size(), "there must be at least one input" );
      FC_ASSERT( fc::ecc::verify_sum( in, out, 0 ) );
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
         "Request ID is required." );
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



   void product_sale_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( product_id.size() < MAX_STRING_LENGTH,
         "Product ID is too long." );
      FC_ASSERT( product_id.size() > 0,
         "Product ID is required." );
      FC_ASSERT( fc::is_utf8( product_id ), 
         "Product ID is not UTF8" );

      validate_uuidv4( product_id );
      
      FC_ASSERT( name.size() < MAX_STRING_LENGTH,
         "Product name is too long" );
      FC_ASSERT( name.size() > 0,
         "Product name is required." );
      FC_ASSERT( fc::is_utf8( name ),
         "Product Name is not UTF8" );

      FC_ASSERT( product_variants.size() == product_prices.size(),
         "Product prices must be the same length as the product variants." );
      FC_ASSERT( product_variants.size() == stock_available.size(),
         "Stock Available must be the same length as the product variants." );

      for( auto a : product_variants )
      {
         validate_account_name( a );
      }

      if( product_details.size() )
      {
         FC_ASSERT( product_details.size() < MAX_STRING_LENGTH,
            "Product details is too long" );
         FC_ASSERT( fc::is_utf8( product_details ),
            "Product details is not UTF8" );
      }

      if( product_image.size() )
      {
         FC_ASSERT( product_image.size() < MAX_STRING_LENGTH,
         "Image is too long" );
         FC_ASSERT( fc::is_utf8( product_image ),
            "Image is not UTF8" );
         FC_ASSERT( product_image.size() == 46 && product_image[0] == 'Q' && product_image[1] == 'm',
            "Image IPFS string should be 46 characters long and begin with 'Qm'." );
      }
      
      for( auto a : product_prices )
      {
         FC_ASSERT( is_valid_symbol( a.symbol ),
            "Product Price symbol is not valid symbol" );
         FC_ASSERT( a.amount >= 0,
            "Product Price must be positive amount" );
      }

      for( auto a : wholesale_discount )
      {
         FC_ASSERT( a.second >= 0 && a.second <= PERCENT_100,
            "Wholesale Discount must be a percentage between 0 and PERCENT_100." );
      }

      for( auto a : stock_available )
      {
         FC_ASSERT( a >= 0,
            "Stock available must be greater than or equal to 0." );
      }
   
      if( json.size() )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON" );
      }

      if( url.size() > 0 )
      {
         validate_url( url );
         FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long" );
         FC_ASSERT( fc::is_utf8( url ),
            "URL is not UTF8" );
      }
      
      for( auto a : delivery_variants )
      {
         validate_account_name( a );
      }

      if( delivery_details.size() > 0 )
      {
         FC_ASSERT( delivery_details.size() < MAX_STRING_LENGTH,
            "Delivery variant is too long" );
         FC_ASSERT( fc::is_utf8( delivery_details ),
            "Delivery variant is not UTF8" );
      }
      
      for( auto a : delivery_prices )
      {
         FC_ASSERT( is_valid_symbol( a.symbol ),
            "Delivery Price symbol is not valid symbol." );
         FC_ASSERT( a.amount >= 0,
            "Delivery Price must be greater than or equal to 0" );
      }
   }


   void product_purchase_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( seller );
      validate_account_name( buyer );

      FC_ASSERT( order_id.size() < MAX_STRING_LENGTH,
         "Order ID is too long." );
      FC_ASSERT( order_id.size() > 0,
         "Order ID is required." );
      FC_ASSERT( fc::is_utf8( order_id ), 
         "Order ID is not UTF8" );

      validate_uuidv4( order_id );

      FC_ASSERT( product_id.size() < MAX_STRING_LENGTH,
         "Product ID is too long." );
      FC_ASSERT( product_id.size() > 0,
         "Product ID is required." );
      FC_ASSERT( fc::is_utf8( product_id ), 
         "Product ID is not UTF8" );

      validate_uuidv4( product_id );

      for( auto a : order_size )
      {
         FC_ASSERT( a >= 0,
            "Order Size must be positive amount" );
      }

      FC_ASSERT( shipping_address.size() < MAX_STRING_LENGTH,
         "Shipping Address is too long" );
      FC_ASSERT( fc::is_utf8( shipping_address ),
         "Shipping Address is not UTF8" );

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


   void product_auction_sale_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( auction_id.size() < MAX_STRING_LENGTH,
         "Auction ID is too long." );
      FC_ASSERT( auction_id.size() > 0,
         "Auction ID is required." );
      FC_ASSERT( fc::is_utf8( auction_id ), 
         "Auction ID is not UTF8" );

      validate_uuidv4( auction_id );

      FC_ASSERT( auction_type.size() < MAX_URL_LENGTH,
         "Auction type is too long." );
      FC_ASSERT( auction_type.size() > 0,
         "Auction type is required." );
      FC_ASSERT( fc::is_utf8( auction_type ), 
         "Auction type is not UTF8" );
      
      FC_ASSERT( name.size() < MAX_STRING_LENGTH,
         "Product name is too long" );
      FC_ASSERT( name.size() > 0,
         "Product name is required." );
      FC_ASSERT( fc::is_utf8( name ),
         "Product Name is not UTF8" );

      if( json.size() )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON" );
      }

      if( url.size() > 0 )
      {
         validate_url( url );
         FC_ASSERT( url.size() < MAX_URL_LENGTH,
            "URL is too long" );
         FC_ASSERT( fc::is_utf8( url ),
            "URL is not UTF8" );
      }

      if( product_details.size() > 0 )
      {
         FC_ASSERT( product_details.size() < MAX_STRING_LENGTH,
            "Product details is too long" );
         FC_ASSERT( fc::is_utf8( product_details ),
            "Product details is not UTF8" );
      }

      if( product_image.size() > 0 )
      {
         FC_ASSERT( product_image.size() < MAX_STRING_LENGTH,
            "Image is too long" );
         FC_ASSERT( fc::is_utf8( product_image ),
            "Image is not UTF8" );
         FC_ASSERT( product_image.size() == 46 && product_image[0] == 'Q' && product_image[1] == 'm',
            "Image IPFS string should be 46 characters long and begin with 'Qm'." );
      } 
      
      FC_ASSERT( is_valid_symbol( reserve_bid.symbol ),
         "Reserve Bid symbol is not valid symbol" );
      FC_ASSERT( reserve_bid.amount >= 0,
         "Reserve Bid must be positive amount." );
      FC_ASSERT( is_valid_symbol( maximum_bid.symbol ),
         "Maximum Bid symbol is not valid symbol" );
      FC_ASSERT( maximum_bid.amount >= 0,
         "Maximum Bid must be positive amount." );
      
      for( auto a : delivery_variants )
      {
         validate_account_name( a );
      }

      if( delivery_details.size() > 0 )
      {
         FC_ASSERT( delivery_details.size() < MAX_STRING_LENGTH,
            "Delivery variant is too long" );
         FC_ASSERT( fc::is_utf8( delivery_details ),
            "Delivery variant is not UTF8" );
      }
      
      for( auto a : delivery_prices )
      {
         FC_ASSERT( is_valid_symbol( a.symbol ),
            "Delivery Price symbol is not valid symbol." );
         FC_ASSERT( a.amount >= 0,
            "Delivery Price must be greater than or equal to 0" );
      }

      FC_ASSERT( final_bid_time > GENESIS_TIME,
         "Final Bid time must be after genesis time." );
      FC_ASSERT( completion_time > GENESIS_TIME,
         "Completion time must be after genesis time." );
   }


   void product_auction_bid_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( buyer );
      validate_account_name( seller );

      FC_ASSERT( bid_id.size() < MAX_STRING_LENGTH,
         "Bid ID is too long." );
      FC_ASSERT( bid_id.size() > 0,
         "Bid ID is required." );
      FC_ASSERT( fc::is_utf8( bid_id ), 
         "Bid ID is not UTF8" );

      validate_uuidv4( bid_id );

      FC_ASSERT( auction_id.size() < MAX_STRING_LENGTH,
         "Auction ID is too long." );
      FC_ASSERT( auction_id.size() > 0,
         "Auction ID is required." );
      FC_ASSERT( fc::is_utf8( auction_id ), 
         "Auction ID is not UTF8" );

      validate_uuidv4( auction_id );

      FC_ASSERT( memo.size() < MAX_MEMO_SIZE,
         "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ),
         "Memo is not UTF8" );

      if( json.size() )
      {
         FC_ASSERT( fc::is_utf8( json ),
            "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid( json ),
            "JSON Metadata not valid JSON" );
      }

      FC_ASSERT( shipping_address.size() < MAX_STRING_LENGTH,
         "Shipping Address is too long" );
      FC_ASSERT( shipping_address.size() > 0,
         "Shipping Address is required." );
      FC_ASSERT( fc::is_utf8( shipping_address ),
         "Shipping Address is not UTF8" );

      validate_account_name( delivery_variant );

      FC_ASSERT( delivery_details.size() < MAX_STRING_LENGTH,
         "Delivery details is too long" );
      FC_ASSERT( delivery_details.size() > 0,
         "Delivery details is required." );
      FC_ASSERT( fc::is_utf8( delivery_details ),
         "Delivery details is not UTF8" );
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

      if( interface.size() )
      {
         validate_account_name( interface );
      }

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

      if( interface.size() )
      {
         validate_account_name( interface );
      }

      validate_account_name( owner );
      
      FC_ASSERT( amount_to_borrow.amount > 0, 
         "Please set a greater than zero amount to borrow and collateral.");
      FC_ASSERT( ( amount_to_borrow * exchange_rate ).amount > 0,
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

      if( interface.size() )
      {
         validate_account_name( interface );
      }

      FC_ASSERT( amount_to_sell.symbol == limit_close_price.base.symbol,
         "Sell asset must be the base of the price" );
      limit_close_price.validate();
      FC_ASSERT( ( amount_to_sell * limit_close_price ).amount > 0,
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

      if( interface.size() )
      {
         validate_account_name( interface );
      }

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

      if( interface.size() )
      {
         validate_account_name( interface );
      }

      FC_ASSERT( order_id.size() < MAX_STRING_LENGTH,
         "Order ID is too long." );
      FC_ASSERT( order_id.size(),
         "Order ID is required." );
      FC_ASSERT( fc::is_utf8( order_id ),
         "Order ID is not UTF8" );
      validate_uuidv4( order_id );

      FC_ASSERT( is_valid_symbol( options_issued.symbol ),
         "Symbol ${symbol} is not a valid symbol.", ("symbol", options_issued.symbol) );
      FC_ASSERT( options_issued.amount >= 0,
         "Amount to issue cannot be negative." );
   }


   //=========================//
   // === Pool Operations === //
   //=========================//



   void liquidity_pool_create_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( first_amount.amount >= BLOCKCHAIN_PRECISION,
         "First Amount must be greater than or equal to 1 unit." );
      FC_ASSERT( is_valid_symbol( first_amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", first_amount.symbol) );
      FC_ASSERT( second_amount.amount >= BLOCKCHAIN_PRECISION,
         "Second Amount must be greater than or equal to 1 unit." );
      FC_ASSERT( is_valid_symbol( second_amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", second_amount.symbol) );
   }

   void liquidity_pool_exchange_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      if( interface.size() )
      {
         validate_account_name( interface );
      }

      FC_ASSERT( amount.amount >= 0,
         "Amount must be greater than or equal to zero." );
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
         "Amount must be greater than or equal to zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( is_valid_symbol( pair_asset ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", pair_asset) );
   }

   void liquidity_pool_withdraw_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount >= 0,
         "Amount must be greater than or equal to zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( is_valid_symbol( receive_asset ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", receive_asset) );
   }

   void credit_pool_collateral_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount >= 0,
         "Amount must be greater than or equal to zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
   }

   void credit_pool_borrow_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount >= 0,
         "Amount must be greater than or equal to zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( collateral.amount >= 0,
         "Amount must be greater than or equal to zero." );
      FC_ASSERT( is_valid_symbol( collateral.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
   }

   void credit_pool_lend_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount >= 0,
         "Amount must be greater than or equal to zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
   }

   void credit_pool_withdraw_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount >= 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
   }

   void option_pool_create_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( is_valid_symbol( first_asset ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", first_asset) );
      FC_ASSERT( is_valid_symbol( second_asset ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", second_asset) );
   }

   void prediction_pool_create_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( is_valid_symbol( prediction_symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", prediction_symbol) );
      FC_ASSERT( is_valid_symbol( collateral_symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", collateral_symbol) );

      flat_set< asset_symbol_type > asset_set;

      for( asset_symbol_type a : outcome_assets )
      {
         FC_ASSERT( a != INVALID_OUTCOME_SYMBOL,
            "Symbol ${symbol} is not a valid symbol", ("symbol", prediction_symbol+"."+a) );
         FC_ASSERT( is_valid_symbol( prediction_symbol+"."+a ),
            "Symbol ${symbol} is not a valid symbol", ("symbol", prediction_symbol+"."+a) );
         asset_set.insert( prediction_symbol+"."+a );
      }

      FC_ASSERT( asset_set.size() == outcome_assets.size(),
         "Outcome assets should not contain any duplicates." );
      FC_ASSERT( outcome_details.size() < MAX_STRING_LENGTH,
         "Details are too long." );
      FC_ASSERT( outcome_details.size(),
         "Details are required." );
      FC_ASSERT( fc::is_utf8( outcome_details ),
         "Details are not UTF8." );
      FC_ASSERT( display_symbol.size() < MAX_STRING_LENGTH,
         "Display Symbol is too long." );
      FC_ASSERT( display_symbol.size(),
         "Display Symbol is required." );
      FC_ASSERT( fc::is_utf8( display_symbol ),
         "Display Symbol is not UTF8." );
      
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

      FC_ASSERT( outcome_time > GENESIS_TIME,
         "Outcome time must be after Genesis time." );
      FC_ASSERT( prediction_bond.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( prediction_bond.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", prediction_bond.symbol) );
   }

   void prediction_pool_exchange_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( is_valid_symbol( prediction_asset ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", prediction_asset) );
   }

   void prediction_pool_resolve_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );

      FC_ASSERT( amount.amount > 0,
         "Amount must be greater than zero." );
      FC_ASSERT( is_valid_symbol( amount.symbol ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( is_valid_symbol( resolution_outcome ),
         "Symbol ${symbol} is not a valid symbol", ("symbol", resolution_outcome) );
   }



   //==========================//
   // === Asset Operations === //
   //==========================//



   void asset_options::validate()const
   {
      FC_ASSERT( display_symbol.size(),
         "Display Symbol are required." );
      FC_ASSERT( display_symbol.size() < MAX_URL_LENGTH,
         "Display Symbol is too long." );
      FC_ASSERT( fc::is_utf8( display_symbol ), 
         "Display Symbol is not formatted in UTF8." );

      FC_ASSERT( details.size(),
         "Details are required." );
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

      FC_ASSERT( url.size() < MAX_URL_LENGTH,
         "URL is too long." );
      FC_ASSERT( fc::is_utf8( url ),
         "URL is not formatted in UTF8." );
      if( url.size() > 0 )
      {
         validate_url( url );
      }

      FC_ASSERT( max_supply > 0, 
         "Maximum asset supply must be greater than zero." );
      FC_ASSERT( max_supply <= MAX_ASSET_SUPPLY,
         "Max supply is too high." );
      FC_ASSERT( market_fee_percent <= PERCENT_100,
         "Market fee percent must be between 0 and 100%." );
      FC_ASSERT( market_fee_share_percent <= PERCENT_100,
         "Market fee share percent must be between 0 and 100%." );
      FC_ASSERT( max_market_fee <= MAX_ASSET_SUPPLY,
         "Max market fee must be between 0 and Max asset supply" );
      
      // There must be no high bits in permissions whose meaning is not known.

      FC_ASSERT( !( issuer_permissions & ~ASSET_ISSUER_PERMISSION_MASK ) );

      // The global_settle flag may never be set (this is a permission only)

      FC_ASSERT( !( flags & int( asset_issuer_permission_flags::global_settle ) ) );

      if( !whitelist_authorities.empty() || !blacklist_authorities.empty() )
      {
         FC_ASSERT( flags & int( asset_issuer_permission_flags::balance_whitelist ) );
      }

      for( account_name_type item : whitelist_authorities )
      {
         FC_ASSERT( std::find( blacklist_authorities.begin(), blacklist_authorities.end(), item ) == blacklist_authorities.end() );
      }
      for( account_name_type item : blacklist_authorities )
      {
         FC_ASSERT( std::find( whitelist_authorities.begin(), whitelist_authorities.end(), item ) == whitelist_authorities.end() );
      }
      for( asset_symbol_type item : whitelist_markets )
      {
         FC_ASSERT( std::find( blacklist_markets.begin(), blacklist_markets.end(), item ) == blacklist_markets.end() );
      }
      for( asset_symbol_type item : blacklist_markets )
      {
         FC_ASSERT( std::find( whitelist_markets.begin(), whitelist_markets.end(), item ) == whitelist_markets.end() );
      }

      // === Currency Asset Options === //

      FC_ASSERT( is_valid_symbol( block_reward.symbol ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", block_reward.symbol) );
      FC_ASSERT( block_reward.amount >= 0,
         "Block reward must be greater than or equal to zero." );
      FC_ASSERT( block_reward_reduction_percent <= PERCENT_100,
         "Block Reward reduction percent must be between 0 and 100%." );
      FC_ASSERT( block_reward_reduction_days >= 0,
         "Block Reward reduction days must be greater than 0." );
      FC_ASSERT( content_reward_percent <= PERCENT_100,
         "Content reward percent must be between 0 and 100%." );
      FC_ASSERT( is_valid_symbol( equity_asset ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", equity_asset) );
      FC_ASSERT( equity_reward_percent <= PERCENT_100,
         "Equity reward percent must be between 0 and 100%." );
      FC_ASSERT( producer_reward_percent <= PERCENT_100,
         "Producer reward percent must be between 0 and 100%." );
      FC_ASSERT( supernode_reward_percent <= PERCENT_100,
         "Supernode reward percent must be between 0 and 100%." );
      FC_ASSERT( power_reward_percent <= PERCENT_100,
         "Power reward percent must be between 0 and 100%." );
      FC_ASSERT( community_fund_reward_percent <= PERCENT_100,
         "Community Fund reward percent must be between 0 and 100%." );
      FC_ASSERT( development_reward_percent <= PERCENT_100,
         "Development reward percent must be between 0 and 100%." );
      FC_ASSERT( marketing_reward_percent <= PERCENT_100,
         "Marketing reward percent must be between 0 and 100%." );
      FC_ASSERT( advocacy_reward_percent <= PERCENT_100,
         "Advocacy reward percent must be between 0 and 100%." );
      FC_ASSERT( activity_reward_percent <= PERCENT_100,
         "Activity reward percent must be between 0 and 100%." );
      FC_ASSERT( producer_block_reward_percent <= PERCENT_100,
         "Producer Block reward percent must be between 0 and 100%." );
      FC_ASSERT( validation_reward_percent <= PERCENT_100,
         "Validation reward percent must be between 0 and 100%." );
      FC_ASSERT( txn_stake_reward_percent <= PERCENT_100,
         "Transaction Stake reward percent must be between 0 and 100%." );
      FC_ASSERT( work_reward_percent <= PERCENT_100,
         "Work reward percent must be between 0 and 100%." );
      FC_ASSERT( producer_activity_reward_percent <= PERCENT_100,
         "Producer Activity reward percent must be between 0 and 100%." );

      // === Stablecoin Options === //

      FC_ASSERT( feed_lifetime >= MIN_FEED_LIFETIME,
         "Feed lifetime must be greater than network minimum." );
      FC_ASSERT( minimum_feeds >= 0,
         "Minimum Feeds must be greater than or equal to 0." );
      FC_ASSERT( asset_settlement_delay >= MIN_SETTLEMENT_DELAY,
         "Force Settlement delay must be greater than network minimum." );
      FC_ASSERT( asset_settlement_offset_percent <= PERCENT_100,
         "Asset Settlement offset percent must be between 0 and 100%." );
      FC_ASSERT( maximum_asset_settlement_volume <= PERCENT_100,
         "Maximum Asset Settlement volume percent must be between 0 and 100%." );
      FC_ASSERT( is_valid_symbol( backing_asset ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", backing_asset) );

      // === Equity Asset Options === //

      FC_ASSERT( dividend_share_percent <= PERCENT_100,
         "Dividend Share percent must be between 0 and 100%." );
      FC_ASSERT( liquid_dividend_percent <= PERCENT_100,
         "Liquid Dividend Share percent must be between 0 and 100%." );
      FC_ASSERT( staked_dividend_percent <= PERCENT_100,
         "Staked Dividend Share percent must be between 0 and 100%." );
      FC_ASSERT( savings_dividend_percent <= PERCENT_100,
         "Savings Dividend Share percent must be between 0 and 100%." );
      FC_ASSERT( liquid_voting_rights <= PERCENT_100,
         "Liquid Voting Rights must be between 0 and 100%." );
      FC_ASSERT( staked_voting_rights <= PERCENT_100,
         "Staked Voting Rights must be between 0 and 100%." );
      FC_ASSERT( savings_voting_rights <= PERCENT_100,
         "Savings Voting Rights must be between 0 and 100%." );
      FC_ASSERT( min_active_time <= fc::days(365) && 
         min_active_time >= fc::days(1),
         "Min active time must be between 1 and 365 days." );
      FC_ASSERT( min_balance <= MAX_ASSET_SUPPLY &&
         min_balance >= 0,
         "Activty min balance must be between 0 and max asset supply." );
      FC_ASSERT( min_producers <= 100 && min_producers >= 0,
         "Activity min producers must be between 0 and 100." );
      FC_ASSERT( boost_balance <= MAX_ASSET_SUPPLY &&
         boost_balance >= 0,
         "Activty boost balance must be between 0 and max asset supply." );
      FC_ASSERT( boost_activity <= MAX_ASSET_SUPPLY &&
         boost_activity >= 0,
         "Activty boost activity must be between 0 and max asset supply." );
      FC_ASSERT( boost_producers <= 100 && 
         boost_producers >= 0,
         "Activity boost producers must be between 0 and 100." );
      FC_ASSERT( boost_top <= PERCENT_100,
         "Boost Top must be between 0 and 100%." );

      // === Credit Asset options === //

      FC_ASSERT( is_valid_symbol( buyback_asset ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", buyback_asset) );
      buyback_price.validate();
      FC_ASSERT( liquid_fixed_interest_rate <= PERCENT_100,
         "Liquid Fixed Interest percent must be between 0 and 100%." );
      FC_ASSERT( liquid_variable_interest_rate <= PERCENT_100,
         "Liquid Variable Interest percent must be between 0 and 100%." );
      FC_ASSERT( staked_fixed_interest_rate <= PERCENT_100,
         "Staked Fixed Interest percent must be between 0 and 100%." );
      FC_ASSERT( staked_variable_interest_rate <= PERCENT_100,
         "Staked Variable Interest percent must be between 0 and 100%." );
      FC_ASSERT( savings_fixed_interest_rate <= PERCENT_100,
         "Savings Fixed Interest percent must be between 0 and 100%." );
      FC_ASSERT( savings_variable_interest_rate <= PERCENT_100,
         "Savings Variable Interest percent must be between 0 and 100%." );
      FC_ASSERT( var_interest_range <= PERCENT_100,
         "Variable Interest Range must be between 0 and 100%." );

      // === Unique Asset Options === //

      FC_ASSERT( is_valid_symbol( ownership_asset ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", ownership_asset) );

      for( account_name_type item : control_list )
      {
         FC_ASSERT( std::find( control_list.begin(), control_list.end(), item ) != control_list.end() );
      }
      for( account_name_type item : access_list )
      {
         FC_ASSERT( std::find( access_list.begin(), access_list.end(), item ) != access_list.end() );
      }

      FC_ASSERT( is_valid_symbol( access_price.symbol ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", access_price.symbol) );
      FC_ASSERT( access_price.amount >= 0,
         "Access Price must be greater than or equal to 0." );

      // === Bond Asset Options === //

      FC_ASSERT( is_valid_symbol( value.symbol ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", value.symbol) );
      FC_ASSERT( value.amount >= 0,
         "Value must be greater than or equal to 0." );
      FC_ASSERT( collateralization <= PERCENT_100,
         "Collateralization percentage must be between 0 and 100%." );
      FC_ASSERT( coupon_rate_percent <= PERCENT_100,
         "Coupon Rate Percent must be between 0 and 100%." );
      maturity_date.validate();
      FC_ASSERT( maturity_date.day == 1,
         "Maturity date should be the first of the month." );
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
      FC_ASSERT( usd_liquidity.symbol == SYMBOL_USD, 
         "Asset must have initial liquidity in the USD asset." );
      FC_ASSERT( asset_type.size() < MAX_URL_LENGTH,
         "Asset Type is invalid." );
      FC_ASSERT( fc::is_utf8( asset_type ),
         "Asset Type is invalid." );

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

   void asset_distribution_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( issuer );
      
      FC_ASSERT( is_valid_symbol( distribution_asset ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", distribution_asset ) );
      FC_ASSERT( is_valid_symbol( fund_asset ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", fund_asset ) );

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

      FC_ASSERT( distribution_rounds > 0, 
         "Distribution Rounds must be greater than 0." );
      FC_ASSERT( distribution_interval_days > 0, 
         "Distribution interval days must be greater than 0." );
      FC_ASSERT( max_intervals_missed > 0, 
         "Max Intervals missed must be greater than 0." );
      FC_ASSERT( min_input_fund_units > 0, 
         "Min Input fund units must be greater than 0." );
      FC_ASSERT( max_input_fund_units > 0, 
         "Max Input fund units must be greater than 0." );

      for( asset_unit a : input_fund_unit )
      {
         validate_account_name( a.name );
         FC_ASSERT( a.units > 0, 
            "Asset unit amount must be greater than 0." );
         FC_ASSERT( a.vesting_time > GENESIS_TIME, 
            "Vesting time must be after genesis time." );
      }

      FC_ASSERT( min_unit_ratio > 0, 
         "Minimum unit ratio must be greater than 0." );
      FC_ASSERT( max_unit_ratio > 0, 
         "Maximum unit ratio must be greater than 0." );
      FC_ASSERT( min_input_balance_units > 0, 
         "Min Input balance units must be greater than 0." );
      FC_ASSERT( max_input_balance_units > 0, 
         "Max Input balance units must be greater than 0." );
      FC_ASSERT( begin_time > GENESIS_TIME, 
         "Begin time must be after genesis time." );
   }

   void asset_distribution_fund_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( sender );
      
      FC_ASSERT( is_valid_symbol( distribution_asset ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", distribution_asset ) );
      FC_ASSERT( is_valid_symbol( amount.symbol ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol ) );
      FC_ASSERT( amount.amount > 0, 
         "Fund amount must be greater than 0." );
   }

   void asset_option_exercise_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      
      FC_ASSERT( is_valid_symbol( amount.symbol ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol ) );
      FC_ASSERT( amount.amount > 0, 
         "Fund amount must be greater than 0." );
   }

   void asset_stimulus_fund_operation::validate()const
   {
      validate_account_name( signatory );
      validate_account_name( account );
      
      FC_ASSERT( is_valid_symbol( amount.symbol ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol ) );
      FC_ASSERT( amount.amount > 0, 
         "Fund amount must be greater than 0." );
      FC_ASSERT( is_valid_symbol( stimulus_asset ), 
         "Symbol ${symbol} is not a valid symbol", ("symbol", stimulus_asset ) );
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

   void asset_collateral_bid_operation::validate()const
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
      if( !new_owner_key.valid() )
      {
         proof_of_work_operation_get_required_active_visitor vtor( a );
         ilog("Got required active authorities from proof of work: ${w}",("w",work ) );
         work.visit( vtor );
      }
   }

   /**
    * X11 and ECC Signature Based Proof of Work Function:
    * 
    * 1 - Take the hash of the Input to the proof.
    * 2 - Get the Private key from the hash.
    * 3 - Take the Hash of the private key.
    * 4 - Sign the Hash of the private key, with that same private key.
    * 5 - Take the Hash of the Signature.
    * 6 - Get the Public Key that would have signed the Hash of the Signature to result in the original signature.
    * 7 - Take the hash of both the Input to the proof and the notional public key.
    */
   void x11_proof_of_work::create( const block_id_type& prev, const account_name_type& account_name, uint64_t n )
   {
      input.miner_account = account_name;
      input.prev_block = prev;
      input.nonce = n;

      x11 key_secret = x11::hash( input );
      private_key_type private_key = fc::ecc::private_key::regenerate( fc::sha256( key_secret ) );

      x11 key_hash = x11::hash( private_key );
      signature_type signature = private_key.sign_compact( fc::sha256( key_hash ) );

      x11 sig_hash = x11::hash( signature );
      public_key_type recovered_pub_key = fc::ecc::public_key( signature, fc::sha256( sig_hash ) );

      x11 work = x11::hash( std::make_pair( input, recovered_pub_key ) );
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

   void x11_proof_of_work::validate()const
   {
      validate_account_name( input.miner_account );
      x11_proof_of_work proof;
      proof.create( input.prev_block, input.miner_account, input.nonce );

      FC_ASSERT( pow_summary == proof.pow_summary, 
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