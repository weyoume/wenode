#pragma once

#include <node/app/api.hpp>
#include <node/private_message/private_message_plugin.hpp>
#include <node/follow/follow_plugin.hpp>
#include <node/app/node_api_objects.hpp>

#include <graphene/utilities/key_conversion.hpp>

#include <fc/real128.hpp>
#include <fc/crypto/base58.hpp>

using namespace node::app;
using namespace node::chain;
using namespace graphene::utilities;
using namespace std;

namespace node { namespace wallet {

using node::app::discussion;
using namespace node::private_message;

typedef uint16_t transaction_handle_type;

struct memo_data {

   static optional<memo_data> from_string( string str ) {
      try {
         if( str.size() > sizeof(memo_data) && str[0] == '#') {
            auto data = fc::from_base58( str.substr(1) );
            auto m  = fc::raw::unpack<memo_data>( data );
            FC_ASSERT( string(m) == str );
            return m;
         }
      } catch ( .. ) {}
      return optional<memo_data>();
   }

   public_key_type from;
   public_key_type to;
   uint64_t        nonce = 0;
   uint32_t        check = 0;
   vector<char>    encrypted;

   operator string()const {
      auto data = fc::raw::pack(*this);
      auto base58 = fc::to_base58( data );
      return '#'+base58;
   }
};

struct brain_key_info
{
   string               brain_priv_key;
   public_key_type      pub_key;
   string               wif_priv_key;
};

struct wallet_data
{
   vector<char>              cipher_keys; /** encrypted keys */

   string                    ws_server = "ws://localhost:8090";
   string                    ws_user;
   string                    ws_password;
};

enum authority_type
{
   owner,
   active,
   posting
};

namespace detail {
class wallet_api_impl;
}

/**
 * This wallet assumes it is connected to the database server with a high-bandwidth, low-latency connection and
 * performs minimal caching. This API could be provided locally to be used by a web interface.
 */
class wallet_api
{
   public:
      wallet_api( const wallet_data& initial_data, fc::api<login_api> rapi );
      virtual ~wallet_api();


      //====================//
      // === Wallet API === //
      //====================//


      bool copy_wallet_file( string destination_filename );


      /** 
       * Returns a list of all commands supported by the wallet API.
       *
       * This lists each command, along with its arguments and return types.
       * For more detailed help on a single command, use \c get_help()
       *
       * @returns A multi-line string of Help Information.
       */
      string                                 help()const;

      /**
       * Returns info about the current state of the blockchain.
       */
      variant                                info();

      /** 
       * Returns Client information.
       * 
       * Includes client version, git version of graphene/fc, version of boost, openssl.
       * 
       * @returns compile time info and client and dependencies versions.
       */
      variant_object                         about() const;

      /**
       * Gets the account information for all accounts for which this wallet has a private key.
       */
      vector<account_api_obj>                list_my_accounts();

      /** 
       * Returns the current wallet filename.
       *
       * This is the filename that will be used when automatically saving the wallet.
       *
       * @see set_wallet_filename()
       * 
       * @return the wallet filename.
       */
      string                                 get_wallet_filename() const;

      /** 
       * Checks whether the wallet has just been created and has not yet had a password set.
       *
       * Calling \c set_password will transition the wallet to the locked state.
       * 
       * @return true if the wallet is new.
       * @ingroup Wallet Management
       */
      bool                                   is_new()const;

      /** 
       * Checks whether the wallet is locked (is unable to use its private keys).
       *
       * This state can be changed by calling \c lock() or \c unlock().
       * 
       * @return true if the wallet is locked.
       * @ingroup Wallet Management
       */
      bool                                   is_locked()const;

      /** 
       * Locks the wallet.
       * 
       * @ingroup Wallet Management
       */
      void                                   lock();

      /** 
       * Unlocks the wallet.
       *
       * The wallet remain unlocked until the \c lock is called
       * or the program exits.
       * 
       * @param password the password previously set with \c set_password()
       * @ingroup Wallet Management
       */
      void                                   unlock( string password );

      /** 
       * Sets a new password on the wallet.
       *
       * The wallet must be either 'new' or 'unlocked' to
       * execute this command.
       * @ingroup Wallet Management
       */
      void                                   set_password( string password );

      /** 
       * Returns detailed help on a single API command.
       * 
       * @param method the name of the API command you want help with.
       * @returns a multi-line string suitable for displaying on a terminal.
       */
      string                                 gethelp( const string& method )const;

      /** 
       * Loads a specified Graphene wallet file.
       *
       * The current wallet is closed before the new wallet is loaded.
       *
       * @warning This does not change the filename that will be used for future
       * wallet writes, so this may cause you to overwrite your original
       * wallet unless you also call \c set_wallet_filename()
       *
       * @param wallet_filename the filename of the wallet JSON file to load.
       *                        If \c wallet_filename is empty, it reloads the
       *                        existing wallet file.
       * @returns true if the specified wallet is loaded.
       */
      bool                                   load_wallet_file( string wallet_filename = "" );

      /** 
       * Saves the current wallet to the given filename.
       *
       * @warning This does not change the wallet filename that will be used for future
       * writes, so think of this function as 'Save a Copy As..' instead of
       * 'Save As..'.  Use \c set_wallet_filename() to make the filename
       * persist.
       * @param wallet_filename the filename of the new wallet JSON file to create
       *                        or overwrite.  If \c wallet_filename is empty,
       *                        save to the current filename.
       */
      void                                   save_wallet_file( string wallet_filename = "" );

      /** 
       * Sets the wallet filename used for future writes.
       *
       * This does not trigger a save, it only changes the default filename
       * that will be used the next time a save is triggered.
       *
       * @param wallet_filename the new filename to use for future saves
       */
      void                                   set_wallet_filename( string wallet_filename );

      /** 
       * Converts a signed_transaction in JSON form to its binary representation.
       *
       * @param tx the transaction to serialize.
       * @returns the binary form of the transaction.  It will not be hex encoded,
       *          this returns a raw string that may have null characters embedded in it
       */
      string                                 serialize_transaction( signed_transaction tx ) const;

     

      //=================//
      // === Key API === //
      //=================//


      /**
       * Get the WIF private key corresponding to a public key. 
       * The private key must already be in the wallet.
       */
      string                                 get_private_key( public_key_type pubkey )const;

      /**
       *  @param role - active | owner | posting | memo
       */
      pair< public_key_type, string >        get_private_key_from_password( string account, string role, string password )const;

      /** 
       * Suggests a safe brain key to use for creating your account.
       * 
       * \c create_account_with_brain_key() requires you to specify a 'brain key',
       * a long passphrase that provides enough entropy to generate cryptographic
       * keys. This function will suggest a suitably random string that should
       * be easy to write down (and, with effort, memorize).
       * 
       * @returns A suggested brain_key.
       */
      brain_key_info                         suggest_brain_key()const;

      /** 
       * Transforms a brain key to reduce the chance of errors when re-entering the key from memory.
       *
       * This takes a user-supplied brain key and
       * normalizes it into the form used
       * for generating private keys. 
       * In particular, this upper-cases all ASCII characters
       * and collapses multiple spaces into one.
       * 
       * @param s the brain key as supplied by the user
       * @returns the brain key in its normalized form
       */
      string                                 normalize_brain_key( string s ) const;

      /**
       * Dumps all private keys owned by the wallet.
       *
       * The keys are printed in WIF format. Import these keys into another wallet
       * using \c import_key()
       * 
       * @returns map containing the wallet's private keys, indexed by their public key.
       */
      map<public_key_type, string>           list_keys();

      /** 
       * Imports a WIF Private Key into the wallet to be used to sign transactions by an account.
       *
       * example: import_key 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
       *
       * @param wif_key the WIF Private Key to import
       */
      bool                                   import_key( string wif_key );

      

      //====================//
      // === Global API === //
      //====================//


      /** 
       * Returns the blockchain's rapidly-changing properties.
       * 
       * The returned @ref dynamic_global_property_object contains
       * information that changes every block interval
       * such as the head block number, and the next witness.
       * 
       * @returns The current dynamic global properties.
       */
      dynamic_global_property_api_obj                 get_dynamic_global_properties() const;


      //======================//
      // === Account API ==== //
      //======================//


      /** 
       * Returns information about the given account.
       *
       * @param name The name of the account to provide information about.
       * @returns Account Object information pertaining to the specified account.
       */
      account_api_obj                                 get_account( string name ) const;

      /** 
       * Returns information about a list of given accounts.
       *
       * @param names The name of the accounts to provide information about.
       * @returns Account Object information pertaining to the specified accounts.
       */
      vector< account_api_obj >                       get_accounts( vector< string > names ) const;

      vector< account_api_obj >                       get_accounts_by_followers( string from, uint32_t limit ) const;

      account_concise_api_obj                         get_concise_account( string name ) const;

      vector< account_concise_api_obj >               get_concise_accounts( vector< string > names ) const;

      extended_account                                get_full_account( string name ) const;

      vector< extended_account >                      get_full_accounts( vector< string > names ) const;

      /**
       * Account operations have sequence numbers from 0 to N where N is the most recent operation.
       * 
       * @param account Account whose history will be returned.
       * @param from The absolute sequence number, -1 means most recent, limit is the number of operations before from.
       * @param limit The maximum number of items that can be queried (0 to 1000], must be less than from.
       * @returns Operations in the range [from-limit, from].
       */
      map< uint32_t, applied_operation >              get_account_history( string account, uint32_t from, uint32_t limit );

      vector< message_state >                         get_messages( vector< string > names ) const;

      vector< balance_state >                         get_balances( vector< string > names ) const;

      vector< key_state >                             get_keychains( vector< string > names) const;

      vector< optional< account_api_obj > >           lookup_account_names( vector< string > account_names )const;

      /** 
       * Lists all accounts registered in the blockchain.
       * 
       * This returns a list of all account names and their account ids, sorted by account name.
       *
       * Use the \c lowerbound and limit parameters to page through the list.
       * To retrieve all accounts, start by setting \c lowerbound to the empty string \c "",
       * and then each iteration, pass the last account name returned 
       * as the \c lowerbound for the next \c list_accounts() call.
       *
       * @param lowerbound the name of the first account to return. 
       * If the named account does not exist, the list will start at the first account that comes after \c lowerbound .
       * @param limit the maximum number of accounts to return (max: 1000).
       * @returns a list of accounts mapping account names to account ids.
       */
      set< string >                                   lookup_accounts( string lower_bound_name, uint32_t limit )const;

      uint64_t                                        get_account_count()const;

      /**
       * Gets the details of an accounts history of owner
       * keys, including all previous keys and thier durations.
       * @param account The name of the accoutn to query.
       */
      vector< owner_authority_history_api_obj >       get_owner_history( string account )const;

      optional< account_recovery_request_api_obj >    get_recovery_request( string account ) const;

      optional< account_bandwidth_api_obj >           get_account_bandwidth( string account, witness::bandwidth_type type )const;



      //===================//
      // === Asset API === //
      //===================//


      vector< extended_asset >                        get_assets( vector< string > assets )const;

      uint64_t                                        get_asset_count()const;

      optional< escrow_api_obj >                      get_escrow( string from, string escrow_id )const;

      /**
       * Returns fund withdraw routes for an account.
       *
       * @param account Account to query routes
       * @param type Withdraw type type [incoming, outgoing, all]
       */
      vector< withdraw_route >                        get_withdraw_routes( string account, withdraw_route_type type = all )const;

      vector< savings_withdraw_api_obj >              get_savings_withdraw_from( string account )const;

      vector< savings_withdraw_api_obj >              get_savings_withdraw_to( string account )const;

      vector< asset_delegation_api_obj >              get_asset_delegations( string account, string from, uint32_t limit = 100 )const;

      vector< asset_delegation_expiration_api_obj >   get_expiring_asset_delegations( string account, time_point from, uint32_t limit = 100 )const;



      //===================//
      // === Board API === //
      //===================//


      vector< extended_board >                        get_boards( vector< string > boards )const;

      vector< extended_board >                        get_boards_by_subscribers( string from, uint32_t limit )const;

      uint64_t                                        get_board_count()const;




      //=====================//
      // === Network API === //
      //=====================//


      /**
       * Returns the list of block producers in the current round.
       */
      vector<account_name_type>                       get_active_producers()const;

      vector< optional< witness_api_obj > >           get_witnesses( vector< witness_id_type > witness_ids )const;

      /** 
       * Lists all witnesses registered in the blockchain.
       * This returns a list of all account names that own witnesses, and the associated witness id,
       * sorted by name.  This lists witnesses whether they are currently voted in or not.
       *
       * Use the \c lowerbound and limit parameters to page through the list.  To retrieve all witnesss,
       * start by setting \c lowerbound to the empty string \c "", and then each iteration, pass
       * the last witness name returned as the \c lowerbound for the next \c list_witnesss() call.
       *
       * @param lowerbound the name of the first witness to return.  If the named witness does not exist,
       *                   the list will start at the witness that comes after \c lowerbound
       * @param limit the maximum number of witnesss to return (max: 1000)
       * @returns a list of witnesss mapping witness names to witness ids
       */
      set< account_name_type >                        lookup_witness_accounts( string lower_bound_name, uint32_t limit )const;

      uint64_t                                        get_witness_count()const;

      /** 
       * Returns information about the given witness.
       * 
       * @param owner_account the name or id of the witness account owner, or the id of the witness
       * @returns the information about the witness stored in the blockchain
       */
      fc::optional< witness_api_obj >                 get_witness_by_account( string account_name )const;

      vector< witness_api_obj >                       get_witnesses_by_voting_power( string from, uint32_t limit )const;

      vector< witness_api_obj >                       get_witnesses_by_mining_power( string from, uint32_t limit )const;

      vector< network_officer_api_obj >               get_development_officers_by_voting_power( string from, uint32_t limit )const;

      vector< network_officer_api_obj >               get_marketing_officers_by_voting_power( string from, uint32_t limit )const;

      vector< network_officer_api_obj >               get_advocacy_officers_by_voting_power( string from, uint32_t limit )const;

      vector< executive_board_api_obj >               get_executive_boards_by_voting_power( string from, uint32_t limit )const;

      vector< supernode_api_obj >                     get_supernodes_by_view_weight( string from, uint32_t limit )const;

      vector< interface_api_obj >                     get_interfaces_by_users( string from, uint32_t limit )const;

      vector< governance_account_api_obj >            get_governance_accounts_by_subscriber_power( string from, uint32_t limit )const;

      vector< community_enterprise_api_obj >          get_enterprise_by_voting_power( string from, string from_id, uint32_t limit )const;




      //====================//
      // === Market API === //
      //====================//



      /**
       * Gets the currently open orders on the market for
       * a specified account.
       */
      vector< order_state >                           get_open_orders( vector< string > names )const;

      market_limit_orders                             get_limit_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;

      market_margin_orders                            get_margin_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;

      market_call_orders                              get_call_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;

      market_credit_loans                             get_credit_loans( string buy_symbol, string sell_symbol, uint32_t limit ) const;

      vector< credit_pool_api_obj >                   get_credit_pools( vector< string > assets ) const;

      vector< liquidity_pool_api_obj >                get_liquidity_pools( string buy_symbol, string sell_symbol ) const;

      market_state                                    get_market_state( string buy_symbol, string sell_symbol )const;




      //================//
      // === Ad API === //
      //================//

      vector< account_ad_state >                      get_account_ads( vector< string > names )const;

      vector< ad_bid_state >                          get_interface_audience_bids( const ad_query& query )const;




      //====================//
      // === Search API === //
      //====================//

      search_result_state                             get_search_query( const search_query& query )const;  




      //=====================================//
      // === Blocks and Transactions API === //
      //=====================================//


      /** 
       * Signs a transaction.
       *
       * Given a fully-formed transaction that is only lacking signatures, this signs
       * the transaction with the necessary keys and optionally broadcasts the transaction.
       * 
       * @param tx the unsigned transaction
       * @param broadcast Set True to broadcast transaction.
       * @return the signed version of the transaction
       */
      annotated_signed_transaction                    sign_transaction( signed_transaction tx, bool broadcast = false );

      /** 
       * Returns an uninitialized object representing a given blockchain operation.
       *
       * Returns a default-initialized object of the given operation type
       * for custom transaction construction.
       *
       * Any operation the blockchain supports can be created using the transaction builder's
       * \c add_operation_to_builder_transaction() , describes the
       * JSON form of the operation for reference. 
       * This will give you a template you can fill in.
       *
       * @param operation_type Type of operation to return, must be defined in `node/chain/operations.hpp`
       * @return a default-constructed operation of the given type
       */
      operation                                       get_prototype_operation( string operation_type );

      void                                            network_add_nodes( const vector<string>& nodes );

      vector< variant >                               network_get_connected_peers();

      /** 
       * Returns the information about a block.
       *
       * @param num Block number requested.
       *
       * @returns Public block data on the blockchain.
       */
      optional< signed_block_api_obj >                get_block( uint32_t num );

      /** 
       * Returns sequence of operations included/generated in a specified block.
       *
       * @param block_num Block height of specified block.
       * @param only_virtual Whether to only return virtual operations.
       * 
       * @returns Operations included in a specified block.
       */
      vector< applied_operation >                     get_ops_in_block( uint32_t block_num, bool only_virtual = true );

      /**
       * Returns transaction by ID.
       * 
       * @param trx_id Transaction ID of specified transaction.
       * 
       * @returns Annotated signed transaction with trx_id
       */
      annotated_signed_transaction                    get_transaction( transaction_id_type trx_id )const;

      /**
       * Convert a JSON transaction to its transaction ID.
       */
      transaction_id_type                             get_transaction_id( const signed_transaction& trx )const { return trx.id(); }





      //========================//
      // === Post + Tag API === //
      //========================//


      vector< vote_state >                 get_active_votes( string author, string permlink )const;

      vector< view_state >                 get_active_views( string author, string permlink )const;

      vector< share_state >                get_active_shares( string author, string permlink )const;

      vector< moderation_state >           get_active_mod_tags( string author, string permlink )const;

      vector< account_vote >               get_account_votes( string account )const;

      vector< account_view >               get_account_views( string account )const;

      vector< account_share >              get_account_shares( string account )const;

      vector< account_moderation >         get_account_moderation( string account )const;

      vector< tag_following_api_obj >      get_tag_followings( vector< string > tags )const;

      vector< tag_api_obj >                get_top_tags( string after_tag, uint32_t limit )const;

      vector< pair< tag_name_type, uint32_t > >   get_tags_used_by_author( string author )const;



      //========================//
      // === Discussion API === //
      //========================//


      discussion                           get_content( string author, string permlink )const;
      
      vector< discussion >                 get_content_replies( string parent, string parent_permlink )const;

      vector< discussion >                 get_replies_by_last_update( account_name_type start_author, string start_permlink, uint32_t limit )const;

      vector< discussion >                 get_discussions_by_payout(const discussion_query& query )const;

      vector< discussion >                 get_post_discussions_by_payout( const discussion_query& query )const;

      vector< discussion >                 get_comment_discussions_by_payout( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_index( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_created( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_active( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_votes( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_views( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_shares( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_children( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_vote_power( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_view_power( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_share_power( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_comment_power( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_feed( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_blog( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_recommended( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_comments( const discussion_query& query )const;



      //===================//
      // === State API === //
      //===================//


      /**
       * Returns the state information associated with the URL specified.
       * 
       * @param url The string of the current URL
       * 
       * @returns state object containing the necessary details for the application page.
       */
      app::state                              get_state( string url );



      //==============================//
      // === Account Transactions === //
      //==============================//



      /**
       * Genrates new keys for the new account which will be controlable by this wallet. 
       *
       *  @param registrar The account creating the new account.
       *  @param new_account_name The name of the new account.
       *  @param json JSON Metadata associated with the new account.
       *  @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           create_account( 
         string registrar, 
         string new_account_name, 
         string json, 
         bool broadcast );

      /**
       * Create new accounts for other users which must provide their desired keys. 
       * The resulting account may not be controllable by this wallet.
       *
       * @param registrar The account creating the new account.
       * @param newname The name of the new account.
       * @param json JSON Metadata associated with the new account.
       * @param owner public owner key of the new account.
       * @param active public active key of the new account.
       * @param posting public posting key of the new account.
       * @param memo public memo key of the new account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           create_account_with_keys( 
         string registrar,
         string newname,
         string json,
         public_key_type owner,
         public_key_type active,
         public_key_type posting,
         public_key_type memo,
         bool broadcast )const;

      /**
       * Generates new keys for the new account which will be controlable by this wallet. 
       * 
       * Accounts are created with Coin Fee and delegation.
       *
       * @param registrar The account creating the new account.
       * @param fee The amount of the fee to be paid.
       * @param delegation The amount of the fee to be paid with delegation.
       * @param new_account_name The name of the new account.
       * @param json JSON Metadata associated with the new account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           create_account_delegated( 
         string registrar, 
         asset fee, 
         asset delegation, 
         string new_account_name, 
         string json, 
         bool broadcast );

      /**
       * Create new accounts for other users which must provide their desired keys. 
       * 
       * The resulting account may not be controllable by this wallet. 
       * There is a fee associated with account creation that is paid by the registrar.
       *
       * These accounts are created with combination of Coin fee and delegated Stake
       *
       * @param registrar The account creating the new account.
       * @param fee The amount of the fee to be paid.
       * @param delegation The amount of the fee to be paid with delegation.
       * @param newname The name of the new account.
       * @param json JSON Metadata associated with the new account.
       * @param owner public owner key of the new account.
       * @param active public active key of the new account.
       * @param posting public posting key of the new account.
       * @param memo public memo key of the new account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           create_account_with_keys_delegated( 
         string registrar,
         asset fee,
         asset delegation,
         string newname,
         string json,
         public_key_type owner,
         public_key_type active,
         public_key_type posting,
         public_key_type memo,
         bool broadcast )const;

      /**
       * Update the keys of an existing account.
       *
       * @param accountname The name of the account.
       * @param json New JSON Metadata to be associated with the account.
       * @param owner New public owner key for the account.
       * @param active New public active key for the account.
       * @param posting New public posting key for the account.
       * @param memo New public memo key for the account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           update_account( 
         string accountname,
         string json,
         public_key_type owner,
         public_key_type active,
         public_key_type posting,
         public_key_type memo,
         bool broadcast )const;

      /**
       * Update the key of an authority for an exisiting account.
       * 
       * @warning You can create impossible authorities using this method.
       * 
       * The method will fail if you create an impossible owner authority, 
       * but will allow impossible active and posting authorities.
       *
       * @param account_name The name of the account whose authority you wish to update.
       * @param type The authority type. e.g. owner, active, or posting.
       * @param key The public key to add to the authority.
       * @param weight The weight the key should have in the authority. 
       *    A weight of 0 indicates the removal of the key.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           update_account_auth_key( 
         string account_name, 
         authority_type type, 
         public_key_type key, 
         weight_type weight, 
         bool broadcast );

      /**
       * Updates the account of an authority for an exisiting account.
       * 
       * @warning You can create impossible authorities using this method.
       * 
       * The method will fail if you create an impossible owner authority, 
       * but will allow impossible active and posting authorities.
       *
       * @param account_name The name of the account whose authority you wish to update.
       * @param type The authority type. e.g. owner, active, or posting.
       * @param auth_account The account to add the the authority.
       * @param weight The weight the account should have in the authority. 
       *    A weight of 0 indicates the removal of the account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           update_account_auth_account( 
         string account_name, 
         authority_type type, 
         string auth_account, 
         weight_type weight, 
         bool broadcast );

      /**
       * Updates the weight threshold of an authority for an account.
       * 
       * @warning You can create impossible authorities using this method.
       * 
       * The method will fail if you create an impossible owner authority, 
       * but will allow impossible active and posting authorities.
       *
       * @param account_name The name of the account whose authority you wish to update.
       * @param type The authority type. e.g. owner, active, or posting.
       * @param threshold The weight threshold required for the authority to be met.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           update_account_auth_threshold( 
         string account_name, 
         authority_type type, 
         uint32_t threshold, 
         bool broadcast );

      /**
       * Updates the account JSON metadata.
       *
       * @param account_name The name of the account you wish to update.
       * @param json The new JSON metadata for the account. Overrides existing metadata.
       * @param broadcast ture if you wish to broadcast the transaction.
       */
      annotated_signed_transaction           update_account_meta( 
         string account_name, 
         string json, 
         bool broadcast );

      /**
       * Updates the Secure Public key of an account.
       * 
       * Used for Sending and reciving encrypted memos and messages.
       *
       * @param account_name The name of the account you wish to update.
       * @param key The new secure public key.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           update_account_secure_public_key(
         string account_name, 
         public_key_type key, 
         bool broadcast );


      /**
       * Activates membership on an account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account The name of the account to activate membership on.
       * @param membership_type The level of membership to activate on the account.
       * @param months Number of months to purchase membership for.
       * @param interface Name of the interface application facilitating the transaction.
       * @param recurring True for membership to automatically recur each month from liquid balance. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_membership(
         string signatory,
         string account,
         string membership_type,
         uint16_t months,
         string interface, 
         bool recurring, 
         bool broadcast );


      /**
       * Votes for an account to become an executive of a business account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account The name of the Account creating the executive vote.
       * @param business_account Business account that the executive is being voted for.
       * @param executive_account The Name of executive being voted for.
       * @param role The Role of the executive.
       * @param vote_rank Rank of voting preference.
       * @param approved True to add, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_vote_executive(
         string signatory,
         string account,
         string business_account,
         string executive_account,
         string role,
         uint16_t vote_rank,
         bool approved,
         bool broadcast );


      /**
       * Votes for an account to become an officer of a business account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account The name of the Account creating the officer vote.
       * @param business_account Business account that the officer is being voted for.
       * @param officer_account The Name of officer being voted for.
       * @param vote_rank Rank of voting preference.
       * @param approved True to add, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_vote_officer(
         string signatory,
         string account,
         string business_account,
         string officer_account,
         string role,
         uint16_t vote_rank,
         bool approved,
         bool broadcast );


      /**
       * Requests that an account be added to the membership of a business account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account requesting to be a member of the business.
       * @param business_account Business account that the member is being added to.
       * @param message Encrypted Message to the business members requesting membership.
       * @param requested True to add, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_member_request(
         string signatory,
         string account,
         string business_account,
         string message,
         bool requested,
         bool broadcast );


      /**
       * Invites an account to be a member of a business account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account requesting to be a member of the business.
       * @param business_account Business account that the member is being added to.
       * @param member Name of member being added.
       * @param message Encrypted Message to the business members requesting membership.
       * @param encrypted_business_key Encrypted Copy of the private key of the business.
       * @param invited True to add, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_member_invite(
         string signatory,
         string account,
         string business_account,
         string member,
         string message,
         string encrypted_business_key,
         bool invited,
         bool broadcast );


      /**
       * Accepts an account's request to be added as a business account member.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account that is accepting the request to add a new member.
       * @param business_account Business account that the member is being added to.
       * @param member Name of member being added.
       * @param encrypted_business_key Encrypted Copy of the private key of the business.
       * @param accepted True to accept, false to reject.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_accept_request(
         string signatory,
         string account,
         string business_account,
         string member,
         string encrypted_business_key,
         bool accepted,
         bool broadcast );


      /**
       * Accepts an invitation to be added as a business account member.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account accepting the invitation.
       * @param business_account Business account that the account was invited to.
       * @param accepted True to accept, false to reject.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_accept_invite(
         string signatory,
         string account,
         string business_account,
         bool accepted,
         bool broadcast );


      /**
       * Removes a member from the membership of a business account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Business account or an executive of the business account.
       * @param business_account Business account that the member is being removed from.
       * @param member Name of member being accepted.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_remove_member(
         string signatory,
         string account,
         string business_account,
         string member,
         bool broadcast );


      /**
       * Blacklists an account or asset.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Name of account.
       * @param listed_account Name of account being added to a black or white list.
       * @param listed_asset Name of asset being added to a black or white list.
       * @param blacklisted True to add to blacklist, false to remove.
       * @param whitelisted True to add to whitelist, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_update_list(
         string signatory,
         string account,
         string listed_account,
         string listed_asset,
         bool blacklisted,
         bool whitelisted,
         bool broadcast );

      /**
       * Vote for a witness to become a block producer. 
       *
       * @param signatory The name of the account signing the transaction.
       * @param account The account voting for a witness.
       * @param vote_rank Rank ordering of the vote.
       * @param witness The witness that is being voted for.
       * @param approved  True to create vote, false to remove vote.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           vote_witness(
         string signatory,
         string account,
         uint16_t vote_rank,
         string witness,
         bool approve = true,
         bool broadcast = false );


      /** 
       * Set the voting proxy for an account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account The name of the account to update.
       * @param proxy The name of account that should proxy to, or empty string to have no proxy.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_update_proxy(
         string signatory,
         string account,
         string proxy,
         bool broadcast = false );

      


      /**
       * Create an account recovery request as a recover account. 
       *
       * Authority object Structure:
       * {"weight_threshold": 1,"account_auths": [], "key_auths": [["new_public_key",1]]}
       *
       * @param signatory The name of the account signing the transaction.
       * @param recovery_account The recovery account is listed as the recovery account on the account to recover.
       * @param account_to_recover The account to recover. This is likely due to a compromised owner authority.
       * @param new_owner_authority The new owner authority the account to recover wishes to have.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           request_account_recovery( 
         string recovery_account, 
         string account_to_recover, 
         authority new_owner_authority, 
         bool broadcast );

      /**
       * Recover an account using a recovery request created by your recovery account. 
       * 
       * Authority object Structure:
       * {"weight_threshold": 1,"account_auths": [], "key_auths": [["old_public_key",1]]} {"weight_threshold": 1,"account_auths": [], "key_auths": [["new_public_key",1]]} true
       *
       * @param signatory The name of the account signing the transaction.
       * @param account_to_recover The account to be recovered.
       * @param new_owner_authority The new authority that your recovery account used in the account recover request.
       * @param recent_owner_authority A recent owner authority on your account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           recover_account(
         string signatory,
         string account_to_recover,
         authority new_owner_uthority,
         authority recent_owner_authority,
         bool broadcast );

      
      /**
       * Allows a nominated reset account to change an account's owner authority.
       *
       * @param signatory The name of the account signing the transaction.
       * @param reset_account The account that is initiating the reset process.
       * @param account_to_reset The Account to be reset.
       * @param new_owner_authority A recent owner authority on your account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           reset_account(
         string signatory,
         string account_to_recover,
         authority new_owner_uthority,
         authority recent_owner_authority,
         bool broadcast );


      /**
       * Updates an accounts specified reset account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account to update.
       * @param new_reset_account Account that has the new authority to reset the account.
       * @param days Days of inactivity required to reset the account. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           set_reset_account(
         string signatory,
         string account,
         string new_reset_account,
         uint16_t days,
         bool broadcast );

      /**
       * Change your recovery account after a 30 day delay.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account_to_recover The account being updated.
       * @param new_recovery_account The account that is authorized to create recovery requests.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           change_recovery_account(
         string signatory,
         string account_to_recover, 
         string new_recovery_account, 
         bool broadcast );


      /**
       * Removes an account's ability to vote in perpetuity.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account The account being updated.
       * @param declined True to decine voting rights, false to cancel pending request.
       * @param broadcast Set True to broadcast transaction.
       */ 
      annotated_signed_transaction           decline_voting_rights(
         string signatory,
         string account, 
         bool declined, 
         bool broadcast );



      /**
       * Requests that a connection be created between two accounts.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account sending the request.
       * @param requested_account Account that is being requested to connect.
       * @param connection_type Type of connection level.
       * @param message Message attached to the request, encrypted with recipient's secure public key.
       * @param requested Set true to request, false to cancel request.
       * @param broadcast Set True to broadcast transaction.
       */ 
      annotated_signed_transaction           connection_request(
         string signatory,
         string account,
         string requested_account,
         string connection_type,
         string message,
         bool requested, 
         bool broadcast );


      /**
       * Accepts an incoming connection request by providing an encrypted posting key.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account accepting the request.
       * @param requesting_account Account that originally requested to connect.
       * @param connection_id uuidv4 for the connection, for local storage of decryption key.
       * @param connection_type Type of connection level.
       * @param encrypted_key The private connection key of the user, encrypted with the public secure key of the requesting account.
       * @param connected Set true to connect, false to delete connection.
       * @param broadcast Set True to broadcast transaction.
       */ 
      annotated_signed_transaction           connection_accept(
         string signatory,
         string account,
         string requesting_account,
         string connection_id,
         string connection_type,
         string encrypted_key,
         bool connected, 
         bool broadcast );


      /**
       * Enables an account to follow another account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param follower Account that is creating the new follow relationship.
       * @param following Account that is being followed by follower.
       * @param interface Account of the interface facilitating the transaction broadcast.
       * @param added Set true to add to list, false to remove from list.
       * @param followed Set true to follow, false to filter.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_follow(
         string signatory,
         string follower,
         string following,
         string interface,
         bool added,
         bool followed,
         bool broadcast );

      
      /**
       * Enables an account to follow a tag.
       *
       * @param signatory The name of the account signing the transaction.
       * @param follower Name of the account following the tag.
       * @param tag Tag being followed.
       * @param interface Account of the interface facilitating the transaction broadcast.
       * @param added Set true to add to list, false to remove from list.
       * @param followed Set true to follow, false to filter.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           tag_follow(
         string signatory,
         string follower,
         string tag,
         string interface,
         bool added,
         bool followed,
         bool broadcast );


      /**
       * Claims an account's daily activity reward.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Name of the account claiming the reward.
       * @param permlink Permlink of the users recent post in the last 24h.
       * @param view_id Recent comment id viewed in the last 24h.
       * @param vote_id Recent comment id voted on in the last 24h.
       * @param interface Account of the interface facilitating the transaction broadcast.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           activity_reward(
         string signatory,
         string account,
         string permlink,
         uint64_t view_id,
         uint64_t vote_id,
         string interface,
         bool broadcast );


      //==============================//
      // === Network Transactions === //
      //==============================//



      //=======================================//
      // === Post and Comment Transactions === //
      //=======================================//


      /**
       * Post or update a comment.
       *
       * @param author the name of the account authoring the comment.
       * @param permlink the accountwide unique permlink for the comment.
       * @param parent_author can be null if this is a top level comment.
       * @param parent_permlink becomes category if parent_author is "".
       * @param title the title of the comment.
       * @param body the body of the comment.
       * @param json the json metadata of the comment.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           post_comment(
         string author, 
         string permlink, 
         string parent_author, 
         string parent_permlink, 
         string title, 
         string body, 
         string json, 
         bool broadcast );

      /**
       * Vote on a comment.
       *
       * @param voter The account voting.
       * @param author The author of the comment to be voted on.
       * @param permlink The permlink of the comment to be voted on. (author, permlink) is a unique pair.
       * @param weight The weight [-100,100] of the vote.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           vote( 
         string voter, 
         string author, 
         string permlink, 
         int16_t weight, 
         bool broadcast );




      //============================//
      // === Board Transactions === //
      //============================//



      //=========================//
      // === Ad Transactions === //
      //=========================//



      //===============================//
      // === Transfer Transactions === //
      //===============================//


      /**
       * Transfer funds from one account to another.
       *
       * @param from The account the funds are coming from
       * @param to The account the funds are going to
       * @param amount The funds being transferred
       * @param memo A memo for the transactionm, encrypted with the to account's public memo key
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer(
         string from, 
         string to,
          asset amount, 
          string memo, 
          bool broadcast = false );



      //==============================//
      // === Balance Transactions === //
      //==============================//


      annotated_signed_transaction            claim_reward_balance( 
         string account, 
         asset reward, 
         bool broadcast );

      /**
       * Stake Asset Transaction
       * 
       * @param from The account the assets is coming from
       * @param to The account getting the staked assets
       * @param amount The amount of assets to stake.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            stake_asset( 
         string from, 
         string to, 
         asset amount, 
         bool broadcast = false );

      /**
       * Set up an unstake assets request.
       * 
       * The request is fulfilled once a week over the duration of the operation.
       *
       * @param from The account the assets are withdrawn from
       * @param assets The amount of assets to withdraw 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            unstake_asset(
         string from, 
         asset amount,
         bool broadcast = false );

      /**
       * Set up an asset withdraw route.
       * 
       * When assets are withdrawn, they will be routed to these accounts
       * based on the specified weights.
       *
       * @param from The account the assets are withdrawn from.
       * @param to   The account receiving either assets or new stake.
       * @param percent The percent of the withdraw to go to the 'to' account. This is denoted in hundreths of a percent.
       *    i.e. 100 is 1% and 10000 is 100%. This value must be between 1 and 100000
       * @param auto_stake Set to true if the from account should receive the withdrawn assets as stake, or false if it should receive
       *    them as a liquid balance.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            unstake_asset_route(
         string from, 
         string to, 
         uint16_t percent, 
         bool auto_stake, 
         bool broadcast = false );

      /**
       * Transfers into savings happen immediately, transfers from savings take 72 hours
       */
      annotated_signed_transaction            transfer_to_savings( 
         string from, 
         string to, 
         asset amount, 
         string memo, 
         bool broadcast = false );

      /**
       * @param request_id - an unique ID assigned by from account, the id is used to cancel the operation and can be reused after the transfer completes
       */
      annotated_signed_transaction            transfer_from_savings( 
         string from, 
         uint32_t request_id, 
         string to, 
         asset amount, 
         string memo, 
         bool broadcast = false );


      /**
       * Delegate assets from one account to another.
       *
       * @param delegator The name of the account delegating assets.
       * @param delegatee The name of the account receiving assets.
       * @param assets The amount of assets to delegate.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            delegate_asset( 
         string delegator, 
         string delegatee, 
         asset assets, 
         bool broadcast );


      //=============================//
      // === Escrow Transactions === //
      //=============================//


      /**
       * Transfer funds from one account to another using escrow.
       *
       * @param from The account the funds are coming from
       * @param to The account the funds are going to
       * @param agent The account acting as the agent in case of dispute
       * @param escrow_id A unique id for the escrow transfer. (from, escrow_id) must be a unique pair
       * @param amount The amount of assets to transfer
       * @param fee The fee paid to the agent
       * @param ratification_deadline The deadline for 'to' and 'agent' to approve the escrow transfer
       * @param escrow_expiration The expiration of the escrow transfer, after which either party can claim the funds
       * @param json JSON encoded meta data
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            escrow_transfer(
         string from,
         string to,
         string agent,
         string escrow_id,
         asset amount,
         asset fee,
         time_point ratification_deadline,
         time_point escrow_expiration,
         string json,
         bool broadcast = false );

      /**
       * Approve a proposed escrow transfer.
       * 
       * Funds cannot be released until after approval.
       *
       * @param from The account that funded the escrow.
       * @param to The destination of the escrow.
       * @param agent The account acting as the agent in case of dispute.
       * @param who The account approving the escrow transfer (either 'to' or 'agent)
       * @param escrow_id A unique id for the escrow transfer
       * @param approve true to approve the escrow transfer, otherwise cancels it and refunds 'from'
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            escrow_approve(
         string from,
         string to,
         string agent,
         string who,
         string escrow_id,
         bool approve,
         bool broadcast = false
      );

      /**
       * Raise a dispute on the escrow transfer before it expires
       *
       * @param from The account that funded the escrow
       * @param to The destination of the escrow
       * @param agent The account acting as the agent in case of dispute
       * @param who The account raising the dispute (either 'from' or 'to')
       * @param escrow_id A unique id for the escrow transfer
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            escrow_dispute(
         string from,
         string to,
         string agent,
         string who,
         string escrow_id,
         bool broadcast = false );

      /**
       * Release funds held in escrow.
       *
       * @param from The account that funded the escrow
       * @param to The account the funds are originally going to
       * @param agent The account acting as the agent in case of dispute
       * @param who The account authorizing the release
       * @param receiver The account that will receive funds being released
       * @param escrow_id A unique id for the escrow transfer
       * @param amount The amount of assets that will be released
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            escrow_release(
         string from,
         string to,
         string agent,
         string who,
         string receiver,
         string escrow_id,
         asset amount,
         bool broadcast = false );



      //==============================//
      // === Trading Transactions === //
      //==============================//


      /**
       * Creates a limit order at the price amount_to_sell / min_to_receive 
       * and will deduct amount_to_sell from account.
       *
       * @param owner The name of the account creating the order.
       * @param order_id is a unique identifier assigned by the registrar of the order, it can be reused after the order has been filled.
       * @param amount_to_sell The amount you wish to sell.
       * @param min_to_receive The amount of the other asset you will receive at a minimum.
       * @param fill_or_kill true if you want the order to be killed if it cannot immediately be filled.
       * @param expiration the time the order should expire if it has not been filled.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             create_order(
         string owner, 
         uint32_t order_id, 
         asset amount_to_sell, 
         asset min_to_receive, 
         bool fill_or_kill, 
         uint32_t expiration, 
         bool broadcast );



      //===========================//
      // === Pool Transactions === //
      //===========================//



      //============================//
      // === Asset Transactions === //
      //============================//



      //=====================================//
      // === Block Producer Transactions === //
      //=====================================//

      
      /**
       * Update a witness object owned by the given account.
       *
       * @param witness_name The name of the witness account.
       * @param url A URL containing some information about the witness.  The empty string makes it remain the same.
       * @param block_signing_key The new block signing public key.  The empty string disables block production.
       * @param props The chain properties the witness is voting on.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction              update_witness(
         string witness_name,
         string url,
         public_key_type block_signing_key,
         const chain_properties& props,
         bool broadcast = false );


      

      

      
      /**
       * Sets the amount of time in the future until a transaction expires.
       */
      void set_transaction_expiration( uint32_t seconds );


      


      


      std::map<string,std::function<string(fc::variant,const fc::variants&)>> get_result_formatters() const;

      fc::signal<void(bool)> lock_changed;
      std::shared_ptr<detail::wallet_api_impl> my;

      void encrypt_keys();

      /**
       * Checks memos against private keys on account and imported in wallet
       */
      void check_memo( const string& memo, const account_api_obj& account )const;

      /**
       *  Returns the encrypted memo if memo starts with '#' otherwise returns memo
       */
      string get_encrypted_memo( string from, string to, string memo );

      /**
       * Returns the decrypted memo if possible given wallet's known private keys
       */
      string decrypt_memo( string memo );

};

struct plain_keys {
   fc::sha512                  checksum;
   map<public_key_type,string> keys;
};

} }

FC_REFLECT( node::wallet::wallet_data,
            (cipher_keys)
            (ws_server)
            (ws_user)
            (ws_password)
          )

FC_REFLECT( node::wallet::brain_key_info, (brain_priv_key)(wif_priv_key) (pub_key))

FC_REFLECT( node::wallet::plain_keys, (checksum)(keys) )

FC_REFLECT_ENUM( node::wallet::authority_type, (owner)(active)(posting) )

FC_API( node::wallet::wallet_api,

        // wallet api

        (help)
        (gethelp)
        (about)
        (is_new)
        (is_locked)
        (lock)
        (unlock)
        (set_password)
        (load_wallet_file)
        (save_wallet_file)

        // key api

        (import_key)
        (suggest_brain_key)
        (list_keys)
        (get_private_key)
        (get_private_key_from_password)
        (normalize_brain_key)

        // query api

        (info)
        (list_my_accounts)
        (list_accounts)
        (list_witnesses)
        (get_witness)
        (get_account)
        (get_block)
        (get_ops_in_block)
        (get_feed_history)
        (get_conversion_requests)
        (get_account_history)
        (get_state)
        (get_withdraw_routes)

        // transaction api

        (create_account)
        (create_account_with_keys)
        (create_account_delegated)
        (create_account_with_keys_delegated)
        (update_account)
        (update_account_auth_key)
        (update_account_auth_account)
        (update_account_auth_threshold)
        (update_account_meta)
        (update_account_secure_public_key)
        (delegate_asset)
        (update_witness)
        (set_voting_proxy)
        (vote_for_witness)
        (follow)
        (transfer)
        (escrow_transfer)
        (escrow_approve)
        (escrow_dispute)
        (escrow_release)
        (stake_asset)
        (unstake_asset)
        (unstake_asset_route)
        (publishFeed)
        (get_order_book)
        (get_open_orders)
        (create_order)
        (cancel_order)
        (post_comment)
        (vote)
        (set_transaction_expiration)
        (challenge)
        (prove)
        (request_account_recovery)
        (recover_account)
        (change_recovery_account)
        (get_owner_history)
        (transfer_to_savings)
        (transfer_from_savings)
        (cancel_transfer_from_savings)
        (get_encrypted_memo)
        (decrypt_memo)
        (decline_voting_rights)
        (claim_reward_balance)

        // private message api

        (send_private_message)
        (get_inbox)
        (get_outbox)

        // helper api

        (get_prototype_operation)
        (serialize_transaction)
        (sign_transaction)

        (network_add_nodes)
        (network_get_connected_peers)

        (get_active_witnesses)
        (get_miner_queue)
        (get_transaction)
      )

FC_REFLECT( node::wallet::memo_data, (from)(to)(nonce)(check)(encrypted) )
