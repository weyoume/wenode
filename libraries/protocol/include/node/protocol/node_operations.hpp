#pragma once
#include <node/protocol/base.hpp>
#include <node/protocol/block_header.hpp>
#include <node/protocol/asset.hpp>

#include <fc/utf8.hpp>
#include <fc/crypto/equihash.hpp>

namespace node { namespace protocol {

   inline void validate_account_name( const string& name )
   {
      FC_ASSERT( is_valid_account_name( name ) , "Account name ${n} is invalid", ("n", name) );
   };

   inline void validate_persona_account_name( const string& name )
   {
      FC_ASSERT( is_valid_persona_account_name( name ) , "Persona Account name ${n} is invalid", ("n", name) );
   };

   inline void validate_profile_account_name( const string& name )
   {
      FC_ASSERT( is_valid_profile_account_name( name ), "Profile Account name ${n} is invalid", ("n", name) );
   };

   inline void validate_business_account_name( const string& name )
   {
      FC_ASSERT( is_valid_business_account_name( name ), "Business Account name ${n} is invalid", ("n", name) );
   };


   inline void validate_permlink( const string& permlink )
   {
      FC_ASSERT( permlink.size() < MAX_PERMLINK_LENGTH, "permlink is too long" );
      FC_ASSERT( fc::is_utf8( permlink ), "permlink not formatted in UTF8" );
   };


   //============================//
   // === Account Operations === //
   //============================//

   /**
    * Creates a brand new WeYouMe account for signing transactions and creating posts.
    */
   struct account_create_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  registrar;                     // Account registering the new account, usually an interface.

      account_name_type                  new_account_name;              // The name of the new account.

      account_types                      account_type;                  // the type of account being created, persona, profile, business, anonymous.

      account_name_type                  referrer;                      // the account that lead to the creation of the new account, by way of referral link.

      account_name_type                  proxy;                         // Account that the new account will delegate its voting power to.

      account_name_type                  governance_account;            // Account that will be the governance account of the new account. Required for business and profile accounts.

      account_name_type                  recovery_account;              // Account that can execute a recovery operation, in the event that the owner key is compromised. 

      string                             details;                       // The account's details string

      string                             url;                           // The account's selected personal URL. 

      string                             json;                          // The JSON string of public profile information

      string                             json_private;                  // The JSON string of encrypted profile information

      authority                          owner;                         // The account authority required for changing the active and posting authorities

      authority                          active;                       // The account authority required for sending payments and trading

      authority                          posting;                       // The account authority required for posting content and voting

      public_key_type                    secure_public_key;             // The secure encryption key for conntent only visible ot this account. 

      public_key_type                    connection_public_key;         // The connection public key used for encrypting Connection level content

      public_key_type                    friend_public_key;             // The connection public key used for encrypting Friend level content

      public_key_type                    companion_public_key;          // The connection public key used for encrypting Companion level content

      optional<business_types>           business_type;                 // The type of business account being created

      optional<share_type>               officer_vote_threshold;      // The voting power required to be an active officer

      asset                              fee;                           // At least min account creation fee for stake on the new account.

      asset                              delegation;                    // Initial amount delegated to the new account

      extensions_type                    extensions;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return registrar; }
   };

   /**
    * Updates the details and authorities of an account. Requires Owner Authority.
    */
   struct account_update_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;

      optional< authority >         owner;                     // Creates a new owner authority for the account, changing the key and account auths required to sign transactions. 

      optional< authority >         active;

      optional< authority >         posting;

      public_key_type               secure_public_key;

      public_key_type               connection_public_key;

      public_key_type               friend_public_key;

      public_key_type               companion_public_key;

      string                        json;

      string                        json_private;

      string                        details;

      string                        url;

      optional<business_types>      business_type;               

      optional<share_type>          officer_vote_threshold;      

      void validate()const;
      void get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   /**
    * Accounts can purchase a membership subscription for their account to gain
    * protocol benefits:
    * 
      Top Level Membership:
      Price $250.00 per month.

      • Governance account, and Interface account revenue activation.
      • Access to WeYouMe Pro Suite for enterprise.
      • $750.00 per month of Premium Partners Content Credit.
      • WYM Stakeholder Dividend boosted by 10%.
      • Access to the WeYouMe FlashChain for interface management.
      • 30% Voting power boost.
      • Activity pool reward share boosted by 100%.
      • 20% Content reward boost.

      Mezzanine Membership:
      Price: $25.00 per month.

      • Posts eligible for WeYouMe Featured page.
      • $50.00 per month of Premium Partners Content Credit.
      • 20% Voting power boost.
      • Activity pool reward share boosted by 50%.
      • 10% Content reward boost.
      • 10% refund on marketplace fees, premium content, subscription content, and exchange trading fees.

      Standard Membership:
      Price: $2.50 per month.

      • Disable all Advertising.
      • $2.50 per month of Premium Partners Content Credit.
      • 10% Voting power boost.
      • Ability to join the Developer, Advocate and Marketing pools.
      • Activity pool reward share boosted by 50%.
    */
   struct account_membership_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      account;

      membership_types       membership_type;

      asset                  fee;

      uint16_t               months;

      account_name_type      interface;

      bool                   recurring = true;

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   /**
    * Votes for an account to become an executive of a business account, weighted according to equity holdings.
    */
   struct account_vote_executive_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            account;               // Account creating the executive vote. 

      account_name_type            business_account;      // Business account that the executive is being voted for.

      account_name_type            executive;             // Name of executive being voted for.

      executive_types              role;                  // Role of the executive.

      bool                         approved = true;       // True to add, false to remove. 

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return business_account; }
   };


   /**
    * Votes for an account to become an officer of a business account, weighted according to equity holdings.
    */
   struct account_vote_officer_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            account;               // Name of the voting account.

      account_name_type            business_account;      // Business account.

      account_name_type            officer;               // Name of officer being voted for.

      bool                         approved = true;       // True to add, false to remove. 

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return business_account; }
   };


   /**
    * Requests that an account be added to the membership of a business account.
    */
   struct account_member_request_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            account;              // Business account or the member account, or an officer of the business account.

      account_name_type            business_account;     // Business account that the member is being added to.

      string                       message;              // Encrypted Message to the business members requesting membership.

      bool                         requested = true;     // True to add, false to remove. 

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   /**
    * Invites an account to be a member of a business account.
    */
   struct account_member_invite_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            account;                   // Business account or the member account, or an officer of the business account.

      account_name_type            business_account;          // Business account that the member is being added to.

      account_name_type            member;                    // Name of member being added

      string                       message;                   // Encrypted Message to the account being invited.

      string                       encrypted_business_key;    // Encrypted Copy of the private key of the business.

      bool                         invited = true;            // True to add, false to remove. 

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   /**
    * Accepts an account's request to be added as a business account member.
    */
   struct account_accept_request_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            account;                   // Account that is accepting the request to add a new member.

      account_name_type            business_account;          // Business account that the member is being added to.

      account_name_type            member;                    // Name of member being accepted.

      string                       encrypted_business_key;    // Encrypted Copy of the private key of the business.

      bool                         accepted = true;           // True to accept, false to reject. 

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   /**
    * Accepts an invitation to be added as a business account member.
    */
   struct account_accept_invite_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            account;              // Account accepting the invitation

      account_name_type            business_account;     // Business account that the account was invited to.

      bool                         accepted = true;      // True to accept, false to reject.

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };

   /**
    * Removes a member from the membership of a business account.
    */
   struct account_remove_member_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            account;              // Business account or an executive of the business account.

      account_name_type            business_account;     // Business account that the member is being added to.

      account_name_type            member;               // Name of member being accepted.

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };

   /**
    * Blacklists an account or asset
    */
   struct account_update_list_operation : public base_operation
   {
      account_name_type                 signatory;

      account_name_type                 account;              // Name of account.

      optional<account_name_type>       listed_account;       // Name of account being added to a black or white list.

      optional<asset_symbol_type>       listed_asset;         // Name of asset being added to a black or white list.

      bool                              blacklisted = true;   // True to add to blacklist, false to remove. 

      bool                              whitelisted = true;   // True to add to whitelist, false to remove. 

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   /**
    * All accounts with a voting power can vote for or against any witness.
    * If a proxy is specified then all existing votes are removed.
    */
   struct account_witness_vote_operation : public base_operation
   {
      account_name_type               signatory;

      account_name_type               account;

      account_name_type               witness;

      bool                            approve = true;   // True to create vote, false to remove vote.

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct account_update_proxy_operation : public base_operation
   {
      account_name_type          signatory;

      account_name_type          account;

      account_name_type          proxy;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   /**
    * All account recovery requests come from a listed recovery account. This
    * is secure based on the assumption that only a trusted account should be
    * a recovery account. It is the responsibility of the recovery account to
    * verify the identity of the account holder of the account to recover by
    * whichever means they have agreed upon. The blockchain assumes identity
    * has been verified when this operation is broadcast.
    *
    * This operation creates an account recovery request which the account to
    * recover has 24 hours to respond to before the request expires and is
    * invalidated.
    *
    * There can only be one active recovery request per account at any one time.
    * Pushing this operation for an account to recover when it already has
    * an active request will either update the request to a new new owner authority
    * and extend the request expiration to 24 hours from the current head block
    * time or it will delete the request. To cancel a request, simply set the
    * weight threshold of the new owner authority to 0, making it an open authority.
    *
    * Additionally, the new owner authority must be satisfiable. In other words,
    * the sum of the key weights must be greater than or equal to the weight
    * threshold.
    *
    * This operation only needs to be signed by the the recovery account.
    * The account to recover confirms its identity to the blockchain in
    * the recover account operation.
    */
   struct request_account_recovery_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  recovery_account;       // The recovery account is listed as the recovery account on the account to recover.

      account_name_type                  account_to_recover;     // The account to recover. This is likely due to a compromised owner authority.

      authority                          new_owner_authority;    // The new owner authority the account to recover wishes to have. This is secret
                                                // known by the account to recover and will be confirmed in a recover_account_operation

      extensions_type                    extensions;             // Extensions. Not currently used.

      const account_name_type& get_creator_name() const { return recovery_account; }
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }

      void validate() const;
   };


   /**
    * Recover an account to a new authority using a previous authority and verification
    * of the recovery account as proof of identity. This operation can only succeed
    * if there was a recovery request sent by the account's recover account.
    *
    * In order to recover the account, the account holder must provide proof
    * of past ownership and proof of identity to the recovery account. Being able
    * to satisfy an owner authority that was used in the past 30 days is sufficient
    * to prove past ownership. The get_owner_history function in the database API
    * returns past owner authorities that are valid for account recovery.
    *
    * Proving identity is an off chain contract between the account holder and
    * the recovery account. The recovery request contains a new authority which
    * must be satisfied by the account holder to regain control. The actual process
    * of verifying authority may become complicated, but that is an application
    * level concern, not a blockchain concern.
    *
    * This operation requires both the past and future owner authorities in the
    * operation because neither of them can be derived from the current chain state.
    * The operation must be signed by keys that satisfy both the new owner authority
    * and the recent owner authority. Failing either fails the operation entirely.
    *
    * If a recovery request was made inadvertantly, the account holder should
    * contact the recovery account to have the request deleted.
    *
    * The two step combination of the account recovery request and recover is
    * safe because the recovery account never has access to secrets of the account
    * to recover. They simply act as an on chain endorsement of off chain identity.
    * In other systems, a fork would be required to enforce such off chain state.
    * Additionally, an account cannot be permanently recovered to the wrong account.
    * While any owner authority from the past 30 days can be used, including a compromised
    * authority, the account can be continually recovered until the recovery account
    * is confident a combination of uncompromised authorities were used to
    * recover the account. The actual process of verifying authority may become
    * complicated, but that is an application level concern, not the blockchain's
    * concern.
    */
   struct recover_account_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account_to_recover;        // The account to be recovered

      authority             new_owner_authority;       // The new owner authority as specified in the request account recovery operation.

      authority             recent_owner_authority;    // A previous owner authority that the account holder will use to prove past ownership of the account to be recovered.

      extensions_type       extensions;                // Extensions. Not currently used.

      void get_required_authorities( vector< authority >& a )const
      {
         a.push_back( new_owner_authority );
         a.push_back( recent_owner_authority );
      }
      const account_name_type& get_creator_name() const { return account_to_recover; }
      void validate() const;
   };


   /**
    *  This operation allows recovery_account to change account_to_reset's owner authority to
    *  new_owner_authority after 60 days of inactivity.
    */
   struct reset_account_operation : public base_operation 
   {
      account_name_type                  signatory;

      account_name_type reset_account;

      account_name_type account_to_reset;

      authority         new_owner_authority;

      void get_required_active_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return reset_account; }
      void validate()const;
   };


   /**
    * This operation allows 'account' owner to control which account has the power
    * to execute the 'reset_account_operation' after a specified duration.
    */
   struct set_reset_account_operation : public base_operation 
   {
      account_name_type                  signatory;

      account_name_type         account;

      account_name_type         current_reset_account;

      account_name_type         reset_account;

      int64_t                   days = 7;

      void validate()const;
      void get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   /**
    * Each account lists another account as their recovery account.
    * The recovery account has the ability to create account_recovery_requests
    * for the account to recover. An account can change their recovery account
    * at any time with a 30 day delay. This delay is to prevent
    * an attacker from changing the recovery account to a malicious account
    * during an attack. These 30 days match the 30 days that an
    * owner authority is valid for recovery purposes.
    *
    * On account creation the recovery account is set either to the creator of
    * the account (The account that pays the creation fee and is a signer on the transaction)
    * or to the empty string if the account was mined. An account with no recovery
    * has the top voted witness as a recovery account, at the time the recover
    * request is created. Note: This does mean the effective recovery account
    * of an account with no listed recovery account can change at any time as
    * witness vote weights. The top voted witness is explicitly the most trusted
    * witness according to stake.
    */
   struct change_recovery_account_operation : public base_operation
   {
      account_name_type         signatory;

      account_name_type         account_to_recover;     ///< The account that would be recovered in case of compromise

      account_name_type         new_recovery_account;   ///< The account that creates the recover request

      extensions_type           extensions;             ///< Extensions. Not currently used.

      void get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account_to_recover; }
      void validate() const;
   };


   struct challenge_authority_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  challenger;

      account_name_type                  challenged;

      bool                               require_owner = false;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return challenger; }
   };


   struct prove_authority_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  challenged;

      bool                               require_owner = false;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ if( !require_owner ) a.insert( signatory ); }
      void get_required_owner_authorities( flat_set<account_name_type>& a )const{  if(  require_owner ) a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return challenged; }
   };


   /**
    * Removes an account's ability to vote in perpetuity for the
    * purposes of ensuring that funds held in trust that are owned by third parties
    * are not used for undue influence over the network
    */
   struct decline_voting_rights_operation : public base_operation
   {
      account_name_type       signatory;

      account_name_type       account;

      bool                    decline = true;

      void get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
      void validate() const;
   };

   /**
    * Initiates a 3 step connection exchange betwen two accounts, for the purposes
    * of exchanging encrypted connection keys that can be used to decrypt
    * private posts made by the account, and enables the creation of private 
    * message transactions between the two accounts.
    * 1 - Alice sends Request txn.
    * 2 - Bob sends Acceptance txn with encrypted private key.
    * 3 - Alice sends returns with Acceptance txn with encrypted private key.
    */
   struct connection_request_operation : public base_operation 
   {
      account_name_type             signatory;

      account_name_type             account;                        // Account sending the request

      account_name_type             requested_account;              // Account that is being requested to connect

      connection_types              connection_type;                // Type of connection level

      string                        message;                        // Message attached to the request, encrypted with recipient's secure public key.

      bool                          requested = true;               // Set true to request, false to cancel request.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };

   /**
    * Accepts an incoming connection request by providing an encrypted posting
    * key, that is decryptable by the recipient that initiated the request.
    * This operation assumes that the account includes a valid encrypted private key
    * and that the initial account confirms by returning an additional acceptance transaction.
    * The protocol cannot ensure the reciprocity of the original sender, or
    * the accuracy of the private keys included. 
    */
   struct connection_accept_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;               // Account accepting the request.

      account_name_type             requesting_account;    // Account that originally requested to connect.

      string                        connection_id;         // Unique uuidv4 for the connection, for local storage of decryption key.

      connection_types              connection_type;       // Type of connection level.

      string                        encrypted_key;         // The private connection key of the user, encrypted with the public secure key of the requesting account.

      bool                          connected = true;      // Set true to connect, false to delete.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };

   /**
    * Enables an account to follow another account, 
    * adding it to the accounts follow object and displaying
    * posts and shares from the account in their home feed.
    */
   struct account_follow_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             follower;

      account_name_type             following;

      string                        details;              // Json string containing details of the following connection "blog / ignore".

      account_name_type             interface;            // Name of the interface account that was used to broadcast the transaction. 

      bool                          followed = true;      // Set true to follow, false to delete.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return follower; }
   };


   /**
    * Activity rewards can be claimed by each account 
    * every day for a share of the activity reward pool.
    * To be eligible, accounts need:
    * 1 - A balance of at least one EQUITY_ASSET.
    * 2 - A recent comment transaction with an above median number of votes and views for that day.
    * 3 - A recent vote transaction.
    * 4 - A recent view transaction.
    * 5 - At least 10 witness votes from their account.
    */
   struct activity_reward_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;        // Name of the account claiming the reward.

      string                        permlink;       // Permlink of the users recent post in the last 24h.

      uint32_t                      view_id;        // Recent comment id viewed in the last 24h.

      uint32_t                      vote_id;        // Recent comment id voted on in the last 24h.

      account_name_type             prime_witness;  // Name of the user's highest voted witness.

      account_name_type             interface;      // Name of the interface account that was used to broadcast the transaction. 

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   //===========================//
   // === Network Operations ===//
   //===========================//

   /**
    * Creates or updates a network officer object for a member.
    * Network officers receive a reward dsitribution from each block
    * based on the amount of votes they have received from supporters of thier
    * work to develop improvements the protocol or its interfaces, 
    * market it to new users, and advocate it to businesses and organisations. 
    */
   struct update_network_officer_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;           // Name of the member's account

      network_officer_types         officer_type;      // The type of network officer that the account serves as. 

      string                        details;           // Information about the network officer and their work

      string                        url;               // The officers's description URL explaining their details. 

      string                        json;              // Additonal information about the officer.

      bool                          active = true;     // Set true to activate governance account, false to deactivate. 
         
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
      void validate() const;
   };

   /**
    * Votes to support reward distribution to a network officer
    * for the work they have done.
    */
   struct network_officer_vote_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;               // The name of the account voting for the officer.

      account_name_type              network_officer;       // The name of the network officer being voted for.

      bool                           approved = true;       // True if approving, false if removing vote.

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
      void validate() const;
   };

   struct executive_officer_set 
   {
      account_name_type                 CHIEF_EXECUTIVE_OFFICER;    // Overall leader of Executive team.

      account_name_type                 CHIEF_OPERATING_OFFICER;    // Manages communications and coordination of team.

      account_name_type                 CHIEF_FINANCIAL_OFFICER;    // Oversees Credit issuance and salaries.

      account_name_type                 CHIEF_DEVELOPMENT_OFFICER;  // Oversees protocol development and upgrades.

      account_name_type                 CHIEF_TECHNOLOGY_OFFICER;   // Manages front end interface stability and network infrastructure.

      account_name_type                 CHIEF_SECURITY_OFFICER;     // Manages security of servers, funds, wallets, keys, and cold storage.

      account_name_type                 CHIEF_GOVERNANCE_OFFICER;   // Manages Governance account moderation teams.

      account_name_type                 CHIEF_MARKETING_OFFICER;    // Manages public facing communication and campaigns.
      
      account_name_type                 CHIEF_DESIGN_OFFICER;       // Oversees graphical design and User interface design.

      account_name_type                 CHIEF_ADVOCACY_OFFICER;     // Coordinates advocacy efforts and teams.

      bool allocated()const
      {
         if( CHIEF_EXECUTIVE_OFFICER.size() &&
            CHIEF_OPERATING_OFFICER.size() &&
            CHIEF_FINANCIAL_OFFICER.size() &&
            CHIEF_DEVELOPMENT_OFFICER.size() &&
            CHIEF_TECHNOLOGY_OFFICER.size() &&
            CHIEF_SECURITY_OFFICER.size() &&
            CHIEF_GOVERNANCE_OFFICER.size() &&
            CHIEF_MARKETING_OFFICER.size() &&
            CHIEF_DESIGN_OFFICER.size() &&
            CHIEF_ADVOCACY_OFFICER.size()
         )
         {
           return true;
         }
         else
         {
            return false;
         }
      }
   };


   /**
    * Creates or updates a executive board object for a development and administration team of a business account.
    * Executive boards enable the issuance of network credit asset for operating a
    * multifaceted development and marketing team for the protocol.
    * Executive boards must:
    * 1 - Hold a minimum balance of 100 Equity assets.
    * 2 - Operate active supernode with at least 100 daily active users.
    * 3 - Operate active interface account with at least 100 daily active users.
    * 4 - Operate an active governance account with at least 100 subscribers.
    * 5 - Have at least 3 members or officers that are top 50 network officers, 1 from each role.
    * 6 - Have at least one member or officer that is an active top 50 witness.
    */
   struct update_executive_board_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;           // Name of the account updating the executive board.

      account_name_type             executive;         // Name of the Executive board account being updated.

      asset                         budget;            // Total amount of Credit asset requested for team compensation and funding.

      string                        details;           // Information about the board and thier team.

      string                        url;               // The teams's description URL explaining their details. 

      string                        json;              // Additonal information about the executive board.

      bool                          active = true;     // Set true to activate the board, false to deactivate. 
         
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
      void validate() const;
   };

   /**
    * Votes to support an executive board
    */
   struct executive_board_vote_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;               // The name of the account voting for the board.

      account_name_type              executive_board;       // The name of the executive board being voted for.

      bool                           approved = true;       // True if approving, false if removing vote.

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
      void validate() const;
   };


   /**
    * Creates or updates a governance account for a member.
    * Provides moderation and profile account services to the network 
    * in exchange for a share of the revenue generated by thier subscribers.
    */
   struct update_governance_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;           // Name of the governance account

      string                        details;           // Information about the governance account's filtering and tagging policies

      string                        url;               // The governance account's description URL explaining their details. 

      string                        json;              // Additonal information about the governance account policies.

      bool                          active = true;     // Set true to activate governance account, false to deactivate. 
         
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
      void validate() const;
   };

   /** 
    * Adds a governance account to the subscription set of the
    * account, causing its content moderation tags to apply to
    * the account in interfaces.
    * This allows an account to opt in to the moderation and enforcement 
    * policies provided by the governance account, and to 
    * its default settings for whitelisting and blacklisting mediators, assets,
    * boards, authors, and interfaces. 
    */
   struct subscribe_governance_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;             // The account subscribing to the governance address
      
      account_name_type             governance_account;  // The name of the governance account

      bool                          subscribe = true;    // True if subscribing, false if unsubscribing.

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
      void validate() const;
   };

   /**
    * Creates or updates an interface object for an application developer.
    * Enables an application to provide advertising inventory on the 
    * advertising exchange, in return for payments from advertisers, and
    * to earn a share of fees generated from memberships, trading fees,
    * premium content sales, and marketplace fees. 
    */
   struct update_interface_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;           // Name of the member's account

      string                        details;           // Information about the interface, and what they are offering to users

      string                        url;               // The interfaces's URL

      string                        json;              // Additonal information about the interface.

      bool                          active = true;     // Set true to activate the interface, false to deactivate. 
         
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
      void validate() const;
   };


   /**
    * Creates or updates a supernode object for an infrastructure provider.
    * Supernodes are required to:
    * 1 - Have a minimum stake of 1 Equity and 100 Coin ( 10 Equity and 1000 Coin receive doubled reward share ).
    * 2 - Operate a full archive node.
    * 3 - Operate a public API endpoint to their node that provides all network API and transaction broadcasting functions. 
    * 4 - Operate a public IPFS gateway endpoint for file upload and download.
    * 5 - Operate a public Bittorrent Seed API endpoint.
    * 6 - Operate an authentication API endpoint for transaction signing.
    * 7 - Operate a notification API endpoint for receiving account notifications.
    * 
    * Supernodes receive rewards from the network proportionately with their 7 day stake weighted average file views.
    * Users dynamically download IPFS media files from the Supernodes with the lowest ping via thier gateway endpoint.
    * Users upload IPFS media files to multiple active Supernodes via their gateway endpoints.
    * Users upload videos files to mutliple bittorrent seed nodes for distribution via webtorrent. 
    * User select the Node, Auth and notification API endpoints with the lowest ping when using the WeYouMe applications. 
    * Supernodes need to remain in the Supernode reward pool for 24 consecutive hours before being eligible for reward distribution.
    * 
    * Open Problem: Ensure that Interfaces correctly attribute Supernodes in view transactions when using their APIs.
    * 
    */
   struct update_supernode_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;                     // Name of the member's account.

      string                        details;                     // Information about the supernode, and the range of storage and node services they operate.

      string                        url;                         // The supernode's reference URL.

      string                        node_api_endpoint;           // The Full Archive node public API endpoint of the supernode.

      string                        notification_api_endpoint;   // The Notification API endpoint of the Supernode. 

      string                        auth_api_endpoint;           // The Transaction signing authentication API endpoint of the supernode.

      string                        ipfs_endpoint;               // The IPFS file storage API endpoint of the supernode.

      string                        bittorrent_endpoint;         // The Bittorrent Seed Box endpoint URL of the Supernode. 

      string                        json;                        // Additonal information about the interface.

      bool                          active = true;               // Set true to activate the interface, false to deactivate. 
         
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
      void validate() const;
   };


   /**
    * Creates a new community enterprise proposal, which provides funding 
    * to projects that work to benefit the network by either 
    * distributing payments based on milestone completion, 
    * to the winner of a competition, or purchases an
    * investment in a business account's cryptoequity asset. 
    * A proposal becomes active when its first milestone is approved.
    */
   struct create_community_enterprise_operation : public base_operation
   {
      account_name_type                                 signatory;

      account_name_type                                 creator;             // The name of the account that created the community enterprise proposal.

      string                                            enterprise_id;       // UUIDv4 referring to the proposal. 

      proposal_types                                    proposal_type;       // The type of proposal, determines release schedule

      flat_map< account_name_type, uint16_t >           beneficiaries;       // Set of account names and percentages of budget value. Should not include the null account.

      vector< pair < string, uint16_t > >               milestones;          // Ordered vector of release milestone descriptions and percentages of budget value.

      fc::optional < asset_symbol_type >                investment;          // Symbol of the asset to be purchased with the funding if the proposal is investment type. 

      string                                            details;             // The proposals's details description. 

      string                                            url;                 // The proposals's reference URL. 

      string                                            json;                // Json metadata of the proposal. 

      time_point                                        begin;               // Enterprise proposal start time.

      uint16_t                                          duration;            // Number of days that the proposal will be paid for.

      asset                                             daily_budget;        // Daily amount of Core asset requested for project compensation and funding.

      asset                                             fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ); // Amount of Core asset paid to community fund to apply.

      bool                                              active = true;       // True to set the proposal to activate, false to deactivate an existing proposal and delay funding. 
         
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return creator; }
      void validate() const;
   };

   /**
    * Claims a milestone from a community enterprise proposal
    * This requests to release the funds in the pending budget to the beneficiaries
    * up to the percentage allocated to the milestone.
    */
   struct claim_enterprise_milestone_operation : public base_operation
   {
      account_name_type         signatory;

      account_name_type         creator;               // The name of the account that created the proposal.

      string                    enterprise_id;         // UUIDv4 referring to the proposal. 

      uint16_t                  milestone = 0;         // Number of the milestone that is being claimed as completed. Number 0 for initial acceptance. 

      string                    details;               // Description of completion of milestone, with supporting evidence.
         
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return creator; }
      void validate() const;
   };

   /**
    * Approves a milestone claim from a community enterprise proposal.
    * This releases the funds that are in the pending budget to the proposal's beneficaries.
    * Community Enterprise proposals need to be approved by: 
    *       - Approvals from at least 5 of the Top 50 witnesses, with a combined voting power of at least 10% of the total voting power.
    * AND
    *       - At least 20 total approvals, from accounts with a total combined voting power of at least 10% of total voting power. 
    */
   struct approve_enterprise_milestone_operation : public base_operation
   {
      account_name_type         signatory;

      account_name_type         account;               // Account approving the milestone, must be a current top 50 witness, developer, marketer, or advocate.

      account_name_type         creator;               // The name of the account that created the proposal.

      string                    enterprise_id;         // UUIDv4 referring to the proposal. 

      uint16_t                  milestone = 0;         // Number of the milestone that is being approved as completed.

      string                    details;               // Description of completion of milestone, with supporting evidence.

      bool                      approved = true;       // True to claim the milestone amount, false to remove approval. 
         
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
      void validate() const;
   };
   

   //=====================================//
   // === Post and Comment Operations === //
   //=====================================//
   

   struct comment_operation : public base_operation
   {
      account_name_type           signatory;

      account_name_type           author;               // Name of the account that created the post.

      string                      permlink;             // Unique identifing string for the post.

      post_types                  post_type;            // Type of post being created, text, image, article, video, audio, file, etc.

      string                      body;                 // String containing text for display when the post is opened.

      string                      media;                // String containing a display image or video file as an IPFS file hash.

      string                      language;             // String containing the two letter ISO language code of the native language of the author. 

      board_name_type             board;                // The name of the board to which the post is uploaded to.

      connection_types            privacy;              // Type of privacy setting for encryption levels to connections.

      account_name_type           interface;            // Name of the interface application that broadcasted the transaction. 

      rating_types                rating;               // User nominated rating as to the maturity of the content, and display sensitivity. 

      account_name_type           parent_author;        // Account that created the post this post is replying to, empty if root post. 

      string                      parent_permlink;      // permlink of the post this post is replying to, empty if root post. 

      flat_set< string >          tags;                 // Set of string tags for sorting the post by.

      string                      json;                 // json string of additional interface specific data relating to the post. 

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return author; }
   };

   struct beneficiary_route_type
   {
      beneficiary_route_type() {}
      beneficiary_route_type( const account_name_type& a, const uint16_t& w ) : account( a ), weight( w ){}

      account_name_type account;

      uint16_t          weight;

      // For use by std::sort such that the route is sorted first by name (ascending)
      bool operator < ( const beneficiary_route_type& o )const { return account < o.account; }
   };

   struct comment_payout_beneficiaries
   {
      vector< beneficiary_route_type > beneficiaries;

      void validate()const;
   };

   typedef static_variant<
            comment_payout_beneficiaries
           > comment_options_extension;

   typedef flat_set< comment_options_extension > comment_options_extensions_type;

   /**
    *  Authors of posts may not want all of the benefits that come from creating a post. This
    *  operation allows authors to update properties associated with their post.
    *  The max_accepted_payout may be decreased, but never increased.
    *  The percent_liquid may be decreased, but never increased.
    */
   struct comment_options_operation : public base_operation
   {
      account_name_type           signatory;

      account_name_type           author;
      
      string                      permlink;

      asset                       max_accepted_payout = asset( 1000000000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );  // USD value of the maximum payout this post will receive
      
      uint16_t                    percent_liquid = PERCENT_100;                                                  // Percentage of reward to keep liquid, remaining received as a staked balance
      
      bool                        allow_votes = true;                                                            // Allows a post to receive votes;
      
      bool                        allow_curation_rewards = true;                                                 // allows voters to recieve curation rewards. Rewards return to reward fund.

      comment_options_extensions_type extensions;

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return author; }
   };

   /**
    * Creates a private encrypted message between two accounts
    */
   struct message_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             sender;         // The account sending the message.

      account_name_type             recipient;      // The receiving account of the message.

      string                        message;        // Encrypted ciphertext of the message being sent. 

      string                        id;             // uuidv4 uniquely identifying the message for local storage.

      time_point                    time;           // Time the message was sent.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return sender; }
   };

   
   struct vote_operation : public base_operation
   {
      account_name_type    signatory;

      account_name_type    voter;           // Name of the voting account.

      account_name_type    author;          // Name of the account that created the post being voted on.

      string               permlink;        // Permlink of the post being voted on.

      int16_t              weight = 0;      // Percentage weight of the voting power applied to the post.

      account_name_type    interface;       // Name of the interface account that was used to broadcast the transaction. 

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return voter; }
   };

   /** 
    * Views the post, which increases the post's reward earnings, and 
    * nominates an interface through which the post was viewed,
    * and nominates a supernode that served the data through IPFS.
    */
   struct view_operation : public base_operation
   {
      account_name_type    signatory;

      account_name_type    viewer;           // Name of the viewing account.

      account_name_type    author;           // Name of the account that created the post being viewed. 

      string               permlink;         // Permlink to the post being viewed.

      account_name_type    interface;        // Name of the interface account that was used to broadcast the transaction and view the post. 

      account_name_type    supernode;        // Name of the supernode account that served the IPFS file data in the post. 

      bool                 viewed = true;    // True if viewing the post, false if removing view object.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return viewer; }
   };

   /**
    * Shares a post to the account's feed, so that all accounts that
    * follow them can see the post. Increases the share count of the post
    * and expands its visibility.
    */
   struct share_operation : public base_operation
   {
      account_name_type    signatory;

      account_name_type    sharer;           // Name of the viewing account.

      account_name_type    author;           // Name of the account that created the post being shared. 

      string               permlink;         // Permlink to the post being shared.

      account_name_type    interface;        // Name of the interface account that was used to broadcast the transaction and share the post.

      bool                 shared = true;    // True if sharing the post, false if removing share.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return sharer; }
   };


   /**
    * Applies a tag to a post for the purposes of filtering from interfaces based on the 
    * content included in the post. Accounts that list @moderator as a board moderator or
    * governance account apply the tag to the post for content management.
    * They additionally can suggest a higher rating level if the rating selected
    * by the author was inaccurate. 
    */
   struct moderation_tag_operation : public base_operation
   {
      account_name_type           signatory;

      account_name_type           moderator;          // Account creating the tag: can be a governance address or a board moderator. 

      account_name_type           author;             // Author of the post being tagged.

      string                      permlink;           // Permlink of the post being tagged.

      flat_set< string >          tags;               // Set of tags to apply to the post for selective interface side filtering.

      rating_types                rating;             // Newly proposed rating for the post

      string                      details;            // String explaining the reason for the tag to the author

      bool                        filter;             // True if the post should be filtered from the board and governance account subscribers

      bool                        applied = true;     // True if applying the tag, false if removing the tag.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return moderator; }
   };


   //==========================//
   // === Board Operations === //
   //==========================//


   /**
    * Creates a new board for collecting posts about a specific topic, 
    * or a group for discussion among a group of people
    * Boards have 4 Types: Board, Group, Event, and Store
    * they have 4 Privacy options: Open, Public, Private, and Exclusive.
    * 
    * Boards contain a collective public key for encrypting posts with
    * and the private key is shared with newly added members when they join.
    * 
    * TODO: Private key sharing object
    */
   struct board_create_operation : public base_operation
   {
      account_name_type           signatory;

      account_name_type           founder;                    // The account that created the board, able to add and remove administrators.

      board_name_type             name;                       // Name of the board.

      board_types                 board_type;                 // Type of board to create.

      board_privacy_types         board_privacy;              // Type of board to create.

      public_key_type             board_public_key;           // Key used for encrypting and decrypting posts. Private key shared with accepted members.

      string                      json;                       // Public plaintext json information about the board, its topic and rules.

      string                      json_private;               // Private ciphertext json information about the board.

      string                      details;                    // Details of the board, describing what it is for.

      string                      url;                        // External reference URL.

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return founder; }
   };



   struct board_update_operation : public base_operation
   {
      account_name_type           signatory;

      account_name_type           account;                   // Account updating the board.

      board_name_type             board;                     // Name of the board.

      board_types                 board_type;                // Type of board to create.

      public_key_type             board_public_key;          // Key used for encrypting and decrypting posts. Private key shared with accepted members.

      string                      json;                      // Public plaintext json information about the board, its topic and rules.

      string                      json_private;              // Private ciphertext json information about the board. Encrypted with board public key.

      string                      details;                   // Details of the board, describing what it is for.

      string                      url;                       // External reference URL.

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct board_add_mod_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;           // Account of an administrator of the board.

      board_name_type                board;             // Board that is being changed.

      account_name_type              moderator;         // New moderator account.

      bool                           added = true;      //  True when adding a new moderator, false when removing.

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct board_add_admin_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;             // Account of the founder of the board.

      board_name_type                board;               // Board that is being changed.

      account_name_type              admin;               // New administrator account.

      bool                           added = true;        //  True when adding a new moderator, false when removing.

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct board_vote_mod_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;           // Account of a member of the board.

      board_name_type                board;             // Board of the member.

      account_name_type              moderator;         // Moderator account.

      bool                           approved = true;      // True when voting for the moderator, false when removing.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct board_transfer_ownership_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;        // Account that created the board.

      board_name_type                board;          // Board that is being changed.

      account_name_type              new_founder;    // Account of the new founder.

      void validate()const;
      void get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct board_join_request_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;               // Account that wants to join the board.

      board_name_type                board;                 // Board that is being changed.

      string                         message;               // Message attatched to the request, encrypted with the boards public key. 

      bool                           requested = true;      // Set true to request, false to cancel request.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct board_join_invite_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;              // Account sending the invitation.

      account_name_type              member;               // New board member account being invited.

      board_name_type                board;                // Board that is the member is being invited to.

      string                         message;              // Message attatched to the request, encrypted with the member's secure public key.

      string                         board_key;            // The Board Private Key, encrypted with the member's secure public key.

      bool                           invited = true;       // Set true to request, false to cancel request.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct board_join_accept_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;            // A Moderator of the board.

      account_name_type              member;             // Account to accept into the board.

      board_name_type                board;              // Board that is being joined.

      string                         board_key;          // The Board Private Key, encrypted with the member's secure public key.

      bool                           accepted = true;    // True to accept request, false to reject request.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct board_invite_accept_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;             // A new member of the board.

      board_name_type                board;               // Board that is being joined.

      bool                           accepted = true;     // True to accept invite, false to reject invite.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct board_remove_member_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;        // Either the member of the board leaving OR a moderator of the board removing the member.

      account_name_type              member;         // Account to be removed from the board membership.

      board_name_type                board;          // Board that is being joined.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct board_blacklist_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;               // A moderator or admin of the board blacklisting member.

      account_name_type              member;                // Account to be blacklisted from interacting with the board.

      board_name_type                board;                 // Board that member is being blacklisted from.

      bool                           blacklisted = true;    // Set to true to add account to blacklist, set to false to remove from blacklist. 

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct board_subscribe_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type              account;             // Account that wants to subscribe to the board.

      board_name_type                board;               // Board to suscribe to.

      account_name_type              interface;           // Name of the interface account that was used to broadcast the transaction and subscribe to the board.

      bool                           subscribed = true;   // true if subscribing, false if unsubscribing. 

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   //================================//
   // === Advertising Operations === //
   //================================//

   struct ad_creative_operation : public base_operation 
   {
      account_name_type           signatory;

      account_name_type           author;           // Account publishing the ad creative.

      format_types                format_type;       // The type of formatting used for the advertisment, determines the interpretation of the creative.

      string                      creative_id;       // uuidv4 referring to the creative

      string                      objective;         // The name of the object being advertised, the link and CTA destination of the creative.

      string                      creative;          // IPFS link to the Media to be displayed, image or video.

      string                      json;              // json string of creative metadata for display.

      bool                        active = true;     // True if the creative is enabled for active display, false to deactivate.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return author; }
   };

   struct ad_campaign_operation : public base_operation 
   {
      account_name_type                signatory;

      account_name_type                account;           // Account creating the ad campaign.

      string                           campaign_id;       // uuidv4 to refer to the campaign.

      asset                            budget;            // Total expenditure of the campaign.

      time_point                       begin;             // Beginning time of the campaign. Bids cannot be created before this time.

      time_point                       end;               // Ending time of the campaign. Bids cannot be created after this time.

      string                           json;              // json metadata for the campaign.

      flat_set<account_name_type>      agents;            // Set of Accounts authorized to create bids for the campaign.

      account_name_type                interface;         // Interface that facilitated the purchase of the advertising campaign.

      bool                             active = true;     // True if the campaign is enabled for bid creation, false to deactivate and reclaim the budget.

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };

   struct ad_inventory_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  provider;       // Account of an interface offering ad supply.

      string                             inventory_id;   // uuidv4 referring to the inventory offering.

      metric_types                       metric;         // Type of expense metric used.

      asset                              min_price;      // Minimum bidding price per metric.

      uint32_t                           inventory;      // Total metrics available.

      string                             json;           // json metadata for the inventory.

      flat_set< string >                 audience;       // List of ad audience_ids, each containing a set of usernames of viewing accounts in their userbase.

      flat_set<account_name_type>        agents;         // Set of Accounts authorized to create delivery transactions for the inventory.

      bool                               active = true;  // True if the inventory is enabled for display, false to deactivate.

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return provider; }
   };

   struct ad_audience_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  account;        // Account creating the audience set, either an advertiser or supplier.

      string                             audience_id;    // uuidv4 referring to the inventory offering.

      string                             json;           // json metadata for the audience.

      flat_set< account_name_type >      audience;       // List of usernames of viewing accounts.

      bool                               active = true;  // True if the audience is enabled for reference, false to deactivate.

      void validate()const;
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   struct ad_bid_operation : public base_operation
   {
      account_name_type                signatory;

      account_name_type                bidder;                // Account that created the ad budget, or an agent of the campaign.
      
      string                           bid_id;                // Bid uuidv4 for referring to the bid.

      account_name_type                account;               // Account that created the campaign that the bid is directed towards. 

      string                           campaign_id;           // Ad campaign uuidv4 to utilise for the bid.

      string                           creative_id;           // Creative uuidv4 offering to bid on.

      account_name_type                provider;              // Account offering inventory supply.

      string                           inventory_id;          // Inventory uuidv4 offering to bid on.

      asset                            bid_price;             // Price offered per metric.

      uint32_t                         inventory_requested;   // Maximum total metrics requested.

      flat_set< string >               included_audiences;    // List of desired audiences for display acceptance. Will be merged into a combined audience

      flat_set< string >               excluded_audiences;    // List of audiences to remove all members from the combined bid audience.

      string                           json;                  // json metadata for the combined audience if created.

      time_point                       expiration;            // Time the the bid is valid until, bid is cancelled after this time if not filled. 

      bool                             active = true;         // True if the bid is open for delivery, false to cancel.

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return bidder; }
   };


   struct ad_deliver_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  account;                // Account creating the delivery, must be an agent of the inventory bidded on.

      account_name_type                  bidder;                 // Account that created the bid on available supply.

      string                             bid_id;                 // Bid uuidv4 for referring to the bid.

      asset                              delivery_price;         // Price charged per metric. Must be equal to or less than bid price. [Typically the Price of second highest bidder]

      uint32_t                           delivered;              // Total metrics delivered.

      flat_set<transaction_id_type>      transactions;           // Delivered metric transactions ids

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
   };


   //=============================//
   // === Transfer Operations === //
   //=============================//


   /**
    * Transfers an asset from one account to another.
    */
   struct transfer_operation : public base_operation
   {
      account_name_type           signatory;

      account_name_type           from;       // Sending account to transfer asset from.
      
      account_name_type           to;         // Recieving account to transfer asset to.
      
      asset                       amount;     // The amount of asset to transfer.

      string                      memo;       // The memo is plain-text, encryption on the memo is advised. 

      void              validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return from; }
   };


   /**
    * Requests a Transfer from an account to another account.
    */
   struct transfer_request_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            to;                 // Account requesting the transfer.
      
      account_name_type            from;               // Account that is being requested to accept the transfer.
      
      asset                        amount;             // The amount of asset to transfer.

      string                       memo;               // The memo is plain-text, encryption on the memo is advised. 

      string                       request_id;         // uuidv4 of the request transaction.

      time_point                   expiration;         // time that the request expires. 

      bool                         requested = true;   // True to send the request, false to cancel an existing request. 

      void         validate()const;
      void         get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return to; }
   };


   /**
    * Accepts a transfer request from an account to another account.
    */
   struct transfer_accept_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            from;               // Account that is accepting the transfer.

      account_name_type            to;                 // Account requesting the transfer.
      
      string                       request_id;         // uuidv4 of the request transaction.

      bool                         accepted = true;    // True to accept the request, false to reject. 

      void              validate()const;
      void              get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return from; }
   };


   /**
    * Transfers an asset periodically from one account to another.
    */
   struct transfer_recurring_operation : public base_operation
   {
      account_name_type           signatory;

      account_name_type           from;           // Sending account to transfer asset from.
      
      account_name_type           to;             // Recieving account to transfer asset to.
      
      asset                       amount;         // The amount of asset to transfer for each payment interval.

      string                      transfer_id;    // uuidv4 of the transfer for reference. 

      time_point                  begin;          // Starting time of the first payment.

      time_point                  end;            // Ending time of the recurring payment. 

      fc::microseconds            interval;       // Microseconds between each transfer event.

      string                      memo;           // The memo is plain-text, encryption on the memo is advised. 

      bool                        active = true;  // true if recurring payment is active, false to cancel.

      void              validate()const;
      void              get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return from; }
   };


   /**
    * Requests a periodic transfer from an account to another account.
    */
   struct transfer_recurring_request_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            to;                 // Account requesting the transfer.
      
      account_name_type            from;               // Account that is being requested to accept the transfer.
      
      asset                        amount;             // The amount of asset to transfer.

      string                       request_id;         // uuidv4 of the request transaction, becomes transfer id when accepted.

      time_point                   begin;              // Starting time of the first payment.

      time_point                   end;                // Ending time of the recurring payment. 

      fc::microseconds             interval;           // Microseconds between each transfer event.

      string                       memo;               // The memo is plain-text, encryption on the memo is advised. 

      time_point                   expiration;         // time that the request expires.

      bool                         requested = true;   // True to send the request, false to cancel an existing request. 

      void              validate()const;
      void              get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return to; }
   };


   /**
    * Accepts a periodic transfer request from an account to another account.
    */
   struct transfer_recurring_accept_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            from;               // Account that is accepting the recurring transfer.

      account_name_type            to;                 // Account requesting the recurring transfer.
      
      string                       request_id;         // uuidv4 of the request transaction.

      bool                         accepted = true;    // True to accept the request, false to reject. 

      void              validate()const;
      void              get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return from; }
   };


   //============================//
   // === Balance Operations === //
   //============================//

   /**
    * Claims an account's reward balance into it's liquid balance from newly issued assets.
    */
   struct claim_reward_balance_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      account;

      asset                  reward;

      void get_required_posting_authorities( flat_set< account_name_type >& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return account; }
      void validate() const;
   };


   /**
    * Stakes a liquid balance of an account into it's staked balance.
    */
   struct stake_asset_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      from;         // Account staking the asset

      account_name_type      to;           // if null, then same as from

      asset                  amount;       // Asset amount and symbol

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return from; }
   };


   /**
    * Divests an amount of the staked balance of an account to it's liquid balance.
    */
   struct unstake_asset_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      from;         // Account unstaking the asset

      account_name_type      to;           // if null, then same as from

      asset                  amount;       // Asset amount and symbol

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return from; }
   };


   /**
    * Allows an account to setup an unstake asset withdraw but with the additional
    * request for the funds to be transferred directly to another account's
    * balance rather than the withdrawing account. In addition, those funds
    * can be immediately staked again in the new account's balance
    */
   struct unstake_asset_route_operation : public base_operation
   {
      account_name_type       signatory;

      account_name_type       from_account;

      account_name_type       to_account;

      uint16_t                percent = 0;

      bool                    auto_stake = false;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return from_account; }
   };


   struct transfer_to_savings_operation : public base_operation 
   {
      account_name_type         signatory;

      account_name_type         from;

      account_name_type         to;

      asset                     amount;

      string                    memo;

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return from; }
      void validate() const;
   };


   struct transfer_from_savings_operation : public base_operation 
   {
      account_name_type      signatory;

      account_name_type      from;

      uint32_t               request_id = 0;

      account_name_type      to;

      asset                  amount;

      string                 memo;

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return from; }
      void validate() const;
   };


   struct cancel_transfer_from_savings_operation : public base_operation 
   {
      account_name_type                  signatory;

      account_name_type     from;

      uint32_t              request_id = 0;

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return from; }
      void validate() const;
   };


   /**
    * Delegate a staked asset balance from one account to the other. The staked amount is still owned
    * by the original account, but content voting rights and bandwidth allocation are transferred
    * to the receiving account.
    */
   struct delegate_asset_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      delegator;        // The account delegating the asset

      account_name_type      delegatee;        // The account receiving the asset

      asset                  amount;            //  The amount of the asset delegated         

      void get_required_active_authorities( flat_set< account_name_type >& a ) const { a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return delegator; }
      void validate() const;
   };


   //===========================//
   // === Escrow Operations === //
   //===========================//


   /**
    *  The purpose of this operation is to enable someone to send money contingently to
    *  another individual. The funds leave the *from* account and go into a temporary balance
    *  where they are held until *from* releases it to *to* or *to* refunds it to *from*.
    *  In the event of a dispute the *agent* can divide the funds between the to/from account.
    *  Disputes can be raised any time before or on the dispute deadline time, after the escrow
    *  has been approved by all parties. 
    *  This operation only creates a proposed escrow transfer. Both the *agent* and *to* must
    *  agree to the terms of the arrangement by approving the escrow.
    *  The escrow agent is paid the fee on approval of all parties. It is up to the escrow agent
    *  to determine the fee. 
    *  Escrow transactions are uniquely identified by 'from' and 'escrow_id', the 'escrow_id' 
    *  is defined by the sender.  
    */
   struct escrow_transfer_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      from;

      account_name_type      to;

      account_name_type      agent;

      uint32_t               escrow_id = 30;

      asset                  amount = asset( 0, SYMBOL_COIN );

      asset                  fee;

      time_point             ratification_deadline;

      time_point             escrow_expiration;

      string                 json;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return from; }
   };


   /**
    *  The agent and to accounts must approve an escrow transaction for it to be valid on
    *  the blockchain. Once a party approves the escrow, the cannot revoke their approval.
    *  Subsequent escrow approve operations, regardless of the approval, will be rejected.
    */
   struct escrow_approve_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      who; // Either to or agent can approve the escrow

      account_name_type      from;

      account_name_type      to;

      account_name_type      agent;

      uint32_t               escrow_id = 30;

      bool                   approve = true;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return who; }
   };


   /**
    *  If either the sender or receiver of an escrow payment has an issue, they can
    *  raise it for dispute. Once a payment is in dispute, the agent has authority over
    *  who gets what.
    */
   struct escrow_dispute_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      who;

      account_name_type      from;

      account_name_type      to;

      account_name_type      agent;

      uint32_t               escrow_id = 30;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return who; }
   };


   /**
    *  This operation can be used by anyone associated with the escrow transfer to
    *  release funds if they have permission.
    *
    *  The permission scheme is as follows:
    *  If there is no dispute and escrow has not expired, either party can release funds to the other.
    *  If escrow expires and there is no dispute, either party can release funds to either party.
    *  If there is a dispute regardless of expiration, the agent can release funds to either party, according to prior agreements.
    */
   struct escrow_release_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        from;       // The escrow "from" account.

      account_name_type        to;         // The escrow "to" account.

      account_name_type        agent;      // The escrow "agent" account.

      account_name_type        who;        // The account that is attempting to release the funds, determines the valid "receiver" account options.

      account_name_type        receiver;   // The account that should receive funds [ From | To ].

      uint32_t                 escrow_id = 30;

      asset                    amount = asset( 0, SYMBOL_COIN );       // the amount of the balance asset to release to "receiver".

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      const account_name_type& get_creator_name() const { return who; }
   };


   //============================//
   // === Trading Operations === //
   //============================//


   /**
    * Creates a new limit order on the network that seeks to exchange an asset for 
    * another asset, at a given exchange rate. Fills other orders when the price is better than
    * the current best opposing order, or enters the book as a new order if the price
    * is less than required to fill an existing order.
    */
   struct limit_order_create_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      owner;

      string                 order_id;                    // UUIDv4 of the order for reference.

      asset                  amount_to_sell;

      bool                   fill_or_kill = false;

      price                  exchange_rate;

      account_name_type      interface;

      time_point             expiration = time_point::maximum();

      void  validate()const;
      const account_name_type& get_creator_name() const { return owner; }
      void  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }

      price                  get_price()const { return exchange_rate; }

      pair< asset_symbol_type, asset_symbol_type > get_market()const
      {
         return exchange_rate.base.symbol < exchange_rate.quote.symbol ?
                std::make_pair(exchange_rate.base.symbol, exchange_rate.quote.symbol) :
                std::make_pair(exchange_rate.quote.symbol, exchange_rate.base.symbol);
      }
   };


   struct limit_order_cancel_operation : public base_operation
   {
      account_name_type       signatory;

      account_name_type       owner;

      string                  order_id;

      void  validate()const;
      const account_name_type& get_creator_name() const { return owner; }
      void  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Creates a new margin order that first borrows a specified asset,
    * using a given collateral asset and sells it for a position asset amount. 
    * The margin order pays interest on the 
    * loan until it is closed, at which time the position is liquidated
    * and the loan is repaid with interest.
    * 
    * TODO: Margin credit liquidity check before opening
    */
   struct margin_order_create_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      owner;

      string                 order_id;                // UUIDv4 of the order for reference.

      price                  exchange_rate;           // The asset pair price to sell the borrowed amount at on the exchange

      asset                  collateral;              // Collateral asset used to back the loan value. Returned to credit collateral object when position is closed. 

      asset                  amount_to_borrow;        // Amount of asset borrowed to purchase the position asset. Repaid when the margin order is closed.                

      optional<price>        stop_price;              // User determined price at which the position will be closed if it falls into a net loss.

      optional<price>        target_price;            // User determined price at which the order will be closed if it rises into a net profit.

      time_point             created; 

      account_name_type      interface;      

      bool                   fill_or_kill = false;

      time_point             expiration = time_point::maximum();

      void  validate()const;
      const account_name_type& get_creator_name() const { return owner; }
      void  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }

      price             get_price()const { return exchange_rate; }

      pair< asset_symbol_type, asset_symbol_type > get_market()const
      {
         return exchange_rate.base.symbol < exchange_rate.quote.symbol ?
                std::make_pair(exchange_rate.base.symbol, exchange_rate.quote.symbol) :
                std::make_pair(exchange_rate.quote.symbol, exchange_rate.base.symbol);
      }
   };


   struct margin_order_close_operation : public base_operation
   {
      account_name_type       signatory;

      account_name_type       owner;

      string                  order_id;

      optional< price >       exchange_rate;

      bool                    force_close = false;

      bool                    fill_or_kill = false;

      void  validate()const;
      const account_name_type& get_creator_name() const { return owner; }
      void  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   struct call_order_update_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        funding_account;            // pays fee, collateral, and cover

      asset                    delta_collateral;           // the amount of collateral to add to the margin position

      asset                    delta_debt;                 // the amount of the debt to be paid off, may be negative to issue new debt

      optional< uint16_t >     target_collateral_ratio;    // maximum CR to maintain when selling collateral on margin call

      account_name_type        interface;                  // Name of the interface that created the transaction.

      extensions_type          extensions;   

      void                validate()const;
      const account_name_type& get_creator_name() const { return funding_account; }
      void                get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   struct bid_collateral_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     bidder;                   // pays fee and additional collateral

      asset                 additional_collateral;    // the amount of collateral to bid for the debt

      asset                 debt_covered;             // the amount of debt to take over

      asset                 fee;

      extensions_type       extensions;

      void                  validate()const;
      const account_name_type& get_creator_name() const { return bidder; }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   //=========================//
   // === Pool Operations === //
   //=========================//


   struct liquidity_pool_create_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;            // Creator of the new liquidity pool.

      asset                 first_amount;       // Initial balance of one asset.

      asset                 second_amount;      // Initial balance of second asset, initial price is the ratio of these two amounts.

      void                  validate()const;
      const account_name_type& get_creator_name() const { return account; }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   struct liquidity_pool_exchange_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;            // Account executing the exchange with the pool.

      asset                 amount;             // Amount of asset to be exchanged.

      asset_symbol_type     receive_asset;      // The asset to recieve from the liquidity pool.

      account_name_type     interface;          // Name of the interface account broadcasting the transaction.

      optional<price>       limit_price;        // The price of acquistion at which to cap the exchange to.

      bool                  acquire = false;    // Set true to acquire the specified amount, false to exchange in.

      void                  validate()const;
      const account_name_type& get_creator_name() const { return account; }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   struct liquidity_pool_fund_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;            // Account funding the liquidity pool to recieve the liquidity pool asset.

      asset                 amount;             // Amount of an asset to contribute to the liquidity pool.

      asset_symbol_type     pair_asset;         // Pair asset to the liquidity pool to recieve liquidity pool assets of. 

      void                  validate()const;
      const account_name_type& get_creator_name() const { return account; }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   struct liquidity_pool_withdraw_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;           // Account withdrawing liquidity pool assets from the pool.

      asset                 amount;            // Amount of the liquidity pool asset to redeem for underlying deposited assets. 

      asset_symbol_type     receive_asset;     // The asset to recieve from the liquidity pool.

      void                  validate()const;
      const account_name_type& get_creator_name() const { return account; }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };

   struct credit_pool_collateral_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;         // Account locking an asset as collateral. 

      asset                 amount;          // Amount of collateral balance to lock, 0 to unlock existing collateral. 

      void                  validate()const;
      const account_name_type& get_creator_name() const { return account; }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   struct credit_pool_borrow_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;         // Account borrowing funds from the pool, must have sufficient collateral.

      asset                 amount;          // Amount of an asset to borrow. Limit of 75% of collateral value. Set to 0 to repay loan.

      asset                 collateral;      // Amount of an asset to use as collateral for the loan. Set to 0 to reclaim collateral to collateral balance. 

      string                loan_id;         // UUDIv4 unique identifier for the loan

      void                  validate()const;
      const account_name_type& get_creator_name() const { return account; }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   struct credit_pool_lend_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;         // Account lending an asset to the credit pool.

      asset                 amount;          // Amount of asset being lent.

      void                  validate()const;
      const account_name_type& get_creator_name() const { return account; }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   struct credit_pool_withdraw_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;       // Account withdrawing its lent asset from the credit pool by redeeming credit-assets. 

      asset                 amount;        // Amount of interest bearing credit assets being redeemed for thier underlying assts. 

      void                  validate()const;
      const account_name_type& get_creator_name() const { return account; }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   //==========================//
   // === Asset Operations === //
   //==========================//


   bool is_valid_symbol( const string& symbol );

   /**
    * Options available to all assets.
    */
   struct asset_options 
   {
      string                          description;                           // Data that describes the purpose of this asset.

      string                          json;                                  // JSON metadata of this asset.
      
      share_type                      max_supply = MAX_ASSET_SUPPLY;         // The maximum supply of this asset which may exist at any given time. 

      uint8_t                         stake_intervals = 0;                   // Weeks required to stake the asset.

      uint8_t                         unstake_intervals = 4;                 // Weeks require to unstake the asset.

      uint16_t                        market_fee_percent = 0;                // Percentage of the total traded will be paid to the issuer of the asset.

      uint16_t                        market_fee_share_percent = 0;          // Percentage of the market fee that will be shared with the account's referrers.

      share_type                      max_market_fee = MAX_ASSET_SUPPLY;     // Market fee charged on a trade is capped to this value.
      
      uint32_t                        issuer_permissions = UIA_ASSET_ISSUER_PERMISSION_MASK; // The flags which the issuer has permission to update.

      uint32_t                        flags = 0;                             // The currently active flags on this permission.

      price                           core_exchange_rate = price(asset(), asset()); // Exchange rate between asset and core for fee pool exchange

      flat_set<account_name_type>     whitelist_authorities;                 // Accounts able to transfer this asset if the flag is set and whitelist is non-empty.

      flat_set<account_name_type>     blacklist_authorities;                 // Accounts which cannot transfer or recieve this asset.

      flat_set<asset_symbol_type>     whitelist_markets;                     // The assets that this asset may be traded against in the market

      flat_set<asset_symbol_type>     blacklist_markets;                     // The assets that this asset may not be traded against in the market, must not overlap whitelist

      extensions_type                 extensions;

      void validate()const;
   };


   /**
    * Options available to currency assets.
    */
   struct currency_options 
   {
      asset_symbol_type   buyback_asset = SYMBOL_USD;

      share_type          annual_issuance = CURRENCY_ISSUANCE_RATE;

      uint16_t            block_producer_percent = CURRENCY_PRODUCER_PERCENT;

      extensions_type     extensions;

      void validate()const;
   };


   /**
    * Options available to BitAssets.
    */
   struct bitasset_options 
   {
      fc::microseconds    feed_lifetime = PRICE_FEED_LIFETIME;                            // Time before a price feed expires

      uint8_t             minimum_feeds = 1;                                              // Minimum number of unexpired feeds required to extract a median feed from

      fc::microseconds    force_settlement_delay = FORCE_SETTLEMENT_DELAY;                // This is the delay between the time a long requests settlement and the chain evaluates the settlement

      uint16_t            force_settlement_offset_percent = FORCE_SETTLEMENT_OFFSET;      // The percentage to adjust the feed price in the short's favor in the event of a forced settlement

      uint16_t            maximum_force_settlement_volume = FORCE_SETTLEMENT_MAX_VOLUME;  // the percentage of current supply which may be force settled within each maintenance interval.

      asset_symbol_type   short_backing_asset = SYMBOL_COIN;                              // The symbol of the asset that the bitasset is collateralized by.

      extensions_type     extensions;

      void validate()const;
   };

   /**
    * Options available to equity assets.
    */
   struct equity_options 
   {
      asset_symbol_type   dividend_asset = SYMBOL_USD;

      uint16_t            dividend_share_percent = DIVIDEND_SHARE_PERCENT;            // Percentage of incoming assets added to the dividends pool

      uint16_t            liquid_dividend_percent = LIQUID_DIVIDEND_PERCENT;          // percentage of equity dividends distributed to liquid balances

      uint16_t            staked_dividend_percent = STAKED_DIVIDEND_PERCENT;          // percentage of equity dividends distributed to staked balances

      uint16_t            savings_dividend_percent = SAVINGS_DIVIDEND_PERCENT;        // percentage of equity dividends distributed to savings balances

      uint16_t            liquid_voting_rights = PERCENT_100;                         // Amount of votes per asset conveyed to liquid holders of the asset

      uint16_t            staked_voting_rights = PERCENT_100;                         // Amount of votes per asset conveyed to staked holders of the asset

      uint16_t            savings_voting_rights = PERCENT_100;                        // Amount of votes per asset conveyed to savings holders of the asset

      fc::microseconds    min_active_time = EQUITY_ACTIVITY_TIME;

      uint16_t            min_witnesses = EQUITY_MIN_WITNESSES;

      uint16_t            boost_balance = EQUITY_BOOST_BALANCE;

      uint16_t            boost_activity = EQUITY_BOOST_ACTIVITY;

      uint16_t            boost_witnesses = EQUITY_BOOST_WITNESSES;

      uint16_t            boost_top = EQUITY_BOOST_TOP_PERCENT;

      extensions_type     extensions;

      void validate()const;
   };


   /**
    * Options available to credit assets.
    */
   struct credit_options 
   {
      asset_symbol_type   buyback_asset = SYMBOL_USD;

      uint32_t            buyback_share_percent = BUYBACK_SHARE_PERCENT;                     // Percentage of incoming assets added to the buyback pool

      uint32_t            liquid_fixed_interest_rate = LIQUID_FIXED_INTEREST_RATE;           // Fixed component of Interest rate of the asset for liquid balances.

      uint32_t            liquid_variable_interest_rate = LIQUID_VARIABLE_INTEREST_RATE;     // Variable component of Interest rate of the asset for liquid balances.

      uint32_t            staked_fixed_interest_rate = STAKED_FIXED_INTEREST_RATE;           // Fixed component of Interest rate of the asset for staked balances.

      uint32_t            staked_variable_interest_rate = STAKED_VARIABLE_INTEREST_RATE;     // Variable component of Interest rate of the asset for staked balances.

      uint32_t            savings_fixed_interest_rate = SAVINGS_FIXED_INTEREST_RATE;         // Fixed component of Interest rate of the asset for savings balances.

      uint32_t            savings_variable_interest_rate = SAVINGS_VARIABLE_INTEREST_RATE;   // Variable component of Interest rate of the asset for savings balances.

      uint32_t            var_interest_range = VAR_INTEREST_RANGE;                           // The percentage range from the buyback price over which to apply the variable interest rate.

      extensions_type     extensions;

      void validate()const;
   };


   /**
    * Creates a new asset object of the asset type provided.
    * Assets can be transferred between accounts to represent ownership of anything of value
    * All assets can be staked and saved, delegated and recieved. 
    */
   struct asset_create_operation : public base_operation
   {
      account_name_type               signatory;

      account_name_type               issuer;                       // Name of the issuing account, can create units and administrate the asset
      
      asset_symbol_type               symbol;                       // The ticker symbol of this asset.

      asset_types                     asset_type;                   // The type of the asset.

      asset                           coin_liquidity;               // Amount of COIN asset to inject into the Coin liquidity pool.  

      asset                           usd_liquidity;                // Amount of USD asset to inject into the USD liquidity pool.

      asset                           credit_liquidity;             // Amount of the new asset to inject into the credit pool.                     

      asset_options                   common_options;

      optional<currency_opts>         currency_opts;                // Options available for currency assets

      optional<bitasset_options>      bitasset_opts;                // Options available for BitAssets.

      optional<equity_options>        equity_opts;                  // Options available for equity assets

      optional<credit_options>        credit_opts;                  // Options available for credit assets

      optional<gateway_opts>          gateway_opts;                 // Options avalable for gateway assets

      extensions_type                 extensions;

      void                            validate()const;
      const account_name_type&        get_creator_name() const { return issuer; }
      void                            get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * @brief Update options common to all assets
    * @ingroup operations
    *
    * There are a number of options which all assets in the network use. These options are enumerated in the @ref
    * asset_options struct. This operation is used to update these options for an existing asset.
    *
    * @note This operation cannot be used to update BitAsset-specific options. For these options, use @ref
    * asset_update_bitasset_operation instead.
    *
    * @pre @ref issuer SHALL be an existing account and MUST match asset_object::issuer on @ref asset_to_update
    * @pre @ref fee SHALL be nonnegative, and @ref issuer MUST have a sufficient balance to pay it
    * @pre @ref new_options SHALL be internally consistent, as verified by @ref validate()
    * @post @ref asset_to_update will have options matching those of new_options
    */
   struct asset_update_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             issuer;

      asset_symbol_type             asset_to_update;

      optional<account_name_type>   new_issuer; // If the asset is to be given a new issuer, specify his ID here.

      asset_options                 new_options;

      extensions_type               extensions;

      void                          validate()const;
      const account_name_type& get_creator_name() const { return issuer; }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * @ingroup operations
    */
   struct asset_issue_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      issuer;               // The issuer of the asset

      asset                  asset_to_issue;       // amount of asset being issued to the account

      account_name_type      issue_to_account;     // Account receiving the newly issued asset.

      string                 memo;                 // user provided data encrypted to the memo key of the "to" account 

      extensions_type        extensions;

      void                   validate()const;
      const account_name_type& get_creator_name() const { return issuer; }
      void                   get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * @brief used to take an asset out of circulation, returning to the issuer
    * @ingroup operations
    *
    * @note You cannot use this operation on market-issued assets.
    */
   struct asset_reserve_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     payer;

      asset                 amount_to_reserve;

      extensions_type       extensions;

      void            validate()const;
      const account_name_type& get_creator_name() const { return payer; }
      void                   get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * @brief used to transfer accumulated fees back to the issuer's balance.
    */
   struct asset_claim_fees_operation : public base_operation
   {
      account_name_type    signatory;

      account_name_type    issuer;

      asset                amount_to_claim;          // asset claimed must be issued by the issuer

      extensions_type      extensions;

      void            validate()const;
      const account_name_type& get_creator_name() const { return issuer; }
      void                   get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * @brief Transfers Core asset from the fee pool of a specified asset back to the issuer's balance

    * @param fee Payment for the operation execution
    * @param issuer Account which will be used for transfering Core asset
    * @param symbol Symbol of the asset whose fee pool is going to be drained
    * @param amount_to_claim Amount of core asset to claim from the fee pool
    * @param extensions Field for future expansion

    * @pre @ref fee must be paid in the asset other than the one whose pool is being drained
    * @pre @ref amount_to_claim should be specified in the core asset
    * @pre @ref amount_to_claim should be nonnegative
    */
   struct asset_claim_pool_operation : public base_operation
   {
      account_name_type      signatory;
      
      account_name_type      issuer;

      asset_symbol_type      symbol;             // Asset symbol

      asset                  amount_to_claim;    // core asset

      extensions_type        extensions;

      void                  validate()const;
      const account_name_type& get_creator_name() const { return issuer; }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   struct asset_fund_fee_pool_operation : public base_operation
   {
      account_name_type       signatory;
      
      account_name_type       from_account;

      asset_symbol_type       symbol;

      asset                   pool_amount; // Must be core asset

      extensions_type         extensions;

      void                    validate()const;
      const account_name_type& get_creator_name() const { return from_account; }
      void                    get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Updates the issuer account of an asset, transferring effective ownership of it to another account,
    * and enabling the new account to issue additional supply, and change the asset's properties. 
    */
   struct asset_update_issuer_operation : public base_operation
   {
      account_name_type      signatory;
      
      account_name_type      issuer;

      asset_symbol_type      asset_to_update;

      account_name_type      new_issuer;

      extensions_type        extensions;

      void                   validate()const;
      const account_name_type& get_creator_name() const { return issuer; }
      void get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Update options specific to BitAssets
    * BitAssets have some options which are not relevant to other asset types. This operation is used to update those
    * options an an existing BitAsset.
    *
    * Issuer MUST be an existing account and must match asset_object::issuer on asset_to_update.
    * Asset_to_update must be a BitAsset, i.e. is_market_issued() returns true.
    * Fee must be nonnegative, and issuer must have a sufficient balance to pay it.
    * new_options must be internally consistent, as verified by validate().
    * asset_to_update will have BitAsset-specific options matching those of new_options.
    */
   struct asset_update_bitasset_operation : public base_operation
   {
      account_name_type       signatory;
      
      account_name_type       issuer;

      asset_symbol_type       asset_to_update;

      bitasset_options        new_options;

      extensions_type         extensions;

      void                   validate()const;
      const account_name_type& get_creator_name() const { return issuer; }
      void                   get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * @brief Update the set of feed-producing accounts for a BitAsset
    * @ingroup operations
    *
    * BitAssets have price feeds selected by taking the median values of recommendations from a set of feed producers.
    * This operation is used to specify which accounts may produce feeds for a given BitAsset.
    *
    * @pre @ref issuer MUST be an existing account, and MUST match asset_object::issuer on @ref asset_to_update
    * @pre @ref issuer MUST NOT be the committee account
    * @pre @ref asset_to_update MUST be a BitAsset, i.e. @ref asset_object::is_market_issued() returns true
    * @pre @ref fee MUST be nonnegative, and @ref issuer MUST have a sufficient balance to pay it
    * @pre Cardinality of @ref new_feed_producers MUST NOT exceed @ref chain_parameters::maximum_asset_feed_publishers
    * @post @ref asset_to_update will have a set of feed producers matching @ref new_feed_producers
    * @post All valid feeds supplied by feed producers in @ref new_feed_producers, which were already feed producers
    * prior to execution of this operation, will be preserved
    */
   struct asset_update_feed_producers_operation : public base_operation
   {
      account_name_type               signatory;
      
      account_name_type               issuer;

      asset_symbol_type               asset_to_update;

      flat_set<account_name_type>     new_feed_producers;

      extensions_type                 extensions;

      void                            validate()const;
      const account_name_type& get_creator_name() const { return issuer; }
      void                            get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * @brief Publish price feeds for market-issued assets
    * @ingroup operations
    *
    * Price feed providers use this operation to publish their price feeds for market-issued assets. A price feed is
    * used to tune the market for a particular market-issued asset. For each value in the feed, the median across all
    * committee_member feeds for that asset is calculated and the market for the asset is configured with the median of that
    * value.
    *
    * The feed in the operation contains three prices: a call price limit, a short price limit, and a settlement price.
    * The call limit price is structured as (collateral asset) / (debt asset) and the short limit price is structured
    * as (asset for sale) / (collateral asset). Note that the asset IDs are opposite to eachother, so if we're
    * publishing a feed for USD, the call limit price will be CORE/USD and the short limit price will be USD/CORE. The
    * settlement price may be flipped either direction, as long as it is a ratio between the market-issued asset and
    * its collateral.
    */
   struct asset_publish_feed_operation : public base_operation
   {
      account_name_type          signatory;
      
      account_name_type          publisher;

      asset_symbol_type          symbol; // asset for which the feed is published

      price_feed                 feed;

      extensions_type            extensions;

      void            validate()const;
      const account_name_type& get_creator_name() const { return publisher; }
      void                   get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Schedules a market-issued asset for automatic settlement
    *
    * Holders of market-issued assests may request a forced settlement for some amount of their asset. This means that
    * the specified sum will be locked by the chain and held for the settlement period, after which time the chain will
    * choose a margin position holder and buy the settled asset using the margin's collateral. The price of this sale
    * will be based on the feed price for the market-issued asset being settled. The exact settlement price will be the
    * feed price at the time of settlement with an offset in favor of the margin position, where the offset is a
    * blockchain parameter set in the global_property_object.
    *
    * The fee is paid by @ref account, and @ref account must authorize this operation
    */
   struct asset_settle_operation : public base_operation
   {
      account_name_type       signatory;

      account_name_type       account;      // Account requesting the force settlement. This account pays the fee.
      
      asset                   amount;       // Amount of asset to force settle. This must be a market-issued asset.

      extensions_type         extensions;

      void                    validate()const;
      const account_name_type& get_creator_name() const { return account; }
      void                    get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    *  @brief allows global settling of bitassets
    *
    *  In order to use this operation, @ref asset_to_settle must have the global_settle flag set
    *  When this operation is executed all balances are converted into the backing asset at the
    *  settle_price and all open margin positions are called at the settle price. If this asset is
    *  used as backing for other bitassets, those bitassets will be force settled at their current
    *  feed price.
    */
   struct asset_global_settle_operation : public base_operation
   {
      account_name_type          signatory;
      
      account_name_type          issuer;              // Issuer of the asset being settled. 

      asset_symbol_type          asset_to_settle;     // Symbol of the asset being settled. 

      price                      settle_price;        // Global settlement price, must be in asset / backing asset. 

      extensions_type            extensions;

      void                       validate()const;
      const account_name_type& get_creator_name() const { return issuer; }
      void                       get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };
   

   //=====================================//
   // === Block Production Operations === //
   //=====================================//


   /**
    * Witnesses must vote on how to set certain chain properties to ensure a smooth
    * and well functioning network. Any time @owner is in the active set of witnesses these
    * properties will be used to control the blockchain configuration.
    */
   struct chain_properties
   {
      asset             account_creation_fee = asset( MIN_ACCOUNT_CREATION_FEE, SYMBOL_COIN );  // Minimum fee required to create a new account by staking.

      uint32_t          maximum_block_size = MAX_BLOCK_SIZE;  // The maximum block size of the network in bytes.

      uint16_t          credit_interest_rate  = CREDIT_INTEREST_RATE;  // The credit interest rate paid to holders of network credit assets.

      fc::microseconds  interest_compound_interval = INTEREST_COMPOUND_INTERVAL;  // The frequency of interest compounding payments. 

      uint16_t          maximum_asset_feed_publishers = MAX_ASSET_FEED_PUBLISHERS;  // The maximum number of accounts that can publish price feeds for a bitasset.

      void validate()const
      {
         FC_ASSERT( account_creation_fee.amount >= MIN_ACCOUNT_CREATION_FEE);
         FC_ASSERT( maximum_block_size >= MIN_BLOCK_SIZE_LIMIT);
         FC_ASSERT( credit_interest_rate >= 0 );
         FC_ASSERT( credit_interest_rate <= PERCENT_100 );
      };
   };


   /**
    *  Users who wish to become a witness must pay a fee acceptable to
    *  the current witnesses to apply for the position and allow voting
    *  to begin.
    *
    *  If the owner isn't a witness they will become a witness.  Witnesses
    *  are charged a fee equal to 1 weeks worth of witness pay which in
    *  turn is derived from the current share supply. The fee is
    *  only applied if the owner is not already a witness.
    *
    *  If the block_signing_key is null then the witness is removed from
    *  contention.  The network will pick the top voted witnesses for
    *  producing blocks.
    */
   struct witness_update_operation : public base_operation
   {
      account_name_type         signatory;
      
      account_name_type         owner;                  // The account that owns the witness.

      string                    details;                // The witnesses details.

      string                    url;                    // External reference URL.

      string                    json;                   // The Witnesses json metadata.

      fc::array<uint16_t, 2>    location;               // Appriximate Longitude / Latitude co-ordinates of the witness.

      public_key_type           block_signing_key;      // The public key used to sign blocks.

      chain_properties          props;                  // The declared chain properties for the network, used to adjust variables.

      bool                      active = true;          // Set active to true to activate, false to deactivate; 

      asset                     fee;                    // The fee paid to register a new witness.

      void validate()const;
      const account_name_type& get_creator_name() const { return owner; }
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   struct proof_of_work_input
   {
      account_name_type       miner_account;

      block_id_type           prev_block;

      uint64_t                nonce = 0;
   };


   struct proof_of_work
   {
      proof_of_work_input     input;

      uint32_t                pow_summary = 0;

      void create( const block_id_type& prev_block, const account_name_type& account_name, uint64_t nonce );
      void validate()const;
   };


   struct equihash_proof_of_work
   {
      proof_of_work_input           input;

      fc::equihash::proof           proof;

      block_id_type                 prev_block;

      uint32_t                      pow_summary = 0;

      void create( const block_id_type& recent_block, const account_name_type& account_name, uint32_t nonce );
      void validate() const;
   };


   typedef fc::static_variant< proof_of_work, equihash_proof_of_work > proof_of_work_type;

   /**
    * Proof of work operation enables miners to publish cryptographic proofs
    * of mining security, which make the chain thermodynamically secure
    * due to the energy expenditure required to redo the work.
    * Miners are added to the mining queue, according to the difficulty of
    * thier published work in the prior 7 days, and the highest producing miners
    * are able to produce blocks in each round, and claim the proof of work reward.
    */
   struct proof_of_work_operation : public base_operation
   {
      proof_of_work_type            work;

      optional< public_key_type >   new_owner_key;

      chain_properties              props;

      void validate()const;

      const account_name_type& get_creator_name() const { return work.get< equihash_proof_of_work >().input.miner_account; }

      void get_required_active_authorities( flat_set<account_name_type>& a )const;

      void get_required_authorities( vector< authority >& a )const
      {
         if( new_owner_key )
         {
            a.push_back( authority( 1, *new_owner_key, 1 ) );
         }
      };
   };

   /**
    * The verify block operation enables the top witnesses and miners to verify that a
    * given block exists at a given height, and that it is valid.
    * Block producers must not publish verification transactions of two different
    * blocks at the same height.
    */
   struct verify_block_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             producer;      // The name of the block producing account

      block_id_type                 block_id;      // The block id of the block being verifed as valid and received. 

      uint32_t                      block_height;  // The height of the block being verified.

      void validate()const;
      const account_name_type& get_creator_name() const { return producer; }
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };

   /**
    * The commit block operation enables producers to assert that:
    * 1 - A given block at a given height is valid.
    * 2 - A supermajority of least Two Thirds Plus One (67) block producers have verified the block.
    * 3 - They will not produce future blocks that do not contain that block as an ancestor.
    * 4 - They stake a given value of core asset on their commitment.
    * If the producer signs duplicate commitments at the same height, or produces blocks that deviate from this
    * blocks history, the staked value is forfeited to any account that publishes a violation proof transaction.
    * 
    * Producers cannot sign commitments for blocks that are already irreversible by production history depth. 
    * After a block becomes irreversible, the fastest Two Thirds Plus One (67) block producers that have committed
    * to the block are rewarded according to their staked amounts from the validation reward pool.
    * If more than 67 producers signed and published commitments transactions, all producers within the last
    * block to include commitment transactions before exceeding 67 are included for the reward distribution. 
    */
   struct commit_block_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             producer;         // The name of the block producing account

      block_id_type                 block_id;         // The block id of the block being committed as irreversible to that producer. 

      uint32_t                      block_height;     // The height of the block being committed to.

      flat_set<transaction_id_type> verifications;    // The set of attesting transaction ids of verification transactions from currently active producers.

      asset                         commitment_stake; // the value of staked balance that the producer stakes on this commitment. 

      void validate()const;
      const account_name_type& get_creator_name() const { return producer; }
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /** 
    * Operation used to declare a violation of the commitment made by a block producer
    * and claim the commitment stake of that producer.
    */
   struct producer_violation_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             reporter;                // The name of the account detecting and reporting the validation violation.
      
      account_name_type             producer;                // The name of the alleged infringing block producer.  

      signed_block_header           first_block;             // The first block signed by the producer.

      signed_block_header           second_block;            // The second block that is in contravention of a commitment transaction. 

      transaction_id_type           commitment_transaction;  // The transaction id of the violated commitment transaction.

      asset                         commitment_stake;        // The staked balance that the producer forfeits from the violated committment. 

      void validate()const;
      const account_name_type& get_creator_name() const { return reporter; }
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   //===========================//
   // === Custom Operations === //
   //===========================//


   /**
    * Provides a generic way to add higher level protocols on top of witness consensus
    * There is no validation for this operation other than that required auths are valid
    */
   struct custom_operation : public base_operation
   {
      flat_set< account_name_type > required_auths;

      uint16_t                      id = 0;

      vector< char >                data;

      void validate()const;
      const account_name_type& get_creator_name() const { return required_auths[0]; }
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_auths ) a.insert(i); }
   };


   /** 
    * Serves the same purpose as custom_operation but also supports required posting authorities.
    * The operation is designed to be more human readable and developer friendly.
    **/
   struct custom_json_operation : public base_operation
   {
      flat_set< account_name_type > required_auths;

      flat_set< account_name_type > required_posting_auths;

      string                        id;      // must be less than 32 characters long, refers to the plugin used to interpret the operation. 

      string                        json;    // must be proper UTF8 / JSON string.

      void validate()const;
      const account_name_type& get_creator_name() const 
      { 
         if( required_auths.begin() != required_auths.end() )
         {
            return *required_auths.begin();
         }
         else if( required_posting_auths.begin() != required_posting_auths.end() )
         {
            return *required_posting_auths.begin();
         }
         else return NULL_ACCOUNT;
      }
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_auths ) a.insert(i); }
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_posting_auths ) a.insert(i); }
   };
   
} } // node::protocol

// TODO: Order and finalize the reflection here


FC_REFLECT( node::protocol::transfer_to_savings_operation, 
         (from)
         (to)
         (amount)
         (memo) 
         );

FC_REFLECT( node::protocol::transfer_from_savings_operation, 
         (from)
         (request_id)
         (to)
         (amount)
         (memo) 
         );

FC_REFLECT( node::protocol::cancel_transfer_from_savings_operation, 
         (from)
         (request_id) 
         );

FC_REFLECT( node::protocol::reset_account_operation, 
         (reset_account)
         (account_to_reset)
         (new_owner_authority) 
         );

FC_REFLECT( node::protocol::set_reset_account_operation, 
         (account)
         (current_reset_account)
         (reset_account) 
         );

FC_REFLECT( node::protocol::proof_of_work, 
         (input)
         (pow_summary) 
         );

FC_REFLECT( node::protocol::proof_of_work_input, 
         (worker_account)
         (prev_block)
         (nonce) 
         );

FC_REFLECT( node::protocol::equihash_proof_of_work, 
         (input)
         (proof)
         (prev_block)
         (pow_summary) 
         );

FC_REFLECT( node::protocol::chain_properties, 
         (account_creation_fee)
         (maximum_block_size)
         (credit_interest_rate) 
         );

FC_REFLECT_TYPENAME( node::protocol::proof_of_work_type )

FC_REFLECT( node::protocol::proof_of_work_operation, 
         (work)
         (new_owner_key)
         (props) 
         );

FC_REFLECT( node::protocol::account_create_operation,
         (fee)
         (creator)
         (new_account_name)
         (owner)
         (active)
         (posting)
         (secure_public_key)
         (json) 
         );

FC_REFLECT( node::protocol::account_update_operation,
         (account)
         (owner)
         (active)
         (posting)
         (secure_public_key)
         (json) 
         );

FC_REFLECT( node::protocol::transfer_operation, 
         (from)
         (to)
         (amount)
         (memo) 
         );

FC_REFLECT( node::protocol::stake_asset_operation, 
         (from)
         (to)
         (amount) 
         );

FC_REFLECT( node::protocol::unstake_asset_operation, 
         (account)
         (amount) 
         );

FC_REFLECT( node::protocol::unstake_asset_route_operation, 
         (from_account)
         (to_account)
         (percent)
         (auto_stake) 
         );

FC_REFLECT( node::protocol::witness_update_operation, 
         (owner)
         (url)
         (block_signing_key)
         (props)
         (fee) 
         );

FC_REFLECT( node::protocol::account_witness_vote_operation, 
         (account)
         (witness)
         (approve) 
         );

FC_REFLECT( node::protocol::account_update_proxy_operation, 
         (account)
         (proxy) 
         );

FC_REFLECT( node::protocol::comment_operation, 
         (parent_author)
         (parent_permlink)
         (author)
         (permlink)
         (title)
         (body)
         (json) 
         );

FC_REFLECT( node::protocol::vote_operation, 
         (voter)
         (author)
         (permlink)
         (weight) 
         );

FC_REFLECT( node::protocol::custom_operation, 
         (required_auths)
         (id)
         (data) 
         );

FC_REFLECT( node::protocol::custom_json_operation, 
         (required_auths)
         (required_posting_auths)
         (id)
         (json) 
         );

FC_REFLECT( node::protocol::limit_order_create_operation, 
         (owner)
         (orderid)
         (amount_to_sell)
         (exchange_rate)
         (fill_or_kill)
         (expiration) 
         );

FC_REFLECT( node::protocol::limit_order_cancel_operation, 
         (owner)
         (orderid) 
         );

FC_REFLECT( node::protocol::beneficiary_route_type, 
         (account)
         (weight) 
         );

FC_REFLECT( node::protocol::comment_payout_beneficiaries, 
         (beneficiaries) 
         );

FC_REFLECT_TYPENAME( node::protocol::comment_options_extension )

FC_REFLECT( node::protocol::comment_options_operation, 
         (author)
         (permlink)
         (max_accepted_payout)
         (percent_liquid)
         (allow_votes)
         (allow_curation_rewards)
         (extensions) 
         );

FC_REFLECT( node::protocol::escrow_transfer_operation, 
         (from)
         (to)
         (amount)
         (escrow_id)
         (agent)
         (fee)
         (json)
         (ratification_deadline)
         (escrow_expiration) 
         );

FC_REFLECT( node::protocol::escrow_approve_operation, 
         (from)
         (to)
         (agent)
         (who)
         (escrow_id)
         (approve) 
         );

FC_REFLECT( node::protocol::escrow_dispute_operation, 
         (from)
         (to)
         (agent)
         (who)
         (escrow_id) 
         );

FC_REFLECT( node::protocol::escrow_release_operation, 
         (from)
         (to)
         (agent)
         (who)
         (receiver)
         (escrow_id)
         (amount) 
         );

FC_REFLECT( node::protocol::challenge_authority_operation, 
         (challenger)
         (challenged)
         (require_owner) 
         );

FC_REFLECT( node::protocol::prove_authority_operation, 
         (challenged)
         (require_owner) 
         );

FC_REFLECT( node::protocol::request_account_recovery_operation, 
         (recovery_account)
         (account_to_recover)
         (new_owner_authority)
         (extensions) 
         );

FC_REFLECT( node::protocol::recover_account_operation, 
         (account_to_recover)
         (new_owner_authority)
         (recent_owner_authority)
         (extensions) 
         );

FC_REFLECT( node::protocol::change_recovery_account_operation, 
         (account_to_recover)
         (new_recovery_account)
         (extensions) 
         );

FC_REFLECT( node::protocol::decline_voting_rights_operation, 
         (account)
         (decline) 
         );

FC_REFLECT( node::protocol::claim_reward_balance_operation, 
         (account)
         (reward) 
         );

FC_REFLECT( node::protocol::delegate_asset_operation, 
         (delegator)
         (delegatee)
         (asset) 
         );

FC_REFLECT( node::protocol::asset_claim_fees_operation, 
         (issuer)
         (amount_to_claim)
         (extensions) 
         );

FC_REFLECT( node::protocol::asset_claim_pool_operation, 
         (issuer)
         (symbol)
         (amount_to_claim)
         (extensions) 
         );

FC_REFLECT( node::protocol::asset_options,
         (max_supply)
         (market_fee_percent)
         (max_market_fee)
         (issuer_permissions)
         (flags)
         (core_exchange_rate)
         (whitelist_authorities)
         (blacklist_authorities)
         (whitelist_markets)
         (blacklist_markets)
         (description)
         (extensions)
         );

FC_REFLECT( node::protocol::bitasset_options,
         (feed_lifetime)
         (minimum_feeds)
         (force_settlement_delay)
         (force_settlement_offset_percent)
         (maximum_force_settlement_volume)
         (short_backing_asset)
         (extensions)
         );

FC_REFLECT( node::protocol::asset_create_operation,
         (issuer)
         (symbol)
         (precision)
         (common_options)
         (bitasset_opts)
         (extensions)
         );

FC_REFLECT( node::protocol::asset_update_operation,
         (issuer)
         (asset_to_update)
         (new_issuer)
         (new_options)
         (extensions)
         );

FC_REFLECT( node::protocol::asset_update_issuer_operation,
         (issuer)
         (asset_to_update)
         (new_issuer)
         (extensions)
         );

FC_REFLECT( node::protocol::asset_update_bitasset_operation,
         (issuer)
         (asset_to_update)
         (new_options)
         (extensions)
         );

FC_REFLECT( node::protocol::asset_update_feed_producers_operation,
         (issuer)
         (asset_to_update)
         (new_feed_producers)
         (extensions)
         );

FC_REFLECT( node::protocol::asset_publish_feed_operation, 
         (publisher)
         (symbol)
         (feed)
         (extensions) 
         );

FC_REFLECT( node::protocol::asset_settle_operation, 
         (account)
         (amount)
         (extensions) 
         );

FC_REFLECT( node::protocol::asset_settle_cancel_operation, 
         (settlement)
         (account)
         (amount)
         (extensions) 
         );

FC_REFLECT( node::protocol::asset_global_settle_operation, 
         (issuer)
         (asset_to_settle)
         (settle_price)
         (extensions) 
         );

FC_REFLECT( node::protocol::asset_issue_operation, 
         (issuer)
         (asset_to_issue)
         (issue_to_account)
         (memo)
         (extensions) 
         );

FC_REFLECT( node::protocol::asset_reserve_operation, 
         (payer)
         (amount_to_reserve)
         (extensions) 
         );

FC_REFLECT( node::protocol::asset_fund_fee_pool_operation, 
         (from_account)
         (symbol)
         (amount)
         (extensions) 
         );