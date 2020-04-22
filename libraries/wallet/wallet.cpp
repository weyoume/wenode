#include <graphene/utilities/git_revision.hpp>
#include <graphene/utilities/key_conversion.hpp>
#include <graphene/utilities/words.hpp>

#include <node/app/api.hpp>
#include <node/protocol/base.hpp>
#include <node/wallet/wallet.hpp>
#include <node/wallet/api_documentation.hpp>
#include <node/wallet/reflect_util.hpp>

#include <node/account_by_key/account_by_key_api.hpp>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <list>

#include <boost/version.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/range/algorithm/sort.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>

#include <fc/container/deque.hpp>
#include <fc/git_revision.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/json.hpp>
#include <fc/io/stdio.hpp>
#include <fc/network/http/websocket.hpp>
#include <fc/rpc/cli.hpp>
#include <fc/rpc/websocket_api.hpp>
#include <fc/crypto/aes.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/thread/mutex.hpp>
#include <fc/thread/scoped_lock.hpp>
#include <fc/smart_ref_impl.hpp>

#ifndef WIN32
# include <sys/types.h>
# include <sys/stat.h>
#endif

#define SEED_PHRASE_WORD_COUNT 12

namespace node { namespace wallet { namespace detail {

template<class T>
optional<T> maybe_id( const string& name_or_id )
{
   if( std::isdigit( name_or_id.front() ) )
   {
      try
      {
         return fc::variant(name_or_id).as<T>();
      }
      catch (const fc::exception&)
      {
      }
   }
   return optional<T>();
}

string pubkey_to_shorthash( const public_key_type& key )
{
   uint32_t x = fc::sha256::hash(key)._hash[0];
   static const char hd[] = "0123456789abcdef";
   string result;

   result += hd[(x >> 0x1c) & 0x0f];
   result += hd[(x >> 0x18) & 0x0f];
   result += hd[(x >> 0x14) & 0x0f];
   result += hd[(x >> 0x10) & 0x0f];
   result += hd[(x >> 0x0c) & 0x0f];
   result += hd[(x >> 0x08) & 0x0f];
   result += hd[(x >> 0x04) & 0x0f];
   result += hd[(x        ) & 0x0f];

   return result;
}

fc::ecc::private_key derive_private_key( 
   const std::string& prefix_string,
   int sequence_number )
{
   std::string sequence_string = std::to_string(sequence_number);
   fc::sha512 h = fc::sha512::hash(prefix_string + " " + sequence_string);
   fc::ecc::private_key derived_key = fc::ecc::private_key::regenerate(fc::sha256::hash(h));
   return derived_key;
}

string normalize_seed_phrase( string s )
{
   size_t i = 0, n = s.length();
   std::string result;
   char c;
   result.reserve( n );

   bool preceded_by_whitespace = false;
   bool non_empty = false;
   while( i < n )
   {
      c = s[i++];
      switch( c )
      {
      case ' ':  case '\t': case '\r': case '\n': case '\v': case '\f':
         preceded_by_whitespace = true;
         continue;

      case 'a': c = 'A'; break;
      case 'b': c = 'B'; break;
      case 'c': c = 'C'; break;
      case 'd': c = 'D'; break;
      case 'e': c = 'E'; break;
      case 'f': c = 'F'; break;
      case 'g': c = 'G'; break;
      case 'h': c = 'H'; break;
      case 'i': c = 'I'; break;
      case 'j': c = 'J'; break;
      case 'k': c = 'K'; break;
      case 'l': c = 'L'; break;
      case 'm': c = 'M'; break;
      case 'n': c = 'N'; break;
      case 'o': c = 'O'; break;
      case 'p': c = 'P'; break;
      case 'q': c = 'Q'; break;
      case 'r': c = 'R'; break;
      case 's': c = 'S'; break;
      case 't': c = 'T'; break;
      case 'u': c = 'U'; break;
      case 'v': c = 'V'; break;
      case 'w': c = 'W'; break;
      case 'x': c = 'X'; break;
      case 'y': c = 'Y'; break;
      case 'z': c = 'Z'; break;

      default:
         break;
      }
      if( preceded_by_whitespace && non_empty )
         result.push_back(' ');
      result.push_back(c);
      preceded_by_whitespace = false;
      non_empty = true;
   }
   return result;
}

struct op_prototype_visitor
{
   typedef void result_type;

   int t = 0;
   flat_map< std::string, operation >& name2op;

   op_prototype_visitor(
      int _t,
      flat_map< std::string, operation >& _prototype_ops
      ):t(_t), name2op(_prototype_ops) {}

   template<typename Type>
   result_type operator()( const Type& op )const
   {
      string name = fc::get_typename<Type>::name();
      size_t p = name.rfind(':');
      if( p != string::npos )
         name = name.substr( p+1 );
      name2op[ name ] = Type();
   }
};

class wallet_api_impl
{
   public:
      api_documentation method_documentation;
   private:
      void enable_umask_protection() {
#ifdef __unix__
         _old_umask = umask( S_IRWXG | S_IRWXO );
#endif
      }

      void disable_umask_protection() {
#ifdef __unix__
         umask( _old_umask );
#endif
      }

      void init_prototype_ops()
      {
         operation op;
         for( int t=0; t<op.count(); t++ )
         {
            op.set_which( t );
            op.visit( op_prototype_visitor(t, _prototype_ops) );
         }
         return;
      }

public:
   wallet_api& self;
   wallet_api_impl( wallet_api& s, const wallet_data& initial_data, fc::api<login_api> rapi )
      : self( s ),
        _remote_api( rapi ),
        _remote_db( rapi->get_api_by_name("database_api")->as< database_api >() ),
        _remote_net_broadcast( rapi->get_api_by_name("network_broadcast_api")->as< network_broadcast_api >() )
   {
      init_prototype_ops();

      _wallet.ws_server = initial_data.ws_server;
      _wallet.ws_user = initial_data.ws_user;
      _wallet.ws_password = initial_data.ws_password;
   }
   virtual ~wallet_api_impl()
   {}

   void encrypt_keys()
   {
      if( !is_locked() )
      {
         plain_keys data;
         data.keys = _keys;
         data.checksum = _checksum;
         auto plain_txt = fc::raw::pack(data);
         _wallet.cipher_keys = fc::aes_encrypt( data.checksum, plain_txt );
      }
   }

   bool copy_wallet_file( string destination_filename )
   {
      fc::path src_path = get_wallet_filename();
      if( !fc::exists( src_path ) )
         return false;
      fc::path dest_path = destination_filename + _wallet_filename_extension;
      int suffix = 0;
      while( fc::exists(dest_path) )
      {
         ++suffix;
         dest_path = destination_filename + "-" + std::to_string( suffix ) + _wallet_filename_extension;
      }
      wlog( "backing up wallet ${src} to ${dest}",
            ("src", src_path)
            ("dest", dest_path) );

      fc::path dest_parent = fc::absolute(dest_path).parent_path();
      try
      {
         enable_umask_protection();
         if( !fc::exists( dest_parent ) )
            fc::create_directories( dest_parent );
         fc::copy( src_path, dest_path );
         disable_umask_protection();
      }
      catch(...)
      {
         disable_umask_protection();
         throw;
      }
      return true;
   }

   bool is_locked()const
   {
      return _checksum == fc::sha512();
   }

   variant info() const
   {
      auto dynamic_props = _remote_db->get_dynamic_global_properties();
      fc::mutable_variant_object result(fc::variant(dynamic_props).get_object());
      result["chain_id"] = CHAIN_ID;
      result["producer_majority_version"] = fc::string( _remote_db->get_producer_schedule().majority_version );
      result["hardfork_version"] = fc::string( _remote_db->get_hardfork_version() );
      result["head_block_num"] = dynamic_props.head_block_number;
      result["head_block_id"] = dynamic_props.head_block_id;
      result["head_block_age"] = fc::get_approximate_relative_time_string( dynamic_props.time, time_point(time_point::now()), " old" );
      result["participation"] = (100*dynamic_props.recent_slots_filled.popcount()) / 128.0;
      result["usd_price"] = dynamic_props.current_median_usd_price;
      result["equity_price"] = dynamic_props.current_median_equity_price;
      result["account_creation_fee"] = _remote_db->get_median_chain_properties().account_creation_fee;
      return result;
   }

   variant_object about() const
   {
      string client_version( graphene::utilities::git_revision_description );
      const size_t pos = client_version.find( '/' );
      if( pos != string::npos && client_version.size() > pos )
         client_version = client_version.substr( pos + 1 );

      fc::mutable_variant_object result;
      result["blockchain_version"]       = BLOCKCHAIN_VERSION;
      result["client_version"]           = client_version;
      result["node_revision"]            = graphene::utilities::git_revision_sha;
      result["node_revision_age"]        = fc::get_approximate_relative_time_string( time_point( fc::seconds( graphene::utilities::git_revision_unix_timestamp ) ) );
      result["fc_revision"]              = fc::git_revision_sha;
      result["fc_revision_age"]          = fc::get_approximate_relative_time_string( time_point( fc::seconds( fc::git_revision_unix_timestamp ) ) );
      result["compile_date"]             = "compiled on " __DATE__ " at " __TIME__;
      result["boost_version"]            = boost::replace_all_copy(std::string(BOOST_LIB_VERSION), "_", ".");
      result["openssl_version"]          = OPENSSL_VERSION_TEXT;

      std::string bitness = boost::lexical_cast<std::string>(8 * sizeof(int*)) + "-bit";
#if defined(__APPLE__)
      std::string os = "osx";
#elif defined(__linux__)
      std::string os = "linux";
#elif defined(_MSC_VER)
      std::string os = "win32";
#else
      std::string os = "other";
#endif
      result["build"] = os + " " + bitness;

      try
      {
         auto v = _remote_api->get_version();
         result["server_blockchain_version"] = v.blockchain_version;
         result["server_node_revision"] = v.node_revision;
         result["server_fc_revision"] = v.fc_revision;
      }
      catch( fc::exception& )
      {
         result["server"] = "could not retrieve server version information";
      }

      return result;
   }

   string get_wallet_filename() const { return _wallet_filename; }

   optional<fc::ecc::private_key>    try_get_private_key(const public_key_type& id)const
   {
      auto it = _keys.find(id);
      if( it != _keys.end() )
         return wif_to_key( it->second );
      return optional<fc::ecc::private_key>();
   }

   fc::ecc::private_key              get_private_key(const public_key_type& id)const
   {
      auto has_key = try_get_private_key( id );
      FC_ASSERT( has_key );
      return *has_key;
   }


   fc::ecc::private_key              get_private_key_for_account( const account_api_obj& account )const
   {
      vector<public_key_type> active_keys = account.active_auth.get_keys();
      if (active_keys.size() != 1)
         FC_THROW("Expecting a simple authority with one active key");
      return get_private_key(active_keys.front());
   }

   /**
    * Imports the private key into the wallet,
    * and associate it with the given account name.
    * 
    * @returns True if the key matches a current key for an account, 
    * false otherwise (but it is stored either way)
   */
   bool import_key( string wif_key )
   {
      fc::optional<fc::ecc::private_key> optional_private_key = wif_to_key( wif_key );
      if( !optional_private_key )
      {
         FC_THROW( "Invalid private key" );
      }
         
      node::chain::public_key_type wif_pub_key = optional_private_key->get_public_key();

      _keys[wif_pub_key] = wif_key;
      return true;
   }

   bool load_wallet_file(string wallet_filename = "")
   {
      if( wallet_filename == "" )
         wallet_filename = _wallet_filename;

      if( ! fc::exists( wallet_filename ) )
         return false;

      _wallet = fc::json::from_file( wallet_filename ).as< wallet_data >();

      return true;
   }

   /**
    * Serialize in memory, then save to disk.
    * This approach lessens the risk of a partially written wallet.
    * if exceptions are thrown in serialization.
    */
   void save_wallet_file( string wallet_filename = "" )
   {
      encrypt_keys();

      if( wallet_filename == "" )
      {
         wallet_filename = _wallet_filename;
      }
         
      wlog( "Saving wallet to file ${fn}", ("fn", wallet_filename) );

      string data = fc::json::to_pretty_string( _wallet );
      try
      {
         enable_umask_protection();
         //
         // Parentheses on the following declaration fails to compile,
         // due to the Most Vexing Parse.  Thanks, C++
         //
         // http://en.wikipedia.org/wiki/Most_vexing_parse
         //
         fc::ofstream outfile{ fc::path( wallet_filename ) };
         outfile.write( data.c_str(), data.length() );
         outfile.flush();
         outfile.close();
         disable_umask_protection();
      }
      catch(...)
      {
         disable_umask_protection();
         throw;
      }
   }

   /**
    * This function generates derived keys starting with index 0 and keeps incrementing
    * the index until it finds a key that isn't registered in the blockchain.  To be
    * safer, it continues checking for a few more keys to make sure there wasn't a short gap
    * caused by a failed registration or the like.
    */
   int find_first_unused_derived_key_index( const fc::ecc::private_key& parent_key )
   {
      int first_unused_index = 0;
      int number_of_consecutive_unused_keys = 0;

      for (int key_index = 0; ; ++key_index)
      {
         fc::ecc::private_key derived_private_key = derive_private_key( key_to_wif( parent_key ), key_index );
         node::chain::public_key_type derived_public_key = derived_private_key.get_public_key();
         if( _keys.find(derived_public_key) == _keys.end() )
         {
            if (number_of_consecutive_unused_keys)
            {
               ++number_of_consecutive_unused_keys;
               if (number_of_consecutive_unused_keys > 5)
                  return first_unused_index;
            }
            else
            {
               first_unused_index = key_index;
               number_of_consecutive_unused_keys = 1;
            }
         }
         else
         {
            // key_index is used
            first_unused_index = 0;
            number_of_consecutive_unused_keys = 0;
         }
      }
   }

   void set_transaction_expiration( uint32_t tx_expiration_seconds )
   {
      FC_ASSERT( tx_expiration_seconds < MAX_TIME_UNTIL_EXPIRATION );
      _tx_expiration_seconds = tx_expiration_seconds;
   }

   annotated_signed_transaction sign_transaction( signed_transaction tx, bool broadcast )
   {
      flat_set< account_name_type >   req_active_approvals;
      flat_set< account_name_type >   req_owner_approvals;
      flat_set< account_name_type >   req_posting_approvals;
      vector< authority >  other_auths;

      tx.get_required_authorities( req_active_approvals, req_owner_approvals, req_posting_approvals, other_auths );

      for( const auto& auth : other_auths )
      {
         for( const auto& a : auth.account_auths )
         {
            req_active_approvals.insert( a.first );
         }
            
      }

      // std::merge lets us de-duplicate account_id's that occur in both
      //   sets, and dump them into a vector (as required by remote_db api)
      //   at the same time
      vector< string > v_approving_account_names;

      std::merge(req_active_approvals.begin(), req_active_approvals.end(),
                 req_owner_approvals.begin() , req_owner_approvals.end(),
                 std::back_inserter( v_approving_account_names ) );

      for( const auto& a : req_posting_approvals )
      {
         v_approving_account_names.push_back(a);
      }

      auto approving_account_objects = _remote_db->get_accounts( v_approving_account_names );

      FC_ASSERT( approving_account_objects.size() == v_approving_account_names.size(), 
         "", ("aco.size:", approving_account_objects.size())("acn",v_approving_account_names.size()) );

      flat_map< string, account_api_obj > approving_account_lut;
      size_t i = 0;

      for( const optional< account_api_obj >& approving_acct : approving_account_objects )
      {
         if( !approving_acct.valid() )
         {
            wlog( "operation_get_required_auths said approval of non-existing account ${name} was needed",
                  ("name", v_approving_account_names[i]) );
            i++;
            continue;
         }
         approving_account_lut[ approving_acct->name ] =  *approving_acct;
         i++;
      }

      auto get_account_from_lut = [&]( const std::string& name ) -> const account_api_obj&
      {
         auto it = approving_account_lut.find( name );
         FC_ASSERT( it != approving_account_lut.end() );
         return it->second;
      };

      flat_set<public_key_type> approving_key_set;

      for( account_name_type& acct_name : req_active_approvals )
      {
         const auto it = approving_account_lut.find( acct_name );
         if( it == approving_account_lut.end() )
            continue;
         const account_api_obj& acct = it->second;
         vector<public_key_type> v_approving_keys = acct.active_auth.get_keys();
         wdump((v_approving_keys));
         for( const public_key_type& approving_key : v_approving_keys )
         {
            wdump((approving_key));
            approving_key_set.insert( approving_key );
         }
      }

      for( account_name_type& acct_name : req_posting_approvals )
      {
         const auto it = approving_account_lut.find( acct_name );
         if( it == approving_account_lut.end() )
            continue;
         const account_api_obj& acct = it->second;
         vector<public_key_type> v_approving_keys = acct.posting_auth.get_keys();
         wdump((v_approving_keys));
         for( const public_key_type& approving_key : v_approving_keys )
         {
            wdump((approving_key));
            approving_key_set.insert( approving_key );
         }
      }

      for( const account_name_type& acct_name : req_owner_approvals )
      {
         const auto it = approving_account_lut.find( acct_name );
         if( it == approving_account_lut.end() )
            continue;
         const account_api_obj& acct = it->second;
         vector<public_key_type> v_approving_keys = acct.owner_auth.get_keys();
         for( const public_key_type& approving_key : v_approving_keys )
         {
            wdump((approving_key));
            approving_key_set.insert( approving_key );
         }
      }
      for( const authority& a : other_auths )
      {
         for( const auto& k : a.key_auths )
         {
            wdump((k.first));
            approving_key_set.insert( k.first );
         }
      }

      auto dyn_props = _remote_db->get_dynamic_global_properties();
      tx.set_reference_block( dyn_props.head_block_id );
      tx.set_expiration( dyn_props.time + fc::seconds(_tx_expiration_seconds) );
      tx.signatures.clear();

      //idump((_keys));
      flat_set< public_key_type > available_keys;
      flat_map< public_key_type, fc::ecc::private_key > available_private_keys;
      for( const public_key_type& key : approving_key_set )
      {
         auto it = _keys.find(key);
         if( it != _keys.end() )
         {
            fc::optional<fc::ecc::private_key> privkey = wif_to_key( it->second );
            FC_ASSERT( privkey.valid(), "Malformed private key in _keys" );
            available_keys.insert(key);
            available_private_keys[key] = *privkey;
         }
      }

      auto minimal_signing_keys = tx.minimize_required_signatures(
         CHAIN_ID,
         available_keys,
         [&]( const string& account_name ) -> const authority&
         { return (get_account_from_lut( account_name ).active_auth); },
         [&]( const string& account_name ) -> const authority&
         { return (get_account_from_lut( account_name ).owner_auth); },
         [&]( const string& account_name ) -> const authority&
         { return (get_account_from_lut( account_name ).posting_auth); },
         MAX_SIG_CHECK_DEPTH
         );

      for( const public_key_type& k : minimal_signing_keys )
      {
         auto it = available_private_keys.find(k);
         FC_ASSERT( it != available_private_keys.end() );
         tx.sign( it->second, CHAIN_ID );
      }

      if( broadcast ) 
      {
         try 
         {
            auto result = _remote_net_broadcast->broadcast_transaction_synchronous( tx );
            annotated_signed_transaction rtrx(tx);
            rtrx.block_num = result.get_object()["block_num"].as_uint64();
            rtrx.transaction_num = result.get_object()["trx_num"].as_uint64();
            return rtrx;
         }
         catch (const fc::exception& e)
         {
            elog("Caught exception while broadcasting tx ${id}:  ${e}", ("id", tx.id().str())("e", e.to_detail_string()) );
            throw;
         }
      }
      return tx;
   }

   std::map<string,std::function<string(fc::variant,const fc::variants&)>>       get_result_formatters() const
   {
      std::map<string,std::function<string(fc::variant,const fc::variants&)> > m;

      m["help"] = [](variant result, const fc::variants& a)
      {
         return result.get_string();
      };

      m["gethelp"] = [](variant result, const fc::variants& a)
      {
         return result.get_string();
      };

      return m;
   }

   void use_network_node_api()
   {
      if( _remote_net_node )
         return;
      try
      {
         _remote_net_node = _remote_api->get_api_by_name("network_node_api")->as< network_node_api >();
      }
      catch( const fc::exception& e )
      {
         elog( "Couldn't get network node API" );
         throw(e);
      }
   }

   void use_remote_account_by_key_api()
   {
      if( _remote_account_by_key_api.valid() )
         return;

      try{ _remote_account_by_key_api = _remote_api->get_api_by_name( "account_by_key_api" )->as< account_by_key::account_by_key_api >(); }
      catch( const fc::exception& e ) { elog( "Couldn't get account_by_key API" ); throw(e); }
   }

   void network_add_nodes( const vector<string>& nodes )
   {
      use_network_node_api();
      for( const string& node_address : nodes )
      {
         (*_remote_net_node)->add_node( fc::ip::endpoint::from_string( node_address ) );
      }
   }

   vector< variant > network_get_connected_peers()
   {
      use_network_node_api();
      const auto peers = (*_remote_net_node)->get_connected_peers();
      vector< variant > result;
      result.reserve( peers.size() );
      for( const auto& peer : peers )
      {
         variant v;
         fc::to_variant( peer, v );
         result.push_back( v );
      }
      return result;
   }

   operation get_prototype_operation( string operation_name )
   {
      auto it = _prototype_ops.find( operation_name );
      if( it == _prototype_ops.end() )
         FC_THROW("Unsupported operation: \"${operation_name}\"", ("operation_name", operation_name));
      return it->second;
   }

   string                                  _wallet_filename;
   wallet_data                             _wallet;

   map<public_key_type,string>             _keys;
   fc::sha512                              _checksum;
   fc::api<login_api>                      _remote_api;
   fc::api<database_api>                   _remote_db;
   fc::api<network_broadcast_api>          _remote_net_broadcast;
   optional< fc::api<network_node_api> >   _remote_net_node;
   optional< fc::api<account_by_key::account_by_key_api> > _remote_account_by_key_api;
   uint32_t                                _tx_expiration_seconds = 30;

   flat_map<string, operation>             _prototype_ops;

   static_variant_map _operation_which_map = create_static_variant_map< operation >();

#ifdef __unix__
   mode_t                  _old_umask;
#endif
   const string _wallet_filename_extension = ".wallet";
};

} } } // node::wallet::detail



namespace node { namespace wallet {

wallet_api::wallet_api( const wallet_data& initial_data, fc::api< login_api > rapi )
   : my( new detail::wallet_api_impl( *this, initial_data, rapi ) ) {}

wallet_api::~wallet_api(){}



      //====================//
      // === Wallet API === //
      //====================//


string                            wallet_api::help()const
{
   std::vector<std::string> method_names = my->method_documentation.get_method_names();
   std::stringstream ss;
   for (const std::string method_name : method_names)
   {
      try
      {
         ss << my->method_documentation.get_brief_description(method_name);
      }
      catch (const fc::key_not_found_exception&)
      {
         ss << method_name << " (no help available)\n";
      }
   }
   return ss.str();
}


variant                           wallet_api::info()
{
   return my->info();
}


variant_object                    wallet_api::about() const
{
   return my->about();
}


vector< string >                  wallet_api::list_my_accounts()
{
   FC_ASSERT( !is_locked(), 
      "Wallet must be unlocked to list accounts" );

   vector< string > results;

   try
   {
      my->use_remote_account_by_key_api();
   }
   catch( fc::exception& e )
   {
      elog( "Connected node needs to enable account_by_key_api" );
      return results;
   }

   vector< public_key_type > pub_keys;
   pub_keys.reserve( my->_keys.size() );

   for( const auto& item : my->_keys )
   {
      pub_keys.push_back( item.first );
   }
      
   auto refs = ( *my->_remote_account_by_key_api )->get_key_references( pub_keys );
   set< string > names;
   for( const auto& item : refs )
   {
      for( const auto& name : item )
      {
         names.insert( name );    // Add to set to de-duplicte names from private keys
      }   
   }

   results.reserve( names.size() );

   for( const auto& name : names )
   {
      results.emplace_back( name );
   } 

   return results;
}


vector< extended_account >        wallet_api::get_my_accounts()
{
   FC_ASSERT( !is_locked(), 
      "Wallet must be unlocked to list accounts" );

   try
   {
      my->use_remote_account_by_key_api();
   }
   catch( fc::exception& e )
   {
      elog( "Connected node needs to enable account_by_key_api" );
   }

   return get_full_accounts( list_my_accounts() );
}


string                            wallet_api::get_wallet_filename() const
{
   return my->get_wallet_filename();
}


bool                              wallet_api::is_new()const
{
   return my->_wallet.cipher_keys.size() == 0;
}


bool                              wallet_api::is_locked()const
{
   return my->is_locked();
}


void                              wallet_api::lock()
{ try {
   FC_ASSERT( !is_locked() );

   encrypt_keys();

   for( auto key : my->_keys )
   {
      key.second = key_to_wif(fc::ecc::private_key());
   }
      
   my->_keys.clear();
   my->_checksum = fc::sha512();
   my->self.lock_changed(true);

} FC_CAPTURE_AND_RETHROW() }


void                              wallet_api::unlock( string password )
{ try {
   FC_ASSERT( password.size() > 0 );

   auto pw = fc::sha512::hash( password.c_str(), password.size() );
   vector< char > decrypted = fc::aes_decrypt( pw, my->_wallet.cipher_keys );
   auto pk = fc::raw::unpack< plain_keys >( decrypted );

   FC_ASSERT( pk.checksum == pw );

   my->_keys = std::move( pk.keys );
   my->_checksum = pk.checksum;
   my->self.lock_changed( false );

} FC_CAPTURE_AND_RETHROW() }


void                              wallet_api::set_password( string password )
{
   if( !is_new() )
   {
      FC_ASSERT( !is_locked(), 
         "The wallet must be unlocked before the password can be set" );
   }
      
   my->_checksum = fc::sha512::hash( password.c_str(), password.size() );
   lock();
}


string                            wallet_api::gethelp( const string& method )const
{
   fc::api< wallet_api > tmp;
   std::stringstream ss;
   ss << "\n";

   std::string doxygenHelpString = my->method_documentation.get_detailed_description( method );
   if (!doxygenHelpString.empty())
   {
      ss << doxygenHelpString;
   } 
   else
   {
      ss << "No help defined for method " << method << "\n";
   }

   return ss.str();
}


bool                              wallet_api::load_wallet_file( string wallet_filename )
{
   return my->load_wallet_file( wallet_filename );
}


void                              wallet_api::save_wallet_file( string wallet_filename )
{
   my->save_wallet_file( wallet_filename );
}


void                              wallet_api::set_wallet_filename( string wallet_filename ) 
{ 
   my->_wallet_filename = wallet_filename; 
}


string                            wallet_api::serialize_transaction( signed_transaction tx )const
{
   return fc::to_hex( fc::raw::pack( tx ) );
}


bool                              wallet_api::copy_wallet_file( string destination_filename )
{
   return my->copy_wallet_file( destination_filename );
}


void                              wallet_api::set_transaction_expiration( uint32_t seconds )
{
   my->set_transaction_expiration(seconds);
}


void                              wallet_api::check_memo( const string& memo, const account_api_obj& account )const
{
   vector< public_key_type > keys;
      
   try
   {
      keys.push_back( fc::ecc::extended_private_key::from_base58( memo ).get_public_key() );   // Check if memo is a private key
   }
   catch( fc::parse_error_exception& ) {}
   catch( fc::assert_exception& ) {}

   // Get possible keys if memo was an account password.
   string owner_seed = account.name + "owner" + memo;
   auto owner_secret = fc::sha256::hash( owner_seed.c_str(), owner_seed.size() );
   keys.push_back( fc::ecc::private_key::regenerate( owner_secret ).get_public_key() );

   string active_seed = account.name + "active" + memo;
   auto active_secret = fc::sha256::hash( active_seed.c_str(), active_seed.size() );
   keys.push_back( fc::ecc::private_key::regenerate( active_secret ).get_public_key() );

   string posting_seed = account.name + "posting" + memo;
   auto posting_secret = fc::sha256::hash( posting_seed.c_str(), posting_seed.size() );
   keys.push_back( fc::ecc::private_key::regenerate( posting_secret ).get_public_key() );

   string secure_seed = account.name + "secure" + memo;
   auto secure_secret = fc::sha256::hash( secure_seed.c_str(), secure_seed.size() );
   keys.push_back( fc::ecc::private_key::regenerate( secure_secret ).get_public_key() );

   string connection_seed = account.name + "connection" + memo;
   auto connection_secret = fc::sha256::hash( connection_seed.c_str(), connection_seed.size() );
   keys.push_back( fc::ecc::private_key::regenerate( connection_secret ).get_public_key() );

   string friend_seed = account.name + "friend" + memo;
   auto friend_secret = fc::sha256::hash( friend_seed.c_str(), friend_seed.size() );
   keys.push_back( fc::ecc::private_key::regenerate( friend_secret ).get_public_key() );

   string companion_seed = account.name + "companion" + memo;
   auto companion_secret = fc::sha256::hash( companion_seed.c_str(), companion_seed.size() );
   keys.push_back( fc::ecc::private_key::regenerate( companion_secret ).get_public_key() );

   // Check keys against public keys in authorites.
   for( auto& key_weight_pair : account.owner_auth.key_auths )
   {
      for( auto& key : keys )
      {
         FC_ASSERT( key_weight_pair.first != key,
            "Detected private key in memo. Cancelling transaction." );
      }
   }

   for( auto& key_weight_pair : account.active_auth.key_auths )
   {
      for( auto& key : keys )
      {
         FC_ASSERT( key_weight_pair.first != key,
            "Detected private key in memo. Cancelling transaction." );
      }
   }

   for( auto& key_weight_pair : account.posting_auth.key_auths )
   {
      for( auto& key : keys )
      {
         FC_ASSERT( key_weight_pair.first != key,
            "Detected private key in memo. Cancelling transaction." );
      }
   }

   const auto& secure_public_key = account.secure_public_key;
   for( auto& key : keys )
   {
      FC_ASSERT( secure_public_key != key,
         "Detected private key in memo. Cancelling transaction." );
   }

   const auto& connection_public_key = account.connection_public_key;
   for( auto& key : keys )
   {
      FC_ASSERT( connection_public_key != key,
         "Detected private key in memo. Cancelling transaction." );
   }

   const auto& friend_public_key = account.friend_public_key;
   for( auto& key : keys )
   {
      FC_ASSERT( friend_public_key != key,
         "Detected private key in memo. Cancelling transaction." );
   }

   const auto& companion_public_key = account.companion_public_key;
   for( auto& key : keys )
   {
      FC_ASSERT( companion_public_key != key,
         "Detected private key in memo. Cancelling transaction." );
   }
      
   // Check against imported keys.
   for( auto& key_pair : my->_keys )
   {
      for( auto& key : keys )
      {
         FC_ASSERT( key != key_pair.first,
            "Detected private key in memo. Cancelling trasanction." );
      }
   }
}


string                            wallet_api::get_encrypted_message(
   string from_public_key,
   string to_public_key,
   string message ) const
{
   if( message.size() > 0 && message[0] == '#' )
   {
      encrypted_message_data m;

      m.from = public_key_type( from_public_key );

      m.to = public_key_type( to_public_key );

      m.nonce = fc::time_point::now().time_since_epoch().count();

      auto from_priv = my->get_private_key( m.from );
      auto shared_secret = from_priv.get_shared_secret( m.to );

      fc::sha512::encoder enc;
      fc::raw::pack( enc, m.nonce );
      fc::raw::pack( enc, shared_secret );
      auto encrypt_key = enc.result();

      m.encrypted = fc::aes_encrypt( encrypt_key, fc::raw::pack( message.substr(1) ) );

      m.check = fc::sha256::hash( encrypt_key )._hash[0];

      return m;
   }
   else 
   {
      return message;
   }
}


string                            wallet_api::get_decrypted_message( string encrypted_message ) const
{
   if( is_locked() )
   {
      return encrypted_message;
   } 

   if( encrypted_message.size() && encrypted_message[0] == '#' )
   {
      auto m = encrypted_message_data::from_string( encrypted_message );
      if( m )
      {
         fc::sha512 shared_secret;
         auto from_key = my->try_get_private_key( m->from );
         if( !from_key )
         {
            auto to_key = my->try_get_private_key( m->to );
            if( !to_key )
            {
               return encrypted_message;
            } 
            shared_secret = to_key->get_shared_secret( m->from );
         } 
         else 
         {
            shared_secret = from_key->get_shared_secret( m->to );
         }

         fc::sha512::encoder enc;
         fc::raw::pack( enc, m->nonce );
         fc::raw::pack( enc, shared_secret );
         auto secure_public_key = enc.result();

         uint32_t check = fc::sha256::hash( secure_public_key )._hash[0];
         if( check != m->check )
         {
            return encrypted_message;
         }

         try 
         {
            vector<char> decrypted = fc::aes_decrypt( secure_public_key, m->encrypted );
            return fc::raw::unpack<std::string>( decrypted );

         } catch ( ... ){}
      }
   }
   return encrypted_message;
}


fc::ecc::private_key              wallet_api::derive_private_key( const std::string& prefix_string, int sequence_number ) const
{
   return detail::derive_private_key( prefix_string, sequence_number );
}


std::map<string,std::function<string(fc::variant,const fc::variants&)> >    wallet_api::get_result_formatters() const
{
   return my->get_result_formatters();
}

      //=================//
      // === Key API === //
      //=================//



string                            wallet_api::get_private_key( public_key_type pubkey )const
{
   return key_to_wif( my->get_private_key( pubkey ) );
}


pair< public_key_type, string >   wallet_api::get_private_key_from_password(
   string account, 
   string role, 
   string password )const 
{
   auto seed = account + role + password;
   FC_ASSERT( seed.size() );
   auto secret = fc::sha256::hash( seed.c_str(), seed.size() );
   auto priv = fc::ecc::private_key::regenerate( secret );

   return std::make_pair( public_key_type( priv.get_public_key() ), key_to_wif( priv ) );
}


seed_phrase_info                  wallet_api::suggest_seed_phrase()const
{
   seed_phrase_info result;
   // create a private key for secure entropy
   fc::sha256 sha_entropy1 = fc::ecc::private_key::generate().get_secret();
   fc::sha256 sha_entropy2 = fc::ecc::private_key::generate().get_secret();
   fc::bigint entropy1( sha_entropy1.data(), sha_entropy1.data_size() );
   fc::bigint entropy2( sha_entropy2.data(), sha_entropy2.data_size() );
   fc::bigint entropy(entropy1);
   entropy <<= 8*sha_entropy1.data_size();
   entropy += entropy2;
   string seed_phrase = "";

   for( int i = 0; i < SEED_PHRASE_WORD_COUNT; i++ )
   {
      fc::bigint choice = entropy % graphene::words::word_list_size;
      entropy /= graphene::words::word_list_size;
      if( i > 0 )
      {
         seed_phrase += " ";
      } 
      seed_phrase += graphene::words::word_list[ choice.to_int64() ];
   }

   seed_phrase = normalize_seed_phrase(seed_phrase);
   fc::ecc::private_key priv_key = detail::derive_private_key( seed_phrase, 0 );
   result.brain_priv_key = seed_phrase;
   result.wif_priv_key = key_to_wif( priv_key );
   result.pub_key = priv_key.get_public_key();
   return result;
}


string                            wallet_api::normalize_seed_phrase( string s ) const
{
   return detail::normalize_seed_phrase( s );
}


map< public_key_type, string >    wallet_api::list_keys()
{
   FC_ASSERT(!is_locked());
   return my->_keys;
}


bool                              wallet_api::import_key( string wif_key )
{
   FC_ASSERT( !is_locked() );
   
   fc::optional<fc::ecc::private_key> optional_private_key = wif_to_key( wif_key );

   if( !optional_private_key )
   {
      FC_THROW( "Invalid private key" );
   }

   if( my->import_key( wif_key ) )
   {
      save_wallet_file();
 
      return true;
   }
   return false;
}


void                              wallet_api::encrypt_keys()
{
   my->encrypt_keys();
}



      //====================//
      // === Global API === //
      //====================//



fc::variant_object                wallet_api::get_config()const
{
   return my->_remote_db->get_config();
}


dynamic_global_property_api_obj   wallet_api::get_dynamic_global_properties() const
{
   return my->_remote_db->get_dynamic_global_properties();
}


median_chain_property_api_obj     wallet_api::get_median_chain_properties() const
{
   return my->_remote_db->get_median_chain_properties();
}


producer_schedule_api_obj         wallet_api::get_producer_schedule() const
{
   return my->_remote_db->get_producer_schedule();
}


hardfork_version                  wallet_api::get_hardfork_version() const
{
   return my->_remote_db->get_hardfork_version();
}


scheduled_hardfork                wallet_api::get_next_scheduled_hardfork() const
{
   return my->_remote_db->get_next_scheduled_hardfork();
}

      //======================//
      // === Account API ==== //
      //======================//


account_api_obj                   wallet_api::get_account( string name ) const
{
   vector< account_api_obj > accounts = my->_remote_db->get_accounts( { name } );
   FC_ASSERT( !accounts.empty(), "Unknown account" );
   return accounts.front();
}

vector< account_api_obj >         wallet_api::get_accounts( vector< string > names ) const
{
   return my->_remote_db->get_accounts( names );
}


vector< account_api_obj >         wallet_api::get_accounts_by_followers( string from, uint32_t limit ) const
{
   return my->_remote_db->get_accounts_by_followers( from, limit );
}


vector< account_concise_api_obj > wallet_api::get_concise_accounts( vector< string > names ) const
{
   return my->_remote_db->get_concise_accounts( names );
}


vector< extended_account >        wallet_api::get_full_accounts( vector< string > names ) const
{
   return my->_remote_db->get_full_accounts( names );
}


map< uint32_t, applied_operation >   wallet_api::get_account_history( string account, uint32_t from, uint32_t limit ) 
{
   map< uint32_t, applied_operation > results = my->_remote_db->get_account_history( account, from, limit );

   if( !is_locked() ) 
   {
      for( auto& item : results ) 
      {
         if( item.second.op.which() == operation::tag<transfer_operation>::value ) 
         {
            auto& top = item.second.op.get<transfer_operation>();
            top.memo = get_decrypted_message( top.memo );
         }
         else if( item.second.op.which() == operation::tag<transfer_request_operation>::value ) 
         {
            auto& top = item.second.op.get<transfer_request_operation>();
            top.memo = get_decrypted_message( top.memo );
         }
         else if( item.second.op.which() == operation::tag<transfer_recurring_operation>::value ) 
         {
            auto& top = item.second.op.get<transfer_recurring_operation>();
            top.memo = get_decrypted_message( top.memo );
         }
         else if( item.second.op.which() == operation::tag<transfer_recurring_request_operation>::value ) 
         {
            auto& top = item.second.op.get<transfer_recurring_request_operation>();
            top.memo = get_decrypted_message( top.memo );
         }
         else if( item.second.op.which() == operation::tag<transfer_to_savings_operation>::value ) 
         {
            auto& top = item.second.op.get<transfer_to_savings_operation>();
            top.memo = get_decrypted_message( top.memo );
         }
         else if( item.second.op.which() == operation::tag<transfer_from_savings_operation>::value ) 
         {
            auto& top = item.second.op.get<transfer_from_savings_operation>();
            top.memo = get_decrypted_message( top.memo );
         }
         else if( item.second.op.which() == operation::tag<escrow_transfer_operation>::value ) 
         {
            auto& top = item.second.op.get<escrow_transfer_operation>();
            top.memo = get_decrypted_message( top.memo );
         }
         else if( item.second.op.which() == operation::tag<asset_issue_operation>::value ) 
         {
            auto& top = item.second.op.get<asset_issue_operation>();
            top.memo = get_decrypted_message( top.memo );
         }
      }
   }
   return results;
}


vector< message_state >           wallet_api::get_messages( vector< string > names ) const
{
   vector< message_state > results = my->_remote_db->get_messages( names );
   for( auto& item : results )
   {
      for( auto& m : item.inbox )
      {
         m.message = get_decrypted_message( m.message );
      }
      for( auto& m : item.outbox )
      {
         m.message = get_decrypted_message( m.message );
      }
      for( auto& n : item.conversations )
      {
         for( auto& m : n.second )
         {
            m.message = get_decrypted_message( m.message );
         }
      }
   }
   return results;
}


vector< balance_state >           wallet_api::get_balances( vector< string > names ) const
{
   return my->_remote_db->get_balances( names );
}


vector< confidential_balance_api_obj > wallet_api::get_confidential_balances( const confidential_query& query ) const
{
   return my->_remote_db->get_confidential_balances( query );
}


vector< key_state >               wallet_api::get_keychains( vector< string > names ) const
{
   vector< key_state > results = my->_remote_db->get_keychains( names );

   for( auto& item : results )
   {
      for( auto& m : item.connection_keys )
      {
         m.second.encrypted_private_key = get_decrypted_message( m.second.encrypted_private_key );
      }
      for( auto& m : item.friend_keys )
      {
         m.second.encrypted_private_key = get_decrypted_message( m.second.encrypted_private_key );
      }
      for( auto& m : item.companion_keys )
      {
         m.second.encrypted_private_key = get_decrypted_message( m.second.encrypted_private_key );
      }
      for( auto& m : item.community_keys )
      {
         m.second.encrypted_private_key = get_decrypted_message( m.second.encrypted_private_key );
      }
      for( auto& m : item.business_keys )
      {
         m.second.encrypted_private_key = get_decrypted_message( m.second.encrypted_private_key );
      }
   }
   return results;
}


set< string >                     wallet_api::lookup_accounts( string lower_bound_name, uint32_t limit )const
{
   return my->_remote_db->lookup_accounts( lower_bound_name, limit );
}


uint64_t                          wallet_api::get_account_count()const
{
   return my->_remote_db->get_account_count();
}


vector< owner_authority_history_api_obj > wallet_api::get_owner_history( string account )const
{
   return my->_remote_db->get_owner_history( account );
}


optional< account_recovery_request_api_obj >  wallet_api::get_recovery_request( string account )const
{
   return my->_remote_db->get_recovery_request( account );
}


optional< account_bandwidth_api_obj >   wallet_api::get_account_bandwidth( string account, producer::bandwidth_type type )const
{
   return my->_remote_db->get_account_bandwidth( account, type );
}



      //===================//
      // === Asset API === //
      //===================//



vector< extended_asset >          wallet_api::get_assets( vector< string > assets )const
{
   return my->_remote_db->get_assets( assets );
}


optional< escrow_api_obj >        wallet_api::get_escrow( string from, string escrow_id )const
{
   return my->_remote_db->get_escrow( from, escrow_id );
}


      //=======================//
      // === Community API === //
      //=======================//


vector< extended_community >          wallet_api::get_communities( vector< string > communities )const
{
   return my->_remote_db->get_communities( communities );
}


vector< extended_community >          wallet_api::get_communities_by_subscribers( string from, uint32_t limit )const
{
   return my->_remote_db->get_communities_by_subscribers( from, limit );
}


      //=====================//
      // === Network API === //
      //=====================//


vector< account_network_state >        wallet_api::get_account_network_state( vector< string > names )const
{
   return my->_remote_db->get_account_network_state( names );
}

vector< account_name_type >            wallet_api::get_active_producers()const
{
   return my->_remote_db->get_active_producers();
}

vector< producer_api_obj >             wallet_api::get_producers_by_voting_power( string from, uint32_t limit )const
{
   return my->_remote_db->get_producers_by_voting_power( from, limit );
}


vector< producer_api_obj >             wallet_api::get_producers_by_mining_power( string from, uint32_t limit )const
{
   return my->_remote_db->get_producers_by_mining_power( from, limit );
}


vector< network_officer_api_obj >      wallet_api::get_development_officers_by_voting_power( string from, uint32_t limit )const
{
   return my->_remote_db->get_development_officers_by_voting_power( from, limit );
}


vector< network_officer_api_obj >      wallet_api::get_marketing_officers_by_voting_power( string from, uint32_t limit )const
{
   return my->_remote_db->get_marketing_officers_by_voting_power( from, limit );
}


vector< network_officer_api_obj >      wallet_api::get_advocacy_officers_by_voting_power( string from, uint32_t limit )const
{
   return my->_remote_db->get_advocacy_officers_by_voting_power( from, limit );
}


vector< executive_board_api_obj >      wallet_api::get_executive_boards_by_voting_power( string from, uint32_t limit )const
{
   return my->_remote_db->get_executive_boards_by_voting_power( from, limit );
}


vector< supernode_api_obj >            wallet_api::get_supernodes_by_view_weight( string from, uint32_t limit )const
{
   return my->_remote_db->get_supernodes_by_view_weight( from, limit );
}


vector< interface_api_obj >            wallet_api::get_interfaces_by_users( string from, uint32_t limit )const
{
   return my->_remote_db->get_interfaces_by_users( from, limit );
}


vector< governance_account_api_obj >   wallet_api::get_governance_accounts_by_subscriber_power( string from, uint32_t limit )const
{
   return my->_remote_db->get_governance_accounts_by_subscriber_power( from, limit );
}


vector< community_enterprise_api_obj >  wallet_api::get_enterprise_by_voting_power( string from, string from_id, uint32_t limit )const
{
   return my->_remote_db->get_enterprise_by_voting_power( from, from_id, limit );
}


      //====================//
      // === Market API === //
      //====================//



vector< order_state >             wallet_api::get_open_orders( vector< string > names )const
{
   return my->_remote_db->get_open_orders( names );
}


market_limit_orders               wallet_api::get_limit_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const
{
   return my->_remote_db->get_limit_orders( buy_symbol, sell_symbol, limit );
}


market_margin_orders              wallet_api::get_margin_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const
{
   return my->_remote_db->get_margin_orders( buy_symbol, sell_symbol, limit );
}


market_option_orders              wallet_api::get_option_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const
{
   return my->_remote_db->get_option_orders( buy_symbol, sell_symbol, limit );
}


market_call_orders                wallet_api::get_call_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const
{
   return my->_remote_db->get_call_orders( buy_symbol, sell_symbol, limit );
}


market_auction_orders             wallet_api::get_auction_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const
{
   return my->_remote_db->get_auction_orders( buy_symbol, sell_symbol, limit );
}


market_credit_loans               wallet_api::get_credit_loans( string buy_symbol, string sell_symbol, uint32_t limit ) const
{
   return my->_remote_db->get_credit_loans( buy_symbol, sell_symbol, limit );
}


vector< credit_pool_api_obj >     wallet_api::get_credit_pools( vector< string > assets )const
{
   return my->_remote_db->get_credit_pools( assets );
}


vector< liquidity_pool_api_obj >  wallet_api::get_liquidity_pools( string buy_symbol, string sell_symbol )const
{
   return my->_remote_db->get_liquidity_pools( buy_symbol, sell_symbol );
}


vector< option_pool_api_obj >     wallet_api::get_option_pools( string buy_symbol, string sell_symbol )const
{
   return my->_remote_db->get_option_pools( buy_symbol, sell_symbol );
}


market_state                      wallet_api::get_market_state( string buy_symbol, string sell_symbol )const
{
   return my->_remote_db->get_market_state( buy_symbol, sell_symbol );
}


      //================//
      // === Ad API === //
      //================//


vector< account_ad_state >        wallet_api::get_account_ads( vector< string > names )const
{
   return my->_remote_db->get_account_ads( names );
}


vector< ad_bid_state >            wallet_api::get_interface_audience_bids( const ad_query& query )const
{
   return my->_remote_db->get_interface_audience_bids( query );
}


      //=====================//
      // === Product API === //
      //=====================//



product_api_obj                   wallet_api::get_product( string seller, string product_id )const
{
   return my->_remote_db->get_product( seller, product_id );
}


vector< account_product_state >   wallet_api::get_account_products( vector< string > names )const
{
   return my->_remote_db->get_account_products( names );
}


      //=====================//
      // === Graph Data  === //
      //=====================//


graph_data_state                     wallet_api::get_graph_query( const graph_query& query )const
{
   return my->_remote_db->get_graph_query( query );
}


vector< graph_node_property_api_obj > wallet_api::get_graph_node_properties( vector< string > names )const
{
   return my->_remote_db->get_graph_node_properties( names );
}


vector< graph_edge_property_api_obj > wallet_api::get_graph_edge_properties( vector< string > names )const
{
   return my->_remote_db->get_graph_edge_properties( names );
}


      //====================//
      // === Search API === //
      //====================//



search_result_state               wallet_api::get_search_query( const search_query& query )const
{
   return my->_remote_db->get_search_query( query );
}


      //=====================================//
      // === Blocks and Transactions API === //
      //=====================================//


optional< signed_block_api_obj >  wallet_api::get_block( uint64_t num )
{
   return my->_remote_db->get_block( num );
}


vector< applied_operation >       wallet_api::get_ops_in_block( uint64_t block_num, bool only_virtual )
{
   return my->_remote_db->get_ops_in_block( block_num, only_virtual );
}


annotated_signed_transaction      wallet_api::get_transaction( transaction_id_type trx_id )const
{
   return my->_remote_db->get_transaction( trx_id );
}


annotated_signed_transaction      wallet_api::sign_transaction( signed_transaction tx, bool broadcast )
{ try {
   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW( (tx) ) }


operation                         wallet_api::get_prototype_operation( string operation_name )
{
   return my->get_prototype_operation( operation_name );
}


void                              wallet_api::network_add_nodes( const vector<string>& nodes )
{
   my->network_add_nodes( nodes );
}


vector< variant >                 wallet_api::network_get_connected_peers()
{
   return my->network_get_connected_peers();
}


transaction_id_type               wallet_api::get_transaction_id( const signed_transaction& trx )const 
{ 
   return trx.id();
}



      //========================//
      // === Post + Tag API === //
      //========================//



vector< vote_state >              wallet_api::get_active_votes( string author, string permlink )const
{
   return my->_remote_db->get_active_votes( author, permlink );
}


vector< view_state >              wallet_api::get_active_views( string author, string permlink )const
{
   return my->_remote_db->get_active_views( author, permlink );
}


vector< share_state >             wallet_api::get_active_shares( string author, string permlink )const
{
   return my->_remote_db->get_active_shares( author, permlink );
}


vector< moderation_state >        wallet_api::get_active_mod_tags( string author, string permlink )const
{
   return my->_remote_db->get_active_mod_tags( author, permlink );
}


vector< account_vote >            wallet_api::get_account_votes( string account, string from_author, string from_permlink, uint32_t limit )const
{
   return my->_remote_db->get_account_votes( account, from_author, from_permlink, limit );
}


vector< account_view >            wallet_api::get_account_views( string account, string from_author, string from_permlink, uint32_t limit )const
{
   return my->_remote_db->get_account_views( account, from_author, from_permlink, limit );
}


vector< account_share >           wallet_api::get_account_shares( string account, string from_author, string from_permlink, uint32_t limit )const
{
   return my->_remote_db->get_account_shares( account, from_author, from_permlink, limit );
}


vector< account_moderation >      wallet_api::get_account_moderation( string account, string from_author, string from_permlink, uint32_t limit )const
{
   return my->_remote_db->get_account_moderation( account, from_author, from_permlink, limit );
}


vector< tag_following_api_obj >   wallet_api::get_tag_followings( vector< string > tags )const
{
   return my->_remote_db->get_tag_followings( tags );
}


vector< tag_api_obj >             wallet_api::get_top_tags( string after_tag, uint32_t limit )const
{
   return my->_remote_db->get_top_tags( after_tag, limit );
}


vector< pair< tag_name_type, uint32_t > >   wallet_api::get_tags_used_by_author( string author )const
{
   return my->_remote_db->get_tags_used_by_author( author );
}



      //========================//
      // === Discussion API === //
      //========================//



discussion                        wallet_api::get_content( string author, string permlink )const
{
   discussion results = my->_remote_db->get_content( author, permlink );

   if( results.encrypted )
   {
      results.body = get_decrypted_message( results.body );
   }
   return results;
}


vector< discussion >              wallet_api::get_content_replies( string parent, string parent_permlink )const
{
   vector< discussion > results = my->_remote_db->get_content_replies( parent, parent_permlink );

   for( auto& d : results )
   {
      if( d.encrypted )
      {
         d.body = get_decrypted_message( d.body );
      }
   }

   return results;
}


vector< discussion >              wallet_api::get_replies_by_last_update( account_name_type start_author, string start_permlink, uint32_t limit )const
{
   vector< discussion > results = my->_remote_db->get_replies_by_last_update( start_author, start_permlink, limit );

   for( auto& d : results )
   {
      if( d.encrypted )
      {
         d.body = get_decrypted_message( d.body );
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_sort_rank( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_sort_rank( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_feed( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_feed( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_blog( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_blog( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_recommended( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_recommended( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_comments( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_comments( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_payout(const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_payout( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_post_discussions_by_payout( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_post_discussions_by_payout( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_comment_discussions_by_payout( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_comment_discussions_by_payout( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_created( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_created( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_active( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_active( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_votes( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_votes( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_views( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_views( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_shares( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_shares( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_children( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_children( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_vote_power( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_vote_power( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_view_power( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_view_power( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_share_power( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_share_power( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}


vector< discussion >              wallet_api::get_discussions_by_comment_power( const discussion_query& query )const
{
   vector< discussion > results = my->_remote_db->get_discussions_by_comment_power( query );

   if( query.include_private )
   {
      for( auto& d : results )
      {
         if( d.encrypted )
         {
            d.body = get_decrypted_message( d.body );
         }
      }
   }
   return results;
}



      //===================//
      // === State API === //
      //===================//



app::state                        wallet_api::get_state( string url )
{
   app::state results = my->_remote_db->get_state(url);

   for( auto& d : results.content )
   {
      if( d.second.encrypted )
      {
         d.second.body = get_decrypted_message( d.second.body );
      }
   }
   
   return results;
}



      //==============================//
      // === Account Transactions === //
      //==============================//



annotated_signed_transaction      wallet_api::account_create(
   string signatory,
   string registrar,
   string new_account_name,
   string referrer,
   string proxy,
   string governance_account,
   string recovery_account,
   string reset_account,
   string details,
   string url,
   string json,
   string json_private,
   authority owner_auth,
   authority active_auth,
   authority posting_auth,
   string secure_public_key,
   string connection_public_key,
   string friend_public_key,
   string companion_public_key,
   string business_type,
   share_type officer_vote_threshold,
   string business_public_key,
   asset fee,
   asset delegation,
   bool generate_keys,
   string password,
   bool broadcast )
{ try {

   FC_ASSERT( !is_locked() );

   account_create_operation op;

   op.signatory = signatory;
   op.registrar = registrar;
   op.new_account_name = new_account_name;
   op.referrer = referrer;
   op.proxy = proxy;
   op.governance_account = governance_account;
   op.recovery_account = recovery_account,
   op.reset_account = reset_account;
   op.details = details;
   op.url = url;
   op.json = json;
   op.json_private = json_private;
   op.fee = fee;
   op.delegation = delegation;

   if( generate_keys )
   {
      seed_phrase_info owner_key = suggest_seed_phrase();
      seed_phrase_info active_key = suggest_seed_phrase();
      seed_phrase_info posting_key = suggest_seed_phrase();
      seed_phrase_info secure_key = suggest_seed_phrase();
      seed_phrase_info connection_key = suggest_seed_phrase();
      seed_phrase_info friend_key = suggest_seed_phrase();
      seed_phrase_info companion_key = suggest_seed_phrase();
      import_key( owner_key.wif_priv_key );
      import_key( active_key.wif_priv_key );
      import_key( posting_key.wif_priv_key );
      import_key( secure_key.wif_priv_key );
      import_key( connection_key.wif_priv_key );
      import_key( friend_key.wif_priv_key );
      import_key( companion_key.wif_priv_key );
      op.owner_auth = authority( 1, owner_key.pub_key, 1 );
      op.active_auth = authority( 1, active_key.pub_key, 1 );
      op.posting_auth = authority( 1, posting_key.pub_key, 1 );
      op.secure_public_key = string( secure_key.pub_key );
      op.connection_public_key = string( connection_key.pub_key );
      op.friend_public_key = string( friend_key.pub_key );
      op.companion_public_key = string( companion_key.pub_key );
   }
   else if( password.length() )
   {
      fc::ecc::private_key owner_key = generate_private_key( new_account_name + "owner" + password );
      fc::ecc::private_key active_key = generate_private_key( new_account_name + "active" + password );
      fc::ecc::private_key posting_key = generate_private_key( new_account_name + "posting" + password );
      fc::ecc::private_key secure_key = generate_private_key( new_account_name + "secure" + password );
      fc::ecc::private_key connection_key = generate_private_key( new_account_name + "connection" + password );
      fc::ecc::private_key friend_key = generate_private_key( new_account_name + "friend" + password );
      fc::ecc::private_key companion_key = generate_private_key( new_account_name + "companion" + password );
      import_key( key_to_wif( owner_key ) );
      import_key( key_to_wif( active_key ) );
      import_key( key_to_wif( posting_key ) );
      import_key( key_to_wif( secure_key ) );
      import_key( key_to_wif( connection_key ) );
      import_key( key_to_wif( friend_key ) );
      import_key( key_to_wif( companion_key ) );
      op.owner_auth = authority( 1, owner_key.get_public_key(), 1 );
      op.active_auth = authority( 1, active_key.get_public_key(), 1 );
      op.posting_auth = authority( 1, posting_key.get_public_key(), 1 );
      op.secure_public_key = string( public_key_type( secure_key.get_public_key() ) );
      op.connection_public_key = string( public_key_type( connection_key.get_public_key() ) );
      op.friend_public_key = string( public_key_type( friend_key.get_public_key() ) );
      op.companion_public_key = string( public_key_type( companion_key.get_public_key() ) );
   }
   else
   {
      op.owner_auth = owner_auth;
      op.active_auth = active_auth;
      op.posting_auth = posting_auth;
      op.secure_public_key = secure_public_key;
      op.connection_public_key = connection_public_key;
      op.friend_public_key = friend_public_key;
      op.companion_public_key = companion_public_key;
   }

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
   
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_update(
   string signatory,
   string account,
   string details,
   string url,
   string json,
   string json_private,
   string pinned_permlink,
   authority owner_auth,
   authority active_auth,
   authority posting_auth,
   string secure_public_key,
   string connection_public_key,
   string friend_public_key,
   string companion_public_key,
   string business_type,
   share_type officer_vote_threshold,
   string business_public_key,
   bool active,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_update_operation op;

   op.signatory = signatory;
   op.account = account;
   op.details = details;
   op.url = url;
   op.json = json;
   op.json_private = json_private;
   op.pinned_permlink = pinned_permlink;
   op.owner_auth = owner_auth;
   op.active_auth = active_auth;
   op.posting_auth = posting_auth;
   op.secure_public_key = secure_public_key;
   op.connection_public_key = connection_public_key;
   op.friend_public_key = friend_public_key;
   op.companion_public_key = companion_public_key;
   
   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }

annotated_signed_transaction      wallet_api::account_profile(
   string signatory,
   string account,
   string governance_account,
   string profile_public_key,
   string first_name,
   string last_name,
   string gender,
   string date_of_birth,
   string email,
   string phone,
   string nationality,
   string address,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_profile_operation op;

   op.signatory = signatory;
   op.account = account;
   op.governance_account = governance_account;
   op.profile_public_key = profile_public_key;
   op.first_name = first_name;
   op.last_name = last_name;
   op.gender = gender;
   op.date_of_birth = date_of_birth;
   op.email = email;
   op.phone = phone;
   op.nationality = nationality;
   op.address = address;
   
   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_verification(
   string signatory,
   string verifier_account,
   string verified_account,
   string shared_image,
   signature_type image_signature,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_verification_operation op;

   op.signatory = signatory;
   op.verifier_account = verifier_account;
   op.verified_account = verified_account;
   op.shared_image = shared_image;
   op.image_signature = image_signature;
   
   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_business(
   string signatory,
   string governance_account,
   string init_ceo_account,
   string business_type,
   int64_t officer_vote_threshold,
   string business_public_key,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_business_operation op;

   op.signatory = signatory;
   op.governance_account = governance_account;
   op.init_ceo_account = init_ceo_account;
   op.business_type = business_type;
   op.officer_vote_threshold = officer_vote_threshold;
   op.business_public_key = business_public_key;
   
   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_membership(
   string signatory,
   string account,
   string membership_type,
   uint16_t months,
   string interface,
   bool recurring,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_membership_operation op;

   op.signatory = signatory;
   op.account = account;
   op.membership_type = membership_type;
   op.months = months;
   op.interface = interface;
   op.recurring = recurring;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_vote_executive(
   string signatory,
   string account,
   string business_account,
   string executive_account,
   string role,
   uint16_t vote_rank,
   bool approved,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_vote_executive_operation op;

   op.signatory = signatory;
   op.account = account;
   op.business_account = business_account;
   op.executive_account = executive_account;
   op.role = role;
   op.vote_rank = vote_rank;
   op.approved = approved;
  
   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_vote_officer(
   string signatory,
   string account,
   string business_account,
   string officer_account,
   uint16_t vote_rank,
   bool approved,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_vote_officer_operation op;

   op.signatory = signatory;
   op.account = account;
   op.business_account = business_account;
   op.officer_account = officer_account;
   op.vote_rank = vote_rank;
   op.approved = approved;
  
   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_member_request(
   string signatory,
   string account,
   string business_account,
   string message,
   bool requested,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_member_request_operation op;

   op.signatory = signatory;
   op.account = account;
   op.business_account = business_account;
   op.message = message;
   op.requested = requested;
  
   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_member_invite(
   string signatory,
   string account,
   string business_account,
   string member,
   string message,
   string encrypted_business_key,
   bool invited,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_member_invite_operation op;

   op.signatory = signatory;
   op.account = account;
   op.business_account = business_account;
   op.member = member;
   op.message = message;
   op.encrypted_business_key = encrypted_business_key;
   op.invited = invited;
  
   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_accept_request(
   string signatory,
   string account,
   string business_account,
   string member,
   string encrypted_business_key,
   bool accepted,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_accept_request_operation op;

   op.signatory = signatory;
   op.account = account;
   op.business_account = business_account;
   op.member = member;
   op.encrypted_business_key = encrypted_business_key;
   op.accepted = accepted;
  
   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_accept_invite(
   string signatory,
   string account,
   string business_account,
   bool accepted,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_accept_invite_operation op;

   op.signatory = signatory;
   op.account = account;
   op.business_account = business_account;
   op.accepted = accepted;
  
   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_remove_member(
   string signatory,
   string account,
   string business_account,
   string member,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_remove_member_operation op;

   op.signatory = signatory;
   op.account = account;
   op.business_account = business_account;
   op.member = member;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_update_list(
   string signatory,
   string account,
   string listed_account,
   string listed_asset,
   bool blacklisted,
   bool whitelisted,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_update_list_operation op;

   op.signatory = signatory;
   op.account = account;
   op.listed_account = listed_account;
   op.listed_asset = listed_asset;
   op.blacklisted = blacklisted;
   op.whitelisted = whitelisted;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );

} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_producer_vote(
   string signatory,
   string account,
   uint16_t vote_rank,
   string producer,
   bool approved,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_producer_vote_operation op;

   op.signatory = signatory;
   op.account = account;
   op.vote_rank = vote_rank;
   op.producer = producer;
   op.approved = approved;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_update_proxy(
   string signatory,
   string account,
   string proxy,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_update_proxy_operation op;

   op.signatory = signatory;
   op.account = account;
   op.proxy = proxy;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::request_account_recovery(
   string signatory,
   string recovery_account, 
   string account_to_recover, 
   authority new_owner_authority, 
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   request_account_recovery_operation op;

   op.signatory = signatory;
   op.recovery_account = recovery_account;
   op.account_to_recover = account_to_recover;
   op.new_owner_authority = new_owner_authority;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::recover_account(
   string signatory,
   string account_to_recover,
   authority new_owner_authority,
   authority recent_owner_authority,
   bool broadcast ) 
{ try {
   FC_ASSERT( !is_locked() );

   recover_account_operation op;

   op.signatory = signatory;
   op.account_to_recover = account_to_recover;
   op.new_owner_authority = new_owner_authority;
   op.recent_owner_authority = recent_owner_authority;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::reset_account(
   string signatory,
   string reset_account,
   string account_to_reset,
   authority new_owner_authority,
   bool broadcast ) 
{ try {
   FC_ASSERT( !is_locked() );

   reset_account_operation op;

   op.signatory = signatory;
   op.reset_account = reset_account;
   op.account_to_reset = account_to_reset;
   op.new_owner_authority = new_owner_authority;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::set_reset_account(
   string signatory,
   string account,
   string new_reset_account,
   uint16_t days,
   bool broadcast ) 
{ try {
   FC_ASSERT( !is_locked() );

   set_reset_account_operation op;

   op.signatory = signatory;
   op.account = account;
   op.new_reset_account = new_reset_account;
   op.days = days;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::change_recovery_account(
   string signatory,
   string account_to_recover,
   string new_recovery_account,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   change_recovery_account_operation op;

   op.signatory = signatory;
   op.account_to_recover = account_to_recover;
   op.new_recovery_account = new_recovery_account;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::decline_voting_rights(
   string signatory,
   string account,
   bool declined,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   decline_voting_rights_operation op;

   op.signatory = signatory;
   op.account = account;
   op.declined = declined;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::connection_request(
   string signatory,
   string account,
   string requested_account,
   string connection_type,
   string message,
   bool requested,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   connection_request_operation op;

   op.signatory = signatory;
   op.account = account;
   op.requested_account = requested_account;
   op.connection_type = connection_type;
   op.message = message;
   op.requested = requested;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::connection_accept(
   string signatory,
   string account,
   string requesting_account,
   string connection_id,
   string connection_type,
   string encrypted_key,
   bool connected,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   connection_accept_operation op;

   op.signatory = signatory;
   op.account = account;
   op.requesting_account = requesting_account;
   op.connection_id = connection_id;
   op.connection_type = connection_type;
   op.encrypted_key = encrypted_key;
   op.connected = connected;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::account_follow(
   string signatory,
   string follower,
   string following,
   string interface,
   bool added,
   bool followed,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   account_follow_operation op;

   op.signatory = signatory;
   op.follower = follower;
   op.following = following;
   op.interface = interface;
   op.added = added;
   op.followed = followed;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::tag_follow(
   string signatory,
   string follower,
   string tag,
   string interface,
   bool added,
   bool followed,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   tag_follow_operation op;

   op.signatory = signatory;
   op.follower = follower;
   op.tag = tag;
   op.interface = interface;
   op.added = added;
   op.followed = followed;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::activity_reward(
   string signatory,
   string account,
   string permlink,
   uint64_t view_id,
   uint64_t vote_id,
   string interface,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   activity_reward_operation op;

   op.signatory = signatory;
   op.account = account;
   op.permlink = permlink;
   op.view_id = view_id;
   op.vote_id = vote_id;
   op.interface = interface;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }



      //==============================//
      // === Network Transactions === //
      //==============================//



annotated_signed_transaction      wallet_api::update_network_officer(
   string signatory,
   string account,
   string officer_type,
   string details,
   string url,
   string json,
   bool active,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   update_network_officer_operation op;

   op.signatory = signatory;
   op.account = account;
   op.officer_type = officer_type;
   op.details = details;
   op.url = url;
   op.json = json;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::network_officer_vote(
   string signatory,
   string account,
   string network_officer,
   uint16_t vote_rank,
   bool approved,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   network_officer_vote_operation op;

   op.signatory = signatory;
   op.account = account;
   op.network_officer = network_officer;
   op.vote_rank = vote_rank;
   op.approved = approved;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::update_executive_board(
   string signatory,
   string account,
   string executive,
   asset budget,
   string details,
   string url,
   string json,
   bool active,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   update_executive_board_operation op;

   op.signatory = signatory;
   op.account = account;
   op.executive = executive;
   op.details = details;
   op.url = url;
   op.json = json;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::executive_board_vote(
   string signatory,
   string account,
   string executive_board,
   uint16_t vote_rank,
   bool approved,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   executive_board_vote_operation op;

   op.signatory = signatory;
   op.account = account;
   op.executive_board = executive_board;
   op.vote_rank = vote_rank;
   op.approved = approved;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::update_governance(
   string signatory,
   string account,
   string details,
   string url,
   string json,
   bool active,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   update_governance_operation op;

   op.signatory = signatory;
   op.account = account;
   op.details = details;
   op.url = url;
   op.json = json;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::subscribe_governance(
   string signatory,
   string account,
   string governance_account,
   bool subscribe,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   subscribe_governance_operation op;

   op.signatory = signatory;
   op.account = account;
   op.governance_account = governance_account;
   op.subscribe = subscribe;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::update_supernode(
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
   bool active,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   update_supernode_operation op;

   op.signatory = signatory;
   op.account = account;
   op.details = details;
   op.url = url;
   op.node_api_endpoint = node_api_endpoint;
   op.notification_api_endpoint = notification_api_endpoint;
   op.auth_api_endpoint = auth_api_endpoint;
   op.ipfs_endpoint = ipfs_endpoint;
   op.bittorrent_endpoint = bittorrent_endpoint;
   op.json = json;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::update_interface(
   string signatory,
   string account,
   string details,
   string url,
   string json,
   bool active,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   update_interface_operation op;

   op.signatory = signatory;
   op.account = account;
   op.details = details;
   op.url = url;
   op.json = json;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::update_mediator(
   string signatory,
   string account,
   string details,
   string url,
   string json,
   asset mediator_bond,
   bool active,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   update_mediator_operation op;

   op.signatory = signatory;
   op.account = account;
   op.details = details;
   op.url = url;
   op.json = json;
   op.mediator_bond = mediator_bond;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::create_community_enterprise(
   string signatory,
   string creator,
   string enterprise_id,
   string proposal_type,
   map< string, uint16_t > beneficiaries,
   vector< uint16_t > milestone_shares,
   vector< string > milestone_details,
   string investment,
   string details,
   string url,
   string json,
   time_point begin,
   uint16_t duration,
   asset daily_budget,
   asset fee,
   bool active,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   create_community_enterprise_operation op;

   op.signatory = signatory;
   op.creator = creator;
   op.enterprise_id = enterprise_id;
   op.proposal_type = proposal_type;

   for( auto b : beneficiaries )
   {
      op.beneficiaries[ account_name_type( b.first ) ] = b.second;
   }

   op.milestone_shares = milestone_shares;
   op.milestone_details = milestone_details;
   op.investment = investment;
   op.details = details;
   op.url = url;
   op.json = json;
   op.begin = begin;
   op.duration = duration;
   op.daily_budget = daily_budget;
   op.fee = fee;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::claim_enterprise_milestone(
   string signatory,
   string creator,
   string enterprise_id,
   uint16_t milestone,
   string details,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   claim_enterprise_milestone_operation op;

   op.signatory = signatory;
   op.creator = creator;
   op.enterprise_id = enterprise_id;
   op.milestone = milestone;
   op.details = details;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::approve_enterprise_milestone(
   string signatory,
   string account,
   string creator,
   string enterprise_id,
   uint16_t milestone,
   uint16_t vote_rank,
   bool approved,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   approve_enterprise_milestone_operation op;

   op.signatory = signatory;
   op.account = account;
   op.creator = creator;
   op.enterprise_id = enterprise_id;
   op.milestone = milestone;
   op.vote_rank = vote_rank;
   op.approved = approved;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }



      //=======================================//
      // === Post and Comment Transactions === //
      //=======================================//



annotated_signed_transaction      wallet_api::comment(
   string signatory,
   string author,
   string permlink,
   string title,
   string body,
   vector< string > ipfs,
   vector< string > magnet,
   string url,
   string language,
   string community,
   string public_key,
   string interface,
   asset comment_price,
   asset reply_price,
   asset premium_price,
   string parent_author, 
   string parent_permlink, 
   vector< string > tags,
   string json,
   comment_options options,
   bool deleted,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   comment_operation op;

   string from_public_key = string( get_account( author ).secure_public_key );

   op.signatory = signatory;
   op.author = author;
   op.permlink = permlink;
   op.title = title;

   if( public_key != string() )
   {
      op.body = get_encrypted_message( from_public_key, public_key, body );
   }
   else
   {
      op.body = body;
   }

   op.ipfs = ipfs;
   op.magnet = magnet;
   op.url = url;
   op.language = language;
   op.community = community;
   op.public_key = public_key;
   op.interface = interface;
   op.comment_price = comment_price;
   op.reply_price = reply_price;
   op.premium_price = premium_price;
   op.parent_author = parent_author;
   op.parent_permlink = parent_permlink;

   set< tag_name_type > t_set;

   for( string t : tags )
   {
      t_set.insert( tag_name_type( t ) );
   }

   op.tags.reserve( t_set.size() );

   for( tag_name_type t : t_set )
   {
      op.tags.push_back( t );
   }

   op.json = json;
   op.options = options;
   op.deleted = deleted;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::message(
   string signatory,
   string sender,
   string recipient,
   string message,
   string uuid,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   message_operation op;

   string from_public_key = string( get_account( sender ).secure_public_key );
   string to_public_key = string( get_account( recipient ).secure_public_key );

   op.signatory = signatory;
   op.sender = sender;
   op.recipient = recipient;
   op.message = get_encrypted_message( from_public_key, to_public_key, message );
   op.uuid = uuid;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }



annotated_signed_transaction      wallet_api::vote(
   string signatory,
   string voter,
   string author,
   string permlink,
   int16_t weight,
   string interface,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   vote_operation op;

   op.signatory = signatory;
   op.voter = voter;
   op.author = author;
   op.permlink = permlink;
   op.weight = weight;
   op.interface = interface;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::view(
   string signatory,
   string viewer,
   string author,
   string permlink,
   string interface,
   string supernode,
   bool viewed,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   view_operation op;

   op.signatory = signatory;
   op.viewer = viewer;
   op.author = author;
   op.permlink = permlink;
   op.interface = interface;
   op.supernode = supernode;
   op.viewed = viewed;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::share(
   string signatory,
   string sharer,
   string author,
   string permlink,
   string reach,
   string interface,
   string community,
   string tag,
   bool shared,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   share_operation op;

   op.signatory = signatory;
   op.sharer = sharer;
   op.author = author;
   op.permlink = permlink;
   op.reach = reach;
   op.interface = interface;
   op.community = community;
   op.tag = tag;
   op.shared = shared;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::moderation_tag(
   string signatory,
   string moderator,
   string author,
   string permlink,
   vector< string > tags,
   uint16_t rating,
   string details,
   string interface,
   bool filter,
   bool applied,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   moderation_tag_operation op;

   op.signatory = signatory;
   op.moderator = moderator;
   op.author = author;
   op.permlink = permlink;

   set< tag_name_type > t_set;

   for( string t : tags )
   {
      t_set.insert( tag_name_type( t ) );
   }

   op.tags.reserve( t_set.size() );

   for( tag_name_type t : t_set )
   {
      op.tags.push_back( t );
   }
   
   op.rating = rating;
   op.details = details;
   op.interface = interface;
   op.filter = filter;
   op.applied = applied;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }



      //================================//
      // === Community Transactions === //
      //================================//



annotated_signed_transaction      wallet_api::community_create(
   string signatory,
   string founder,
   string name,
   string community_privacy,
   string community_public_key,
   string json,
   string json_private,
   string details,
   string url,
   string reward_currency,
   uint16_t max_rating,
   uint32_t flags,
   uint32_t permissions,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_create_operation op;

   op.signatory = signatory;
   op.founder = founder;
   op.name = name;
   op.community_privacy = community_privacy;
   op.community_public_key = community_public_key;
   op.json = json;
   op.json_private = json_private;
   op.details = details;
   op.url = url;
   op.reward_currency = reward_currency;
   op.max_rating = max_rating;
   op.flags = flags;
   op.permissions = permissions;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_update(
   string signatory,
   string account,
   string community,
   string community_public_key,
   string json,
   string json_private,
   string details,
   string url,
   string pinned_author,
   string pinned_permlink,
   string reward_currency,
   uint16_t max_rating,
   uint32_t flags,
   uint32_t permissions,
   bool active,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_update_operation op;

   op.signatory = signatory;
   op.account = account;
   op.community = community;
   op.community_public_key = community_public_key;
   op.json = json;
   op.json_private = json_private;
   op.details = details;
   op.url = url;
   op.pinned_author = pinned_author;
   op.pinned_permlink = pinned_permlink;
   op.reward_currency = reward_currency;
   op.max_rating = max_rating;
   op.flags = flags;
   op.permissions = permissions;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_add_mod(
   string signatory,
   string account,
   string community,
   string moderator,
   bool added,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_add_mod_operation op;

   op.signatory = signatory;
   op.account = account;
   op.community = community;
   op.moderator = moderator;
   op.added = added;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_add_admin(
   string signatory,
   string account,
   string community,
   string admin,
   bool added,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_add_admin_operation op;

   op.signatory = signatory;
   op.account = account;
   op.community = community;
   op.admin = admin;
   op.added = added;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_vote_mod(
   string signatory,
   string account,
   string community,
   string moderator,
   uint16_t vote_rank,
   bool approved,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_vote_mod_operation op;

   op.signatory = signatory;
   op.account = account;
   op.community = community;
   op.moderator = moderator;
   op.vote_rank = vote_rank;
   op.approved = approved;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_transfer_ownership(
   string signatory,
   string account,
   string community,
   string new_founder,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_transfer_ownership_operation op;

   op.signatory = signatory;
   op.account = account;
   op.community = community;
   op.new_founder = new_founder;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_join_request(
   string signatory,
   string account,
   string community,
   string message,
   bool requested,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_join_request_operation op;

   op.signatory = signatory;
   op.account = account;
   op.community = community;
   op.message = message;
   op.requested = requested;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_join_invite(
   string signatory,
   string account,
   string member,
   string community,
   string message,
   string encrypted_community_key,
   bool invited,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_join_invite_operation op;

   op.signatory = signatory;
   op.account = account;
   op.community = community;
   op.message = message;
   op.encrypted_community_key = encrypted_community_key;
   op.invited = invited;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_join_accept(
   string signatory,
   string account,
   string member,
   string community,
   string encrypted_community_key,
   bool accepted,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_join_accept_operation op;

   op.signatory = signatory;
   op.account = account;
   op.member = member;
   op.community = community;
   op.encrypted_community_key = encrypted_community_key;
   op.accepted = accepted;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_invite_accept(
   string signatory,
   string account,
   string community,
   bool accepted,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_invite_accept_operation op;

   op.signatory = signatory;
   op.account = account;
   op.community = community;
   op.accepted = accepted;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_remove_member(
   string signatory,
   string account,
   string member,
   string community,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_remove_member_operation op;

   op.signatory = signatory;
   op.account = account;
   op.member = member;
   op.community = community;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_blacklist(
   string signatory,
   string account,
   string member,
   string community,
   bool blacklisted,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_blacklist_operation op;

   op.signatory = signatory;
   op.account = account;
   op.member = member;
   op.community = community;
   op.blacklisted = blacklisted;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_subscribe(
   string signatory,
   string account,
   string community,
   string interface,
   bool added,
   bool subscribed,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_subscribe_operation op;

   op.signatory = signatory;
   op.account = account;
   op.community = community;
   op.interface = interface;
   op.added = added;
   op.subscribed = subscribed; 

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_event(
   string signatory,
   string account,
   string community,
   string event_name,
   string location,
   string details,
   string url,
   string json,
   vector< string > invited,
   time_point event_start_time,
   time_point event_end_time,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_event_operation op;

   op.signatory = signatory;
   op.account = account;
   op.community = community;
   op.event_name = event_name;
   op.location = location;
   op.details = details;
   op.url = url;
   op.json = json;

   set< account_name_type > inv;
   
   for( auto a : invited )
   {
      inv.insert( account_name_type( a ) );
   }

   op.invited.reserve( inv.size() );

   for( auto a : inv )
   {
      op.invited.push_back( a );
   }

   op.event_start_time = event_start_time;
   op.event_end_time = event_end_time;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::community_event_attend(
   string signatory,
   string account,
   string community,
   string event_name,
   bool interested,
   bool attending,
   bool not_attending,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   community_event_attend_operation op;

   op.signatory = signatory;
   op.account = account;
   op.community = community;
   op.event_name = event_name;
   op.interested = interested;
   op.attending = attending;
   op.not_attending = not_attending;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }



      //=========================//
      // === Ad Transactions === //
      //=========================//



annotated_signed_transaction      wallet_api::ad_creative(
   string signatory,
   string account,
   string author,
   string objective,
   string creative_id,
   string creative,
   string json,
   string format_type,
   bool active,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   ad_creative_operation op;

   op.signatory = signatory;
   op.account = account;
   op.author = author;
   op.objective = objective;
   op.creative_id = creative_id;
   op.creative = creative;
   op.json = json;
   op.format_type = format_type;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::ad_campaign(
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
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   ad_campaign_operation op;

   op.signatory = signatory;
   op.account = account;
   op.campaign_id = campaign_id;
   op.budget = budget;
   op.begin = begin;
   op.end = end;
   op.json = json;

   set< account_name_type > age;

   for( auto a : agents )
   {
      age.insert( account_name_type( a ) );
   }

   op.agents.reserve( age.size() );

   for( auto a : age )
   {
      op.agents.push_back( a );
   }

   op.interface = interface;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::ad_inventory(
   string signatory,
   string provider,
   string inventory_id,
   string audience_id,
   string metric,
   asset min_price,
   uint32_t inventory,
   string json,
   bool active,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   ad_inventory_operation op;

   op.signatory = signatory;
   op.provider = provider;
   op.inventory_id = inventory_id;
   op.audience_id = audience_id;
   op.metric = metric;
   op.json = json;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::ad_audience(
   string signatory,
   string account,
   string audience_id,
   string json,
   vector< string > audience,
   bool active,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   ad_audience_operation op;

   op.signatory = signatory;
   op.account = account;
   op.audience_id = audience_id;
   op.json = json;

   set< account_name_type > aud;
   
   for( auto a : audience )
   {
      aud.insert( account_name_type( a ) );
   }

   op.audience.reserve( aud.size() );

   for( auto a : aud )
   {
      op.audience.push_back( a );
   }

   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::ad_bid(
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
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   ad_bid_operation op;

   op.signatory = signatory;
   op.bidder = bidder;
   op.bid_id = bid_id;
   op.account = account;
   op.campaign_id = campaign_id;
   op.author = author;
   op.creative_id = creative_id;
   op.provider = provider;
   op.inventory_id = inventory_id;
   op.bid_price = bid_price;
   op.requested = requested;
   op.included_audiences = included_audiences;
   op.excluded_audiences = excluded_audiences;
   op.json = json;
   op.expiration = expiration;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


      //============================//
      //==== Graph Transactions ====//
      //============================//


annotated_signed_transaction      wallet_api::graph_node(
   string signatory,
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
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   graph_node_operation op;

   op.signatory = signatory;
   op.account = account;

   set< graph_node_name_type > nodes;
   
   for( auto a : node_types )
   {
      nodes.insert( graph_node_name_type( a ) );
   }

   op.node_types.reserve( nodes.size() );

   for( auto a : nodes )
   {
      op.node_types.push_back( a );
   }

   op.node_id = node_id;
   op.name = name;
   op.details = details;
   op.attributes = attributes;
   op.attribute_values = attribute_values;
   op.json = json;
   op.json_private = json_private;
   op.node_public_key = node_public_key;
   op.interface = interface;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::graph_edge(
   string signatory,
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
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   graph_edge_operation op;

   op.signatory = signatory;
   op.account = account;

   set< graph_edge_name_type > edges;
   
   for( auto a : edge_types )
   {
      edges.insert( graph_edge_name_type( a ) );
   }

   op.edge_types.reserve( edges.size() );

   for( auto a : edges )
   {
      op.edge_types.push_back( a );
   }

   op.edge_id = edge_id;
   op.from_node_account = from_node_account;
   op.from_node_id = from_node_id;
   op.to_node_account = to_node_account;
   op.to_node_id = to_node_id;
   op.name = name;
   op.details = details;
   op.attributes = attributes;
   op.attribute_values = attribute_values;
   op.json = json;
   op.json_private = json_private;
   op.edge_public_key = edge_public_key;
   op.interface = interface;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::graph_node_property(
   string signatory,
   string account,
   string node_type,
   string graph_privacy,
   string edge_permission,
   string details,
   string url,
   string json,
   vector< string > attributes,
   string interface,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   graph_node_property_operation op;

   op.signatory = signatory;
   op.account = account;
   op.node_type = node_type;
   op.graph_privacy = graph_privacy;
   op.edge_permission = edge_permission;
   op.details = details;
   op.url = url;
   op.json = json;
   op.attributes = attributes;
   op.interface = interface;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::graph_edge_property(
   string signatory,
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
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   graph_edge_property_operation op;

   op.signatory = signatory;
   op.account = account;
   op.edge_type = edge_type;
   op.graph_privacy = graph_privacy;

   set< graph_node_name_type > from_nodes;
   
   for( auto a : from_node_types )
   {
      from_nodes.insert( graph_node_name_type( a ) );
   }

   op.from_node_types.reserve( from_nodes.size() );

   for( auto a : from_nodes )
   {
      op.from_node_types.push_back( a );
   }

   set< graph_node_name_type > to_nodes;
   
   for( auto a : to_node_types )
   {
      to_nodes.insert( graph_node_name_type( a ) );
   }

   op.to_node_types.reserve( to_nodes.size() );

   for( auto a : to_nodes )
   {
      op.to_node_types.push_back( a );
   }

   op.details = details;
   op.url = url;
   op.json = json;
   op.attributes = attributes;
   op.interface = interface;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }



      //===============================//
      // === Transfer Transactions === //
      //===============================//



annotated_signed_transaction      wallet_api::transfer(
   string signatory,
   string from,
   string to,
   asset amount,
   string memo,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   check_memo( memo, get_account( from ) );

   transfer_operation op;

   string from_public_key = string( get_account( from ).secure_public_key );
   string to_public_key = string( get_account( to ).secure_public_key );

   op.signatory = signatory;
   op.from = from;
   op.to = to;
   op.amount = amount;
   op.memo = get_encrypted_message( from_public_key, to_public_key, memo );

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::transfer_request(
   string signatory,
   string to,
   string from,
   asset amount,
   string memo,
   string request_id,
   time_point expiration,
   bool requested,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   check_memo( memo, get_account( from ) );

   transfer_request_operation op;

   string from_public_key = string( get_account( from ).secure_public_key );
   string to_public_key = string( get_account( to ).secure_public_key );

   op.signatory = signatory;
   op.from = from;
   op.to = to;
   op.amount = amount;
   op.memo = get_encrypted_message( from_public_key, to_public_key, memo );
   op.request_id = request_id;
   op.expiration = expiration;
   op.requested = requested;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::transfer_accept(
   string signatory,
   string from,
   string to,
   string request_id,
   bool accepted,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   transfer_accept_operation op;

   op.signatory = signatory;
   op.from = from;
   op.to = to;
   op.request_id = request_id;
   op.accepted = accepted;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::transfer_recurring(
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
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   check_memo( memo, get_account( from ) );

   transfer_recurring_operation op;

   string from_public_key = string( get_account( from ).secure_public_key );
   string to_public_key = string( get_account( to ).secure_public_key );

   op.signatory = signatory;
   op.from = from;
   op.to = to;
   op.amount = amount;
   op.transfer_id = transfer_id;
   op.begin = begin;
   op.payments = payments;
   op.interval = interval;
   op.memo = get_encrypted_message( from_public_key, to_public_key, memo );
   op.extensible = extensible;
   op.fill_or_kill = fill_or_kill;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::transfer_recurring_request(
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
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   check_memo( memo, get_account( from ) );

   transfer_recurring_request_operation op;

   string from_public_key = string( get_account( from ).secure_public_key );
   string to_public_key = string( get_account( to ).secure_public_key );

   op.signatory = signatory;
   op.from = from;
   op.to = to;
   op.amount = amount;
   op.request_id = request_id;
   op.begin = begin;
   op.payments = payments;
   op.interval = interval;
   op.memo = get_encrypted_message( from_public_key, to_public_key, memo );
   op.expiration = expiration;
   op.extensible = extensible;
   op.fill_or_kill = fill_or_kill;
   op.requested = requested;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::transfer_recurring_accept(
   string signatory,
   string from,
   string to,
   string request_id,
   bool accepted,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   transfer_recurring_accept_operation op;

   op.signatory = signatory;
   op.from = from;
   op.to = to;
   op.request_id = request_id;
   op.accepted = accepted;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::transfer_confidential(
   vector< confidential_input > inputs,
   vector< confidential_output > outputs,
   asset fee,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   transfer_confidential_operation op;

   op.inputs = inputs;
   op.outputs = outputs;
   op.fee = fee;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::transfer_to_confidential(
   string signatory,
   string from,
   asset amount,
   blind_factor_type blinding_factor,
   vector< confidential_output > outputs,
   asset fee,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   transfer_to_confidential_operation op;

   op.signatory = signatory;
   op.from = from;
   op.amount = amount;
   op.blinding_factor = blinding_factor;
   op.outputs = outputs;
   op.fee = fee;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::transfer_from_confidential(
   string to,
   asset amount,
   blind_factor_type blinding_factor,
   vector< confidential_input > inputs,
   asset fee,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   transfer_from_confidential_operation op;

   op.to = to;
   op.amount = amount;
   op.blinding_factor = blinding_factor;
   op.inputs = inputs;
   op.fee = fee;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


      //==============================//
      // === Balance Transactions === //
      //==============================//



annotated_signed_transaction      wallet_api::claim_reward_balance(
   string signatory,
   string account,
   asset reward,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   claim_reward_balance_operation op;

   op.signatory = signatory;
   op.account = account;
   op.reward = reward;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::stake_asset(
   string signatory,
   string from,
   string to,
   asset amount,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   stake_asset_operation op;

   op.signatory = signatory;
   op.from = from;
   op.to = to;
   op.amount = amount;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::unstake_asset(
   string signatory,
   string from,
   string to,
   asset amount,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   unstake_asset_operation op;

   op.signatory = signatory;
   op.from = from;
   op.to = to;
   op.amount = amount;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction       wallet_api::unstake_asset_route( 
   string signatory,
   string from,
   string to,
   uint16_t percent,
   bool auto_stake,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   unstake_asset_route_operation op;

   op.signatory = signatory;
   op.from = from;
   op.to = to;
   op.percent = percent;
   op.auto_stake = auto_stake;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::transfer_to_savings(
   string signatory,
   string from, 
   string to, 
   asset amount, 
   string memo, 
   bool broadcast  )
{ try {
   FC_ASSERT( !is_locked() );

   check_memo( memo, get_account( from ) );

   transfer_to_savings_operation op;

   string from_public_key = string( get_account( from ).secure_public_key );
   string to_public_key = string( get_account( to ).secure_public_key );

   op.signatory = signatory;
   op.from = from;
   op.to = to;
   op.memo = get_encrypted_message( from_public_key, to_public_key, memo );
   op.amount = amount;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::transfer_from_savings(
   string signatory,
   string from,
   string to,
   asset amount,
   string request_id,
   string memo,
   bool transferred,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   check_memo( memo, get_account( from ) );

   transfer_from_savings_operation op;

   string from_public_key = string( get_account( from ).secure_public_key );
   string to_public_key = string( get_account( to ).secure_public_key );

   op.signatory = signatory;
   op.from = from;
   op.to = to;
   op.amount = amount;
   op.request_id = request_id;
   op.memo = get_encrypted_message( from_public_key, to_public_key, memo );
   op.transferred = transferred;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction       wallet_api::delegate_asset(
   string signatory,
   string delegator, 
   string delegatee, 
   asset amount, 
   bool broadcast)
{ try {
   FC_ASSERT( !is_locked() );

   delegate_asset_operation op;

   op.signatory = signatory;
   op.delegator = delegator;
   op.delegatee = delegatee;
   op.amount = amount;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }



      //==================================//
      // === Marketplace Transactions === //
      //==================================//



annotated_signed_transaction       wallet_api::product_update(
   string signatory,
   string account,
   string product_id,
   string name,
   string sale_type,
   string url,
   string json,
   vector< string > product_variants,
   vector< string > product_details,
   vector< string > product_images,
   vector< asset > product_prices,
   vector< uint32_t > stock_available,
   vector< string > delivery_variants,
   vector< string > delivery_details,
   vector< asset > delivery_prices,
   bool broadcast)
{ try {
   FC_ASSERT( !is_locked() );

   product_update_operation op;

   op.signatory = signatory;
   op.account = account;
   op.product_id = product_id;
   op.name = name;
   op.sale_type = sale_type;
   op.url = url;
   op.json = json;
   op.product_variants = product_variants;
   op.product_details = product_details;
   op.product_images = product_images;
   op.product_prices = product_prices;
   op.stock_available = stock_available;
   op.delivery_variants = delivery_variants;
   op.delivery_details = delivery_details;
   op.delivery_prices = delivery_prices;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction       wallet_api::product_purchase(
   string signatory,
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
   bool completed,
   bool broadcast)
{ try {
   FC_ASSERT( !is_locked() );

   product_purchase_operation op;

   op.signatory = signatory;
   op.buyer = buyer;
   op.order_id = order_id;
   op.seller = seller;
   op.product_id = product_id;
   op.order_variants = order_variants;
   op.order_size = order_size;
   op.memo = memo;
   op.json = json;
   op.shipping_address = shipping_address;
   op.delivery_variant = delivery_variant;
   op.delivery_details = delivery_details;
   op.acceptance_time = acceptance_time;
   op.escrow_expiration = escrow_expiration;
   op.completed = completed;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }



annotated_signed_transaction      wallet_api::escrow_transfer(
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
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   check_memo( memo, get_account( from ) );

   escrow_transfer_operation op;

   string from_public_key;
   string to_public_key;

   if( account == from )
   {
      from_public_key = string( get_account( from ).secure_public_key );
      to_public_key = string( get_account( to ).secure_public_key );
   }
   else if( account == to )
   {
      from_public_key = string( get_account( to ).secure_public_key );
      to_public_key = string( get_account( from ).secure_public_key );
   }

   op.signatory = signatory;
   op.account = account;
   op.from = from;
   op.to = to;
   op.escrow_id = escrow_id;
   op.amount = amount;
   op.acceptance_time = acceptance_time;
   op.escrow_expiration = escrow_expiration;
   op.memo = get_encrypted_message( from_public_key, to_public_key, memo );
   op.json = json;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::escrow_approve(
   string signatory,
   string account,
   string mediator,
   string escrow_from,
   string escrow_id,
   bool approved,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   escrow_approve_operation op;

   op.signatory = signatory;
   op.account = account;
   op.mediator = mediator;
   op.escrow_from = escrow_from;
   op.escrow_id = escrow_id;
   op.approved = approved;
   
   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::escrow_dispute(
   string signatory,
   string account,
   string escrow_from,
   string escrow_id,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   escrow_dispute_operation op;

   op.signatory = signatory;
   op.account = account;
   op.escrow_from = escrow_from;
   op.escrow_id = escrow_id;
   
   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::escrow_release(
   string signatory,
   string account,
   string escrow_from,
   string escrow_id,
   uint16_t release_percent,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   escrow_release_operation op;

   op.signatory = signatory;
   op.account = account;
   op.escrow_from = escrow_from;
   op.escrow_id = escrow_id;
   op.release_percent = release_percent;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }



      //==============================//
      // === Trading Transactions === //
      //==============================//



annotated_signed_transaction      wallet_api::limit_order(
   string signatory,
   string owner,
   string order_id,
   asset amount_to_sell,
   price exchange_rate,
   string interface,
   time_point expiration,
   bool opened,
   bool fill_or_kill,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   limit_order_operation op;

   op.signatory = signatory;
   op.owner = owner;
   op.order_id = order_id;
   op.amount_to_sell = amount_to_sell;
   op.exchange_rate = exchange_rate;
   op.interface = interface;
   op.expiration = expiration;
   op.opened = opened;
   op.fill_or_kill = fill_or_kill;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::margin_order(
   string signatory,
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
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   margin_order_operation op;

   op.signatory = signatory;
   op.owner = owner;
   op.order_id = order_id;
   op.exchange_rate = exchange_rate;
   op.collateral = collateral;
   op.amount_to_borrow = amount_to_borrow;
   op.stop_loss_price = stop_loss_price;
   op.take_profit_price = take_profit_price;
   op.limit_stop_loss_price = limit_stop_loss_price;
   op.limit_take_profit_price = limit_take_profit_price;
   op.interface = interface;
   op.expiration = expiration;
   op.opened = opened;
   op.fill_or_kill = fill_or_kill;
   op.force_close = force_close;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::auction_order(
   string signatory,
   string owner,
   string order_id,
   asset amount_to_sell,
   price limit_close_price,
   string interface,
   time_point expiration,
   bool opened,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   auction_order_operation op;

   op.signatory = signatory;
   op.owner = owner;
   op.order_id = order_id;
   op.amount_to_sell = amount_to_sell;
   op.limit_close_price = limit_close_price;
   op.interface = interface;
   op.expiration = expiration;
   op.opened = opened;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::call_order(
   string signatory,
   string owner,
   asset collateral,
   asset debt,
   uint16_t target_collateral_ratio,
   string interface,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   call_order_operation op;

   op.signatory = signatory;
   op.owner = owner;
   op.collateral = collateral;
   op.debt = debt;
   op.target_collateral_ratio = target_collateral_ratio;
   op.interface = interface;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::option_order(
   string signatory,
   string owner,
   string order_id,
   asset options_issued,
   string interface,
   bool opened,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   option_order_operation op;

   op.signatory = signatory;
   op.owner = owner;
   op.order_id = order_id;
   op.options_issued = options_issued;
   op.interface = interface;
   op.opened = opened;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }



      //===========================//
      // === Pool Transactions === //
      //===========================//



annotated_signed_transaction      wallet_api::liquidity_pool_create(
   string signatory,
   string account,
   asset first_amount,
   asset second_amount,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   liquidity_pool_create_operation op;

   op.signatory = signatory;
   op.account = account;
   op.first_amount = first_amount;
   op.second_amount = second_amount;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::liquidity_pool_exchange(
   string signatory,
   string account,
   asset amount,
   string receive_asset,
   string interface,
   price limit_price,
   bool acquire,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   liquidity_pool_exchange_operation op;

   op.signatory = signatory;
   op.account = account;
   op.amount = amount;
   op.receive_asset = receive_asset;
   op.interface = interface;
   op.limit_price = limit_price;
   op.acquire = acquire;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::liquidity_pool_fund(
   string signatory,
   string account,
   asset amount,
   string pair_asset,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   liquidity_pool_fund_operation op;

   op.signatory = signatory;
   op.account = account;
   op.amount = amount;
   op.pair_asset = pair_asset;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::liquidity_pool_withdraw(
   string signatory,
   string account,
   asset amount,
   string receive_asset,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   liquidity_pool_withdraw_operation op;

   op.signatory = signatory;
   op.account = account;
   op.amount = amount;
   op.receive_asset = receive_asset;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::credit_pool_collateral(
   string signatory,
   string account,
   asset amount,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   credit_pool_collateral_operation op;

   op.signatory = signatory;
   op.account = account;
   op.amount = amount;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::credit_pool_borrow(
   string signatory,
   string account,
   asset amount,
   asset collateral,
   string loan_id,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   credit_pool_borrow_operation op;

   op.signatory = signatory;
   op.account = account;
   op.amount = amount;
   op.collateral = collateral;
   op.loan_id = loan_id;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::credit_pool_lend(
   string signatory,
   string account,
   asset amount,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   credit_pool_lend_operation op;

   op.signatory = signatory;
   op.account = account;
   op.amount = amount;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::credit_pool_withdraw(
   string signatory,
   string account,
   asset amount,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   credit_pool_withdraw_operation op;

   op.signatory = signatory;
   op.account = account;
   op.amount = amount;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::option_pool_create(
   string signatory,
   string account,
   string first_asset,
   string second_asset,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   option_pool_create_operation op;

   op.signatory = signatory;
   op.account = account;
   op.first_asset = first_asset;
   op.second_asset = second_asset;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::prediction_pool_create(
   string signatory,
   string account,
   string prediction_symbol,
   string collateral_symbol,
   vector< string > outcome_assets,
   vector< string > outcome_details,
   string display_symbol,
   string json,
   string url,
   string details,
   time_point outcome_time,
   asset prediction_bond,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   prediction_pool_create_operation op;

   op.signatory = signatory;
   op.account = account;
   op.prediction_symbol = prediction_symbol;
   op.collateral_symbol = collateral_symbol;

   set< asset_symbol_type > out;
   
   for( auto a : outcome_assets )
   {
      out.insert( asset_symbol_type( a ) );
   }

   op.outcome_assets.reserve( out.size() );

   for( auto a : out )
   {
      op.outcome_assets.push_back( a );
   }

   op.outcome_details = outcome_details;
   op.display_symbol = display_symbol;
   op.json = json;
   op.url = url;
   op.details = details;
   op.outcome_time = outcome_time;
   op.prediction_bond = prediction_bond;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::prediction_pool_exchange(
   string signatory,
   string account,
   asset amount,
   string prediction_asset,
   bool exchange_base,
   bool withdraw,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   prediction_pool_exchange_operation op;

   op.signatory = signatory;
   op.account = account;
   op.amount = amount;
   op.prediction_asset = prediction_asset;
   op.exchange_base = exchange_base;
   op.withdraw = withdraw;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::prediction_pool_resolve(
   string signatory,
   string account,
   asset amount,
   string resolution_outcome,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   prediction_pool_resolve_operation op;

   op.signatory = signatory;
   op.account = account;
   op.amount = amount;
   op.resolution_outcome = resolution_outcome;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }



      //============================//
      // === Asset Transactions === //
      //============================//



annotated_signed_transaction      wallet_api::asset_create(
   string signatory,
   string issuer,
   string symbol,
   string asset_type,
   asset coin_liquidity,
   asset usd_liquidity,
   asset credit_liquidity,
   asset_options options,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_create_operation op;

   op.signatory = signatory;
   op.issuer = issuer;
   op.symbol = symbol;
   op.asset_type = asset_type;
   op.coin_liquidity = coin_liquidity;
   op.usd_liquidity = usd_liquidity;
   op.credit_liquidity = credit_liquidity;
   op.options = options;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_update(
   string signatory,
   string issuer,
   string asset_to_update,
   asset_options new_options,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_update_operation op;

   op.signatory = signatory;
   op.issuer = issuer;
   op.asset_to_update = asset_to_update;
   op.new_options = new_options;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_issue(
   string signatory,
   string issuer,
   asset asset_to_issue,
   string issue_to_account,
   string memo,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   check_memo( memo, get_account( issuer ) );

   asset_issue_operation op;

   string from_public_key = string( get_account( issuer ).secure_public_key );
   string to_public_key = string( get_account( issue_to_account ).secure_public_key );
   
   op.signatory = signatory;
   op.issuer = issuer;
   op.asset_to_issue = asset_to_issue;
   op.issue_to_account = issue_to_account;
   op.memo = get_encrypted_message( from_public_key, to_public_key, memo );

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_reserve(
   string signatory,
   string payer,
   asset amount_to_reserve,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_reserve_operation op;
   
   op.signatory = signatory;
   op.payer = payer;
   op.amount_to_reserve = amount_to_reserve;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_update_issuer(
   string signatory,
   string issuer,
   string asset_to_update,
   string new_issuer,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_update_issuer_operation op;

   op.signatory = signatory;
   op.issuer = issuer;
   op.asset_to_update = asset_to_update;
   op.new_issuer = new_issuer;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_distribution(
   string signatory,
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
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_distribution_operation op;

   op.signatory = signatory;
   op.issuer = issuer;
   op.distribution_asset = distribution_asset;
   op.fund_asset = fund_asset;
   op.details = details;
   op.url = url;
   op.json = json;
   op.distribution_rounds = distribution_rounds;
   op.distribution_interval_days = distribution_interval_days;
   op.max_intervals_missed = max_intervals_missed;
   op.min_input_fund_units = min_input_fund_units;
   op.max_input_fund_units = max_input_fund_units;
   op.input_fund_unit = input_fund_unit;
   op.output_distribution_unit = output_distribution_unit;
   op.min_unit_ratio = min_unit_ratio;
   op.max_unit_ratio = max_unit_ratio;
   op.min_input_balance_units = min_input_balance_units;
   op.max_input_balance_units = max_input_balance_units;
   op.begin_time = begin_time;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_distribution_fund(
   string signatory,
   string sender,
   string distribution_asset,
   asset amount,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_distribution_fund_operation op;

   op.signatory = signatory;
   op.sender = sender;
   op.distribution_asset = distribution_asset;
   op.amount = amount;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_option_exercise(
   string signatory,
   string account,
   asset amount,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_option_exercise_operation op;

   op.signatory = signatory;
   op.account = account;
   op.amount = amount;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_stimulus_fund(
   string signatory,
   string account,
   string stimulus_asset,
   asset amount,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_stimulus_fund_operation op;

   op.signatory = signatory;
   op.account = account;
   op.stimulus_asset = stimulus_asset;
   op.amount = amount;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_update_feed_producers(
   string signatory,
   string issuer,
   string asset_to_update,
   flat_set< account_name_type > new_feed_producers,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_update_feed_producers_operation op;

   op.signatory = signatory;
   op.issuer = issuer;
   op.asset_to_update = asset_to_update;
   op.new_feed_producers = new_feed_producers;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_publish_feed(
   string signatory,
   string publisher,
   string symbol,
   price_feed feed,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_publish_feed_operation op;

   op.signatory = signatory;
   op.publisher = publisher;
   op.symbol = symbol;
   op.feed = feed;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_settle(
   string signatory,
   string account,
   asset amount,
   string interface,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_settle_operation op;

   op.signatory = signatory;
   op.account = account;
   op.amount = amount;
   op.interface = interface;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_global_settle(
   string signatory,
   string issuer,
   string asset_to_settle,
   price settle_price,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_global_settle_operation op;

   op.signatory = signatory;
   op.issuer = issuer;
   op.asset_to_settle = asset_to_settle;
   op.settle_price = settle_price;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::asset_collateral_bid(
   string signatory,
   string bidder,
   asset collateral,
   asset debt,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   asset_collateral_bid_operation op;

   op.signatory = signatory;
   op.bidder = bidder;
   op.collateral = collateral;
   op.debt = debt;

   signed_transaction tx;
   tx.operations.push_back( op );
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


      //=====================================//
      // === Block Producer Transactions === //
      //=====================================//


annotated_signed_transaction      wallet_api::producer_update(
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
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   producer_update_operation op;

   op.signatory = signatory;
   op.owner = owner;
   op.details = details;
   op.url = url;
   op.json = json;
   op.latitude = latitude;
   op.longitude = longitude;
   op.block_signing_key = block_signing_key;
   op.props = props;
   op.active = active;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::proof_of_work(
   proof_of_work_type work,
   string new_owner_key,
   chain_properties props,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   proof_of_work_operation op;

   op.work = work;
   op.new_owner_key = new_owner_key;
   op.props = props;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::verify_block(
   string signatory,
   string producer,
   string block_id,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   verify_block_operation op;

   op.signatory = signatory;
   op.producer = producer;
   op.block_id = block_id_type( block_id );

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::commit_block(
   string signatory,
   string producer,
   string block_id,
   flat_set< transaction_id_type > verifications,
   asset commitment_stake,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   commit_block_operation op;

   op.signatory = signatory;
   op.producer = producer;
   op.block_id = block_id_type( block_id );
   op.verifications = verifications;
   op.commitment_stake = commitment_stake;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::producer_violation(
   string signatory,
   string reporter,
   vector< char > first_trx,
   vector< char > second_trx,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   producer_violation_operation op;

   op.signatory = signatory;
   op.reporter = reporter;
   op.first_trx = first_trx;
   op.second_trx = second_trx;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


      //=============================//
      //==== Custom Transactions ====//
      //=============================//


annotated_signed_transaction      wallet_api::custom(
   flat_set< account_name_type > required_auths,
   uint16_t id,
   vector< char > data,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   custom_operation op;

   op.required_auths = required_auths;
   op.id = id;
   op.data = data;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction      wallet_api::custom_json(
   flat_set< account_name_type > required_auths,
   flat_set< account_name_type > required_posting_auths,
   string id,
   string json,
   bool broadcast )
{ try {
   FC_ASSERT( !is_locked() );

   custom_json_operation op;

   op.required_auths = required_auths;
   op.required_posting_auths = required_posting_auths;
   op.id = id;
   op.json = json;

   signed_transaction tx;
   tx.operations.push_back(op);
   tx.validate();

   return my->sign_transaction( tx, broadcast );
} FC_CAPTURE_AND_RETHROW() }


} } // node::wallet