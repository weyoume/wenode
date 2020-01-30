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
       * Gets the account information for all accounts for which this wallet has a private key.
       */
      vector< account_api_obj >              list_my_accounts();

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
       *  Returns the encrypted memo if memo starts with '#' otherwise returns memo.
       */
      string                                 get_encrypted_memo( string from, string to, string memo );

      /**
       * Returns the decrypted memo if possible given wallet's known private keys.
       */
      string                                 decrypt_memo( string memo );

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
       */
      string                                 get_private_key( public_key_type pubkey )const;

      /**
       *  @param role - active | owner | posting | memo
       */
      pair< public_key_type, string >        get_private_key_from_password( string account, string role, string password )const;

      /** 
       * Suggests a safe seed phrase to use for creating your account.
       * 
       * \c create_account_with_seed_phrase() requires you to specify a 'seed phrase',
       * a long passphrase that provides enough entropy to generate cryptographic
       * keys. This function will suggest a suitably random string that should
       * be easy to write down (and, with effort, memorize).
       * 
       * @returns A suggested seed_phrase.
       */
      seed_phrase_info                         suggest_seed_phrase()const;

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
       * The returned @ref dynamic_global_property_object contains
       * information that changes every block interval
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
      chain_properties                                get_chain_properties()const;


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


      /** 
       * Returns the current balances of the network reward fund pools.
       * 
       * @returns Funds contained within reward pools, pending allocation to network participants. 
       */
      reward_fund_api_obj                             get_reward_fund()const;


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
       * Returns concise information about the given account.
       *
       * @param name The name of the account to provide information about.
       * @returns Account Object information pertaining to the specified account.
       */
      account_concise_api_obj                         get_concise_account( string name ) const;


      /** 
       * Returns concise information about a list of given accounts.
       *
       * @param names The name of the accounts to provide information about.
       * @returns Account Object information pertaining to the specified accounts.
       */
      vector< account_concise_api_obj >               get_concise_accounts( vector< string > names ) const;


      /** 
       * Returns full representation of the given account, and all objects that it controls.
       *
       * @param name The name of the account to provide information about.
       * @returns Account Object information pertaining to the specified account.
       */
      extended_account                                get_full_account( string name ) const;


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
      vector< message_state >                         get_messages( vector< string > names ) const;


      /** 
       * Returns the balance states of a list of accounts.
       * 
       * Includes all balances that an account owns.
       *
       * @param names The names of the accounts to provide balance information.
       * @returns Balance state information pertaining to the specified accounts.
       */
      vector< balance_state >                         get_balances( vector< string > names ) const;


      /** 
       * Returns the key state sets of a list of accounts.
       * 
       * Includes all key objects from accounts and boards that are connected.
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
       * Use the \c lowerbound and limit parameters to page through the list.
       * To retrieve all accounts, start by setting \c lowerbound to the empty string \c "",
       * and then each iteration, pass the last account name returned 
       * as the \c lowerbound for the next \c list_accounts() call.
       *
       * @param lowerbound The name of the first account to return. If the named account does not exist, the list will start at the first account that comes after \c lowerbound .
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

      /**
       * Gets the details of an accounts history of owner keys, including all previous keys and their times in use.
       * 
       * @param account The name of the account to query.
       * @returns Previously used owner authority keys.
       */
      vector< owner_authority_history_api_obj >       get_owner_history( string account )const;

      /**
       * Gets an active account recovery request for a specified account.
       * 
       * @param account The name of the account to query.
       * @returns If an account recovery request is active, returns it.
       */
      optional< account_recovery_request_api_obj >    get_recovery_request( string account ) const;


      /**
       * Gets an accounts bandwidth information.
       * 
       * @param account The name of the account to query.
       * @returns Account bandwidth object from the specified account.
       */
      optional< account_bandwidth_api_obj >           get_account_bandwidth( string account, producer::bandwidth_type type )const;



      //===================//
      // === Asset API === //
      //===================//

      /** 
       * Returns all available information about a specified asset.
       *
       * @param asset The name of the asset to retrieve information about.
       * @returns Asset Object information pertaining to the specified asset.
       */
      extended_asset                                  get_asset( string asset )const;


      /** 
       * Returns all available information about a list of specified assets.
       *
       * @param assets The list of assets to retrieve information about.
       * @returns Asset Objects information pertaining to the specified list of assets.
       */
      vector< extended_asset >                        get_assets( vector< string > assets )const;


      /** 
       * Returns the number of registered assets.
       * 
       * @returns Number of assets that have been created on the network.
       */
      uint64_t                                        get_asset_count()const;


      /** 
       * Retrieves the details of an active escrow transfer.
       *
       * @param from The account that is the sender of the funds into the escrow.
       * @param escrow_id The uuidv4 of the escrow transfer.
       * @returns Escrow information of the transfer with the specified id. 
       */
      optional< escrow_api_obj >                      get_escrow( string from, string escrow_id )const;

      /**
       * Returns staked fund withdraw routes for an account.
       *
       * @param account Account to query routes.
       * @param type Withdraw type type [incoming, outgoing, all].
       * @returns Withdraw routes of an account.
       */
      vector< withdraw_route >                        get_withdraw_routes( string account, withdraw_route_type type = all )const;


      /**
       * Returns active savings withdrawals from an account.
       *
       * @param account Account to query withdrawls.
       * @returns Savings withdrawals from an account.
       */
      vector< savings_withdraw_api_obj >              get_savings_withdraw_from( string account )const;


      /**
       * Returns active savings withdrawals to an account.
       *
       * @param account Account to query withdrawls.
       * @returns Savings withdrawals to an account.
       */
      vector< savings_withdraw_api_obj >              get_savings_withdraw_to( string account )const;


      /**
       * Returns active asset delegations from an account.
       *
       * @param account Account to query delegations.
       * @returns Delegation balances from an account.
       */
      vector< asset_delegation_api_obj >              get_asset_delegations( string account, string from, uint32_t limit = 100 )const;


      /**
       * Returns delegation expirations from an account that will return deletaed balance after expiration.
       *
       * @param account Account to query delegations.
       * @param from Time to begin from.
       * @param limit maximum amount of delegation expirations to retrieve.
       * @returns Delegation balances from an account.
       */
      vector< asset_delegation_expiration_api_obj >   get_expiring_asset_delegations( string account, time_point from, uint32_t limit = 100 )const;



      //===================//
      // === Board API === //
      //===================//


      /** 
       * Returns all available information about a specified board.
       *
       * @param board The name of the board to retrieve information about.
       * @returns Board Object information pertaining to the specified board.
       */
      extended_board                                  get_board( string board )const;


      /** 
       * Returns all available information about a specified list of boards.
       *
       * @param boards List of boards to retrieve information about.
       * @returns Board Object information pertaining to the specified board.
       */
      vector< extended_board >                        get_boards( vector< string > boards )const;



      /** 
       * Returns a list of the boards with the highest number of subscribers.
       *
       * @param from First board to retrieve in the ranking order.
       * @param limit Amount of boards to retrieve.
       * @returns List of Board Objects pertaining to the top subscribed boards.
       */
      vector< extended_board >                        get_boards_by_subscribers( string from, uint32_t limit )const;


      /** 
       * Returns the number of registered boards.
       * 
       * @returns Number of boards that have been created on the network.
       */
      uint64_t                                        get_board_count()const;


      //=====================//
      // === Network API === //
      //=====================//


      /** 
       * Returns information about a specified producer.
       * 
       * @param name The name of the producer account.
       * @returns Producer object information pertaining to the specified producer. 
       */
      producer_api_obj                                get_producer_by_account( string name )const;


      /** 
       * Returns information about a list of producers.
       * 
       * @param names List of names of the producers.
       * @returns Producer objects information pertaining to the list of specified producers. 
       */
      vector< producer_api_obj >                      get_producers_by_account( vector< string > names )const;


      /**
       * Retrieves the current list of active block producers in the current round.
       * 
       * @returns All Producers in the current round of block production.
       */
      vector< account_name_type >                     get_active_producers()const;


      /** 
       * Lists all producers that have been created.
       *
       * @param from The name of the first producer to return.
       * @param limit The maximum number of producers to return (max: 1000)
       * @returns List of producers in alphabetical order.
       */
      set< account_name_type >                        lookup_producer_accounts( string from, uint32_t limit )const;


      /** 
       * Returns the number of registered producers.
       * 
       * @returns Number of producers that have been created on the network.
       */
      uint64_t                                        get_producer_count()const;

      
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
       * Returns a specifed network officer.
       *
       * @param names names of the officer.
       * @returns Network officer object information pertaining to the specified officer. 
       */
      network_officer_api_obj                         get_network_officer_by_account( string name )const;


      /** 
       * Returns a specifed list of the network officers.
       *
       * @param names List of names of the officers.
       * @returns Network officer object information pertaining to the list of specified officers. 
       */
      vector< network_officer_api_obj >               get_network_officers_by_account( vector< string > names )const;


      /** 
       * Returns a list of the network development officers with the highest voting power from stakeholders.
       *
       * @param from First officer to retrieve in the ranking order.
       * @param limit Amount of officers to retrieve.
       * @returns List of network officer objects pertaining to the top officers.
       */
      vector< network_officer_api_obj >               get_development_officers_by_voting_power( string from, uint32_t limit )const;


      /** 
       * Returns a list of the network marketing officers with the highest voting power from stakeholders.
       *
       * @param from First officer to retrieve in the ranking order.
       * @param limit Amount of officers to retrieve.
       * @returns List of network officer objects pertaining to the top officers.
       */
      vector< network_officer_api_obj >               get_marketing_officers_by_voting_power( string from, uint32_t limit )const;


      /** 
       * Returns a list of the network advocacy officers with the highest voting power from stakeholders.
       *
       * @param from First officer to retrieve in the ranking order.
       * @param limit Amount of officers to retrieve.
       * @returns List of network officer objects pertaining to the top officers.
       */
      vector< network_officer_api_obj >               get_advocacy_officers_by_voting_power( string from, uint32_t limit )const;


      /** 
       * Returns a specifed executive board.
       *
       * @param name name of the board business account.
       * @returns Executive board object information pertaining to the account.
       */
      executive_board_api_obj                         get_executive_board_by_account( string name )const;


      /** 
       * Returns a specifed list of executive boards.
       *
       * @param names List of names of executive boards.
       * @returns List of Executive board object information pertaining to the specified accounts.
       */
      vector< executive_board_api_obj >               get_executive_boards_by_account( vector< string > names )const;


      /** 
       * Returns a list of the highest voted executive boards.
       *
       * @param from The first account in the rankings to retrieve.
       * @param limit The amount of executive boards to return.
       * @returns List of Executive boards with the highest stakeholder voting power.
       */
      vector< executive_board_api_obj >               get_executive_boards_by_voting_power( string from, uint32_t limit )const;

      /** 
       * Returns a specifed Supernode.
       *
       * @param name The name of the supernode.
       * @returns Executive board object information pertaining to the account.
       */
      supernode_api_obj                               get_supernode_by_account( string name )const;


      /** 
       * Returns a specifed list of supernodes.
       *
       * @param names List of names of supernodes.
       * @returns List of supernode object information pertaining to the specified accounts.
       */
      vector< supernode_api_obj >                     get_supernodes_by_account( vector< string > names )const;


      /** 
       * Returns a list of the supernodes with the highest view weight from stakeholders.
       *
       * @param from The first account in the rankings to retrieve.
       * @param limit The amount of supernodes to return.
       * @returns List of Supernodes with the highest stakeholder view weight.
       */
      vector< supernode_api_obj >                     get_supernodes_by_view_weight( string from, uint32_t limit )const;

      /** 
       * Returns a specifed Interface.
       *
       * @param name The name of the interface.
       * @returns Executive board object information pertaining to the account.
       */
      interface_api_obj                               get_interface_by_account( string name )const;


      /** 
       * Returns a specifed list of interfaces.
       *
       * @param names List of names of interfaces.
       * @returns List of interface object information pertaining to the specified accounts.
       */
      vector< interface_api_obj >                     get_interfaces_by_account( vector< string > names )const;


      /** 
       * Returns a list of the interfaces with the highest recent view count.
       *
       * @param from The first account in the rankings to retrieve.
       * @param limit The amount of interfaces to return.
       * @returns List of interfaces with the highest recent view count.
       */
      vector< interface_api_obj >                     get_interfaces_by_users( string from, uint32_t limit )const;

      /** 
       * Returns a specifed Governance account.
       *
       * @param name The name of the governance account.
       * @returns governance account object information pertaining to the account.
       */
      governance_account_api_obj                      get_governance_account_by_account( string name )const;


      /** 
       * Returns a specifed list of governance accounts.
       *
       * @param names List of names of governance accounts.
       * @returns List of governance account object information pertaining to the specified accounts.
       */
      vector< governance_account_api_obj >            get_governance_accounts_by_account( vector< string > names )const;
      

      /** 
       * Returns a list of the governance accounts with the highest subscriber voting power.
       *
       * @param from The first account in the rankings to retrieve.
       * @param limit The amount of governance accounts to return.
       * @returns List of governance accounts with the highest subscriber voting power.
       */
      vector< governance_account_api_obj >            get_governance_accounts_by_subscriber_power( string from, uint32_t limit )const;


      /** 
       * Returns a list of the community enterprise proposals with the highest voting power from total approvals.
       *
       * @param from The creator of the first enterprise in the rankings to retrieve.
       * @param from_id The enterprise_id of the first enterprise in the rankings to retrieve.
       * @param limit The amount of enterprise proposals to return.
       * @returns List of enterprise proposals with the highest voting power from total approvals.
       */
      vector< community_enterprise_api_obj >          get_enterprise_by_voting_power( string from, string from_id, uint32_t limit )const;



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
       * Returns a list of open call orders on a market price pair.
       *
       * @param buy_symbol Asset to be the base price of ask orders.
       * @param sell_symbol Asset to the the base price of bid orders.
       * @param limit Maximum number of orders to retrieve from both sides of the orderbook.
       * @returns Orders in a specified price pair.
       */
      market_call_orders                              get_call_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;


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
       * Returns the votes that have been made on a specified comment.
       * 
       * @param author Author of the comment.
       * @param permlink Permlink of the comment.
       * @returns List of the state of all votes made on the comment.
       */
      vector< vote_state >                 get_active_votes( string author, string permlink )const;


      /**
       * Returns the views that have been made on a specified comment.
       * 
       * @param author Author of the comment.
       * @param permlink Permlink of the comment.
       * @returns List of the state of all views made on the comment.
       */
      vector< view_state >                 get_active_views( string author, string permlink )const;


      /**
       * Returns the shares that have been made on a specified comment.
       * 
       * @param author Author of the comment.
       * @param permlink Permlink of the comment.
       * @returns List of the state of all shares made on the comment.
       */
      vector< share_state >                get_active_shares( string author, string permlink )const;


      /**
       * Returns the moderation tags that have been made on a specified comment.
       * 
       * @param author Author of the comment.
       * @param permlink Permlink of the comment.
       * @returns List of the state of all moderation tags made on the comment.
       */
      vector< moderation_state >           get_active_mod_tags( string author, string permlink )const;


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
      vector< tag_following_api_obj >      get_tag_followings( vector< string > tags )const;


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
       * @param parent The author to query.
       * @param parent_permlink The Permlink of the first post to return.
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
       * followed boards, and followed tags. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_feed( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from posts that are in a specified blog from an account, board, or tag.
       * 
       * Blogs include all posts that are created by an account, created with a specifed tag, or in a specifed board.
       * in addition to posts that are shared by the account, or shared with the board or tag. 
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_blog( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information from posts that are recommended for an account.
       * 
       * Recommended posts include a randomized selection of not yet viewed posts from authors, boards and tags
       * that are from authors, boards, and tags that the account 
       * has previously positively engaged with, and authors boards and tags that are closely related to
       * those that are engaged with, but not yet followed.
       * 
       * @param query The details of the posts to return, and sorting options used. 
       * @returns The Discussion information from the posts.
       */
      vector< discussion >                 get_discussions_by_recommended( const discussion_query& query )const;


      /**
       * Retrieves the Discussion information relating to the comments most recently updated by a given author.
       * 
       * @param parent The author to query.
       * @param parent_permlink The Permlink of the first post to return.
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
      app::state                              get_state( string url );



      //==============================//
      // === Account Transactions === //
      //==============================//



      /**
       * Generates a new Account with specified authorities.
       *
       * @param signatory The name of the account signing the transaction.
       * @param registrar The account creating the new account.
       * @param new_account_name The name of the new account.
       * @param referrer The name of the account that lead to the creation of the account.
       * @param proxy Account that is able to vote on behalf of the account.
       * @param governance_account Account that is able to provide governance moderation and verification.
       * @param recovery_account Account able to create recovery requests if the key is compromised.
       * @param reset_account Account able to reset the owner key after inactivity.
       * @param details The account's details string.
       * @param url The account's selected personal URL.
       * @param json The JSON string of public profile information.
       * @param json_private The JSON string of encrypted profile information.
       * @param owner The account authority required for changing the active and posting authorities.
       * @param active The account authority required for sending payments and trading.
       * @param posting The account authority required for posting content and voting.
       * @param secure_public_key The secure encryption key for content only visible to this account.
       * @param connection_public_key The connection public key used for encrypting Connection level content.
       * @param friend_public_key The connection public key used for encrypting Friend level content.
       * @param companion_public_key The connection public key used for encrypting Companion level content.
       * @param business_type The type of business account being created.
       * @param officer_vote_threshold The voting power required to be an active officer.
       * @param business_public_key The public key used for encrypted business content.
       * @param fee Account creation fee for stake on the new account.
       * @param delegation Initial amount delegated to the new account.
       * @param use_wallet_keys Set True to use keys generated locally by this wallet. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_create(
         string signatory,
         string registrar,
         string new_account_name,
         string account_type,
         string referrer,
         string proxy,
         string governance_account,
         string recovery_account,
         string reset_account,
         string details,
         string url,
         string json,
         string json_private,
         authority owner,
         authority active,
         authority posting,
         string secure_public_key,
         string connection_public_key,
         string friend_public_key,
         string companion_public_key,
         string business_type,
         share_type officer_vote_threshold,
         string business_public_key,
         asset fee,
         asset delegation,
         bool use_wallet_keys,
         bool broadcast );
     

      /**
       * Update the details and authorities of an existing account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account The name of the new account.
       * @param details The account's details string.
       * @param url The account's selected personal URL.
       * @param json The JSON string of public profile information.
       * @param json_private The JSON string of encrypted profile information.
       * @param pinned_permlink The permlink of the pinned comment of the author's blog. 
       * @param owner The account authority required for changing the active and posting authorities.
       * @param active The account authority required for sending payments and trading.
       * @param posting The account authority required for posting content and voting.
       * @param secure_public_key The secure encryption key for content only visible to this account.
       * @param connection_public_key The connection public key used for encrypting Connection level content.
       * @param friend_public_key The connection public key used for encrypting Friend level content.
       * @param companion_public_key The connection public key used for encrypting Companion level content.
       * @param business_type The type of business account being created.
       * @param officer_vote_threshold The voting power required to be an active officer.
       * @param business_public_key The public key used for encrypted business content.
       * @param deleted Set to True to Delete Account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           account_update( 
         string signatory,
         string account,
         string details,
         string url,
         string json,
         string json_private,
         string pinned_permlink,
         authority owner,
         authority active,
         authority posting,
         string secure_public_key,
         string connection_public_key,
         string friend_public_key,
         string companion_public_key,
         string business_type,
         share_type officer_vote_threshold,
         string business_public_key,
         bool deleted,
         bool broadcast )const;


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
       * Vote for a Producer for selection to producer blocks. 
       *
       * @param signatory The name of the account signing the transaction.
       * @param account The account voting for a producer.
       * @param vote_rank Rank ordering of the vote.
       * @param producer The producer that is being voted for.
       * @param approved  True to create vote, false to remove vote.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           vote_producer(
         string signatory,
         string account,
         uint16_t vote_rank,
         string producer,
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


      /**
       * Creates or updates a network officer object for a member.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Name of the member's account.
       * @param officer_type The type of network officer that the account serves as. 
       * @param details Information about the network officer and their work
       * @param url The officers's description URL explaining their details. 
       * @param json Additonal information about the officer.
       * @param active Set true to activate the officer, false to deactivate. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           update_network_officer(
         string signatory,
         string account,
         string officer_type,
         string details,
         string url,
         string json,
         bool active
         bool broadcast );


      /**
       * Votes to support a network officer.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account The name of the account voting for the officer.
       * @param network_officer The name of the network officer being voted for.
       * @param vote_rank Number of vote rank ordering.
       * @param approved True if approving, false if removing vote.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           network_officer_vote(
         string signatory,
         string account,
         string network_officer,
         uint16_t vote_rank,
         bool approved,
         bool broadcast );

      
      /**
       * Creates or updates a executive board object for a member.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Name of the account updating the executive board.
       * @param executive Name of the Executive board account being updated.
       * @param budget The type of executive board that the account serves as. 
       * @param details Information about the executive board and their work
       * @param url The teams's description URL explaining their details. 
       * @param json Additonal information about the executive board.
       * @param active Set true to activate the board, false to deactivate. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           update_executive_board(
         string signatory,
         string account,
         string executive,
         asset budget
         string details,
         string url,
         string json,
         bool active
         bool broadcast );


      /**
       * Votes to support a executive board.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account The name of the account voting for the board.
       * @param executive_board The name of the executive board being voted for.
       * @param vote_rank Number of vote rank ordering.
       * @param approved True if approving, false if removing vote.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           executive_board_vote(
         string signatory,
         string account,
         string executive_board,
         uint16_t vote_rank,
         bool approved,
         bool broadcast );


      /**
       * Creates or updates a governance account for a member.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Name of the governance account.
       * @param details Information about the governance account's filtering and tagging policies
       * @param url The governance account's description URL explaining their details. 
       * @param json Additonal information about the governance account policies.
       * @param active Set true to activate governance account, false to deactivate. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           update_governance(
         string signatory,
         string account,
         string details,
         string url,
         string json,
         bool active
         bool broadcast );


      /**
       * Adds a governance account to the subscription set of the account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account The account subscribing to the governance account.
       * @param governance_account The name of the governance account.
       * @param subscribe True if subscribing, false if unsubscribing.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           subscribe_governance(
         string signatory,
         string account,
         string governance_account,
         bool subscribe,
         bool broadcast );


      /**
       * Creates or updates a supernode object for an infrastructure provider.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Name of the member's account.
       * @param details Information about the supernode, and the range of storage and node services they operate.
       * @param url The supernode's reference URL.
       * @param node_api_endpoint The Full Archive node public API endpoint of the supernode.
       * @param notification_api_endpoint The Notification API endpoint of the Supernode.
       * @param auth_api_endpoint The Transaction signing authentication API endpoint of the supernode.
       * @param ipfs_endpoint The IPFS file storage API endpoint of the supernode.
       * @param bittorrent_endpoint The Bittorrent Seed Box endpoint URL of the Supernode. 
       * @param json Additonal information about the Supernode.
       * @param active Set true to activate the supernode, false to deactivate. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           update_supernode(
         string signatory,
         string account,
         string details,
         string url,
         string node_api_endpoint,
         string notification_api_endpoint,
         string auth_api_endpoint,
         string ipfs_endpoint,
         string bittorrent_endpoint,
         string json,
         bool active
         bool broadcast );

      
      /**
       * Creates or updates an interface object for an application developer.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Name of the member's account.
       * @param details Information about the interface, and what they are offering to users.
       * @param url The interface's reference URL.
       * @param json Additonal information about the interface.
       * @param active Set true to activate the interface, false to deactivate. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           update_interface(
         string signatory,
         string account,
         string details,
         string url,
         string json,
         bool active
         bool broadcast );


      /**
       * Creates or updates a mediator object for marketplace escrow facilitator.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Name of the member's account.
       * @param details Information about the mediator, and what they are offering to users
       * @param url The mediator's reference URL.
       * @param json Additonal information about the mediator.
       * @param mediator_bond Amount of Core asset to stake in the mediation pool. 
       * @param active Set true to activate the interface, false to deactivate. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           update_mediator(
         string signatory,
         string account,
         string details,
         string url,
         string json,
         asset mediator_bond,
         bool active
         bool broadcast );


      /**
       * Creates a new community enterprise proposal.
       *
       * @param signatory The name of the account signing the transaction.
       * @param creator The name of the account that created the community enterprise proposal.
       * @param enterprise_id uuidv4 referring to the proposal.
       * @param proposal_type The type of proposal, determines release schedule.
       * @param beneficiaries Set of account names and percentages of budget value. Should not include the null account.
       * @param milestones Ordered vector of release milestone descriptions and percentages of budget value.
       * @param investment Symbol of the asset to be purchased with the funding if the proposal is investment type. 
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
      annotated_signed_transaction           create_community_enterprise(
         string signatory,
         string creator,
         string enterprise_id,
         string proposal_type,
         flat_map< string, uint16_t > beneficiaries,
         vector< pair < string, uint16_t > > milestones,
         string investment,
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
       * Claims a milestone from a community enterprise proposal.
       *
       * @param signatory The name of the account signing the transaction.
       * @param creator The name of the account that created the community enterprise proposal.
       * @param enterprise_id uuidv4 referring to the proposal.
       * @param milestone Number of the milestone that is being claimed as completed. Number 0 for initial acceptance. 
       * @param details Description of completion of milestone, with supporting evidence.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           claim_enterprise_milestone(
         string signatory,
         string creator,
         string enterprise_id,
         uint16_t milestones,
         string details,
         bool broadcast );


      /**
       * Approves a milestone claim from a community enterprise proposal.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account approving the milestone.
       * @param creator The name of the account that created the community enterprise proposal.
       * @param enterprise_id uuidv4 referring to the proposal.
       * @param milestone Number of the milestone that is being approved as completed. 
       * @param vote_rank The rank of the approval for enterprise proposals.
       * @param approved True to approve the milestone claim, false to remove approval. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           approve_enterprise_milestone(
         string signatory,
         string account,
         string creator,
         string enterprise_id,
         uint16_t milestone,
         uint16_t vote_rank,
         bool approved,
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
       *    "rating": "\"general\"",
       *    "max_accepted_payout": "\"1000000000.00000000 MUSD\"",
       *    "percent_liquid": 10000,
       *    "allow_replies": true,
       *    "allow_votes": true,
       *    "allow_views": true,
       *    "allow_shares": true,
       *    "allow_curation_rewards": true,
       *    "beneficiaries": []
       * }
       *
       * @param signatory The name of the account signing the transaction.
       * @param author Name of the account that created the post.
       * @param permlink Unique identifing string for the post.
       * @param title Content related name of the post, used to find post with search API.
       * @param body String containing text for display when the post is opened.
       * @param ipfs Vector of Strings containing IPFS file hashes: images, videos, files.
       * @param magnet Vector of Strings containing bittorrent magnet links to torrent file swarms: videos, files.
       * @param language String containing the two letter ISO language code of the native language of the author.
       * @param board The name of the board to which the post is uploaded to.
       * @param public_key The public key used to encrypt the post, holders of the private key may decrypt.
       * @param interface Name of the interface application that broadcasted the transaction.
       * @param comment_price Price that is required to comment on the post.
       * @param premium_price Price that is required to unlock premium content.
       * @param parent_author Account that created the post this post is replying to, empty if root post.
       * @param parent_permlink Permlink of the post this post is replying to, empty if root post.
       * @param tags Set of string tags for sorting the post by.
       * @param json json string of additional interface specific data relating to the post.
       * @param options Settings for the post, that effect how the network applies and displays it.
       * @param deleted True to delete post, false to create post.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           comment(
         string signatory,
         string author,
         string permlink,
         string title,
         string body,
         vector< string > ipfs,
         vector< string > magnet,
         string language,
         string board,
         string public_key,
         string interface,
         asset comment_price,
         asset premium_price,
         string parent_author, 
         string parent_permlink, 
         vector< string > tags,
         string json,
         comment_options options,
         bool deleted,
         bool broadcast );


      /**
       * Creates a private encrypted message between two accounts.
       *
       * @param signatory The name of the account signing the transaction.
       * @param sender The account sending the message.
       * @param recipient The receiving account of the message.
       * @param message Encrypted ciphertext of the message being sent. 
       * @param uuid uuidv4 uniquely identifying the message for local storage.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           message(
         string signatory,
         string sender, 
         string recipient, 
         string message,
         string uuid,
         bool broadcast );
      

      /**
       * Votes for a comment to allocate content rewards and increase the posts ranked ordering.
       *
       * @param signatory The name of the account signing the transaction.
       * @param voter Name of the voting account.
       * @param author Name of the account that created the post being voted on.
       * @param permlink Permlink of the post being voted on.
       * @param weight Percentage weight of the voting power applied to the post.
       * @param interface Name of the interface account that was used to broadcast the transaction. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           vote(
         string signatory,
         string voter, 
         string author, 
         string permlink, 
         int16_t weight,
         string interface,
         bool broadcast );


      /**
       * Views a post, which increases the post's content reward earnings.
       *
       * @param signatory The name of the account signing the transaction.
       * @param viewer Name of the viewing account.
       * @param author Name of the account that created the post being viewed.
       * @param permlink Permlink of the post being viewed.
       * @param interface Name of the interface account that was used to broadcast the transaction and view the post.
       * @param supernode Name of the supernode account that served the IPFS file data in the post.
       * @param viewed True if viewing the post, false if removing view object.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           view(
         string signatory,
         string viewer,
         string author,
         string permlink,
         string interface,
         string supernode,
         bool viewed,
         bool broadcast );


      /**
       * Shares a post to the account's feed.
       *
       * @param signatory The name of the account signing the transaction.
       * @param sharer Name of the viewing account.
       * @param author Name of the account that created the post being shared.
       * @param permlink Permlink of the post being shared.
       * @param reach Audience reach selection for share.
       * @param interface Name of the interface account that was used to broadcast the transaction and share the post.
       * @param board Optionally share the post with a new board.
       * @param tag Optionally share the post with a new tag.
       * @param shared True if sharing the post, false if removing share.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           share(
         string signatory,
         string sharer,
         string author,
         string permlink,
         string reach,
         string interface,
         string board,
         string tag,
         bool shared,
         bool broadcast );


      /**
       * Applies a set of tags to a post for filtering from interfaces.
       *
       * @param signatory The name of the account signing the transaction.
       * @param moderator Account creating the tag: can be a governance address or a board moderator. 
       * @param author Author of the post being tagged.
       * @param permlink Permlink of the post being tagged.
       * @param tags Set of tags to apply to the post for selective interface side filtering.
       * @param rating Newly proposed rating for the post.
       * @param details String explaining the reason for the tag to the author.
       * @param interface Interface account used for the transaction.
       * @param filter True if the post should be filtered from the board and governance account subscribers.
       * @param applied True if applying the tag, false if removing the tag.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           moderation_tag(
         string signatory,
         string moderator,
         string author,
         string permlink,
         vector< string > tags,
         string rating,
         string details,
         string interface,
         bool filter,
         bool applied,
         bool broadcast );



      //============================//
      // === Board Transactions === //
      //============================//



      /**
       * Creates a new board for collecting posts about a specific topic.
       *
       * @param signatory The name of the account signing the transaction.
       * @param founder The account that created the board, able to add and remove administrators.
       * @param name Name of the board.
       * @param board_type Type of board Structure to use, determines content types.
       * @param board_privacy Type of board Privacy to us, determines access permissions and encryption.
       * @param board_public_key Key used for encrypting and decrypting posts. Private key shared with accepted members.
       * @param json Public plaintext json information about the board, its topic and rules.
       * @param json_private Private ciphertext json information about the board.
       * @param details Details of the board, describing what it is for.
       * @param url External reference URL.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_create(
         string signatory,
         string founder,
         string name,
         string board_type,
         string board_privacy,
         string board_public_key,
         string json,
         string json_private,
         string details,
         string url,
         bool broadcast );


      /**
       * Updates the details of an existing board.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account updating the board.
       * @param board Name of the board.
       * @param board_public_key Key used for encrypting and decrypting posts. Private key shared with accepted members.
       * @param json Public plaintext json information about the board, its topic and rules.
       * @param json_private Private ciphertext json information about the board.
       * @param details Details of the board, describing what it is for.
       * @param url External reference URL.
       * @param pinned_author Author of the pinned post.
       * @param pinned_permlink Permlink of the pinned post.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_update(
         string signatory,
         string account,
         string board,
         string board_public_key,
         string json,
         string json_private,
         string details,
         string url,
         string pinned_author,
         atring pinned_permlink,
         bool broadcast );


      /**
       * Adds a new moderator to a board.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account of an administrator of the board.
       * @param board Board that the moderator is being added to.
       * @param moderator New moderator account.
       * @param added True when adding a new moderator, false when removing.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_add_mod(
         string signatory,
         string account,
         string board,
         string moderator,
         bool added,
         bool broadcast );


      /**
       * Adds a new administrator to a board.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account of the founder of the board.
       * @param board Board that the admin is being added to.
       * @param admin New administrator account.
       * @param added True when adding a new administrator, false when removing.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_add_admin(
         string signatory,
         string account,
         string board,
         string admin,
         bool added,
         bool broadcast );


      /**
       * Votes for a moderator to increase their mod weight.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account of a member of the board.
       * @param board Board that the moderator is being voted into.
       * @param moderator Moderator account.
       * @param vote_rank Voting rank for the specified board moderator.
       * @param approved True when voting for the moderator, false when removing.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_vote_mod(
         string signatory,
         string account,
         string board,
         string moderator,
         uint16_t vote_rank,
         bool approved,
         bool broadcast );


      /**
       * Transfers a board to a new account as the founder.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account that created the board.
       * @param board Board that is being transferred.
       * @param new_founder Account of the new founder.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_transfer_ownership(
         string signatory,
         string account,
         string board,
         string new_founder,
         bool broadcast );


      /**
       * Requests that an account be added as a new member of a board.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account that wants to join the board.
       * @param board Board that is being requested to join.
       * @param message Message attatched to the request, encrypted with the boards public key.
       * @param requested Set true to request, false to cancel request.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_join_request(
         string signatory,
         string account,
         string board,
         string message,
         bool requested,
         bool broadcast );


      /**
       * Invite a new member to a board.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account sending the invitation.
       * @param member New board member account being invited.
       * @param board Board that is the member is being invited to.
       * @param message Message attatched to the invite, encrypted with the member's secure public key.
       * @param encrypted_board_key The Board Private Key, encrypted with the member's secure public key.
       * @param invited Set true to invite, false to cancel invite.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_join_invite(
         string signatory,
         string account,
         string member,
         string board,
         string message,
         string encrypted_board_key,
         bool invited,
         bool broadcast );


      /**
       * Used to accept to a request and admit a new member.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account within the board accepting the request.
       * @param member Account to accept into the board.
       * @param board Board that is being joined.
       * @param encrypted_board_key The Board Private Key, encrypted with the member's secure public key.
       * @param accepted Set true to invite, false to cancel invite.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_join_accept(
         string signatory,
         string account,
         string member,
         string board,
         string encrypted_board_key,
         bool accepted,
         bool broadcast );


      /**
       * Accepts a board invitation.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account A new member of the board.
       * @param board Board that the account was invited to.
       * @param accepted True to accept invite, false to reject invite.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_invite_accept(
         string signatory,
         string account,
         string board,
         bool accepted,
         bool broadcast );


      /**
       * Removes a specifed member of a board.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Either the member of the board leaving OR a moderator of the board removing the member.
       * @param member Account to be removed from the board membership.
       * @param board Board that that member is being removed from.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_remove_member(
         string signatory,
         string account,
         string member,
         string board,
         bool broadcast );


      /**
       * Adds a specifed account to the board's blacklist.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Moderator or admin of the board.
       * @param member Account to be blacklisted from interacting with the board.
       * @param board Board that member is being blacklisted from.
       * @param blacklisted Set to true to add account to blacklist, set to false to remove from blacklist. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_blacklist(
         string signatory,
         string account,
         string member,
         string board,
         bool blacklisted,
         bool broadcast );


      /**
       * Adds a board to an account's subscriptions.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account that wants to subscribe to the board.
       * @param board Board to suscribe to.
       * @param interface Name of the interface account that was used to broadcast the transaction and subscribe to the board.
       * @param added True to add to lists, false to remove.
       * @param subscribed true if subscribing, false if filtering.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           board_subscribe(
         string signatory,
         string account,
         string board,
         string interface,
         bool added,
         bool subscribed,
         bool broadcast );



      //=========================//
      // === Ad Transactions === //
      //=========================//



      /**
       * Creates a new ad creative to be used in a campaign for display in interfaces.
       *
       * @param signatory The name of the account signing the transaction.
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
         string signatory,
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
       * @param signatory The name of the account signing the transaction.
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
         string signatory,
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
       * @param signatory The name of the account signing the transaction.
       * @param provider Account of an interface offering ad supply.
       * @param inventory_id uuidv4 referring to the inventory offering.
       * @param audience_id uuidv4 referring to audience object containing usernames of desired accounts in interface's audience.
       * @param metric Type of expense metric used.
       * @param min_price Minimum bidding price per metric.
       * @param inventory Total metrics available.
       * @param json JSON metadata for the inventory.
       * @param agents Set of Accounts authorized to create bids for the campaign.
       * @param active True if the inventory is enabled for display, false to deactivate.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           ad_inventory(
         string signatory,
         string provider,
         string inventory_id,
         string audience_id,
         string metric,
         asset min_price,
         uint32_t inventory,
         string json,
         vector< string > agents,
         string interface,
         bool active,
         bool broadcast );


      /**
       * Contains a set of accounts that are valid for advertising display.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account creating the audience set.
       * @param audience_id uuidv4 referring to the audience for inclusion in inventory and campaigns.
       * @param json JSON metadata for the audience.
       * @param audience List of usernames of viewing accounts.
       * @param active True if the audience is enabled for reference, false to deactivate.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           ad_audience(
         string signatory,
         string account,
         string audience_id,
         string json,
         vector< string > audience,
         bool active,
         bool broadcast );


      /**
       * Creates a new advertising bid offer.
       *
       * @param signatory The name of the account signing the transaction.
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
       * @param json JSON metadata for the inventory.
       * @param expiration Time the the bid is valid until, bid is cancelled after this time if not filled. 
       * @param active True if the bid is open for delivery, false to cancel.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction           ad_bid(
         string signatory,
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
         string json,
         time_point expiration,
         bool active,
         bool broadcast );



      //===============================//
      // === Transfer Transactions === //
      //===============================//


      /**
       * Transfer funds from one account to another.
       *
       * @param signatory The name of the account signing the transaction.
       * @param from The account the funds are coming from.
       * @param to The account the funds are going to.
       * @param amount The funds being transferred.
       * @param memo The memo for the transaction, encryption on the memo is advised. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer(
         string signatory,
         string from,
         string to,
         asset amount,
         string memo,
         bool broadcast = false );


      /**
       * Requests a Transfer from an account to another account.
       *
       * @param signatory The name of the account signing the transaction.
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
         string signatory,
         string to,
         string from,
         asset amount,
         string memo,
         string request_id,
         time_point expiration,
         bool requested,
         bool broadcast = false );


      /**
       * Accepts a transfer request from an account to another account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param from Account that is accepting the transfer.
       * @param to Account requesting the transfer.
       * @param request_id uuidv4 of the request transaction.
       * @param accepted True to accept the request, false to reject. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_accept(
         string signatory,
         string from,
         string to,
         string request_id,
         bool accepted,
         bool broadcast = false );


      /**
       * Transfers an asset periodically from one account to another.
       *
       * @param signatory The name of the account signing the transaction.
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
         string signatory,
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
         bool broadcast = false );


      /**
       * Requests a periodic transfer from an account to another account.
       *
       * @param signatory The name of the account signing the transaction.
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
         string signatory,
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
         bool broadcast = false );


      /**
       * Accepts a periodic transfer request from an account to another account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param from Account that is accepting the recurring transfer.
       * @param to Account requesting the recurring transfer.
       * @param request_id uuidv4 of the request transaction.
       * @param accepted True to accept the request, false to reject. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_recurring_accept(
         string signatory,
         string from,
         string to,
         string request_id,
         bool accepted,
         bool broadcast = false );



      //==============================//
      // === Balance Transactions === //
      //==============================//


      /**
       * Claims an account's reward balance into it's liquid balance from newly issued assets.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account claiming its reward balance from the network.
       * @param reward Amount of Reward balance to claim.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            claim_reward_balance(
         string signatory,
         string account,
         asset reward,
         bool broadcast );


      /**
       * Stakes a liquid balance of an account into it's staked balance.
       * 
       * @param signatory The name of the account signing the transaction.
       * @param from Account staking the asset.
       * @param to Account to stake the asset to, Same as from if null.
       * @param amount Amount of Funds to transfer to staked balance from liquid balance.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            stake_asset(
         string signatory,
         string from,
         string to,
         asset amount,
         bool broadcast = false );


      /**
       * Divests an amount of the staked balance of an account to it's liquid balance.
       *
       * @param signatory The name of the account signing the transaction.
       * @param from Account unstaking the asset.
       * @param to Account to unstake the asset to, Same as from if null.
       * @param amount Amount of Funds to transfer from staked balance to liquid balance.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            unstake_asset(
         string signatory,
         string from,
         string to,
         asset amount,
         bool broadcast = false );


      /**
       * Set up an asset withdraw route.
       * 
       * @param signatory The name of the account signing the transaction.
       * @param from The account the assets are withdrawn from.
       * @param to The account receiving either assets or new stake.
       * @param percent The percent of the withdraw to go to the 'to' account.
       * @param auto_stake True if the stake should automatically be staked on the to account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            unstake_asset_route(
         string signatory,
         string from, 
         string to, 
         uint16_t percent, 
         bool auto_stake, 
         bool broadcast = false );

      /**
       * Transfer liquid funds balance into savings for security.
       * 
       * @param signatory The name of the account signing the transaction.
       * @param from The account the assets are transferred from.
       * @param to The account that is recieving the savings balance, same as from if null.
       * @param amount Funds to be transferred from liquid to savings balance.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_to_savings(
         string signatory,
         string from,
         string to,
         asset amount,
         string memo,
         bool broadcast = false );

      /**
       * Withdraws a specified balance from savings after a time duration.
       * 
       * @param signatory The name of the account signing the transaction.
       * @param from Account to transfer savings balance from.
       * @param to Account to recieve the savings withdrawal.
       * @param amount Amount of asset to transfer from savings.
       * @param request_id uuidv4 referring to the transfer.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param transferred True if the transfer is accepted, false to cancel transfer.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            transfer_from_savings(
         string signatory,
         string from,
         string to,
         asset amount,
         string request_id,
         string memo,
         bool transferred,
         bool broadcast = false );


      /**
       * Delegate a staked asset balance from one account to the other.
       *
       * @param signatory The name of the account signing the transaction.
       * @param delegator The account delegating the asset.
       * @param delegatee The account receiving the asset.
       * @param amount The amount of the asset delegated.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            delegate_asset(
         string signatory,
         string delegator, 
         string delegatee, 
         asset amount, 
         bool broadcast );



      //=============================//
      // === Escrow Transactions === //
      //=============================//


      /**
       * Creates a proposed escrow transfer between two accounts.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account creating the transaction to initate the escrow.
       * @param from Account sending funds for a purchase.
       * @param to Account receiving funds from a purchase.
       * @param escrow_id uuidv4 referring to the escrow transaction.
       * @param amount Amount of the asset to be transferred upon success.
       * @param acceptance_time Time that the escrow proposal must be approved before.
       * @param escrow_expiration Time after which balance can be claimed by FROM or TO freely.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param json Additonal JSON object attribute details.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            escrow_transfer(
         string signatory,
         string account,
         string from,
         string to,
         string escrow_id,
         asset amount,
         time_point acceptance_time,
         time_point escrow_expiration,
         string memo,
         string json,
         bool broadcast = false );

      /**
       * Approves an escrow transfer, causing it to be locked in.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account creating the transaction to approve the escrow.
       * @param mediator Nominated mediator to join the escrow for potential dispute resolution.
       * @param escrow_from The account sending funds into the escrow.
       * @param escrow_id uuidv4 referring to the escrow being approved.
       * @param approved Set true to approve escrow, false to reject the escrow. All accounts must approve before activation.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            escrow_approve(
         string signatory,
         string account,
         string mediator,
         string escrow_from,
         string escrow_id,
         bool approved,
         bool broadcast = false
      );

      /**
       * Raise a dispute on the escrow transfer before it expires
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account creating the transaction to dispute the escrow and raise it for resolution
       * @param escrow_from The account sending funds into the escrow.
       * @param escrow_id  uuidv4 referring to the escrow being disputed.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            escrow_dispute(
         string signatory,
         string account,
         string escrow_from,
         string escrow_id,
         bool broadcast = false );

      /**
       * Raise a dispute on the escrow transfer before it expires
       *
       * @param signatory The name of the account signing the transaction.
       * @param account The account creating the operation to release the funds.
       * @param escrow_from The escrow FROM account.
       * @param escrow_id uuidv4 referring to the escrow.
       * @param release_percent Percentage of escrow to release to the TO Account / remaining will be refunded to FROM account.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction            escrow_release(
         string signatory,
         string account,
         string escrow_from,
         string escrow_id,
         uint16_t release_percent,
         bool broadcast = false );



      //==============================//
      // === Trading Transactions === //
      //==============================//


      /**
       * Creates a new limit order for exchanging assets at a specifed price.
       *
       * @param signatory The name of the account signing the transaction.
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
         string signatory,
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
       * @param signatory The name of the account signing the transaction.
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
         string signatory,
         string owner,
         string order_id,
         price exchange_rate,
         asset collateral,
         asset amount_to_borrow,
         price stop_loss_price,
         price take_profit_price,
         price limit_top_loss_price,
         price limit_take_profit_price,
         string interface,
         time_point expiration,
         bool opened,
         bool fill_or_kill,
         bool force_close,
         bool broadcast );


      /**
       * Creates a new collateralized debt position in a market issued asset.
       *
       * @param signatory The name of the account signing the transaction.
       * @param owner Owner of the debt position and collateral.
       * @param collateral Amount of collateral to add to the margin position.
       * @param debt Amount of the debt to be issued.
       * @param target_collateral_ratio Maximum CR to maintain when selling collateral on margin call.
       * @param interface Name of the interface that broadcasted the transaction.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             call_order(
         string signatory,
         string owner,
         asset collateral,
         asset debt,
         uint16_t target_collateral_ratio,
         string interface,
         bool broadcast );


      /**
       * Used to create a bid for outstanding debt of a globally settled market issued asset.
       *
       * @param signatory The name of the account signing the transaction.
       * @param bidder Adds additional collateral to the market issued asset.
       * @param collateral The amount of collateral to bid for the debt.
       * @param debt The amount of debt to take over.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             bid_collateral(
         string signatory,
         string bidder,
         asset collateral,
         asset debt,
         bool broadcast );



      //===========================//
      // === Pool Transactions === //
      //===========================//



      /**
       * Creates a new liquidity pool between two assets.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Creator of the new liquidity pool.
       * @param first_amount Initial balance of one asset.
       * @param second_amount Initial balance of second asset, initial price is the ratio of these two amounts.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             liquidity_pool_create(
         string signatory,
         string account,
         asset first_amount,
         asset second_amount,
         bool broadcast );


      /**
       * Exchanges an asset directly from liquidity pools.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account executing the exchange with the pool.
       * @param amount Amount of asset to be exchanged.
       * @param receive_asset The asset to recieve from the liquidity pool.
       * @param interface Name of the interface account broadcasting the transaction.
       * @param limit_price The price of acquistion at which to cap the exchange to.
       * @param acquire Set true to acquire the specified amount, false to exchange in.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             liquidity_pool_exchange(
         string signatory,
         string account,
         asset amount,
         asset_symbol_type receive_asset,
         string interface,
         price limit_price,
         bool acquire,
         bool broadcast );


      /**
       * Adds capital to a liquidity pool.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account funding the liquidity pool to recieve the liquidity pool asset.
       * @param amount Amount of an asset to contribute to the liquidity pool.
       * @param pair_asset Pair asset to the liquidity pool to recieve liquidity pool assets of. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             liquidity_pool_fund(
         string signatory,
         string account,
         asset amount,
         asset_symbol_type pair_asset,
         bool broadcast );


      /**
       * Removes capital from a liquidity pool.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account withdrawing liquidity pool assets from the pool.
       * @param amount Amount of the liquidity pool asset to redeem for underlying deposited assets. 
       * @param recieve_asset The asset to recieve from the liquidity pool.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             liquidity_pool_withdraw(
         string signatory,
         string account,
         asset amount,
         asset_symbol_type recieve_asset,
         bool broadcast );


      /**
       * Adds an asset to an account's credit collateral position of that asset.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account locking an asset as collateral. 
       * @param amount Amount of collateral balance to lock, 0 to unlock existing collateral.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             credit_pool_collateral(
         string signatory,
         string account,
         asset amount,
         bool broadcast );


      /**
       * Borrows an asset from the credit pool of the asset.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account borrowing funds from the pool, must have sufficient collateral.
       * @param amount Amount of an asset to borrow. Limit of 75% of collateral value. Set to 0 to repay loan.
       * @param collateral Amount of an asset to use as collateral for the loan. Set to 0 to reclaim collateral to collateral balance.
       * @param loan_id uuidv4 unique identifier for the loan.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             credit_pool_borrow(
         string signatory,
         string account,
         asset amount,
         asset collateral,
         string load_id,
         bool broadcast );


      /**
       * Lends an asset to a credit pool.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account lending an asset to the credit pool.
       * @param amount Amount of asset being lent.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             credit_pool_lend(
         string signatory,
         string account,
         asset amount,
         bool broadcast );


      /**
       * Withdraws an asset from the specified credit pool.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account withdrawing its lent asset from the credit pool by redeeming credit-assets. 
       * @param amount Amount of interest bearing credit assets being redeemed for thier underlying assets. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             credit_pool_withdraw(
         string signatory,
         string account,
         asset amount,
         bool broadcast );


      //============================//
      // === Asset Transactions === //
      //============================//



      /**
       * Creates a new asset object of the asset type provided.
       *
       * @param signatory The name of the account signing the transaction.
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
         string signatory,
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
       * @param signatory The name of the account signing the transaction.
       * @param issuer Name of the issuing account, can create units and administrate the asset.
       * @param asset_to_update The ticker symbol of this asset.
       * @param options Series of options paramters that apply to all asset types.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_update(
         string signatory,
         string issuer,
         string asset_to_update,
         asset_options new_options,
         bool broadcast );


      /**
       * Issues an amount of an asset to a specified account.
       *
       * @param signatory The name of the account signing the transaction.
       * @param issuer The issuer of the asset.
       * @param asset_to_issue Amount of asset being issued to the account.
       * @param issue_to_account Account receiving the newly issued asset.
       * @param memo The memo for the transaction, encryption on the memo is advised.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_issue(
         string signatory,
         string issuer,
         asset asset_to_issue,
         string issue_to_account,
         string memo,
         bool broadcast );


      /**
       * Takes a specified amount of an asset out of circulation.
       *
       * @param signatory The name of the account signing the transaction.
       * @param payer Account that is reserving the asset back to the unissued supply.
       * @param amount_to_reserve Amount of the asset being reserved.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_reserve(
         string signatory,
         string payer,
         asset amount_to_reserve,
         bool broadcast );


      /**
       * Updates the issuer account of an asset.
       *
       * @param signatory The name of the account signing the transaction.
       * @param issuer The current issuer of the asset.
       * @param asset_to_update The asset symbol being updated.
       * @param new_issuer Name of the account specified to become the new issuer of the asset.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_update_issuer(
         string signatory,
         string issuer,
         string asset_to_update,
         string new_issuer,
         bool broadcast );


      /**
       * Update the set of feed-producing accounts for a BitAsset.
       *
       * @param signatory The name of the account signing the transaction.
       * @param issuer The issuer of the BitAsset.
       * @param asset_to_update The BitAsset being updated.
       * @param new_feed_producers Set of accounts that can determine the price feed of the asset.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_update_feed_producers(
         string signatory,
         string issuer,
         string asset_to_update,
         flat_set< account_name_type > new_feed_producers,
         bool broadcast );


      /**
       * Publish price feeds for BitAssets.
       *
       * @param signatory The name of the account signing the transaction.
       * @param publisher Account publishing the price feed.
       * @param symbol Asset for which the feed is published.
       * @param feed Exchange rate between bitasset and backing asset.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_publish_feed(
         string signatory,
         string publisher,
         string symbol,
         price_feed feed,
         bool broadcast );


      /**
       * Schedules a BitAsset balance for automatic settlement.
       *
       * @param signatory The name of the account signing the transaction.
       * @param account Account requesting the force settlement.
       * @param amount Amount of asset to force settle. Set to 0 to cancel order.
       * @param interface  Account of the interface used to broadcast the operation.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_settle(
         string signatory,
         string account,
         asset amount,
         string interface,
         bool broadcast );


      /**
       * Globally Settles a BitAsset, collecting all remaining collateral and debt and setting a global settlement price.
       *
       * @param signatory The name of the account signing the transaction.
       * @param issuer Issuer of the asset being settled. 
       * @param asset_to_settle Symbol of the asset being settled. 
       * @param settle_price Global settlement price, must be in asset / backing asset. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction             asset_global_settle(
         string signatory,
         string issuer,
         string asset_to_settle
         price settle_price,
         bool broadcast );



      //=====================================//
      // === Block Producer Transactions === //
      //=====================================//

      
      /**
       * Creates or updates a producer for a specified account, enabling block production.
       *
       * @param signatory The name of the account signing the transaction.
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
      annotated_signed_transaction              producer_update(
         string signatory,
         string owner,
         string details,
         string url,
         string json,
         double latitude,
         double longitude,
         string block_signing_key,
         chain_properties props,
         bool active,
         bool broadcast = false );


      /**
       * Enables mining producers to publish cryptographic proofs of work.
       *
       * @param work Proof of work, containing a reference to a prior block, and a nonce resulting in a low hash value.
       * @param new_owner_key If creating a new account with a proof of work, the owner key of the new account.
       * @param props Chain properties values for selection of adjustable network parameters. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction              proof_of_work(
         proof_of_work_type work,
         string new_owner_key,
         chain_properties props,
         bool broadcast = false );


      /**
       * Enables block producers to verify that a valid block exists at a given height.
       *
       * @param signatory The name of the account signing the transaction.
       * @param producer The name of the block producing account.
       * @param block_id The block id of the block being verifed as valid and received. 
       * @param block_height The height of the block being verified.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction              verify_block(
         string signatory,
         string producer,
         string block_id,
         uint64_t block_height,
         bool broadcast = false );


      /**
       * Stakes COIN on the validity and acceptance of a block.
       *
       * @param signatory The name of the account signing the transaction.
       * @param producer The name of the block producing account.
       * @param block_id The block id of the block being committed as irreversible to that producer. 
       * @param block_height The height of the block being committed to.
       * @param verifications The set of attesting transaction ids of verification transactions from currently active producers.
       * @param commitment_stake The value of staked balance that the producer stakes on this commitment. Must be at least one unit of COIN. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction              commit_block(
         string signatory,
         string producer,
         string block_id,
         uint64_t block_height,
         flat_set< transaction_id_type > verifications,
         asset commitment_stake,
         bool broadcast = false );


      /**
       * Declares a violation of a block commitment.
       *
       * @param signatory The name of the account signing the transaction.
       * @param reporter The account detecting and reporting the violation.
       * @param first_trx The first transaction signed by the producer.
       * @param second_trx The second transaction that is in contravention of the first commitment transaction. 
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction              producer_violation(
         string signatory,
         string reporter,
         signed_transaction first_trx,
         signed_transaction second_trx,
         bool broadcast = false );



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
      annotated_signed_transaction              custom_operation(
         flat_set< account_name_type > required_auths,
         uint16_t id,
         vector< char > data,
         bool broadcast = false );


      /**
       * Provides a generic way (using JSON String) to add higher level protocols on top of network consensus.
       *
       * @param required_auths Set of account active authorities required for the transaction signature.
       * @param required_posting_auths Set of posting authorities required for the transaction signature.
       * @param id ID of the operation, refers to the plugin used to interpret the operation. Less than 32 characters long.
       * @param json Valid UTF8 / JSON string of operation data.
       * @param broadcast Set True to broadcast transaction.
       */
      annotated_signed_transaction              custom_json_operation(
         flat_set< account_name_type > required_auths,
         uint16_t id,
         vector< char > data,
         bool broadcast = false );

       /**
       * @}
       */

};

struct plain_keys {
   fc::sha512                  checksum;
   map<public_key_type,string> keys;
};

} }     // node::wallet

FC_REFLECT( node::wallet::wallet_data,
            (cipher_keys)
            (ws_server)
            (ws_user)
            (ws_password)
          )

FC_REFLECT( node::wallet::seed_phrase_info, (brain_priv_key)(wif_priv_key) (pub_key))

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
        (suggest_seed_phrase)
        (list_keys)
        (get_private_key)
        (get_private_key_from_password)
        (normalize_seed_phrase)

        // query api

        (info)
        (list_my_accounts)
        (list_accounts)
        (list_producers)
        (get_producer)
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
        (update_producer)
        (set_voting_proxy)
        (vote_producer)
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

        (get_active_producers)
        (get_miner_queue)
        (get_transaction)
      )

FC_REFLECT( node::wallet::memo_data, (from)(to)(nonce)(check)(encrypted) )
