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
      account_name_type             registrar;                              ///< Account registering the new account, usually an interface.

      account_name_type             new_account_name;                       ///< The name of the new account.

      account_name_type             referrer;                               ///< The account that lead to the creation of the new account, by way of referral link.

      account_name_type             proxy;                                  ///< Account that the new account will delegate its voting power to.

      account_name_type             recovery_account;                       ///< Account that can execute a recovery operation, in the event that the owner key is compromised. 

      account_name_type             reset_account;                          ///< Account that has the ability to execute a reset operation after 60 days of inactivity.

      string                        details;                                ///< The account's Public details string. Profile Biography.

      string                        url;                                    ///< The account's Public personal URL.

      string                        profile_image;                          ///< IPFS Reference of the Public profile image of the account.

      string                        cover_image;                            ///< IPFS Reference of the Public cover image of the account.

      string                        json;                                   ///< The JSON string of additional Public profile information.

      string                        json_private;                           ///< The JSON string of additional encrypted profile information. Encrypted with connection key.

      string                        first_name;                             ///< Encrypted First name of the user. Encrypted with connection key.

      string                        last_name;                              ///< Encrypted Last name of the user. Encrypted with connection key.

      string                        gender;                                 ///< Encrypted Gender of the user. Encrypted with connection key.

      string                        date_of_birth;                          ///< Encrypted Date of birth of the user. Format: DD-MM-YYYY. Encrypted with connection key.

      string                        email;                                  ///< Encrypted Email address of the user. Encrypted with connection key.

      string                        phone;                                  ///< Encrypted Phone Number of the user. Encrypted with connection key.

      string                        nationality;                            ///< Encrypted Country of user's residence. Encrypted with connection key.

      string                        relationship;                           ///< Encrypted Relationship status of the account. Encrypted with connection key.

      string                        political_alignment;                    ///< Encrypted Political alignment. Encrypted with connection key.

      vector< tag_name_type >       interests;                              ///< Set of tags of the interests of the user.

      authority                     owner_auth;                             ///< The account authority required for changing the active and posting authorities.

      authority                     active_auth;                            ///< The account authority required for sending payments and trading.

      authority                     posting_auth;                           ///< The account authority required for posting content and voting.

      string                        secure_public_key;                      ///< The secure encryption key for content only visible to this account.

      string                        connection_public_key;                  ///< The connection public key used for encrypting Connection level content.

      string                        friend_public_key;                      ///< The connection public key used for encrypting Friend level content.

      string                        companion_public_key;                   ///< The connection public key used for encrypting Companion level content.

      asset                         fee;                                    ///< Account creation fee for stake on the new account.

      asset                         delegation;                             ///< Initial amount delegated to the new account.

      void                          validate()const;
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( registrar ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( registrar ); }
   };


   /**
    * Updates the details and authorities of an account.
    */
   struct account_update_operation : public base_operation
   {
      account_name_type             account;                       ///< Name of the account to update.

      string                        details;                       ///< The account's Public details string. Profile Biography.

      string                        url;                           ///< The account's Public personal URL.

      string                        profile_image;                 ///< IPFS Reference of the Public profile image of the account.

      string                        cover_image;                   ///< IPFS Reference of the Public cover image of the account.

      string                        json;                          ///< The JSON string of additional Public profile information.

      string                        json_private;                  ///< The JSON string of additional encrypted profile information. Encrypted with connection key.

      string                        first_name;                    ///< Encrypted First name of the user. Encrypted with connection key.

      string                        last_name;                     ///< Encrypted Last name of the user. Encrypted with connection key.

      string                        gender;                        ///< Encrypted Gender of the user. Encrypted with connection key.

      string                        date_of_birth;                 ///< Encrypted Date of birth of the user. Format: DD-MM-YYYY. Encrypted with connection key.

      string                        email;                         ///< Encrypted Email address of the user. Encrypted with connection key.

      string                        phone;                         ///< Encrypted Phone Number of the user. Encrypted with connection key.

      string                        nationality;                   ///< Encrypted Country of user's residence. Encrypted with connection key.

      string                        relationship;                  ///< Encrypted Relationship status of the account. Encrypted with connection key.

      string                        political_alignment;           ///< Encrypted Political alignment. Encrypted with connection key.
      
      string                        pinned_permlink;               ///< Post permlink pinned to the top of the account's profile.

      vector< tag_name_type >       interests;                     ///< Set of tags of the interests of the user.

      authority                     owner_auth;                    ///< Creates a new owner authority for the account, changing the key and account auths required to sign transactions.

      authority                     active_auth;                   ///< Creates a new active authority for the account, changing the key and account auths required to sign transactions.

      authority                     posting_auth;                  ///< Creates a new posting authority for the account, changing the key and account auths required to sign transactions.

      string                        secure_public_key;             ///< The secure encryption key for content only visible to this account.

      string                        connection_public_key;         ///< The connection public key used for encrypting Connection level content.

      string                        friend_public_key;             ///< The connection public key used for encrypting Friend level content.

      string                        companion_public_key;          ///< The connection public key used for encrypting Companion level content.

      bool                          active = true;                 ///< True when account is active. False to set account as inactive.

      void                          validate()const;
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( account ); }
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
      account_name_type             verifier_account;      ///< Name of the Account with the profile.

      account_name_type             verified_account;      ///< Name of the account being verifed.

      string                        shared_image;          ///< IPFS reference to an image containing both people and a recent head block id.

      bool                          verified = true;       ///< True to verify the account, false to remove verification.

      void                          validate()const;
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( verifier_account ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( verifier_account ); }
   };


   /**
    * Accounts can purchase a membership subscription for their account to gain protocol benefits:
    * 
    * Top Level Membership:
    * Price $100.00 per month.

      • Governance account, and Interface account revenue activation.
      • Access to WeYouMe Pro Suite for enterprise.
      • WYM Stakeholder Dividend boosted by 10%.
      • Access to the WeYouMe FlashChain for interface management.
      • 30% Voting power boost.
      • Activity pool reward share boosted by 100%.
      • 20% Content reward boost.

      Mezzanine Membership:
      Price: $25.00 per month.

      • Posts eligible for WeYouMe Featured page.
      • 20% Voting power boost.
      • Activity pool reward share boosted by 50%.
      • 10% Content reward boost.
      • 10% refund on marketplace fees, premium content, subscription content, and exchange trading fees.

      Standard Membership:
      Price: $10.00 per month.

      • Disable all Advertising.
      • 10% Voting power boost.
      • Ability to join the Developer, Advocate and Marketing pools.
      • Activity pool reward share boosted by 50%.
    */
   struct account_membership_operation : public base_operation
   {
      account_name_type             account;              ///< The name of the account to activate membership on.

      string                        membership_type;      ///< The level of membership to activate on the account.

      uint16_t                      months;               ///< Number of months to purchase membership for.

      account_name_type             interface;            ///< Name of the interface application facilitating the transaction.

      bool                          recurring = true;     ///< True for membership to automatically recur each month from liquid balance.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Blacklists an account or asset.
    */
   struct account_update_list_operation : public base_operation
   {
      account_name_type                 account;              ///< Name of account.

      optional< account_name_type >     listed_account;       ///< Name of account being added to a black or white list.

      optional< asset_symbol_type >     listed_asset;         ///< Name of asset being added to a black or white list.

      bool                              blacklisted = true;   ///< True to add to blacklist, false to remove.

      bool                              whitelisted = false;  ///< True to add to whitelist, false to remove.

      void                              validate() const;
      void                              get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type             account;           ///< Account creating the vote.

      uint16_t                      vote_rank = 1;     ///< Rank ordering of the vote.

      account_name_type             producer;          ///< producer being voted for.

      bool                          approved = true;   ///< True to create vote, false to remove vote.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type             account;      ///< The name of the account to update.

      account_name_type             proxy;        ///< The name of account that should proxy to, or empty string to have no proxy.

      void                          validate()const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
   struct account_request_recovery_operation : public base_operation
   {
      account_name_type             recovery_account;       ///< The recovery account is listed as the recovery account on the account to recover.

      account_name_type             account_to_recover;     ///< The account to recover. This is likely due to a compromised owner authority.

      authority                     new_owner_authority;    ///< The new owner authority the account to recover wishes to have.

      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( recovery_account ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( recovery_account ); }
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
   struct account_recover_operation : public base_operation
   {
      account_name_type             account_to_recover;        ///< The account to be recovered

      authority                     new_owner_authority;       ///< The new owner authority as specified in the request account recovery operation.

      authority                     recent_owner_authority;    ///< A previous owner authority that the account holder will use to prove past ownership of the account to be recovered.

      void                          get_required_authorities( vector< authority >& a )const { a.push_back( new_owner_authority ); a.push_back( recent_owner_authority ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const { a.insert( account_to_recover ); }
      void                          validate() const;
   };


   /**
    * Allows reset_account to change account_to_reset's owner authority.
    * 
    * Enabled after 7 days of inactivity.
    */
   struct account_reset_operation : public base_operation 
   {
      account_name_type             reset_account;          ///< The account that is initiating the reset process.

      account_name_type             account_to_reset;       ///< The Account to be reset.

      authority                     new_owner_authority;    ///< A recent owner authority on your account.

      void                          get_required_active_authorities( flat_set<account_name_type>& a )const { a.insert( reset_account ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( reset_account ); }
      void                          validate()const;
   };


   /**
    * Allows account owner to control which account has the power to reset.
    * 
    * This is done using the account_reset_operation after a specified duration of days.
    */
   struct account_reset_update_operation : public base_operation 
   {
      account_name_type             account;             ///< Account to update.

      account_name_type             new_reset_account;   ///< Account that has the new authority to reset the account.

      uint16_t                      days = 7;            ///< Days of inactivity required to reset the account. 

      void                          validate()const;
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( account ); }
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
    * request is created. 
    * 
    * The effective recovery account of an account with no listed recovery account 
    * can change at any time asproducer vote weights.
    * The top voted producer is explicitly the most trusted producer according to stake.
    */
   struct account_recovery_update_operation : public base_operation
   {
      account_name_type             account_to_recover;     ///< The account being updated.

      account_name_type             new_recovery_account;   ///< The account that is authorized to create recovery requests.

      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( account_to_recover ); }
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
   struct account_decline_voting_operation : public base_operation
   {
      account_name_type             account;             ///< The account that is declining voting rights

      bool                          declined = true;     ///< True to decine voting rights, false to cancel pending request.

      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          validate() const;
   };


   /**
    * Creates or updates a connection between two accounts.
    * 
    * Used for the purposes of exchanging encrypted 
    * connection keys that can be used to decrypt
    * private posts made by the account, and enables the creation of private 
    * message transactions between the two accounts. 
    * 
    * Initiates the 2 step connection exchange process:
    * 
    * 1 - Alice sends Connection with encrypted private key.
    * 2 - Bob confirms and returns a connection txn with encrypted private key.
    * 
    * Encrypted key should be decryptable by the recipient's private secure key.
    * 
    * This operation assumes that the account includes 
    * a valid encrypted private key and that the initial account 
    * confirms by returning an additional acceptance transaction.
    */
   struct account_connection_operation : public base_operation
   {
      account_name_type             account;               ///< Account accepting the request.

      account_name_type             connecting_account;    ///< Account that should be connected to.

      string                        connection_id;         ///< uuidv4 for the connection, for local storage of decryption key.

      string                        connection_type;       ///< Type of connection level.

      string                        message;               ///< Message attached to the connection, encrypted with recipient's secure public key.

      string                        json;                  ///< JSON metadata attached to the connection, encrypted with recipient's secure public key.

      string                        encrypted_key;         ///< The encrypted private connection/friend/companion key of the user.

      bool                          connected = true;      ///< Set true to connect, false to delete connection.

      void                          validate()const;
      void                          get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Enables an account to follow another account.
    *  
    * Adds follower to the account's account_following_object and displays
    * posts and shares from the account in their home feed.
    */
   struct account_follow_operation : public base_operation
   {
      account_name_type             follower;             ///< Account that is creating the new follow relationship.

      account_name_type             following;            ///< Account that is being followed by follower.

      account_name_type             interface;            ///< Interface account that was used to broadcast the transaction. 

      bool                          added = true;         ///< Set true to add to list, false to remove from list.

      bool                          followed = true;      ///< Set true to follow, false to filter.

      void                          validate()const;
      void                          get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( follower ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( follower ); }
   };


   /**
    * Enables an account to follow a tag.
    * 
    * Includes the tag in the accounts following object, and displays
    * posts and shares from posts that use the tag in their feeds.
    */
   struct account_follow_tag_operation : public base_operation
   {
      account_name_type             follower;           ///< Name of the account following the tag.

      tag_name_type                 tag;                ///< Tag being followed.

      account_name_type             interface;          ///< Name of the interface account that was used to broadcast the transaction. 

      bool                          added = true;       ///< Set true to add to list, false to remove from list.

      bool                          followed = true;    ///< Set true to follow, false to filter.

      void                          validate()const;
      void                          get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( follower ); }
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
    * 2 - A recent comment transaction with at least 10% of the median number of votes and views, and vote and view power on all posts in the last 30 days.
    * 3 - A recent vote transaction.
    * 4 - A recent view transaction.
    * 5 - At least 10 producer votes.
    */
   struct account_activity_operation : public base_operation
   {
      account_name_type             account;        ///< Name of the account claiming the reward.

      string                        permlink;       ///< Permlink of the users recent qualifying post in the last 24h.

      account_name_type             interface;      ///< Name of the interface account that was used to broadcast the transaction.

      void                          validate()const;
      void                          get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   //=============================//
   //==== Business Operations ====//
   //=============================//


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

      uint16_t                        enterprise_fund_reward_percent = COMMUNITY_FUND_REWARD_PERCENT;  ///< Percentage of reward paid to community fund proposals.

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

      date_type                       maturity_date = date_type( 1, 1, 1970 );                           ///< Date at which the bond will mature. Principle value will be automatically paid from business.

      // === Stimulus Asset Options === //

      asset_symbol_type               redemption_asset = SYMBOL_USD;                                     ///< Symbol of the asset that can be redeemed in exchange the stimulus asset.

      price                           redemption_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ),asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) ); ///< Price at which the stimulus asset is redeemed. Redemption asset is base.
      
      vector< account_name_type >     distribution_list;                                                 ///< List of accounts that receive an equal balance of the stimulus asset.

      vector< account_name_type >     redemption_list;                                                   ///< List of accounts that can receive and redeem the stimulus asset.

      asset                           distribution_amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_USD );   ///< Amount of stimulus asset distributed each interval.

      void validate()const;
   };


   /**
    * Creates a Business Entity Integrated Structure.
    * 
    * A Business structure is comprised of:
    * 
    * 1 - An Underlying Account
    * 2 - An Equity Asset
    * 3 - A Credit Asset
    * 4 - A Public Community
    * 5 - A Private community
    * 
    * Contains a heirarchial structure of executives and directors that can be used for managing an enterprise of multiple
    * accounts, and giving individuals delegated control over account transaction signing authority.
    * 
    * Equity Holders vote for Directors.
    * Directors vote for the Chief Executive.
    * Chief Executive Appoints Executives.
    * All Executives can sign business account transactions.
    * 
    * Business underlying accounts have no key signing authorities, and only use account authorities.
    * The account authorities are updated to the current set of executives, as appointed by the Chief Executive.
    * Community and asset ownership cannot be transferred away from the business underlying account.
    */
   struct business_create_operation : public base_operation
   {
      account_name_type             founder;                                ///< Account registering the new business.

      account_name_type             new_business_name;                      ///< The name of the new business underlying account.

      string                        new_business_trading_name;              ///< Trading business name, UPPERCASE letters only.

      string                        details;                                ///< The account's Public details string. Profile Biography.

      string                        url;                                    ///< The account's Public URL.

      string                        profile_image;                          ///< IPFS Reference of the Public profile image of the account.

      string                        cover_image;                            ///< IPFS Reference of the Public cover image of the account.

      string                        secure_public_key;                      ///< The secure encryption key for content only visible to this account.

      string                        connection_public_key;                  ///< The connection public key used for encrypting Connection level content.

      string                        friend_public_key;                      ///< The connection public key used for encrypting Friend level content.

      string                        companion_public_key;                   ///< The connection public key used for encrypting Companion level content.

      account_name_type             interface;                              ///< Account of the interface that broadcasted the transaction.

      asset_symbol_type             equity_asset;                           ///< Equity assets that offer dividends and voting power over the Business Account's structure.

      uint16_t                      equity_revenue_share;                   ///< Equity asset that the account distributes a percentage of incoming revenue to as dividend payments.

      asset_options                 equity_options;                         ///< Series of options parameters that apply to the Equity Asset.
      
      asset_symbol_type             credit_asset;                           ///< Credit asset that offer interest and buybacks from the business account.

      uint16_t                      credit_revenue_share;                   ///< Credit asset that the account uses a percentage of incoming revenue to repurchase at face value.

      asset_options                 credit_options;                         ///< Series of options parameters that apply to the Credit Asset.
      
      community_name_type           public_community;                       ///< Name of the business public community.

      string                        public_display_name;                    ///< Display name of the business public community.

      string                        public_community_member_key;            ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.

      string                        public_community_moderator_key;         ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.

      string                        public_community_admin_key;             ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.

      string                        public_community_secure_key;            ///< Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.

      string                        public_community_standard_premium_key;  ///< Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.

      string                        public_community_mid_premium_key;       ///< Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.

      string                        public_community_top_premium_key;       ///< Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.
      
      community_name_type           private_community;                      ///< Name of the business private community. Should be randomized string.
      
      string                        private_display_name;                   ///< Display name of the business private community.
      
      string                        private_community_member_key;           ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.

      string                        private_community_moderator_key;        ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.

      string                        private_community_admin_key;            ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.

      string                        private_community_secure_key;           ///< Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.

      string                        private_community_standard_premium_key; ///< Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.

      string                        private_community_mid_premium_key;      ///< Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.

      string                        private_community_top_premium_key;      ///< Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.

      asset_symbol_type             reward_currency = SYMBOL_COIN;          ///< The Currency asset used for content rewards in the community.

      asset                         standard_membership_price;              ///< Price paid per month by community standard members.

      asset                         mid_membership_price;                   ///< Price paid per month by all mid level community members.

      asset                         top_membership_price;                   ///< Price paid per month by all top level community members.
      
      asset                         coin_liquidity;                         ///< Amount of COIN asset to inject into the Coin liquidity pool.

      asset                         usd_liquidity;                          ///< Amount of USD asset to inject into the USD liquidity pool.

      asset                         credit_liquidity;                       ///< Amount of the new asset to issue and inject into the credit pool.

      asset                         fee;                                    ///< Account creation fee for stake on the new account.

      asset                         delegation;                             ///< Initial amount delegated to the new account.

      void                          validate()const;
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( founder ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( founder ); }
   };


   /**
    * Updates the Business Structure of an account.
    * 
    * Can be updated by the Chief Executive Officer of the business account,
    * once they have been elected by the directors of the business.
    */
   struct business_update_operation : public base_operation
   {
      account_name_type                  chief_executive;                 ///< Name of the Chief Executive Officer of the Business.

      account_name_type                  business;                        ///< Name of the Business Account to update.

      string                             business_trading_name;           ///< Trading business name, UPPERCASE letters only.

      uint16_t                           equity_revenue_share;            ///< Equity asset that the account distributes a percentage of incoming revenue to as dividend payments.

      uint16_t                           credit_revenue_share;            ///< Credit asset that the account uses a percentage of incoming revenue to repurchase at face value.

      vector< account_name_type >        executives;                      ///< Set of all Executive accounts appointed by the Chief Executive Officer.

      bool                               active = true;                   ///< True when the business account is active, false to dissolve Business Account.

      void                               validate()const;
      void                               get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( chief_executive ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( chief_executive ); }
   };


   /**
    * Create an executive of a business account.
    * 
    * Executives can be appointed by the Chief Executive to be a signing account authority for the business.
    */
   struct business_executive_operation : public base_operation
   {
      account_name_type             executive;             ///< Name of Executive Account.

      account_name_type             business;              ///< Business account that the executive is being created for.

      bool                          active = true;         ///< True to activate Executive, false to deactivate.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( executive ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( executive ); }
   };


   /**
    * Votes for an account to become an executive of a business account.
    * 
    * Weighted according to equity holdings.
    */
   struct business_executive_vote_operation : public base_operation
   {
      account_name_type             director;              ///< Director Account voting for the Executive.

      account_name_type             executive;             ///< Name of executive being voted for.

      account_name_type             business;              ///< Business account that the Executive is being voted for.

      bool                          approved = true;       ///< True to add, false to remove.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( director ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( director ); }
   };


   /**
    * Create a director of a business account.
    * 
    * Directors can be voted by the equity holders of the business account.
    * Directors then vote for the chief executive.
    */
   struct business_director_operation : public base_operation
   {
      account_name_type             director;              ///< Name of the director being created.

      account_name_type             business;              ///< Business account that the director is being created for.

      bool                          active = true;         ///< True to add, false to remove.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( director ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( director ); }
   };


   /**
    * Votes for an account to become a director of a business account.
    * 
    * Weighted according to equity holdings.
    */
   struct business_director_vote_operation : public base_operation
   {
      account_name_type             account;                    ///< Name of the voting account.

      account_name_type             director;                   ///< Name of the business director being voted for.

      account_name_type             business;                   ///< Business account of the director.

      uint16_t                      vote_rank = 1;              ///< Rank of voting preference.

      bool                          approved = true;            ///< True to add, false to remove.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   //===============================//
   //==== Governance Operations ====//
   //===============================//


   /**
    * Creates a Governance Entity Integrated Structure.
    * 
    * A Governance structure is comprised of:
    * 
    * 1 - An Underlying Account
    * 2 - An Equity Asset
    * 3 - A Credit Asset
    * 4 - A Public Community
    * 5 - A Private community
    * 
    * Contains a heirarchial structure of executives and directors that can be used for managing a 
    * Governance system for content moderation and fee share revenue aggregaton and public goods funding, 
    * and gives individual accounts delegated control over account transaction signing authority.
    * 
    * Governance Members vote for Directors.
    * Directors vote for the Chief Executive.
    * Chief Executive Appoints Executives.
    * All Executives can sign Governance account transactions.
    * 
    * Governance underlying accounts have no key signing authorities, and only use account authorities.
    * The account authorities are updated to the current set of Executives, as appointed by the Chief Executive.
    * Community and asset ownership cannot be transferred away from the Governance underlying account.
    */
   struct governance_create_operation : public base_operation
   {
      account_name_type             founder;                                ///< Account registering the new Governance Account.

      account_name_type             new_governance_name;                    ///< The name of the new governance underlying account.

      string                        new_governance_display_name;            ///< Trading governance name, UPPERCASE letters only.

      string                        details;                                ///< The account's Public details string. Profile Biography.

      string                        url;                                    ///< The account's Public URL.

      string                        profile_image;                          ///< IPFS Reference of the Public profile image of the account.

      string                        cover_image;                            ///< IPFS Reference of the Public cover image of the account.

      string                        secure_public_key;                      ///< The secure encryption key for content only visible to this account.

      string                        connection_public_key;                  ///< The connection public key used for encrypting Connection level content.

      string                        friend_public_key;                      ///< The connection public key used for encrypting Friend level content.

      string                        companion_public_key;                   ///< The connection public key used for encrypting Companion level content.

      account_name_type             interface;                              ///< Account of the interface that broadcasted the transaction.

      asset_symbol_type             equity_asset;                           ///< Equity assets that offer dividends and voting power over the governance Account's structure.

      uint16_t                      equity_revenue_share;                   ///< Equity asset that the account distributes a percentage of incoming revenue to as dividend payments.

      asset_options                 equity_options;                         ///< Series of options parameters that apply to the Equity Asset.
      
      asset_symbol_type             credit_asset;                           ///< Credit assets that offer interest and buybacks from the governance account.

      uint16_t                      credit_revenue_share;                   ///< Credit asset that the account uses a percentage of incoming revenue to repurchase at face value.

      asset_options                 credit_options;                         ///< Series of options parameters that apply to the Credit Asset.
      
      community_name_type           public_community;                       ///< Name of the governance public community.

      string                        public_display_name;                    ///< Display name of the governance public community.

      string                        public_community_member_key;            ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.

      string                        public_community_moderator_key;         ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.

      string                        public_community_admin_key;             ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.

      string                        public_community_secure_key;            ///< Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.

      string                        public_community_standard_premium_key;  ///< Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.

      string                        public_community_mid_premium_key;       ///< Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.

      string                        public_community_top_premium_key;       ///< Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.
      
      community_name_type           private_community;                      ///< Name of the governance private community. Should be randomized string.
      
      string                        private_display_name;                   ///< Display name of the governance private community.
      
      string                        private_community_member_key;           ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.

      string                        private_community_moderator_key;        ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.

      string                        private_community_admin_key;            ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.

      string                        private_community_secure_key;           ///< Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.

      string                        private_community_standard_premium_key; ///< Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.

      string                        private_community_mid_premium_key;      ///< Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.

      string                        private_community_top_premium_key;      ///< Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.

      asset_symbol_type             reward_currency = SYMBOL_COIN;          ///< The Currency asset used for content rewards in the community.

      asset                         standard_membership_price;              ///< Price paid per month by community standard members.

      asset                         mid_membership_price;                   ///< Price paid per month by all mid level community members.

      asset                         top_membership_price;                   ///< Price paid per month by all top level community members.
      
      asset                         coin_liquidity;                         ///< Amount of COIN asset to inject into the Coin liquidity pool.

      asset                         usd_liquidity;                          ///< Amount of USD asset to inject into the USD liquidity pool.

      asset                         credit_liquidity;                       ///< Amount of the new asset to issue and inject into the credit pool.

      asset                         fee;                                    ///< Account creation fee for stake on the new account.

      asset                         delegation;                             ///< Initial amount delegated to the new account.

      void                          validate()const;
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( founder ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( founder ); }
   };


   /**
    * Updates the Governance Structure of an account.
    * 
    * Can be updated by the Chief Executive Officer of the governance account,
    * once they have been elected by the directors of the governance.
    */
   struct governance_update_operation : public base_operation
   {
      account_name_type                  chief_executive;                 ///< Name of the Chief Executive Officer of the Governance Account.

      account_name_type                  governance;                      ///< Name of the Governance Account to update.

      string                             governance_display_name;         ///< Non-consensus Governance Account display name, UPPERCASE letters only.

      uint16_t                           equity_revenue_share;            ///< Equity asset that the account distributes a percentage of incoming revenue to as dividend payments.

      uint16_t                           credit_revenue_share;            ///< Credit asset that the account uses a percentage of incoming revenue to repurchase at face value.

      vector< account_name_type >        executives;                      ///< Set of all Executive accounts appointed by the Chief Executive Officer.

      bool                               active = true;                   ///< True when the governance account is active, false to dissolve governance account.

      void                               validate()const;
      void                               get_required_owner_authorities( flat_set<account_name_type>& a )const { a.insert( chief_executive ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( chief_executive ); }
   };


   /**
    * Create an executive of a governance account.
    * 
    * Executives can be appointed by the Chief Executive to be a signing account authority for the Governance account.
    */
   struct governance_executive_operation : public base_operation
   {
      account_name_type             executive;             ///< Name of Executive Account.

      account_name_type             governance;            ///< Governance account that the executive is being voted for.

      bool                          active = true;         ///< True to activate Executive, false to deactivate.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( executive ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( executive ); }
   };


   /**
    * Votes for an account to become an executive of a governance account.
    * 
    * Weighted according to equity holdings.
    */
   struct governance_executive_vote_operation : public base_operation
   {
      account_name_type             director;              ///< Director Account voting for the Executive.

      account_name_type             executive;             ///< Name of executive being voted for.

      account_name_type             governance;            ///< Governance account that the Executive is being voted for.

      bool                          approved = true;       ///< True to add, false to remove.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( director ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( director ); }
   };


   /**
    * Create a director of a governance account.
    * 
    * Directors can be voted by the equity holders of the governance account.
    * Directors then vote for the chief executive.
    */
   struct governance_director_operation : public base_operation
   {
      account_name_type             director;              ///< Name of director being created.

      account_name_type             governance;            ///< Governance account.

      bool                          active = true;         ///< True to add, false to remove.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( director ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( director ); }
   };


   /**
    * Votes for an account to become a director of a governance account, 
    * weighted according to equity holdings.
    */
   struct governance_director_vote_operation : public base_operation
   {
      account_name_type             account;                    ///< Name of the voting account.

      account_name_type             director;                   ///< Name of the governance director being voted for.

      account_name_type             governance;                 ///< Governance account of the director.

      uint16_t                      vote_rank = 1;              ///< Rank of voting preference.

      bool                          approved = true;            ///< True to add, false to remove.

      void                          validate() const;
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /** 
    * Adds an account to the membership set of a Governance account. 
    * 
    * This causes its content moderation tags to 
    * apply to posts requested by the account in interfaces.
    * 
    * This allows an account to opt in to the moderation 
    * and enforcement policies provided by the governance account, 
    * and to its default settings for whitelisting 
    * and blacklisting mediators, assets,
    * communities, authors, and interfaces.
    * 
    * Members of Governance accounts can vote for the directors of the account, 
    * which determine the chief executive of the account.
    * Accounts can only be a member of one Governance account at a time, 
    * and each member must be approved by the Governance account.
    * 
    * Accounts must request to join a Governance Account before being added.
    */
   struct governance_member_operation : public base_operation
   {
      account_name_type              governance;               ///< The name of the governance account creating the membership.
      
      account_name_type              account;                  ///< The account being added as a participating member of to the governance address.
      
      account_name_type              interface;                ///< Account of the interface signing the transaction.

      bool                           approved = true;          ///< True to approve the membership, false to remove.

      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( governance ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( governance ); }
      void                           validate() const;
   };


   /** 
    * Creates a request to join a Governance account as a new member.
    * 
    * Members of Governance accounts can vote for the directors of the account, 
    * which determine the chief executive of the account.
    * 
    * Accounts can only be a member of one Governance account at a time, 
    * and each member must be approved by the Governance account.
    * 
    * Once the request is accepted, any previous Governance account 
    * membership is recinded, and votes are revoked.
    */
   struct governance_member_request_operation : public base_operation
   {
      account_name_type              account;                  ///< The account creating the request.
      
      account_name_type              governance;               ///< The name of the governance account that membership is requested for.
      
      account_name_type              interface;                ///< Account of the interface signing the transaction.

      string                         message;                  ///< Encrypted message to the Governance Account team, encrypted with Governance connection key.

      bool                           active = true;            ///< True to create request, false to cancel request.

      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           validate() const;
   };


   /**
    * Creates a Resolution for a Governance Account.
    * 
    * Members can then vote for the resolution.
    * 
    * The Resolution is passed if more than 51% of members vote to approve it, 
    * and rejected if more than 51% of members disapprove it.
    */
   struct governance_resolution_operation : public base_operation
   {
      account_name_type              governance;             ///< The name of the governance account creating the Resolution.

      string                         resolution_id;          ///< uuidv4 referring to the resolution.

      string                         ammendment_id;          ///< uuidv4 referring to the resolution ammendment.

      string                         title;                  ///< Title of the resolution to be voted on.

      string                         details;                ///< Short description text of purpose and summary of the resolution to be voted on.

      string                         body;                   ///< Full Text of the body of the resolution to be voted on.

      string                         url;                    ///< The Resolution description URL explaining more details.

      string                         json;                   ///< JSON metadata of the resolution.

      account_name_type              interface;              ///< Account of the interface that most recently updated the resolution.

      time_point                     completion_time;        ///< Time the resolution completes.

      bool                           active = true;          ///< True to create the resolution, false to remove the ammendment if it is not passed.

      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( governance ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( governance ); }
      void                           validate() const;
   };


   /**
    * Creates a Vote for a Governance Account Resolution.
    * 
    * The Resolution is passed if more than 51% of members vote to approve it, 
    * and rejected if more than 51% of members disapprove it.
    */
   struct governance_resolution_vote_operation : public base_operation
   {
      account_name_type              account;                ///< The name of the account voting on the resolution.

      account_name_type              governance;             ///< The name of the governance account that created the Resolution.

      string                         resolution_id;          ///< uuidv4 referring to the resolution.

      string                         ammendment_id;          ///< uuidv4 referring to the resolution ammendment.

      account_name_type              interface;              ///< Account of the interface that most recently updated the resolution vote.

      bool                           approved = true;        ///< True to approve the resolution, false to disapprove resolution.

      bool                           active = true;          ///< True to activate a vote or update existing vote, false to retract existing vote.

      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           validate() const;
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
   struct network_officer_update_operation : public base_operation
   {
      account_name_type             account;           ///< Name of the member's account.

      string                        officer_type;      ///< The type of network officer that the account serves as. 

      string                        details;           ///< Information about the network officer and their work.

      string                        url;               ///< The officers's description URL explaining their details. 

      string                        json;              ///< Additional information about the officer.

      asset_symbol_type             reward_currency;   ///< Symbol of the currency asset that the network officer requests.

      bool                          active = true;     ///< Set true to activate the officer, false to deactivate. 
         
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type              account;               ///< The name of the account voting for the officer.

      account_name_type              network_officer;       ///< The name of the network officer being voted for.

      uint16_t                       vote_rank = 1;         ///< Number of vote rank ordering.

      bool                           approved = true;       ///< True if approving, false if removing vote.

      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
   struct supernode_update_operation : public base_operation
   {
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
         
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
   struct interface_update_operation : public base_operation
   {
      account_name_type              account;           ///< Name of the member's account.

      string                         details;           ///< Information about the interface, and what they are offering to users.

      string                         url;               ///< The interfaces's URL.

      string                         json;              ///< Additional information about the interface.

      bool                           active = true;     ///< Set true to activate the interface, false to deactivate. 
         
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
   struct mediator_update_operation : public base_operation
   {
      account_name_type              account;           ///< Name of the member's account.

      string                         details;           ///< Information about the mediator, and what they are offering to users

      string                         url;               ///< The mediator's reference URL.

      string                         json;              ///< Additional information about the mediator.

      asset                          mediator_bond;     ///< Amount of Core asset to stake in the mediation pool. 

      bool                           active = true;     ///< Set true to activate the mediator, false to deactivate. 
         
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           validate() const;
   };


   /**
    * Creates or updates an enterprise proposal.
    * 
    * Provides funding to enterprise proposals that work to benefit the network by
    * distributing payments based on quadratic voting support, and quadratic funding.
    * 
    * An Enterprise Proposal becomes active when it is supported by a 
    * minimum amount of voters, including producers, and receives a 
    * minimum amount of funding proportionally to the budget.
    * 
    * To begin recieving funding from the enterprise funding pool, proposals need to be approved by:
    * 
    * - At least 5 of the Top 50 producers, with a combined voting power of at least 1% of the total producer voting power.
    * - At least 20 total votes, from accounts with a total combined voting power of at least 1% of total voting power.
    * - At least 1% of the enterprise budget in direct funding.
    * - At least 5 total direct funding accounts.
    */
   struct enterprise_update_operation : public base_operation
   {
      account_name_type              account;                  ///< The name of the account that created the enterprise proposal.

      string                         enterprise_id;            ///< UUIDv4 referring to the enterprise proposal.

      string                         details;                  ///< The enterprise proposals's details description.

      string                         url;                      ///< The enterprise proposals's reference URL.

      string                         json;                     ///< JSON metadata of the enterprise proposal.

      asset                          budget;                   ///< Total amount requested for enterprise funding. Enterprise is completed when budget is received.

      bool                           active = true;            ///< True to set the proposal to activate, false to deactivate an existing proposal and delay funding. 
         
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           validate() const;
   };


   /**
    * Votes for an enterprise proposal to receive funding.
    * 
    * 50% of enterprise pool funding is distributed to the proposals 
    * based on the total voting power directed to them.
    */
   struct enterprise_vote_operation : public base_operation
   {
      account_name_type              voter;                       ///< Account voting for the enterprise project proposal.

      account_name_type              account;                     ///< The name of the account that created the proposal.

      string                         enterprise_id;               ///< UUIDv4 referring to the proposal.

      int16_t                        vote_rank = 1;               ///< The rank of the approval for enterprise proposals.

      bool                           approved = true;             ///< True to approve the milestone claim, false to remove approval.

      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( voter ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( voter ); }
      void                           validate() const;
   };


   /**
    * Directly sends funds to an enterprise proposal.
    * 
    * 50% of enterprise pool funding is distributed to the projects 
    * based on the total amount sent in direct funding by accounts.
    */
   struct enterprise_fund_operation : public base_operation
   {
      account_name_type              funder;                     ///< The name of the account sending the funding.

      account_name_type              account;                    ///< The name of the account that created the proposal.

      string                         enterprise_id;              ///< UUIDv4 referring to the proposal.

      asset                          amount;                     ///< Amount of funding that is to be paid to the enterprise proposal.

      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( funder ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( funder ); }
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
    * Contains all the parameters for calculating the content reward distribution for a post.
    * 
    * Snapshots the reward curve conditions at the time the post is 
    * created for consistent calculation for duration of reward decay rate.
    */
   struct comment_reward_curve
   {
      uint128_t                         constant_factor = CONTENT_CONSTANT;                         ///< Constant added to reward value for reward curve calculation.

      uint16_t                          sqrt_percent = 0;                                           ///< Weighting percentage of the Square Root component of the reward curve.

      uint16_t                          linear_percent = 0;                                         ///< Weighting percentage of the Linear component of the reward curve.

      uint16_t                          semi_quadratic_percent = PERCENT_100;                       ///< Weighting percentage of the Semi-Quadratic component of the reward curve.

      uint16_t                          quadratic_percent = 0;                                      ///< Weighting percentage of the Quadratic component of the reward curve.

      uint16_t                          reward_interval_amount = CONTENT_REWARD_INTERVAL_AMOUNT;    ///< Number of reward intervals for distribution.

      uint16_t                          reward_interval_hours = CONTENT_REWARD_INTERVAL_HOURS;      ///< Number of hours for each content reward interval.

      uint16_t                          author_reward_percent = AUTHOR_REWARD_PERCENT;              ///< Percentage of Content rewards that are paid to the Author.

      uint16_t                          vote_reward_percent = VOTE_REWARD_PERCENT;                  ///< Percentage of Content rewards that are paid to all Voters.

      uint16_t                          view_reward_percent = VIEW_REWARD_PERCENT;                  ///< Percentage of Content rewards that are paid to all Viewers.

      uint16_t                          share_reward_percent = SHARE_REWARD_PERCENT;                ///< Percentage of Content rewards that are paid to all Sharers.

      uint16_t                          comment_reward_percent = COMMENT_REWARD_PERCENT;            ///< Percentage of Content rewards that are paid to all Commenters.

      uint16_t                          storage_reward_percent = STORAGE_REWARD_PERCENT;            ///< Percentage of Content rewards that are paid to Viewer Supernodes.

      uint16_t                          moderator_reward_percent = MODERATOR_REWARD_PERCENT;        ///< Percentage of Content rewards that are paid to Community Moderators.

      fc::microseconds                  reward_interval()const
      {
         return fc::hours( reward_interval_hours );
      }

      fc::microseconds                  reward_duration()const
      {
         return fc::hours( reward_interval_hours * reward_interval_amount );
      }

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

      bool                                  channel = false;                    ///< True when a post is a channel post, and included in the channel feed of the community as the community name instead of author.

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
    * and encrypted with a public_key that a desired audience 
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
      account_name_type                     editor;                         ///< Name of the account creating or editing the post. Original author, or a nominated collaborator.

      account_name_type                     author;                         ///< Name of the account that created the post.

      string                                permlink;                       ///< Unique identifing string for the post. Should be a uuidv4 for private posts.

      account_name_type                     parent_author;                  ///< Account that created the post this post is replying to, empty if root post.

      string                                parent_permlink;                ///< Permlink of the post this post is replying to, empty if root post.
      
      string                                title;                          ///< Content related name of the post, used to find post with search API.

      string                                body;                           ///< Public text for display when the post is opened.

      string                                body_private;                   ///< Encrypted and private text for display when the post is opened and decrypted.

      string                                url;                            ///< Plaintext URL for the post to link to. For a livestream post, should link to the streaming server embed.

      string                                url_private;                    ///< Private Encrypted ciphertext URL for the post to link to. For a livestream post, should link to the streaming server embed.

      string                                ipfs;                           ///< Public IPFS file hash: images, videos, files.

      string                                ipfs_private;                   ///< Private and encrypted IPFS file hash: images, videos, files.

      string                                magnet;                         ///< Bittorrent magnet links to torrent file swarms: videos, files.

      string                                magnet_private;                 ///< Bittorrent magnet links to Private and Encrypted torrent file swarms: videos, files.

      string                                json;                           ///< JSON string of additional interface specific data relating to the post.

      string                                json_private;                   ///< Private and Encrypted JSON string of additional interface specific data relating to the post.

      string                                language;                       ///< String containing the two letter ISO language code of the native language of the author.

      string                                public_key;                     ///< The public key used to encrypt the post, holders of the private key may decrypt.

      community_name_type                   community;                      ///< The name of the community to which the post is uploaded to.

      vector< tag_name_type >               tags;                           ///< Set of tags for sorting the post by and placing the post into ranked feeds.

      vector< account_name_type >           collaborating_authors;          ///< Set of Authors that are able to create edits for the post.

      vector< account_name_type >           supernodes;                     ///< Set of Supernodes that the IPFS file is hosted with. Each should additionally hold the private key.

      account_name_type                     interface;                      ///< Name of the Interface application that broadcasted the transaction.

      double                                latitude;                       ///< Latitude co-ordinates of the comment.

      double                                longitude;                      ///< Longitude co-ordinates of the comment.

      asset                                 comment_price;                  ///< Price that is required to comment on the post.

      asset                                 premium_price;                  ///< Price that is required to unlock premium content.

      comment_options                       options;                        ///< Settings for the post, that effect how the network applies and displays it.

      bool                                  deleted = false;                ///< True to delete post, false to create post.

      void                                  validate()const;
      void                                  get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( editor ); }
      void                                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( editor ); }
   };


   /**
    * Votes for a comment to allocate content rewards and increase the posts ranked ordering.
    */
   struct comment_vote_operation : public base_operation
   {
      account_name_type              voter;           ///< Name of the voting account.

      account_name_type              author;          ///< Name of the account that created the post being voted on.

      string                         permlink;        ///< Permlink of the post being voted on.

      int16_t                        weight = 0;      ///< Percentage weight of the voting power applied to the post.

      account_name_type              interface;       ///< Name of the interface account that was used to broadcast the transaction.

      string                         reaction;        ///< An Emoji selected as a reaction to the post while voting.

      string                         json;            ///< JSON Metadata of the vote.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( voter ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( voter ); }
   };


   /** 
    * Views a post, which increases the post's content reward earnings.
    * 
    * Nominates an interface through which the post was viewed,
    * and nominates a supernode that served the data through IPFS.
    */
   struct comment_view_operation : public base_operation
   {
      account_name_type              viewer;           ///< Name of the viewing account.

      account_name_type              author;           ///< Name of the account that created the post being viewed. 

      string                         permlink;         ///< Permlink to the post being viewed.

      account_name_type              interface;        ///< Name of the interface account that was used to broadcast the transaction and view the post. 

      account_name_type              supernode;        ///< Name of the supernode account that served the IPFS file data in the post.

      string                         json;             ///< JSON Metadata of the view.

      bool                           viewed = true;    ///< True if viewing the post, false if removing view object.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( viewer ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( viewer ); }
   };


   /**
    * Shares a post to the account's feed.
    * 
    * All accounts that follow them can then see the post.
    * 
    * Increases the share count of the post.
    * 
    * Expands its visibility by adding it to feeds of new accounts.
    * 
    * Increases the content rewards earned by the post, and increases its rank ordering.
    */
   struct comment_share_operation : public base_operation
   {
      account_name_type                     sharer;              ///< Name of this sharer account.

      account_name_type                     author;              ///< Name of the account that created the post being shared.

      string                                permlink;            ///< Permlink to the post being shared.

      string                                reach;               ///< Audience reach selection for share.

      account_name_type                     interface;           ///< Name of the interface account that was used to broadcast the transaction and share the post.

      vector< community_name_type >         communities;         ///< Share the post with a set of communities.

      vector< tag_name_type >               tags;                ///< Share the post with a set of tags.

      string                                json;                ///< JSON Metadata of the share.

      bool                                  shared = true;       ///< True if sharing the post, false if removing previous share.

      void                                  validate()const;
      void                                  get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( sharer ); }
      void                                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( sharer ); }
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
   struct comment_moderation_operation : public base_operation
   {
      account_name_type              moderator;                         ///< Account creating the tag: can be a governance address or a community moderator. 

      account_name_type              author;                            ///< Author of the post being moderated.

      string                         permlink;                          ///< Permlink of the post being moderated.

      vector< tag_name_type >        tags;                              ///< Set of tags to apply to the post for selective interface side filtering.

      uint16_t                       rating;                            ///< Moderator's assigned rating for the post.

      string                         details;                           ///< String explaining the reason for the tag to the author.

      string                         json;                              ///< JSON Metadata of the moderation tag.

      account_name_type              interface;                         ///< Interface account used for the transaction.

      bool                           filter = false;                    ///< True if the post should be filtered from the community and governance account subscribers.

      bool                           removal_requested = false;         ///< True if the moderator formally requests that the post be removed by the author.

      vector< beneficiary_route_type >  beneficiaries_requested;        ///< Beneficiary routes that are requested for revenue sharing with a claimant.

      bool                           applied = true;                    ///< True if applying the tag, false if removing the tag.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( moderator ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( moderator ); }
   };


   /**
    * Creates a private encrypted message.
    * 
    * Collected into an inbox structure for direct private messaging.
    * 
    * Messages can be sent to connected accounts, 
    * where they are encrypted with 
    * the secure keys of the sender and reciever.
    * 
    * Messages can be sent to communities, where they can be read by all
    * community members with the equivalent private community decryption key.
    */
   struct message_operation : public base_operation
   {
      account_name_type              sender;                  ///< The account sending the message.

      account_name_type              recipient;               ///< The receiving account of the message.

      community_name_type            community;               ///< The name of the community that the message should be sent to.

      string                         public_key;              ///< The public key used to encrypt the message, the recipient holder of the private key may decrypt.

      string                         message;                 ///< Encrypted ciphertext of the message being sent.

      string                         ipfs;                    ///< Encrypted Private IPFS file hash: Voice message audio, images, videos, files.

      string                         json;                    ///< Encrypted Message metadata.

      string                         uuid;                    ///< uuidv4 uniquely identifying the message for local storage.

      account_name_type              interface;               ///< Account of the interface that broadcasted the transaction.
      
      account_name_type              parent_sender;           ///< The account sending the message that this message replies to.

      string                         parent_uuid;             ///< uuidv4 of the message that this message replies to.

      time_point                     expiration;              ///< Time that the message expires and is automatically deleted from API access.

      bool                           forward = false;         ///< True when the parent message is being forwarded into the conversation.

      bool                           active = true;           ///< True to send or edit message, false to delete message.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( sender ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( sender ); }
   };


   /**
    * Lists contain a curated group of accounts, comments, communities and other objects.
    * 
    * Used to collect a group of objects for reference and browsing.
    */
   struct list_operation : public base_operation
   {
      account_name_type              creator;             ///< Name of the account that created the list.

      string                         list_id;             ///< uuidv4 referring to the list.

      string                         name;                ///< Name of the list.

      string                         details;             ///< Public details description of the list.

      string                         json;                ///< Public JSON metadata of the list.

      account_name_type              interface;           ///< Account of the interface that broadcasted the transaction.

      vector< int64_t >              accounts;            ///< Account IDs within the list.

      vector< int64_t >              comments;            ///< Comment IDs within the list.

      vector< int64_t >              communities;         ///< Community IDs within the list.

      vector< int64_t >              assets;              ///< Asset IDs within the list.

      vector< int64_t >              products;            ///< Product IDs within the list.

      vector< int64_t >              auctions;            ///< Auction IDs within the list.

      vector< int64_t >              nodes;               ///< Graph node IDs within the list.

      vector< int64_t >              edges;               ///< Graph edge IDs within the list.

      vector< int64_t >              node_types;          ///< Graph node property IDs within the list.

      vector< int64_t >              edge_types;          ///< Graph edge property IDs within the list.

      bool                           active = true;       ///< True when the list is active, false to remove list.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( creator ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( creator ); }
   };


   /**
    * Polls enable accounts to vote on a series of options.
    * 
    * Polls have a fixed duration, and determine the winning option.
    */
   struct poll_operation : public base_operation
   {
      account_name_type              creator;                  ///< Name of the account that created the poll.

      string                         poll_id;                  ///< uuidv4 referring to the poll.

      community_name_type            community;                ///< Community that the poll is shown within. Null for no community.

      string                         public_key;               ///< Public key for encrypting details and poll options.

      account_name_type              interface;                ///< Account of the interface that broadcasted the transaction.

      string                         details;                  ///< Text describing the question being asked.

      string                         json;                     ///< JSON metadata of the poll.

      string                         poll_option_0;            ///< Poll option zero, vote 0 to select.

      string                         poll_option_1;            ///< Poll option one, vote 1 to select.

      string                         poll_option_2;            ///< Poll option two, vote 2 to select.

      string                         poll_option_3;            ///< Poll option three, vote 3 to select.

      string                         poll_option_4;            ///< Poll option four, vote 4 to select.

      string                         poll_option_5;            ///< Poll option five, vote 5 to select.

      string                         poll_option_6;            ///< Poll option six, vote 6 to select.

      string                         poll_option_7;            ///< Poll option seven, vote 7 to select.

      string                         poll_option_8;            ///< Poll option eight, vote 8 to select.

      string                         poll_option_9;            ///< Poll option nine, vote 9 to select.

      time_point                     completion_time;          ///< Time the poll voting completes.

      bool                           active = true;            ///< True when the poll is active, false to remove the poll and all poll votes.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( creator ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( creator ); }
   };


   /**
    * Poll Vote for a specified poll option.
    * 
    * Polls have a fixed duration, and determine the winning option.
    */
   struct poll_vote_operation : public base_operation
   {
      account_name_type              voter;               ///< Name of the account that created the vote.

      account_name_type              creator;             ///< Name of the account that created the poll.

      string                         poll_id;             ///< uuidv4 referring to the poll.

      account_name_type              interface;           ///< Account of the interface that broadcasted the transaction.

      uint16_t                       poll_option;         ///< Poll option chosen.

      bool                           active = true;       ///< True when the poll vote is active, false to remove.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( voter ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( voter ); }
   };


   /**
    * Premium purchase requests that a Premium Post to be made available for decryption to a purchaser.
    * 
    * Premium content can be released by the author of the post
    * or a designated supernode that holds the decryption key from a release transaction.
    * 
    * Funds are released to the author when the post is viewed by a view transaction.
    * The Supernode referenced in the view transaction earns a share of the purchase price.
    * 
    * If a view is not broadcasted, this indicates a rejection by the purchaser,
    * and the purchase price, less the network fee, is refunded.
    * 
    * Premium Content Transactions rely on a non-trivial degree of trust between transacting accounts:
    * 
    * - An Author may provide an inaccurate decryption key to the supernodes or purchaser.
    * - An Author may provide a misleading or false content description that does not match the underlying content.
    * - A Supernode may receive a valid key and transmit a false key to purchasers.
    * - A Supernode may transmit the decryption key to an unauthorized account after designation.
    * - A Purchaser may receive a valid key and refuse/fail to sign a view transaction.
    * - A Purchaser may also transmit the decryption key to an unauthorized account after purchasing.
    * 
    * The Premium Content Mechanism Design chooses to optimize for the interests 
    * of the purchaser ( and the application the purchaser is using )
    * by giving them the final say over releasing the funds if they do not broadcast a view transaction.
    * 
    * For highly sensitive or valuable encrypted content, the marketplace escrow system should instead be used
    * for a direct asynchronous dispute resolution process between the buyer and seller with mediation involved.
    * 
    * The Premium Content Mechanism Design maximizes speed and lowers overhead for a rapid unlocking system that 
    * is aligned towards high volume, low value premium content transactions where the use of a double opt-in mediation
    * system would be too slow and cumbersome for inline content browsing, 
    * and trades off the potential for the purchaser to fail to pay for the content some of the time.
    * 
    * The Purchaser's interface recieves a share of the premium content purchasing price as an incentive
    * to offer a balanced user experience that maximizes the potential for users to complete the payment, 
    * while offering a reasonable ability to refuse to pay for misleading or fraudulent content.
    * The intention of this system is to operate on a "customer is always right" approach, 
    * to maximise purchase volume potential and lower the cognitive costs associated with micropayments.
    * 
    * An alternative approach that releases funds regardless of purchaser approval of the premium
    * content would create high incentives for misleading premium content posts, and would pollute
    * the mechanism beyond the tolerence of a discerning audience. Concern for misleading content
    * would also significantly diminish demand to a much greater extent than the rate of 
    * fraud would reduce premium content revenue.
    * Including a Mediation or dispute resolution system would also slow down the purchase process significantly, 
    * require deposit bonds for all parties, and have excessive cognitive costs.
    * 
    * The ratio of consensual viewing transaction approval follow-through from 
    * incoming purchases would strongly indicate the quality of the underlying content to potential purchasers.
    * 
    * The Premium Content purchasing system should be used 
    * by creators with the objective of maximizing distribution volume and overall income, 
    * rather than minimizing the risk of individual instances of fraud by the purchaser.
    * If this is sub-optimal for the creator's use case, 
    * they are strongly advised to use an escrow transfer instead.
    */
   struct premium_purchase_operation : public base_operation
   {
      account_name_type              account;            ///< Name of the account purchasing the premium content.

      account_name_type              author;             ///< Name of the author of the premium post.

      string                         permlink;           ///< Permlink of the premium post.

      account_name_type              interface;          ///< Interface account used for the transaction.

      bool                           purchased = true;   ///< True to purchase the premium content, false to cancel existing undelivered purchase. 

      void                           validate()const;
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Premium Release enables a Premium Post to be decrypted by a purchaser.
    * 
    * Creates a Premium content key object which holds the private key, 
    * encrypted with the public secure key of its recipient account.
    * The author may release the premium content to any account at any time, 
    * however supernodes may only release a premium post when 
    * it has a preceding purchase transaction pending.
    * 
    * A supernode must be listed in the post as a designated supernode 
    * in order to create release transactions for the post, 
    * and a premium key must be released to a supernode by 
    * the author before it can be redistributed to purchasers.
    * 
    * The Account that provides the encrypted private key
    * receives a share of the premium content purchase amount.
    * 
    * Supernodes may compete to provide the fastest release transactions to purchasing users,
    * and should avoid supernodes that do not release keys accurately over time.
    * The more supernodes that a premium post is released to, 
    * the faster the purchase response time will become.
    * 
    * Multiple Supernodes may release the key to a purchaser, 
    * with the purchaser having discretion over which 
    * supernode to credit for the transaction with its choice of viewing Supernode.
    * In the Event that a premium post is purchased, 
    * and no Premium content keys are created, 
    * the purchase is refunded to the viewer.
    * If a Premium content key is created, 
    * the refund amount is divided between the purchaser, 
    * the author, and the network.
    */
   struct premium_release_operation : public base_operation
   {
      account_name_type              provider;              ///< Name of the account releasing the premium content.

      account_name_type              account;               ///< Name of the account purchasing the premium content.

      account_name_type              author;                ///< Name of the author of the premium post.

      string                         permlink;              ///< Permlink of the premium post.

      account_name_type              interface;             ///< Account of the interface that broadcasted the transaction.

      string                         encrypted_key;         ///< The private decryption key of the post, encrypted with the public secure key of the purchasing account.

      void                           validate()const;
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( provider ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( provider ); }
   };


   //==============================//
   //==== Community Operations ====//
   //==============================//


   /**
    * Creates a new community for collecting posts about a specific topic.
    * 
    * Communities contain a series of collective public keys for encrypting private posts with,
    * and the private key is shared with newly added members when they join.
    */
   struct community_create_operation : public base_operation
   {
      account_name_type                  founder;                            ///< The account that created the community, able to add and remove administrators.

      community_name_type                name;                               ///< Name of the community. Should be randomized string if private community. 

      string                             display_name;                       ///< The full name of the community (non-consensus), encrypted with the member key if private community.

      string                             details;                            ///< Describes the community and what it is about and the rules of posting, encrypted with the member key if private community.

      string                             url;                                ///< Community URL link for more details, encrypted with the member key if private community.

      string                             profile_image;                      ///< IPFS Reference of the Icon image of the community, encrypted with the member key if private community.

      string                             cover_image;                        ///< IPFS Reference of the Cover image of the community, encrypted with the member key if private community.

      string                             json;                               ///< Public plaintext JSON information about the community, its topic and rules.

      string                             json_private;                       ///< Private ciphertext JSON information about the community, encrypted with the member key if private community.

      flat_set< tag_name_type >          tags;                               ///< Set of tags of the topics within the community to enable discovery.

      bool                               private_community = false;          ///< True when the community is private, and all posts, events, directives, polls must be encrypted.

      bool                               channel = false;                    ///< True when a community is a channel, and only accepts channel posts from admins.
      
      string                             author_permission;                  ///< Determines which accounts can create root posts.

      string                             reply_permission;                   ///< Determines which accounts can create replies to root posts.

      string                             vote_permission;                    ///< Determines which accounts can create comment votes on posts and comments.

      string                             view_permission;                    ///< Determines which accounts can create comment views on posts and comments.

      string                             share_permission;                   ///< Determines which accounts can create comment shares on posts and comments.

      string                             message_permission;                 ///< Determines which accounts can create direct messages in the community.

      string                             poll_permission;                    ///< Determines which accounts can create polls in the community.

      string                             event_permission;                   ///< Determines which accounts can create events in the community.

      string                             directive_permission;               ///< Determines which accounts can create directives and directive votes in the community.

      string                             add_permission;                     ///< Determines which accounts can add new members, and accept member requests.

      string                             request_permission;                 ///< Determines which accounts can request to join the community.

      string                             remove_permission;                  ///< Determines which accounts can remove and blacklist.

      string                             community_member_key;               ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.

      string                             community_moderator_key;            ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.

      string                             community_admin_key;                ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.

      string                             community_secure_key;               ///< Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.

      string                             community_standard_premium_key;     ///< Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.

      string                             community_mid_premium_key;          ///< Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.

      string                             community_top_premium_key;          ///< Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.

      account_name_type                  interface;                          ///< Account of the interface that broadcasted the transaction.
      
      asset_symbol_type                  reward_currency = SYMBOL_COIN;      ///< The Currency asset used for content rewards in the community.

      asset                              standard_membership_price;          ///< Price paid per month by community standard members.

      asset                              mid_membership_price;               ///< Price paid per month by all mid level community members.

      asset                              top_membership_price;               ///< Price paid per month by all top level community members.

      flat_set< account_name_type >      verifiers;                          ///< Accounts that are considered ground truth sources of verification authority, wth degree 0.

      uint64_t                           min_verification_count = 0;         ///< Minimum number of incoming verification transaction to be considered verified by this community.

      uint64_t                           max_verification_distance = 0;      ///< Maximum number of degrees of seperation from a verfier to be considered verified by this community.
      
      uint16_t                           max_rating = 9;                     ///< Highest severity rating that posts in the community can have.

      uint32_t                           flags = 0;                          ///< The currently active flags on the community for content settings.

      uint32_t                           permissions = COMMUNITY_PERMISSION_MASK;  ///< The flag permissions that can be activated on the community for content settings.

      void                               validate()const;
      void                               get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( founder ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( founder ); }
   };


   /**
    * Updates the details of an existing community.
    * 
    * If the community public key is changed, all existing members must be reinitiated
    * by creating a new community_member_object containing an encrypted copy of the new key.
    */
   struct community_update_operation : public base_operation
   {
      account_name_type              account;                            ///< Account updating the community. Administrator of the community.

      community_name_type            community;                          ///< Name of the community. Should be randomized string if private community. 

      string                         display_name;                       ///< The full name of the community (non-consensus), encrypted with the member key if private community.

      string                         details;                            ///< Describes the community and what it is about and the rules of posting, encrypted with the member key if private community.

      string                         url;                                ///< Community URL link for more details, encrypted with the member key if private community.

      string                         profile_image;                      ///< IPFS Reference of the Icon image of the community, encrypted with the member key if private community.

      string                         cover_image;                        ///< IPFS Reference of the Cover image of the community, encrypted with the member key if private community.

      string                         json;                               ///< Public plaintext JSON information about the community, its topic and rules.

      string                         json_private;                       ///< Private ciphertext JSON information about the community, encrypted with the member key if private community.

      account_name_type              pinned_author;                      ///< Author of Post pinned to the top of the community's page.

      string                         pinned_permlink;                    ///< Permlink of Post pinned to the top of the community's page, encrypted with the member key if private community.

      flat_set< tag_name_type >      tags;                               ///< Set of tags of the topics within the community to enable discovery.

      bool                           private_community;                  ///< True when the community is private, and all posts must be encrypted.

      bool                           channel = false;                    ///< True when a community is a channel, and only accepts channel posts from admins.

      string                         author_permission;                  ///< Determines which accounts can create root posts.

      string                         reply_permission;                   ///< Determines which accounts can create replies to root posts.

      string                         vote_permission;                    ///< Determines which accounts can create comment votes on posts and comments.

      string                         view_permission;                    ///< Determines which accounts can create comment views on posts and comments.

      string                         share_permission;                   ///< Determines which accounts can create comment shares on posts and comments.

      string                         message_permission;                 ///< Determines which accounts can create direct messages in the community.

      string                         poll_permission;                    ///< Determines which accounts can create polls in the community.

      string                         event_permission;                   ///< Determines which accounts can create events in the community.

      string                         directive_permission;               ///< Determines which accounts can create directives and directive votes in the community.

      string                         add_permission;                     ///< Determines which accounts can add new members, and accept member requests.

      string                         request_permission;                 ///< Determines which accounts can request to join the community.

      string                         remove_permission;                  ///< Determines which accounts can remove and blacklist.

      string                         community_member_key;               ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.

      string                         community_moderator_key;            ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.

      string                         community_admin_key;                ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.

      string                         community_secure_key;               ///< Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.

      string                         community_standard_premium_key;     ///< Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.

      string                         community_mid_premium_key;          ///< Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.

      string                         community_top_premium_key;          ///< Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.

      asset                          standard_membership_price;          ///< Price paid per month by communitys standard members.

      asset                          mid_membership_price;               ///< Price paid per month by all mid level community members.

      asset                          top_membership_price;               ///< Price paid per month by all top level community members.

      flat_set< account_name_type >  verifiers;                          ///< Accounts that are considered ground truth sources of verification authority, wth degree 0.

      uint64_t                       min_verification_count = 0;         ///< Minimum number of incoming verification transaction to be considered verified by this community.

      uint64_t                       max_verification_distance = 0;      ///< Maximum number of degrees of seperation from a verfier to be considered verified by this community.
      
      uint16_t                       max_rating = 9;                     ///< Highest severity rating that posts in the community can have.

      uint32_t                       flags = 0;                          ///< The currently active flags on the community for content settings.

      uint32_t                       permissions = COMMUNITY_PERMISSION_MASK;  ///< The flag permissions that can be activated on the community for content settings.

      bool                           active = true;                      ///< True when the community is active, false to deactivate. 

      void                           validate()const;
      void                           get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Used to add or remove a member from a community, to a specified level of permission.
    * 
    * This operation discloses the encrypted_community_key
    * for decrypting posts, from an existing 
    * member of the community that has access to it. 
    */
   struct community_member_operation : public base_operation
   {
      account_name_type              account;                    ///< Account within the community accepting the request.

      account_name_type              member;                     ///< Account to accept into the community.

      community_name_type            community;                  ///< Community that is being joined.

      account_name_type              interface;                  ///< Name of the interface account that was used to broadcast the transaction.

      string                         member_type;                ///< Membership and Key encryption access and permission level.

      string                         encrypted_community_key;    ///< The Community Private Key of the member_type level, encrypted with the member's secure public key.

      time_point                     expiration;                 ///< Time that the membership will expire, and the key must be renewed.
      
      bool                           accepted = true;            ///< True to add member, false to remove member from that member type.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Requests that an account be added as a new member of a community.
    * 
    * Must be accepted using community_member_operation by a member of the community with at least that rank.
    * Cannot be used to join a community with request permission set to NONE.
    * Used to pay premium membership fees to the founder, and highly voted community members.
    */
   struct community_member_request_operation : public base_operation
   {
      account_name_type              account;            ///< Account that wants to join the community.

      community_name_type            community;          ///< Community that is being requested to join.

      account_name_type              interface;          ///< Name of the interface account that was used to broadcast the transaction.

      string                         member_type;        ///< Membership and Key encryption access and permission level.

      string                         message;            ///< Message attatched to the request, encrypted with the communities public key.

      time_point                     expiration;         ///< Time that the request will expire, and must be renewed.

      bool                           requested = true;   ///< Set true to request, false to cancel request.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Votes for a member to increase their membership approval weight.
    * 
    * Moderators and Admins with a higher approval weight receive a higher 
    * proportion of incoming community management rewards and premium membership fees.
    */
   struct community_member_vote_operation : public base_operation
   {
      account_name_type              account;          ///< Account of a member of the community.

      community_name_type            community;        ///< Community that the moderator is being voted into.

      account_name_type              member;           ///< Account of the member being voted for.

      account_name_type              interface;        ///< Name of the interface account that was used to broadcast the transaction.

      uint16_t                       vote_rank = 1;    ///< Voting rank for the specified community moderator

      bool                           approved = true;  ///< True when voting for the moderator, false when removing.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Adds a community to an account's subscriptions.
    * 
    * Subscribed communities are included in an account's communities
    * feed, and can be browsed in feeds. 
    * Optionally, communities can be filtered and posts from that community will never be shown in feeds.
    */
   struct community_subscribe_operation : public base_operation
   {
      account_name_type              account;             ///< Account that wants to subscribe to the community.

      community_name_type            community;           ///< Community to suscribe to.

      account_name_type              interface;           ///< Name of the interface account that was used to broadcast the transaction and subscribe to the community.

      bool                           added = true;        ///< True to add to community subscription feed, false to remove.
      
      bool                           subscribed = true;   ///< true if subscribing, false if filtering. 

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type              account;               ///< Moderator or admin of the community.

      account_name_type              member;                ///< Account to be blacklisted from interacting with the community.

      community_name_type            community;             ///< Community that member is being blacklisted from.

      bool                           blacklisted = true;    ///< Set to true to add account to blacklist, set to false to remove from blacklist. 

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Used to create a federation connection between two communities.
    * 
    * Federated Communities share their community private keys between members, 
    * enabling them to decrypt all private posts and messages within the other.
    * 
    * The Federation type determines whether the Federation connection shares
    * Members, Moderator, and Admin accounts between communities.
    * 
    * When an account is added as a member to a community, they are added as member to
    * all downstream federated communities. 
    * When an account is added as a moderator, they are added as a moderator to all 
    * downstream communities with at least a moderator level federation.
    * When an account is added as an admin, they are added as an admin to all 
    * downstream communities with at least an admin level federation.
    * 
    * Directed Federated communities act as concentric circles, where membership in 
    * one is membership in the other, but membership in the outer circle
    * does not grant membership to the inner circle. 
    * Mutually Federated communities act as mirrors, that combine the membership of 
    * both communities into both communities.
    * Making a new community and Federating with an existing community 
    * effectively duplicates the membership base of an existing community into
    * the new community. 
    * 
    * This operation discloses the private encrypted_community_key
    * for decrypting posts, from an existing 
    * member of the community that has access to it.
    */
   struct community_federation_operation : public base_operation
   {
      account_name_type              account;                      ///< Admin within the community creating the federation.

      string                         federation_id;                ///< uuidv4 for the federation, for local storage of decryption key.

      community_name_type            community;                    ///< Community that the account is an admin within.

      community_name_type            federated_community;          ///< Community to create a new federation with.

      string                         message;                      ///< Encrypted Message to include to the members of the requested community, encrypted with equivalent community key.

      string                         json;                         ///< Encrypted JSON metadata, encrypted with equivalent community key.

      string                         federation_type;              ///< Type of Federation level.

      string                         encrypted_community_key;      ///< The Community Private Key, encrypted with the federated communities public key.

      bool                           share_accounts = true;        ///< True to share accounts across the Federation, false to only share decryption keys.

      bool                           accepted = true;              ///< True to accept request, false to remove federation.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type              account;                ///< Account that created the event.

      community_name_type            community;              ///< Community that the event is within.

      string                         event_id;               ///< UUIDv4 referring to the event within the Community. Unique on community/event_id

      string                         public_key;             ///< Public key for encrypting the event details. Null if public event

      string                         event_name;             ///< The Display Name of the event. Encrypted if private event.

      string                         location;               ///< Address of the location of the event. Encrypted if private event.

      double                         latitude;               ///< Latitude co-ordinates of the event.

      double                         longitude;              ///< Longitude co-ordinates of the event.

      string                         details;                ///< Event details describing the purpose of the event. Encrypted if private event.

      string                         url;                    ///< Link containining additional event information. Encrypted if private event.

      string                         json;                   ///< Additional Event JSON data. Encrypted if private event.

      account_name_type              interface;              ///< Account of the interface that broadcasted the transaction.

      asset                          event_price;            ///< Amount paid to join the attending list as a ticket holder to the event. 

      time_point                     event_start_time;       ///< Time that the Event will begin.

      time_point                     event_end_time;         ///< Time that the event will end.

      bool                           active = true;          ///< True if the event is active, false to remove the event.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Responds to an event invitation.
    * 
    * Denotes the status of an account attending an event.
    */
   struct community_event_attend_operation : public base_operation
   {
      account_name_type              attendee;                    ///< Account that is attending the event.

      community_name_type            community;                  ///< Community that the event is within.

      string                         event_id;                   ///< UUIDv4 referring to the event within the Community. Unique on community/event_id.

      string                         public_key;                 ///< Public key for encrypting details. Null if public event.

      string                         message;                    ///< Encrypted message to the community operating the event.

      string                         json;                       ///< Additional Event JSON data. Encrypted if private event.

      account_name_type              interface;                  ///< Account of the interface that broadcasted the transaction.

      bool                           interested = true;          ///< True to set interested in the event, and receive notifications about it, false for not interested.

      bool                           attending = true;           ///< True to attend the event, false to for not attending.

      bool                           active = true;              ///< True to create attendance for the event, false to remove.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( attendee ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( attendee ); }
   };


   /**
    * A Community directive that contains action instructions and deliverables for community members.
    */
   struct community_directive_operation : public base_operation
   {
      account_name_type              account;                    ///< Account that created the directive.

      string                         directive_id;               ///< UUIDv4 referring to the directive. Unique on account/directive_id.

      account_name_type              parent_account;             ///< Account that created the parent directive.

      string                         parent_directive_id;        ///< UUIDv4 referring to the parent directive. Unique on account/directive_id.

      community_name_type            community;                  ///< Community that the directive is given to.

      string                         public_key;                 ///< Public key for encrypting the directive details. Null if public directive.

      account_name_type              interface;                  ///< Account of the interface that broadcasted the transaction.

      string                         details;                    ///< Text details of the directive. Should explain the action items.

      string                         cover_image;                ///< IPFS image for display of this directive in the interface. 

      string                         ipfs;                       ///< IPFS file reference for the directive. Images or other files can be attatched.

      string                         json;                       ///< Additional Directive JSON metadata.

      time_point                     directive_start_time;       ///< Time that the Directive will begin.

      time_point                     directive_end_time;         ///< Time that the Directive must be completed by.

      bool                           completed = false;          ///< True when the directive has been completed.

      bool                           active = true;              ///< True while the directive is active, false to remove.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Votes to approve or disapprove a directive made by a member of a community.
    * 
    * Used for Consensus directive selection, and for directive feedback.
    */
   struct community_directive_vote_operation : public base_operation
   {
      account_name_type              voter;                      ///< Account creating the directive vote.

      account_name_type              account;                    ///< Account that created the directive.

      string                         directive_id;               ///< UUIDv4 referring to the directive. Unique on account/directive_id.

      string                         public_key;                 ///< Public key for encrypting the directive vote details. Null if public directive vote.

      account_name_type              interface;                  ///< Account of the interface that broadcasted the transaction.

      string                         details;                    ///< Text details of the directive vote. Should contain directive feedback.

      string                         json;                       ///< Additional Directive JSON metadata.

      bool                           approve = true;             ///< True when the directive is approved, false when it is opposed.

      bool                           active = true;              ///< True while the directive vote is active, false to remove.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( voter ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( voter ); }
   };


   /**
    * Determines the State of an account's active 
    * directive selection, and its outgoing directives.
    */
   struct community_directive_member_operation : public base_operation
   {
      account_name_type              account;                       ///< Account recieving and creating directives within a community. 

      community_name_type            community;                     ///< Community that the directive member is contained within.

      account_name_type              interface;                     ///< Account of the interface that broadcasted the transaction.
      
      string                         public_key;                    ///< Public key for encrypting the directive member details. Null if public directive member.

      string                         details;                       ///< Text details of the directive member. Should elaborate interests and priorities for voting selection.

      string                         json;                          ///< Additional Directive Member JSON metadata.

      string                         command_directive_id;          ///< The Current outgoing directive as community co-ordinator.

      string                         delegate_directive_id;         ///< The Current outgoing directive to all hierachy subordinate members.

      string                         consensus_directive_id;        ///< The Current outgoing directive for community consensus selection.

      string                         emergent_directive_id;         ///< The Current outgoing emergent directive for selection by other members.

      bool                           active = true;                 ///< True when the account is active for the directive distribution.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Votes to approve or disapprove a directive made by a member of a community.
    * 
    * Used for Consensus directive selection, and for directive feedback.
    */
   struct community_directive_member_vote_operation : public base_operation
   {
      account_name_type              voter;                      ///< Account creating the directive member vote.

      account_name_type              member;                     ///< Account being voted on.

      community_name_type            community;                  ///< Community that the directive member vote is contained within.

      string                         public_key;                 ///< Public key for encrypting the directive member vote details. Null if public directive member vote.

      account_name_type              interface;                  ///< Account of the interface that broadcasted the transaction.

      string                         details;                    ///< Text details of the directive member vote. Should contain directive member feedback.

      string                         json;                       ///< Additional Directive member vote JSON metadata.

      bool                           approve = true;             ///< True when the directive member is approved, false when they is opposed.

      bool                           active = true;              ///< True while the directive member vote is active, false to remove.

      void                           validate()const;
      void                           get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( voter ); }
      void                           get_creator_name( flat_set<account_name_type>& a )const{ a.insert( voter ); }
   };


   //================================//
   //==== Advertising Operations ====//
   //================================//


   /**
    * Creates a new ad creative to be used in a campaign for display in interfaces.
    */
   struct ad_creative_operation : public base_operation 
   {
      account_name_type        account;           ///< Account publishing the ad creative.

      account_name_type        author;            ///< Author of the objective item referenced.

      string                   objective;         ///< The reference of the object being advertised, the link and CTA destination of the creative.

      string                   creative_id;       ///< uuidv4 referring to the creative

      string                   creative;          ///< IPFS link to the Media to be displayed, image or video.

      string                   json;              ///< JSON string of creative metadata for display.

      string                   format_type;       ///< The type of formatting used for the advertisment, determines the interpretation of the creative.

      bool                     active = true;     ///< True if the creative is enabled for active display, false to deactivate.

      void                     validate()const;
      void                     get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      void                             get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type                  provider;       ///< Account of an interface offering ad supply.

      string                             inventory_id;   ///< uuidv4 referring to the inventory offering.

      string                             audience_id;    ///< uuidv4 referring to audience object containing usernames of desired accounts in interface's audience.

      string                             metric;         ///< Type of expense metric used.

      asset                              min_price;      ///< Minimum bidding price per metric.

      uint32_t                           inventory;      ///< Total metrics available.

      string                             json;           ///< JSON metadata for the inventory.

      bool                               active = true;  ///< True if the inventory is enabled for display, false to deactivate.

      void                               validate()const;
      void                               get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( provider ); }
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
      account_name_type                  account;        ///< Account creating the audience set.

      string                             audience_id;    ///< uuidv4 referring to the audience for inclusion in inventory and campaigns.

      string                             json;           ///< JSON metadata for the audience.

      vector< account_name_type >        audience;       ///< List of usernames of the accounts to be included in the audience.

      bool                               active = true;  ///< True if the audience is enabled for reference, false to deactivate.

      void                               validate()const;
      void                               get_required_posting_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      void                             get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( bidder ); }
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
      void                               get_required_posting_authorities( flat_set<account_name_type>& a )const { a.insert( account ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Creates a new edge in the Network's Graph Database.
    */
   struct graph_edge_operation : public base_operation
   {
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
      void                               get_required_posting_authorities( flat_set<account_name_type>& a )const { a.insert( account ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Creates a new type of node for instantiation in the Network's Graph Database.
    */
   struct graph_node_property_operation : public base_operation
   {
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
      void                               get_required_posting_authorities( flat_set<account_name_type>& a )const { a.insert( account ); }
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Creates a new type of edge for instantiation in the Network's Graph Database.
    */
   struct graph_edge_property_operation : public base_operation
   {
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
      void                               get_required_posting_authorities( flat_set<account_name_type>& a )const { a.insert( account ); }
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
      account_name_type           from;       ///< Sending account to transfer asset from.
      
      account_name_type           to;         ///< Recieving account to transfer asset to.
      
      asset                       amount;     ///< The funds being transferred.

      string                      memo;       ///< The memo for the transaction, encryption on the memo is advised. 

      void                        validate()const;
      void                        get_required_active_authorities( flat_set<account_name_type>& a )const { a.insert( from ); }
      void                        get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   /**
    * Requests a Transfer from an account to another account.
    */
   struct transfer_request_operation : public base_operation
   {
      account_name_type            to;                 ///< Account requesting the transfer.
      
      account_name_type            from;               ///< Account that is being requested to accept the transfer.
      
      asset                        amount;             ///< The funds being transferred.

      string                       memo;               ///< The memo for the transaction, encryption on the memo is advised. 

      string                       request_id;         ///< uuidv4 of the request transaction.

      time_point                   expiration;         ///< time that the request expires. 

      bool                         requested = true;   ///< True to send the request, false to cancel an existing request. 

      void                         validate()const;
      void                         get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( to ); }
      void                         get_creator_name( flat_set<account_name_type>& a )const{ a.insert( to ); }
   };


   /**
    * Accepts a transfer request from an account to another account.
    */
   struct transfer_accept_operation : public base_operation
   {
      account_name_type            from;               ///< Account that is accepting the transfer.

      account_name_type            to;                 ///< Account requesting the transfer.
      
      string                       request_id;         ///< uuidv4 of the request transaction.

      bool                         accepted = true;    ///< True to accept the request, false to reject. 

      void                         validate()const;
      void                         get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
      void                         get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   /**
    * Transfers an asset periodically from one account to another.
    */
   struct transfer_recurring_operation : public base_operation
   {
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
      void                        get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
      void                        get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   /**
    * Requests a periodic transfer from an account to another account.
    */
   struct transfer_recurring_request_operation : public base_operation
   {
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
      void                         get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( to ); }
      void                         get_creator_name( flat_set<account_name_type>& a )const{ a.insert( to ); }
   };


   /**
    * Accepts a periodic transfer request from an account to another account.
    */
   struct transfer_recurring_accept_operation : public base_operation
   {
      account_name_type            from;               ///< Account that is accepting the recurring transfer.

      account_name_type            to;                 ///< Account requesting the recurring transfer.
      
      string                       request_id;         ///< uuidv4 of the request transaction.

      bool                         accepted = true;    ///< True to accept the request, false to reject. 

      void                         validate()const;
      void                         get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
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
      account_name_type                  from;                ///< Account to transfer funds from and create new confidential balances. 

      asset                              amount;              ///< Amount of funds to transfer. 

      blind_factor_type                  blinding_factor;     ///< Factor to blind the output values. 

      vector< confidential_output >      outputs;             ///< Confidential balance outputs. 

      asset                              fee;                 ///< Fee amount paid for the transfer. 

      void                               validate()const;
      void                               get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
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
      account_name_type        account;      ///< Account claiming its reward balance from the network.

      asset                    reward;       ///< Amount of Reward balance to claim.

      void                     get_required_posting_authorities( flat_set< account_name_type >& a )const{ a.insert( account ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                     validate() const;
   };


   /**
    * Stakes a liquid balance of an account into it's staked balance.
    */
   struct stake_asset_operation : public base_operation
   {
      account_name_type        from;         ///< Account staking the asset.

      account_name_type        to;           ///< Account to stake the asset to, Same as from if null.

      asset                    amount;       ///< Amount of Funds to transfer to staked balance from liquid balance.

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   /**
    * Divests an amount of the staked balance of an account to it's liquid balance.
    */
   struct unstake_asset_operation : public base_operation
   {
      account_name_type        from;         ///< Account unstaking the asset.

      account_name_type        to;           ///< Account to unstake the asset to, Same as from if null.

      asset                    amount;       ///< Amount of Funds to transfer from staked balance to liquid balance.

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
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
      account_name_type        from;                   ///< The account the assets are withdrawn from.

      account_name_type        to;                     ///< The account receiving either assets or new stake.

      uint16_t                 percent = 0;            ///< The percent of the withdraw to go to the 'to' account.

      bool                     auto_stake = false;     ///< True if the stake should automatically be staked on the to account.

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const { a.insert( from ); }
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
      account_name_type         from;       ///< The account the assets are transferred from.

      account_name_type         to;         ///< The account that is recieving the savings balance, same as from if null.

      asset                     amount;     ///< Funds to be transferred from liquid to savings balance.

      string                    memo;       ///< The memo for the transaction, encryption on the memo is advised.

      void                      get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
      void                      get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
      void                      validate() const;
   };


   /**
    * Withdraws a specified balance from savings after a time duration.
    */
   struct transfer_from_savings_operation : public base_operation 
   {
      account_name_type        from;                 ///< Account to transfer savings balance from.

      account_name_type        to;                   ///< Account to receive the savings withdrawal.

      asset                    amount;               ///< Amount of asset to transfer from savings.

      string                   request_id;           ///< uuidv4 referring to the transfer.

      string                   memo;                 ///< The memo for the transaction, encryption on the memo is advised.

      bool                     transferred = true;   ///< True if the transfer is accepted, false to cancel transfer.

      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
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
      account_name_type        delegator;        ///< The account delegating the asset.

      account_name_type        delegatee;        ///< The account receiving the asset.

      asset                    amount;           ///< The amount of the asset delegated.

      void                     get_required_active_authorities( flat_set< account_name_type >& a ) const { a.insert( delegator ); }
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

      void                               get_required_active_authorities( flat_set< account_name_type >& a ) const { a.insert( account ); }
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

      void                           get_required_active_authorities( flat_set< account_name_type >& a ) const { a.insert( buyer ); }
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

      void                               get_required_active_authorities( flat_set< account_name_type >& a ) const { a.insert( account ); }
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

      void                           get_required_active_authorities( flat_set< account_name_type >& a ) const { a.insert( buyer ); }
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
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type        account;            ///< Account creating the transaction to approve the escrow.

      account_name_type        mediator;           ///< Nominated mediator to join the escrow for potential dispute resolution.

      account_name_type        escrow_from;        ///< The account sending funds into the escrow.

      string                   escrow_id;          ///< uuidv4 referring to the escrow being approved.

      bool                     approved = true;    ///< Set true to approve escrow, false to reject the escrow. All accounts must approve before activation.

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type        account;        ///< Account creating the transaction to dispute the escrow and raise it for resolution

      account_name_type        escrow_from;    ///< The account sending funds into the escrow.

      string                   escrow_id;      ///< uuidv4 referring to the escrow being disputed.

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type        account;           ///< The account creating the operation to release the funds.

      account_name_type        escrow_from;       ///< The escrow FROM account.

      string                   escrow_id;         ///< uuidv4 referring to the escrow.

      uint16_t                 release_percent;   ///< Percentage of escrow to release to the TO Account / remaining will be refunded to FROM account

      void                     validate()const;
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      void                   get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( owner ); }
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
      void                   get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( owner ); }
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
      account_name_type        owner;                    ///< Owner of the Auction order.

      string                   order_id;                 ///< uuidv4 of the order for reference.

      asset                    amount_to_sell;           ///< Amount of asset to sell at auction clearing price.

      price                    limit_close_price;        ///< The asset pair price to sell the amount at the auction clearing price. Amount to sell is base.

      account_name_type        interface;                ///< Name of the interface that created the transaction.

      time_point               expiration;               ///< Time that the order expires.

      bool                     opened = true;            ///< True to open new order, false to cancel existing order.

      void                     validate()const;
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( owner ); }
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( owner ); }
   };


   /**
    * Creates a new collateralized debt position in a market issued asset.
    * 
    * Market issued assets are used to create a digital representation of
    * an externally priced asset, using a regularly published price feed.
    */
   struct call_order_operation : public base_operation
   {
      account_name_type        owner;                      ///< Owner of the debt position and collateral.

      asset                    collateral;                 ///< Amount of collateral to add to the margin position.

      asset                    debt;                       ///< Amount of the debt to be issued.

      optional< uint16_t >     target_collateral_ratio;    ///< Maximum CR to maintain when selling collateral on margin call.

      account_name_type        interface;                  ///< Name of the interface that created the transaction.

      void                     validate()const;
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( owner ); }
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( owner ); }
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
      account_name_type        owner;                 ///< Owner of the Option order.

      string                   order_id;              ///< uuidv4 of the order for reference.

      asset                    options_issued;        ///< Amount of assets to issue covered options contract assets against. Must be a whole number.

      account_name_type        interface;             ///< Name of the interface that created the transaction.

      bool                     opened = true;         ///< True to open new order, false to close existing order by repaying options assets.

      void                     validate()const;
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( owner ); }
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( owner ); }
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
      account_name_type     account;            ///< Creator of the new liquidity pool.

      asset                 first_amount;       ///< Initial balance of one asset.

      asset                 second_amount;      ///< Initial balance of second asset, initial price is the ratio of these two amounts.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type     account;            ///< Account executing the exchange with the pool.

      asset                 amount;             ///< Amount of asset to be exchanged.

      asset_symbol_type     receive_asset;      ///< The asset to receive from the liquidity pool.

      account_name_type     interface;          ///< Name of the interface account broadcasting the transaction.

      optional< price >     limit_price;        ///< The price of acquistion at which to cap the exchange to.

      bool                  acquire = false;    ///< Set true to acquire the specified amount, false to exchange in.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type     account;            ///< Account funding the liquidity pool to receive the liquidity pool asset.

      asset                 amount;             ///< Amount of an asset to contribute to the liquidity pool.

      asset_symbol_type     pair_asset;         ///< Pair asset to the liquidity pool to receive liquidity pool assets of. 

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type     account;           ///< Account withdrawing liquidity pool assets from the pool.

      asset                 amount;            ///< Amount of the liquidity pool asset to redeem for underlying deposited assets. 

      asset_symbol_type     receive_asset;     ///< The asset to receive from the liquidity pool.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type     account;         ///< Account locking an asset as collateral.

      asset                 amount;          ///< Amount of collateral balance to lock, 0 to unlock existing collateral.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type     account;              ///< Account borrowing funds from the pool, must have sufficient collateral.

      asset                 amount;               ///< Amount of an asset to borrow. Limit of 75% of collateral value. Set to 0 to repay loan.

      asset                 collateral;           ///< Amount of an asset to use as collateral for the loan. Set to 0 to reclaim collateral to collateral balance.

      string                loan_id;              ///< uuidv4 unique identifier for the loan.

      bool                  flash_loan = false;   ///< True to execute a no collateral loan within a single transaction. Must be repaid within same txn.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type     account;         ///< Account lending an asset to the credit pool.

      asset                 amount;          ///< Amount of asset being lent.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type     account;       ///< Account withdrawing its lent asset from the credit pool by redeeming credit-assets. 

      asset                 amount;        ///< Amount of interest bearing credit assets being redeemed for their underlying assets. 

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type     account;           ///< Creator of the new option pool.

      asset_symbol_type     first_asset;       ///< First asset in the option trading pair.

      asset_symbol_type     second_asset;      ///< Second asset in the option trading pair.

      void                  validate()const;
      void                  get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      void                               get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type                  account;                    ///< Account executing the exchange with the pool.

      asset                              amount;                     ///< Amount of collateral asset to be exchanged.

      asset_symbol_type                  prediction_asset;           ///< Base Asset to the prediction pool to exchange with.

      bool                               exchange_base = false;      ///< True to exchange base asset, false to exchange one of all prediction assets.

      bool                               withdraw = false;           ///< True to Withdraw collateral, false to add funds. 

      void                               validate()const;
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                               get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type                  account;                    ///< Account executing the exchange with the pool.

      asset                              amount;                     ///< Amount of prediction asset to vote with.

      asset_symbol_type                  resolution_outcome;         ///< Base Asset to the prediction pool to exchange with.

      void                               validate()const;
      void                               get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                               get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   //==========================//
   //==== Asset Operations ====//
   //==========================//



   /**
    * Creates a new asset object of the asset type provided.
    * 
    * Assets can be transferred between accounts
    * to represent ownership of anything of value.
    * All assets can be staked and saved, delegated and received.
    */
   struct asset_create_operation : public base_operation
   {
      account_name_type               issuer;                  ///< Name of the issuing account, can create units and administrate the asset.
      
      asset_symbol_type               symbol;                  ///< The ticker symbol of this asset.

      string                          asset_type;              ///< The type of the asset. Determines asset characteristics and features.

      asset                           coin_liquidity;          ///< Amount of COIN asset to inject into the Coin liquidity pool.  

      asset                           usd_liquidity;           ///< Amount of USD asset to inject into the USD liquidity pool.

      asset                           credit_liquidity;        ///< Amount of the new asset to issue and inject into the credit pool.

      asset_options                   options;                 ///< Series of options parameters that apply to each asset type.

      void                            validate()const;
      void                            get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                            get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
   };


   /**
    * Updates an Asset to use a new set of options.
    */
   struct asset_update_operation : public base_operation
   {
      account_name_type             issuer;                   ///< Account that issued the asset.

      asset_symbol_type             asset_to_update;          ///< Asset symbol that is being updated.

      asset_options                 new_options;              ///< Options used for all asset types.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
   };


   /**
    * Issues an amount of an asset to a specified account.
    * 
    * The asset must be user issued, cannot be market issued.
    */
   struct asset_issue_operation : public base_operation
   {
      account_name_type             issuer;               ///< The issuer of the asset.

      asset                         asset_to_issue;       ///< Amount of asset being issued to the account.

      account_name_type             issue_to_account;     ///< Account receiving the newly issued asset.

      string                        memo;                 ///< The memo for the transaction, encryption on the memo is advised.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
   };


   /**
    * Takes a specified amount of an asset out of circulation.
    * 
    * Returns the asset to the reserved, available supply for new issuance.
    */
   struct asset_reserve_operation : public base_operation
   {
      account_name_type             payer;               ///< Account that is reserving the asset back to the unissued supply.

      asset                         amount_to_reserve;   ///< Amount of the asset being reserved.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( payer ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( payer ); }
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
      account_name_type             issuer;             ///< The current issuer of the asset.

      asset_symbol_type             asset_to_update;    ///< The asset being updated.

      account_name_type             new_issuer;         ///< Name of the account specified to become the new issuer of the asset.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                          get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
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
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
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
      account_name_type             sender;                    ///< The account which sent the amount into the distribution.

      asset_symbol_type             distribution_asset;        ///< Distribution asset for the fund to be sent to.

      asset                         amount;                    ///< Asset amount being sent for distribution.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( sender ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( sender ); }
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
      account_name_type             account;              ///< The account exercising the option asset.

      asset                         amount;               ///< Option assets being exercised by exchanging the quoted asset for the underlying. 

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   /**
    * Adds funds to the redemption pool of a stimulus asset.
    * 
    * Stimulus assets are redeemable for the underlying redemption pool 
    * and all balances are redistributed once per month. 
    */
   struct asset_stimulus_fund_operation : public base_operation
   {
      account_name_type             account;              ///< The account funding the stimulus asset.

      asset_symbol_type             stimulus_asset;       ///< Asset symbol of the asset to add stimulus funds to.

      asset                         amount;               ///< Redemption asset being injected into the redemption pool.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
    * in new_feed_producers, which were already feed producers
    * prior to execution of this operation, will be preserved.
    * 
    * Design inspired by Bitshares Protocol:
    * https://www.bitshares.foundation/download/articles/BitSharesBlockchain.pdf
    */
   struct asset_update_feed_producers_operation : public base_operation
   {
      account_name_type               issuer;                  ///< The issuer of the BitAsset.

      asset_symbol_type               asset_to_update;         ///< The BitAsset being updated.

      flat_set< account_name_type >   new_feed_producers;      ///< Set of accounts that can determine the price feed of the asset.

      void                            validate()const;
      void                            get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                            get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
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
      account_name_type             publisher;     ///< Account publishing the price feed.

      asset_symbol_type             symbol;        ///< Asset for which the feed is published.

      price_feed                    feed;          ///< Exchange rate between stablecoin and backing asset.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( publisher ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( publisher ); }
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
      account_name_type             account;          ///< Account requesting the force settlement.
      
      asset                         amount;           ///< Amount of asset to force settle. Set to 0 to cancel order.

      account_name_type             interface;        ///< Account of the interface used to broadcast the operation.

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
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
      account_name_type             issuer;              ///< Issuer of the asset being settled. 

      asset_symbol_type             asset_to_settle;     ///< Symbol of the asset being settled. 

      price                         settle_price;        ///< Global settlement price, must be in asset / backing asset. 

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( issuer ); }
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
      account_name_type        bidder;        ///< Adds additional collateral to the market issued asset.

      asset                    collateral;    ///< The amount of collateral to bid for the debt.

      asset                    debt;          ///< The amount of debt to take over.

      void                     validate()const;
      void                     get_creator_name( flat_set<account_name_type>& a )const{ a.insert( bidder ); }
      void                     get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( bidder ); }
   };
   

   //=====================================//
   //==== Block Production Operations ====//
   //=====================================//



   /**
    * Chain Properties object determines the fundamental parameters of the network.
    * 
    * Each Block Producer broadcasts this set of variables and the 
    * protocol determines the median value.
    * 
    * In order to increase or decrease a value, at least half
    * of the producers must concurrently change the value.
    * 
    * Includes the content reward curve,
    * the times used for decaying network processes,
    * the Percentages used for determining interest rates,
    * the reserve rates of content curation interactions,
    * the account membership prices, 
    * and the reserve ratios of credit mechanisms
    */
   struct chain_properties
   {
      comment_reward_curve   reward_curve = comment_reward_curve();                         ///< The Parameters defining the content reward distribution curve.

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

      void validate()const;
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
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( owner ); }
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
    * Proofs of Work make the blockchain thermodynamically 
    * secure against rewritingdue to the energy expenditure 
    * required to redo the work.
    * 
    * The Network uses the X11 Mining algorithm for hashing.
    * 
    * Miners are added to the block production set according to 
    * the number of proofs of work in the prior 7 days.
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
      account_name_type             producer;      ///< The name of the block producing account.

      block_id_type                 block_id;      ///< The Block ID of the block being verifed as valid and received. 

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( producer ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( producer ); }
   };


   /**
    * Stakes COIN on the validity and acceptance of a block.
    * 
    * The commit block operation enables producers to assert that:
    * 
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
      account_name_type                 producer;            ///< The name of the block producing account.

      block_id_type                     block_id;            ///< The block id of the block being committed as irreversible to that producer.

      flat_set< account_name_type >     verifications;       ///< The set of attesting verifications from currently active producers.

      asset                             commitment_stake;    ///< The value of staked balance that the producer stakes on this commitment. Must be at least one unit of COIN. 

      void                              validate()const;
      void                              get_creator_name( flat_set<account_name_type>& a )const{ a.insert( producer ); }
      void                              get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( producer ); }
   };


   /** 
    * Declares a violation of a block commitment.
    * 
    * Enables a reporter to claim the commitment stake of that producer.
    */
   struct producer_violation_operation : public base_operation
   {
      account_name_type             reporter;      ///< The account detecting and reporting the violation.

      vector<char>                  first_trx;     ///< The first transaction signed by the producer.

      vector<char>                  second_trx;    ///< The second transaction that is in contravention of the first commitment transaction. 

      void                          validate()const;
      void                          get_creator_name( flat_set<account_name_type>& a )const{ a.insert( reporter ); }
      void                          get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( reporter ); }
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
         (registrar)
         (new_account_name)
         (referrer)
         (proxy)
         (recovery_account)
         (reset_account)
         (details)
         (url)
         (profile_image)
         (cover_image)
         (json)
         (json_private)
         (first_name)
         (last_name)
         (gender)
         (date_of_birth)
         (email)
         (phone)
         (nationality)
         (relationship)
         (political_alignment)
         (interests)
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
         (account)
         (details)
         (url)
         (profile_image)
         (cover_image)
         (json)
         (json_private)
         (first_name)
         (last_name)
         (gender)
         (date_of_birth)
         (email)
         (phone)
         (nationality)
         (relationship)
         (political_alignment)
         (pinned_permlink)
         (interests)
         (owner_auth)
         (active_auth)
         (posting_auth)
         (secure_public_key)
         (connection_public_key)
         (friend_public_key)
         (companion_public_key)
         (active)
         );

FC_REFLECT( node::protocol::account_verification_operation,
         (verifier_account)
         (verified_account)
         (shared_image)
         (verified)
         );

FC_REFLECT( node::protocol::account_membership_operation,
         (account)
         (membership_type)
         (months)
         (interface)
         (recurring)
         );

FC_REFLECT( node::protocol::account_update_list_operation,
         (account)
         (listed_account)
         (listed_asset)
         (blacklisted)
         (whitelisted)
         );

FC_REFLECT( node::protocol::account_producer_vote_operation,
         (account)
         (vote_rank)
         (producer)
         (approved)
         );

FC_REFLECT( node::protocol::account_update_proxy_operation, 
         (account)
         (proxy) 
         );

FC_REFLECT( node::protocol::account_request_recovery_operation, 
         (recovery_account)
         (account_to_recover)
         (new_owner_authority)
         );

FC_REFLECT( node::protocol::account_recover_operation, 
         (account_to_recover)
         (new_owner_authority)
         (recent_owner_authority)
         );

FC_REFLECT( node::protocol::account_reset_operation, 
         (reset_account)
         (account_to_reset)
         (new_owner_authority) 
         );

FC_REFLECT( node::protocol::account_reset_update_operation, 
         (account)
         (new_reset_account)
         (days)
         );

FC_REFLECT( node::protocol::account_recovery_update_operation, 
         (account_to_recover)
         (new_recovery_account)
         );

FC_REFLECT( node::protocol::account_decline_voting_operation,
         (account)
         (declined)
         );

FC_REFLECT( node::protocol::account_connection_operation,
         (account)
         (connecting_account)
         (connection_id)
         (connection_type)
         (message)
         (json)
         (encrypted_key)
         (connected)
         );

FC_REFLECT( node::protocol::account_follow_operation,
         (follower)
         (following)
         (interface)
         (added)
         (followed)
         );

FC_REFLECT( node::protocol::account_follow_tag_operation, 
         (follower)
         (tag)
         (interface)
         (added)
         (followed)
         );

FC_REFLECT( node::protocol::account_activity_operation, 
         (account)
         (permlink)
         (interface)
         );


   //============================//
   //==== Business Operations ===//
   //============================//


FC_REFLECT( node::protocol::business_create_operation,
         (founder)
         (new_business_name)
         (new_business_trading_name)
         (details)
         (url)
         (profile_image)
         (cover_image)
         (secure_public_key)
         (connection_public_key)
         (friend_public_key)
         (companion_public_key)
         (interface)
         (equity_asset)
         (equity_revenue_share)
         (equity_options)
         (credit_asset)
         (credit_revenue_share)
         (credit_options)
         (public_community)
         (public_display_name)
         (public_community_member_key)
         (public_community_moderator_key)
         (public_community_admin_key)
         (public_community_secure_key)
         (public_community_standard_premium_key)
         (public_community_mid_premium_key)
         (public_community_top_premium_key)
         (private_community)
         (private_display_name)
         (private_community_member_key)
         (private_community_moderator_key)
         (private_community_admin_key)
         (private_community_secure_key)
         (private_community_standard_premium_key)
         (private_community_mid_premium_key)
         (private_community_top_premium_key)
         (reward_currency)
         (standard_membership_price)
         (mid_membership_price)
         (top_membership_price)
         (coin_liquidity)
         (usd_liquidity)
         (credit_liquidity)
         (fee)
         (delegation)
         );

FC_REFLECT( node::protocol::business_update_operation,
         (chief_executive)
         (business)
         (business_trading_name)
         (equity_revenue_share)
         (credit_revenue_share)
         (executives)
         (active )
         );

FC_REFLECT( node::protocol::business_executive_operation,
         (executive)
         (business)
         (active)
         );

FC_REFLECT( node::protocol::business_executive_vote_operation,
         (director)
         (executive)
         (business)
         (approved)
         );

FC_REFLECT( node::protocol::business_director_operation,
         (director)
         (business)
         (active)
         );

FC_REFLECT( node::protocol::business_director_vote_operation,
         (account)
         (director)
         (business)
         (vote_rank)
         (approved)
         );

   //==============================//
   //==== Governance Operations ===//
   //==============================//


FC_REFLECT( node::protocol::governance_create_operation,
         (founder)
         (new_governance_name)
         (new_governance_display_name)
         (details)
         (url)
         (profile_image)
         (cover_image)
         (secure_public_key)
         (connection_public_key)
         (friend_public_key)
         (companion_public_key)
         (interface)
         (equity_asset)
         (equity_revenue_share)
         (equity_options)
         (credit_asset)
         (credit_revenue_share)
         (credit_options)
         (public_community)
         (public_display_name)
         (public_community_member_key)
         (public_community_moderator_key)
         (public_community_admin_key)
         (public_community_secure_key)
         (public_community_standard_premium_key)
         (public_community_mid_premium_key)
         (public_community_top_premium_key)
         (private_community)
         (private_display_name)
         (private_community_member_key)
         (private_community_moderator_key)
         (private_community_admin_key)
         (private_community_secure_key)
         (private_community_standard_premium_key)
         (private_community_mid_premium_key)
         (private_community_top_premium_key)
         (reward_currency)
         (standard_membership_price)
         (mid_membership_price)
         (top_membership_price)
         (coin_liquidity)
         (usd_liquidity)
         (credit_liquidity)
         (fee)
         (delegation)
         );

FC_REFLECT( node::protocol::governance_update_operation,
         (chief_executive)
         (governance)
         (governance_display_name)
         (equity_revenue_share)
         (credit_revenue_share)
         (executives)
         (active )
         );

FC_REFLECT( node::protocol::governance_executive_operation,
         (executive)
         (governance)
         (active)
         );

FC_REFLECT( node::protocol::governance_executive_vote_operation,
         (director)
         (executive)
         (governance)
         (approved)
         );

FC_REFLECT( node::protocol::governance_director_operation,
         (director)
         (governance)
         (active)
         );

FC_REFLECT( node::protocol::governance_director_vote_operation,
         (account)
         (director)
         (governance)
         (vote_rank)
         (approved)
         );

FC_REFLECT( node::protocol::governance_member_operation,
         (governance)
         (account)
         (interface)
         (approved)
         );

FC_REFLECT( node::protocol::governance_member_request_operation,
         (account)
         (governance)
         (interface)
         (message)
         (active)
         );

FC_REFLECT( node::protocol::governance_resolution_operation,
         (governance)
         (resolution_id)
         (ammendment_id)
         (title)
         (details)
         (body)
         (url)
         (json)
         (interface)
         (completion_time)
         (active)
         );

FC_REFLECT( node::protocol::governance_resolution_vote_operation,
         (account)
         (governance)
         (resolution_id)
         (ammendment_id)
         (interface)
         (approved)
         (active)
         );


   //===========================//
   //==== Network Operations ===//
   //===========================//


FC_REFLECT( node::protocol::network_officer_update_operation, 
         (account)
         (officer_type)
         (details)
         (url)
         (json)
         (reward_currency)
         (active)
         );

FC_REFLECT( node::protocol::network_officer_vote_operation, 
         (account)
         (network_officer)
         (vote_rank)
         (approved)
         );

FC_REFLECT( node::protocol::supernode_update_operation, 
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

FC_REFLECT( node::protocol::interface_update_operation, 
         (account)
         (details)
         (url)
         (json)
         (active)
         );

FC_REFLECT( node::protocol::mediator_update_operation, 
         (account)
         (details)
         (url)
         (json)
         (mediator_bond)
         (active)
         );

FC_REFLECT( node::protocol::enterprise_update_operation, 
         (account)
         (enterprise_id)
         (details)
         (url)
         (json)
         (budget)
         (active)
         );

FC_REFLECT( node::protocol::enterprise_vote_operation, 
         (voter)
         (account)
         (enterprise_id)
         (vote_rank)
         (approved)
         );

FC_REFLECT( node::protocol::enterprise_fund_operation, 
         (funder)
         (account)
         (enterprise_id)
         (amount)
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
         (channel)
         (allow_replies)
         (allow_votes)
         (allow_views)
         (allow_shares)
         (allow_curation_rewards)
         (beneficiaries)
         );

FC_REFLECT( node::protocol::comment_reward_curve,
         (constant_factor)
         (sqrt_percent)
         (linear_percent)
         (semi_quadratic_percent)
         (quadratic_percent)
         (reward_interval_amount)
         (reward_interval_hours)
         (author_reward_percent)
         (vote_reward_percent)
         (view_reward_percent)
         (share_reward_percent)
         (comment_reward_percent)
         (storage_reward_percent)
         (moderator_reward_percent)
         );

FC_REFLECT( node::protocol::comment_operation,
         (editor)
         (author)
         (permlink)
         (parent_author)
         (parent_permlink)
         (title)
         (body)
         (body_private)
         (url)
         (url_private)
         (ipfs)
         (ipfs_private)
         (magnet)
         (magnet_private)
         (json)
         (json_private)
         (language)
         (public_key)
         (community)
         (tags)
         (collaborating_authors)
         (supernodes)
         (interface)
         (latitude)
         (longitude)
         (comment_price)
         (premium_price)
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

FC_REFLECT( node::protocol::comment_vote_operation,
         (voter)
         (author)
         (permlink)
         (weight)
         (interface)
         (reaction)
         (json)
         );

FC_REFLECT( node::protocol::comment_view_operation,
         (viewer)
         (author)
         (permlink)
         (interface)
         (supernode)
         (json)
         (viewed)
         );

FC_REFLECT( node::protocol::comment_share_operation,
         (sharer)
         (author)
         (permlink)
         (reach)
         (interface)
         (communities)
         (tags)
         (json)
         (shared)
         );

FC_REFLECT( node::protocol::comment_moderation_operation,
         (moderator)
         (author)
         (permlink)
         (tags)
         (rating)
         (details)
         (json)
         (interface)
         (filter)
         (removal_requested)
         (beneficiaries_requested)
         (applied)
         );

FC_REFLECT( node::protocol::message_operation,
         (sender)
         (recipient)
         (community)
         (public_key)
         (message)
         (ipfs)
         (json)
         (uuid)
         (interface)
         (parent_sender)
         (parent_uuid)
         (expiration)
         (forward)
         (active)
         );

FC_REFLECT( node::protocol::list_operation,
         (creator)
         (list_id)
         (name)
         (details)
         (json)
         (interface)
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
         (active)
         );

FC_REFLECT( node::protocol::poll_operation,
         (creator)
         (poll_id)
         (community)
         (public_key)
         (interface)
         (details)
         (json)
         (poll_option_0)
         (poll_option_1)
         (poll_option_2)
         (poll_option_3)
         (poll_option_4)
         (poll_option_5)
         (poll_option_6)
         (poll_option_7)
         (poll_option_8)
         (poll_option_9)
         (completion_time)
         (active)
         );

FC_REFLECT( node::protocol::poll_vote_operation,
         (voter)
         (creator)
         (poll_id)
         (interface)
         (poll_option)
         (active)
         );

FC_REFLECT( node::protocol::premium_purchase_operation,
         (account)
         (author)
         (permlink)
         (interface)
         (purchased)
         );

FC_REFLECT( node::protocol::premium_release_operation,
         (provider)
         (account)
         (author)
         (permlink)
         (interface)
         (encrypted_key)
         );


   //==============================//
   //==== Community Operations ====//
   //==============================//


FC_REFLECT( node::protocol::community_create_operation,
         (founder)
         (name)
         (display_name)
         (details)
         (url)
         (profile_image)
         (cover_image)
         (json)
         (json_private)
         (tags)
         (private_community)
         (channel)
         (author_permission)
         (reply_permission)
         (vote_permission)
         (view_permission)
         (share_permission)
         (message_permission)
         (poll_permission)
         (event_permission)
         (directive_permission)
         (add_permission)
         (request_permission)
         (remove_permission)
         (community_member_key)
         (community_moderator_key)
         (community_admin_key)
         (community_secure_key)
         (community_standard_premium_key)
         (community_mid_premium_key)
         (community_top_premium_key)
         (interface)
         (reward_currency)
         (standard_membership_price)
         (mid_membership_price)
         (top_membership_price)
         (verifiers)
         (min_verification_count)
         (max_verification_distance)
         (max_rating)
         (flags)
         (permissions)
         );

FC_REFLECT( node::protocol::community_update_operation,
         (account)
         (community)
         (display_name)
         (details)
         (url)
         (profile_image)
         (cover_image)
         (json)
         (json_private)
         (pinned_author)
         (pinned_permlink)
         (tags)
         (private_community)
         (channel)
         (author_permission)
         (reply_permission)
         (vote_permission)
         (view_permission)
         (share_permission)
         (message_permission)
         (poll_permission)
         (event_permission)
         (directive_permission)
         (add_permission)
         (request_permission)
         (remove_permission)
         (community_member_key)
         (community_moderator_key)
         (community_admin_key)
         (community_secure_key)
         (community_standard_premium_key)
         (community_mid_premium_key)
         (community_top_premium_key)
         (standard_membership_price)
         (mid_membership_price)
         (top_membership_price)
         (verifiers)
         (min_verification_count)
         (max_verification_distance)
         (max_rating)
         (flags)
         (permissions)
         (active)
         );

FC_REFLECT( node::protocol::community_member_operation,
         (account)
         (member)
         (community)
         (interface)
         (member_type)
         (encrypted_community_key)
         (expiration)
         (accepted)
         );

FC_REFLECT( node::protocol::community_member_request_operation,
         (account)
         (community)
         (interface)
         (member_type)
         (message)
         (expiration)
         (requested)
         );

FC_REFLECT( node::protocol::community_member_vote_operation,
         (account)
         (community)
         (member)
         (interface)
         (vote_rank)
         (approved)
         );

FC_REFLECT( node::protocol::community_subscribe_operation,
         (account)
         (community)
         (interface)
         (added)
         (subscribed)
         );

FC_REFLECT( node::protocol::community_blacklist_operation,
         (account)
         (member)
         (community)
         (blacklisted)
         );

FC_REFLECT( node::protocol::community_federation_operation,
         (account)
         (federation_id)
         (community)
         (federated_community)
         (message)
         (json)
         (federation_type)
         (encrypted_community_key)
         (share_accounts)
         (accepted)
         );

FC_REFLECT( node::protocol::community_event_operation,
         (account)
         (community)
         (event_id)
         (public_key)
         (event_name)
         (location)
         (latitude)
         (longitude)
         (details)
         (url)
         (json)
         (interface)
         (event_price)
         (event_start_time)
         (event_end_time)
         (active)
         );

FC_REFLECT( node::protocol::community_event_attend_operation,
         (attendee)
         (community)
         (event_id)
         (public_key)
         (message)
         (json)
         (interface)
         (interested)
         (attending)
         (active)
         );

FC_REFLECT( node::protocol::community_directive_operation,
         (account)
         (directive_id)
         (parent_account)
         (parent_directive_id)
         (community)
         (public_key)
         (interface)
         (details)
         (cover_image)
         (ipfs)
         (json)
         (directive_start_time)
         (directive_end_time)
         (completed)
         (active)
         );

FC_REFLECT( node::protocol::community_directive_vote_operation,
         (voter)
         (account)
         (directive_id)
         (public_key)
         (interface)
         (details)
         (json)
         (approve)
         (active)
         );

FC_REFLECT( node::protocol::community_directive_member_operation,
         (account)
         (community)
         (interface)
         (public_key)
         (details)
         (json)
         (command_directive_id)
         (delegate_directive_id)
         (consensus_directive_id)
         (emergent_directive_id)
         (active)
         );

FC_REFLECT( node::protocol::community_directive_member_vote_operation,
         (voter)
         (member)
         (community)
         (public_key)
         (interface)
         (details)
         (json)
         (approve)
         (active)
         );


   //================================//
   //==== Advertising Operations ====//
   //================================//


FC_REFLECT( node::protocol::ad_creative_operation,
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
         (account)
         (audience_id)
         (json)
         (audience)
         (active)
         );

FC_REFLECT( node::protocol::ad_bid_operation,
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
         (from)
         (to)
         (amount)
         (memo)
         );

FC_REFLECT( node::protocol::transfer_request_operation,
         (to)
         (from)
         (amount)
         (memo)
         (request_id)
         (expiration)
         (requested)
         );

FC_REFLECT( node::protocol::transfer_accept_operation,
         (from)
         (to)
         (request_id)
         (accepted)
         );

FC_REFLECT( node::protocol::transfer_recurring_operation,
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
         (account)
         (reward)
         );

FC_REFLECT( node::protocol::stake_asset_operation,
         (from)
         (to)
         (amount) 
         );

FC_REFLECT( node::protocol::unstake_asset_operation, 
         (from)
         (to)
         (amount) 
         );

FC_REFLECT( node::protocol::unstake_asset_route_operation, 
         (from)
         (to)
         (percent)
         (auto_stake)
         );

FC_REFLECT( node::protocol::transfer_to_savings_operation,
         (from)
         (to)
         (amount)
         (memo) 
         );

FC_REFLECT( node::protocol::transfer_from_savings_operation, 
         (from)
         (to)
         (amount)
         (request_id)
         (memo)
         (transferred)
         );

FC_REFLECT( node::protocol::delegate_asset_operation, 
         (delegator)
         (delegatee)
         (amount)
         );



   //================================//
   //==== Marketplace Operations ====//
   //================================//


FC_REFLECT( node::protocol::product_sale_operation, 
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
         (buyer)
         (order_id)
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
         (buyer)
         (bid_id)
         (seller)
         (auction_id)
         (bid_price_commitment)
         (blinding_factor)
         (public_bid_amount)
         (memo)
         (json)
         (shipping_address)
         (delivery_variant)
         (delivery_details)
         );

FC_REFLECT( node::protocol::escrow_transfer_operation, 
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
         (account)
         (mediator)
         (escrow_from)
         (escrow_id)
         (approved)
         );

FC_REFLECT( node::protocol::escrow_dispute_operation, 
         (account)
         (escrow_from)
         (escrow_id) 
         );

FC_REFLECT( node::protocol::escrow_release_operation, 
         (account)
         (escrow_from)
         (escrow_id)
         (release_percent)
         );



   //============================//
   //==== Trading Operations ====//
   //============================//



FC_REFLECT( node::protocol::limit_order_operation, 
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
         (owner)
         (order_id)
         (amount_to_sell)
         (limit_close_price)
         (interface)
         (expiration)
         (opened)
         );

FC_REFLECT( node::protocol::call_order_operation, 
         (owner)
         (collateral)
         (debt)
         (target_collateral_ratio)
         (interface)
         );

FC_REFLECT( node::protocol::option_order_operation, 
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
         (account)
         (first_amount)
         (second_amount)
         );

FC_REFLECT( node::protocol::liquidity_pool_exchange_operation, 
         (account)
         (amount)
         (receive_asset)
         (interface)
         (limit_price)
         (acquire)
         );

FC_REFLECT( node::protocol::liquidity_pool_fund_operation, 
         (account)
         (amount)
         (pair_asset)
         );

FC_REFLECT( node::protocol::liquidity_pool_withdraw_operation, 
         (account)
         (amount)
         (receive_asset)
         );

FC_REFLECT( node::protocol::credit_pool_collateral_operation, 
         (account)
         (amount)
         );

FC_REFLECT( node::protocol::credit_pool_borrow_operation, 
         (account)
         (amount)
         (collateral)
         (loan_id)
         (flash_loan)
         );

FC_REFLECT( node::protocol::credit_pool_lend_operation, 
         (account)
         (amount)
         );

FC_REFLECT( node::protocol::credit_pool_withdraw_operation, 
         (account)
         (amount)
         );

FC_REFLECT( node::protocol::option_pool_create_operation, 
         (account)
         (first_asset)
         (second_asset)
         );

FC_REFLECT( node::protocol::prediction_pool_create_operation, 
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
         (account)
         (amount)
         (prediction_asset)
         (exchange_base)
         (withdraw)
         );

FC_REFLECT( node::protocol::prediction_pool_resolve_operation, 
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
         (enterprise_fund_reward_percent)
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
         (ownership_asset)
         (control_list)
         (access_list)
         (access_price)
         (value)
         (collateralization)
         (coupon_rate_percent)
         (maturity_date)
         (redemption_asset)
         (redemption_price)
         (distribution_list)
         (redemption_list)
         (distribution_amount)
         );

FC_REFLECT( node::protocol::asset_create_operation,
         (issuer)
         (symbol)
         (asset_type)
         (coin_liquidity)
         (usd_liquidity)
         (credit_liquidity)
         (options)
         );

FC_REFLECT( node::protocol::asset_update_operation,
         (issuer)
         (asset_to_update)
         (new_options)
         );

FC_REFLECT( node::protocol::asset_issue_operation, 
         (issuer)
         (asset_to_issue)
         (issue_to_account)
         (memo)
         );

FC_REFLECT( node::protocol::asset_reserve_operation, 
         (payer)
         (amount_to_reserve)
         );

FC_REFLECT( node::protocol::asset_update_issuer_operation,
         (issuer)
         (asset_to_update)
         (new_issuer)
         );

FC_REFLECT( node::protocol::asset_distribution_operation,
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
         (sender)
         (distribution_asset)
         (amount)
         );

FC_REFLECT( node::protocol::asset_option_exercise_operation,
         (account)
         (amount)
         );

FC_REFLECT( node::protocol::asset_stimulus_fund_operation,
         (account)
         (stimulus_asset)
         (amount)
         );

FC_REFLECT( node::protocol::asset_update_feed_producers_operation,
         (issuer)
         (asset_to_update)
         (new_feed_producers)
         );

FC_REFLECT( node::protocol::asset_publish_feed_operation, 
         (publisher)
         (symbol)
         (feed)
         );

FC_REFLECT( node::protocol::asset_settle_operation, 
         (account)
         (amount)
         (interface)
         );

FC_REFLECT( node::protocol::asset_global_settle_operation, 
         (issuer)
         (asset_to_settle)
         (settle_price)
         );

FC_REFLECT( node::protocol::asset_collateral_bid_operation, 
         (bidder)
         (collateral)
         (debt)
         );


   //=====================================//
   //==== Block Production Operations ====//
   //=====================================//


FC_REFLECT( node::protocol::chain_properties,
         (reward_curve)
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
         );

FC_REFLECT( node::protocol::producer_update_operation, 
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
         (producer)
         (block_id)
         );

FC_REFLECT( node::protocol::commit_block_operation,
         (producer)
         (block_id)
         (verifications)
         (commitment_stake)
         );

FC_REFLECT( node::protocol::producer_violation_operation,
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