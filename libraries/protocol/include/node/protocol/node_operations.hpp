#pragma once

#include <node/protocol/base.hpp>
#include <node/protocol/block_header.hpp>
#include <node/protocol/utilities.hpp>
#include <node/protocol/asset.hpp>
#include <node/protocol/x11.hpp>

#include <fc/utf8.hpp>

namespace node { namespace protocol {

   using node::protocol::authority;
   using node::protocol::block_id_type;
   using node::protocol::transaction_id_type;
   using node::protocol::chain_id_type;
   using node::protocol::account_name_type;
   using node::protocol::asset_symbol_type;
   using node::protocol::community_name_type;
   using node::protocol::graph_node_name_type;
   using node::protocol::graph_edge_name_type;
   using node::protocol::fixed_string_32;
   using node::protocol::tag_name_type;
   using node::protocol::share_type;
   using node::protocol::asset;
   using node::protocol::price;
   using node::protocol::price_feed;
   using node::protocol::option_strike;
   using node::protocol::asset_unit;
   using node::protocol::x11;


   //============================//
   //==== Account Operations ====//
   //============================//

   /**
    * @defgroup operations Blockchain Operations
    * @{
    */
   

   /**
    * Creates a brand new WeYouMe account for signing transactions and creating posts.
    * 
    * Referenced by username, contains account information.
    */
   struct account_create_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             registrar;                     ///< Account registering the new account, usually an interface.

      account_name_type             new_account_name;              ///< The name of the new account.

      account_name_type             referrer;                      ///< the account that lead to the creation of the new account, by way of referral link.

      account_name_type             proxy;                         ///< Account that the new account will delegate its voting power to.

      account_name_type             recovery_account;              ///< Account that can execute a recovery operation, in the event that the owner key is compromised. 

      account_name_type             reset_account;                 ///< Account that has the ability to execute a reset operation after 60 days of inactivity.

      string                        details;                       ///< The account's details string.

      string                        url;                           ///< The account's selected personal URL. 

      string                        image;                         ///< IPFS Reference of the profile image of the account. 

      string                        json;                          ///< The JSON string of public profile information.

      string                        json_private;                  ///< The JSON string of encrypted profile information. Encrypted with connection key.

      string                        first_name;                    ///< Encrypted First name of the user. Encrypted with connection key.

      string                        last_name;                     ///< Encrypted Last name of the user. Encrypted with connection key.

      string                        gender;                        ///< Encrypted Gender of the user. Encrypted with connection key.

      string                        date_of_birth;                 ///< Encrypted Date of birth of the user. Format: DD-MM-YYYY. Encrypted with connection key.

      string                        email;                         ///< Encrypted Email address of the user. Encrypted with connection key.

      string                        phone;                         ///< Encrypted Phone Number of the user. Encrypted with connection key.

      string                        nationality;                   ///< Encrypted Country of user's residence. Encrypted with connection key.

      authority                     owner_auth;                    ///< The account authority required for changing the active and posting authorities.

      authority                     active_auth;                   ///< The account authority required for sending payments and trading.

      authority                     posting_auth;                  ///< The account authority required for posting content and voting.

      string                        secure_public_key;             ///< The secure encryption key for content only visible to this account.

      string                        connection_public_key;         ///< The connection public key used for encrypting Connection level content.

      string                        friend_public_key;             ///< The connection public key used for encrypting Friend level content.

      string                        companion_public_key;          ///< The connection public key used for encrypting Companion level content.

      asset                         fee;                           ///< Account creation fee for stake on the new account.

      asset                         delegation;                    ///< Initial amount delegated to the new account.

      void                          validate()const;
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( registrar ); }
   };


   /**
    * Updates the details and authorities of an account.
    */
   struct account_update_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;                       ///< Name of the account to update.

      string                        details;                       ///< The account's Public details string.

      string                        url;                           ///< The account's Public personal URL.

      string                        image;                         ///< IPFS Reference of the Public profile image of the account.

      string                        json;                          ///< The JSON string of additional Public profile information.

      string                        json_private;                  ///< The JSON string of additional encrypted profile information. Encrypted with connection key.

      string                        first_name;                    ///< Encrypted First name of the user. Encrypted with connection key.

      string                        last_name;                     ///< Encrypted Last name of the user. Encrypted with connection key.

      string                        gender;                        ///< Encrypted Gender of the user. Encrypted with connection key.

      string                        date_of_birth;                 ///< Encrypted Date of birth of the user. Format: DD-MM-YYYY. Encrypted with connection key.

      string                        email;                         ///< Encrypted Email address of the user. Encrypted with connection key.

      string                        phone;                         ///< Encrypted Phone Number of the user. Encrypted with connection key.

      string                        nationality;                   ///< Encrypted Country of user's residence. Encrypted with connection key.

      authority                     owner_auth;                    ///< Creates a new owner authority for the account, changing the key and account auths required to sign transactions.

      authority                     active_auth;                   ///< Creates a new active authority for the account, changing the key and account auths required to sign transactions.

      authority                     posting_auth;                  ///< Creates a new posting authority for the account, changing the key and account auths required to sign transactions.

      string                        secure_public_key;             ///< The secure encryption key for content only visible to this account.

      string                        connection_public_key;         ///< The connection public key used for encrypting Connection level content.

      string                        friend_public_key;             ///< The connection public key used for encrypting Friend level content.

      string                        companion_public_key;          ///< The connection public key used for encrypting Companion level content.

      string                        pinned_permlink;               ///< Permlink of the users pinned post.

      bool                          active = true;                 ///< True when account is active. False to set account as inactive.

      void                          validate()const;
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Enables the verification of an account by another account.
    * 
    * The verifier account declares that in their view,
    * the owner of the verified account is a unique human person.
    * 
    * They upload an image of both people in the same picture, 
    * holding a hand writen note containing both account names
    * and a recent head_block_id of the blockchain.
    */
   struct account_verification_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             verifier_account;      ///< Name of the Account with the profile.

      account_name_type             verified_account;      ///< Name of the account being verifed.

      string                        shared_image;          ///< IPFS reference to an image containing both people and a recent head block id.

      bool                          verified = true;       ///< True to verify the account, false to remove verification.

      void                          validate()const;
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( verifier_account ); }
   };


   /**
    * Creates or Updates the business structure of an account.
    * 
    * Contains a heirarchial structure of members, officers, and 
    * executives that can be used for managing an enterprise of multiple
    * accounts, and giving individuals delegated control over account
    * transaction signatory authority. 
    * 
    * Able to issue additional assets
    * that distribute portions of incoming revenue.
    * Options for control structures that offer different 
    * authority dynamics, from most permissive to most restrictive.
    */
   struct account_business_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;                  ///< Name of the business account to update.

      account_name_type             init_ceo_account;         ///< Name of the account that should become the initial Chief Executive Officer.

      string                        business_type;            ///< The type of business account being created.

      share_type                    officer_vote_threshold;   ///< The voting power required to be an active officer.

      string                        business_public_key;      ///< The public key used for encrypted business content.

      bool                          active = true;            ///< True when the business account is active, false to deactivate business account.

      void                          validate()const;
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Accounts can purchase a membership subscription for their account to gain
    * protocol benefits:
    * 
    * Top Level Membership:
    * Price $250.00 per month.

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
      account_name_type             signatory;

      account_name_type             account;              ///< The name of the account to activate membership on.

      string                        membership_type;      ///< The level of membership to activate on the account.

      uint16_t                      months;               ///< Number of months to purchase membership for.

      account_name_type             interface;            ///< Name of the interface application facilitating the transaction.

      bool                          recurring = true;     ///< True for membership to automatically recur each month from liquid balance. 

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Votes for an account to become an executive of a business account, weighted according to equity holdings.
    */
   struct account_vote_executive_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;               ///< Account creating the executive vote. 

      account_name_type             business_account;      ///< Business account that the executive is being voted for.

      account_name_type             executive_account;     ///< Name of executive being voted for.

      string                        role;                  ///< Role of the executive.

      uint16_t                      vote_rank = 1;         ///< Rank of voting preference.

      bool                          approved = true;       ///< True to add, false to remove. 

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( business_account ); }
   };


   /**
    * Votes for an account to become an officer of a business account, 
    * weighted according to equity holdings.
    */
   struct account_vote_officer_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;               ///< Name of the voting account.

      account_name_type             business_account;      ///< Business account.

      account_name_type             officer_account;       ///< Name of officer being voted for.

      uint16_t                      vote_rank = 1;         ///< Rank of voting preference.

      bool                          approved = true;       ///< True to add, false to remove. 

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( business_account ); }
   };


   /**
    * Requests that an account be added to the membership of a business account.
    */
   struct account_member_request_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;              ///< Account requesting to be a member of the business.

      account_name_type             business_account;     ///< Business account that the member is being added to.

      string                        message;              ///< Encrypted Message to the business members requesting membership.

      bool                          requested = true;     ///< True to add, false to remove. 

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Invites an account to be a member of a business account.
    */
   struct account_member_invite_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;                   ///< Business account or the member account, or an officer of the business account.

      account_name_type             business_account;          ///< Business account that the member is being added to.

      account_name_type             member;                    ///< Name of member being added.

      string                        message;                   ///< Encrypted Message to the account being invited.

      string                        encrypted_business_key;    ///< Encrypted Copy of the private key of the business.

      bool                          invited = true;            ///< True to add, false to remove.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Accepts an account's request to be added as a business account member.
    */
   struct account_accept_request_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;                   ///< Account that is accepting the request to add a new member.

      account_name_type             business_account;          ///< Business account that the member is being added to.

      account_name_type             member;                    ///< Name of member being accepted.

      string                        encrypted_business_key;    ///< Encrypted Copy of the private key of the business.

      bool                          accepted = true;           ///< True to accept, false to reject. 

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Accepts an invitation to be added as a business account member.
    */
   struct account_accept_invite_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;              ///< Account accepting the invitation.

      account_name_type             business_account;     ///< Business account that the account was invited to.

      bool                          accepted = true;      ///< True to accept, false to reject.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Removes a member from the membership of a business account.
    */
   struct account_remove_member_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;              ///< Business account or an executive of the business account.

      account_name_type             business_account;     ///< Business account that the member is being removed from.

      account_name_type             member;               ///< Name of member being accepted.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Blacklists an account or asset.
    */
   struct account_update_list_operation : public base_operation
   {
      account_name_type                 signatory;

      account_name_type                 account;              ///< Name of account.

      optional< account_name_type >     listed_account;       ///< Name of account being added to a black or white list.

      optional< asset_symbol_type >     listed_asset;         ///< Name of asset being added to a black or white list.

      bool                              blacklisted = true;   ///< True to add to blacklist, false to remove. 

      bool                              whitelisted = false;  ///< True to add to whitelist, false to remove. 

      void                              validate() const;
      void                              get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                              get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Vote for a producer to be selected for block production. 
    * 
    * All accounts with voting power can vote for producers to produce blocks, 
    * the top 50 voted producers are able to produce one block each round, 
    * combined with the top 50 miners.
    * If a proxy is specified then all existing votes are removed.
    */
   struct account_producer_vote_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;           ///< Account creating the vote.

      uint16_t                      vote_rank = 1;     ///< Rank ordering of the vote.

      account_name_type             producer;          ///< producer being voted for.

      bool                          approved = true;   ///< True to create vote, false to remove vote.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Updates the Proxy account for a specified account.
    * 
    * Proxy is able to vote for producers, network officers and 
    * additional network functionalities on behalf of the account.
    */
   struct account_update_proxy_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;      ///< The name of the account to update.

      account_name_type             proxy;        ///< The name of account that should proxy to, or empty string to have no proxy.

      void                          validate()const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Creates an account recovery request to change its owner authority.
    * 
    * The account to recover has 24 hours to respond to before the 
    * request expires and is invalidated.
    * 
    * All account recovery requests come from a listed recovery account. This
    * is secure based on the assumption that only a trusted account should be
    * a recovery account. It is the responsibility of the recovery account to
    * verify the identity of the account holder of the account to recover by
    * whichever means they have agreed upon. The blockchain assumes identity
    * has been verified when this operation is broadcast.
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
      account_name_type             signatory;

      account_name_type             recovery_account;       ///< The recovery account is listed as the recovery account on the account to recover.

      account_name_type             account_to_recover;     ///< The account to recover. This is likely due to a compromised owner authority.

      authority                     new_owner_authority;    ///< The new owner authority the account to recover wishes to have.

      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( recovery_account ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          validate() const;
   };


   /**
    * Recover an account to a new authority using a previous authority.
    * 
    * Uses verification of the recovery account as proof of identity. 
    * This operation can only succeed
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
      account_name_type             signatory;

      account_name_type             account_to_recover;        ///< The account to be recovered

      authority                     new_owner_authority;       ///< The new owner authority as specified in the request account recovery operation.

      authority                     recent_owner_authority;    ///< A previous owner authority that the account holder will use to prove past ownership of the account to be recovered.

      void                          get_required_authorities( vector< authority >& a )const { a.push_back( new_owner_authority ); a.push_back( recent_owner_authority ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const { a.insert( account_to_recover ); }
      void                          validate() const;
   };


   /**
    * Allows @ref reset_account to change @ref account_to_reset's owner authority.
    * 
    * Enabled after 7 days of inactivity.
    */
   struct reset_account_operation : public base_operation 
   {
      account_name_type             signatory;

      account_name_type             reset_account;          ///< The account that is initiating the reset process.

      account_name_type             account_to_reset;       ///< The Account to be reset.

      authority                     new_owner_authority;    ///< A recent owner authority on your account.

      void                          get_required_active_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( reset_account ); }
      void                          validate()const;
   };


   /**
    * Allows @ref account owner to control which account has the power to reset.
    * 
    * This is done using the reset_account_operation after a specified duration of days.
    */
   struct set_reset_account_operation : public base_operation 
   {
      account_name_type             signatory;

      account_name_type             account;             ///< Account to update.

      account_name_type             new_reset_account;   ///< Account that has the new authority to reset the account.

      uint16_t                      days = 7;            ///< Days of inactivity required to reset the account. 

      void                          validate()const;
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Lists another account as a recovery account.
    * 
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
    * has the top voted producer as a recovery account, at the time the recover
    * request is created. Note: This does mean the effective recovery account
    * of an account with no listed recovery account can change at any time as
    * producer vote weights. The top voted producer is explicitly the most trusted
    * producer according to stake.
    */
   struct change_recovery_account_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account_to_recover;     ///< The account being updated.

      account_name_type             new_recovery_account;   ///< The account that is authorized to create recovery requests.

      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account_to_recover ); }
      void                          validate() const;
   };


   /**
    * Removes an account's ability to vote in perpetuity.
    * 
    * Used for the purposes of ensuring that funds held 
    * in trust that are owned by third parties
    * are not used for undue influence over the network.
    */
   struct decline_voting_rights_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;             ///< The account being updated.

      bool                          declined = true;     ///< True to decine voting rights, false to cancel pending request.

      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          validate() const;
   };


   /**
    * Requests that a connection be created between two accounts.
    * 
    * Used for the purposes of exchanging encrypted 
    * connection keys that can be used to decrypt
    * private posts made by the account, and enables the creation of private 
    * message transactions between the two accounts. 
    * 
    * Initiates a 3 step connection exchange process:
    * 
    * 1 - Alice sends Request txn.
    * 2 - Bob sends Acceptance txn with encrypted private key.
    * 3 - Alice sends returns with Acceptance txn with encrypted private key.
    */
   struct connection_request_operation : public base_operation 
   {
      account_name_type             signatory;

      account_name_type             account;                        ///< Account sending the request.

      account_name_type             requested_account;              ///< Account that is being requested to connect.

      string                        connection_type;                ///< Type of connection level.

      string                        message;                        ///< Message attached to the request, encrypted with recipient's secure public key.

      bool                          requested = true;               ///< Set true to request, false to cancel request.

      void                          validate()const;
      void                          get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Accepts an incoming connection request by providing an encrypted posting key.
    * 
    * @ref encrypted_key should be decryptable by the recipient that initiated the request.
    * 
    * This operation assumes that the account includes a valid encrypted private key
    * and that the initial account confirms by returning an additional acceptance transaction.
    * The protocol cannot ensure the reciprocity of the original sender, or
    * the accuracy of the private keys included.
    */
   struct connection_accept_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;               ///< Account accepting the request.

      account_name_type             requesting_account;    ///< Account that originally requested to connect.

      string                        connection_id;         ///< uuidv4 for the connection, for local storage of decryption key.

      string                        connection_type;       ///< Type of connection level.

      string                        encrypted_key;         ///< The private connection key of the user, encrypted with the public secure key of the requesting account.

      bool                          connected = true;      ///< Set true to connect, false to delete connection.

      void                          validate()const;
      void                          get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Enables an account to follow another account.
    *  
    * Adds @ref follower to the account's @ref account_following_object and displays
    * posts and shares from the account in their home feed.
    */
   struct account_follow_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             follower;             ///< Account that is creating the new follow relationship.

      account_name_type             following;            ///< Account that is being followed by follower.

      account_name_type             interface;            ///< Interface account that was used to broadcast the transaction. 

      bool                          added = true;         ///< Set true to add to list, false to remove from list.

      bool                          followed = true;      ///< Set true to follow, false to filter.

      void                          validate()const;
      void                          get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( follower ); }
   };


   /**
    * Enables an account to follow a tag.
    * 
    * Includes the tag in the accounts following object, and displays
    * posts and shares from posts that use the tag in their feeds.
    */
   struct tag_follow_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             follower;           ///< Name of the account following the tag.

      tag_name_type                 tag;                ///< Tag being followed.

      account_name_type             interface;          ///< Name of the interface account that was used to broadcast the transaction. 

      bool                          added = true;       ///< Set true to add to list, false to remove from list.

      bool                          followed = true;    ///< Set true to follow, false to filter.

      void                          validate()const;
      void                          get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( follower ); }
   };


   /**
    * Claims an account's daily activity reward.
    * 
    * Activity rewards can be claimed by each account 
    * every day for a share of the activity reward pool.
    * 
    * To be eligible, accounts need:
    * 
    * 1 - A staked balance of at least one EQUITY ASSET.
    * 2 - A recent comment transaction with at least 20% of the median number of votes and views, and vote and view power on all posts in the last 30 days.
    * 3 - A recent vote transaction.
    * 4 - A recent view transaction.
    * 5 - At least 10 producer votes.
    */
   struct activity_reward_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;        ///< Name of the account claiming the reward.

      string                        permlink;       ///< Permlink of the users recent post in the last 24h.

      account_name_type             interface;      ///< Name of the interface account that was used to broadcast the transaction. 

      void                          validate()const;
      void                          get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   //============================//
   //==== Network Operations ====//
   //============================//


   /**
    * Creates or updates a network officer object for a member.
    * 
    * Network officers receive a reward dsitribution from each block
    * based on the amount of votes they have received from supporters of their
    * work to develop improvements the protocol or its interfaces, 
    * market it to new users, and advocate it to businesses and organisations. 
    */
   struct update_network_officer_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;           ///< Name of the member's account.

      string                        officer_type;      ///< The type of network officer that the account serves as. 

      string                        details;           ///< Information about the network officer and their work.

      string                        url;               ///< The officers's description URL explaining their details. 

      string                        json;              ///< Additional information about the officer.

      bool                          active = true;     ///< Set true to activate the officer, false to deactivate. 
         
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          validate() const;
   };


   /**
    * Votes to support a network officer.
    * 
    * Top voted Network Officers receive a reward distribution to compensate them
    * for the work they have done.
    */
   struct network_officer_vote_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;               ///< The name of the account voting for the officer.

      account_name_type              network_officer;       ///< The name of the network officer being voted for.

      uint16_t                       vote_rank = 1;         ///< Number of vote rank ordering.

      bool                           approved = true;       ///< True if approving, false if removing vote.

      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           validate() const;
   };


   /**
    * Set of all executive officers that can be allocated in a business account.
    */
   struct executive_officer_set
   {
      account_name_type                 CHIEF_EXECUTIVE_OFFICER;    ///< Overall leader of Executive team.

      account_name_type                 CHIEF_OPERATING_OFFICER;    ///< Manages communications and coordination of team.

      account_name_type                 CHIEF_FINANCIAL_OFFICER;    ///< Oversees Credit issuance and salaries.

      account_name_type                 CHIEF_DEVELOPMENT_OFFICER;  ///< Oversees protocol development and upgrades.

      account_name_type                 CHIEF_TECHNOLOGY_OFFICER;   ///< Manages front end interface stability and network infrastructure.

      account_name_type                 CHIEF_SECURITY_OFFICER;     ///< Manages security of servers, funds, wallets, keys, and cold storage.

      account_name_type                 CHIEF_GOVERNANCE_OFFICER;   ///< Manages Governance account moderation teams.

      account_name_type                 CHIEF_MARKETING_OFFICER;    ///< Manages public facing communication and campaigns.
      
      account_name_type                 CHIEF_DESIGN_OFFICER;       ///< Oversees graphical design and User interface design.

      account_name_type                 CHIEF_ADVOCACY_OFFICER;     ///< Coordinates advocacy efforts and teams.

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
    * Creates or updates an executive board object.
    * 
    * Used for a development and administration team of a business account.
    * 
    * Executive boards enable the issuance of network credit asset for operating a
    * multifaceted development and marketing team for the protocol.
    * 
    * Executive boards must:
    * 
    * 1 - Hold a minimum balance of 100 Equity assets.
    * 2 - Operate an active supernode with at least 100 daily active users.
    * 3 - Operate an active interface with at least 100 daily active users.
    * 4 - Operate an active governance account with at least 100 subscribers.
    * 5 - Have at least 3 members or officers that are top 50 network officers, 1 from each role.
    * 6 - Have at least one member or officer that is an active top 50 producers.
    */
   struct update_executive_board_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;           ///< Name of the account updating the executive board.

      account_name_type             executive;         ///< Name of the Executive board account being updated.

      asset                         budget;            ///< Total amount of Credit asset requested for team compensation and funding.

      string                        details;           ///< Information about the board and their team.

      string                        url;               ///< The teams's description URL explaining their details. 

      string                        json;              ///< Additional information about the executive board.

      bool                          active = true;     ///< Set true to activate the board, false to deactivate. 
         
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          validate() const;
   };


   /**
    * Votes to support an executive board.
    */
   struct executive_board_vote_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;               ///< The name of the account voting for the community.

      account_name_type              executive_board;       ///< The name of the executive board being voted for.

      uint16_t                       vote_rank = 1;         ///< Vote preference ranking.

      bool                           approved = true;       ///< True if approving, false if removing vote.

      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           validate() const;
   };


   /**
    * Creates or updates a governance account for a member.
    * 
    * Provides moderation and profile account services to the network 
    * in exchange for a share of the revenue generated by their subscribers.
    */
   struct update_governance_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;           ///< Name of the governance account.

      string                         details;           ///< Information about the governance account's filtering and tagging policies

      string                         url;               ///< The governance account's description URL explaining their details. 

      string                         json;              ///< Additional information about the governance account policies.

      bool                           active = true;     ///< Set true to activate governance account, false to deactivate. 
         
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           validate() const;
   };


   /** 
    * Adds a governance account to the subscription set of the account. 
    * 
    * This causes its content moderation tags to apply to posts requested by
    * the account in interfaces.
    * 
    * This allows an account to opt in to the moderation and enforcement 
    * policies provided by the governance account, and to 
    * its default settings for whitelisting and blacklisting mediators, assets,
    * communities, authors, and interfaces.
    */
   struct subscribe_governance_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;             ///< The account subscribing to the governance address
      
      account_name_type              governance_account;  ///< The name of the governance account.

      bool                           subscribe = true;    ///< True if subscribing, false if unsubscribing.

      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           validate() const;
   };


   /**
    * Creates or updates a supernode object for an infrastructure provider.
    * 
    * Supernodes are required to:
    * 
    * 1 - Have a minimum stake of 1 Equity and 100 Coin ( 10 Equity and 1000 Coin receive doubled reward share ).
    * 2 - Operate a full archive node.
    * 3 - Operate a public API endpoint to their node that provides all network API and transaction broadcasting functions. 
    * 4 - Operate a public IPFS gateway endpoint for file upload and download.
    * 5 - Operate a public Bittorrent Seed API endpoint.
    * 6 - Operate an authentication API endpoint for transaction signing.
    * 7 - Operate a notification API endpoint for receiving account notifications.
    * 
    * Supernodes receive rewards from the network proportionately with their 7 day stake weighted average file views.
    * Users dynamically download IPFS media files from the Supernodes with the lowest ping via their gateway endpoint.
    * Users upload IPFS media files to multiple active Supernodes via their gateway endpoints.
    * Users upload videos files to mutliple bittorrent seed nodes for distribution via webtorrent. 
    * User select the Node, Auth and notification API endpoints with the lowest ping when using the WeYouMe applications. 
    * Supernodes need to remain in the Supernode reward pool for 24 consecutive hours before being eligible for reward distribution.
    * 
    * Open Problem: Ensure that Interfaces correctly attribute Supernodes in view transactions when using their APIs.
    */
   struct update_supernode_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;                     ///< Name of the member's account.

      string                         details;                     ///< Information about the supernode, and the range of storage and node services they operate.

      string                         url;                         ///< The supernode's reference URL.

      string                         node_api_endpoint;           ///< The Full Archive node public API endpoint of the supernode.

      string                         notification_api_endpoint;   ///< The Notification API endpoint of the Supernode. 

      string                         auth_api_endpoint;           ///< The Transaction signing authentication API endpoint of the supernode.

      string                         ipfs_endpoint;               ///< The IPFS file storage API endpoint of the supernode.

      string                         bittorrent_endpoint;         ///< The Bittorrent Seed Box endpoint URL of the Supernode. 

      string                         json;                        ///< Additional information about the Supernode.

      bool                           active = true;               ///< Set true to activate the supernode, false to deactivate. 
         
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           validate() const;
   };


   /**
    * Creates or updates an interface object for an application developer.
    * 
    * Enables an application to provide advertising inventory on the 
    * advertising exchange, in return for payments from advertisers, and
    * to earn a share of fees generated from memberships, trading fees,
    * premium content sales, and marketplace fees. 
    */
   struct update_interface_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;           ///< Name of the member's account.

      string                         details;           ///< Information about the interface, and what they are offering to users.

      string                         url;               ///< The interfaces's URL.

      string                         json;              ///< Additional information about the interface.

      bool                           active = true;     ///< Set true to activate the interface, false to deactivate. 
         
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           validate() const;
   };



   /**
    * Creates or updates a mediator object for marketplace escrow facilitator.
    * 
    * Enables a community member or business to provide escrow mediation 
    * services to marketplace participants, acting to resolve disputes
    * in escrow transfers, and earn marketplace fees from services rendered. 
    */
   struct update_mediator_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;           ///< Name of the member's account.

      string                         details;           ///< Information about the mediator, and what they are offering to users

      string                         url;               ///< The mediator's reference URL.

      string                         json;              ///< Additional information about the mediator.

      asset                          mediator_bond;     ///< Amount of Core asset to stake in the mediation pool. 

      bool                           active = true;     ///< Set true to activate the mediator, false to deactivate. 
         
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           validate() const;
   };


   /**
    * Creates a new community enterprise proposal.
    * 
    * Provides funding to projects that work to benefit the network by
    * distributing payments based on milestone completion.
    * 
    * A proposal becomes active when its first milestone is approved.
    */
   struct create_community_enterprise_operation : public base_operation
   {
      account_name_type                                 signatory;

      account_name_type                                 creator;             ///< The name of the account that created the community enterprise proposal.

      string                                            enterprise_id;       ///< uuidv4 referring to the proposal.

      flat_map< account_name_type, uint16_t >           beneficiaries;       ///< Set of account names and percentages of budget value. Should not include the null account.

      vector< uint16_t >                                milestone_shares;    ///< Ordered vector of release milestone descriptions.

      string                                            details;             ///< The proposals's details description.

      string                                            url;                 ///< The proposals's reference URL. 

      string                                            json;                ///< Json metadata of the proposal. 

      time_point                                        begin;               ///< Enterprise proposal start time.

      uint16_t                                          duration;            ///< Number of days that the proposal will be paid for.

      asset                                             daily_budget;        ///< Daily amount requested for project compensation and funding.

      asset                                             fee;                 ///< Amount of Asset paid to community fund to apply.

      bool                                              active = true;       ///< True to set the proposal to activate, false to deactivate an existing proposal and delay funding. 
         
      void                                              get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                                              get_creator_name( flat_set<account_name_type>& a )const{ a.insert( creator ); }
      void                                              validate() const;
   };


   /**
    * Claims a milestone from a community enterprise proposal.
    * 
    * This requests to release the funds in the pending budget to the beneficiaries
    * up to the percentage allocated to the milestone.
    */
   struct claim_enterprise_milestone_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              creator;               ///< The name of the account that created the proposal.

      string                         enterprise_id;         ///< UUIDv4 referring to the proposal.

      uint16_t                       milestone = 0;         ///< Number of the milestone that is being claimed as completed. Number 0 for initial acceptance. 
         
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( creator ); }
      void                           validate() const;
   };


   /**
    * Approves a milestone claim from a community enterprise proposal.
    * 
    * This releases the funds that are in the pending budget to the proposal's beneficaries.
    * Community Enterprise proposals need to be approved by:
    * 
    * - Approvals from at least 5 of the Top 50 producers, with a combined voting power of at least 1% of the total producer voting power.
    * AND
    * - At least 20 total approvals, from accounts with a total combined voting power of at least 1% of total voting power. 
    */
   struct approve_enterprise_milestone_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;           ///< Account approving the milestone.

      account_name_type              creator;           ///< The name of the account that created the proposal.

      string                         enterprise_id;     ///< UUIDv4 referring to the proposal. 

      int16_t                        milestone = 0;     ///< Number of the milestone that is being approved as completed.

      int16_t                        vote_rank = 1;     ///< The rank of the approval for enterprise proposals.

      bool                           approved = true;   ///< True to approve the milestone claim, false to remove approval. 
         
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           validate() const;
   };
   


   //=====================================//
   //==== Post and Comment Operations ====//
   //=====================================//



   struct beneficiary_route_type
   {
      beneficiary_route_type() {}
      beneficiary_route_type( const account_name_type& a, const uint16_t& w ) : account( a ), weight( w ){}

      account_name_type           account;

      uint16_t                    weight;

      ///< For use by std::sort such that the route is sorted first by name (ascending)
      bool operator < ( const beneficiary_route_type& o )const { return account < o.account; }
   };


   struct comment_payout_beneficiaries
   {
      vector< beneficiary_route_type >            beneficiaries;

      void validate()const;
   };

   /**
    * Allows authors to update properties associated with their post.
    * 
    * The max_accepted_payout may be decreased, but never increased.
    * The percent_liquid may be decreased, but never increased.
    */
   struct comment_options
   {
      string                                post_type = post_format_values[0];  ///< Type of post being created, text, image, article, video, audio, file, etc.

      string                                reach = feed_reach_values[7];       ///< The extent to which the post will be distributed to account's followers and connections feeds.

      string                                reply_connection = connection_tier_values[0];  ///< Replies to the comment must be connected to the author to at least this level. 

      uint16_t                              rating = 1;                         ///< User nominated rating as to the maturity of the content, and display sensitivity.

      asset_symbol_type                     reward_currency = SYMBOL_COIN;      ///< The reward currency that the post will earn.

      asset                                 max_accepted_payout = MAX_ACCEPTED_PAYOUT;   ///< USD value of the maximum payout this post will receive.
      
      uint16_t                              percent_liquid = PERCENT_100;       ///< Percentage of reward to keep liquid, remaining received as a staked balance.

      bool                                  allow_replies = true;               ///< Allows a post to receive comment replies.
      
      bool                                  allow_votes = true;                 ///< Allows a post to receive votes.

      bool                                  allow_views = true;                 ///< Allows a post to receive views.

      bool                                  allow_shares = true;                ///< Allows a post to receive shares.
      
      bool                                  allow_curation_rewards = true;      ///< allows voters, viewers, sharers, and commenters to receive curation rewards.

      vector< beneficiary_route_type >      beneficiaries;                      ///< Vector of accounts that will receive an allocation of content rewards from the post.

      void validate()const;
   };

   
   /**
    * Creates a new comment from an author.
    * 
    * Comments are the primary unit of content on the network, 
    * and allow for users to share posts with their followers and communities.
    * 
    * Users can create multiple types of posts including:
    * 
    * - Text Posts: Short text only posts up to 300 characters.
    * - Article posts: Long Text and image posts, formatted using markdown.
    * - Image Posts: contains a single image, contained as an IPFS file reference.
    * - Video Posts: contains a single video, contained as a magnet Bittorrent fileswarm reference.
    * - Link Posts: contains a URL link, and a description.
    * 
    * Posts can be public, and unencrypted, or private 
    * and encrypted with a @ref public_key that a desired audience 
    * has been given access to prior to publication.
    * 
    * Posts can be made to communities, which collect 
    * posts under a common topic for a community, and are 
    * moderated by a group specified by the founder of the community.
    * 
    * Posts earn an allocation of Content rewards from coin issuance, 
    * depending on the amount of Votes, Views, Shares, 
    * and Comments that it receives, and the voting power that each 
    * interacting account has.
    */
   struct comment_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              author;               ///< Name of the account that created the post.

      string                         permlink;             ///< Unique identifing string for the post.

      string                         title;                ///< Content related name of the post, used to find post with search API.

      string                         body;                 ///< String containing text for display when the post is opened.

      string                         ipfs;                 ///< IPFS file hash: images, videos, files.

      string                         magnet;               ///< Bittorrent magnet links to torrent file swarms: videos, files.

      string                         url;                  ///< String containing a URL for the post to link to.

      string                         language;             ///< String containing the two letter ISO language code of the native language of the author.

      community_name_type            community;            ///< The name of the community to which the post is uploaded to.

      string                         public_key;           ///< The public key used to encrypt the post, holders of the private key may decrypt.

      account_name_type              interface;            ///< Name of the interface application that broadcasted the transaction.

      double                         latitude;             ///< Latitude co-ordinates of the comment.

      double                         longitude;            ///< Longitude co-ordinates of the comment.

      asset                          comment_price;        ///< Price that is required to comment on the post.

      asset                          reply_price;          ///< Price that is paid to the comment root author when the root author replies.

      asset                          premium_price;        ///< Price that is required to unlock premium content.

      account_name_type              parent_author;        ///< Account that created the post this post is replying to, empty if root post.

      string                         parent_permlink;      ///< Permlink of the post this post is replying to, empty if root post.

      vector< tag_name_type >        tags;                 ///< Set of string tags for sorting the post by.

      string                         json;                 ///< json string of additional interface specific data relating to the post.

      comment_options                options;              ///< Settings for the post, that effect how the network applies and displays it. 

      bool                           deleted = false;      ///< True to delete post, false to create post. 

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( author ); }
   };


   /**
    * Creates a private encrypted message between two accounts.
    * 
    * Collected into an inbox structure for direct private messaging
    * using a series of encrypted private keys, exchanged during a @ref connection_accept_operation
    */
   struct message_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              sender;         ///< The account sending the message.

      account_name_type              recipient;      ///< The receiving account of the message.

      string                         message;        ///< Encrypted ciphertext of the message being sent. 

      string                         uuid;           ///< uuidv4 uniquely identifying the message for local storage.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( sender ); }
   };


   /**
    * Votes for a comment to allocate content rewards and increase the posts ranked ordering.
    */
   struct vote_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              voter;           ///< Name of the voting account.

      account_name_type              author;          ///< Name of the account that created the post being voted on.

      string                         permlink;        ///< Permlink of the post being voted on.

      int16_t                        weight = 0;      ///< Percentage weight of the voting power applied to the post.

      account_name_type              interface;       ///< Name of the interface account that was used to broadcast the transaction. 

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( voter ); }
   };


   /** 
    * Views a post, which increases the post's content reward earnings.
    * 
    * Nominates an interface through which the post was viewed,
    * and nominates a supernode that served the data through IPFS.
    */
   struct view_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              viewer;           ///< Name of the viewing account.

      account_name_type              author;           ///< Name of the account that created the post being viewed. 

      string                         permlink;         ///< Permlink to the post being viewed.

      account_name_type              interface;        ///< Name of the interface account that was used to broadcast the transaction and view the post. 

      account_name_type              supernode;        ///< Name of the supernode account that served the IPFS file data in the post. 

      bool                           viewed = true;    ///< True if viewing the post, false if removing view object.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( viewer ); }
   };


   /**
    * Shares a post to the account's feed.
    * 
    * All accounts that follow them can then see the post. 
    * Increases the share count of the post
    * and expands its visibility by adding it to feeds of new accounts.
    * Increases the content rewards earned by the post, and increases its rank ordering.
    */
   struct share_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              sharer;           ///< Name of the viewing account.

      account_name_type              author;           ///< Name of the account that created the post being shared.

      string                         permlink;         ///< Permlink to the post being shared.

      string                         reach;            ///< Audience reach selection for share.

      account_name_type              interface;        ///< Name of the interface account that was used to broadcast the transaction and share the post.

      optional< community_name_type >  community;      ///< Optionally share the post with a new community.

      optional< tag_name_type >      tag;              ///< Optionally share the post with a new tag.

      bool                           shared = true;    ///< True if sharing the post, false if removing share.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( sharer ); }
   };


   /**
    * Applies a set of tags to a post for filtering from interfaces.
    * 
    * Moderation tags enable interface providers to coordinate 
    * moderation efforts on-chain and provides a method for 
    * discretion to be provided to displaying content, 
    * based on the governance addresses subscribed to by the 
    * viewing user.
    * 
    * Tags should be based on the content included in the post. 
    * Accounts that list the moderator account as a 
    * community moderator or governance account apply the 
    * tag to the post for content management.
    * 
    * They can suggest a higher rating level if the rating selected
    * by the author was inaccurate. 
    */
   struct moderation_tag_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              moderator;          ///< Account creating the tag: can be a governance address or a community moderator. 

      account_name_type              author;             ///< Author of the post being tagged.

      string                         permlink;           ///< Permlink of the post being tagged.

      vector< tag_name_type >        tags;               ///< Set of tags to apply to the post for selective interface side filtering.

      uint16_t                       rating;             ///< Newly proposed rating for the post.

      string                         details;            ///< String explaining the reason for the tag to the author.

      account_name_type              interface;          ///< Interface account used for the transaction.

      bool                           filter = false;     ///< True if the post should be filtered from the community and governance account subscribers.

      bool                           applied = true;     ///< True if applying the tag, false if removing the tag.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( moderator ); }
   };


   /**
    * Lists contain a curated group of accounts, comments, communities and other objects.
    * 
    * Used to collect a group of objects for reference and browsing.
    */
   struct list_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              creator;             ///< Name of the account that created the list.

      string                         list_id;             ///< uuidv4 referring to the list.

      string                         name;                ///< Name of the list, unique for each account.

      flat_set< int64_t >            accounts;            ///< Account IDs within the list.

      flat_set< int64_t >            comments;            ///< Comment IDs within the list.

      flat_set< int64_t >            communities;         ///< Community IDs within the list.

      flat_set< int64_t >            assets;              ///< Asset IDs within the list.

      flat_set< int64_t >            products;            ///< Product IDs within the list.

      flat_set< int64_t >            auctions;            ///< Auction IDs within the list.

      flat_set< int64_t >            nodes;               ///< Graph node IDs within the list.

      flat_set< int64_t >            edges;               ///< Graph edge IDs within the list.

      flat_set< int64_t >            node_types;          ///< Graph node property IDs within the list.

      flat_set< int64_t >            edge_types;          ///< Graph edge property IDs within the list.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( creator ); }
   };


   /**
    * Polls enable accounts to vote on a series of options.
    * 
    * Polls have a fixed duration, and determine the winning option.
    */
   struct poll_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              creator;             ///< Name of the account that created the poll.

      string                         poll_id;             ///< uuidv4 referring to the poll.

      string                         details;             ///< Text describing the question being asked.

      vector< fixed_string_32 >      poll_options;        ///< Available poll voting options.

      time_point                     completion_time;     ///< Time the poll voting completes.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( creator ); }
   };


   /**
    * Poll Vote for a specified poll option.
    * 
    * Polls have a fixed duration, and determine the winning option.
    */
   struct poll_vote_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              voter;               ///< Name of the account that created the vote.

      account_name_type              creator;             ///< Name of the account that created the poll.

      string                         poll_id;             ///< uuidv4 referring to the poll.

      uint16_t                       poll_option;         ///< Poll option chosen.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( voter ); }
   };


   //==============================//
   //==== Community Operations ====//
   //==============================//


   /**
    * Creates a new community for collecting posts about a specific topic.
    * 
    * Communities have 8 Privacy and permission options: 
    * 
    * OPEN_PUBLIC_COMMUNITY:       All Users can read, interact, post, and request to join. Accounts cannot be blacklisted.
    * GENERAL_PUBLIC_COMMUNITY:    All Users can read, interact, post, and request to join.
    * EXCLUSIVE_PUBLIC_COMMUNITY:  All users can read, interact, and request to join. Members can post and invite.
    * CLOSED_PUBLIC_COMMUNITY:     All users can read, and request to join. Members can interact, post, and invite.
    * OPEN_PRIVATE_COMMUNITY:      Members can read and interact, and create posts. Moderators can invite and accept.
    * GENERAL_PRIVATE_COMMUNITY:   Members can read and interact, and create posts. Moderators can invite and accept. Cannot share posts.
    * EXCLUSIVE_PRIVATE_COMMUNITY: Members can read and interact, and post. Cannot share posts or request to join. Admins can invite and accept.
    * CLOSED_PRIVATE_COMMUNITY:    Members can read and interact. Moderators can post. Cannot share posts or request to join. Admins can invite and accept.
    * 
    * Communities contain a collective public key for encrypting private posts with
    * and the private key is shared with newly added members when they join.
    */
   struct community_create_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              founder;               ///< The account that created the community, able to add and remove administrators.

      community_name_type            name;                  ///< Name of the community.

      string                         community_privacy;     ///< Type of community Privacy to us, determines access permissions and encryption

      string                         community_public_key;  ///< Key used for encrypting and decrypting posts. Private key shared with accepted members.

      string                         json;                  ///< Public plaintext json information about the community, its topic and rules.

      string                         json_private;          ///< Private ciphertext json information about the community.

      string                         details;               ///< Details of the community, describing what it is for.

      string                         url;                   ///< External reference URL.

      asset_symbol_type              reward_currency = SYMBOL_COIN;  ///< The Currency asset used for content rewards in the community.

      uint16_t                       max_rating = 9;        ///< Highest severity rating that posts in the community can have.

      uint32_t                       flags = 0;             ///< The currently active flags on the community for content settings.

      uint32_t                       permissions = COMMUNITY_PERMISSION_MASK;  ///< The flag permissions that can be activated on the community for content settings.

      void                           validate()const;
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( founder ); }
   };

   /**
    * Updates the details of an existing community.
    * 
    * If the community public key is changed, all existing members must be reinitiated
    * by creating a new @ref community_member_key_object containing an encrypted copy of the new 
    * @ref community_public_key
    */
   struct community_update_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;               ///< Account updating the community. Administrator of the community.

      community_name_type            community;             ///< Name of the community.

      string                         community_public_key;  ///< Key used for encrypting and decrypting posts. Private key shared with accepted members.

      string                         json;                  ///< Public plaintext json information about the community, its topic and rules.

      string                         json_private;          ///< Private ciphertext json information about the community. Encrypted with community public key.

      string                         details;               ///< Details of the community, describing what it is for.

      string                         url;                   ///< External reference URL.

      account_name_type              pinned_author;         ///< Author of the pinned post.

      string                         pinned_permlink;       ///< Permlink of the pinned post.

      asset_symbol_type              reward_currency = SYMBOL_COIN;  ///< The Currency asset used for content rewards in the community.

      uint16_t                       max_rating = 9;        ///< Highest severity rating that posts in the community can have.

      uint32_t                       flags = 0;             ///< The currently active flags on the community for content settings.

      uint32_t                       permissions = COMMUNITY_PERMISSION_MASK;  ///< The flag permissions that can be activated on the community for content settings.

      bool                           active = true;         ///< True when the community is active, false to deactivate. 

      void                           validate()const;
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Adds a new moderator to a community.
    * 
    * Moderators have a heightened authority delegated to them
    * to enable the to enforce a community's rules and topic suitability.
    * Moderators all earn a share of the community's posts content rewards.
    */
   struct community_add_mod_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;           ///< Account of an administrator of the community.

      community_name_type            community;         ///< Community that the moderator is being added to.

      account_name_type              moderator;         ///< New moderator account.

      bool                           added = true;      ///< True when adding a new moderator, false when removing.

      void                           validate()const;
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Adds a new administrator to a community.
    * 
    * Admins have a very high authority delegated to them
    * to enable management and appointment of moderators.
    * Admins can update the details of a community. 
    */
   struct community_add_admin_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;          ///< Account of the founder of the community.

      community_name_type            community;        ///< Community that the admin is being added to.

      account_name_type              admin;            ///< New administrator account.

      bool                           added = true;     ///< True when adding a new administrator, false when removing.

      void                           validate()const;
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Votes for a moderator to increase their mod weight.
    * 
    * Moderators with a higher mod weight receive a higher 
    * proportion of incoming moderator rewards in a community.
    */
   struct community_vote_mod_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;          ///< Account of a member of the community.

      community_name_type            community;        ///< Community that the moderator is being voted into.

      account_name_type              moderator;        ///< Moderator account.

      uint16_t                       vote_rank = 1;    ///< Voting rank for the specified community moderator

      bool                           approved = true;  ///< True when voting for the moderator, false when removing.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Transfers a community to a new account as the founder.
    * 
    * Can be used to pass communities from one account to another
    * in the case of restructuring a moderation team
    * or a founder changing to a new account.
    */
   struct community_transfer_ownership_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;        ///< Account that created the community.

      community_name_type            community;      ///< Community that is being transferred.

      account_name_type              new_founder;    ///< Account of the new founder.

      void                           validate()const;
      void                           get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Requests that an account be added as a new member of a community.
    * 
    * Must be accepted using @ref community_join_accept_operation
    * by a member of the community.
    * 
    * Cannot be used to join an exclusive community.
    */
   struct community_join_request_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;            ///< Account that wants to join the community.

      community_name_type            community;          ///< Community that is being requested to join.

      string                         message;            ///< Message attatched to the request, encrypted with the communities public key. 

      bool                           requested = true;   ///< Set true to request, false to cancel request.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Invite a new member to a community.
    * 
    * The account must then accept the invitation to be added.
    * Invitation includes the @ref encrypted_community_key for
    * accessing private content within the community. 
    */
   struct community_join_invite_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;                   ///< Account sending the invitation.

      account_name_type              member;                    ///< New community member account being invited.

      community_name_type            community;                 ///< Community that is the member is being invited to.

      string                         message;                   ///< Message attatched to the invite, encrypted with the member's secure public key.

      string                         encrypted_community_key;   ///< The Community Private Key, encrypted with the member's secure public key.

      bool                           invited = true;            ///< Set true to invite, false to cancel invite.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Used to accept to a request and admit a new member.
    * 
    * This operation discloses the @ref encrypted_community_key
    * for decrypting posts, from an existing 
    * member of the community that has access to it. 
    */
   struct community_join_accept_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;                    ///< Account within the community accepting the request.

      account_name_type              member;                     ///< Account to accept into the community.

      community_name_type            community;                  ///< Community that is being joined.

      string                         encrypted_community_key;    ///< The Community Private Key, encrypted with the member's secure public key.

      bool                           accepted = true;            ///< True to accept request, false to reject request.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Accepts a community invitation.
    * 
    * Adds the invited account to become a new member of the
    * specifed community.
    */
   struct community_invite_accept_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;             ///< A new member of the community.

      community_name_type            community;           ///< Community that the account was invited to.

      bool                           accepted = true;     ///< True to accept invite, false to reject invite.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Removes a specifed member of a community.
    * 
    * Enables the moderation team of a community to remove
    * members that become problematic and fail to follow a communities rules,
    * or are added by mistake.
    */
   struct community_remove_member_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;        ///< Either the member of the community leaving OR a moderator of the community removing the member.

      account_name_type              member;         ///< Account to be removed from the community membership.

      community_name_type            community;      ///< Community that member is being removed from.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Adds a specifed account to the community's blacklist.
    * 
    * Blacklisted accounts cannot execute any operations related to the community,
    * such as requesting to join it, subscribe to it, or interact with any posts contained within it. 
    */
   struct community_blacklist_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;               ///< Moderator or admin of the community.

      account_name_type              member;                ///< Account to be blacklisted from interacting with the community.

      community_name_type            community;             ///< Community that member is being blacklisted from.

      bool                           blacklisted = true;    ///< Set to true to add account to blacklist, set to false to remove from blacklist. 

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Adds a community to an account's subscriptions.
    * 
    * Subscribed communities are included in an account's communities
    * feed, and can be browsed in feeds.
    */
   struct community_subscribe_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;             ///< Account that wants to subscribe to the community.

      community_name_type            community;           ///< Community to suscribe to.

      account_name_type              interface;           ///< Name of the interface account that was used to broadcast the transaction and subscribe to the community.

      bool                           added = true;        ///< True to add to lists, false to remove.
      
      bool                           subscribed = true;   ///< true if subscribing, false if filtering. 

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Creates or updates the event details within a community.
    * 
    * Events are created within Communities, 
    * and the membership of the community
    * is invited to the event.
    * 
    * Invite an account to join the community to
    * invite them to the event.
    */
   struct community_event_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;                ///< Account that created the event.

      community_name_type            community;              ///< Community that the event is within.

      string                         event_name;             ///< The Name of the event.

      string                         location;               ///< Address of the location of the event.

      double                         latitude;               ///< Latitude co-ordinates of the event.

      double                         longitude;              ///< Longitude co-ordinates of the event.

      string                         details;                ///< Event details describing the purpose of the event.

      string                         url;                    ///< Link containining additional event information.

      string                         json;                   ///< Additional Event JSON data.

      time_point                     event_start_time;       ///< Time that the Event will begin.

      time_point                     event_end_time;         ///< Time that the event will end.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Responds to an event invitation.
    * 
    * Denotes the status of an account attending an event.
    */
   struct community_event_attend_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              account;                ///< Account that is attending the event.

      community_name_type            community;              ///< Community that the event is within.

      bool                           interested = true;      ///< True to set interested in the event, and receive notifications about it, false to remove interedt status.

      bool                           attending = true;       ///< True to attend the event, false to remove attending status.

      bool                           not_attending = false;  ///< True to state not attending the event, false to remove not attending status.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   //================================//
   //==== Advertising Operations ====//
   //================================//


   /**
    * Creates a new ad creative to be used in a campaign for display in interfaces.
    */
   struct ad_creative_operation : public base_operation 
   {
      account_name_type        signatory;

      account_name_type        account;           ///< Account publishing the ad creative.

      account_name_type        author;            ///< Author of the objective item referenced.

      string                   objective;         ///< The reference of the object being advertised, the link and CTA destination of the creative.

      string                   creative_id;       ///< uuidv4 referring to the creative

      string                   creative;          ///< IPFS link to the Media to be displayed, image or video.

      string                   json;              ///< JSON string of creative metadata for display.

      string                   format_type;       ///< The type of formatting used for the advertisment, determines the interpretation of the creative.

      bool                     active = true;     ///< True if the creative is enabled for active display, false to deactivate.

      void                     validate()const;
      void                     get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Creates a new ad campaign to enable ad bidding.
    * 
    * Campaign contains the ad budget used to bid for display
    * in interfaces that are providing inventory.
    */
   struct ad_campaign_operation : public base_operation 
   {
      account_name_type                signatory;

      account_name_type                account;           ///< Account creating the ad campaign.

      string                           campaign_id;       ///< uuidv4 to refer to the campaign.

      asset                            budget;            ///< Total expenditure of the campaign.

      time_point                       begin;             ///< Beginning time of the campaign. Bids cannot be created before this time.

      time_point                       end;               ///< Ending time of the campaign. Bids cannot be created after this time.

      string                           json;              ///< json metadata for the campaign.

      vector< account_name_type >      agents;            ///< Set of Accounts authorized to create bids for the campaign.

      account_name_type                interface;         ///< Interface that facilitated the purchase of the advertising campaign.

      bool                             active = true;     ///< True if the campaign is enabled for bid creation, false to deactivate and reclaim the budget.

      void                             validate()const;
      void                             get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                             get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Declares the availability of a supply of ad inventory.
    * 
    * Ad inventory can be bidded on by ad campign owners, for display 
    * in the interfaces operated by the provider, to engage 
    * with their audience.
    */
   struct ad_inventory_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  provider;       ///< Account of an interface offering ad supply.

      string                             inventory_id;   ///< uuidv4 referring to the inventory offering.

      string                             audience_id;    ///< uuidv4 referring to audience object containing usernames of desired accounts in interface's audience.

      string                             metric;         ///< Type of expense metric used.

      asset                              min_price;      ///< Minimum bidding price per metric.

      uint32_t                           inventory;      ///< Total metrics available.

      string                             json;           ///< JSON metadata for the inventory.

      bool                               active = true;  ///< True if the inventory is enabled for display, false to deactivate.

      void                               validate()const;
      void                               get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( provider ); }
   };

   /**
    * Contains a set of accounts that are valid for advertising display.
    * 
    * Accounts in an ad audience are considered a targettign group by
    * an ad campaign, and an available display audience by
    * interface providers.
    */
   struct ad_audience_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  account;        ///< Account creating the audience set.

      string                             audience_id;    ///< uuidv4 referring to the audience for inclusion in inventory and campaigns.

      string                             json;           ///< JSON metadata for the audience.

      vector< account_name_type >        audience;       ///< List of usernames of viewing accounts.

      bool                               active = true;  ///< True if the audience is enabled for reference, false to deactivate.

      void                               validate()const;
      void                               get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };

   /**
    * Creates a new advertising bid offer.
    * 
    * Advertising bids can be delivered by interfaces when they
    * display an ad creative that leads to a specified transaction from
    * the audience member's account.
    * 
    * Ad bid payments are made after an operation is broadcast
    * to the network by an audience member, with an interface reference
    * to the provider of the inventory.
    */
   struct ad_bid_operation : public base_operation
   {
      account_name_type                signatory;

      account_name_type                bidder;                ///< Account that created the ad budget, or an agent of the campaign.
      
      string                           bid_id;                ///< Bid uuidv4 for referring to the bid.

      account_name_type                account;               ///< Account that created the campaign that the bid is directed towards.

      string                           campaign_id;           ///< Ad campaign uuidv4 to utilise for the bid.

      account_name_type                author;                ///< Account that was the author of the creative.

      string                           creative_id;           ///< uuidv4 referring to the creative item to bid on.

      account_name_type                provider;              ///< Account offering inventory supply.

      string                           inventory_id;          ///< Inventory uuidv4 offering to bid on.

      asset                            bid_price;             ///< Price offered per metric. Value must be in coin asset. 

      uint32_t                         requested;             ///< Maximum total metrics requested.

      vector< string >                 included_audiences;    ///< List of desired audiences for display acceptance, accounts must be in inventory audience.

      vector< string >                 excluded_audiences;    ///< List of audiences to remove all members from the combined bid audience.

      string                           audience_id;           ///< uuidv4 for the combined display acceptance.

      string                           json;                  ///< JSON metadata for the Bid and new inventory.

      time_point                       expiration;            ///< Time the the bid is valid until, bid is cancelled after this time if not filled. 

      bool                             active = true;         ///< True if the bid is open for delivery, false to cancel.

      void                             validate()const;
      void                             get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                             get_creator_name( flat_set<account_name_type>& a )const{ a.insert( bidder ); }
   };



   //==========================//
   //==== Graph Operations ====//
   //==========================//



   /**
    * Creates a new node in the Network's Graph Database.
    */
   struct graph_node_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  account;                ///< Name of the account that created the node.

      vector< graph_node_name_type >     node_types;             ///< Types of node being created, determines the required attributes.

      string                             node_id;                ///< uuidv4 identifying the node. Unique for each account.

      string                             name;                   ///< Name of the node.

      string                             details;                ///< Describes the additional details of the node.

      vector< fixed_string_32 >          attributes;             ///< The Attributes of the node.

      vector< fixed_string_32 >          attribute_values;       ///< The Attribute values of the node.

      string                             json;                   ///< Public plaintext JSON node attribute information.

      string                             json_private;           ///< Private encrypted ciphertext JSON node attribute information.

      string                             node_public_key;        ///< Key used for encrypting and decrypting private node JSON data.

      account_name_type                  interface;              ///< Name of the application that facilitated the creation of the node.

      void                               validate()const;
      void                               get_required_posting_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Creates a new edge in the Network's Graph Database.
    */
   struct graph_edge_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  account;              ///< Name of the account that created the edge.

      vector< graph_edge_name_type >     edge_types;           ///< Types of the edge being created.

      string                             edge_id;              ///< uuidv4 identifying the edge.

      account_name_type                  from_node_account;    ///< The account that is the creator of the Base connecting node.

      string                             from_node_id;         ///< The uuidv4 of the base connecting node.

      account_name_type                  to_node_account;      ///< The account that is the creator of the Node being connected to.

      string                             to_node_id;           ///< The uuidv4 of the Node being connected to.

      string                             name;                 ///< Name of the edge.

      string                             details;              ///< Describes the edge.

      vector< fixed_string_32 >          attributes;           ///< The Attributes of the edge.

      vector< fixed_string_32 >          attribute_values;     ///< The Attribute values of the edge.

      string                             json;                 ///< Public plaintext JSON edge attribute information.

      string                             json_private;         ///< Private encrypted ciphertext JSON edge attribute information.

      string                             edge_public_key;      ///< Key used for encrypting and decrypting private edge JSON data.

      account_name_type                  interface;            ///< Name of the application that facilitated the creation of the edge.

      void                               validate()const;
      void                               get_required_posting_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Creates a new type of node for instantiation in the Network's Graph Database.
    */
   struct graph_node_property_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  account;                     ///< Name of the account that created the node type.

      graph_node_name_type               node_type;                   ///< Name of the type of node being specified.

      string                             graph_privacy;               ///< Encryption level of the node attribute data.

      string                             edge_permission;             ///< The Level of connection required to create an edge to or from this node type. 

      string                             details;                     ///< Describes the additional details of the node.

      string                             url;                         ///< Reference URL link for more details.

      string                             json;                        ///< JSON Metadata for the node type.

      vector< fixed_string_32 >          attributes;                  ///< List of attributes that each node is required to have.

      account_name_type                  interface;                   ///< Name of the application that facilitated the creation of the node type.

      void                               validate()const;
      void                               get_required_posting_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Creates a new type of edge for instantiation in the Network's Graph Database.
    */
   struct graph_edge_property_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  account;                     ///< Name of the account that created the edge type.

      graph_edge_name_type               edge_type;                   ///< Name of the type of edge being specified.

      string                             graph_privacy;               ///< Encryption level of the edge attribute data.

      vector< graph_node_name_type >     from_node_types;             ///< Types of node that the edge can connect from. Empty for all types. 

      vector< graph_node_name_type >     to_node_types;               ///< Types of node that the edge can connect to. Empty for all types.

      string                             details;                     ///< Describes the additional details of the node.

      string                             url;                         ///< Reference URL link for more details.

      string                             json;                        ///< JSON Metadata for the edge type.

      vector< fixed_string_32 >          attributes;                  ///< List of attributes that each edge is required to have.

      account_name_type                  interface;                   ///< Name of the application that facilitated the creation of the edge type.

      void                               validate()const;
      void                               get_required_posting_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };



   //=============================//
   //==== Transfer Operations ====//
   //=============================//


   /**
    * Transfers funds from one account to another.
    * 
    * Includes a memo for reference. When the memo contains 
    * an author/permlink that refers to a specific comment, 
    * the transfer is interpretted as a comment payment, 
    * and enables the account to comment on the post 
    * and registers as a direct tip for the creator.
    */
   struct transfer_operation : public base_operation
   {
      account_name_type           signatory;

      account_name_type           from;       ///< Sending account to transfer asset from.
      
      account_name_type           to;         ///< Recieving account to transfer asset to.
      
      asset                       amount;     ///< The funds being transferred.

      string                      memo;       ///< The memo for the transaction, encryption on the memo is advised. 

      void                        validate()const;
      void                        get_required_active_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      void                        get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   /**
    * Requests a Transfer from an account to another account.
    */
   struct transfer_request_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            to;                 ///< Account requesting the transfer.
      
      account_name_type            from;               ///< Account that is being requested to accept the transfer.
      
      asset                        amount;             ///< The funds being transferred.

      string                       memo;               ///< The memo for the transaction, encryption on the memo is advised. 

      string                       request_id;         ///< uuidv4 of the request transaction.

      time_point                   expiration;         ///< time that the request expires. 

      bool                         requested = true;   ///< True to send the request, false to cancel an existing request. 

      void                         validate()const;
      void                         get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                         get_creator_name( flat_set<account_name_type>& a )const{ a.insert( to ); }
   };


   /**
    * Accepts a transfer request from an account to another account.
    */
   struct transfer_accept_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            from;               ///< Account that is accepting the transfer.

      account_name_type            to;                 ///< Account requesting the transfer.
      
      string                       request_id;         ///< uuidv4 of the request transaction.

      bool                         accepted = true;    ///< True to accept the request, false to reject. 

      void                         validate()const;
      void                         get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                         get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   /**
    * Transfers an asset periodically from one account to another.
    */
   struct transfer_recurring_operation : public base_operation
   {
      account_name_type           signatory;

      account_name_type           from;                   ///< Sending account to transfer asset from.
      
      account_name_type           to;                     ///< Recieving account to transfer asset to.
      
      asset                       amount;                 ///< The amount of asset to transfer for each payment interval.

      string                      transfer_id;            ///< uuidv4 of the transfer for reference. 

      time_point                  begin;                  ///< Starting time of the first payment.

      uint32_t                    payments;               ///< Number of payments to process in total.

      fc::microseconds            interval;               ///< Microseconds between each transfer event.

      string                      memo;                   ///< The memo for the transaction, encryption on the memo is advised.

      bool                        extensible = false;     ///< True if the payment duration should be extended in the event a payment is missed.

      bool                        fill_or_kill = false;   ///< True if the payment should be cancelled if a payment is missed.

      bool                        active = true;          ///< true if recurring payment is active, false to cancel.

      void                        validate()const;
      void                        get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                        get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   /**
    * Requests a periodic transfer from an account to another account.
    */
   struct transfer_recurring_request_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            to;                     ///< Account requesting the transfer.
      
      account_name_type            from;                   ///< Account that is being requested to accept the transfer.
      
      asset                        amount;                 ///< The amount of asset to transfer.

      string                       request_id;             ///< uuidv4 of the request transaction, becomes transfer id when accepted.

      time_point                   begin;                  ///< Starting time of the first payment.

      uint32_t                     payments;               ///< Number of payments to process in total.

      fc::microseconds             interval;               ///< Microseconds between each transfer event.

      string                       memo;                   ///< The memo for the transaction, encryption on the memo is advised.

      time_point                   expiration;             ///< Time that the request expires.

      bool                         extensible = false;     ///< True if the payment duration should be extended in the event a payment is missed.

      bool                         fill_or_kill = false;   ///< True if the payment should be cancelled if a payment is missed.

      bool                         requested = true;       ///< True to send the request, false to cancel an existing request. 

      void                         validate()const;
      void                         get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                         get_creator_name( flat_set<account_name_type>& a )const{ a.insert( to ); }
   };


   /**
    * Accepts a periodic transfer request from an account to another account.
    */
   struct transfer_recurring_accept_operation : public base_operation
   {
      account_name_type            signatory;

      account_name_type            from;               ///< Account that is accepting the recurring transfer.

      account_name_type            to;                 ///< Account requesting the recurring transfer.
      
      string                       request_id;         ///< uuidv4 of the request transaction.

      bool                         accepted = true;    ///< True to accept the request, false to reject. 

      void                         validate()const;
      void                         get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                         get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   /** 
    * Provided to maintain the invariant that all authority
    * required by an operation is explicit in the operation.
    */
   struct confidential_input
   {
      fc::ecc::commitment_type       commitment;

      authority                      owner;
   };

   /**
    * When sending a stealth tranfer we assume users are unable to scan
    * the full blockchain; therefore, payments require confirmation data
    * to be passed out of band. We assume this out-of-band channel is
    * not secure and therefore the contents of the confirmation must be encrypted. 
    */
   struct stealth_confirmation
   {
      struct memo_data
      {
         optional< public_key_type >      from;

         asset                            amount;

         fc::sha256                       blinding_factor;

         fc::ecc::commitment_type         commitment;

         uint32_t                         check = 0;
      };

      /**
       * Packs *this then encodes as base58 encoded string.
       */
      operator string()const;
      /**
       * Unpacks from a base58 string
       */
      stealth_confirmation( const std::string& base58 );
      stealth_confirmation(){}

      public_key_type                    one_time_key;

      optional< public_key_type >        to;

      vector< char >                     encrypted_memo;
   };

   /**
    * Defines data required to create a new blind commitment.
    * The blinded output that must be proven to be greater than 0.
    */
   struct confidential_output
   {
      fc::ecc::commitment_type                commitment;

      range_proof_type                        range_proof;       ///< Only required if there is more than one blind output.

      authority                               owner;

      optional< stealth_confirmation >        stealth_memo;
   };


   /**
    * Transfers funds from a confidential balance owner to another owner.
    * 
    * Every account has three balances:
    *
    * 1. Public Balance - everyone can see the balance changes and the parties involved
    * 2. Blinded Balance - everyone can see who is transacting but not the amounts involved
    * 3. Stealth Balance - both the amounts and parties involved are obscured
    *
    * Account owners may set a flag that allows their account to receive(or not) transfers of these kinds
    * Asset issuers can enable or disable the use of each of these types of accounts.
    * All assets in a confidential transfer must be of the same type.
    *
    * Using the "temp account" which has no permissions required, users can transfer a
    * stealth balance to the temp account and then use the temp account to register a new
    * account. In this way users can use stealth funds to create anonymous accounts with which
    * they can perform other actions that are not compatible with blinded balances (such as market orders)
    *
    * There are two ways to transfer value while maintaining privacy:
    * 
    * 1. Account to account with amount kept secret.
    * 2. Stealth transfers with amount sender/receiver kept secret.
    *
    * When doing account to account transfers, everyone with access to the
    * memo key can see the amounts, but they will not have access to the funds.
    * When using stealth transfers the same key is used for control and reading
    * the memo.
    *
    * This operation is more expensive than a normal transfer and has
    * a fee proportional to the size of the operation.
    *
    * Using this operation you can transfer from an account and/or blinded balances
    * to an account and/or blinded balances.
    *
    * Stealth Transfers:
    *
    * Assuming Receiver has key pair R,r and has shared public key R with Sender
    * Assuming Sender has key pair S,s
    * Generate one time key pair  O,o  as s.child(nonce) where nonce can be inferred from transaction
    * Calculate secret V = o*R
    * blinding_factor = sha256(V)
    * memo is encrypted via aes of V
    * owner = R.child(sha256(blinding_factor))
    *
    * Sender gives Receiver output ID to complete the payment.
    *
    * This process can also be used to send money to a cold wallet 
    * without having to pre-register any accounts.
    *
    * Outputs are assigned the same IDs as the inputs until no more input IDs are available,
    * in which case a the return value will be the *first* ID allocated for an output. Additional
    * output IDs are allocated sequentially thereafter.
    * If there are fewer outputs than inputs
    * then the input IDs are freed and never used again.
    */
   struct transfer_confidential_operation : public base_operation
   {
      vector< confidential_input >         inputs;       ///< Inputs to the confidential transfer from confidential balances.

      vector< confidential_output >        outputs;      ///< Outputs of the confidential transfer to new confidential balances.

      asset                                fee;          ///< Fee paid for the transfer.
      
      void                                 validate()const;
      void                                 get_required_authorities( vector<authority>& a )const { for( const auto& in : inputs ){ a.push_back( in.owner ); } }
      void                                 get_creator_name( flat_set<account_name_type>& a )const { a.insert( NULL_ACCOUNT ); }
   };

   
   /**
    * Converts public account balance to a confidential balance.
    */
   struct transfer_to_confidential_operation : public base_operation
   {
      account_name_type                  signatory;      

      account_name_type                  from;                ///< Account to transfer funds from and create new confidential balances. 

      asset                              amount;              ///< Amount of funds to transfer. 

      blind_factor_type                  blinding_factor;     ///< Factor to blind the output values. 

      vector< confidential_output >      outputs;             ///< Confidential balance outputs. 

      asset                              fee;                 ///< Fee amount paid for the transfer. 

      void                               validate()const;
      void                               get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   /**
    * Converts confidential balances to a public account balance.
    */
   struct transfer_from_confidential_operation : public base_operation
   {
      account_name_type                 to;                   ///< Account to transfer the balances to.

      asset                             amount;               ///< Amount of funds to transfer from confidential balances.

      blind_factor_type                 blinding_factor;      ///< Factor to blind the input values.

      vector< confidential_input >      inputs;               ///< Confidential balance inputs.

      asset                             fee;                  ///< Fee amount paid for the transfer.

      void                              validate()const;
      void                              get_required_authorities( vector<authority>& a )const { for( const auto& in : inputs ){ a.push_back( in.owner ); } }
      void                              get_creator_name( flat_set<account_name_type>& a )const { a.insert( NULL_ACCOUNT ); }
   };



   //============================//
   //==== Balance Operations ====//
   //============================//



   /**
    * Claims an account's reward balance into it's liquid balance from newly issued assets.
    */
   struct claim_reward_balance_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        account;      ///< Account claiming its reward balance from the network.

      asset                    reward;       ///< Amount of Reward balance to claim.

      void                     get_required_posting_authorities( flat_set< account_name_type >& a )const{ a.insert( signatory ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                     validate() const;
   };


   /**
    * Stakes a liquid balance of an account into it's staked balance.
    */
   struct stake_asset_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        from;         ///< Account staking the asset.

      account_name_type        to;           ///< Account to stake the asset to, Same as from if null.

      asset                    amount;       ///< Amount of Funds to transfer to staked balance from liquid balance.

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   /**
    * Divests an amount of the staked balance of an account to it's liquid balance.
    */
   struct unstake_asset_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        from;         ///< Account unstaking the asset.

      account_name_type        to;           ///< Account to unstake the asset to, Same as from if null.

      asset                    amount;       ///< Amount of Funds to transfer from staked balance to liquid balance.

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   /**
    * Allows an account to setup an unstake asset withdraw.
    * 
    * The funds unstaked will be transferred directly to another account's
    * balance rather than the withdrawing account. In addition, those funds
    * can be immediately staked again in the new account's balance.
    */
   struct unstake_asset_route_operation : public base_operation
   {
      account_name_type        signatory;              ///< The account the assets are withdrawn from.

      account_name_type        from;                   ///< The account the assets are withdrawn from.

      account_name_type        to;                     ///< The account receiving either assets or new stake.

      uint16_t                 percent = 0;            ///< The percent of the withdraw to go to the 'to' account.

      bool                     auto_stake = false;     ///< True if the stake should automatically be staked on the to account.

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const { a.insert( signatory ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   /**
    * Transfer liquid funds balance into savings for security.
    * 
    * Savings have a three day delay period for withdrawals, 
    * which places funds in a more protected state in the event that
    * an account is compromised. 
    */
   struct transfer_to_savings_operation : public base_operation 
   {
      account_name_type         signatory;

      account_name_type         from;       ///< The account the assets are transferred from.

      account_name_type         to;         ///< The account that is recieving the savings balance, same as from if null.

      asset                     amount;     ///< Funds to be transferred from liquid to savings balance.

      string                    memo;       ///< The memo for the transaction, encryption on the memo is advised.

      void                      get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                      get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
      void                      validate() const;
   };


   /**
    * Withdraws a specified balance from savings after a time duration.
    */
   struct transfer_from_savings_operation : public base_operation 
   {
      account_name_type        signatory;

      account_name_type        from;                 ///< Account to transfer savings balance from.

      account_name_type        to;                   ///< Account to receive the savings withdrawal.

      asset                    amount;               ///< Amount of asset to transfer from savings.

      string                   request_id;           ///< uuidv4 referring to the transfer.

      string                   memo;                 ///< The memo for the transaction, encryption on the memo is advised.

      bool                     transferred = true;   ///< True if the transfer is accepted, false to cancel transfer.

      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
      void                     validate() const;
   };


   /**
    * Delegate a staked asset balance from one account to the other.
    * 
    * The staked amount is still owned by the original account, 
    * but content voting rights and bandwidth allocation are transferred
    * to the receiving account.
    */
   struct delegate_asset_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        delegator;        ///< The account delegating the asset.

      account_name_type        delegatee;        ///< The account receiving the asset.

      asset                    amount;           ///< The amount of the asset delegated.

      void                     get_required_active_authorities( flat_set< account_name_type >& a ) const { a.insert( signatory ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( delegator ); }
      void                     validate() const;
   };



   //================================//
   //==== Marketplace Operations ====//
   //================================//



   /**
    * Creates or updates a product item for marketplace purchasing with escrow transfers.
    */
   struct product_sale_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  account;                ///< The Seller of the product.

      string                             product_id;             ///< uuidv4 referring to the product.

      string                             name;                   ///< The descriptive name of the product.

      string                             url;                    ///< Reference URL of the product or seller.

      string                             json;                   ///< JSON metadata attributes of the product.

      vector< fixed_string_32 >          product_variants;       ///< The collection of product variants. Each map must have a key for each variant.

      string                             product_details;        ///< The Description details of each variant of the product.

      string                             product_image;          ///< IPFS references to images of each product variant.

      vector< asset >                    product_prices;         ///< The price for each variant of the product.

      flat_map< uint32_t, uint16_t >     wholesale_discount;     ///< Discount percentages that are applied when quantity is above a given size.

      vector< uint32_t >                 stock_available;        ///< The available stock of each variant of the product.

      vector< fixed_string_32 >          delivery_variants;      ///< The types of product delivery available to purchasers.

      string                             delivery_details;       ///< The details of product delivery variants.

      vector< asset >                    delivery_prices;        ///< The price for each variant of delivery.

      bool                               active = true;          ///< True when the product is active and able to be sold, false when discontinued.

      void                               get_required_active_authorities( flat_set< account_name_type >& a ) const { a.insert( signatory ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                               validate() const;
   };


   /**
    * Requests a purchase of a specifed quantity of a product.
    * 
    * Creates a new escrow object for the purchase, 
    * which must be approved by the seller within acceptance_time.
    */
   struct product_purchase_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              buyer;               ///< The Buyer of the product.

      string                         order_id;            ///< uuidv4 referring to the purchase order.

      account_name_type              seller;              ///< The Seller of the product.

      string                         product_id;          ///< uuidv4 referring to the product.

      vector< fixed_string_32 >      order_variants;      ///< Variants of product ordered in the purchase.

      vector< uint32_t >             order_size;          ///< The number of each product variant ordered.

      string                         memo;                ///< The memo for the transaction, encryption on the memo is advised.

      string                         json;                ///< Additional JSON object attribute details.

      string                         shipping_address;    ///< The shipping address requested, encrypted with the secure key of the seller.

      fixed_string_32                delivery_variant;    ///< The type of product delivery selected.

      string                         delivery_details;    ///< The Description details of the delivery.

      time_point                     acceptance_time;     ///< Time that the escrow proposal must be approved before.

      time_point                     escrow_expiration;   ///< Time after which balance can be claimed by FROM or TO freely.

      void                           get_required_active_authorities( flat_set< account_name_type >& a ) const { a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( buyer ); }
      void                           validate() const;
   };


   /**
    * Creates or updates a product auction sale. 
    * 
    * Auction items are purchased by auction bidders, 
    * according to the auction type.
    * 
    * The marketplace generates an escrow transfer between 
    * the seller and the winning bidder
    * at the auction completion time.
    */
   struct product_auction_sale_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  account;                ///< The Seller of the auction product.

      string                             auction_id;             ///< uuidv4 referring to the auction product.

      string                             auction_type;           ///< The Auction price selection mechanism.

      string                             name;                   ///< The descriptive name of the product.

      string                             url;                    ///< Reference URL of the product or seller.

      string                             json;                   ///< JSON metadata attributes of the product.

      string                             product_details;        ///< The Description details of the product for auction.

      string                             product_image;          ///< IPFS references to images of the product for auction.

      asset                              reserve_bid;            ///< The min auction bid, or minimum price of a reverse auction at final bid time.

      asset                              maximum_bid;            ///< The max auction bid. Auction will immediately conclude if this price is bidded. Starting price of reverse auction.

      vector< fixed_string_32 >          delivery_variants;      ///< The types of product delivery available to purchasers.

      string                             delivery_details;       ///< The details of product delivery variants.

      vector< asset >                    delivery_prices;        ///< The price for each variant of delivery.

      time_point                         final_bid_time;         ///< No more bids will be accepted after this time. Concealed bids must be revealed before completion time.

      time_point                         completion_time;        ///< Time that the auction will select the winning bidder, or end if no bids.

      void                               get_required_active_authorities( flat_set< account_name_type >& a ) const { a.insert( signatory ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                               validate() const;
   };


   /**
    * Creates or updates a bid on a product auction sale.
    * 
    * - Open auctions enable the bidders to publicly 
    * declare the amount that they are willing to pay, 
    * and bump their bids until completion time.
    * 
    * - Reverse Auctions start the bidding at a maximum
    * price and decrease linearly until a bid is made.
    * 
    * - Concealed Auctions enable bidders to bid privately 
    * by commiting their bid amount,
    * and revealing it after the auction is closed.
    * Either the first or second highest revealed price
    * is used when the auction is completed.
    */
   struct product_auction_bid_operation : public base_operation
   {
      account_name_type              signatory;

      account_name_type              buyer;                  ///< The Buyer of the product.

      string                         bid_id;                 ///< uuidv4 referring to the auction bid.

      account_name_type              seller;                 ///< The Seller of the product.

      string                         auction_id;             ///< uuidv4 referring to the auction.

      commitment_type                bid_price_commitment;   ///< Concealed value of the bid price amount.

      blind_factor_type              blinding_factor;        ///< Factor to blind the bid price.

      share_type                     public_bid_amount;      ///< Set to 0 initially for concealed bid, revealed to match commitment. Revealed in initial bid if open.

      string                         memo;                   ///< The memo for the transaction, encryption on the memo is advised.

      string                         json;                   ///< Additional JSON object attribute details.

      string                         shipping_address;       ///< The shipping address requested, encrypted with the secure key of the seller.

      fixed_string_32                delivery_variant;       ///< The type of product delivery selected.

      string                         delivery_details;       ///< The Description details of the delivery.

      void                           get_required_active_authorities( flat_set< account_name_type >& a ) const { a.insert( signatory ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( buyer ); }
      void                           validate() const;
   };


   /**
    * Creates a proposed escrow transfer between two accounts.
    * 
    * The sender and recipient must both add mediators
    * when accepting the escrow, and all four accounts must then approve 
    * an escrow transaction for it to be valid.
    * When approving an Escrow, each account must deposit a bond 
    * of 10% of amount into the escrow as security against sybil attacks, 
    * which is forfeited in the event of 
    * a dispute ruling againt the individual.
    * 
    * Escrow transactions are uniquely identified by the FROM account and an ESCROW_ID uuidv4.
    * 
    * The purpose of this operation is to enable someone to send money contingently to
    * another account for a marketplace transaction.
    * The funds leave the FROM account when approved and go into a temporary balance
    * where they are held until FROM releases it to TO or TO refunds it to FROM.
    * 
    * If a product or service is not delivered, or a refund is requested but not released,
    * Disputes can be raised any time before or on the dispute deadline time, after the escrow
    * has been approved by all parties.
    * 
    * In the event of a dispute:
    * 
    * 1 - A set of 5 additional mediators are assigned to the escrow by the protocol to be
    * an impartial jury for the resolution of the dispute. 
    * 2 - Each additional mediator approves assignment, and deposits 10% of the amount in dispute.
    * 3 - All accounts that have approved the escrow must vote to release the funds
    * according to a specifed percentage.
    * 4 - After 7 days, the median release percentage is selected and enacted. 
    * 5 - Accounts that did not vote to release funds forfeit their security bond to the dispute pool.
    * 6 - Accounts forfeit a percentage of their security bond depending on the difference 
    * between their final voted release percentage and the median voted release percentage.
    * 7 - The dispute pool of all forfeited funds is divded equally between all accounts
    * that voted to release funds.
    */
   struct escrow_transfer_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        account;                 ///< Account creating the transaction to initate the escrow.

      account_name_type        from;                    ///< Account sending funds for a purchase.

      account_name_type        to;                      ///< Account receiving funds from a purchase.

      string                   escrow_id;               ///< uuidv4 referring to the escrow transaction.

      asset                    amount;                  ///< Amount of the asset to be transferred upon success.

      time_point               acceptance_time;         ///< Time that the escrow proposal must be approved before.

      time_point               escrow_expiration;       ///< Time after which balance can be claimed by FROM or TO freely.

      string                   memo;                    ///< The memo for the transaction, encryption on the memo is advised.

      string                   json;                    ///< Additional JSON object attribute details.

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Approves an escrow transfer, causing it to be locked in. 
    * 
    * Approving accounts must lock in 10% of the escrow amount as a security bond.
    * Once a party approves the escrow, the cannot revoke their approval.
    * 
    * In the event of a dispute, all assigned mediators must approve the escrow
    * to be able to vote on its resolution and release of funds.
    */
   struct escrow_approve_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        account;            ///< Account creating the transaction to approve the escrow.

      account_name_type        mediator;           ///< Nominated mediator to join the escrow for potential dispute resolution.

      account_name_type        escrow_from;        ///< The account sending funds into the escrow.

      string                   escrow_id;          ///< uuidv4 referring to the escrow being approved.

      bool                     approved = true;    ///< Set true to approve escrow, false to reject the escrow. All accounts must approve before activation.

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * If an escrow payment has an issue, they can raise it for dispute.
    * 
    * Once a payment is in dispute, 
    * a team of mediators are appointed and 
    * deposit a security bond before being able 
    * to vote on the resolution of the dispute. 
    */
   struct escrow_dispute_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        account;        ///< Account creating the transaction to dispute the escrow and raise it for resolution

      account_name_type        escrow_from;    ///< The account sending funds into the escrow.

      string                   escrow_id;      ///< uuidv4 referring to the escrow being disputed.

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Releases the funds contained within an escrow transfer.
    *
    * The permission scheme is as follows:
    * 
    * If there is no dispute and escrow has not expired:
    *  
    * - Either party can release funds to the other.
    * - All security bonds are refunded and released.
    * 
    * If escrow expires and there is no dispute:
    * 
    * - Either party can release funds to either party.
    * - All security bonds are refunded and released.
    * 
    * If there is a dispute (before expiration):
    * 
    * - All approving accounts can broadcast release percentage votes.
    * - Funds are distributed 7 days after the dispute is raised.
    * 
    * The escrow will use the median release vote to 
    * determine the amount to be transmitted to the TO account
    * and the remaining perentage will be transmitted to the FROM account.
    * The difference between the median vote 
    * and each approving account's release vote
    * is deducted as a percentage from each account's security bond 
    * and equally distributed to all other accounts that voted to release funds.
    * 
    * 100% of the security bond is foreited from accounts 
    * that fail to vote within 7 days after approving the escrow. 
    */
   struct escrow_release_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        account;           ///< The account creating the operation to release the funds.

      account_name_type        escrow_from;       ///< The escrow FROM account.

      string                   escrow_id;         ///< uuidv4 referring to the escrow.

      uint16_t                 release_percent;   ///< Percentage of escrow to release to the TO Account / remaining will be refunded to FROM account

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };



   //============================//
   //==== Trading Operations ====//
   //============================//



   /**
    * Creates a new limit order for exchanging assets at a specifed price.
    * 
    * Fills other orders when the price is better than
    * the current best opposing order, or enters the book as a new order if the price
    * is less than required to fill an existing order.
    */
   struct limit_order_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      owner;                 ///< Account that owns the asset being sold.

      string                 order_id;              ///< uuidv4 of the order for reference.

      asset                  amount_to_sell;        ///< Asset being sold on exchange.

      price                  exchange_rate;         ///< Minimum price to sell asset. Amount to sell is base.

      account_name_type      interface;             ///< Account of the interface that broadcasted the operation.

      time_point             expiration;            ///< Time that the order expires.

      bool                   opened = true;         ///< True to open new order, false to cancel existing order. 

      bool                   fill_or_kill = false;  ///< True if the order should be removed if it does not immediately fill on the orderbook.

      void                   validate()const;
      void                   get_creator_name( flat_set<account_name_type>& a )const{ a.insert( owner ); }
      void                   get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Creates a new margin order for trading assets.
    * 
    * Margin order first borrows a specified asset,
    * using a given collateral asset and sells it for a position asset amount.
    * The margin order pays interest on the 
    * loan until it is closed, at which time the position is liquidated
    * and the loan is repaid with interest.
    */
   struct margin_order_operation : public base_operation
   {
      account_name_type      signatory;

      account_name_type      owner;                      ///< Account that is the owner of the new margin position.

      string                 order_id;                   ///< uuidv4 of the order for reference.

      price                  exchange_rate;              ///< The asset pair price to sell the borrowed amount at on the exchange. Amount to borrow is base, Position is quote.

      asset                  collateral;                 ///< Collateral asset used to back the loan value. Returned to credit collateral object when position is closed. 

      asset                  amount_to_borrow;           ///< Amount of asset borrowed to purchase the position asset. Repaid when the margin order is closed.       

      optional< price >      stop_loss_price;            ///< Price at which the position will be closed if it falls into a net loss.

      optional< price >      take_profit_price;          ///< Price at which the order will be closed if it rises into a net profit.

      optional< price >      limit_stop_loss_price;      ///< Price at which the position will be closed if it falls into a net loss.

      optional< price >      limit_take_profit_price;    ///< Price at which the order will be closed if it rises into a net profit.

      account_name_type      interface;                  ///< Name of the interface that broadcasted the transaction.

      time_point             expiration;                 ///< Time that the order expires.

      bool                   opened = true;              ///< Set true to open the order, false to close existing order.

      bool                   fill_or_kill = false;       ///< Set true to cancel the order if it does not fill against the orderbook immediately.

      bool                   force_close = false;        ///< Set true when closing to force liquidate the order against the liquidity pool at available price.

      void                   validate()const;
      void                   get_creator_name( flat_set<account_name_type>& a )const{ a.insert( owner ); }
      void                   get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Creates a new auction order that sells at the daily auction clearing price.
    * 
    * Each day, the network selects an auction price for clearing the trades to maximise 
    * the amount of exchanged volume. 
    * Auction orders only match against other auction orders once per day, at a single
    * auction clearing price. 
    */
   struct auction_order_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        owner;                    ///< Owner of the Auction order.

      string                   order_id;                 ///< uuidv4 of the order for reference.

      asset                    amount_to_sell;           ///< Amount of asset to sell at auction clearing price.

      price                    limit_close_price;        ///< The asset pair price to sell the amount at the auction clearing price. Amount to sell is base.

      account_name_type        interface;                ///< Name of the interface that created the transaction.

      time_point               expiration;               ///< Time that the order expires.

      bool                     opened = true;            ///< True to open new order, false to cancel existing order.

      void                     validate()const;
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( owner ); }
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Creates a new collateralized debt position in a market issued asset.
    * 
    * Market issued assets are used to create a digital representation of
    * an externally priced asset, using a regularly published price feed.
    */
   struct call_order_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        owner;                      ///< Owner of the debt position and collateral.

      asset                    collateral;                 ///< Amount of collateral to add to the margin position.

      asset                    debt;                       ///< Amount of the debt to be issued.

      optional< uint16_t >     target_collateral_ratio;    ///< Maximum CR to maintain when selling collateral on margin call.

      account_name_type        interface;                  ///< Name of the interface that created the transaction.

      void                     validate()const;
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( owner ); }
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Creates a new option order that issues new option assets that can be sold for premium.
    * /
    * Option orders create covered option contract assets.
    * Option contract assets cover 100 units of the underlying asset, 
    * and allow the holder to exercise the option to execute a trade 
    * at a specified strike price and expire at a specified time period.
    * 
    * If exercised, the options orders that were first issued are 
    * assigned to purchase the exchanged asset for the covered asset.
    * Balances of options are cleared upon expiry, and option orders that
    * are not exercised are closed, with funds returned to the owner.
    * 
    * The asset in the price pair with the lower ID is considered
    * the base asset, therefore the unit of account.
    * If MEUSD is one asset in the pair, MEUSD always the base asset. 
    * Options with a strike price to buy the base asset are put options,
    * and options to buy the quote asset are call options.
    * 
    * The Option Chain sheet creates 21 option strike price positions for each month.
    * The strike prices are determined by rounding the price of the 
    * quote asset in the base asset to the nearest significant figure, 
    * and places strike prices in 5% intervals above and below the the rounded mid price.
    * 
    * Example: price of WYM is currently 3.257 MEUSD
    * Rounds down to 3.00 MEUSD.
    * 
    * Prices up the book would be:
    * 3.00, 3.15, 3.30, 3.45, 3.60, 3.75, 3.90, 4.05, 4.2, 4.35, 4.50.
    * 
    * Prices down the book would be:
    * 3.00, 2.85, 2.70, 2.55, 2.40, 2.25, 2.10, 1.95, 1.80, 1.65, 1.50.
    * 
    * Options are available for 12 months ahead of the current month, 
    * and are newly opened at each expiry.
    * 
    * Option assets are named as:
    * OPT.TYPE.QUOTE_SYMBOL.STRIKE_PRICE.BASE_SYMBOL.EXP_DAY_EXP_MONTH.EXP_YEAR
    * 
    * Example:
    * OPT.CALL.WYM.3.5.MEUSD.1.1.2021 == WYM call option at a strike price of 3.50 MEUSD expiring on 1st January 2021.
    * OPT.PUT.WYM.2.0.MEUSD.1.2.2021 == WYM put option at a strike price of 2.00 MEUSD expiring on 1st February 2021.
    */
   struct option_order_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        owner;                 ///< Owner of the Option order.

      string                   order_id;              ///< uuidv4 of the order for reference.

      asset                    options_issued;        ///< Amount of assets to issue covered options contract assets against. Must be a whole number.

      account_name_type        interface;             ///< Name of the interface that created the transaction.

      bool                     opened = true;         ///< True to open new order, false to close existing order by repaying options assets.

      void                     validate()const;
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( owner ); }
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };



   //=========================//
   //==== Pool Operations ====//
   //=========================//



   /**
    * Creates a new liquidity pool between two assets.
    * 
    * Liquidity pools can be used for autonomous market making between 
    * two assets, and providing continously liquidity to an asset's trading pair.
    * Limit and margin orders will consume the best priced liquidity from either other orders,
    * or the pairs liquidity pools. 
    * Accounts that add liquidity to the pool earn a share fo the fees it earns from trading.
    * 
    * Design Inspired by Bancor Protocol:
    * https://storage.googleapis.com/website-bancor/2018/04/01ba8253-bancor_protocol_whitepaper_en.pdf
    */
   struct liquidity_pool_create_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;            ///< Creator of the new liquidity pool.

      asset                 first_amount;       ///< Initial balance of one asset.

      asset                 second_amount;      ///< Initial balance of second asset, initial price is the ratio of these two amounts.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Exchanges an asset directly from liquidity pools.
    * 
    * The asset is traded with the core asset's liquidity pool, and then
    * the proceeds are trading with the receive asset's liquidity pool for the 
    * best liquidity.
    * 
    * Design Inspired by Bancor Protocol:
    * https://storage.googleapis.com/website-bancor/2018/04/01ba8253-bancor_protocol_whitepaper_en.pdf
    */
   struct liquidity_pool_exchange_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;            ///< Account executing the exchange with the pool.

      asset                 amount;             ///< Amount of asset to be exchanged.

      asset_symbol_type     receive_asset;      ///< The asset to receive from the liquidity pool.

      account_name_type     interface;          ///< Name of the interface account broadcasting the transaction.

      optional< price >     limit_price;        ///< The price of acquistion at which to cap the exchange to.

      bool                  acquire = false;    ///< Set true to acquire the specified amount, false to exchange in.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Adds capital to a liquidity pool.
    * 
    * Earns a share of the fees generated by the pool
    * Returns a liquidity pool asset which can be redeemed for 
    * the assets contained within the pool, and is 
    * equivalent to an autonomously re-balanced holding of 50% of both assets.
    * 
    * Design Inspired by Bancor Protocol:
    * https://storage.googleapis.com/website-bancor/2018/04/01ba8253-bancor_protocol_whitepaper_en.pdf
    */
   struct liquidity_pool_fund_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;            ///< Account funding the liquidity pool to receive the liquidity pool asset.

      asset                 amount;             ///< Amount of an asset to contribute to the liquidity pool.

      asset_symbol_type     pair_asset;         ///< Pair asset to the liquidity pool to receive liquidity pool assets of. 

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Removes capital from a liquidity pool.
    * 
    * Redeems a liquidity pool asset for an asset contained within the 
    * reserves of the pool.
    * 
    * Design Inspired by Bancor Protocol:
    * https://storage.googleapis.com/website-bancor/2018/04/01ba8253-bancor_protocol_whitepaper_en.pdf
    */
   struct liquidity_pool_withdraw_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;           ///< Account withdrawing liquidity pool assets from the pool.

      asset                 amount;            ///< Amount of the liquidity pool asset to redeem for underlying deposited assets. 

      asset_symbol_type     receive_asset;     ///< The asset to receive from the liquidity pool.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Adds an asset to an account's credit collateral position of that asset.
    * 
    * Collateral pools are used to borrow funds in 
    * credit borrow orders, margin orders, and option orders.
    * 
    * Design inspired by Compound Protocol:
    * https://compound.finance/documents/Compound.Whitepaper.pdf
    */
   struct credit_pool_collateral_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;         ///< Account locking an asset as collateral.

      asset                 amount;          ///< Amount of collateral balance to lock, 0 to unlock existing collateral.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Borrows an asset from the credit pool of the asset.
    * 
    * Creates a collateralized loan from the credit pool 
    * which is backed by the value fo the collateral, 
    * according to available balances within liquidity pools.
    * 
    * Borrowers pay a continous interest rate to the pool,
    * which is redeemed by lenders when they withdraw credit pool assets.
    * 
    * User can enable a Flash Loan to remove the collateralization
    * requirement, and borrow an unlimited amount of funds from the 
    * credit pool, by repaying the loan with one day's worth of interest
    * within the same transaction.
    * 
    * Design inspired by Compound Protocol:
    * https://compound.finance/documents/Compound.Whitepaper.pdf
    * 
    * Flash Loan Design inspired by Aave Protocol:
    * https://github.com/aave/aave-protocol/blob/master/docs/Aave_Protocol_Whitepaper_v1_0.pdf
    */
   struct credit_pool_borrow_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;              ///< Account borrowing funds from the pool, must have sufficient collateral.

      asset                 amount;               ///< Amount of an asset to borrow. Limit of 75% of collateral value. Set to 0 to repay loan.

      asset                 collateral;           ///< Amount of an asset to use as collateral for the loan. Set to 0 to reclaim collateral to collateral balance.

      string                loan_id;              ///< uuidv4 unique identifier for the loan.

      bool                  flash_loan = false;   ///< True to execute a no collateral loan within a single transaction. Must be repaid within same txn.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Lends an asset to a credit pool.
    * 
    * Returns units of the credit pool asset for the amount,
    * which can be redeemed at a later time for an amount of the 
    * lent asset, plus a share in the interest accumulated 
    * within the pool from borrowers.
    * 
    * Design inspired by Compound Protocol:
    * https://compound.finance/documents/Compound.Whitepaper.pdf
    */
   struct credit_pool_lend_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;         ///< Account lending an asset to the credit pool.

      asset                 amount;          ///< Amount of asset being lent.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Withdraws an asset from the specified credit pool.
    * 
    * Redeems a credit pool asset for its underlying credit reserves, 
    * plus additional interest earned from borrowers.
    * 
    * Design inspired by Compound Protocol:
    * https://compound.finance/documents/Compound.Whitepaper.pdf
    */
   struct credit_pool_withdraw_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;       ///< Account withdrawing its lent asset from the credit pool by redeeming credit-assets. 

      asset                 amount;        ///< Amount of interest bearing credit assets being redeemed for their underlying assets. 

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Creates a new Option pool between two assets.
    * 
    * Option pools are used to issue option assets at a range of
    * strike prices. Once per month, the assets expire and cycle over
    * to the next month. 
    * 
    * The option pool contains the chain sheet for each asset pair.
    */
   struct option_pool_create_operation : public base_operation
   {
      account_name_type     signatory;

      account_name_type     account;           ///< Creator of the new option pool.

      asset_symbol_type     first_asset;       ///< First asset in the option trading pair.

      asset_symbol_type     second_asset;      ///< Second asset in the option trading pair.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Creates a new prediction pool for a prediction market.
    * 
    * Prediction markets are used to create a series of 
    * assets corresponding to the outcomes of a public 
    * event. The value of each outcome asset trades at the
    * market's valuation of the likelyhood of each outcome
    * occuring. The outcome that occurs is exchangable for 
    * one unit of the underlying asset.
    * 
    * The Outcome selection is voted on by the holders of the 
    * prediction market asset, according to quadratic voting.
    * The bondholders that vote for the successfully selected
    * outcome receive a share in the total prediction bond pool, 
    * plus the fees earned by the prediction market pool.
    * 
    * Prediction pools are resolved over 7 days after their outcome time. 
    * Bondholders that fail to vote lose their prediction bond 
    * to the reoslvers of the successful outcome.
    * 
    * Design Inspired by Augur Protocol:
    * https://www.augur.net/whitepaper.pdf
    */
   struct prediction_pool_create_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  account;                  ///< Creator of the new prediction pool.

      asset_symbol_type                  prediction_symbol;        ///< Symbol of the prediction pool primary asset.

      asset_symbol_type                  collateral_symbol;        ///< Symbol of the collateral asset backing the prediction market.

      vector< asset_symbol_type >        outcome_assets;           ///< Symbol Suffixes for each outcome of the prediction market.

      string                             outcome_details;          ///< Description of each outcome and the resolution conditions for each asset.

      string                             display_symbol;           ///< Non-consensus display name for interface reference.

      string                             json;                     ///< JSON Metadata of the prediction market.

      string                             url;                      ///< Reference URL of the prediction market.

      string                             details;                  ///< Description of the market, how it will be resolved using known public data sources.

      time_point                         outcome_time;             ///< Time after which the outcome of the prediction market will become known and resolutions open.

      asset                              prediction_bond;          ///< Initial deposit placed by the issuer on the market, which requires them to vote in the resolution process.

      void                               validate()const;
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                               get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Adds or removes capital collateral funds from a prediction pool.
    * 
    * The collateral asset is exchanged for equal units of all 
    * prediction assets.
    * Earns a share of the fees generated by the pool
    * Returns a prediction pool base asset which earn a share in the 
    * fees generated by the market.
    * 
    * Design Inspired by Augur Protocol:
    * https://www.augur.net/whitepaper.pdf
    */
   struct prediction_pool_exchange_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  account;                    ///< Account executing the exchange with the pool.

      asset                              amount;                     ///< Amount of collateral asset to be exchanged.

      asset_symbol_type                  prediction_asset;           ///< Base Asset to the prediction pool to exchange with.

      bool                               exchange_base = false;      ///< True to exchange base asset, false to exchange one of all prediction assets.

      bool                               withdraw = false;           ///< True to Withdraw collateral, false to add funds. 

      void                               validate()const;
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                               get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Votes for a specified prediction market outcome resolution.
    * 
    * Prediction market assets are spent to vote for an outcome asset,
    * and the most supported outcome is selected by quadratic voting.
    * The Voters that support the winning option receive a share in the 
    * prediction bond pool, plus market fees. 
    * 
    * Design Inspired by Augur Protocol:
    * https://www.augur.net/whitepaper.pdf
    */
   struct prediction_pool_resolve_operation : public base_operation
   {
      account_name_type                  signatory;

      account_name_type                  account;                    ///< Account executing the exchange with the pool.

      asset                              amount;                     ///< Amount of prediction asset to vote with.

      asset_symbol_type                  resolution_outcome;         ///< Base Asset to the prediction pool to exchange with.

      void                               validate()const;
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                               get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   //==========================//
   //==== Asset Operations ====//
   //==========================//


   /**
    * Options available to all assets.
    */
   struct asset_options 
   {
      string                          display_symbol;                        ///< Non-consensus display name for interface reference.

      string                          details;                               ///< Data that describes the purpose of this asset.

      string                          json;                                  ///< Additional JSON metadata of this asset.

      string                          url;                                   ///< Reference URL for the asset. 
      
      share_type                      max_supply = MAX_ASSET_SUPPLY;         ///< The maximum supply of this asset which may exist at any given time. 

      uint8_t                         stake_intervals = 0;                   ///< Weeks required to stake the asset.

      uint8_t                         unstake_intervals = 4;                 ///< Weeks require to unstake the asset.

      uint16_t                        market_fee_percent = 0;                ///< Percentage of the total traded will be paid to the issuer of the asset.

      uint16_t                        market_fee_share_percent = 0;          ///< Percentage of the market fee that will be shared with the account's referrers.

      share_type                      max_market_fee = MAX_ASSET_SUPPLY;     ///< Market fee charged on a trade is capped to this value.
      
      uint32_t                        issuer_permissions = ASSET_ISSUER_PERMISSION_MASK; ///< The flags which the issuer has permission to update.

      uint32_t                        flags = 0;                             ///< The currently active flags on this permission.

      vector< account_name_type >     whitelist_authorities;                 ///< Accounts able to transfer this asset if the flag is set and whitelist is non-empty.

      vector< account_name_type >     blacklist_authorities;                 ///< Accounts which cannot transfer or receive this asset.

      vector< asset_symbol_type >     whitelist_markets;                     ///< The assets that this asset may be traded against in the market

      vector< asset_symbol_type >     blacklist_markets;                     ///< The assets that this asset may not be traded against in the market.

      // === Currency Asset Options === //

      asset                           block_reward = BLOCK_REWARD;                                    ///< The value of the initial reward paid into the reward fund every block.

      uint16_t                        block_reward_reduction_percent = 0;                             ///< The percentage by which the block reward is reduced each period. 0 for no reduction.

      uint16_t                        block_reward_reduction_days = 0;                                ///< Number of days between reduction events. 0 for no reduction.

      uint16_t                        content_reward_percent = CONTENT_REWARD_PERCENT;                ///< Percentage of reward paid to content creators.

      asset_symbol_type               equity_asset = SYMBOL_EQUITY;                                   ///< Asset that will receive equity rewards.

      uint16_t                        equity_reward_percent = EQUITY_REWARD_PERCENT;                  ///< Percentage of reward paid to staked equity asset holders.

      uint16_t                        producer_reward_percent = PRODUCER_REWARD_PERCENT;              ///< Percentage of reward paid to block producers.

      uint16_t                        supernode_reward_percent = SUPERNODE_REWARD_PERCENT;            ///< Percentage of reward paid to supernode operators.

      uint16_t                        power_reward_percent = POWER_REWARD_PERCENT;                    ///< Percentage of reward paid to staked currency asset holders.

      uint16_t                        community_fund_reward_percent = COMMUNITY_FUND_REWARD_PERCENT;  ///< Percentage of reward paid to community fund proposals.

      uint16_t                        development_reward_percent = DEVELOPMENT_REWARD_PERCENT;        ///< Percentage of reward paid to elected developers.

      uint16_t                        marketing_reward_percent = MARKETING_REWARD_PERCENT;            ///< Percentage of reward paid to elected marketers.

      uint16_t                        advocacy_reward_percent = ADVOCACY_REWARD_PERCENT;              ///< Percentage of reward paid to elected advocates.

      uint16_t                        activity_reward_percent = ACTIVITY_REWARD_PERCENT;              ///< Percentage of reward paid to active accounts each day.

      uint16_t                        producer_block_reward_percent = PRODUCER_BLOCK_PERCENT;         ///< Percentage of producer reward paid to the producer of each individual block.

      uint16_t                        validation_reward_percent = PRODUCER_VALIDATOR_PERCENT;         ///< Percentage of producer reward paid to validators of blocks.

      uint16_t                        txn_stake_reward_percent = PRODUCER_TXN_STAKE_PERCENT;          ///< Percentage of producer reward paid to producers according to transaction stake weight of blocks.

      uint16_t                        work_reward_percent = PRODUCER_WORK_PERCENT;                    ///< Percentage of producer reward paid to proof of work mining producers for each proof.

      uint16_t                        producer_activity_reward_percent = PRODUCER_ACTIVITY_PERCENT;   ///< Percentage of producer reward paid to the highest voted producer in activity rewards.

      // === Stablecoin Options === //

      fc::microseconds                feed_lifetime = PRICE_FEED_LIFETIME;                            ///< Time before a price feed expires.

      uint8_t                         minimum_feeds = 1;                                              ///< Minimum number of unexpired feeds required to extract a median feed from.

      fc::microseconds                asset_settlement_delay = ASSET_SETTLEMENT_DELAY;                ///< This is the delay between the time a long requests settlement and the chain evaluates the settlement.

      uint16_t                        asset_settlement_offset_percent = ASSET_SETTLEMENT_OFFSET;      ///< The percentage to adjust the feed price in the short's favor in the event of a forced settlement.

      uint16_t                        maximum_asset_settlement_volume = ASSET_SETTLEMENT_MAX_VOLUME;  ///< the percentage of current supply which may be force settled within each 24h interval.

      asset_symbol_type               backing_asset = SYMBOL_COIN;                                    ///< The symbol of the asset that the stablecoin is collateralized by.

      // === Equity Asset Options === //

      uint16_t                        dividend_share_percent = DIVIDEND_SHARE_PERCENT;        ///< Percentage of incoming assets added to the dividends pool.

      uint16_t                        liquid_dividend_percent = LIQUID_DIVIDEND_PERCENT;      ///< Percentage of equity dividends distributed to liquid balances.

      uint16_t                        staked_dividend_percent = STAKED_DIVIDEND_PERCENT;      ///< Percentage of equity dividends distributed to staked balances.

      uint16_t                        savings_dividend_percent = SAVINGS_DIVIDEND_PERCENT;    ///< Percentage of equity dividends distributed to savings balances.

      uint16_t                        liquid_voting_rights = PERCENT_100;                     ///< Amount of votes per asset conveyed to liquid holders of the asset.

      uint16_t                        staked_voting_rights = PERCENT_100;                     ///< Amount of votes per asset conveyed to staked holders of the asset.

      uint16_t                        savings_voting_rights = PERCENT_100;                    ///< Amount of votes per asset conveyed to savings holders of the asset.

      fc::microseconds                min_active_time = EQUITY_ACTIVITY_TIME;                 ///< Time that account must have a recent activity reward within to earn dividend.

      share_type                      min_balance = BLOCKCHAIN_PRECISION;                     ///< Minimum amount of equity required to earn dividends.

      uint16_t                        min_producers = EQUITY_MIN_PRODUCERS;                   ///< Minimum amount of producer votes required to earn dividends.

      share_type                      boost_balance = EQUITY_BOOST_BALANCE;                   ///< Amount of equity balance to earn double dividends.

      share_type                      boost_activity = EQUITY_BOOST_ACTIVITY;                 ///< Amount of recent activity rewards required to earn double dividends.

      uint16_t                        boost_producers = EQUITY_BOOST_PRODUCERS;               ///< Amount of producer votes required to earn double dividends.

      uint16_t                        boost_top = EQUITY_BOOST_TOP_PERCENT;                   ///< Percent bonus earned by Top membership accounts.

      // === Credit Asset options === //

      asset_symbol_type               buyback_asset = SYMBOL_USD;                                        ///< Asset used to repurchase the credit asset.

      price                           buyback_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ),asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );  ///< Price that credit asset is repurchased at to repay creditors.

      uint16_t                        buyback_share_percent = BUYBACK_SHARE_PERCENT;                     ///< Percentage of incoming assets added to the buyback pool.

      uint16_t                        liquid_fixed_interest_rate = LIQUID_FIXED_INTEREST_RATE;           ///< Fixed component of Interest rate of the asset for liquid balances.

      uint16_t                        liquid_variable_interest_rate = LIQUID_VARIABLE_INTEREST_RATE;     ///< Variable component of Interest rate of the asset for liquid balances.

      uint16_t                        staked_fixed_interest_rate = STAKED_FIXED_INTEREST_RATE;           ///< Fixed component of Interest rate of the asset for staked balances.

      uint16_t                        staked_variable_interest_rate = STAKED_VARIABLE_INTEREST_RATE;     ///< Variable component of Interest rate of the asset for staked balances.

      uint16_t                        savings_fixed_interest_rate = SAVINGS_FIXED_INTEREST_RATE;         ///< Fixed component of Interest rate of the asset for savings balances.

      uint16_t                        savings_variable_interest_rate = SAVINGS_VARIABLE_INTEREST_RATE;   ///< Variable component of Interest rate of the asset for savings balances.

      uint16_t                        var_interest_range = VAR_INTEREST_RANGE;                           ///< The percentage range from the buyback price over which to apply the variable interest rate.

      // === Unique Asset Options === //

      asset_symbol_type               ownership_asset = SYMBOL_EQUITY;                                   ///< Asset that represents controlling ownership of the unique asset. Same as symbol for no liquid ownership asset.

      vector< account_name_type >     control_list;                                                      ///< List of accounts that have control over access to the unique asset.

      vector< account_name_type >     access_list;                                                       ///< List of accounts that have access to the unique asset.

      asset                           access_price = asset( BLOCKCHAIN_PRECISION, SYMBOL_USD );          ///< Price per day for all accounts in the access list.

      // === Bond Asset Options === //

      asset                           value = asset( 100*BLOCKCHAIN_PRECISION, SYMBOL_USD );             ///< Face value amount of each unit of the bond. Interest is paid as a percentage of value.

      uint16_t                        collateralization = BOND_COLLATERALIZATION_PERCENT;                ///< Percentage of value that is locked in collateral to back the bonds. Should be at least 10%.

      uint16_t                        coupon_rate_percent = BOND_COUPON_RATE_PERCENT;                    ///< Percentage rate of the value that is paid each month in interest to the holders.

      date_type                       maturity_date = date_type( 1, 1, 1970 );                           ///< Date at which the bond will mature. Principle value will be automatically paid from business_account.

      // === Stimulus Asset Options === //

      asset_symbol_type               redemption_asset = SYMBOL_USD;                                     ///< Symbol of the asset that can be redeemed in exchange the stimulus asset.

      price                           redemption_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ),asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) ); ///< Price at which the stimulus asset is redeemed. Redemption asset is base.
      
      vector< account_name_type >     distribution_list;                                                 ///< List of accounts that receive an equal balance of the stimulus asset.

      vector< account_name_type >     redemption_list;                                                   ///< List of accounts that can receive and redeem the stimulus asset.

      asset                           distribution_amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_USD );   ///< Amount of stimulus asset distributed each interval.

      void validate()const;
   };


   /**
    * Creates a new asset object of the asset type provided.
    * 
    * Assets can be transferred between accounts
    * to represent ownership of anything of value.
    * All assets can be staked and saved, delegated and received.
    */
   struct asset_create_operation : public base_operation
   {
      account_name_type               signatory;

      account_name_type               issuer;                  ///< Name of the issuing account, can create units and administrate the asset.
      
      asset_symbol_type               symbol;                  ///< The ticker symbol of this asset.

      string                          asset_type;              ///< The type of the asset. Determines asset characteristics and features.

      asset                           coin_liquidity;          ///< Amount of COIN asset to inject into the Coin liquidity pool.  

      asset                           usd_liquidity;           ///< Amount of USD asset to inject into the USD liquidity pool.

      asset                           credit_liquidity;        ///< Amount of the new asset to issue and inject into the credit pool.

      asset_options                   options;                 ///< Series of options parameters that apply to each asset type.

      void                            validate()const;
      void                            get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                            get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Updates an Asset to use a new set of options.
    */
   struct asset_update_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             issuer;                   ///< Account that issued the asset.

      asset_symbol_type             asset_to_update;          ///< Asset symbol that is being updated.

      asset_options                 new_options;              ///< Options used for all asset types.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Issues an amount of an asset to a specified account.
    * 
    * The asset must be user issued, cannot be market issued.
    */
   struct asset_issue_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             issuer;               ///< The issuer of the asset.

      asset                         asset_to_issue;       ///< Amount of asset being issued to the account.

      account_name_type             issue_to_account;     ///< Account receiving the newly issued asset.

      string                        memo;                 ///< The memo for the transaction, encryption on the memo is advised.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Takes a specified amount of an asset out of circulation.
    * 
    * Returns the asset to the reserved, available supply for new issuance.
    */
   struct asset_reserve_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             payer;               ///< Account that is reserving the asset back to the unissued supply.

      asset                         amount_to_reserve;   ///< Amount of the asset being reserved.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( payer ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Updates the issuer account of an asset.
    * 
    * Transfers effective ownership of the asset to another account,
    * and enables the new account to issue additional supply, 
    * and change the asset's properties.
    */
   struct asset_update_issuer_operation : public base_operation
   {
      account_name_type             signatory;
      
      account_name_type             issuer;             ///< The current issuer of the asset.

      asset_symbol_type             asset_to_update;    ///< The asset being updated.

      account_name_type             new_issuer;         ///< Name of the account specified to become the new issuer of the asset.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Creates a new asset distribution.
    * 
    * Asset distributions enables an asset to be newly issued 
    * to accounts sending incoming payments according to
    * specified parameters.
    * 
    * Enables funds to be sent into the distribution
    * to create distribution balances.
    * 
    * Design Inspired by the Process flow of STEEM's Smart Media Tokens:
    * https://smt.steem.com/smt-whitepaper.pdf
    */
   struct asset_distribution_operation : public base_operation
   {
      account_name_type             signatory;
      
      account_name_type             issuer;                       ///< The account which created the asset.

      asset_symbol_type             distribution_asset;           ///< Asset that is generated by the distribution.

      asset_symbol_type             fund_asset;                   ///< Asset being accepted for distribution assets.

      string                        details;                      ///< Description of the distribution process.

      string                        url;                          ///< Reference URL of the distribution process.

      string                        json;                         ///< JSON Metadata of the distribution process.

      uint32_t                      distribution_rounds;          ///< Number of distribution rounds, total distribution amount is divided between all rounds.

      uint32_t                      distribution_interval_days;   ///< Duration of each distribution round, in days.

      uint32_t                      max_intervals_missed;         ///< Number of Rounds that can be missed before the distribution is closed early.

      share_type                    min_input_fund_units;         ///< Minimum funds required for each round of the distribution process.

      share_type                    max_input_fund_units;         ///< Maximum funds to be accepted before closing each distribution round.
      
      vector< asset_unit >          input_fund_unit;              ///< The integer unit ratio for distribution of incoming funds.

      vector< asset_unit >          output_distribution_unit;     ///< The integer unit ratio for distribution of released funds.

      share_type                    min_unit_ratio;               ///< The lowest value of unit ratio between input and output units.

      share_type                    max_unit_ratio;               ///< The highest possible initial value of unit ratio between input and output units. 

      share_type                    min_input_balance_units;      ///< Minimum fund units that each sender can contribute in an individual balance.

      share_type                    max_input_balance_units;      ///< Maximum fund units that each sender can contribute in an individual balance.
      
      time_point                    begin_time;                   ///< Time to begin the first distribution round.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Funds a new asset distribution balance.
    * 
    * Every distribution interval, all pending distribution balances 
    * are converted into the distribution asset.
    * 
    * Design Inspired by the Process flow of STEEM's Smart Media Tokens:
    * https://smt.steem.com/smt-whitepaper.pdf
    */
   struct asset_distribution_fund_operation : public base_operation
   {
      account_name_type             signatory;
      
      account_name_type             sender;                    ///< The account which sent the amount into the distribution.

      asset_symbol_type             distribution_asset;        ///< Distribution asset for the fund to be sent to.

      asset                         amount;                    ///< Asset amount being sent for distribution.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( sender ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Uses an option asset to obtain the underlying asset at the strike price.
    * 
    * Option assets enable the holder to exercise them
    * to trade the asset at the strike price.
    * 
    * The Exercised options match against the outstanding
    * option orders that were created first, which are assigned the 
    * exchanged quote funds. 
    */
   struct asset_option_exercise_operation : public base_operation
   {
      account_name_type             signatory;
      
      account_name_type             account;              ///< The account exercising the option asset.

      asset                         amount;               ///< Option assets being exercised by exchanging the quoted asset for the underlying. 

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Adds funds to the redemption pool of a stimulus asset.
    * 
    * Stimulus assets are redeemable for the underlying redemption pool 
    * and all balances are redistributed once per month. 
    */
   struct asset_stimulus_fund_operation : public base_operation
   {
      account_name_type             signatory;
      
      account_name_type             account;              ///< The account funding the stimulus asset.

      asset_symbol_type             stimulus_asset;       ///< Asset symbol of the asset to add stimulus funds to.

      asset                         amount;               ///< Redemption asset being injected into the redemption pool.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Update the set of feed-producing accounts for a BitAsset.
    *
    * BitAssets have price feeds selected by taking the 
    * median values of recommendations from a set of feed producers.
    * This operation is used to specify which
    * accounts may produce feeds for a given BitAsset.
    * 
    * All valid feeds supplied by feed producers 
    * in @ref new_feed_producers, which were already feed producers
    * prior to execution of this operation, will be preserved.
    * 
    * Design inspired by Bitshares Protocol:
    * https://www.bitshares.foundation/download/articles/BitSharesBlockchain.pdf
    */
   struct asset_update_feed_producers_operation : public base_operation
   {
      account_name_type               signatory;
      
      account_name_type               issuer;                  ///< The issuer of the BitAsset.

      asset_symbol_type               asset_to_update;         ///< The BitAsset being updated.

      flat_set< account_name_type >   new_feed_producers;      ///< Set of accounts that can determine the price feed of the asset.

      void                            validate()const;
      void                            get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                            get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Publish price feeds for BitAssets.
    *
    * Price feed providers use this operation to publish 
    * their price feeds for BitAssets. A price feed is
    * used to tune the market for a particular BitAssets. 
    * For each value in the feed, the median across all
    * committee_member feeds for that asset is calculated and the 
    * market for the asset is configured with the median of that value.
    *
    * The feed in the operation contains three elements:
    * 
    * - A call price limit
    * - A short price limit
    * - A settlement price
    * 
    * The call limit price is structured as (collateral asset) / (debt asset) 
    * and the short limit price is structured
    * as (asset for sale) / (collateral asset).
    * 
    * Note that the call price limit and settlement price are
    * directionally opposite to eachother, so if we're
    * publishing a feed for USD, the call limit price 
    * will be CORE/USD and the short limit price will be USD/CORE.
    * 
    * Design inspired by Bitshares Protocol:
    * https://www.bitshares.foundation/download/articles/BitSharesBlockchain.pdf
    */
   struct asset_publish_feed_operation : public base_operation
   {
      account_name_type             signatory;
      
      account_name_type             publisher;     ///< Account publishing the price feed.

      asset_symbol_type             symbol;        ///< Asset for which the feed is published.

      price_feed                    feed;          ///< Exchange rate between stablecoin and backing asset.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( publisher ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Schedules a BitAsset balance for automatic settlement.
    *
    * Holders of market-issued assests may request a 
    * forced settlement for some amount of their asset. 
    * This means that the specified sum will be locked 
    * by the chain and held for the settlement period, 
    * after which time the chain will
    * choose a margin position holder and buy the 
    * settled asset using the margin's collateral.
    * 
    * The price of this sale will be based on the feed 
    * price for the BitAsset being settled.
    * 
    * Design inspired by Bitshares Protocol:
    * https://www.bitshares.foundation/download/articles/BitSharesBlockchain.pdf
    */
   struct asset_settle_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             account;          ///< Account requesting the force settlement.
      
      asset                         amount;           ///< Amount of asset to force settle. Set to 0 to cancel order.

      account_name_type             interface;        ///< Account of the interface used to broadcast the operation.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Globally Settles a BitAsset, collecting all remaining collateral and debt and setting a global settlement price.
    *
    * In order to use this operation, asset_to_settle 
    * must have the global_settle flag set.
    * When this operation is executed all balances 
    * are converted into the backing asset at the
    * settle_price and all open margin positions 
    * are called at the settle price.
    * 
    * If this asset is used as backing for other stablecoins, 
    * those stablecoins will be force settled at their current
    * feed price.
    * 
    * Design inspired by Bitshares Protocol:
    * https://www.bitshares.foundation/download/articles/BitSharesBlockchain.pdf
    */
   struct asset_global_settle_operation : public base_operation
   {
      account_name_type             signatory;
      
      account_name_type             issuer;              ///< Issuer of the asset being settled. 

      asset_symbol_type             asset_to_settle;     ///< Symbol of the asset being settled. 

      price                         settle_price;        ///< Global settlement price, must be in asset / backing asset. 

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Used to create a bid for outstanding debt of a globally settled market issued asset.
    * 
    * When the sum of bids for an asset's debt exceed 
    * the total amount of outstanding debt
    * for the asset, the collateral in the global 
    * settlement fund is distributed to all accounts
    * that create bids that bring the asset 
    * into operation and solvency again.
    */
   struct asset_collateral_bid_operation : public base_operation
   {
      account_name_type        signatory;

      account_name_type        bidder;        ///< Adds additional collateral to the market issued asset.

      asset                    collateral;    ///< The amount of collateral to bid for the debt.

      asset                    debt;          ///< The amount of debt to take over.

      void                     validate()const;
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( bidder ); }
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };
   

   //=====================================//
   //==== Block Production Operations ====//
   //=====================================//


   struct chain_properties
   {
      asset                  account_creation_fee = MIN_ACCOUNT_CREATION_FEE;               ///< Minimum fee required to create a new account by staking.

      asset                  asset_coin_liquidity = MIN_ASSET_COIN_LIQUIDITY;               ///< Minimum COIN required to create a new asset.

      asset                  asset_usd_liquidity = MIN_ASSET_USD_LIQUIDITY;                 ///< Minimum USD required to create a new asset.

      uint64_t               maximum_block_size = MAX_BLOCK_SIZE;                           ///< The maximum block size of the network in bytes. No Upper bound on block size limit.

      fc::microseconds       pow_target_time = POW_TARGET_TIME;                             ///< The targeted time for each proof of work

      fc::microseconds       pow_decay_time = POW_DECAY_TIME;                               ///< Time over which proof of work output is averaged over

      fc::microseconds       txn_stake_decay_time = TXN_STAKE_DECAY_TIME;                   ///< Time over which transaction stake is averaged over

      uint16_t               escrow_bond_percent = ESCROW_BOND_PERCENT;                     ///< Percentage of an escrow transfer that is deposited for dispute resolution

      uint16_t               credit_interest_rate = CREDIT_INTEREST_RATE;                   ///< The credit interest rate paid to holders of network credit assets.

      uint16_t               credit_open_ratio = CREDIT_OPEN_RATIO;                         ///< The minimum required collateralization ratio for a credit loan to be opened. 

      uint16_t               credit_liquidation_ratio = CREDIT_LIQUIDATION_RATIO;           ///< The minimum permissible collateralization ratio before a loan is liquidated. 

      uint16_t               credit_min_interest = CREDIT_MIN_INTEREST;                     ///< The minimum component of credit pool interest rates. 

      uint16_t               credit_variable_interest = CREDIT_VARIABLE_INTEREST;           ///< The variable component of credit pool interest rates, applied at equal base and borrowed balances.

      uint16_t               market_max_credit_ratio = MARKET_MAX_CREDIT_RATIO;             ///< The maximum percentage of core asset liquidity balances that can be loaned.

      uint16_t               margin_open_ratio = MARGIN_OPEN_RATIO;                         ///< The minimum required collateralization ratio for a credit loan to be opened. 

      uint16_t               margin_liquidation_ratio = MARGIN_LIQUIDATION_RATIO;           ///< The minimum permissible collateralization ratio before a loan is liquidated. 

      uint16_t               maximum_asset_feed_publishers = MAX_ASSET_FEED_PUBLISHERS;     ///< The maximum number of accounts that can publish price feeds for a stablecoin.

      asset                  membership_base_price = MEMBERSHIP_FEE_BASE;                   ///< The price for standard membership per month.

      asset                  membership_mid_price = MEMBERSHIP_FEE_MID;                     ///< The price for Mezzanine membership per month.

      asset                  membership_top_price = MEMBERSHIP_FEE_TOP;                     ///< The price for top level membership per month.

      uint32_t               author_reward_percent = AUTHOR_REWARD_PERCENT;                 ///< The percentage of content rewards distributed to post authors.

      uint32_t               vote_reward_percent = VOTE_REWARD_PERCENT;                     ///< The percentage of content rewards distributed to post voters.

      uint32_t               view_reward_percent = VIEW_REWARD_PERCENT;                     ///< The percentage of content rewards distributed to post viewers.

      uint32_t               share_reward_percent = SHARE_REWARD_PERCENT;                   ///< The percentage of content rewards distributed to post sharers.

      uint32_t               comment_reward_percent = COMMENT_REWARD_PERCENT;               ///< The percentage of content rewards distributed to post commenters.

      uint32_t               storage_reward_percent = STORAGE_REWARD_PERCENT;               ///< The percentage of content rewards distributed to viewing supernodes.

      uint32_t               moderator_reward_percent = MODERATOR_REWARD_PERCENT;           ///< The percentage of content rewards distributed to community moderators.

      fc::microseconds       content_reward_decay_rate = CONTENT_REWARD_DECAY_RATE;         ///< The time over which content rewards are distributed

      fc::microseconds       content_reward_interval = CONTENT_REWARD_INTERVAL;             ///< Time taken per distribution of content rewards.

      uint32_t               vote_reserve_rate = VOTE_RESERVE_RATE;                         ///< The number of votes regenerated per day.

      uint32_t               view_reserve_rate = VIEW_RESERVE_RATE;                         ///< The number of views regenerated per day.

      uint32_t               share_reserve_rate = SHARE_RESERVE_RATE;                       ///< The number of shares regenerated per day.

      uint32_t               comment_reserve_rate = COMMENT_RESERVE_RATE;                   ///< The number of comments regenerated per day.

      fc::microseconds       vote_recharge_time = VOTE_RECHARGE_TIME;                       ///< Time taken to fully recharge voting power.

      fc::microseconds       view_recharge_time = VIEW_RECHARGE_TIME;                       ///< Time taken to fully recharge viewing power.

      fc::microseconds       share_recharge_time = SHARE_RECHARGE_TIME;                     ///< Time taken to fully recharge sharing power.

      fc::microseconds       comment_recharge_time = COMMENT_RECHARGE_TIME;                 ///< Time taken to fully recharge commenting power.

      fc::microseconds       curation_auction_decay_time = CURATION_AUCTION_DECAY_TIME;     ///< time of curation reward decay after a post is created. 

      double                 vote_curation_decay = VOTE_CURATION_DECAY;                     ///< Number of votes for the half life of voting curation reward decay.

      double                 view_curation_decay = VIEW_CURATION_DECAY;                     ///< Number of views for the half life of viewer curation reward decay.

      double                 share_curation_decay = SHARE_CURATION_DECAY;                   ///< Number of shares for the half life of sharing curation reward decay.

      double                 comment_curation_decay = COMMENT_CURATION_DECAY;               ///< Number of comments for the half life of comment curation reward decay.

      fc::microseconds       supernode_decay_time = SUPERNODE_DECAY_TIME;                   ///< Amount of time to average the supernode file weight over. 

      uint16_t               enterprise_vote_percent_required = ENTERPRISE_VOTE_THRESHOLD_PERCENT;     ///< Percentage of total voting power required to approve enterprise milestones. 

      uint64_t               maximum_asset_whitelist_authorities = MAX_ASSET_WHITELIST_AUTHORITIES;  ///< The maximum amount of whitelisted or blacklisted authorities for user issued assets 

      uint8_t                max_stake_intervals = MAX_ASSET_STAKE_INTERVALS;               ///< Maximum weeks that an asset can stake over.

      uint8_t                max_unstake_intervals = MAX_ASSET_UNSTAKE_INTERVALS;           ///< Maximum weeks that an asset can unstake over.

      asset                  max_exec_budget = MAX_EXEC_BUDGET;                             ///< Maximum budget that an executive board can claim.

      void validate()const
      {
         FC_ASSERT( account_creation_fee.symbol == SYMBOL_COIN,
            "Acccount creation fee must be in the core asset." );
         FC_ASSERT( account_creation_fee >= MIN_ACCOUNT_CREATION_FEE,
            "Account creation fee must be at least 1 Unit of core asset." );
         FC_ASSERT( asset_coin_liquidity.symbol == SYMBOL_COIN,
            "Asset COIN liquidity must be in the core asset." );
         FC_ASSERT( asset_coin_liquidity >= MIN_ASSET_COIN_LIQUIDITY,
            "Asset COIN liquidity must be at least 10 units of COIN." );
         FC_ASSERT( asset_usd_liquidity.symbol == SYMBOL_USD,
            "Asset USD liquidity must be in the USD asset." );
         FC_ASSERT( asset_usd_liquidity >= MIN_ASSET_USD_LIQUIDITY,
            "Asset USD liquidity must be at least 10 units of USD." );
         FC_ASSERT( maximum_block_size >= MIN_BLOCK_SIZE_LIMIT,
            "Maximum blocksize must be greater than minimum limit requirement." );
         FC_ASSERT( pow_target_time >= fc::minutes(1) && pow_target_time <= fc::hours(1),
            "POW target time must be between 1 minute and 1 hour." );
         FC_ASSERT( pow_decay_time >= fc::days(1) && pow_decay_time <= fc::days(30),
            "POW Decay time must be between 1 and 30 days." );
         FC_ASSERT( txn_stake_decay_time >= fc::days(1) && txn_stake_decay_time <= fc::days(30),
            "Transaction Stake Decay time must be between 1 and 30 days." );
         FC_ASSERT( escrow_bond_percent >= 0 && escrow_bond_percent <= PERCENT_100,
            "Credit interest rate must be between 0 and PERCENT_100." );
         FC_ASSERT( credit_interest_rate >= 0 && credit_interest_rate <= PERCENT_100,
            "Credit interest rate must be between 0 and PERCENT_100." );
         FC_ASSERT( credit_open_ratio >= PERCENT_100 && credit_open_ratio <= PERCENT_100 * 2,
            "Credit interest rate must be PERCENT_100 and 2 * PERCENT_100." );
         FC_ASSERT( credit_liquidation_ratio >= PERCENT_100 && credit_liquidation_ratio <= PERCENT_100 * 2,
            "Credit interest rate must be PERCENT_100 and 2 * PERCENT_100." );
         FC_ASSERT( credit_min_interest >= 0 && credit_min_interest <= PERCENT_100,
            "Credit min interest rate must be between 0 and PERCENT_100." );
         FC_ASSERT( credit_variable_interest >= 0 && credit_variable_interest <= PERCENT_100,
            "Credit variable interest rate must be between 0 and PERCENT_100." );
         FC_ASSERT( market_max_credit_ratio >= 0 && market_max_credit_ratio <= PERCENT_100,
            "Market max credit ratio must be between 0 and PERCENT_100." );
         FC_ASSERT( margin_open_ratio >= PERCENT_1 && margin_open_ratio <= PERCENT_100,
            "Margin Open Ratio must be between PERCENT_1 and PERCENT_100." );
         FC_ASSERT( margin_liquidation_ratio >= PERCENT_1 && margin_liquidation_ratio <= PERCENT_100,
            "Margin Liquidation Ratio must be between PERCENT_1 and PERCENT_100." );
         FC_ASSERT( maximum_asset_feed_publishers >= MAX_ASSET_FEED_PUBLISHERS / 10 &&
            maximum_asset_feed_publishers <= MAX_ASSET_FEED_PUBLISHERS * 10,
            "Maximum asset feed publishers must be between 10 and 1000." );
         FC_ASSERT( membership_base_price >= MEMBERSHIP_FEE_BASE / 25 && membership_base_price <= MEMBERSHIP_FEE_BASE * 100,
            "Membership base price must be between $0.10 and $250.00." );
         FC_ASSERT( membership_base_price.symbol == SYMBOL_USD,
            "Membership base price must be in the USD asset." );
         FC_ASSERT( membership_mid_price >= MEMBERSHIP_FEE_MID / 25 && membership_mid_price <= MEMBERSHIP_FEE_MID * 100,
            "Membership mid price must be between $1.00 and $2500.00." );
         FC_ASSERT( membership_mid_price.symbol == SYMBOL_USD,
            "Membership mid price must be in the USD asset." );
         FC_ASSERT( membership_top_price >= MEMBERSHIP_FEE_TOP / 25 && membership_top_price <= MEMBERSHIP_FEE_TOP * 100,
            "Membership top price must be between $10.00 and $25000.00." );
         FC_ASSERT( membership_top_price.symbol == SYMBOL_USD,
            "Membership top price must be in the USD asset." );

         FC_ASSERT( vote_reward_percent >= PERCENT_10_OF_PERCENT_1 && vote_reward_percent <= 20 * PERCENT_1,
            "Vote reward percent must be between PERCENT_10_OF_PERCENT_1 and 20 * PERCENT_1." );
         FC_ASSERT( view_reward_percent >= PERCENT_10_OF_PERCENT_1 && view_reward_percent <= 20 * PERCENT_1,
            "View reward percent must be between PERCENT_10_OF_PERCENT_1 and 20 * PERCENT_1." );
         FC_ASSERT( share_reward_percent >= PERCENT_10_OF_PERCENT_1 && share_reward_percent <= 20 * PERCENT_1,
            "Share reward percent must be between PERCENT_10_OF_PERCENT_1 and 20 * PERCENT_1." );
         FC_ASSERT( comment_reward_percent >= PERCENT_10_OF_PERCENT_1 && comment_reward_percent <= 20 * PERCENT_1,
            "Comment reward percent must be between PERCENT_10_OF_PERCENT_1 and 20 * PERCENT_1." );
         FC_ASSERT( storage_reward_percent >= PERCENT_10_OF_PERCENT_1 && storage_reward_percent <= 10 * PERCENT_1,
            "Storage reward percent must be between PERCENT_10_OF_PERCENT_1 and 10 * PERCENT_1." );
         FC_ASSERT( moderator_reward_percent >= PERCENT_10_OF_PERCENT_1 && moderator_reward_percent <= 10 * PERCENT_1,
            "Moderator reward percent must be between PERCENT_10_OF_PERCENT_1 and 10 * PERCENT_1." );

         FC_ASSERT( content_reward_decay_rate >= fc::days(1) && content_reward_decay_rate <= fc::days(365),
            "Content reward decay rate must be between 1 and 365 days." );
         FC_ASSERT( content_reward_interval >= fc::hours(1) && content_reward_interval <= fc::days(7),
            "Content reward interval must be between 1 hour and 7 days." );

         FC_ASSERT( vote_reserve_rate >= 1 && vote_reserve_rate <= 10000,
            "Vote reserve rate must be between 1 and 10000." );
         FC_ASSERT( view_reserve_rate >= 1 && view_reserve_rate <= 10000,
            "View reserve rate must be between 1 and 10000." );
         FC_ASSERT( share_reserve_rate >= 1 && share_reserve_rate <= 10000,
            "Share reserve rate must be between 1 and 10000." );
         FC_ASSERT( comment_reserve_rate >= 1 && comment_reserve_rate <= 10000,
            "Comment reserve rate must be between 1 and 10000." );

         FC_ASSERT( vote_recharge_time >= fc::days(1) && vote_recharge_time <= fc::days(365),
            "Vote Recharge time must be between 1 and 365 days." );
         FC_ASSERT( view_recharge_time >= fc::days(1) && view_recharge_time <= fc::days(365),
            "View Recharge time must be between 1 and 365 days." );
         FC_ASSERT( share_recharge_time >= fc::days(1) && share_recharge_time <= fc::days(365),
            "Share Recharge time must be between 1 and 365 days." );
         FC_ASSERT( comment_recharge_time >= fc::days(1) && comment_recharge_time <= fc::days(365),
            "Comment Recharge time must be between 1 and 365 days." );
         FC_ASSERT( curation_auction_decay_time >= fc::minutes(1) && curation_auction_decay_time <= fc::days(1),
            "Curation auction decay time must be between 1 minute and 1 day." );

         FC_ASSERT( vote_curation_decay >= 1 && vote_curation_decay <= 100000,
            "Vote curation decay must be between 1 and 100,000." );
         FC_ASSERT( view_curation_decay >= 1 && view_curation_decay <= 100000,
            "View curation decay must be between 1 and 100,000." );
         FC_ASSERT( share_curation_decay >= 1 && share_curation_decay <= 100000,
            "Share curation decay must be between 1 and 100,000." );
         FC_ASSERT( comment_curation_decay >= 1 && comment_curation_decay <= 100000,
            "Comment curation decay must be between 1 and 100,000." );

         FC_ASSERT( supernode_decay_time >= fc::days(1) && supernode_decay_time <= fc::days(365),
            "Supernode Decay time must be between 1 and 365 days." );
         FC_ASSERT( enterprise_vote_percent_required >= 0 && enterprise_vote_percent_required <= PERCENT_100,
            "Enterprise vote percent required must be between 0 and PERCENT_100." );
         FC_ASSERT( maximum_asset_whitelist_authorities >= MAX_ASSET_WHITELIST_AUTHORITIES && 
            maximum_asset_whitelist_authorities <= 10 * MAX_ASSET_WHITELIST_AUTHORITIES,
            "Executive types amount must be between 1000 and 10,000." );
         FC_ASSERT( max_stake_intervals >= MAX_ASSET_STAKE_INTERVALS && max_stake_intervals <= 100 * MAX_ASSET_STAKE_INTERVALS,
            "Max stake intervals must be between 104 and 10400." );
         FC_ASSERT( max_unstake_intervals >= MAX_ASSET_UNSTAKE_INTERVALS && max_unstake_intervals <= 100 * MAX_ASSET_UNSTAKE_INTERVALS,
            "Max unstake intervals must be between 104 and 10400." );
         FC_ASSERT( max_exec_budget.symbol == SYMBOL_CREDIT,
            "Max Excutive Budget must be in the CREDIT asset." );
         FC_ASSERT( max_exec_budget >= MAX_EXEC_BUDGET,
            "Max Excutive Budget must be less than or equal to 1,000,000 MCR." );
      };
   };

   /**
    * Creates or updates a producer for a specified account, enabling block production.
    * 
    * If the owner isn't a producer they will become a producer, 
    * and become eligible to receive producer votes 
    * from all stakeholding accounts.
    *  
    * The network will pick the top voted producers 
    * and top producing miners for producing blocks.
    * 
    * Producer Details should address the following particulars:
    * 
    * - The Hardware that the producer operating their node on.
    * - The Backup Node setup that is in use.
    * - Any Relevant past experience operating block producers.
    * - The Team members that are operating the producer.
    * - Network services offered by the producer: API endpoints, applications, tools.
    * - Estimated Hash Power of mining hardware in use.
    * - Best reasons to vote in support of the Producer.
    * 
    * The Chain Properties Object: props contains a variety of network
    * parameters that are used for managing the characteristics of the 
    * protocol, and are voted on by block producers.
    * The median value for each parameter is used, enabling each value to
    * be updated with the support of the majority of the 
    * block producers.
    */
   struct producer_update_operation : public base_operation
   {
      account_name_type             signatory;
      
      account_name_type             owner;                  ///< The account that owns the producer.

      string                        details;                ///< The producer's details for stakeholder voting reference.

      string                        url;                    ///< Producer's reference URL for more information.

      string                        json;                   ///< The producers json metadata.

      double                        latitude;               ///< Latitude co-ordinates of the producer.

      double                        longitude;              ///< Longitude co-ordinates of the producer.

      string                        block_signing_key;      ///< The public key used to sign blocks.

      chain_properties              props;                  ///< Chain properties values for selection of adjustable network parameters. 

      bool                          active = true;          ///< Set active to true to activate producer, false to deactivate.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( owner ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   struct proof_of_work_input
   {
      block_id_type                 prev_block = block_id_type();

      account_name_type             miner_account = NULL_ACCOUNT;
      
      uint64_t                      nonce = 0;

      string                        to_string()const;
   };


   struct x11_proof_of_work
   {
      proof_of_work_input           input;

      x11                           work;

      void create( block_id_type prev_block, account_name_type account_name, uint64_t nonce );
      void validate()const;
   };


   struct equihash_proof_of_work
   {
      proof_of_work_input           input;

      fc::equihash::proof           proof;

      block_id_type                 prev_block;

      fc::sha256                    work;

      void create( block_id_type recent_block, account_name_type account_name, uint64_t nonce );
      void validate() const;
   };


   typedef fc::static_variant< x11_proof_of_work, equihash_proof_of_work > proof_of_work_type;


   /**
    * Enables mining producers to publish cryptographic proofs of work.
    * 
    * Proofs of Work make the blockchain thermodynamically secure against rewriting
    * due to the energy expenditure required to redo the work.
    * 
    * The Network uses the X11 Mining algorithm for hashing.
    * 
    * Miners are added to the block production set according to the number of 
    * proofs of work in the prior 7 days.
    * 
    * The highest producing miners are able to produce blocks in each round, 
    * and claim the proof of work reward when they publish a proof.
    */
   struct proof_of_work_operation : public base_operation
   {
      proof_of_work_type            work;              ///< Proof of work, containing a reference to a prior block, and a nonce resulting in a low hash value.

      fc::optional< string >        new_owner_key;     ///< If creating a new account with a proof of work, the owner key of the new account.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const { a.insert( work.get< x11_proof_of_work >().input.miner_account ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const;
      void                          get_required_authorities( vector< authority >& a )const { if( new_owner_key.valid() ){ a.push_back( authority( 1, *new_owner_key, 1 ) ); } };
   };


   /**
    * Enables block producers to verify that a valid block exists at a given height.
    * 
    * Once a block has been verfied, it can be commited by adding stake weight 
    * to the verfication, and locking it in, preventing the details from being updated. 
    */
   struct verify_block_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             producer;      ///< The name of the block producing account.

      block_id_type                 block_id;      ///< The Block ID of the block being verifed as valid and received. 

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( producer ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /**
    * Stakes COIN on the validity and acceptance of a block.
    * 
    * The commit block operation enables producers to assert that:
    * 1 - A given block at a given height is valid.
    * 2 - A supermajority of least Two Thirds Plus One (67) block producers have verified the block.
    * 3 - They will not produce future blocks that do not contain that block as an ancestor.
    * 4 - They stake a given value of COIN on their commitment.
    * 
    * In resolution of the Nothing At Stake problem of consensus, 
    * in which producers sign multiple
    * commitments without validating blocks to ensure maximum reward, 
    * they are penalized for commiting to different blocks at the same height, 
    * or producing blocks which do not contain a committed block as an ancestor.
    * 
    * If the producer signs duplicate commitments at the same height,
    * or produces blocks that deviate from this blocks history,
    * the staked value is forfeited to any account that publishes a violation proof transaction.
    * 
    * Producers cannot sign commitments for blocks that 
    * are already irreversible by production history depth, 
    * or have already been committed by more than two thirds of producers.
    * 
    * After a block becomes irreversible, the fastest Two Thirds Plus One (67) 
    * block producers that have committed to the block are rewarded according to their 
    * staked amounts from the validation reward pool.
    * 
    * If more than 67 producers signed and published commitments transactions, 
    * all producers within the last block to include commitment transactions 
    * before exceeding 67 are included for the reward distribution.
    */
   struct commit_block_operation : public base_operation
   {
      account_name_type                 signatory;

      account_name_type                 producer;            ///< The name of the block producing account.

      block_id_type                     block_id;            ///< The block id of the block being committed as irreversible to that producer.

      flat_set< account_name_type >     verifications;       ///< The set of attesting verifications from currently active producers.

      asset                             commitment_stake;    ///< The value of staked balance that the producer stakes on this commitment. Must be at least one unit of COIN. 

      void                              validate()const;
      void                              get_creator_name( flat_set<account_name_type>& a )const{ a.insert( producer ); }
      void                              get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   /** 
    * Declares a violation of a block commitment.
    * 
    * Enables a reporter to claim the commitment stake of that producer.
    */
   struct producer_violation_operation : public base_operation
   {
      account_name_type             signatory;

      account_name_type             reporter;      ///< The account detecting and reporting the violation.

      vector<char>                  first_trx;     ///< The first transaction signed by the producer.

      vector<char>                  second_trx;    ///< The second transaction that is in contravention of the first commitment transaction. 

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( reporter ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( signatory ); }
   };


   //===========================//
   //==== Custom Operations ====//
   //===========================//


   /**
    * Provides a generic way (using char vector) to add higher level protocols on top of network consensus.
    * 
    * No validation applied for this operation other than that required auths are valid.
    */
   struct custom_operation : public base_operation
   {
      flat_set< account_name_type >      required_auths;    ///< Set of account authorities required for the transaction signature.

      uint16_t                           id = 0;            ///< ID of the custom operation, determines the how to interpret the operation.

      vector< char >                     data;              ///< Char vector of data contained within the operation. Max size 8 KB

      void                               validate()const;
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( *required_auths.begin() ); }
      void                               get_required_active_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_auths ) a.insert(i); }
   };


   /**
    * Provides a generic way (using JSON String) to add higher level protocols on top of network consensus.
    * 
    * No validation applied for this operation other than that required auths are valid.
    * Typically JSON operations can be interpretted by plugins as containing 
    * information useful to a specific class of API serving nodes, and can facilitate the 
    * addition of new types of operations when plugins create overlaid object structures for
    * management of state that is outside direct consensus. 
    */
   struct custom_json_operation : public base_operation
   {
      flat_set< account_name_type >     required_auths;           ///< Set of account active authorities required for the transaction signature.

      flat_set< account_name_type >     required_posting_auths;   ///< Set of posting authorities required for the transaction signature.

      string                            id;                       ///< ID of the operation, refers to the plugin used to interpret the operation. Less than 32 characters long.

      string                            json;                     ///< Valid UTF8 / JSON string of operation data. Max size 8 KB

      void                              validate()const;
      void                              get_creator_name( flat_set<account_name_type>& a )const 
      { 
         if( required_auths.begin() != required_auths.end() )
         {
            a.insert( *required_auths.begin() );
         }
         else if( required_posting_auths.begin() != required_posting_auths.end() )
         {
            a.insert( *required_posting_auths.begin() );
         }
         else
         {
            a.insert( account_name_type( NULL_ACCOUNT ) );
         }
      }
      void                              get_required_active_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_auths ) a.insert(i); }
      void                              get_required_posting_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_posting_auths ) a.insert(i); }
   };

   /**
    * @}
    */
   
} } ///< node::protocol

   //============================//
   //==== Account Operations ====//
   //============================//

FC_REFLECT( node::protocol::account_create_operation,
         (signatory)
         (registrar)
         (new_account_name)
         (referrer)
         (proxy)
         (recovery_account)
         (reset_account)
         (details)
         (url)
         (image)
         (json)
         (json_private)
         (first_name)
         (last_name)
         (gender)
         (date_of_birth)
         (email)
         (phone)
         (nationality)
         (owner_auth)
         (active_auth)
         (posting_auth)
         (secure_public_key)
         (connection_public_key)
         (friend_public_key)
         (companion_public_key)
         (fee)
         (delegation)
         );

FC_REFLECT( node::protocol::account_update_operation,
         (signatory)
         (account)
         (details)
         (url)
         (image)
         (json)
         (json_private)
         (first_name)
         (last_name)
         (gender)
         (date_of_birth)
         (email)
         (phone)
         (nationality)
         (owner_auth)
         (active_auth)
         (posting_auth)
         (secure_public_key)
         (connection_public_key)
         (friend_public_key)
         (companion_public_key)
         (pinned_permlink)
         (active)
         );

FC_REFLECT( node::protocol::account_verification_operation,
         (signatory)
         (verifier_account)
         (verified_account)
         (shared_image)
         (verified)
         );

FC_REFLECT( node::protocol::account_business_operation,
         (signatory)
         (account)
         (init_ceo_account)
         (business_type)
         (officer_vote_threshold)
         (business_public_key)
         (active)
         );

FC_REFLECT( node::protocol::account_membership_operation,
         (signatory)
         (account)
         (membership_type)
         (months)
         (interface)
         (recurring)
         );

FC_REFLECT( node::protocol::account_vote_executive_operation,
         (signatory)
         (account)
         (business_account)
         (executive_account)
         (role)
         (vote_rank)
         (approved)
         );

FC_REFLECT( node::protocol::account_vote_officer_operation,
         (signatory)
         (account)
         (business_account)
         (officer_account)
         (vote_rank)
         (approved)
         );

FC_REFLECT( node::protocol::account_member_request_operation,
         (signatory)
         (account)
         (business_account)
         (message)
         (requested)
         );

FC_REFLECT( node::protocol::account_member_invite_operation,
         (signatory)
         (account)
         (business_account)
         (member)
         (message)
         (encrypted_business_key)
         (invited)
         );

FC_REFLECT( node::protocol::account_accept_request_operation,
         (signatory)
         (account)
         (business_account)
         (member)
         (encrypted_business_key)
         (accepted)
         );

FC_REFLECT( node::protocol::account_accept_invite_operation,
         (signatory)
         (account)
         (business_account)
         (accepted)
         );

FC_REFLECT( node::protocol::account_remove_member_operation,
         (signatory)
         (account)
         (business_account)
         (member)
         );

FC_REFLECT( node::protocol::account_update_list_operation,
         (signatory)
         (account)
         (listed_account)
         (listed_asset)
         (blacklisted)
         (whitelisted)
         );

FC_REFLECT( node::protocol::account_producer_vote_operation,
         (signatory)
         (account)
         (vote_rank)
         (producer)
         (approved)
         );

FC_REFLECT( node::protocol::account_update_proxy_operation, 
         (signatory)
         (account)
         (proxy) 
         );

FC_REFLECT( node::protocol::request_account_recovery_operation, 
         (signatory)
         (recovery_account)
         (account_to_recover)
         (new_owner_authority)
         );

FC_REFLECT( node::protocol::recover_account_operation, 
         (signatory)
         (account_to_recover)
         (new_owner_authority)
         (recent_owner_authority)
         );

FC_REFLECT( node::protocol::reset_account_operation, 
         (signatory)
         (reset_account)
         (account_to_reset)
         (new_owner_authority) 
         );

FC_REFLECT( node::protocol::set_reset_account_operation, 
         (signatory)
         (account)
         (new_reset_account)
         (days)
         );

FC_REFLECT( node::protocol::change_recovery_account_operation, 
         (signatory)
         (account_to_recover)
         (new_recovery_account)
         );

FC_REFLECT( node::protocol::decline_voting_rights_operation, 
         (signatory)
         (account)
         (declined)
         );

FC_REFLECT( node::protocol::connection_request_operation, 
         (signatory)
         (account)
         (requested_account)
         (connection_type)
         (message)
         (requested)
         );       

FC_REFLECT( node::protocol::connection_accept_operation, 
         (signatory)
         (account)
         (requesting_account)
         (connection_id)
         (connection_type)
         (encrypted_key)
         (connected)
         );

FC_REFLECT( node::protocol::account_follow_operation, 
         (signatory)
         (follower)
         (following)
         (interface)
         (added)
         (followed)
         );

FC_REFLECT( node::protocol::tag_follow_operation, 
         (signatory)
         (follower)
         (tag)
         (interface)
         (added)
         (followed)
         );

FC_REFLECT( node::protocol::activity_reward_operation, 
         (signatory)
         (account)
         (permlink)
         (interface)
         );


   //===========================//
   //==== Network Operations ===//
   //===========================//



FC_REFLECT( node::protocol::update_network_officer_operation, 
         (signatory)
         (account)
         (officer_type)
         (details)
         (url)
         (json)
         (active)
         );

FC_REFLECT( node::protocol::network_officer_vote_operation, 
         (signatory)
         (account)
         (network_officer)
         (vote_rank)
         (approved)
         );

FC_REFLECT( node::protocol::executive_officer_set, 
         (CHIEF_EXECUTIVE_OFFICER)
         (CHIEF_OPERATING_OFFICER)
         (CHIEF_FINANCIAL_OFFICER)
         (CHIEF_DEVELOPMENT_OFFICER)
         (CHIEF_TECHNOLOGY_OFFICER)
         (CHIEF_SECURITY_OFFICER)
         (CHIEF_GOVERNANCE_OFFICER)
         (CHIEF_MARKETING_OFFICER)
         (CHIEF_DESIGN_OFFICER)
         (CHIEF_ADVOCACY_OFFICER)
         );

FC_REFLECT( node::protocol::update_executive_board_operation, 
         (signatory)
         (account)
         (executive)
         (budget)
         (details)
         (url)
         (json)
         (active)
         );

FC_REFLECT( node::protocol::executive_board_vote_operation, 
         (signatory)
         (account)
         (executive_board)
         (vote_rank)
         (approved)
         );

FC_REFLECT( node::protocol::update_governance_operation, 
         (signatory)
         (account)
         (details)
         (url)
         (json)
         (active)
         );

FC_REFLECT( node::protocol::subscribe_governance_operation, 
         (signatory)
         (account)
         (governance_account)
         (subscribe)
         );

FC_REFLECT( node::protocol::update_supernode_operation, 
         (signatory)
         (account)
         (details)
         (url)
         (node_api_endpoint)
         (notification_api_endpoint)
         (auth_api_endpoint)
         (ipfs_endpoint)
         (bittorrent_endpoint)
         (json)
         (active)
         );

FC_REFLECT( node::protocol::update_interface_operation, 
         (signatory)
         (account)
         (details)
         (url)
         (json)
         (active)
         );

FC_REFLECT( node::protocol::update_mediator_operation, 
         (signatory)
         (account)
         (details)
         (url)
         (json)
         (mediator_bond)
         (active)
         );

FC_REFLECT( node::protocol::create_community_enterprise_operation, 
         (signatory)
         (creator)
         (enterprise_id)
         (beneficiaries)
         (milestone_shares)
         (details)
         (url)
         (json)
         (begin)
         (duration)
         (daily_budget)
         (fee)
         (active)
         );

FC_REFLECT( node::protocol::claim_enterprise_milestone_operation, 
         (signatory)
         (creator)
         (enterprise_id)
         (milestone)
         );

FC_REFLECT( node::protocol::approve_enterprise_milestone_operation, 
         (signatory)
         (account)
         (creator)
         (enterprise_id)
         (milestone)
         (vote_rank)
         (approved)
         );

   //=====================================//
   //==== Post and Comment Operations ====//
   //=====================================//


FC_REFLECT( node::protocol::comment_options, 
         (post_type)
         (reach)
         (reply_connection)
         (rating)
         (reward_currency)
         (max_accepted_payout)
         (percent_liquid)
         (allow_replies)
         (allow_votes)
         (allow_views)
         (allow_shares)
         (allow_curation_rewards)
         (beneficiaries)
         );

FC_REFLECT( node::protocol::comment_operation,
         (signatory)
         (author)
         (permlink)
         (title)
         (body)
         (ipfs)
         (magnet)
         (url)
         (language)
         (community)
         (public_key)
         (interface)
         (latitude)
         (longitude)
         (comment_price)
         (reply_price)
         (premium_price)
         (parent_author)
         (parent_permlink)
         (tags)
         (json)
         (options)
         (deleted)
         );

FC_REFLECT( node::protocol::beneficiary_route_type, 
         (account)
         (weight) 
         );

FC_REFLECT( node::protocol::comment_payout_beneficiaries, 
         (beneficiaries) 
         );

FC_REFLECT( node::protocol::message_operation,
         (signatory)
         (sender)
         (recipient)
         (message)
         (uuid)
         );

FC_REFLECT( node::protocol::vote_operation,
         (signatory)
         (voter)
         (author)
         (permlink)
         (weight)
         (interface)
         );

FC_REFLECT( node::protocol::view_operation,
         (signatory)
         (viewer)
         (author)
         (permlink)
         (interface)
         (supernode)
         (viewed)
         );

FC_REFLECT( node::protocol::share_operation,
         (signatory)
         (sharer)
         (author)
         (permlink)
         (reach)
         (interface)
         (community)
         (tag)
         (shared)
         );

FC_REFLECT( node::protocol::moderation_tag_operation,
         (signatory)
         (moderator)
         (author)
         (permlink)
         (tags)
         (rating)
         (details)
         (interface)
         (filter)
         (applied)
         );

FC_REFLECT( node::protocol::list_operation,
         (signatory)
         (creator)
         (list_id)
         (name)
         (accounts)
         (comments)
         (communities)
         (assets)
         (products)
         (auctions)
         (nodes)
         (edges)
         (node_types)
         (edge_types)
         );

FC_REFLECT( node::protocol::poll_operation,
         (signatory)
         (creator)
         (poll_id)
         (details)
         (poll_options)
         (completion_time)
         );

FC_REFLECT( node::protocol::poll_vote_operation,
         (signatory)
         (voter)
         (creator)
         (poll_id)
         (poll_option)
         );


   //==============================//
   //==== Community Operations ====//
   //==============================//

FC_REFLECT( node::protocol::community_create_operation,
         (signatory)
         (founder)
         (name)
         (community_privacy)
         (community_public_key)
         (json)
         (json_private)
         (details)
         (url)
         );

FC_REFLECT( node::protocol::community_update_operation,
         (signatory)
         (account)
         (community)
         (community_public_key)
         (json)
         (json_private)
         (details)
         (url)
         (pinned_author)
         (pinned_permlink)
         (active)
         );

FC_REFLECT( node::protocol::community_add_mod_operation,
         (signatory)
         (account)
         (community)
         (moderator)
         (added)
         );

FC_REFLECT( node::protocol::community_add_admin_operation,
         (signatory)
         (account)
         (community)
         (admin)
         (added)
         );

FC_REFLECT( node::protocol::community_vote_mod_operation,
         (signatory)
         (account)
         (community)
         (moderator)
         (vote_rank)
         (approved)
         );

FC_REFLECT( node::protocol::community_transfer_ownership_operation,
         (signatory)
         (account)
         (community)
         (new_founder)
         );

FC_REFLECT( node::protocol::community_join_request_operation,
         (signatory)
         (account)
         (community)
         (message)
         (requested)
         );

FC_REFLECT( node::protocol::community_join_invite_operation,
         (signatory)
         (account)
         (member)
         (community)
         (message)
         (encrypted_community_key)
         (invited)
         );

FC_REFLECT( node::protocol::community_join_accept_operation,
         (signatory)
         (account)
         (member)
         (community)
         (encrypted_community_key)
         (accepted)
         );

FC_REFLECT( node::protocol::community_invite_accept_operation,
         (signatory)
         (account)
         (community)
         (accepted)
         );

FC_REFLECT( node::protocol::community_remove_member_operation,
         (signatory)
         (account)
         (member)
         (community)
         );

FC_REFLECT( node::protocol::community_blacklist_operation,
         (signatory)
         (account)
         (member)
         (community)
         (blacklisted)
         );

FC_REFLECT( node::protocol::community_subscribe_operation,
         (signatory)
         (account)
         (community)
         (interface)
         (added)
         (subscribed)
         );

FC_REFLECT( node::protocol::community_event_operation,
         (signatory)
         (account)
         (community)
         (event_name)
         (location)
         (latitude)
         (longitude)
         (details)
         (url)
         (json)
         (event_start_time)
         (event_end_time)
         );

FC_REFLECT( node::protocol::community_event_attend_operation,
         (signatory)
         (account)
         (community)
         (attending)
         (not_attending)
         );


   //================================//
   //==== Advertising Operations ====//
   //================================//


FC_REFLECT( node::protocol::ad_creative_operation,
         (signatory)
         (account)
         (author)
         (objective)
         (creative_id)
         (creative)
         (json)
         (format_type)
         (active)
         );

FC_REFLECT( node::protocol::ad_campaign_operation,
         (signatory)
         (account)
         (campaign_id)
         (budget)
         (begin)
         (end)
         (json)
         (agents)
         (interface)
         (active)
         );

FC_REFLECT( node::protocol::ad_inventory_operation,
         (signatory)
         (provider)
         (inventory_id)
         (audience_id)
         (metric)
         (min_price)
         (inventory)
         (json)
         (active)
         );

FC_REFLECT( node::protocol::ad_audience_operation,
         (signatory)
         (account)
         (audience_id)
         (json)
         (audience)
         (active)
         );

FC_REFLECT( node::protocol::ad_bid_operation,
         (signatory)
         (bidder)
         (bid_id)
         (account)
         (campaign_id)
         (author)
         (creative_id)
         (provider)
         (inventory_id)
         (bid_price)
         (requested)
         (included_audiences)
         (excluded_audiences)
         (audience_id)
         (json)
         (expiration)
         (active)
         );


   //==========================//
   //==== Graph Operations ====//
   //==========================//


FC_REFLECT( node::protocol::graph_node_operation,
         (signatory)
         (account)
         (node_types)
         (node_id)
         (name)
         (details)
         (attributes)
         (attribute_values)
         (json)
         (json_private)
         (node_public_key)
         (interface)
         );

FC_REFLECT( node::protocol::graph_edge_operation,
         (signatory)
         (account)
         (edge_types)
         (edge_id)
         (from_node_account)
         (from_node_id)
         (to_node_account)
         (to_node_id)
         (name)
         (details)
         (attributes)
         (attribute_values)
         (json)
         (json_private)
         (edge_public_key)
         (interface)
         );

FC_REFLECT( node::protocol::graph_node_property_operation,
         (signatory)
         (account)
         (node_type)
         (graph_privacy)
         (edge_permission)
         (details)
         (url)
         (json)
         (attributes)
         (interface)
         );

FC_REFLECT( node::protocol::graph_edge_property_operation,
         (signatory)
         (account)
         (edge_type)
         (graph_privacy)
         (from_node_types)
         (to_node_types)
         (details)
         (url)
         (json)
         (attributes)
         (interface)
         );


   //=============================//
   //==== Transfer Operations ====//
   //=============================//


FC_REFLECT( node::protocol::transfer_operation,
         (signatory)
         (from)
         (to)
         (amount)
         (memo)
         );

FC_REFLECT( node::protocol::transfer_request_operation,
         (signatory)
         (to)
         (from)
         (amount)
         (memo)
         (request_id)
         (expiration)
         (requested)
         );

FC_REFLECT( node::protocol::transfer_accept_operation,
         (signatory)
         (from)
         (to)
         (request_id)
         (accepted)
         );

FC_REFLECT( node::protocol::transfer_recurring_operation,
         (signatory)
         (from)
         (to)
         (amount)
         (transfer_id)
         (begin)
         (payments)
         (interval)
         (memo)
         (extensible)
         (fill_or_kill)
         (active)
         );

FC_REFLECT( node::protocol::transfer_recurring_request_operation,
         (signatory)
         (to)
         (from)
         (amount)
         (request_id)
         (begin)
         (payments)
         (interval)
         (memo)
         (expiration)
         (extensible)
         (fill_or_kill)
         (requested)
         );

FC_REFLECT( node::protocol::transfer_recurring_accept_operation,
         (signatory)
         (from)
         (to)
         (request_id)
         (accepted)
         );

FC_REFLECT( node::protocol::stealth_confirmation,
         (one_time_key)
         (to)
         (encrypted_memo)
         );

FC_REFLECT( node::protocol::stealth_confirmation::memo_data,
         (from)
         (amount)
         (blinding_factor)
         (commitment)
         (check)
         );

FC_REFLECT( node::protocol::blind_memo,
         (from)
         (amount)
         (message)
         (check)
         );

FC_REFLECT( node::protocol::confidential_input,
         (commitment)
         (owner)
         );

FC_REFLECT( node::protocol::confidential_output,
         (commitment)
         (range_proof)
         (owner)
         (stealth_memo)
         );

FC_REFLECT( node::protocol::transfer_confidential_operation,
         (inputs)
         (outputs)
         (fee)
         );

FC_REFLECT( node::protocol::transfer_to_confidential_operation,
         (signatory)
         (from)
         (amount)
         (blinding_factor)
         (outputs)
         (fee)
         );

FC_REFLECT( node::protocol::transfer_from_confidential_operation,
         (to)
         (amount)
         (blinding_factor)
         (inputs)
         (fee)
         );



   //============================//
   //==== Balance Operations ====//
   //============================//



FC_REFLECT( node::protocol::claim_reward_balance_operation,
         (signatory)
         (account)
         (reward)
         );

FC_REFLECT( node::protocol::stake_asset_operation,
         (signatory)
         (from)
         (to)
         (amount) 
         );

FC_REFLECT( node::protocol::unstake_asset_operation, 
         (signatory)
         (from)
         (to)
         (amount) 
         );

FC_REFLECT( node::protocol::unstake_asset_route_operation, 
         (signatory)
         (from)
         (to)
         (percent)
         (auto_stake)
         );

FC_REFLECT( node::protocol::transfer_to_savings_operation,
         (signatory)
         (from)
         (to)
         (amount)
         (memo) 
         );

FC_REFLECT( node::protocol::transfer_from_savings_operation, 
         (signatory)
         (from)
         (to)
         (amount)
         (request_id)
         (memo)
         (transferred)
         );

FC_REFLECT( node::protocol::delegate_asset_operation, 
         (signatory)
         (delegator)
         (delegatee)
         (amount)
         );



   //================================//
   //==== Marketplace Operations ====//
   //================================//


FC_REFLECT( node::protocol::product_sale_operation, 
         (signatory)
         (account)
         (product_id)
         (name)
         (url)
         (json)
         (product_variants)
         (product_details)
         (product_image)
         (product_prices)
         (wholesale_discount)
         (stock_available)
         (delivery_variants)
         (delivery_details)
         (delivery_prices)
         (active)
         );

FC_REFLECT( node::protocol::product_purchase_operation, 
         (signatory)
         (buyer)
         (product_id)
         (seller)
         (product_id)
         (order_variants)
         (order_size)
         (memo)
         (json)
         (shipping_address)
         (delivery_variant)
         (delivery_details)
         (acceptance_time)
         (escrow_expiration)
         );

FC_REFLECT( node::protocol::product_auction_sale_operation, 
         (signatory)
         (account)
         (auction_id)
         (auction_type)
         (name)
         (url)
         (json)
         (product_details)
         (product_image)
         (reserve_bid)
         (maximum_bid)
         (delivery_variants)
         (delivery_details)
         (delivery_prices)
         (final_bid_time)
         (completion_time)
         );

FC_REFLECT( node::protocol::product_auction_bid_operation, 
         (signatory)
         (buyer)
         (bid_id)
         (seller)
         (auction_id)
         (bid_price_commitment)
         (public_bid_amount)
         (memo)
         (json)
         (shipping_address)
         (delivery_variant)
         (delivery_details)
         );

FC_REFLECT( node::protocol::escrow_transfer_operation, 
         (signatory)
         (account)
         (from)
         (to)
         (escrow_id)
         (amount)
         (acceptance_time)
         (escrow_expiration)
         (memo)
         (json)
         );

FC_REFLECT( node::protocol::escrow_approve_operation, 
         (signatory)
         (account)
         (mediator)
         (escrow_from)
         (escrow_id)
         (approved)
         );

FC_REFLECT( node::protocol::escrow_dispute_operation, 
         (signatory)
         (account)
         (escrow_from)
         (escrow_id) 
         );

FC_REFLECT( node::protocol::escrow_release_operation, 
         (signatory)
         (account)
         (escrow_from)
         (escrow_id)
         (release_percent)
         );



   //============================//
   //==== Trading Operations ====//
   //============================//



FC_REFLECT( node::protocol::limit_order_operation, 
         (signatory)
         (owner)
         (order_id)
         (amount_to_sell)
         (exchange_rate)
         (interface)
         (expiration)
         (opened)
         (fill_or_kill)
         );

FC_REFLECT( node::protocol::margin_order_operation, 
         (signatory)
         (owner)
         (order_id)
         (exchange_rate)
         (collateral)
         (amount_to_borrow)
         (stop_loss_price)
         (take_profit_price)
         (limit_stop_loss_price)
         (limit_take_profit_price)
         (interface)
         (expiration)
         (opened)
         (fill_or_kill)
         (force_close)
         );

FC_REFLECT( node::protocol::auction_order_operation, 
         (signatory)
         (owner)
         (order_id)
         (amount_to_sell)
         (limit_close_price)
         (interface)
         (expiration)
         (opened)
         );

FC_REFLECT( node::protocol::call_order_operation, 
         (signatory)
         (owner)
         (collateral)
         (debt)
         (target_collateral_ratio)
         (interface)
         );

FC_REFLECT( node::protocol::option_order_operation, 
         (signatory)
         (owner)
         (order_id)
         (options_issued)
         (interface)
         (opened)
         );



   //=========================//
   //==== Pool Operations ====//
   //=========================//



FC_REFLECT( node::protocol::liquidity_pool_create_operation, 
         (signatory)
         (account)
         (first_amount)
         (second_amount)
         );

FC_REFLECT( node::protocol::liquidity_pool_exchange_operation, 
         (signatory)
         (account)
         (amount)
         (receive_asset)
         (interface)
         (limit_price)
         (acquire)
         );

FC_REFLECT( node::protocol::liquidity_pool_fund_operation, 
         (signatory)
         (account)
         (amount)
         (pair_asset)
         );

FC_REFLECT( node::protocol::liquidity_pool_withdraw_operation, 
         (signatory)
         (account)
         (amount)
         (receive_asset)
         );

FC_REFLECT( node::protocol::credit_pool_collateral_operation, 
         (signatory)
         (account)
         (amount)
         );

FC_REFLECT( node::protocol::credit_pool_borrow_operation, 
         (signatory)
         (account)
         (amount)
         (collateral)
         (loan_id)
         (flash_loan)
         );

FC_REFLECT( node::protocol::credit_pool_lend_operation, 
         (signatory)
         (account)
         (amount)
         );

FC_REFLECT( node::protocol::credit_pool_withdraw_operation, 
         (signatory)
         (account)
         (amount)
         );

FC_REFLECT( node::protocol::option_pool_create_operation, 
         (signatory)
         (account)
         (first_asset)
         (second_asset)
         );

FC_REFLECT( node::protocol::prediction_pool_create_operation, 
         (signatory)
         (account)
         (prediction_symbol)
         (collateral_symbol)
         (outcome_assets)
         (outcome_details)
         (display_symbol)
         (json)
         (url)
         (details)
         (outcome_time)
         (prediction_bond)
         );

FC_REFLECT( node::protocol::prediction_pool_exchange_operation, 
         (signatory)
         (account)
         (amount)
         (prediction_asset)
         (exchange_base)
         (withdraw)
         );

FC_REFLECT( node::protocol::prediction_pool_resolve_operation, 
         (signatory)
         (account)
         (amount)
         (resolution_outcome)
         );



   //==========================//
   //==== Asset Operations ====//
   //==========================//



FC_REFLECT( node::protocol::asset_options,
         (display_symbol)
         (details)
         (json)
         (url)
         (max_supply)
         (stake_intervals)
         (unstake_intervals)
         (market_fee_percent)
         (market_fee_share_percent)
         (max_market_fee)
         (issuer_permissions)
         (flags)
         (whitelist_authorities)
         (blacklist_authorities)
         (whitelist_markets)
         (blacklist_markets)
         (block_reward)
         (block_reward_reduction_percent)
         (block_reward_reduction_days)
         (content_reward_percent)
         (equity_asset)
         (equity_reward_percent)
         (producer_reward_percent)
         (supernode_reward_percent)
         (power_reward_percent)
         (community_fund_reward_percent)
         (development_reward_percent)
         (marketing_reward_percent)
         (advocacy_reward_percent)
         (activity_reward_percent)
         (producer_block_reward_percent)
         (validation_reward_percent)
         (txn_stake_reward_percent)
         (work_reward_percent)
         (producer_activity_reward_percent)
         (feed_lifetime)
         (minimum_feeds)
         (asset_settlement_delay)
         (asset_settlement_offset_percent)
         (maximum_asset_settlement_volume)
         (backing_asset)
         (dividend_share_percent)
         (liquid_dividend_percent)
         (staked_dividend_percent)
         (savings_dividend_percent)
         (liquid_voting_rights)
         (staked_voting_rights)
         (savings_voting_rights)
         (min_active_time)
         (min_balance)
         (min_producers)
         (boost_balance)
         (boost_activity)
         (boost_producers)
         (boost_top)
         (buyback_asset)
         (buyback_price)
         (buyback_share_percent)
         (liquid_fixed_interest_rate)
         (liquid_variable_interest_rate)
         (staked_fixed_interest_rate)
         (staked_variable_interest_rate)
         (savings_fixed_interest_rate)
         (savings_variable_interest_rate)
         (var_interest_range)
         );

FC_REFLECT( node::protocol::asset_create_operation,
         (signatory)
         (issuer)
         (symbol)
         (asset_type)
         (coin_liquidity)
         (usd_liquidity)
         (credit_liquidity)
         (options)
         );

FC_REFLECT( node::protocol::asset_update_operation,
         (signatory)
         (issuer)
         (asset_to_update)
         (new_options)
         );

FC_REFLECT( node::protocol::asset_issue_operation, 
         (signatory)
         (issuer)
         (asset_to_issue)
         (issue_to_account)
         (memo)
         );

FC_REFLECT( node::protocol::asset_reserve_operation, 
         (signatory)
         (payer)
         (amount_to_reserve)
         );

FC_REFLECT( node::protocol::asset_update_issuer_operation,
         (signatory)
         (issuer)
         (asset_to_update)
         (new_issuer)
         );

FC_REFLECT( node::protocol::asset_distribution_operation,
         (signatory)
         (issuer)
         (distribution_asset)
         (fund_asset)
         (details)
         (url)
         (json)
         (distribution_rounds)
         (distribution_interval_days)
         (max_intervals_missed)
         (min_input_fund_units)
         (max_input_fund_units)
         (input_fund_unit)
         (output_distribution_unit)
         (min_unit_ratio)
         (max_unit_ratio)
         (min_input_balance_units)
         (max_input_balance_units)
         (begin_time)
         );

FC_REFLECT( node::protocol::asset_distribution_fund_operation,
         (signatory)
         (sender)
         (distribution_asset)
         (amount)
         );

FC_REFLECT( node::protocol::asset_option_exercise_operation,
         (signatory)
         (account)
         (amount)
         );

FC_REFLECT( node::protocol::asset_stimulus_fund_operation,
         (signatory)
         (account)
         (stimulus_asset)
         (amount)
         );

FC_REFLECT( node::protocol::asset_update_feed_producers_operation,
         (signatory)
         (issuer)
         (asset_to_update)
         (new_feed_producers)
         );

FC_REFLECT( node::protocol::asset_publish_feed_operation, 
         (signatory)
         (publisher)
         (symbol)
         (feed)
         );

FC_REFLECT( node::protocol::asset_settle_operation, 
         (signatory)
         (account)
         (amount)
         (interface)
         );

FC_REFLECT( node::protocol::asset_global_settle_operation, 
         (signatory)
         (issuer)
         (asset_to_settle)
         (settle_price)
         );

FC_REFLECT( node::protocol::asset_collateral_bid_operation, 
         (signatory)
         (bidder)
         (collateral)
         (debt)
         );


   //=====================================//
   //==== Block Production Operations ====//
   //=====================================//


FC_REFLECT( node::protocol::chain_properties,
         (account_creation_fee)
         (asset_coin_liquidity)
         (asset_usd_liquidity)
         (maximum_block_size)
         (pow_target_time)
         (pow_decay_time)
         (txn_stake_decay_time)
         (escrow_bond_percent)
         (credit_interest_rate)
         (credit_open_ratio)
         (credit_liquidation_ratio)
         (credit_min_interest)
         (credit_variable_interest)
         (market_max_credit_ratio)
         (margin_open_ratio)
         (margin_liquidation_ratio)
         (maximum_asset_feed_publishers)
         (membership_base_price)
         (membership_mid_price)
         (membership_top_price)
         (author_reward_percent)
         (vote_reward_percent)
         (view_reward_percent)
         (share_reward_percent)
         (comment_reward_percent)
         (storage_reward_percent)
         (moderator_reward_percent)
         (content_reward_decay_rate)
         (content_reward_interval)
         (vote_reserve_rate)
         (view_reserve_rate)
         (share_reserve_rate)
         (comment_reserve_rate)
         (vote_recharge_time)
         (view_recharge_time)
         (share_recharge_time)
         (comment_recharge_time)
         (curation_auction_decay_time)
         (vote_curation_decay)
         (view_curation_decay)
         (share_curation_decay)
         (comment_curation_decay)
         (supernode_decay_time)
         (enterprise_vote_percent_required)
         (maximum_asset_whitelist_authorities)
         (max_stake_intervals)
         (max_unstake_intervals)
         (max_exec_budget)
         );

FC_REFLECT( node::protocol::producer_update_operation, 
         (signatory)
         (owner)
         (details)
         (url)
         (json)
         (latitude)
         (longitude)
         (block_signing_key)
         (props)
         (active)
         );

FC_REFLECT( node::protocol::x11_proof_of_work,
         (input)
         (work) 
         );

FC_REFLECT( node::protocol::proof_of_work_input,
         (miner_account)
         (prev_block)
         (nonce) 
         );

FC_REFLECT( node::protocol::equihash_proof_of_work,
         (input)
         (proof)
         (prev_block)
         (work) 
         );

FC_REFLECT_TYPENAME( node::protocol::proof_of_work_type )

FC_REFLECT( node::protocol::proof_of_work_operation,
         (work)
         (new_owner_key)
         );

FC_REFLECT( node::protocol::verify_block_operation,
         (signatory)
         (producer)
         (block_id)
         );

FC_REFLECT( node::protocol::commit_block_operation,
         (signatory)
         (producer)
         (block_id)
         (verifications)
         (commitment_stake)
         );

FC_REFLECT( node::protocol::producer_violation_operation,
         (signatory)
         (reporter)
         (first_trx)
         (second_trx)
         );


   //===========================//
   //==== Custom Operations ====//
   //===========================//


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