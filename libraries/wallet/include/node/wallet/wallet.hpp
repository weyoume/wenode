#pragma once

#include <node/app/api.hpp>
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

typedef uint16_t transaction_handle_type;


/**
 * Contains a pair of public keys, checksum and encrypted data from a message.
 * Adds a # onto strings
 */
struct encrypted_message_data 
{
   static optional< encrypted_message_data > from_string( string str ) 
   {
      try 
      {
         if( str.size() > sizeof( encrypted_message_data ) && str[0] == '#') 
         {
            auto data = fc::from_base58( str.substr(1) );
            auto m  = fc::raw::unpack< encrypted_message_data >( data );
            FC_ASSERT( string( m ) == str );
            return m;
         }
      } 
      catch ( ... ) {}

      return optional< encrypted_message_data >();
   }

   public_key_type          from;          ///< Public key of sending account.

   public_key_type          to;            ///< Public key of the receiving account.

   uint64_t                 nonce = 0;     ///< Iterated value derived from the time of encryption.

   uint32_t                 check = 0;     ///< Hash checksum of the plaintext.

   vector< char >           encrypted;     ///< Raw encrypted data of the message.

   operator string()const                  ///< Returns the base58 Hash-prefixed compressed object.
   {
      auto data = fc::raw::pack( *this );
      auto base58 = fc::to_base58( data );
      return '#'+base58;
   }
};

struct seed_phrase_info
{
   string               brain_priv_key;       ///< 12 Word Seed phrase generated.

   public_key_type      pub_key;              ///< Public key of the keypair derived by the seed phrase.
   
   string               wif_priv_key;         ///< Private key of the keypair derived by the seed phrase.
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
 * Wallet API Provides an extensive Command Line Interface with the
 * Blockchain, and facilitates all operations broadcasting and database API.
 * 
 * Wallet assumes it is connected to the database server
 * with a high-bandwidth, low-latency connection and
 * performs minimal caching.
 * 
 * Wallet API can be provided locally to be used by a web interface.
 */
class wallet_api
{
   public:
      wallet_api( const wallet_data& initial_data, fc::api<login_api> rapi );
      virtual ~wallet_api();

      /**
       * @defgroup wallet Wallet Commands
       * @{
       */


      //====================//
      // === Wallet API === //
      //====================//


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
       * Lists the names all accounts for which this wallet has a private key.
       */
      vector< string >                       list_my_accounts();

      /**
       * Gets the full extended account information for all accounts for which this wallet has a private key.
       */
      vector< extended_account >             get_my_accounts();

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

      /**
       * Copies the wallet file to a specified file pathway.
       * 
       * @param destination_filename The Filepath to copy the active wallet to.
       * @returns True if successful.
       */
      bool                                   copy_wallet_file( string destination_filename );

      /**
       * Sets the amount of time in the future until a transaction expires.
       * 
       * @param seconds the number of seconds to set each new transaction to expire within.
       */
      void                                   set_transaction_expiration( uint32_t seconds );

      /**
       * Checks memos against private keys on account and imported in wallet.
       */
      void                                   check_memo( const string& memo, const account_api_obj& account )const;

      /**
       *  Returns the encrypted message if message starts with '#' otherwise returns message.
       */
      string                                 get_encrypted_message( string from_public_key, string to_public_key, string message )const;

      /**
       * Returns the decrypted plaintext message if possible given wallet's known private keys.
       */
      string                                 get_decrypted_message( string message )const;

      /**
       * Returns the private key derived from the prefix and sequence number. 
       */
      fc::ecc::private_key                   derive_private_key( const std::string& prefix_string, int sequence_number ) const;


      std::map<string,std::function<string(fc::variant,const fc::variants&)>> get_result_formatters() const;

      fc::signal<void(bool)>                        lock_changed;
      
      std::shared_ptr<detail::wallet_api_impl>       my;

     

      //=================//
      // === Key API === //
      //=================//


      /**
       * Get the WIF private key corresponding to a public key. 
       * The private key must already be in the wallet.
       * 
       * @param pubkey The public key to retrieve the wallet's private key for. 
       * @returns The private key corresponding to the public key.
       */
      string                                 get_private_key( public_key_type pubkey )const;

      /**
       * Gets the WIF private key that is generated from a specified account using a provided password for 
       * an authority type.
       * @param account The name of the account.
       * @param role active | owner | posting | memo
       * @param password the string password to be used for key generation.
       * @returns The WIF Private key of the password. 
       */
      pair< public_key_type, string >        get_private_key_from_password( string account, string role, string password )const;

      /** 
       * Suggests a safe seed phrase to use for creating your account.
       * 
       * create_account_with_seed_phrase() requires you to specify a 'seed phrase',
       * a long passphrase that provides enough entropy to generate cryptographic
       * keys. This function will suggest a suitably random string that should
       * be easy to write down (and, with effort, memorize).
       * 
       * @returns A suggested seed_phrase.
       */
      seed_phrase_info                       suggest_seed_phrase()const;

      /** 
       * Transforms a seed phrase to reduce the chance of errors when re-entering the key from memory.
       *
       * This takes a user-supplied seed phrase and
       * normalizes it into the form used
       * for generating private keys. 
       * In particular, this upper-cases all ASCII characters
       * and collapses multiple spaces into one.
       * 
       * @param s the seed phrase as supplied by the user
       * @returns the seed phrase in its normalized form
       */
      string                                 normalize_seed_phrase( string s ) const;

      /**
       * Dumps all private keys owned by the wallet.
       *
       * The keys are printed in WIF format. Import these keys into another wallet
       * using import_key()
       * 
       * @returns map containing the wallet's private keys, indexed by their public key.
       */
      map< public_key_type, string >         list_keys();

      /** 
       * Imports a WIF Private Key into the wallet to be used to sign transactions by an account.
       *
       * example: import_key 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
       *
       * @param wif_key the WIF Private Key to import
       */
      bool                                   import_key( string wif_key );


      void                                   encrypt_keys();

      

      //====================//
      // === Global API === //
      //====================//


      
      /** 
       * Returns the Configuration variables of the protocol.
       * 
       * @returns All values of config.hpp preprocessor directive variables. 
       */
      fc::variant_object                              get_config()const;


      /** 
       * Returns the blockchain's rapidly-changing properties.
       * 
       * Contains information that changes every block interval
       * such as the head block number, and the next producer.
       * 
       * @returns The current dynamic global properties.
       */
      dynamic_global_property_api_obj                 get_dynamic_global_properties() const;


      /** 
       * Returns the current median values of the Chain properties.
       * 
       * @returns All values of network selected chain properties.
       */
      median_chain_property_api_obj                   get_median_chain_properties() const;


      /** 
       * Returns the Producer schedule values.
       * 
       * @returns Producer schedule object values, determining block production conditions.
       */
      producer_schedule_api_obj                       get_producer_schedule()const;


      /** 
       * Returns the current harkfork version of the network.
       * 
       * @returns Version of the most recent hardfork activated on the network
       */
      hardfork_version                                get_hardfork_version()const;


      /** 
       * Returns the next planned harkfork version of the network.
       * 
       * @returns Version of the next planned hardfork to be activated on the network.
       */
      scheduled_hardfork                              get_next_scheduled_hardfork()const;


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


      /** 
       * Returns an ordered list of the accounts with the most followers.
       *
       * @param from The name of the first account to include.
       * @param limit Number of accounts to include.
       * @returns Account objects of the most followed accounts.
       */
      vector< account_api_obj >                       get_accounts_by_followers( string from, uint32_t limit ) const;


      /** 
       * Returns concise information about a list of given accounts.
       *
       * @param names The name of the accounts to provide information about.
       * @returns Account Object information pertaining to the specified accounts.
       */
      vector< account_concise_api_obj >               get_concise_accounts( vector< string > names ) const;


      /** 
       * Returns full representation of the given list of accounts, and all objects that it controls.
       *
       * @param names The name of the accounts to provide information about.
       * @returns Account Object information pertaining to the specified accounts.
       */
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


      /** 
       * Returns the message states of a list of accounts.
       * 
       * Includes inbox and outbox, and conversational threads.
       *
       * @param names The names of the accounts to provide message information.
       * @returns Message state information pertaining to the specified accounts.
       */
      vector< account_message_state >                 get_account_messages( vector< string > names ) const;


      /** 
       * Returns the balance states of a list of accounts.
       * 
       * Includes all balances that an account owns.
       *
       * @param names The names of the accounts to provide balance information.
       * @returns Balance state information pertaining to the specified accounts.
       */
      vector< account_balance_state >                 get_account_balances( vector< string > names ) const;


      /** 
       * Returns the confidential balances from a query.
       * 
       * Includes all balances with specified owner authorities and 
       * commitments.
       *
       * @param query The specified list of authorities and commitments.
       * @returns Balance state information pertaining to the specified accounts.
       */
      vector< confidential_balance_api_obj >          get_confidential_balances( const confidential_query& query ) const;


      /** 
       * Returns the key state sets of a list of accounts.
       * 
       * Includes all key objects from accounts and communities that are connected.
       *
       * @param names The names of the accounts to provide key information.
       * @returns Key state information pertaining to the specified accounts.
       */
      vector< key_state >                             get_keychains( vector< string > names ) const;


      /** 
       * Lists all account names registered on the blockchain.
       * 
       * This returns a list of all account names sorted by account name.
       *
       * Use the lowerbound and limit parameters to page through the list.
       * To retrieve all accounts, start by setting lowerbound to the empty string "",
       * and then each iteration, pass the last account name returned 
       * as the lowerbound for the next list_accounts() call.
       *
       * @param lower_bound_name The name of the first account to return.
       * @param limit Maximum number of accounts to return (max: 1000).
       * @returns List of alphabetically ordered account names.
       */
      set< string >                                   lookup_accounts( string lower_bound_name, uint32_t limit )const;


      /** 
       * Returns the number of registered accounts.
       * 
       * @returns Number of accounts that have been created on the network.
       */
      uint64_t                                        get_account_count()const;


      //===================//
      // === Asset API === //
      //===================//


      /** 
       * Returns all available information about a list of specified assets.
       *
       * @param assets The list of assets to retrieve information about.
       * @returns Asset Objects information pertaining to the specified list of assets.
       */
      vector< extended_asset >                        get_assets( vector< string > assets )const;


      /** 
       * Retrieves the details of an active escrow transfer.
       *
       * @param from The account that is the sender of the funds into the escrow.
       * @param escrow_id The uuidv4 of the escrow transfer.
       * @returns Escrow information of the transfer with the specified id. 
       */
      optional< escrow_api_obj >                      get_escrow( string from, string escrow_id )const;



      //=======================//
      // === Community API === //
      //=======================//


      /** 
       * Returns all available information about a specified list of communities.
       *
       * @param communities List of communities to retrieve information about.
       * @returns Community Object information pertaining to the specified community.
       */
      vector< extended_community >                    get_communities( vector< string > communities )const;



      /** 
       * Returns a list of the communities with the highest number of subscribers.
       *
       * @param from First community to retrieve in the ranking order.
       * @param limit Amount of communities to retrieve.
       * @returns List of Community Objects pertaining to the top subscribed communities.
       */
      vector< extended_community >                    get_communities_by_subscribers( string from, uint32_t limit )const;


      //=====================//
      // === Network API === //
      //=====================//


      /** 
       * Returns information about a list of accounts.
       * 
       * @param names List of names of the accounts.
       * @returns All Network objects related to each account.
       */
      vector< account_network_state >                 get_account_network_state( vector< string > names )const;


      /**
       * Retrieves the current list of active block producers in the current round.
       * 
       * @returns All Producers in the current round of block production.
       */
      vector< account_name_type >                     get_active_producers()const;

      
      /** 
       * Returns a list of the producers with the highest voting power from stakeholders.
       *
       * @param from First producer to retrieve in the ranking order.
       * @param limit Amount of producers to retrieve.
       * @returns List of Producer Objects pertaining to the top voting producers.
       */
      vector< producer_api_obj >                      get_producers_by_voting_power( string from, uint32_t limit )const;


      /** 
       * Returns a list of the producers with the highest mining power from proofs of work.
       *
       * @param from First producer to retrieve in the ranking order.
       * @param limit Amount of producers to retrieve.
       * @returns List of Producer Objects pertaining to the top mining producers.
       */
      vector< producer_api_obj >                      get_producers_by_mining_power( string from, uint32_t limit )const;


      /** 
       * Returns a list of the network development officers with the highest voting power from stakeholders.
       *
       * @param currency Symbol of the Currency to retrieve officers from.
       * @param from First officer to retrieve in the ranking order.
       * @param limit Amount of officers to retrieve.
       * @returns List of network officer objects pertaining to the top officers.
       */
      vector< network_officer_api_obj >               get_development_officers_by_voting_power( string currency, string from, uint32_t limit )const;


      /** 
       * Returns a list of the network marketing officers with the highest voting power from stakeholders.
       *
       * @param currency Symbol of the Currency to retrieve officers from.
       * @param from First officer to retrieve in the ranking order.
       * @param limit Amount of officers to retrieve.
       * @returns List of network officer objects pertaining to the top officers.
       */
      vector< network_officer_api_obj >               get_marketing_officers_by_voting_power( string currency, string from, uint32_t limit )const;


      /** 
       * Returns a list of the network advocacy officers with the highest voting power from stakeholders.
       *
       * @param currency Symbol of the Currency to retrieve officers from.
       * @param from First officer to retrieve in the ranking order.
       * @param limit Amount of officers to retrieve.
       * @returns List of network officer objects pertaining to the top officers.
       */
      vector< network_officer_api_obj >               get_advocacy_officers_by_voting_power( string currency, string from, uint32_t limit )const;


      /** 
       * Returns a list of the supernodes with the highest view weight from stakeholders.
       *
       * @param from The first account in the rankings to retrieve.
       * @param limit The amount of supernodes to return.
       * @returns List of Supernodes with the highest stakeholder view weight.
       */
      vector< supernode_api_obj >                     get_supernodes_by_view_weight( string from, uint32_t limit )const;


      /** 
       * Returns a list of the interfaces with the highest recent view count.
       *
       * @param from The first account in the rankings to retrieve.
       * @param limit The amount of interfaces to return.
       * @returns List of interfaces with the highest recent view count.
       */
      vector< interface_api_obj >                     get_interfaces_by_users( string from, uint32_t limit )const;
      

      /** 
       * Returns a list of the governance accounts with the highest subscriber voting power.
       *
       * @param from The first account in the rankings to retrieve.
       * @param limit The amount of governance accounts to return.
       * @returns List of governance accounts with the highest subscriber voting power.
       */
      vector< governance_api_obj >            get_governances_by_members( string from, uint32_t limit )const;


      /** 
       * Returns a list of the community enterprise proposals with the highest voting power from total approvals.
       *
       * @param from The creator of the first enterprise in the rankings to retrieve.
       * @param from_id The enterprise_id of the first enterprise in the rankings to retrieve.
       * @param limit The amount of enterprise proposals to return.
       * @returns List of enterprise proposals with the highest voting power from total approvals.
       */
      vector< enterprise_api_obj >          get_enterprise_by_voting_power( string from, string from_id, uint32_t limit )const;



      //====================//
      // === Market API === //
      //====================//



      /** 
       * Returns a list of open limit, margin, and call orders from a specified list of accounts.
       *
       * @param names List of account names
       * @returns Open orders of the specified list of accounts.
       */
      vector< order_state >                           get_open_orders( vector< string > names )const;


      /** 
       * Returns a list of open limit orders on a market price pair.
       *
       * @param buy_symbol Asset to be the base price of ask orders.
       * @param sell_symbol Asset to the the base price of bid orders.
       * @param limit Maximum number of orders to retrieve from both sides of the orderbook.
       * @returns Orders in a specified price pair.
       */
      market_limit_orders                             get_limit_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;


      /** 
       * Returns a list of open margin orders on a market price pair.
       *
       * @param buy_symbol Asset to be the base price of ask orders.
       * @param sell_symbol Asset to the the base price of bid orders.
       * @param limit Maximum number of orders to retrieve from both sides of the orderbook.
       * @returns Orders in a specified price pair.
       */
      market_margin_orders                            get_margin_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;


      /** 
       * Returns a list of open option orders on a market price pair.
       *
       * @param buy_symbol Asset to be the base price of ask orders.
       * @param sell_symbol Asset to the the base price of bid orders.
       * @param limit Maximum number of orders to retrieve from both sides of the orderbook.
       * @returns Orders in a specified price pair.
       */
      market_option_orders                            get_option_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;


      /** 
       * Returns a list of open call orders on a market price pair.
       *
       * @param buy_symbol Asset to be the base price of ask orders.
       * @param sell_symbol Asset to the the base price of bid orders.
       * @param limit Maximum number of orders to retrieve from both sides of the orderbook.
       * @returns Orders in a specified price pair.
       */
      market_call_orders                              get_call_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;


      /** 
       * Returns a list of open auction orders on a market price pair.
       *
       * @param buy_symbol Asset to be the base price of ask orders.
       * @param sell_symbol Asset to the the base price of bid orders.
       * @param limit Maximum number of orders to retrieve from both sides of the orderbook.
       * @returns Orders in a specified price pair.
       */
      market_auction_orders                           get_auction_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;


      /** 
       * Returns a list of open loans on a market price pair, between debt and collateral assets.
       *
       * @param buy_symbol Asset to be the base price of ask orders.
       * @param sell_symbol Asset to the the base price of bid orders.
       * @param limit Maximum number of orders to retrieve from both sides of the orderbook.
       * @returns Orders in a specified price pair.
       */
      market_credit_loans                             get_credit_loans( string buy_symbol, string sell_symbol, uint32_t limit ) const;


      /** 
       * Returns a set of the credit pools available on a list of assets.
       *
       * @param assets list of symbols of assets that credit pools should be included from.
       * @returns Credit pool objects from a list of assets.
       */
      vector< credit_pool_api_obj >                   get_credit_pools( vector< string > assets )const;


      /** 
       * Returns a set of the credit pools available on a list of assets.
       *
       * @param buy_symbol Asset to be the included in the liquidity pools.
       * @param sell_symbol Asset to be the included in the liquidity pools.
       * @returns Liquidity pool objects relating to the specified symbols.
       */
      vector< liquidity_pool_api_obj >                get_liquidity_pools( string buy_symbol, string sell_symbol )const;


      /** 
       * Returns a set of the option pools available on a list of assets.
       *
       * @param buy_symbol Asset to be the included in the option pools.
       * @param sell_symbol Asset to be the included in the option pools.
       * @returns Option pool objects relating to the specified symbols.
       */
      vector< option_pool_api_obj >                   get_option_pools( string buy_symbol, string sell_symbol )const;


      /** 
       * Retrieves a full state of orders on an asset trading pair, including all open orders and asset pools.
       *
       * @param buy_symbol Asset to be the included in the market state.
       * @param sell_symbol Asset to be the included in the market state.
       * @returns Full state of the market trading asset pair.
       */
      market_state                                    get_market_state( string buy_symbol, string sell_symbol )const;



      //================//
      // === Ad API === //
      //================//



      /** 
       * Returns a list of active ad objects operated by a specifed list of accounts.
       * 
       * - Creatives
       * - Campaigns
       * - Audiences
       * - Inventory
       * - Bids: created by this account, incoming on creative, outgoing from campaigns, incoming on inventory.
       *
       * @param names List of account names to include in ad state objects.
       * @returns Active ad objects of the specified list of accounts.
       */
      vector< account_ad_state >                      get_account_ads( vector< string > names )const;


      /** 
       * Returns the state of bids in response to an ad query.
       *
       * @param query Details of the interface, browsing account, format of display ads.
       * @returns Active ad bids that fit the query parameters.
       */
      vector< ad_bid_state >                          get_interface_audience_bids( const ad_query& query )const;


      //=====================//
      // === Product API === //
      //=====================//


      /** 
       * Returns a specified product.
       *
       * @param seller The seller account of the product.
       * @param product_id uuidv4 of the product.
       * @returns Product sale details.
       */
      product_sale_api_obj                            get_product_sale( string seller, string product_id )const;

      /** 
       * Returns a specified product.
       *
       * @param seller The seller account of the product.
       * @param auction_id uuidv4 of the auction.
       * @returns Product auction details.
       */
      product_auction_sale_api_obj                    get_product_auction_sale( string seller, string auction_id )const;


      /** 
       * Returns all product offers by a specified list of sellers.
       *
       * @param names The seller accounts to retrieve.
       * @returns Products, auctions, purchases and bids.
       */
      vector< account_product_state >                 get_account_products( vector< string > names )const;



      //===================//
      // === Graph API === //
      //===================//

      
      /** 
       * Returns a specified set of nodes and edges.
       *
       * @param query The query details for nodes and edges to retrieve.
       * @returns Nodes and edges matching the query
       */
      graph_data_state                                get_graph_query( const graph_query& query )const;


      /** 
       * Returns the properties of a list of specified node types.
       *
       * @param names The accounts to retrieve.
       * @returns Node property details of node types.
       */
      vector< graph_node_property_api_obj >           get_graph_node_properties( vector< string > names )const;


      /** 
       * Returns the properties of a list of specified edge types.
       *
       * @param names The accounts to retrieve.
       * @returns Edge property details of node types.
       */
      vector< graph_edge_property_api_obj >           get_graph_edge_properties( vector< string > names )const;


      //====================//
      // === Search API === //
      //====================//



      /** 
       * Returns the results of a search query.
       *
       * @param query Details of the search term, result types, and searching account.
       * @returns Active ad bids that fit the query parameters.
       */
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
       * @param tx The unsigned transaction.
       * @param broadcast Set True to broadcast transaction.
       * @return The signed version of the transaction.
       */
      annotated_signed_transaction                    sign_transaction( signed_transaction tx, bool broadcast );

      /** 
       * Returns an uninitialized object representing a given blockchain operation.
       *
       * Returns a default-initialized object of the given operation type
       * for custom transaction construction.
       *
       * Any operation the blockchain supports can be created using the transaction builder's
       * add_operation_to_builder_transaction() , describes the
       * JSON form of the operation for reference. 
       * 
       * This will give you a template you can fill in.
       *
       * @param operation_type Type of operation to return, must be defined in `node/chain/operations.hpp`
       * @return A default-constructed operation of the given type.
       */
      operation                                       get_prototype_operation( string operation_type );


      /**
       * Adds a set of specified nodes to the nodes peers.
       * 
       * @param nodes List of strings referring to the nodes.
       */
      void                                            network_add_nodes( const vector<string>& nodes );


      /**
       * Finds the nodes that are currently connected as peers to the node. 
       * 
       * @returns List of connected nodes.
       */
      vector< variant >                               network_get_connected_peers();

      /** 
       * Returns the information about a block.
       *
       * @param num Block number requested.
       * @returns Public block data of the block at the specified height.
       */
      optional< signed_block_api_obj >                get_block( uint64_t num );

      /** 
       * Returns sequence of operations included or generated in a specified block.
       *
       * @param block_num Block height of specified block.
       * @param only_virtual Whether to only return virtual operations.
       * @returns Operations included in a specified block.
       */
      vector< applied_operation >                     get_ops_in_block( uint64_t block_num, bool only_virtual = true );

      /**
       * Returns transaction by ID.
       * 
       * @param trx_id Transaction ID of specified transaction.
       * @returns Annotated signed transaction with trx_id
       */
      annotated_signed_transaction                    get_transaction( transaction_id_type trx_id )const;

      /**
       * Convert a JSON transaction to its transaction ID.
       * 
       * @param trx A transaction to find the ID of. 
       */
      transaction_id_type                             get_transaction_id( const signed_transaction& trx )const;



      //========================//
      // === Post + Tag API === //
      //========================//



      /**
       * Returns the votes, views, shares, and moderation tags that have been made on a specified comment.
       * 
       * @param author Author of the comment.
       * @param permlink Permlink of the comment.
       * @returns List of the state of all votes, views, shares, and moderation tags made on the comment.
       */
      comment_interaction_state            get_comment_interactions( string author, string permlink )const;


      /**
       * Returns the votes that an account has made.
       * 
       * @param account The creator of the vote.
       * @param from_author Author of the first comment.
       * @param from_permlink Permlink of the first comment.
       * @param limit The amount of votes to include.
       * @returns List of the votes made by the account.
       */
      vector< account_vote >               get_account_votes( string account, string from_author, string from_permlink, uint32_t limit )const;


      /**
       * Returns the views that an account has made.
       * 
       * @param account The creator of the views.
       * @param from_author Author of the first comment.
       * @param from_permlink Permlink of the first comment.
       * @param limit The amount of views to include.
       * @returns List of the views made by the account.
       */
      vector< account_view >               get_account_views( string account, string from_author, string from_permlink, uint32_t limit )const;


      /**
       * Returns the shares that an account has made.
       * 
       * @param account The creator of the shares.
       * @param from_author Author of the first comment.
       * @param from_permlink Permlink of the first comment.
       * @param limit The amount of shares to include.
       * @returns List of the shares made by the account.
       */
      vector< account_share >              get_account_shares( string account, string from_author, string from_permlink, uint32_t limit )const;


      /**
       * Returns the moderation tags that an account has made.
       * 
       * @param account The creator of the moderation tags.
       * @param from_author Author of the first comment.
       * @param from_permlink Permlink of the first comment.
       * @param limit The amount of moderation tags to include.
       * @returns List of the moderation tags made by the account.
       */
      vector< account_moderation >         get_account_moderation( string account, string from_author, string from_permlink, uint32_t limit )const;


      /**
       * Retrieves the tag following information of a specified list of tags.
       * 
       * @param tags List of tags to return. 
       * @returns Accounts that follow each tag.
       */
      vector< account_tag_following_api_obj >      get_account_tag_followings( vector< string > tags )const;


      /**
       * Retrieves the top tags by the number of posts made using them.
       * 
       * @param after_tag The first tag to return.
       * @param limit the number of tags to return. 
       * @returns tag details for the top tags
       */
      vector< tag_api_obj >                get_top_tags( string after_tag, uint32_t limit )const;


      /**
       * Retrieves the top tags used by a specifed author.
       * 
       * @param author The author to query.
       * @returns The tags used by an author the most. 
       */
      vector< pair< tag_name_type, uint32_t > >   get_tags_used_by_author( string author )const;



      //========================//
      // === Discussion API === //
      //========================//

      /**
       * Retrieves the Discussion information relating to a post with a specified author/permlink.
       * 
       * @param author The author to query.
       * @param permlink The Permlink of the post to query.
       * @returns The Discussion information from the post. 
       */
      discussion                           get_content( string author, string permlink )const;
      

      /**
       * Retrieves the Discussion information relating to the comments of a parent post with a specified author/permlink.
       * 
       * @param parent The parent author to query.
       * @param parent_permlink The parent Permlink of the post to query.
       * @returns The Discussion information from the comments of the post. 
       */
      vector< discussion >                 get_content_replies( string parent, string parent_permlink )const;


      /**
       * Retrieves the Discussion information relating to the comments most recently updated on posts made by a given author.
       * 
       * @param start_author The author to query.
       * @param start_permlink The Permlink of the first post to return.
       * @param limit the maximum number of replies to retrieve.
       * @returns The Discussion information from the comments made on posts by the author.
       */
      vector< discussion >                 get_replies_by_last_update( account_name_type start_author, string start_permlink, uint32_t limit )const;


      /**
       * Retrieves the Discussion information from a variety of variable sorting option combinations.
       * 
       * Can use any of 55 combinations of sorting type and sorting time preference options.
       * to get a customized slice of the posts according to desired metrics and combinations of them.
       * Utilizes the WeYouMe Sorting Algorithm, which combines both power and absolute amount of
       * votes, views, shares and comments, and balances against time passed since uploading according to 
       * the time preference option. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_sort_rank( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from posts that are in an account's feeds.
       * 
       * Feeds can include posts made by followed and connected accounts, as well as from
       * followed communities, and followed tags. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_feed( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from posts that are in a specified blog from an account, community, or tag.
       * 
       * Blogs include all posts that are created by an account, created with a specifed tag, or in a specifed community.
       * in addition to posts that are shared by the account, or shared with the community or tag. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_blog( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from posts that are featured.
       * 
       * Featured posts include a stream of posts made by member account authors
       * that are highly voted, shared, viewed and commented on in a 24 hour period.
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_featured( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from posts that are recommended for an account.
       * 
       * Recommended posts include a randomized selection of not yet viewed posts from authors, communities and tags
       * that are from authors, communities, and tags that the account 
       * has previously positively engaged with, and authors communities and tags that are closely related to
       * those that are engaged with, but not yet followed.
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_recommended( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information relating to the comments most recently updated by a given author.
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the comments of the author.
       */
      vector< discussion >                 get_discussions_by_comments( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the posts and comments with the highest amount of net rewards.
       * 
       * @param query The details of the posts to return.
       * @returns The Discussion information from the posts and comments.
       */
      vector< discussion >                 get_discussions_by_payout(const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the posts with the highest amount of net rewards.
       * 
       * @param query The details of the posts to return.
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_post_discussions_by_payout( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the comments with the highest amount of net rewards.
       * 
       * @param query The details of the comments to return.
       * @returns The Discussion information from the comments.
       */
      vector< discussion >                 get_comment_discussions_by_payout( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the most recently created posts.
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_created( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the posts that have been most recently created, updated, or replied to.
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_active( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the posts with the highest absolute amount of upvotes. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_votes( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the posts with the highest absolute amount of views. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_views( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the posts with the highest absolute amount of shares. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_shares( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the posts with the highest absolute amount of comments. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_children( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the posts with the highest stake weighted voting power. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_vote_power( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the posts with the highest stake weighted viewing power. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_view_power( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the posts with the highest stake weighted sharing power. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_share_power( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from the posts with the highest stake weighted commenting power. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_comment_power( const discussion_query& query )const;


      
      //===================//
      // === State API === //
      //===================//


      /**
       * Returns the state information associated with the URL specified.
       * 
       * @param url The string of the current URL.
       * @returns state object containing the necessary details for the application page.
       */
      app::state                           get_state( string url );



      //==============================//
      // === Account Transactions === //
      //==============================//


      /**
       * Generates a new Account with specified authorities.
       *
       * @param registrar The account creating the new account.
       * @param new_account_name The name of the new account.
       * @param referrer The name of the account that lead to the creation of the account.
       * @param proxy Account that is able to vote on behalf of the account.
       * @param recovery_account Account able to create recovery requests if the key is compromised.
       * @param reset_account Account able to reset the owner key after inactivity.
       * @param details The account's details string.
       * @param url The account's selected personal URL.
       * @param profile_image The account's Public profile image IPFS reference.
       * @param cover_image The account's Public cover image IPFS reference.
       * @param json The JSON string of public profile information.
       * @param json_private The JSON string of encrypted profile information.
       * @param first_name The user's Encrypted first name. 
       * @param last_name The user's Encrypted last name. 
       * @param gender The user's Encrypted specified gender.
       * @param date_of_birth The user's Encrypted date of birth.
       * @param email The user's Encrypted email address.
       * @param phone The user's Encrypted phone number.
       * @param nationality The user's Encrypted country of residence.
       * @param relationship The user's Encrypted relationship status.
       * @param owner The account authority required for changing the active and posting authorities.
       * @param active The account authority required for sending payments and trading.
       * @param posting The account authority required for posting content and voting.
       * @param secure_public_key The secure encryption key for content only visible to this account.
       * @param connection_public_key The connection public key used for encrypting Connection level content.
       * @param friend_public_key The connection public key used for encrypting Friend level content.
       * @param companion_public_key The connection public key used for encrypting Companion level content.
       * @param fee Account creation fee for stake on the new account.
       * @param delegation Initial amount delegated to the new account.
       * @param generate_keys Set True to use keys generated locally by this wallet.
       * @param password Password to use for generating new keys.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_create(
         string registrar,
         string new_account_name,
         string referrer,
         string proxy,
         string recovery_account,
         string reset_account,
         string details,
         string url,
         string profile_image,
         string cover_image,
         string json,
         string json_private,
         string first_name,
         string last_name,
         string gender,
         string date_of_birth,
         string email,
         string phone,
         string nationality,
         string relationship,
         authority owner,
         authority active,
         authority posting,
         string secure_public_key,
         string connection_public_key,
         string friend_public_key,
         string companion_public_key,
         asset fee,
         asset delegation,
         bool generate_keys,
         string password,
         bool broadcast );
     

      /**
       * Update the details and authorities of an existing account.
       *
       * @param account The name of the new account.
       * @param details The account's details string.
       * @param url The account's selected personal URL.
       * @param profile_image The account's Public profile image IPFS reference.
       * @param cover_image The account's Public cover image IPFS reference.
       * @param json The JSON string of public profile information.
       * @param json_private The JSON string of encrypted profile information.
       * @param first_name The user's Encrypted first name. 
       * @param last_name The user's Encrypted last name. 
       * @param gender The user's Encrypted specified gender.
       * @param date_of_birth The user's Encrypted date of birth.
       * @param email The user's Encrypted email address.
       * @param phone The user's Encrypted phone number.
       * @param nationality The user's Encrypted country of residence.
       * @param relationship The user's Encrypted relationship status.
       * @param pinned_permlink The permlink of the pinned comment of the author's blog.
       * @param owner_auth The account authority required for changing the active and posting authorities.
       * @param active_auth The account authority required for sending payments and trading.
       * @param posting_auth The account authority required for posting content and voting.
       * @param secure_public_key The secure encryption key for content only visible to this account.
       * @param connection_public_key The connection public key used for encrypting Connection level content.
       * @param friend_public_key The connection public key used for encrypting Friend level content.
       * @param companion_public_key The connection public key used for encrypting Companion level content.
       * @param active True when account is active, false to deactivate.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_update( 
         string account,
         string details,
         string url,
         string profile_image,
         string cover_image,
         string json,
         string json_private,
         string first_name,
         string last_name,
         string gender,
         string date_of_birth,
         string email,
         string phone,
         string nationality,
         string relationship,
         string pinned_permlink,
         authority owner_auth,
         authority active_auth,
         authority posting_auth,
         string secure_public_key,
         string connection_public_key,
         string friend_public_key,
         string companion_public_key,
         bool active,
         bool broadcast );


      /**
       * Create or Update an account verification between two accounts.
       *
       * @param verifier_account The name of the account creating the verification.
       * @param verified_account The account being verified.
       * @param shared_image IPFS reference to the image of both account owners.
       * @param verified True when the verification is active, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_verification( 
         string verifier_account,
         string verified_account,
         string shared_image,
         bool verified,
         bool broadcast );


      /**
       * Activates membership on an account.
       *
       * @param account The name of the account to activate membership on.
       * @param membership_type The level of membership to activate on the account.
       * @param months Number of months to purchase membership for.
       * @param interface Name of the interface application facilitating the transaction.
       * @param recurring True for membership to automatically recur each month from liquid balance.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_membership(
         string account,
         string membership_type,
         uint16_t months,
         string interface, 
         bool recurring, 
         bool broadcast );


      /**
       * Blacklists an account or asset.
       *
       * @param account Name of account.
       * @param listed_account Name of account being added to a black or white list.
       * @param listed_asset Name of asset being added to a black or white list.
       * @param blacklisted True to add to blacklist, false to remove.
       * @param whitelisted True to add to whitelist, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_update_list(
         string account,
         string listed_account,
         string listed_asset,
         bool blacklisted,
         bool whitelisted,
         bool broadcast );


      /**
       * Vote for a Producer for selection to producer blocks. 
       *
       * @param account The account voting for a producer.
       * @param vote_rank Rank ordering of the vote.
       * @param producer The producer that is being voted for.
       * @param approved  True to create vote, false to remove vote.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_producer_vote(
         string account,
         uint16_t vote_rank,
         string producer,
         bool approved,
         bool broadcast );


      /** 
       * Set the voting proxy for an account.
       *
       * @param account The name of the account to update.
       * @param proxy The name of account that should proxy to, or empty string to have no proxy.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_update_proxy(
         string account,
         string proxy,
         bool broadcast );


      /**
       * Create an account recovery request as a recover account. 
       *
       * Authority object Structure:
       * {"weight_threshold": 1,"account_auths": [], "key_auths": [["new_public_key",1]]}
       *
       * @param recovery_account The recovery account is listed as the recovery account on the account to recover.
       * @param account_to_recover The account to recover. This is likely due to a compromised owner authority.
       * @param new_owner_authority The new owner authority the account to recover wishes to have.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_request_recovery(
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
       * @param account_to_recover The account to be recovered.
       * @param new_owner_authority The new authority that your recovery account used in the account recover request.
       * @param recent_owner_authority A recent owner authority on your account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_recover(
         string account_to_recover,
         authority new_owner_authority,
         authority recent_owner_authority,
         bool broadcast );

      
      /**
       * Allows a nominated reset account to change an account's owner authority.
       *
       * @param reset_account The account that is initiating the reset process.
       * @param account_to_reset The Account to be reset.
       * @param new_owner_authority A recent owner authority on your account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_reset(
         string reset_account,
         string account_to_reset,
         authority new_owner_authority,
         bool broadcast );


      /**
       * Updates an accounts specified reset account.
       *
       * @param account Account to update.
       * @param new_reset_account Account that has the new authority to reset the account.
       * @param days Days of inactivity required to reset the account. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_reset_update(
         string account,
         string new_reset_account,
         uint16_t days,
         bool broadcast );


      /**
       * Change your recovery account after a 30 day delay.
       *
       * @param account_to_recover The account being updated.
       * @param new_recovery_account The account that is authorized to create recovery requests.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_recovery_update(
         string account_to_recover, 
         string new_recovery_account, 
         bool broadcast );


      /**
       * Removes an account's ability to vote in perpetuity.
       *
       * @param account The account being updated.
       * @param declined True to decine voting rights, false to cancel pending request.
       * @param broadcast Set True to broadcast transaction.
       */ 
      annotated_signed_transaction           account_decline_voting(
         string account, 
         bool declined, 
         bool broadcast );


      /**
       * Accepts an incoming connection request by providing an encrypted posting key.
       *
       * @param account Account accepting the request.
       * @param connecting_account Account that originally requested to connect.
       * @param connection_id uuidv4 for the connection, for local storage of decryption key.
       * @param connection_type Type of connection level.
       * @param encrypted_key The private connection key of the user, encrypted with the public secure key of the requesting account.
       * @param connected Set true to connect, false to delete connection.
       * @param broadcast Set True to broadcast transaction.
       */ 
      annotated_signed_transaction           account_connection(
         string account,
         string connecting_account,
         string connection_id,
         string connection_type,
         string encrypted_key,
         bool connected, 
         bool broadcast );


      /**
       * Enables an account to follow another account.
       *
       * @param follower Account that is creating the new follow relationship.
       * @param following Account that is being followed by follower.
       * @param interface Account of the interface facilitating the transaction broadcast.
       * @param added Set true to add to list, false to remove from list.
       * @param followed Set true to follow, false to filter.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_follow(
         string follower,
         string following,
         string interface,
         bool added,
         bool followed,
         bool broadcast );

      
      /**
       * Enables an account to follow a tag.
       *
       * @param follower Name of the account following the tag.
       * @param tag Tag being followed.
       * @param interface Account of the interface facilitating the transaction broadcast.
       * @param added Set true to add to list, false to remove from list.
       * @param followed Set true to follow, false to filter.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_follow_tag(
         string follower,
         string tag,
         string interface,
         bool added,
         bool followed,
         bool broadcast );


      /**
       * Claims an account's daily activity reward.
       *
       * @param account Name of the account claiming the reward.
       * @param permlink Permlink of the users recent post in the last 24h.
       * @param interface Account of the interface facilitating the transaction broadcast.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_activity(
         string account,
         string permlink,
         string interface,
         bool broadcast );


      //===============================//
      // === Business Transactions === //
      //===============================//


      /**
       * Creates a Business Entity Integrated Structure.
       *
       * @param account The name of the Account creating the executive vote.
       * @param founder Account registering the new business.
       * @param new_business_name The name of the new business underlying account.
       * @param new_business_trading_name Trading business name, UPPERCASE letters only.
       * @param details The account's Public details string. Profile Biography.
       * @param url The account's Public URL.
       * @param profile_image IPFS Reference of the Public profile image of the account.
       * @param cover_image IPFS Reference of the Public cover image of the account.
       * @param secure_public_key The secure encryption key for content only visible to this account.
       * @param connection_public_key The connection public key used for encrypting Connection level content.
       * @param friend_public_key The connection public key used for encrypting Friend level content.
       * @param companion_public_key The connection public key used for encrypting Companion level content.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param equity_asset Equity assets that offer dividends and voting power over the Business Account's structure.
       * @param equity_revenue_share Equity asset that the account distributes a percentage of incoming revenue to as dividend payments.
       * @param equity_options Series of options parameters that apply to the Equity Asset.
       * @param credit_asset Credit asset that offer interest and buybacks from the business account.
       * @param credit_revenue_share Credit asset that the account uses a percentage of incoming revenue to repurchase at face value.
       * @param credit_options Series of options parameters that apply to the Credit Asset.
       * @param public_community Name of the business public community.
       * @param public_display_name Display name of the business public community.
       * @param public_community_member_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.
       * @param public_community_moderator_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.
       * @param public_community_admin_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.
       * @param public_community_secure_key Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.
       * @param public_community_standard_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.
       * @param public_community_mid_premium_key; Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.
       * @param public_community_top_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.
       * @param private_community Name of the business private community. Should be randomized string.
       * @param private_display_name Display name of the business private community.
       * @param private_community_member_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.
       * @param private_community_moderator_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.
       * @param private_community_admin_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.
       * @param private_community_secure_key Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.
       * @param private_community_standard_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.
       * @param private_community_mid_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.
       * @param private_community_top_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.
       * @param reward_currency The Currency asset used for content rewards in the community.
       * @param standard_membership_price Price paid per month by community standard members.
       * @param mid_membership_price Price paid per month by all mid level community members.
       * @param top_membership_price Price paid per month by all top level community members.
       * @param coin_liquidity Amount of COIN asset to inject into the Coin liquidity pool.
       * @param usd_liquidity Amount of USD asset to inject into the USD liquidity pool.
       * @param credit_liquidity Amount of the new asset to issue and inject into the credit pool.
       * @param fee Account creation fee for stake on the new account.
       * @param delegation Initial amount delegated to the new account.
       */
      annotated_signed_transaction           business_create(
         string founder,
         string new_business_name,
         string new_business_trading_name,
         string details,
         string url,
         string profile_image,
         string cover_image,
         string secure_public_key,
         string connection_public_key,
         string friend_public_key,
         string companion_public_key,
         string interface,
         string equity_asset,
         uint16_t equity_revenue_share,
         asset_options equity_options,
         string credit_asset,
         uint16_t credit_revenue_share,
         asset_options credit_options,
         string public_community,
         string public_display_name,
         string public_community_member_key,
         string public_community_moderator_key,
         string public_community_admin_key,
         string public_community_secure_key,
         string public_community_standard_premium_key,
         string public_community_mid_premium_key,
         string public_community_top_premium_key,
         string private_community,
         string private_display_name,
         string private_community_member_key,
         string private_community_moderator_key,
         string private_community_admin_key,
         string private_community_secure_key,
         string private_community_standard_premium_key,
         string private_community_mid_premium_key,
         string private_community_top_premium_key,
         string reward_currency,
         asset standard_membership_price,
         asset mid_membership_price,
         asset top_membership_price,
         asset coin_liquidity,
         asset usd_liquidity,
         asset credit_liquidity,
         asset fee,
         asset delegation,
         bool broadcast );


      /**
       * Creates a Business Entity Integrated Structure.
       *
       * @param chief_executive Name of the Chief Executive Officer of the Business.
       * @param business Name of the Business Account to update.
       * @param business_trading_name Trading business name, UPPERCASE letters only.
       * @param equity_revenue_share Equity asset that the account distributes a percentage of incoming revenue to as dividend payments.
       * @param credit_revenue_share Credit asset that the account uses a percentage of incoming revenue to repurchase at face value.
       * @param executives Set of all Executive accounts appointed by the Chief Executive Officer.
       * @param active  True when the business account is active, false to dissolve Business Account.
       */
      annotated_signed_transaction           business_update(
         string chief_executive,
         string business,
         string business_trading_name,
         uint16_t equity_revenue_share,
         uint16_t credit_revenue_share,
         vector< string > executives,
         bool active,
         bool broadcast );


      /**
       * Create an executive of a business account.
       *
       * @param executive Name of Executive Account.
       * @param business Business account that the executive is being created for.
       * @param active True to activate Executive, false to deactivate.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           business_executive(
         string executive,
         string business,
         bool active,
         bool broadcast );


      /**
       * Votes for an account to become an executive of a business account.
       *
       * @param director Director Account voting for the Executive.
       * @param executive The Name of executive being voted for.
       * @param business Business account that the executive is being voted for.
       * @param approved True to add, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           business_executive_vote(
         string director,
         string executive,
         string business,
         bool approved,
         bool broadcast );

      
      /**
       * Create a director of a business account.
       *
       * @param director Name of the director being created.
       * @param business Business account that the director is being created for
       * @param active True to activate Executive, false to deactivate.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           business_director(
         string director,
         string business,
         bool active,
         bool broadcast );


      /**
       * Votes for an account to become a director of a business account.
       *
       * @param account Name of the voting account.
       * @param director The Name of the director being voted for.
       * @param business Business account that the director is being voted for.
       * @param vote_rank Rank of voting preference.
       * @param approved True to add, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           business_director_vote(
         string account,
         string director,
         string business,
         uint16_t vote_rank,
         bool approved,
         bool broadcast );



      //=================================//
      // === Governance Transactions === //
      //=================================//


      /**
       * Creates a governance Entity Integrated Structure.
       *
       * @param account The name of the Account creating the executive vote.
       * @param founder Account registering the new governance.
       * @param new_governance_name The name of the new governance underlying account.
       * @param new_governance_display_name Trading governance name, UPPERCASE letters only.
       * @param details The account's Public details string. Profile Biography.
       * @param url The account's Public URL.
       * @param profile_image IPFS Reference of the Public profile image of the account.
       * @param cover_image IPFS Reference of the Public cover image of the account.
       * @param secure_public_key The secure encryption key for content only visible to this account.
       * @param connection_public_key The connection public key used for encrypting Connection level content.
       * @param friend_public_key The connection public key used for encrypting Friend level content.
       * @param companion_public_key The connection public key used for encrypting Companion level content.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param equity_asset Equity assets that offer dividends and voting power over the governance Account's structure.
       * @param equity_revenue_share Equity asset that the account distributes a percentage of incoming revenue to as dividend payments.
       * @param equity_options Series of options parameters that apply to the Equity Asset.
       * @param credit_asset Credit asset that offer interest and buybacks from the governance account.
       * @param credit_revenue_share Credit asset that the account uses a percentage of incoming revenue to repurchase at face value.
       * @param credit_options Series of options parameters that apply to the Credit Asset.
       * @param public_community Name of the governance public community.
       * @param public_display_name Display name of the governance public community.
       * @param public_community_member_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.
       * @param public_community_moderator_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.
       * @param public_community_admin_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.
       * @param public_community_secure_key Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.
       * @param public_community_standard_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.
       * @param public_community_mid_premium_key; Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.
       * @param public_community_top_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.
       * @param private_community Name of the governance private community. Should be randomized string.
       * @param private_display_name Display name of the governance private community.
       * @param private_community_member_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.
       * @param private_community_moderator_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.
       * @param private_community_admin_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.
       * @param private_community_secure_key Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.
       * @param private_community_standard_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.
       * @param private_community_mid_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.
       * @param private_community_top_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.
       * @param reward_currency The Currency asset used for content rewards in the community.
       * @param standard_membership_price Price paid per month by community standard members.
       * @param mid_membership_price Price paid per month by all mid level community members.
       * @param top_membership_price Price paid per month by all top level community members.
       * @param coin_liquidity Amount of COIN asset to inject into the Coin liquidity pool.
       * @param usd_liquidity Amount of USD asset to inject into the USD liquidity pool.
       * @param credit_liquidity Amount of the new asset to issue and inject into the credit pool.
       * @param fee Account creation fee for stake on the new account.
       * @param delegation Initial amount delegated to the new account.
       */
      annotated_signed_transaction           governance_create(
         string founder,
         string new_governance_name,
         string new_governance_display_name,
         string details,
         string url,
         string profile_image,
         string cover_image,
         string secure_public_key,
         string connection_public_key,
         string friend_public_key,
         string companion_public_key,
         string interface,
         string equity_asset,
         uint16_t equity_revenue_share,
         asset_options equity_options,
         string credit_asset,
         uint16_t credit_revenue_share,
         asset_options credit_options,
         string public_community,
         string public_display_name,
         string public_community_member_key,
         string public_community_moderator_key,
         string public_community_admin_key,
         string public_community_secure_key,
         string public_community_standard_premium_key,
         string public_community_mid_premium_key,
         string public_community_top_premium_key,
         string private_community,
         string private_display_name,
         string private_community_member_key,
         string private_community_moderator_key,
         string private_community_admin_key,
         string private_community_secure_key,
         string private_community_standard_premium_key,
         string private_community_mid_premium_key,
         string private_community_top_premium_key,
         string reward_currency,
         asset standard_membership_price,
         asset mid_membership_price,
         asset top_membership_price,
         asset coin_liquidity,
         asset usd_liquidity,
         asset credit_liquidity,
         asset fee,
         asset delegation,
         bool broadcast );


      /**
       * Creates a governance Entity Integrated Structure.
       *
       * @param chief_executive Name of the Chief Executive Officer of the governance account.
       * @param governance Name of the governance Account to update.
       * @param governance_display_name Non-consensus Governance Account display name, UPPERCASE letters only.
       * @param equity_revenue_share Equity asset that the account distributes a percentage of incoming revenue to as dividend payments.
       * @param credit_revenue_share Credit asset that the account uses a percentage of incoming revenue to repurchase at face value.
       * @param executives Set of all Executive accounts appointed by the Chief Executive Officer.
       * @param active  True when the governance account is active, false to dissolve governance Account.
       */
      annotated_signed_transaction           governance_update(
         string chief_executive,
         string governance,
         string governance_display_name,
         uint16_t equity_revenue_share,
         uint16_t credit_revenue_share,
         vector< string > executives,
         bool active,
         bool broadcast );


      /**
       * Create an executive of a governance account.
       *
       * @param executive Name of Executive Account.
       * @param governance governance account that the executive is being created for.
       * @param active True to activate Executive, false to deactivate.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           governance_executive(
         string executive,
         string governance,
         bool active,
         bool broadcast );


      /**
       * Votes for an account to become an executive of a governance account.
       *
       * @param director Director Account voting for the Executive.
       * @param executive The Name of executive being voted for.
       * @param governance governance account that the executive is being voted for.
       * @param approved True to add, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           governance_executive_vote(
         string director,
         string executive,
         string governance,
         bool approved,
         bool broadcast );

      
      /**
       * Create a director of a governance account.
       *
       * @param director Name of the director being created.
       * @param governance governance account that the director is being created for
       * @param active True to activate Executive, false to deactivate.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           governance_director(
         string director,
         string governance,
         bool active,
         bool broadcast );


      /**
       * Votes for an account to become a director of a governance account.
       *
       * @param account Name of the voting account.
       * @param director The Name of the director being voted for.
       * @param governance governance account that the director is being voted for.
       * @param vote_rank Rank of voting preference.
       * @param approved True to add, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           governance_director_vote(
         string account,
         string director,
         string governance,
         uint16_t vote_rank,
         bool approved,
         bool broadcast );

      /**
       * Adds an account to the membership set of a Governance account. 
       *
       * @param governance The name of the governance account creating the membership.
       * @param account The account being added as a participating member of to the governance address.
       * @param interface Account of the interface signing the transaction.
       * @param approved True to approve the membership, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           governance_member(
         string governance,
         string account,
         string interface,
         bool approved,
         bool broadcast );

      /**
       * Creates a request to join a Governance account as a new member.
       *
       * @param account The account creating the request.
       * @param governance The name of the governance account that membership is requested for.
       * @param interface Account of the interface signing the transaction.
       * @param message Encrypted message to the Governance Account team, encrypted with Governance connection key.
       * @param active True to create request, false to cancel request.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           governance_member_request(
         string account,
         string governance,
         string interface,
         string message,
         bool active,
         bool broadcast );


      /**
       * Creates a Resolution for a Governance Account.
       *
       * @param governance The name of the governance account creating the Resolution.
       * @param resolution_id uuidv4 referring to the resolution.
       * @param ammendment_id uuidv4 referring to the resolution ammendment.
       * @param title Title of the resolution to be voted on.
       * @param details Short description text of purpose and summary of the resolution to be voted on.
       * @param body Full Text of the body of the resolution to be voted on.
       * @param url The Resolution description URL explaining more details.
       * @param json JSON metadata of the resolution.
       * @param interface Account of the interface that most recently updated the resolution.
       * @param completion_time Time the resolution completes.
       * @param active True to create request, false to cancel request.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           governance_resolution(
         string governance,
         string resolution_id,
         string ammendment_id,
         string title,
         string details,
         string body,
         string url,
         string json,
         string interface,
         time_point completion_time,
         bool active,
         bool broadcast );


      /**
       * Creates a Vote for a Governance Account Resolution.
       *
       * @param account The name of the account voting on the resolution.
       * @param governance The name of the governance account that created the Resolution.
       * @param resolution_id uuidv4 referring to the resolution.
       * @param ammendment_id uuidv4 referring to the resolution ammendment.
       * @param interface Account of the interface that most recently updated the resolution vote.
       * @param approved True to approve the resolution, false to disapprove resolution.
       * @param active True to activate a vote or update existing vote, false to retract existing vote.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           governance_resolution_vote(
         string account,
         string governance,
         string resolution_id,
         string ammendment_id,
         string interface,
         bool approved,
         bool active,
         bool broadcast );


      //==============================//
      // === Network Transactions === //
      //==============================//


      /**
       * Creates or updates a network officer object for a member.
       *
       * @param account Name of the member's account.
       * @param officer_type The type of network officer that the account serves as. 
       * @param details Information about the network officer and their work
       * @param url The officers's description URL explaining their details. 
       * @param json Additional information about the officer.
       * @param reward_currency Currency Asset for the Network Officer to earn.
       * @param active Set true to activate the officer, false to deactivate. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           network_officer_update(
         string account,
         string officer_type,
         string details,
         string url,
         string json,
         string reward_currency,
         bool active,
         bool broadcast );


      /**
       * Votes to support a network officer.
       *
       * @param account The name of the account voting for the officer.
       * @param network_officer The name of the network officer being voted for.
       * @param vote_rank Number of vote rank ordering.
       * @param approved True if approving, false if removing vote.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           network_officer_vote(
         string account,
         string network_officer,
         uint16_t vote_rank,
         bool approved,
         bool broadcast );


      /**
       * Creates or updates a supernode object for an infrastructure provider.
       *
       * @param account Name of the member's account.
       * @param details Information about the supernode, and the range of storage and node services they operate.
       * @param url The supernode's reference URL.
       * @param node_api_endpoint The Full Archive node public API endpoint of the supernode.
       * @param notification_api_endpoint The Notification API endpoint of the Supernode.
       * @param auth_api_endpoint The Transaction signing authentication API endpoint of the supernode.
       * @param ipfs_endpoint The IPFS file storage API endpoint of the supernode.
       * @param bittorrent_endpoint The Bittorrent Seed Box endpoint URL of the Supernode. 
       * @param json Additional information about the Supernode.
       * @param active Set true to activate the supernode, false to deactivate. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           supernode_update(
         string account,
         string details,
         string url,
         string node_api_endpoint,
         string notification_api_endpoint,
         string auth_api_endpoint,
         string ipfs_endpoint,
         string bittorrent_endpoint,
         string json,
         bool active,
         bool broadcast );

      
      /**
       * Creates or updates an interface object for an application developer.
       *
       * @param account Name of the member's account.
       * @param details Information about the interface, and what they are offering to users.
       * @param url The interface's reference URL.
       * @param json Additional information about the interface.
       * @param active Set true to activate the interface, false to deactivate. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           interface_update(
         string account,
         string details,
         string url,
         string json,
         bool active,
         bool broadcast );


      /**
       * Creates or updates a mediator object for marketplace escrow facilitator.
       *
       * @param account Name of the member's account.
       * @param details Information about the mediator, and what they are offering to users
       * @param url The mediator's reference URL.
       * @param json Additional information about the mediator.
       * @param mediator_bond Amount of Core asset to stake in the mediation pool. 
       * @param active Set true to activate the interface, false to deactivate. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           mediator_update(
         string account,
         string details,
         string url,
         string json,
         asset mediator_bond,
         bool active,
         bool broadcast );


      /**
       * Creates a new community enterprise proposal.
       *
       * @param creator The name of the account that created the community enterprise proposal.
       * @param enterprise_id uuidv4 referring to the proposal.
       * @param beneficiaries Set of account names and percentages of budget value. Should not include the null account.
       * @param milestone_shares Ordered vector of release milestone percentages of budget value.
       * @param details The proposals's details description. 
       * @param url The proposals's reference URL. 
       * @param json Json metadata of the proposal.
       * @param begin Enterprise proposal start time.
       * @param duration Number of days that the proposal will be paid for.
       * @param daily_budget Daily amount of Core asset requested for project compensation and funding
       * @param fee Amount of Core asset paid to community fund to apply.
       * @param active True to set the proposal to activate, false to deactivate an existing proposal and delay funding. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           enterprise_update(
         string creator,
         string enterprise_id,
         map< string, uint16_t > beneficiaries,
         vector< uint16_t > milestone_shares,
         string details,
         string url,
         string json,
         time_point begin,
         uint16_t duration,
         asset daily_budget,
         asset fee,
         bool active,
         bool broadcast );


      /**
       * Approves a milestone claim from a community enterprise proposal.
       *
       * @param account Account approving the milestone.
       * @param creator The name of the account that created the community enterprise proposal.
       * @param enterprise_id uuidv4 referring to the proposal.
       * @param milestone Number of the milestone that is being approved as completed. 
       * @param vote_rank The rank of the approval for enterprise proposals.
       * @param approved True to approve the milestone claim, false to remove approval. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           enterprise_vote(
         string account,
         string creator,
         string enterprise_id,
         uint16_t milestone,
         uint16_t vote_rank,
         bool approved,
         bool broadcast );


      /**
       * Claims a milestone from a community enterprise proposal.
       *
       * @param creator The name of the account that created the community enterprise proposal.
       * @param enterprise_id uuidv4 referring to the proposal.
       * @param milestone Number of the milestone that is being claimed as completed. Number 0 for initial acceptance. 
       * @param details Description of completion of milestone, with supporting evidence.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           enterprise_fund(
         string creator,
         string enterprise_id,
         uint16_t milestone,
         string details,
         bool broadcast );
         


      //=======================================//
      // === Post and Comment Transactions === //
      //=======================================//


      /**
       * Post or update a comment.
       * 
       * Uses comment options object with schematic:
       * {
       *    "post_type": "\"text\"",
       *    "reach": "\"tag\"",
       *    "rating": 9,
       *    "max_accepted_payout": "\"1000000000.00000000 MUSD\"",
       *    "percent_liquid": 10000,
       *    "allow_replies": true,
       *    "channel": false,
       *    "allow_votes": true,
       *    "allow_views": true,
       *    "allow_shares": true,
       *    "allow_curation_rewards": true,
       *    "beneficiaries": []
       * }
       *
       * @param editor Name of the account creating or editing the post. Original author, or a nominated collaborator.
       * @param author Name of the account that created the post.
       * @param permlink Unique identifing string for the post.
       * @param parent_author Account that created the post this post is replying to, empty if root post.
       * @param parent_permlink Permlink of the post this post is replying to, empty if root post.
       * @param title Content related name of the post, used to find post with search API.
       * @param body String containing text for display when the post is opened.
       * @param body_private String containing text for display when the post is opened.
       * @param url Plaintext URL for the post to link to. For a livestream post, should link to the streaming server embed.
       * @param url_private Private Encrypted ciphertext URL for the post to link to. For a livestream post, should link to the streaming server embed.
       * @param ipfs Public IPFS file hash: images, videos, files.
       * @param ipfs_private Private and encrypted IPFS file hash: images, videos, files.
       * @param magnet Bittorrent magnet links to torrent file swarms: videos, files.
       * @param magnet_private Bittorrent magnet links to Private and Encrypted torrent file swarms: videos, files.
       * @param json JSON string of additional interface specific data relating to the post.
       * @param json_private Private and Encrypted JSON string of additional interface specific data relating to the post.
       * @param language String containing the two letter ISO language code of the native language of the author.
       * @param public_key The public key used to encrypt the post, holders of the private key may decrypt.
       * @param community The name of the community to which the post is uploaded to.
       * @param tags Set of string tags for sorting the post by.
       * @param collaborating_authors Set of Authors that are able to create edits for the post.
       * @param supernodes Set of the Supernodes that the IPFS file is hosted with. Each should additionally hold the private key.
       * @param interface Name of the interface application that broadcasted the transaction.
       * @param latitude Latitude co-ordinates of the comment.
       * @param longitude Longitude co-ordinates of the comment.
       * @param comment_price Price that is required to comment on the post.
       * @param premium_price Price that is required to unlock premium content.
       * @param options Settings for the post, that effect how the network applies and displays it.
       * @param deleted True to delete post, false to create post.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           comment(
         string editor,
         string author,
         string permlink,
         string parent_author, 
         string parent_permlink, 
         string title,
         string body,
         string body_private,
         string url,
         string url_private,
         string ipfs,
         string ipfs_private,
         string magnet,
         string magnet_private,
         string json,
         string json_private,
         string language,
         string public_key,
         string community,
         vector< string > tags,
         vector< string > collaborating_authors,
         vector< string > supernodes,
         string interface,
         double latitude,
         double longitude,
         asset comment_price,
         asset premium_price,
         comment_options options,
         bool deleted,
         bool broadcast );
      

      /**
       * Votes for a comment to allocate content rewards and increase the posts ranked ordering.
       *
       * @param voter Name of the voting account.
       * @param author Name of the account that created the post being voted on.
       * @param permlink Permlink of the post being voted on.
       * @param weight Percentage weight of the voting power applied to the post.
       * @param interface Name of the interface account that was used to broadcast the transaction. 
       * @param reaction An Emoji selected as a reaction to the post while voting.
       * @param json JSON Metadata of the vote.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           comment_vote(
         string voter, 
         string author, 
         string permlink, 
         int16_t weight,
         string interface,
         string reaction,
         string json,
         bool broadcast );


      /**
       * Views a post, which increases the post's content reward earnings.
       *
       * @param viewer Name of the viewing account.
       * @param author Name of the account that created the post being viewed.
       * @param permlink Permlink of the post being viewed.
       * @param interface Name of the interface account that was used to broadcast the transaction and view the post.
       * @param supernode Name of the supernode account that served the IPFS file data in the post.
       * @param json JSON Metadata of the view.
       * @param viewed True if viewing the post, false if removing view object.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           comment_view(
         string viewer,
         string author,
         string permlink,
         string interface,
         string supernode,
         string json,
         bool viewed,
         bool broadcast );


      /**
       * Shares a post to the account's feed.
       *
       * @param sharer Name of the viewing account.
       * @param author Name of the account that created the post being shared.
       * @param permlink Permlink of the post being shared.
       * @param reach Audience reach selection for share.
       * @param interface Name of the interface account that was used to broadcast the transaction and share the post.
       * @param communities Optionally share the post with a new community.
       * @param tags Optionally share the post with a new tag.
       * @param json JSON Metadata of the share.
       * @param shared True if sharing the post, false if removing share.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           comment_share(
         string sharer,
         string author,
         string permlink,
         string reach,
         string interface,
         vector< string > communities,
         vector< string > tags,
         string json,
         bool shared,
         bool broadcast );


      /**
       * Applies a set of tags to a post for filtering from interfaces.
       *
       * @param moderator Account creating the tag: can be a governance address or a community moderator. 
       * @param author Author of the post being tagged.
       * @param permlink Permlink of the post being tagged.
       * @param tags Set of tags to apply to the post for selective interface side filtering.
       * @param rating Newly proposed rating for the post.
       * @param details String explaining the reason for the tag to the author.
       * @param json JSON Metadata of the moderation tag.
       * @param interface Interface account used for the transaction.
       * @param filter True if the post should be filtered from the community and governance account subscribers.
       * @param removal_requested True if the moderator formally requests that the post be removed by the author.
       * @param beneficiaries_requested Beneficiary routes that are requested for revenue sharing with a claimant.
       * @param applied True if applying the tag, false if removing the tag.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           comment_moderation(
         string moderator,
         string author,
         string permlink,
         vector< string > tags,
         uint16_t rating,
         string details,
         string json,
         string interface,
         bool filter,
         bool removal_requested,
         vector< beneficiary_route_type > beneficiaries_requested,
         bool applied,
         bool broadcast );


      /**
       * Creates a private encrypted message between two accounts.
       *
       * @param sender The account sending the message.
       * @param recipient The receiving account of the message.
       * @param community The name of the community that the message should be sent to.
       * @param public_key The public key used to encrypt the post, holders of the private key may decrypt.
       * @param message Encrypted ciphertext of the message being sent.
       * @param ipfs Encrypted Private IPFS file hash: Voice message audio, images, videos, files.
       * @param json Encrypted Message metadata.
       * @param uuid uuidv4 uniquely identifying the message for local storage.
       * @param interface Interface account used for the transaction.
       * @param parent_sender The account sending the message that this message replies to.
       * @param parent_uuid uuidv4 of the message that this message replies to.
       * @param expiration Time that the message expires and is automatically deleted from API access.
       * @param forward True when the parent message is being forwarded into the conversation.
       * @param active True to send or edit message, false to delete message.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           message(
         string sender,
         string recipient,
         string community,
         string public_key,
         string message,
         string ipfs,
         string json,
         string uuid,
         string interface,
         string parent_sender,
         string parent_uuid,
         time_point expiration,
         bool forward,
         bool active,
         bool broadcast );


      /**
       * Lists contain a curated group of accounts, comments, communities and other objects.
       *
       * @param creator Name of the account that created the list.
       * @param list_id uuidv4 referring to the list.
       * @param name Name of the list, unique for each account.
       * @param details Public details description of the list.
       * @param json Public JSON metadata of the list.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param accounts Account IDs within the list.
       * @param comments Comment IDs within the list.
       * @param communities Community IDs within the list.
       * @param assets Asset IDs within the list.
       * @param products Product IDs within the list.
       * @param auctions Auction IDs within the list.
       * @param nodes Graph node IDs within the list.
       * @param edges Graph edge IDs within the list.
       * @param node_types Graph node property IDs within the list.
       * @param edge_types Graph edge property IDs within the list.
       * @param active True when the list is active, false to remove list.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           list(
         string creator,
         string list_id,
         string name,
         string details,
         string json,
         string interface,
         flat_set< int64_t > accounts,
         flat_set< int64_t > comments,
         flat_set< int64_t > communities,
         flat_set< int64_t > assets,
         flat_set< int64_t > products,
         flat_set< int64_t > auctions,
         flat_set< int64_t > nodes,
         flat_set< int64_t > edges,
         flat_set< int64_t > node_types,
         flat_set< int64_t > edge_types,
         bool active,
         bool broadcast );


      /**
       * Polls enable accounts to vote on a series of options.
       *
       * @param creator Name of the account that created the poll.
       * @param poll_id uuidv4 referring to the poll.
       * @param community Community that the poll is shown within. Null for no community.
       * @param public_key Public key for encrypting details and poll options.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param details Text describing the question being asked.
       * @param json JSON metadata of the poll.
       * @param poll_option_0 Poll option zero, vote 0 to select.
       * @param poll_option_1 Poll option one, vote 1 to select.
       * @param poll_option_2 Poll option two, vote 2 to select.
       * @param poll_option_3 Poll option three, vote 3 to select.
       * @param poll_option_4 Poll option four, vote 4 to select.
       * @param poll_option_5 Poll option five, vote 5 to select.
       * @param poll_option_6 Poll option six, vote 6 to select.
       * @param poll_option_7 Poll option seven, vote 7 to select.
       * @param poll_option_8 Poll option eight, vote 8 to select.
       * @param poll_option_9 Poll option nine, vote 9 to select.
       * @param completion_time Time the poll voting completes.
       * @param active True when the poll is active, false to remove the poll and all poll votes.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           poll(
         string creator,
         string poll_id,
         string community,
         string public_key,
         string interface,
         string details,
         string json,
         string poll_option_0,
         string poll_option_1,
         string poll_option_2,
         string poll_option_3,
         string poll_option_4,
         string poll_option_5,
         string poll_option_6,
         string poll_option_7,
         string poll_option_8,
         string poll_option_9,
         time_point completion_time,
         bool active,
         bool broadcast );


      /**
       * Poll Vote for a specified poll option.
       *
       * @param voter Name of the account that created the vote.
       * @param creator Name of the account that created the poll.
       * @param poll_id uuidv4 referring to the poll.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param poll_option Poll option chosen.
       * @param active True when the poll vote is active, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           poll_vote(
         string voter,
         string creator,
         string poll_id,
         string interface,
         uint16_t poll_option,
         bool active,
         bool broadcast );


      /**
       * Premium purchase requests that a Premium Post to be made available for decryption to a purchaser.
       *
       * @param account Name of the account purchasing the premium content.
       * @param author Name of the author of the premium post.
       * @param permlink Permlink of the premium post.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param purchased True to purchase the premium content, false to cancel existing undelivered purchase. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           premium_purchase(
         string account,
         string author,
         string permlink,
         string interface,
         bool purchased,
         bool broadcast );


      /**
       * Premium Release enables a Premium Post to be decrypted by a purchaser.
       *
       * @param provider Name of the account releasing the premium content.
       * @param account Name of the account purchasing the premium content.
       * @param author Name of the author of the premium post.
       * @param permlink Permlink of the premium post.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param encrypted_key  The private decryption key of the post, encrypted with the public secure key of the purchasing account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           premium_release(
         string provider,
         string account,
         string author,
         string permlink,
         string interface,
         string encrypted_key,
         bool broadcast );



      //================================//
      // === Community Transactions === //
      //================================//



      /**
       * Creates a new community for collecting posts about a specific topic.
       *
       * @param founder The account that created the community, able to add and remove administrators.
       * @param name Name of the community.
       * @param display_name The full name of the community (non-consensus), encrypted with the member key if private community.
       * @param details Details of the community, describing what it is for.
       * @param url External reference URL.
       * @param profile_image IPFS Reference of the Icon image of the community, encrypted with the member key if private community.
       * @param cover_image IPFS Reference of the Cover image of the community, encrypted with the member key if private community.
       * @param json Public plaintext json information about the community, its topic and rules.
       * @param json_private Private ciphertext json information about the community.
       * @param tags Set of tags of the topics within the community to enable discovery.
       * @param private_community True when the community is private, and all posts, events, directives, polls must be encrypted.
       * @param channel True when all posts must be channel posts from admins.
       * @param author_permission Determines which accounts can create root posts.
       * @param reply_permission Determines which accounts can create replies to root posts.
       * @param vote_permission Determines which accounts can create comment votes on posts and comments.
       * @param view_permission Determines which accounts can create comment views on posts and comments.
       * @param share_permission Determines which accounts can create comment shares on posts and comments.
       * @param message_permission Determines which accounts can create direct messages in the community.
       * @param poll_permission Determines which accounts can create polls in the community.
       * @param event_permission Determines which accounts can create events in the community.
       * @param directive_permission Determines which accounts can create directives and directive votes in the community.
       * @param add_permission Determines which accounts can add new members, and accept member requests.
       * @param request_permission Determines which accounts can request to join the community.
       * @param remove_permission Determines which accounts can remove and blacklist.
       * @param community_member_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.
       * @param community_moderator_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.
       * @param community_admin_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.
       * @param community_secure_key Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.
       * @param community_standard_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.
       * @param community_mid_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.
       * @param community_top_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param reward_currency The Currency asset used for content rewards in the community.
       * @param standard_membership_price Price paid per month by community standard members.
       * @param mid_membership_price Price paid per month by all mid level community members.
       * @param top_membership_price Price paid per month by all top level community members.
       * @param verifiers Accounts that are considered ground truth sources of verification authority, wth degree 0.
       * @param min_verification_count Minimum number of incoming verification transaction to be considered verified by this community.
       * @param max_verification_distance Maximum number of degrees of seperation from a verfier to be considered verified by this community.
       * @param max_rating Highest severity rating that posts in the community can have.
       * @param flags The currently active flags on the community for content settings.
       * @param permissions The flag permissions that can be activated on the community for content settings.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_create(
         string founder,
         string name,
         string display_name,
         string details,
         string url,
         string profile_image,
         string cover_image,
         string json,
         string json_private,
         vector< string > tags,
         bool private_community,
         bool channel,
         string author_permission,
         string reply_permission,
         string vote_permission,
         string view_permission,
         string share_permission,
         string message_permission,
         string poll_permission,
         string event_permission,
         string directive_permission,
         string add_permission,
         string request_permission,
         string remove_permission,
         string community_member_key,
         string community_moderator_key,
         string community_admin_key,
         string community_secure_key,
         string community_standard_premium_key,
         string community_mid_premium_key,
         string community_top_premium_key,
         string interface,
         string reward_currency,
         asset standard_membership_price,
         asset mid_membership_price,
         asset top_membership_price,
         vector< string > verifiers,
         uint64_t min_verification_count,
         uint64_t max_verification_distance,
         uint16_t max_rating,
         uint32_t flags,
         uint32_t permissions,
         bool broadcast );


      /**
       * Updates the details of an existing community.
       *
       * @param account Account updating the community. Administrator of the community.
       * @param community Name of the community. Should be randomized string if private community.
       * @param display_name The full name of the community (non-consensus), encrypted with the member key if private community.
       * @param details Details of the community, describing what it is for.
       * @param url External reference URL.
       * @param profile_image IPFS Reference of the Icon image of the community, encrypted with the member key if private community.
       * @param cover_image IPFS Reference of the Cover image of the community, encrypted with the member key if private community.
       * @param json Public plaintext json information about the community, its topic and rules.
       * @param json_private Private ciphertext json information about the community.
       * @param pinned_author Author of Post pinned to the top of the community's page.
       * @param pinned_permlink Permlink of Post pinned to the top of the community's page, encrypted with the member key if private community.
       * @param tags Set of tags of the topics within the community to enable discovery.
       * @param private_community True when the community is private, and all posts, events, directives, polls must be encrypted.
       * @param channel True when all posts must be channel posts from admins.
       * @param author_permission Determines which accounts can create root posts.
       * @param reply_permission Determines which accounts can create replies to root posts.
       * @param vote_permission Determines which accounts can create comment votes on posts and comments.
       * @param view_permission Determines which accounts can create comment views on posts and comments.
       * @param share_permission Determines which accounts can create comment shares on posts and comments.
       * @param message_permission Determines which accounts can create direct messages in the community.
       * @param poll_permission Determines which accounts can create polls in the community.
       * @param event_permission Determines which accounts can create events in the community.
       * @param directive_permission Determines which accounts can create directives and directive votes in the community.
       * @param add_permission Determines which accounts can add new members, and accept member requests.
       * @param request_permission Determines which accounts can request to join the community.
       * @param remove_permission Determines which accounts can remove and blacklist.
       * @param community_member_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.
       * @param community_moderator_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.
       * @param community_admin_key Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.
       * @param community_secure_key Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.
       * @param community_standard_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.
       * @param community_mid_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.
       * @param community_top_premium_key Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.
       * @param standard_membership_price Price paid per month by community standard members.
       * @param mid_membership_price Price paid per month by all mid level community members.
       * @param top_membership_price Price paid per month by all top level community members.
       * @param verifiers Accounts that are considered ground truth sources of verification authority, wth degree 0.
       * @param min_verification_count Minimum number of incoming verification transaction to be considered verified by this community.
       * @param max_verification_distance Maximum number of degrees of seperation from a verfier to be considered verified by this community.
       * @param max_rating Highest severity rating that posts in the community can have.
       * @param flags The currently active flags on the community for content settings.
       * @param permissions The flag permissions that can be activated on the community for content settings.
       * @param active True when the community is active, false to deactivate.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_update(
         string account,
         string community,
         string display_name,
         string details,
         string url,
         string profile_image,
         string cover_image,
         string json,
         string json_private,
         string pinned_author,
         string pinned_permlink,
         vector< string > tags,
         bool private_community,
         bool channel,
         string author_permission,
         string reply_permission,
         string vote_permission,
         string view_permission,
         string share_permission,
         string message_permission,
         string poll_permission,
         string event_permission,
         string directive_permission,
         string add_permission,
         string request_permission,
         string remove_permission,
         string community_member_key,
         string community_moderator_key,
         string community_admin_key,
         string community_secure_key,
         string community_standard_premium_key,
         string community_mid_premium_key,
         string community_top_premium_key,
         asset standard_membership_price,
         asset mid_membership_price,
         asset top_membership_price,
         vector< string > verifiers,
         uint64_t min_verification_count,
         uint64_t max_verification_distance,
         uint16_t max_rating,
         uint32_t flags,
         uint32_t permissions,
         bool active,
         bool broadcast );


      /**
       * Used to accept to a request and admit a new member.
       *
       * @param account Account within the community accepting the request.
       * @param member Account to accept into the community.
       * @param community Community that is being joined.
       * @param interface Name of the interface account that was used to broadcast the transaction.
       * @param member_type Membership and Key encryption access and permission level.
       * @param encrypted_community_key The Community Private Key, encrypted with the member's secure public key.
       * @param accepted Set true to invite, false to cancel invite.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_member(
         string account,
         string member,
         string community,
         string interface,
         string member_type,
         string encrypted_community_key,
         bool accepted,
         bool broadcast );


      /**
       * Requests that an account be added as a new member of a community.
       *
       * @param account Account that wants to join the community.
       * @param community Community that is being requested to join.
       * @param interface Name of the interface account that was used to broadcast the transaction.
       * @param member_type Membership and Key encryption access and permission level.
       * @param message Message attatched to the request, encrypted with the communities public key.
       * @param requested Set true to request, false to cancel request.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_member_request(
         string account,
         string community,
         string interface,
         string member_type,
         string message,
         bool requested,
         bool broadcast );


      /**
       * Votes for a moderator to increase their mod weight.
       *
       * @param account Account of a member of the community.
       * @param community Community that the moderator is being voted into.
       * @param member Account of the member being voted for.
       * @param interface Name of the interface account that was used to broadcast the transaction.
       * @param vote_rank Voting rank for the specified community moderator.
       * @param approved True when voting for the moderator, false when removing.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_member_vote(
         string account,
         string community,
         string member,
         string interface,
         uint16_t vote_rank,
         bool approved,
         bool broadcast );


      /**
       * Adds a community to an account's subscriptions.
       *
       * @param account Account that wants to subscribe to the community.
       * @param community Community to suscribe to.
       * @param interface Name of the interface account that was used to broadcast the transaction and subscribe to the community.
       * @param added True to add to lists, false to remove.
       * @param subscribed true if subscribing, false if filtering.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_subscribe(
         string account,
         string community,
         string interface,
         bool added,
         bool subscribed,
         bool broadcast );


      /**
       * Adds a specifed account to the community's blacklist.
       *
       * @param account Moderator or admin of the community.
       * @param member Account to be blacklisted from interacting with the community.
       * @param community Community that member is being blacklisted from.
       * @param blacklisted Set to true to add account to blacklist, set to false to remove from blacklist. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_blacklist(
         string account,
         string member,
         string community,
         bool blacklisted,
         bool broadcast );


      /**
       * Used to create a federation connection between two communities.
       *
       * @param account Admin within the community creating the federation.
       * @param federation_id uuidv4 for the federation, for local storage of decryption key.
       * @param community Community that the account is an admin within.
       * @param federated_community Community that the account is an admin within.
       * @param message Encrypted Message to include to the members of the requested community, encrypted with equivalent community key.
       * @param json Encrypted JSON metadata, encrypted with equivalent community key.
       * @param federation_type Type of Federation level.
       * @param encrypted_community_key The Federated Community Private Key, encrypted with the member's secure public key.
       * @param share_accounts True to share accounts across the Federation, false to only share decryption keys.
       * @param accepted True to accept request, false to reject federation.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_federation(
         string account,
         string federation_id,
         string community,
         string federated_community,
         string message,
         string json,
         string federation_type,
         string encrypted_community_key,
         bool share_accounts,
         bool accepted,
         bool broadcast );


      /**
       * Creates or updates a community event.
       *
       * @param account Account that created the event.
       * @param community Community being invited to join.
       * @param event_id UUIDv4 referring to the event within the Community. Unique on community/event_id.
       * @param public_key Public key for encrypting the event details. Null if public event.
       * @param event_name The Name of the event. Unique within each community.
       * @param location Address of the location of the event.
       * @param latitude Latitude co-ordinates of the event.
       * @param longitude Longitude co-ordinates of the event.
       * @param details Event details describing the purpose of the event.
       * @param url Link containining additional event information.
       * @param json Additional Event JSON data.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param event_price Amount paid to join the attending list as a ticket holder to the event.
       * @param event_start_time Time that the Event will begin.
       * @param event_end_time Time that the event will end.
       * @param active True if the community is active, false to suspend all interaction to cancel or end the event.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_event(
         string account,
         string community,
         string event_id,
         string public_key,
         string event_name,
         string location,
         double latitude,
         double longitude,
         string details,
         string url,
         string json,
         string interface,
         asset event_price,
         time_point event_start_time,
         time_point event_end_time,
         bool active,
         bool broadcast );


      /**
       * Denotes the status of an account attending an event.
       *
       * @param attendee Account that is attending the event.
       * @param community Community that the event is within.
       * @param event_id UUIDv4 referring to the event within the Community. Unique on community/event_id.
       * @param public_key Public key for encrypting the event details. Null if public event.
       * @param message Encrypted message to the community operating the event.
       * @param json Additional Event JSON data. Encrypted if private event.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param interested True to set interested in the event, and receive notifications about it, false for not interested.
       * @param attending True to attend the event, false to for not attending.
       * @param active True to create attendance for the event, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_event_attend(
         string attendee,
         string community,
         string event_id,
         string public_key,
         string message,
         string json,
         string interface,
         bool interested,
         bool attending,
         bool active,
         bool broadcast );


      /**
       * A Community directive that contains action instructions and deliverables for community members.
       *
       * @param account Account that created the directive.
       * @param directive_id UUIDv4 referring to the directive. Unique on account/directive_id.
       * @param parent_account Account that created the parent directive.
       * @param parent_directive_id UUIDv4 referring to the parent directive. Unique on account/directive_id.
       * @param community Community that the directive is given to.
       * @param public_key Public key for encrypting the directive details. Null if public directive.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param details Text details of the directive. Should explain the action items.
       * @param cover_image IPFS image for display of this directive in the interface. 
       * @param ipfs IPFS file reference for the directive. Images or other files can be attatched.
       * @param json Additional Directive JSON metadata.
       * @param directive_start_time Time that the Directive will begin.
       * @param directive_end_time Time that the Directive must be completed by.
       * @param completed True when the directive has been completed.
       * @param active  True while the directive is active, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_directive(
         string account,
         string directive_id,
         string parent_account,
         string parent_directive_id,
         string community,
         string public_key,
         string interface,
         string details, 
         string cover_image,
         string ipfs,
         string json,
         time_point directive_start_time,
         time_point directive_end_time,
         bool completed,
         bool active,
         bool broadcast )

      /**
       * Votes to approve or disapprove a directive made by a member of a community.
       * 
       * Used for Consensus directive selection, and for directive feedback.
       *
       * @param voter Account creating the directive vote.
       * @param account Account that created the directive.
       * @param directive_id UUIDv4 referring to the directive. Unique on account/directive_id.
       * @param public_key Public key for encrypting the directive vote details. Null if public directive.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param details Text details of the directive vote. Should contain directive feedback.
       * @param json Additional Directive JSON metadata.
       * @param approve True when the directive is approved, false when it is opposed.
       * @param active True while the directive vote is active, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_directive_vote(
         string voter,
         string account,
         string directive_id,
         string public_key,
         string interface,
         string details,
         string json,
         bool approve,
         bool active,
         bool broadcast )


      /**
       * Determines the State of an account's active 
       * directive selection, and its outgoing directives.
       *
       * @param account Account recieving and creating directives within a community.
       * @param community Community that the directive member is contained within.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param public_key Public key for encrypting the directive member details. Null if public directive member.
       * @param details Text details of the directive member. Should elaborate interests and priorities for voting selection
       * @param json Additional Directive Member JSON metadata.
       * @param command_directive_id The Current outgoing directive as community co-ordinator.
       * @param delegate_directive_id The Current outgoing directive to all hierachy subordinate members.
       * @param consensus_directive_id The Current outgoing directive for community consensus selection.
       * @param emergent_directive_id The Current outgoing emergent directive for selection by other members.
       * @param active True when the account is active for the directive distribution.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_directive_member(
         string account,
         string community,
         string interface,
         string public_key,
         string details,
         string json,
         string command_directive_id,
         string delegate_directive_id,
         string consensus_directive_id,
         string emergent_directive_id,
         bool active,
         bool broadcast )
      
      /**
       * Votes to approve or disapprove a directive made by a member of a community.
       * 
       * Used for Consensus directive selection, and for directive feedback.
       *
       * @param voter Account creating the directive member vote.
       * @param member Account being voted on.
       * @param community Community that the directive member vote is contained within.
       * @param interface Account of the interface that broadcasted the transaction.
       * @param public_key Public key for encrypting the directive member vote details. Null if public directive member vote.
       * @param details Text details of the directive member vote. Should contain directive member feedback.
       * @param json Additional Directive Member vote JSON metadata.
       * @param approve True when the directive member is approved, false when they is opposed.
       * @param active True while the directive member vote is active, false to remove.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           community_directive_member_vote(
         string voter,
         string member,
         string community,
         string public_key,
         string interface,
         string details,
         string json, 
         bool approve,
         bool active,
         bool broadcast );



      //=========================//
      // === Ad Transactions === //
      //=========================//



      /**
       * Creates a new ad creative to be used in a campaign for display in interfaces.
       *
       * @param account Account publishing the ad creative.
       * @param author Author of the objective item referenced.
       * @param objective The reference of the object being advertised, the link and CTA destination of the creative.
       * @param creative_id uuidv4 referring to the creative.
       * @param creative IPFS link to the Media to be displayed, image or video.
       * @param json JSON string of creative metadata for display.
       * @param format_type The type of formatting used for the advertisment, determines the interpretation of the creative.
       * @param active True if the creative is enabled for active display, false to deactivate.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           ad_creative(
         string account,
         string author,
         string objective,
         string creative_id,
         string creative,
         string json,
         string format_type,
         bool active,
         bool broadcast );


      /**
       * Creates a new ad campaign to enable ad bidding.
       *
       * @param account Account creating the ad campaign.
       * @param campaign_id uuidv4 referring to the campaign.
       * @param budget Total expenditure of the campaign.
       * @param begin Beginning time of the campaign. Bids cannot be created before this time.
       * @param end Ending time of the campaign. Bids cannot be created after this time.
       * @param json JSON string of creative metadata for display.
       * @param agents Set of Accounts authorized to create bids for the campaign.
       * @param interface Interface that facilitated the purchase of the advertising campaign.
       * @param active True if the campaign is enabled for bid creation, false to deactivate and reclaim the budget.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           ad_campaign(
         string account,
         string campaign_id,
         asset budget,
         time_point begin,
         time_point end,
         string json,
         vector< string > agents,
         string interface,
         bool active,
         bool broadcast );


      /**
       * Declares the availability of a supply of ad inventory.
       *
       * @param provider Account of an interface offering ad supply.
       * @param inventory_id uuidv4 referring to the inventory offering.
       * @param audience_id uuidv4 referring to audience object containing usernames of desired accounts in interface's audience.
       * @param metric Type of expense metric used.
       * @param min_price Minimum bidding price per metric.
       * @param inventory Total metrics available.
       * @param json JSON metadata for the inventory.
       * @param active True if the inventory is enabled for display, false to deactivate.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           ad_inventory(
         string provider,
         string inventory_id,
         string audience_id,
         string metric,
         asset min_price,
         uint32_t inventory,
         string json,
         bool active,
         bool broadcast );


      /**
       * Contains a set of accounts that are valid for advertising display.
       *
       * @param account Account creating the audience set.
       * @param audience_id uuidv4 referring to the audience for inclusion in inventory and campaigns.
       * @param json JSON metadata for the audience.
       * @param audience List of usernames of viewing accounts.
       * @param active True if the audience is enabled for reference, false to deactivate.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           ad_audience(
         string account,
         string audience_id,
         string json,
         vector< string > audience,
         bool active,
         bool broadcast );


      /**
       * Creates a new advertising bid offer.
       *
       * @param bidder Account that created the ad budget, or an agent of the campaign.
       * @param bid_id Bid uuidv4 for referring to the bid.
       * @param account Account that created the campaign that the bid is directed towards.
       * @param campaign_id Ad campaign uuidv4 to utilise for the bid.
       * @param author Account that was the author of the creative.
       * @param creative_id uuidv4 referring to the creative item to bid on.
       * @param provider Account of an interface offering ad supply.
       * @param inventory_id Inventory uuidv4 offering to bid on.
       * @param bid_price Price offered per metric.
       * @param requested Maximum total metrics requested.
       * @param included_audiences List of desired audiences for display acceptance, accounts must be in inventory audience.
       * @param excluded_audiences List of audiences to remove all members from the combined bid audience.
       * @param audience_id Audience uuidv4 for combined audience.
       * @param json JSON metadata for the inventory.
       * @param expiration Time the the bid is valid until, bid is cancelled after this time if not filled. 
       * @param active True if the bid is open for delivery, false to cancel.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           ad_bid(
         string bidder,
         string bid_id,
         string account,
         string campaign_id,
         string author,
         string creative_id,
         string provider,
         string inventory_id,
         asset bid_price,
         uint32_t requested,
         vector< string > included_audiences,
         vector< string > excluded_audiences,
         string audience_id,
         string json,
         time_point expiration,
         bool active,
         bool broadcast );


      //============================//
      //==== Graph Transactions ====//
      //============================//



      /**
       * Creates a new node in the Network's Graph Database.
       *
       * @param account Name of the account that created the node.
       * @param node_types Types of node being created, determines the required attributes.
       * @param node_id uuidv4 identifying the node. Unique for each account.
       * @param name Name of the node.
       * @param details Describes the additional details of the node.
       * @param attributes The Attributes of the node.
       * @param attribute_values The Attribute values of the node.
       * @param json Public plaintext JSON node attribute information.
       * @param json_private Private encrypted ciphertext JSON node attribute information.
       * @param node_public_key Key used for encrypting and decrypting private node JSON data.
       * @param interface Name of the application that facilitated the creation of the node.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           graph_node(
         string account,
         vector< string > node_types,
         string node_id,
         string name,
         string details,
         vector< string > attributes,
         vector< string > attribute_values,
         string json,
         string json_private,
         string node_public_key,
         string interface,
         bool broadcast );


      /**
       * Creates a new edge in the Network's Graph Database.
       *
       * @param account Name of the account that created the edge.
       * @param edge_types Types of edge being created, determines the required attributes.
       * @param edge_id uuidv4 identifying the edge. Unique for each account.
       * @param from_node_account The account that is the creator of the Base connecting node.
       * @param from_node_id The uuidv4 of the base connecting node.
       * @param to_node_account The account that is the creator of the Node being connected to.
       * @param to_node_id The uuidv4 of the Node being connected to.
       * @param name Name of the edge.
       * @param details Describes the additional details of the edge.
       * @param attributes The Attributes of the edge.
       * @param attribute_values The Attribute values of the edge.
       * @param json Public plaintext JSON edge attribute information.
       * @param json_private Private encrypted ciphertext JSON edge attribute information.
       * @param edge_public_key Key used for encrypting and decrypting private edge JSON data.
       * @param interface Name of the application that facilitated the creation of the edge.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           graph_edge(
         string account,
         vector< string > edge_types,
         string edge_id,
         string from_node_account,
         string from_node_id,
         string to_node_account,
         string to_node_id,
         string name,
         string details,
         vector< string > attributes,
         vector< string > attribute_values,
         string json,
         string json_private,
         string edge_public_key,
         string interface,
         bool broadcast );


      /**
       * Creates a new type of node for instantiation in the Network's Graph Database.
       *
       * @param account Name of the account that created the node.
       * @param node_type Name of the type of node being specified.
       * @param graph_privacy Encryption level of the node attribute data.
       * @param edge_permission The Level of connection required to create an edge to or from this node type. 
       * @param details Describes the additional details of the node.
       * @param url The Attributes of the node.
       * @param json Public plaintext JSON node attribute information.
       * @param attributes List of attributes that each node is required to have.
       * @param interface Name of the application that facilitated the creation of the node.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           graph_node_property(
         string account,
         string node_type,
         string graph_privacy,
         string edge_permission,
         string details,
         string url,
         string json,
         vector< string > attributes,
         string interface,
         bool broadcast );


      /**
       * Creates a new type of edge for instantiation in the Network's Graph Database.
       *
       * @param account Name of the account that created the edge type.
       * @param edge_type Name of the type of edge being specified.
       * @param graph_privacy Encryption level of the edge attribute data.
       * @param from_node_types Types of node that the edge can connect from. Empty for all types. 
       * @param to_node_types Types of node that the edge can connect to. Empty for all types.
       * @param details Describes the additional details of the edge.
       * @param url Reference URL link for more details.
       * @param json JSON Metadata for the edge type.
       * @param attributes List of attributes that each edge is required to have.
       * @param interface Name of the application that facilitated the creation of the edge type.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           graph_edge_property(
         string account,
         string edge_type,
         string graph_privacy,
         vector< string > from_node_types,
         vector< string > to_node_types,
         string details,
         string url,
         string json,
         vector< string > attributes,
         string interface,
         bool broadcast );




      //===============================//
      // === Transfer Transactions === //
      //===============================//


      /**
       * Transfer funds from one account to another.
       *
       * @param from The account the funds are coming from.
       * @param to The account the funds are going to.
       * @param amount The funds being transferred.
       * @param memo The memo for the transaction, encryption on the memo is advised. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer(
         string from,
         string to,
         asset amount,
         string memo,
         bool broadcast );


      /**
       * Requests a Transfer from an account to another account.
       *
       * @param to Account requesting the transfer.
       * @param from Account that is being requested to accept the transfer.
       * @param amount The funds being transferred.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param request_id uuidv4 of the request transaction.
       * @param expiration time that the request expires.
       * @param requested True to send the request, false to cancel an existing request.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_request(
         string to,
         string from,
         asset amount,
         string memo,
         string request_id,
         time_point expiration,
         bool requested,
         bool broadcast );


      /**
       * Accepts a transfer request from an account to another account.
       *
       * @param from Account that is accepting the transfer.
       * @param to Account requesting the transfer.
       * @param request_id uuidv4 of the request transaction.
       * @param accepted True to accept the request, false to reject. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_accept(
         string from,
         string to,
         string request_id,
         bool accepted,
         bool broadcast );


      /**
       * Transfers an asset periodically from one account to another.
       *
       * @param from Sending account to transfer asset from.
       * @param to Recieving account to transfer asset to.
       * @param amount The amount of asset to transfer for each payment interval.
       * @param transfer_id uuidv4 of the transfer for reference.
       * @param begin Starting time of the first payment.
       * @param payments Number of payments to process in total.
       * @param interval Microseconds between each transfer event.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param extensible True if the payment duration should be extended in the event a payment is missed.
       * @param fill_or_kill True if the payment should be cancelled if a payment is missed.
       * @param active True if recurring payment is active, false to cancel.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_recurring(
         string from,
         string to,
         asset amount,
         string transfer_id,
         time_point begin,
         uint32_t payments,
         fc::microseconds interval,
         string memo,
         bool extensible,
         bool fill_or_kill,
         bool active,
         bool broadcast );


      /**
       * Requests a periodic transfer from an account to another account.
       *
       * @param to Account requesting the transfer.
       * @param from Account that is being requested to accept the transfer.
       * @param amount The amount of asset to transfer for each payment interval.
       * @param request_id uuidv4 of the request transaction, becomes transfer id when accepted.
       * @param begin Starting time of the first payment.
       * @param payments Number of payments to process in total.
       * @param interval Microseconds between each transfer event.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param expiration Time that the request expires.
       * @param extensible True if the payment duration should be extended in the event a payment is missed.
       * @param fill_or_kill True if the payment should be cancelled if a payment is missed.
       * @param requested True to send the request, false to cancel an existing request. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_recurring_request(
         string from,
         string to,
         asset amount,
         string request_id,
         time_point begin,
         uint32_t payments,
         fc::microseconds interval,
         string memo,
         time_point expiration,
         bool extensible,
         bool fill_or_kill,
         bool requested,
         bool broadcast );


      /**
       * Accepts a periodic transfer request from an account to another account.
       *
       * @param from Account that is accepting the recurring transfer.
       * @param to Account requesting the recurring transfer.
       * @param request_id uuidv4 of the request transaction.
       * @param accepted True to accept the request, false to reject. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_recurring_accept(
         string from,
         string to,
         string request_id,
         bool accepted,
         bool broadcast );


      /**
       * Transfers funds from a confidential balance owner to another owner.
       *
       * @param inputs Inputs to the confidential transfer from confidential balances.
       * @param outputs Outputs of the confidential transfer to new confidential balances.
       * @param fee Fee paid for the transfer.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_confidential(
         vector< confidential_input > inputs,
         vector< confidential_output > outputs,
         asset fee,
         bool broadcast );


      /**
       * Converts public account balance to a confidential balance.
       *
       * @param from Account to transfer funds from and create new confidential balances.
       * @param amount Amount of funds to transfer.
       * @param blinding_factor Factor to blind the output values.
       * @param outputs Confidential balance outputs.
       * @param fee Fee amount paid for the transfer.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_to_confidential(
         string from,
         asset amount,
         blind_factor_type blinding_factor,
         vector< confidential_output > outputs,
         asset fee,
         bool broadcast );


      /**
       * Converts confidential balances to a public account balance.
       *
       * @param to Account to transfer the balances to.
       * @param amount Amount of funds to transfer from confidential balances.
       * @param blinding_factor Factor to blind the input values.
       * @param inputs Confidential balance inputs.
       * @param fee Fee amount paid for the transfer.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_from_confidential(
         string to,
         asset amount,
         blind_factor_type blinding_factor,
         vector< confidential_input > inputs,
         asset fee,
         bool broadcast );



      //==============================//
      // === Balance Transactions === //
      //==============================//


      /**
       * Claims an account's reward balance into it's liquid balance from newly issued assets.
       *
       * @param account Account claiming its reward balance from the network.
       * @param reward Amount of Reward balance to claim.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            claim_reward_balance(
         string account,
         asset reward,
         bool broadcast );


      /**
       * Stakes a liquid balance of an account into it's staked balance.
       * 
       * @param from Account staking the asset.
       * @param to Account to stake the asset to, Same as from if null.
       * @param amount Amount of Funds to transfer to staked balance from liquid balance.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            stake_asset(
         string from,
         string to,
         asset amount,
         bool broadcast );


      /**
       * Divests an amount of the staked balance of an account to it's liquid balance.
       *
       * @param from Account unstaking the asset.
       * @param to Account to unstake the asset to, Same as from if null.
       * @param amount Amount of Funds to transfer from staked balance to liquid balance.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            unstake_asset(
         string from,
         string to,
         asset amount,
         bool broadcast );


      /**
       * Set up an asset withdraw route.
       * 
       * @param from The account the assets are withdrawn from.
       * @param to The account receiving either assets or new stake.
       * @param percent The percent of the withdraw to go to the 'to' account.
       * @param auto_stake True if the stake should automatically be staked on the to account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            unstake_asset_route(
         string from, 
         string to, 
         uint16_t percent, 
         bool auto_stake, 
         bool broadcast );

      /**
       * Transfer liquid funds balance into savings for security.
       * 
       * @param from The account the assets are transferred from.
       * @param to The account that is recieving the savings balance, same as from if null.
       * @param amount Funds to be transferred from liquid to savings balance.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_to_savings(
         string from,
         string to,
         asset amount,
         string memo,
         bool broadcast );

      /**
       * Withdraws a specified balance from savings after a time duration.
       * 
       * @param from Account to transfer savings balance from.
       * @param to Account to receive the savings withdrawal.
       * @param amount Amount of asset to transfer from savings.
       * @param request_id uuidv4 referring to the transfer.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param transferred True if the transfer is accepted, false to cancel transfer.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_from_savings(
         string from,
         string to,
         asset amount,
         string request_id,
         string memo,
         bool transferred,
         bool broadcast );


      /**
       * Delegate a staked asset balance from one account to the other.
       *
       * @param delegator The account delegating the asset.
       * @param delegatee The account receiving the asset.
       * @param amount The amount of the asset delegated.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            delegate_asset(
         string delegator, 
         string delegatee, 
         asset amount, 
         bool broadcast );



      //==================================//
      // === Marketplace Transactions === //
      //==================================//


      /**
       * Creates or updates a product item for marketplace purchasing with escrow transfers.
       *
       * @param account The Seller of the product.
       * @param product_id uuidv4 referring to the product.
       * @param name The descriptive name of the product.
       * @param url Reference URL of the product or seller.
       * @param json JSON metadata attributes of the product.
       * @param product_variants The collection of product variants. Each map must have a key for each variant.
       * @param product_details The Description details of each variant of the product.
       * @param product_image IPFS references to images of each product variant.
       * @param product_prices The price (or min auction price) for each variant of the product.
       * @param wholesale_discount Discount percentages that are applied when quantity is above a given size.
       * @param stock_available The available stock of each variant of the product.
       * @param delivery_variants The types of product delivery available to purchasers.
       * @param delivery_details The details of product delivery variants.
       * @param delivery_prices The price for each variant of delivery.
       * @param active True when the product is active and able to be sold, false when discontinued.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            product_sale(
         string account,
         string product_id,
         string name,
         string url,
         string json,
         vector< string > product_variants,
         vector< string > product_details,
         string product_image,
         vector< asset > product_prices,
         flat_map< uint32_t, uint16_t > wholesale_discount,
         vector< uint32_t > stock_available,
         vector< string > delivery_variants,
         vector< string > delivery_details,
         vector< asset > delivery_prices,
         bool active,
         bool broadcast );


      /**
       * Requests a purchase of a specifed quantity of a product.
       *
       * @param buyer The Buyer of the product.
       * @param order_id uuidv4 referring to the purchase order.
       * @param seller The Seller of the product.
       * @param product_id uuidv4 refrring to the product.
       * @param order_variants Variants of product ordered in the purchase.
       * @param order_size The number of each product variant ordered.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param json Additional JSON object attribute details.
       * @param shipping_address The shipping address requested, encrypted with the secure key of the seller.
       * @param delivery_variant The type of product delivery selected.
       * @param delivery_details The Description details of the delivery.
       * @param acceptance_time Time that the escrow proposal must be approved before.
       * @param escrow_expiration Time after which balance can be claimed by FROM or TO freely.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            product_purchase(
         string buyer,
         string order_id,
         string seller,
         string product_id,
         vector< string > order_variants,
         vector< uint32_t > order_size,
         string memo,
         string json,
         string shipping_address,
         string delivery_variant,
         string delivery_details,
         time_point acceptance_time,
         time_point escrow_expiration,
         bool broadcast );


      /**
       * Creates or updates a product auction sale. 
       *
       * @param account The Seller of the auction product.
       * @param auction_id uuidv4 referring to the auction product.
       * @param auction_type The Auction price selection mechanism.
       * @param name The descriptive name of the product.
       * @param url Reference URL of the product or seller.
       * @param json JSON metadata attributes of the product.
       * @param product_details The Description details of each variant of the product.
       * @param product_image IPFS references to images of each product variant.
       * @param reserve_bid The min auction bid, or minimum price of a reverse auction at final bid time.
       * @param maximum_bid The max auction bid. Auction will immediately conclude if this price is bidded. Starting price of reverse auction.
       * @param delivery_variants The types of product delivery available to purchasers.
       * @param delivery_details The details of product delivery variants.
       * @param delivery_prices The price for each variant of delivery.
       * @param final_bid_time No more bids will be accepted after this time. Concealed bids must be revealed before completion time.
       * @param completion_time Time that the auction will select the winning bidder, or end if no bids.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            product_auction_sale(
         string account,
         string auction_id,
         string auction_type,
         string name,
         string url,
         string json,
         string product_details,
         string product_image,
         asset reserve_bid,
         asset maximum_bid,
         vector< string > delivery_variants,
         vector< string > delivery_details,
         vector< asset > delivery_prices,
         time_point final_bid_time,
         time_point completion_time,
         bool broadcast );


      /**
       * Requests a purchase of a specifed quantity of a product.
       *
       * @param buyer The Buyer of the product.
       * @param bid_id uuidv4 referring to the auction bid.
       * @param seller The Seller of the product.
       * @param auction_id uuidv4 referring to the product.
       * @param bid_price_commitment Concealed value of the bid price amount.
       * @param blinding_factor Factor to blind the bid price.
       * @param public_bid_amount Set to 0 initially for concealed bid, revealed to match commitment. Revealed in initial bid if open.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param json Additional JSON object attribute details.
       * @param shipping_address The shipping address requested, encrypted with the secure key of the seller.
       * @param delivery_variant The type of product delivery selected.
       * @param delivery_details The Description details of the delivery.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            product_auction_bid(
         string buyer,
         string bid_id,
         string seller,
         string auction_id,
         commitment_type bid_price_commitment,
         blind_factor_type blinding_factor,
         share_type public_bid_amount,
         string memo,
         string json,
         string shipping_address,
         string delivery_variant,
         string delivery_details,
         bool broadcast );


      /**
       * Creates a proposed escrow transfer between two accounts.
       *
       * @param account Account creating the transaction to initate the escrow.
       * @param from Account sending funds for a purchase.
       * @param to Account receiving funds from a purchase.
       * @param escrow_id uuidv4 referring to the escrow transaction.
       * @param amount Amount of the asset to be transferred upon success.
       * @param acceptance_time Time that the escrow proposal must be approved before.
       * @param escrow_expiration Time after which balance can be claimed by FROM or TO freely.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param json Additional JSON object attribute details.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            escrow_transfer(
         string account,
         string from,
         string to,
         string escrow_id,
         asset amount,
         time_point acceptance_time,
         time_point escrow_expiration,
         string memo,
         string json,
         bool broadcast );

      /**
       * Approves an escrow transfer, causing it to be locked in.
       *
       * @param account Account creating the transaction to approve the escrow.
       * @param mediator Nominated mediator to join the escrow for potential dispute resolution.
       * @param escrow_from The account sending funds into the escrow.
       * @param escrow_id uuidv4 referring to the escrow being approved.
       * @param approved Set true to approve escrow, false to reject the escrow. All accounts must approve before activation.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            escrow_approve(
         string account,
         string mediator,
         string escrow_from,
         string escrow_id,
         bool approved,
         bool broadcast
      );

      /**
       * Raise a dispute on the escrow transfer before it expires
       *
       * @param account Account creating the transaction to dispute the escrow and raise it for resolution
       * @param escrow_from The account sending funds into the escrow.
       * @param escrow_id  uuidv4 referring to the escrow being disputed.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            escrow_dispute(
         string account,
         string escrow_from,
         string escrow_id,
         bool broadcast );

      /**
       * Raise a dispute on the escrow transfer before it expires
       *
       * @param account The account creating the operation to release the funds.
       * @param escrow_from The escrow FROM account.
       * @param escrow_id uuidv4 referring to the escrow.
       * @param release_percent Percentage of escrow to release to the TO Account / remaining will be refunded to FROM account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            escrow_release(
         string account,
         string escrow_from,
         string escrow_id,
         uint16_t release_percent,
         bool broadcast );



      //==============================//
      // === Trading Transactions === //
      //==============================//


      /**
       * Creates a new limit order for exchanging assets at a specifed price.
       *
       * @param owner Account that owns the asset being sold.
       * @param order_id uuidv4 of the order for reference.
       * @param amount_to_sell Asset being sold on exchange.
       * @param exchange_rate Minimum price to sell asset.
       * @param interface Name of the interface that broadcasted the transaction.
       * @param expiration Time that the order expires.
       * @param opened True to open new order, false to cancel existing order. 
       * @param fill_or_kill True if the order should be removed if it does not immediately fill on the orderbook.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             limit_order(
         string owner,
         string order_id,
         asset amount_to_sell,
         price exchange_rate,
         string interface,
         time_point expiration,
         bool opened,
         bool fill_or_kill,
         bool broadcast );


      /**
       * Creates a new margin order for trading assets.
       *
       * @param owner Account that is the owner of the new margin position.
       * @param order_id uuidv4 of the order for reference.
       * @param exchange_rate The asset pair price to sell the borrowed amount at on the exchange.
       * @param collateral Collateral asset used to back the loan value. Returned to credit collateral object when position is closed. 
       * @param amount_to_borrow Amount of asset borrowed to purchase the position asset. Repaid when the margin order is closed.
       * @param stop_loss_price Price at which the position will be closed if it falls into a net loss.
       * @param take_profit_price Price at which the order will be closed if it rises into a net profit.
       * @param limit_stop_loss_price Price at which the position will be closed if it falls into a net loss.
       * @param limit_take_profit_price Price at which the order will be closed if it rises into a net profit.
       * @param interface Name of the interface that broadcasted the transaction.
       * @param expiration Time that the order expires.
       * @param opened Set true to open the order, false to close existing order.
       * @param fill_or_kill Set true to cancel the order if it does not fill against the orderbook immediately.
       * @param force_close Set true when closing to force liquidate the order against the liquidity pool at available price.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             margin_order(
         string owner,
         string order_id,
         price exchange_rate,
         asset collateral,
         asset amount_to_borrow,
         price stop_loss_price,
         price take_profit_price,
         price limit_stop_loss_price,
         price limit_take_profit_price,
         string interface,
         time_point expiration,
         bool opened,
         bool fill_or_kill,
         bool force_close,
         bool broadcast );


      /**
       * Creates a new auction order that sells at the daily auction clearing price.
       *
       * @param owner Owner of the Auction order.
       * @param order_id uuidv4 of the order for reference.
       * @param amount_to_sell Amount of asset to sell at auction clearing price.
       * @param limit_close_price The asset pair price to sell the amount at the auction clearing price. Amount to sell is base.
       * @param interface Name of the interface that created the transaction.
       * @param expiration Time that the order expires.
       * @param opened True to open new order, false to cancel existing order.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             auction_order(
         string owner,
         string order_id,
         asset amount_to_sell,
         price limit_close_price,
         string interface,
         time_point expiration,
         bool opened,
         bool broadcast );


      /**
       * Creates a new collateralized debt position in a market issued asset.
       *
       * @param owner Owner of the debt position and collateral.
       * @param collateral Amount of collateral to add to the margin position.
       * @param debt Amount of the debt to be issued.
       * @param target_collateral_ratio Maximum CR to maintain when selling collateral on margin call.
       * @param interface Name of the interface that broadcasted the transaction.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             call_order(
         string owner,
         asset collateral,
         asset debt,
         uint16_t target_collateral_ratio,
         string interface,
         bool broadcast );


      /**
       * Creates a new auction order that sells at the daily auction clearing price.
       *
       * @param owner Owner of the Option order.
       * @param order_id uuidv4 of the order for reference.
       * @param options_issued Amount of assets to issue covered options contract assets against. Must be a whole number.
       * @param interface Name of the interface that created the transaction.
       * @param opened True to open new order, false to close existing order by repaying options assets.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             option_order(
         string owner,
         string order_id,
         asset options_issued,
         string interface,
         bool opened,
         bool broadcast );


      //===========================//
      // === Pool Transactions === //
      //===========================//



      /**
       * Creates a new liquidity pool between two assets.
       *
       * @param account Creator of the new liquidity pool.
       * @param first_amount Initial balance of one asset.
       * @param second_amount Initial balance of second asset, initial price is the ratio of these two amounts.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             liquidity_pool_create(
         string account,
         asset first_amount,
         asset second_amount,
         bool broadcast );


      /**
       * Exchanges an asset directly from liquidity pools.
       *
       * @param account Account executing the exchange with the pool.
       * @param amount Amount of asset to be exchanged.
       * @param receive_asset The asset to receive from the liquidity pool.
       * @param interface Name of the interface account broadcasting the transaction.
       * @param limit_price The price of acquistion at which to cap the exchange to.
       * @param acquire Set true to acquire the specified amount, false to exchange in.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             liquidity_pool_exchange(
         string account,
         asset amount,
         string receive_asset,
         string interface,
         price limit_price,
         bool acquire,
         bool broadcast );


      /**
       * Adds capital to a liquidity pool.
       *
       * @param account Account funding the liquidity pool to receive the liquidity pool asset.
       * @param amount Amount of an asset to contribute to the liquidity pool.
       * @param pair_asset Pair asset to the liquidity pool to receive liquidity pool assets of. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             liquidity_pool_fund(
         string account,
         asset amount,
         string pair_asset,
         bool broadcast );


      /**
       * Removes capital from a liquidity pool.
       *
       * @param account Account withdrawing liquidity pool assets from the pool.
       * @param amount Amount of the liquidity pool asset to redeem for underlying deposited assets. 
       * @param receive_asset The asset to receive from the liquidity pool.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             liquidity_pool_withdraw(
         string account,
         asset amount,
         string receive_asset,
         bool broadcast );


      /**
       * Adds an asset to an account's credit collateral position of that asset.
       *
       * @param account Account locking an asset as collateral. 
       * @param amount Amount of collateral balance to lock, 0 to unlock existing collateral.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             credit_pool_collateral(
         string account,
         asset amount,
         bool broadcast );


      /**
       * Borrows an asset from the credit pool of the asset.
       *
       * @param account Account borrowing funds from the pool, must have sufficient collateral.
       * @param amount Amount of an asset to borrow. Limit of 75% of collateral value. Set to 0 to repay loan.
       * @param collateral Amount of an asset to use as collateral for the loan. Set to 0 to reclaim collateral to collateral balance.
       * @param loan_id uuidv4 unique identifier for the loan.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             credit_pool_borrow(
         string account,
         asset amount,
         asset collateral,
         string loan_id,
         bool broadcast );


      /**
       * Lends an asset to a credit pool.
       *
       * @param account Account lending an asset to the credit pool.
       * @param amount Amount of asset being lent.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             credit_pool_lend(
         string account,
         asset amount,
         bool broadcast );


      /**
       * Withdraws an asset from the specified credit pool.
       *
       * @param account Account withdrawing its lent asset from the credit pool by redeeming credit-assets. 
       * @param amount Amount of interest bearing credit assets being redeemed for their underlying assets. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             credit_pool_withdraw(
         string account,
         asset amount,
         bool broadcast );

      
      /**
       * Creates a new Option pool between two assets.
       *
       * @param account Creator of the new option pool.
       * @param first_asset First asset in the option trading pair.
       * @param second_asset Second asset in the option trading pair.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             option_pool_create(
         string account,
         string first_asset,
         string second_asset,
         bool broadcast );


      /**
       * Creates a new prediction pool for a prediction market.
       *
       * @param account Creator of the new prediction pool.
       * @param prediction_symbol Ticker symbol of the prediction pool primary asset.
       * @param collateral_symbol Symbol of the collateral asset backing the prediction market.
       * @param outcome_assets Symbols for each outcome of the prediction market.
       * @param outcome_details Description of each outcome and the resolution conditions for each asset.
       * @param display_symbol Non-consensus display name for interface reference.
       * @param json JSON Metadata of the prediction market.
       * @param url Reference URL of the prediction market.
       * @param details Description of the market, how it will be resolved using known public data sources.
       * @param outcome_time Time after which the outcome of the prediction market will become known and resolutions open.
       * @param prediction_bond Initial deposit placed by the issuer on the market, which requires them to vote in the resolution process.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             prediction_pool_create(
         string account,
         string prediction_symbol,
         string collateral_symbol,
         vector< string > outcome_assets,
         string outcome_details,
         string display_symbol,
         string json,
         string url,
         string details,
         time_point outcome_time,
         asset prediction_bond,
         bool broadcast );


      /**
       * Adds or removes capital collateral funds from a prediction pool.
       *
       * @param account Account executing the exchange with the pool.
       * @param amount Amount of collateral asset to be exchanged.
       * @param prediction_asset Base Asset to the prediction pool to exchange with.
       * @param exchange_base True to exchange base asset, false to exchange one of all prediction assets.
       * @param withdraw True to Withdraw collateral, false to add funds.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             prediction_pool_exchange(
         string account,
         asset amount,
         string prediction_asset,
         bool exchange_base,
         bool withdraw,
         bool broadcast );


      /**
       * Votes for a specified prediction market outcome resolution.
       *
       * @param account Account executing the exchange with the pool.
       * @param amount Amount of prediction asset to vote with.
       * @param resolution_outcome Base Asset to the prediction pool to exchange with.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             prediction_pool_resolve(
         string account,
         asset amount,
         string resolution_outcome,
         bool broadcast );



      //============================//
      // === Asset Transactions === //
      //============================//



      /**
       * Creates a new asset object of the asset type provided.
       *
       * @param issuer Name of the issuing account, can create units and administrate the asset.
       * @param symbol The ticker symbol of this asset.
       * @param asset_type The type of the asset. Determines asset characteristics and features.
       * @param coin_liquidity Amount of COIN asset to inject into the Coin liquidity pool.
       * @param usd_liquidity Amount of USD asset to inject into the USD liquidity pool.
       * @param credit_liquidity Amount of the new asset to issue and inject into the credit pool.
       * @param options Series of options parameters that determine asset charactertistics.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_create(
         string issuer,
         string symbol,
         string asset_type,
         asset coin_liquidity,
         asset usd_liquidity,
         asset credit_liquidity,
         asset_options options,
         bool broadcast );


      /**
       * Updates an Asset to use a new set of options.
       *
       * @param issuer Name of the issuing account, can create units and administrate the asset.
       * @param asset_to_update The ticker symbol of this asset.
       * @param new_options Series of options paramters that apply to all asset types.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_update(
         string issuer,
         string asset_to_update,
         asset_options new_options,
         bool broadcast );


      /**
       * Issues an amount of an asset to a specified account.
       *
       * @param issuer The issuer of the asset.
       * @param asset_to_issue Amount of asset being issued to the account.
       * @param issue_to_account Account receiving the newly issued asset.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_issue(
         string issuer,
         asset asset_to_issue,
         string issue_to_account,
         string memo,
         bool broadcast );


      /**
       * Takes a specified amount of an asset out of circulation.
       *
       * @param payer Account that is reserving the asset back to the unissued supply.
       * @param amount_to_reserve Amount of the asset being reserved.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_reserve(
         string payer,
         asset amount_to_reserve,
         bool broadcast );


      /**
       * Updates the issuer account of an asset.
       *
       * @param issuer The current issuer of the asset.
       * @param asset_to_update The asset symbol being updated.
       * @param new_issuer Name of the account specified to become the new issuer of the asset.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_update_issuer(
         string issuer,
         string asset_to_update,
         string new_issuer,
         bool broadcast );


      /**
       * Creates a new asset distribution.
       *
       * @param issuer The account which created the asset.
       * @param distribution_asset Asset that is generated by the distribution.
       * @param fund_asset Asset being accepted for distribution assets.
       * @param details Description of the distribution process.
       * @param url Reference URL of the distribution process.
       * @param json JSON Metadata of the distribution process.
       * @param distribution_rounds Number of distribution rounds, total distribution amount is divided between all rounds.
       * @param distribution_interval_days Duration of each distribution round, in days.
       * @param max_intervals_missed Number of Rounds that can be missed before the distribution is closed early.
       * @param min_input_fund_units Minimum funds required for each round of the distribution process.
       * @param max_input_fund_units Maximum funds to be accepted before closing each distribution round.
       * @param input_fund_unit The integer unit ratio for distribution of incoming funds.
       * @param output_distribution_unit The integer unit ratio for distribution of released funds.
       * @param min_unit_ratio The lowest value of unit ratio between input and output units.
       * @param max_unit_ratio The highest possible initial value of unit ratio between input and output units. 
       * @param min_input_balance_units Minimum fund units that each sender can contribute in an individual balance.
       * @param max_input_balance_units Maximum fund units that each sender can contribute in an individual balance.
       * @param begin_time Time to begin the first distribution round.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_distribution(
         string issuer,
         string distribution_asset,
         string fund_asset,
         string details,
         string url,
         string json,
         uint32_t distribution_rounds,
         uint32_t distribution_interval_days,
         uint32_t max_intervals_missed,
         int64_t min_input_fund_units,
         int64_t max_input_fund_units,
         vector< asset_unit > input_fund_unit,
         vector< asset_unit > output_distribution_unit,
         int64_t min_unit_ratio,
         int64_t max_unit_ratio,
         int64_t min_input_balance_units,
         int64_t max_input_balance_units,
         time_point begin_time,
         bool broadcast );


      /**
       * Funds a new asset distribution balance.
       *
       * @param sender The account which sent the amount into the distribution.
       * @param distribution_asset Distribution asset for the fund to be sent to.
       * @param amount Asset amount being sent for distribution.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_distribution_fund(
         string sender,
         string distribution_asset,
         asset amount,
         bool broadcast );


      /**
       * Uses an option asset to obtain the underlying asset at the strike price.
       *
       * @param account The account exercising the option asset.
       * @param amount Option assets being exercised by exchanging the quoted asset for the underlying. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_option_exercise(
         string account,
         asset amount,
         bool broadcast );


      /**
       * Adds funds to the redemption pool of a stimulus asset.
       *
       * @param account The account funding the stimulus asset.
       * @param stimulus_asset Asset symbol of the asset to add stimulus funds to.
       * @param amount Redemption asset being injected into the redemption pool.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_stimulus_fund(
         string account,
         string stimulus_asset,
         asset amount,
         bool broadcast );


      /**
       * Update the set of feed-producing accounts for a BitAsset.
       *
       * @param issuer The issuer of the BitAsset.
       * @param asset_to_update The BitAsset being updated.
       * @param new_feed_producers Set of accounts that can determine the price feed of the asset.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_update_feed_producers(
         string issuer,
         string asset_to_update,
         flat_set< account_name_type > new_feed_producers,
         bool broadcast );


      /**
       * Publish price feeds for BitAssets.
       *
       * @param publisher Account publishing the price feed.
       * @param symbol Asset for which the feed is published.
       * @param feed Exchange rate between stablecoin and backing asset.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_publish_feed(
         string publisher,
         string symbol,
         price_feed feed,
         bool broadcast );


      /**
       * Schedules a BitAsset balance for automatic settlement.
       *
       * @param account Account requesting the force settlement.
       * @param amount Amount of asset to force settle. Set to 0 to cancel order.
       * @param interface  Account of the interface used to broadcast the operation.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_settle(
         string account,
         asset amount,
         string interface,
         bool broadcast );


      /**
       * Globally Settles a BitAsset, collecting all remaining collateral and debt and setting a global settlement price.
       *
       * @param issuer Issuer of the asset being settled. 
       * @param asset_to_settle Symbol of the asset being settled. 
       * @param settle_price Global settlement price, must be in asset / backing asset. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_global_settle(
         string issuer,
         string asset_to_settle,
         price settle_price,
         bool broadcast );


      /**
       * Used to create a bid for outstanding debt of a globally settled market issued asset.
       *
       * @param bidder Adds additional collateral to the market issued asset.
       * @param collateral The amount of collateral to bid for the debt.
       * @param debt The amount of debt to take over.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_collateral_bid(
         string bidder,
         asset collateral,
         asset debt,
         bool broadcast );



      //=====================================//
      // === Block Producer Transactions === //
      //=====================================//

      
      /**
       * Creates or updates a producer for a specified account, enabling block production.
       *
       * @param owner The account that owns the producer.
       * @param details The producer's details for stakeholder voting reference.
       * @param url Producer's reference URL for more information.
       * @param json The producers json metadata.
       * @param latitude Latitude co-ordinates of the producer.
       * @param longitude Longitude co-ordinates of the producer.
       * @param block_signing_key The public key used to sign blocks.
       * @param props Chain properties values for selection of adjustable network parameters. 
       * @param active Set active to true to activate producer, false to deactivate
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             producer_update(
         string owner,
         string details,
         string url,
         string json,
         double latitude,
         double longitude,
         string block_signing_key,
         chain_properties props,
         bool active,
         bool broadcast );


      /**
       * Enables mining producers to publish cryptographic proofs of work.
       *
       * @param work Proof of work, containing a reference to a prior block, and a nonce resulting in a low hash value.
       * @param new_owner_key If creating a new account with a proof of work, the owner key of the new account.
       * @param props Chain properties values for selection of adjustable network parameters. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             proof_of_work(
         proof_of_work_type work,
         string new_owner_key,
         chain_properties props,
         bool broadcast );


      /**
       * Enables block producers to verify that a valid block exists at a given height.
       *
       * @param producer The name of the block producing account.
       * @param block_id The block id of the block being verifed as valid and received.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             verify_block(
         string producer,
         string block_id,
         bool broadcast );


      /**
       * Stakes COIN on the validity and acceptance of a block.
       *
       * @param producer The name of the block producing account.
       * @param block_id The block id of the block being committed as irreversible to that producer.
       * @param verifications The set of attesting transaction ids of verification transactions from currently active producers.
       * @param commitment_stake The value of staked balance that the producer stakes on this commitment. Must be at least one unit of COIN.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             commit_block(
         string producer,
         string block_id,
         flat_set< transaction_id_type > verifications,
         asset commitment_stake,
         bool broadcast );


      /**
       * Declares a violation of a block commitment.
       *
       * @param reporter The account detecting and reporting the violation.
       * @param first_trx The first transaction signed by the producer.
       * @param second_trx The second transaction that is in contravention of the first commitment transaction. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             producer_violation(
         string reporter,
         vector< char > first_trx,
         vector< char > second_trx,
         bool broadcast );



      //=============================//
      //==== Custom Transactions ====//
      //=============================//


      /**
       * Provides a generic way (using char vector) to add higher level protocols on top of network consensus.
       *
       * @param required_auths Set of account authorities required for the transaction signature.
       * @param id ID of the custom operation, determines the how to interpret the operation.
       * @param data Char vector of data contained within the operation.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             custom(
         flat_set< account_name_type > required_auths,
         uint16_t id,
         vector< char > data,
         bool broadcast );


      /**
       * Provides a generic way (using JSON String) to add higher level protocols on top of network consensus.
       *
       * @param required_auths Set of account active authorities required for the transaction signature.
       * @param required_posting_auths Set of posting authorities required for the transaction signature.
       * @param id ID of the operation, refers to the plugin used to interpret the operation. Less than 32 characters long.
       * @param json Valid UTF8 / JSON string of operation data.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             custom_json(
         flat_set< account_name_type > required_auths,
         flat_set< account_name_type > required_posting_auths,
         string id,
         string json,
         bool broadcast );

      /**
       * @}
       */

};

struct plain_keys {
   fc::sha512                      checksum;

   map<public_key_type,string>     keys;
};

} }      // node::wallet

FC_REFLECT( node::wallet::wallet_data,
         (cipher_keys)
         (ws_server)
         (ws_user)
         (ws_password)
         );

FC_REFLECT( node::wallet::seed_phrase_info, 
         (brain_priv_key)
         (wif_priv_key)
         (pub_key)
         );

FC_REFLECT( node::wallet::plain_keys,
         (checksum)
         (keys)
         );

FC_REFLECT_ENUM( node::wallet::authority_type, 
         (owner)
         (active)
         (posting)
         );

FC_REFLECT( node::wallet::encrypted_message_data, 
         (from)
         (to)
         (nonce)
         (check)
         (encrypted)
         );

FC_API( node::wallet::wallet_api,
         (help)
         (info)
         (about)
         (list_my_accounts)
         (get_my_accounts)
         (get_wallet_filename)
         (is_new)
         (is_locked)
         (lock)
         (unlock)
         (set_password)
         (gethelp)
         (load_wallet_file)
         (save_wallet_file)
         (set_wallet_filename)
         (serialize_transaction)
         (copy_wallet_file)
         (set_transaction_expiration)
         (check_memo)
         (get_encrypted_message)
         (get_decrypted_message)
         (derive_private_key)
         (get_private_key)
         (get_private_key_from_password)
         (suggest_seed_phrase)
         (normalize_seed_phrase)
         (list_keys)
         (import_key)
         (encrypt_keys)
         (get_config)
         (get_dynamic_global_properties)
         (get_median_chain_properties)
         (get_producer_schedule)
         (get_hardfork_version)
         (get_next_scheduled_hardfork)
         (get_account)
         (get_accounts)
         (get_accounts_by_followers)
         (get_concise_accounts)
         (get_full_accounts)
         (get_account_history)
         (get_account_messages)
         (get_account_balances)
         (get_confidential_balances)
         (get_keychains)
         (lookup_accounts)
         (get_account_count)
         (get_assets)
         (get_escrow)
         (get_communities)
         (get_communities_by_subscribers)
         (get_account_network_state)
         (get_active_producers)
         (get_producers_by_voting_power)
         (get_producers_by_mining_power)
         (get_development_officers_by_voting_power)
         (get_marketing_officers_by_voting_power)
         (get_advocacy_officers_by_voting_power)
         (get_supernodes_by_view_weight)
         (get_interfaces_by_users)
         (get_governances_by_members)
         (get_enterprise_by_voting_power)
         (get_open_orders)
         (get_limit_orders)
         (get_margin_orders)
         (get_auction_orders)
         (get_call_orders)
         (get_option_orders)
         (get_credit_loans)
         (get_credit_pools)
         (get_liquidity_pools)
         (get_option_pools)
         (get_market_state)
         (get_account_ads)
         (get_interface_audience_bids)
         (get_product_sale)
         (get_product_auction_sale)
         (get_account_products)
         (get_graph_query)
         (get_graph_node_properties)
         (get_graph_edge_properties)
         (get_search_query)
         (sign_transaction)
         (get_prototype_operation)
         (network_add_nodes)
         (network_get_connected_peers)
         (get_block)
         (get_ops_in_block)
         (get_transaction)
         (get_transaction_id)
         (get_comment_interactions)
         (get_account_votes)
         (get_account_views)
         (get_account_shares)
         (get_account_moderation)
         (get_account_tag_followings)
         (get_top_tags)
         (get_tags_used_by_author)
         (get_content)
         (get_content_replies)
         (get_replies_by_last_update)
         (get_discussions_by_sort_rank)
         (get_discussions_by_feed)
         (get_discussions_by_blog)
         (get_discussions_by_featured)
         (get_discussions_by_recommended)
         (get_discussions_by_comments)
         (get_discussions_by_payout)
         (get_post_discussions_by_payout)
         (get_comment_discussions_by_payout)
         (get_discussions_by_created)
         (get_discussions_by_active)
         (get_discussions_by_votes)
         (get_discussions_by_views)
         (get_discussions_by_shares)
         (get_discussions_by_children)
         (get_discussions_by_vote_power)
         (get_discussions_by_view_power)
         (get_discussions_by_share_power)
         (get_discussions_by_comment_power)
         (get_state)
         (account_create)
         (account_update)
         (account_verification)
         (account_membership)
         (account_update_list)
         (account_producer_vote)
         (account_update_proxy)
         (account_request_recovery)
         (account_recover)
         (account_reset)
         (account_reset_update)
         (account_recovery_update)
         (account_decline_voting)
         (account_connection)
         (account_follow)
         (account_tag_follow)
         (account_activity)
         (business_create)
         (business_update)
         (business_executive)
         (business_executive_vote)
         (business_director)
         (business_director_vote)
         (governance_create)
         (governance_update)
         (governance_executive)
         (governance_executive_vote)
         (governance_director)
         (governance_director_vote)
         (governance_member)
         (governance_member_request)
         (governance_resolution)
         (governance_resolution_vote)
         (network_officer_update)
         (network_officer_vote)
         (supernode_update)
         (interface_update)
         (mediator_update)
         (enterprise_update)
         (enterprise_fund)
         (enterprise_vote)
         (comment)
         (message)
         (comment_vote)
         (comment_view)
         (comment_share)
         (comment_moderation)
         (list)
         (poll)
         (poll_vote)
         (premium_purchase)
         (premium_release)
         (community_create)
         (community_update)
         (community_member)
         (community_member_request)
         (community_member_vote)
         (community_subscribe)
         (community_blacklist)
         (community_federation)
         (community_event)
         (community_event_attend)
         (community_directive)
         (community_directive_vote)
         (community_directive_member)
         (community_directive_member_vote)
         (ad_creative)
         (ad_campaign)
         (ad_inventory)
         (ad_audience)
         (ad_bid)
         (graph_node)
         (graph_edge)
         (graph_node_property)
         (graph_edge_property)
         (transfer)
         (transfer_request)
         (transfer_accept)
         (transfer_recurring)
         (transfer_recurring_request)
         (transfer_recurring_accept)
         (transfer_confidential)
         (transfer_to_confidential)
         (transfer_from_confidential)
         (claim_reward_balance)
         (stake_asset)
         (unstake_asset)
         (unstake_asset_route)
         (transfer_to_savings)
         (transfer_from_savings)
         (delegate_asset)
         (product_sale)
         (product_purchase)
         (product_auction_sale)
         (product_auction_bid)
         (escrow_transfer)
         (escrow_approve)
         (escrow_dispute)
         (escrow_release)
         (limit_order)
         (margin_order)
         (auction_order)
         (call_order)
         (option_order)
         (liquidity_pool_create)
         (liquidity_pool_exchange)
         (liquidity_pool_fund)
         (liquidity_pool_withdraw)
         (credit_pool_collateral)
         (credit_pool_borrow)
         (credit_pool_lend)
         (credit_pool_withdraw)
         (option_pool_create)
         (prediction_pool_create)
         (prediction_pool_exchange)
         (prediction_pool_resolve)
         (asset_create)
         (asset_update)
         (asset_issue)
         (asset_reserve)
         (asset_update_issuer)
         (asset_distribution)
         (asset_distribution_fund)
         (asset_option_exercise)
         (asset_stimulus_fund)
         (asset_update_feed_producers)
         (asset_publish_feed)
         (asset_settle)
         (asset_global_settle)
         (asset_collateral_bid)
         (producer_update)
         (proof_of_work)
         (verify_block)
         (commit_block)
         (producer_violation)
         (custom)
         (custom_json)
         );