#include <node/producer/producer_plugin.hpp>
#include <node/producer/producer_objects.hpp>
#include <node/producer/producer_operations.hpp>

#include <node/chain/account_object.hpp>
#include <node/chain/database.hpp>
#include <node/chain/database_exceptions.hpp>
#include <node/chain/generic_custom_operation_interpreter.hpp>
#include <node/chain/index.hpp>
#include <node/chain/node_objects.hpp>

#include <node/app/impacted.hpp>

#include <fc/time.hpp>

#include <graphene/utilities/key_conversion.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/thread/thread.hpp>
#include <fc/thread/future.hpp>
#include <fc/thread/mutex.hpp>
#include <fc/thread/scoped_lock.hpp>

#include <iostream>
#include <memory>


#define DISTANCE_CALC_PRECISION (10000)


namespace node { namespace producer {

namespace bpo = boost::program_options;

using std::string;
using std::vector;

using protocol::signed_transaction;
using chain::account_object;

void new_chain_banner( std::map<public_key_type, fc::ecc::private_key> _private_keys )
{
  std::cerr << "\n"
      "********************************\n"
      "*                              *\n"
      "*   ------- NEW CHAIN ------   *\n"
      "*   -        Welcome       -   *\n"
      "*   ------------------------   *\n"
      "*                              *\n"
      "********************************\n"
      "\n"
			"\n";
  std::cerr << "\n"
      "********************************\n"
      "*   ---- INIT PUBLIC KEY ---   *\n"
      "*   " << INIT_PUBLIC_KEY_STR <<  "   *\n"
      "*   ------------------------   *\n"
      "*                              *\n"
      "********************************\n"
      "\n"
			"\n";

	#if SHOW_PRIVATE_KEYS
		std::cerr << "\n"
      "********************************\n"
      "*   ------- KEYPAIRS -------   *\n"
      "*   ------------------------   *\n";
      for( auto& key : _private_keys )
      {
         std::cerr << "*   -------- PUB ---------   *\n" 
         "*   " << string( key.first ) << "   *\n"
         "*   ------ PRIVATE -------  *\n" 
         "*   " << key.second.get_secret().str() << "   *\n";
      }
      std::cerr <<	"*   ------------------------   *\n"
      "*                              *\n"
      "********************************\n"
      "\n"
			"\n";
	#endif

  return;
}

namespace detail
{
   using namespace node::chain;

   class producer_plugin_impl
   {
      public:
         producer_plugin_impl( producer_plugin& plugin )
            : _self( plugin ){}

         void plugin_initialize();

         void pre_apply_block( const node::protocol::signed_block& blk );
         void pre_transaction( const signed_transaction& trx );
         void pre_operation( const operation_notification& note );
         void post_operation( const chain::operation_notification& note );
         void on_block( const signed_block& b );

         void update_account_bandwidth( const account_object& a, uint32_t trx_size );

         producer_plugin& _self;
         std::shared_ptr< generic_custom_operation_interpreter< producer_plugin_operation > > _custom_operation_interpreter;

         std::set< node::protocol::account_name_type >                     _dupe_customs;

         uint32_t                                                          _mining_threads = 1;

         std::vector<std::shared_ptr<fc::thread> >                         _thread_pool;
   };

   void producer_plugin_impl::plugin_initialize()
   {
      _custom_operation_interpreter = std::make_shared< generic_custom_operation_interpreter< producer_plugin_operation > >( _self.database() );

      _custom_operation_interpreter->register_evaluator< enable_content_editing_evaluator >( &_self );

      _self.database().set_custom_operation_interpreter( _self.plugin_name(), _custom_operation_interpreter );
   }

   struct comment_options_extension_visitor
   {
      comment_options_extension_visitor( const comment_object& c, const database& db ) : _c( c ), _db( db ) {}

      typedef void result_type;

      const comment_object& _c;
      const database& _db;

      void operator()( const comment_payout_beneficiaries& cpb )const
      {
         ASSERT( cpb.beneficiaries.size() <= 8,
            chain::plugin_exception,
            "Cannot specify more than 8 beneficiaries." );
      }
   };

   void producer_plugin_impl::pre_apply_block( const node::protocol::signed_block& b )
   {
      _dupe_customs.clear();
   }

   void check_memo( const string& memo, const account_object& account, const account_authority_object& auth )
   {
      vector< public_key_type > keys;

      try
      {
         // Check if memo is a private key
         keys.push_back( fc::ecc::extended_private_key::from_base58( memo ).get_public_key() );
      }
      catch( fc::parse_error_exception& ) {}
      catch( fc::assert_exception& ) {}

      // Get possible keys if memo was an account password
      string owner_seed = account.name + "owner" + memo;
      auto owner_secret = fc::sha256::hash( owner_seed.c_str(), owner_seed.size() );
      keys.push_back( fc::ecc::private_key::regenerate( owner_secret ).get_public_key() );

      string active_seed = account.name + "active" + memo;
      auto active_secret = fc::sha256::hash( active_seed.c_str(), active_seed.size() );
      keys.push_back( fc::ecc::private_key::regenerate( active_secret ).get_public_key() );

      string posting_seed = account.name + "posting" + memo;
      auto posting_secret = fc::sha256::hash( posting_seed.c_str(), posting_seed.size() );
      keys.push_back( fc::ecc::private_key::regenerate( posting_secret ).get_public_key() );

      // Check keys against public keys in authorites
      for( auto& key_weight_pair : auth.owner_auth.key_auths )
      {
         for( auto& key : keys )
            ASSERT( key_weight_pair.first != key, chain::plugin_exception,
               "Detected private owner key in memo field. You should change your owner keys." );
      }

      for( auto& key_weight_pair : auth.active_auth.key_auths )
      {
         for( auto& key : keys )
            ASSERT( key_weight_pair.first != key, chain::plugin_exception,
               "Detected private active key in memo field. You should change your active keys." );
      }

      for( auto& key_weight_pair : auth.posting_auth.key_auths )
      {
         for( auto& key : keys )
            ASSERT( key_weight_pair.first != key, chain::plugin_exception,
               "Detected private posting key in memo field. You should change your posting keys." );
      }

      const auto& secure_public_key = account.secure_public_key;
      for( auto& key : keys )
         ASSERT( secure_public_key != key, chain::plugin_exception,
            "Detected private memo key in memo field. You should change your memo key." );
   }

   struct operation_visitor
   {
      operation_visitor( const chain::database& db ) : _db( db ) {}

      const chain::database& _db;

      typedef void result_type;

      template< typename T >
      void operator()( const T& )const {}

      void operator()( const comment_operation& o )const
      {
         if( o.parent_author != ROOT_POST_PARENT )
         {
            const auto& parent = _db.find_comment( o.parent_author, o.parent_permlink );

            if( parent != nullptr )
            ASSERT( parent->depth < SOFT_MAX_COMMENT_DEPTH,
               chain::plugin_exception,
               "Comment is nested ${x} posts deep, maximum depth is ${y}.", ("x",parent->depth)("y",SOFT_MAX_COMMENT_DEPTH) );
         }

         auto itr = _db.find< comment_object, by_permlink >( boost::make_tuple( o.author, o.permlink ) );

         if( itr != nullptr && itr->cashout_time == fc::time_point::maximum() )
         {
            auto edit_lock = _db.find< content_edit_lock_object, by_account >( o.author );

            ASSERT( edit_lock != nullptr && _db.head_block_time() < edit_lock->lock_time,
               chain::plugin_exception,
               "The comment is archived" );
         }
      }

      void operator()( const transfer_operation& o )const
      {
         if( o.memo.length() > 0 )
            check_memo( o.memo,
                        _db.get< account_object, chain::by_name >( o.from ),
                        _db.get< account_authority_object, chain::by_account >( o.from ) );
      }

      void operator()( const transfer_to_savings_operation& o )const
      {
         if( o.memo.length() > 0 )
            check_memo( o.memo,
                        _db.get< account_object, chain::by_name >( o.from ),
                        _db.get< account_authority_object, chain::by_account >( o.from ) );
      }

      void operator()( const transfer_from_savings_operation& o )const
      {
         if( o.memo.length() > 0 )
            check_memo( o.memo,
                        _db.get< account_object, chain::by_name >( o.from ),
                        _db.get< account_authority_object, chain::by_account >( o.from ) );
      }
   };

   void producer_plugin_impl::pre_transaction( const signed_transaction& trx )
   {
      const auto& _db = _self.database();
      flat_set< account_name_type > required; vector<authority> other;
      trx.get_required_authorities( required, required, required, other );

      auto trx_size = fc::raw::pack_size(trx);

      for( const auto& auth : required )
      {
         const auto& acnt = _db.get_account( auth );

         update_account_bandwidth( acnt, trx_size );
      }
   }

   void producer_plugin_impl::pre_operation( const operation_notification& note )
   {
      const auto& _db = _self.database();
      if( _db.is_producing() )
      {
         note.op.visit( operation_visitor( _db ) );
      }
   }

   void producer_plugin_impl::post_operation( const operation_notification& note )
   {
      const auto& db = _self.database();

      switch( note.op.which() )
      {
         case operation::tag< custom_operation >::value:
         case operation::tag< custom_json_operation >::value:
         {
            flat_set< account_name_type > impacted;
            app::operation_get_impacted_accounts( note.op, impacted );

            for( auto& account : impacted )
               if( db.is_producing() )
                  ASSERT( _dupe_customs.insert( account ).second, plugin_exception,
                     "Account ${a} already submitted a custom json operation this block.",
                     ("a", account) );
         }
            break;
         default:
            break;
      }
   }

   void producer_plugin_impl::on_block( const signed_block& b )
   {
      auto& db = _self.database();
      const dynamic_global_property_object& props = db.get_dynamic_global_properties();
      const median_chain_property_object& median_props = db.get_median_chain_properties();
      int64_t max_block_size = median_props.maximum_block_size;

      auto reserve_ratio_ptr = db.find( reserve_ratio_id_type() );

      if( BOOST_UNLIKELY( reserve_ratio_ptr == nullptr ) )
      {
         db.create< reserve_ratio_object >([&]( reserve_ratio_object &r )
         {
            r.average_block_size = 0;
            r.current_reserve_ratio = MAX_RESERVE_RATIO * RESERVE_RATIO_PRECISION;
            r.max_virtual_bandwidth = 
               ( uint128_t( MAX_BLOCK_SIZE ) * uint128_t( MAX_RESERVE_RATIO ) * uint128_t( BANDWIDTH_PRECISION.value ) * uint128_t( BANDWIDTH_AVERAGE_WINDOW.to_seconds() ) ) / 
               uint128_t( BLOCK_INTERVAL.to_seconds() );
         });
      }
      else
      {
         db.modify( *reserve_ratio_ptr, [&]( reserve_ratio_object& r )
         {
            // 100 Block moving average block size.
            r.average_block_size = ( 99 * r.average_block_size + fc::raw::pack_size( b ) ) / 100;

            /**
            * About once per minute the average network use is consulted and used to
            * adjust the reserve ratio. Anything above 25% usage reduces the reserve
            * ratio proportional to the distance from 25%. If usage is at 50% then
            * the reserve ratio will half. Likewise, if it is at 12% it will increase by 50%.
            *
            * If the reserve ratio is consistently low, then it is probably time to increase
            * the capcacity of the network.
            *
            * This algorithm is designed to react quickly to observations significantly
            * different from past observed behavior and make small adjustments when
            * behavior is within expected norms.
            */
            if( props.head_block_number % BLOCKS_PER_MINUTE == 0 )
            {
               int64_t distance = ( ( r.average_block_size - ( max_block_size / 4 ) ) * DISTANCE_CALC_PRECISION )
                  / ( max_block_size / 4 );
               auto old_reserve_ratio = r.current_reserve_ratio;

               if( distance > 0 )
               {
                  r.current_reserve_ratio -= ( r.current_reserve_ratio * distance ) / ( distance + DISTANCE_CALC_PRECISION );

                  // We do not want the reserve ratio to drop below 1
                  if( r.current_reserve_ratio < RESERVE_RATIO_PRECISION )
                  {
                     r.current_reserve_ratio = RESERVE_RATIO_PRECISION;
                  }
               }
               else
               {
                  // By default, we should always slowly increase the reserve ratio.
                  r.current_reserve_ratio += std::max( RESERVE_RATIO_MIN_INCREMENT, ( r.current_reserve_ratio * distance ) / ( distance - DISTANCE_CALC_PRECISION ) );

                  if( r.current_reserve_ratio > MAX_RESERVE_RATIO * RESERVE_RATIO_PRECISION )
                  {
                     r.current_reserve_ratio = MAX_RESERVE_RATIO * RESERVE_RATIO_PRECISION;
                  }
               }

               if( old_reserve_ratio != r.current_reserve_ratio )
               {
                  ilog( "Reserve ratio updated from ${old} to ${new}. Block: ${blocknum}",
                     ("old", old_reserve_ratio)
                     ("new", r.current_reserve_ratio)
                     ("blocknum", db.head_block_num()) );
               }

               r.max_virtual_bandwidth = ( uint128_t( max_block_size ) * uint128_t( r.current_reserve_ratio )
                                         * uint128_t( BANDWIDTH_PRECISION.value * BANDWIDTH_AVERAGE_WINDOW.count() ) )
                                         / ( BLOCK_INTERVAL.count() * RESERVE_RATIO_PRECISION );
            }
         });
      }

      _dupe_customs.clear();
   }

   void producer_plugin_impl::update_account_bandwidth( const account_object& a, uint32_t trx_size )
   {
      database& _db = _self.database();
      const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
      bool has_bandwidth = true;

      if( props.total_voting_power > 0 )
      {
         auto band = _db.find< account_bandwidth_object, by_account >( a.name );

         if( band == nullptr )
         {
            band = &_db.create< account_bandwidth_object >( [&]( account_bandwidth_object& b )
            {
               b.account = a.name;
            });
         }

         share_type new_bandwidth;
         share_type trx_bandwidth = share_type( trx_size ) * BANDWIDTH_PRECISION;
         fc::microseconds delta_time = _db.head_block_time() - band->last_bandwidth_update;

         if( delta_time > BANDWIDTH_AVERAGE_WINDOW )
         {
            new_bandwidth = 0;
         }
            
         else
         {
            new_bandwidth = ( ( ( BANDWIDTH_AVERAGE_WINDOW - delta_time ).to_seconds() * fc::uint128( band->average_bandwidth.value ) )
               / BANDWIDTH_AVERAGE_WINDOW.to_seconds() ).to_uint64();
         }
            

         new_bandwidth += trx_bandwidth;

         _db.modify( *band, [&]( account_bandwidth_object& b )
         {
            b.average_bandwidth = new_bandwidth;
            b.lifetime_bandwidth += trx_bandwidth;
            b.last_bandwidth_update = _db.head_block_time();
         });

         fc::uint128 account_voting_power( _db.get_voting_power(a).value );
         fc::uint128 total_voting_power( props.total_voting_power );
         fc::uint128 account_average_bandwidth( band->average_bandwidth.value );
         share_type account_lifetime_bandwidth( band->lifetime_bandwidth );
         fc::uint128 max_virtual_bandwidth( _db.get( reserve_ratio_id_type() ).max_virtual_bandwidth );
         // in effect
         // if a users voting power times the global max virtual bandwidth is great than the accounts average bandwidth 
         // times the total score of the network then it has enough bandwidth
         // max network bandwidth * account score // 50 * 10 = 500
         // account average bandwidth * total network score // 1 * 1000 = 1000
         // max network bandwidth * account score // 50 * 10 = 500
         // 528482304000000000000 * 0 = 0
         // needs to be greater than
         // account average bandwidth * total network score
         // 288000000 * 842277942443315491 = 2.4257604742367484e+26

         // This basically says that if it's the accounts first transaction they have the bandwidth.
         // This is a hack to get around not being able to claim score reward balance in the first place due to not having enough score to do so.....
//         if(trx_bandwidth.value != account_lifetime_bandwidth.value && trx_bandwidth.value != new_bandwidth.value){
//         }
         has_bandwidth = ( account_voting_power * max_virtual_bandwidth ) > ( account_average_bandwidth * total_voting_power );

         if( _db.is_producing() )
            ASSERT( has_bandwidth, chain::plugin_exception,
               "Account: ${account} bandwidth limit exceeded. Please wait to transact or increase staked balance.",
               ("account", a.name)
               ("account_voting_power", account_voting_power)
               ("account_average_bandwidth", account_average_bandwidth)
               ("max_virtual_bandwidth", max_virtual_bandwidth)
               ("total_voting_power", total_voting_power) );
      }
   }
}

producer_plugin::producer_plugin( application* app )
   : plugin( app ), _my( new detail::producer_plugin_impl( *this ) ) {}

producer_plugin::~producer_plugin()
{
   try
   {
      if( _block_production_task.valid() )
         _block_production_task.cancel_and_wait(__FUNCTION__);
   }
   catch(fc::canceled_exception&)
   {
      //Expected exception. Move along.
   }
   catch(fc::exception& e)
   {
      edump((e.to_detail_string()));
   }
}

void producer_plugin::plugin_set_program_options(
   boost::program_options::options_description& command_line_options,
   boost::program_options::options_description& config_file_options)
{
   string producer_id_example = "init_producer";
   command_line_options.add_options()
      ("enable-stale-production", bpo::bool_switch()->notifier([this](bool e){_production_enabled = e;}), "Enable block production, even if the chain is stale.")
      ("required-participation", bpo::bool_switch()->notifier([this](int e){_required_producer_participation = uint32_t(e*PERCENT_1);}), "Percent of producers (0-99) that must be participating in order to produce blocks")
      ("producer,w", bpo::value<vector<string>>()->composing()->multitoken(),
      ("name of producer controlled by this node (e.g. " + producer_id_example+" )" ).c_str())
      ("private-key", bpo::value<vector<string>>()->composing()->multitoken(), "WIF PRIVATE KEY to be used by one or more producers or miners" )
      ("commit-asset", bpo::value<int64_t>(), "Asset value to be committed in validations by producers" )
      ("mining-producer", bpo::value<string>(), "Account used for mining proofs of work" )
      ("mining-props", bpo::value<string>(), "Blockchain Properties used when mining" )
      ;
   config_file_options.add(command_line_options);
}

std::string producer_plugin::plugin_name()const
{
   return "producer";
}

using std::vector;
using std::pair;
using std::string;

void producer_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{ try {
   _options = &options;
   LOAD_VALUE_SET(options, "producer", _producers, string)

   if( options.count("private-key") )
   {
      const std::vector<std::string> keys = options["private-key"].as< std::vector< std::string > >();
      for (const std::string& wif_key : keys )
      {
         fc::optional<fc::ecc::private_key> private_key = graphene::utilities::wif_to_key(wif_key);
         FC_ASSERT( private_key.valid(), "unable to parse private key" );
         _private_keys[private_key->get_public_key()] = *private_key;
      }
   }

   if( options.count("commit-coin-amount") )
   {
      share_type commit_coin_amount = options["commit-coin-amount"].as< share_type >();

      if( commit_coin_amount > 0 )
      {
         _commit_coin_amount = asset( commit_coin_amount * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      }
   }

   if( options.count("mining-threads") > 0 )
   {
      wlog( "Initializing ${n} mining threads", ("n", _my->_mining_threads) );
      _my->_mining_threads = options.at("mining-threads").as< uint32_t >();
      _my->_thread_pool.resize( _my->_mining_threads );
      for( uint32_t i = 0; i < _my->_mining_threads; ++i )
      {
         _my->_thread_pool[i] = std::make_shared<fc::thread>();
      }
   }

   if( options.count("mining-producer") )
   {
      _mining_producer = options.at("mining-producer").as< std::string >();
   }

   if( options.count("mining-props") )
   {
      std::string props = options.at("mining-props").as< std::string >();
      _mining_props = fc::json::from_string( props ).as< chain::chain_properties >();
   }

   chain::database& db = database();

   db.post_apply_operation.connect( [&]( const operation_notification& note ){ _my->post_operation( note ); } );
   db.pre_apply_block.connect( [&]( const signed_block& b ){ _my->pre_apply_block( b ); } );
   db.on_pre_apply_transaction.connect( [&]( const signed_transaction& tx ){ _my->pre_transaction( tx ); } );
   db.pre_apply_operation.connect( [&]( const operation_notification& note ){ _my->pre_operation( note ); } );
   db.applied_block.connect( [&]( const signed_block& b ){ _my->on_block( b ); } );

   add_plugin_index< account_bandwidth_index >( db );
   add_plugin_index< content_edit_lock_index >( db );
   add_plugin_index< reserve_ratio_index     >( db );
} FC_LOG_AND_RETHROW() }

void producer_plugin::plugin_startup()
{ try {
   ilog("producer plugin:  plugin_startup() begin");
   chain::database& d = database();

   if( !_producers.empty() )
   {
      ilog("Launching block production for ${n} producers.", ("n", _producers.size()));
      idump( (_producers) );
      app().set_block_production(true);
      if( _production_enabled )
      {
         if( d.head_block_num() == 0 )
         {
            new_chain_banner( _private_keys );
         }
         _production_skip_flags |= node::chain::database::skip_undo_history_check;
      }
      schedule_production_loop();
   }
   else
   {
      elog("No producers configured! Please add producer names and private keys to configuration.");
   }
   ilog("producer plugin:  plugin_startup() end");
} FC_CAPTURE_AND_RETHROW() }

void producer_plugin::plugin_shutdown()
{
   return;
}



   //======================================//
   // ===== Block Production Methods ===== //
   //======================================//



void producer_plugin::schedule_production_loop()
{
   // Schedule for the next tick, If we would wait less than 20ms, wait for the whole Tick.

   fc::time_point fc_now = fc::time_point::now();

   fc::microseconds time_to_next_tick = PRODUCER_TICK_INTERVAL - fc::microseconds( fc_now.time_since_epoch().count() % PRODUCER_TICK_INTERVAL.count() );

   // Sleep for at least 20ms

   if( time_to_next_tick < fc::microseconds( 20000 ) )
   {
      time_to_next_tick += PRODUCER_TICK_INTERVAL;
   }   
      
   fc::time_point next_wakeup( fc_now + time_to_next_tick );

   _block_production_task = fc::schedule([this]
   { 
      block_production_loop(); 
   }, next_wakeup, "producer Block Production");
}

block_production_condition::block_production_condition_enum producer_plugin::block_production_loop()
{
   if( fc::time_point::now() < fc::time_point(GENESIS_TIME) )
   {
      wlog( "waiting until genesis time to produce block: ${t}", ("t",GENESIS_TIME) );
      schedule_production_loop();
      return block_production_condition::wait_for_genesis;
   }

   block_production_condition::block_production_condition_enum result;
   fc::mutable_variant_object capture;

   try
   {
      result = maybe_produce_block(capture);
   }
   catch( const fc::canceled_exception& )
   {
      //We're trying to exit. Go ahead and let this one out.
      throw;
   }
   catch( const node::chain::unknown_hardfork_exception& e )
   {
      // Hit a hardfork that the current node know nothing about, stop production and inform user
      elog( "${e}\nNode may be out of date...", ("e", e.to_detail_string()) );
      throw;
   }
   catch( const fc::exception& e )
   {
      elog("Got exception while generating block:\n${e}", ("e", e.to_detail_string()));
      result = block_production_condition::exception_producing_block;
   }

   switch( result )
   {
      case block_production_condition::produced:
         ilog("Producer: ${w} Successfully Generated block #${n} with timestamp ${t}.", (capture));
         break;
      case block_production_condition::not_synced:
         ilog("Not Producing Block: Production is disabled until we receive a recent block.");
         break;
      case block_production_condition::not_my_turn:
         ilog("Not Producing Block: Awaiting Allocated production time slot.");
         break;
      case block_production_condition::not_time_yet:
         ilog("Not Producing Block: Production Slot has not yet arrived.");
         break;
      case block_production_condition::no_private_key:
         ilog("Not Producing Block: Node with Producer: ${sw} does not have access to the private key for Publick key: ${sk}", (capture) );
         break;
      case block_production_condition::low_participation:
         elog("Not Producing Block: Node appears to be on a minority fork with only ${pct}% producer participation.", (capture) );
         break;
      case block_production_condition::lag:
         elog("Not Producing Block: Node unresponsive within 500ms of the allocated slot time. Actual response time: ${lag} ms");
         break;
      case block_production_condition::consecutive:
         elog("Not Producing Block: Last block was generated by the same producer.\nThis node is probably disconnected from the network so block production has been disabled.\nDisable this check with --allow-consecutive option.");
         break;
      case block_production_condition::exception_producing_block:
         elog("Failure when producing block with no transactions");
         break;
      case block_production_condition::wait_for_genesis:
         break;
   }

   schedule_production_loop();
   return result;
}


block_production_condition::block_production_condition_enum producer_plugin::maybe_produce_block( fc::mutable_variant_object& capture )
{
   chain::database& db = database();
   fc::time_point now_fine = fc::time_point::now();
   fc::time_point now = now_fine + fc::microseconds( 500000 );

   if( !_production_enabled )
   {
      if( db.get_slot_time(1) >= now )
      {
         _production_enabled = true;
      } 
      else
      {
         return block_production_condition::not_synced;
      } 
   }
 
   uint32_t slot = db.get_slot_at_time( now );
   if( slot == 0 )
   {
      capture("next_time", db.get_slot_time(1));
      return block_production_condition::not_time_yet;
   }

   assert( now > db.head_block_time() );

   string scheduled_producer = db.get_scheduled_producer( slot );
	ilog("the scheduled_producer is ${producer}", ("producer", scheduled_producer));
   
   if( _producers.find( scheduled_producer ) == _producers.end() )
   {
      capture("sw", scheduled_producer);
      return block_production_condition::not_my_turn;
   }

   const auto& producer_by_name = db.get_index< chain::producer_index >().indices().get< chain::by_name >();
   auto itr = producer_by_name.find( scheduled_producer );

   fc::time_point scheduled_time = db.get_slot_time( slot );
   node::protocol::public_key_type scheduled_key = itr->signing_key;
   auto private_key_itr = _private_keys.find( scheduled_key );

   if( private_key_itr == _private_keys.end() )
   {
      capture("sw", scheduled_producer);
      capture("sk", scheduled_key);
      return block_production_condition::no_private_key;
   }

   uint32_t prate = db.producer_participation_rate();
   if( prate < _required_producer_participation )
   {
      capture("pct", uint32_t(100*uint64_t(prate) / PERCENT_1));
      return block_production_condition::low_participation;
   }

   if( llabs((scheduled_time - now).count()) > fc::milliseconds( 500 ).count() )
   {
      capture("st", scheduled_time)("now", now)("lag", (scheduled_time - now).count()/1000 );
      return block_production_condition::lag;
   }

   int retry = 0;
   do
   {
      try
      {
      signed_block block = db.generate_block(
         scheduled_time,
         scheduled_producer,
         private_key_itr->second,
         _production_skip_flags
         );
         capture("n", block.block_num())("t", block.timestamp)("c", now)("w",scheduled_producer);
         fc::async( [this,block](){ p2p_node().broadcast(graphene::net::block_message(block)); } );

         return block_production_condition::produced;
      }
      catch( fc::exception& e )
      {
         elog( "${e}", ("e",e.to_detail_string()) );
         elog( "Clearing pending transactions and attempting again" );
         db.clear_pending();
         retry++;
      }
   } while( retry < 2 );

   return block_production_condition::exception_producing_block;
}



   //============================//
   // ===== Mining Methods ===== //
   //============================//



void producer_plugin::schedule_mining_loop()
{
   // Schedule for the next tick, If we would wait less than 20ms, wait for the whole Tick.

   fc::time_point fc_now = fc::time_point::now();

   fc::microseconds time_to_next_tick = MINING_TICK_INTERVAL - fc::microseconds( fc_now.time_since_epoch().count() % MINING_TICK_INTERVAL.count() );

   // Sleep for at least 20ms

   if( time_to_next_tick < fc::microseconds( 20000 ) )
   {
      time_to_next_tick += MINING_TICK_INTERVAL;
   }   
      
   fc::time_point next_wakeup( fc_now + time_to_next_tick );

   _block_mining_task = fc::schedule([this]
   { 
      block_mining_loop(); 
   }, next_wakeup, "producer Block Production");
}

block_mining_condition::block_mining_condition_enum producer_plugin::block_mining_loop()
{
   if( fc::time_point::now() < fc::time_point(GENESIS_TIME) )
   {
      wlog( "waiting until genesis time to mine Proof of Work: ${t}", ("t",GENESIS_TIME) );
      schedule_mining_loop();
      return block_mining_condition::wait_for_genesis;
   }

   block_mining_condition::block_mining_condition_enum result;
   fc::mutable_variant_object capture;
   
   try
   {
      result = maybe_mine_proof_of_work(capture);
   }
   catch( const fc::canceled_exception& )
   {
      //We're trying to exit. Go ahead and let this one out.
      throw;
   }
   catch( const node::chain::unknown_hardfork_exception& e )
   {
      // Hit a hardfork that the current node know nothing about, stop mining and inform user
      elog( "${e}\nNode may be out of date...", ("e", e.to_detail_string()) );
      throw;
   }
   catch( const fc::exception& e )
   {
      elog("Got exception while generating block:\n${e}", ("e", e.to_detail_string()));
      result = block_mining_condition::exception_mining_block;
   }

   switch( result )
   {
      case block_mining_condition::mined:
         ilog("Producer: ${w} Successfully Generated Proof of Work on previous Block ID: ${id} with nonce ${n} with timestamp: ${t}.", (capture));
         break;
      case block_mining_condition::not_synced:
         ilog("Not Mining Block: Production is disabled until we receive a recent block.");
         break;
      case block_mining_condition::not_my_turn:
         ilog("Not Mining Block: Awaiting Allocated production time slot.");
         break;
      case block_mining_condition::not_time_yet:
         ilog("Not Mining Block: Production Slot has not yet arrived.");
         break;
      case block_mining_condition::no_private_key:
         ilog("Not Mining Block: Node with Producer: ${sw} does not have access to the private key for Public key: ${sk}", (capture) );
         break;
      case block_mining_condition::low_participation:
         elog("Not Mining Block: Node appears to be on a minority fork with only ${pct}% producer participation.", (capture) );
         break;
      case block_mining_condition::lag:
         elog("Not Mining Block: Node unresponsive within 500ms of the allocated slot time. Actual response time: ${lag} ms", (capture));
         break;
      case block_mining_condition::consecutive:
         elog("Not Mining Block: Last block was generated by the same producer.\nThis node is probably disconnected from the network so block production has been disabled.\nDisable this check with --allow-consecutive option.");
         break;
      case block_mining_condition::exception_mining_block:
         elog("Failure when Mining block");
         break;
      case block_mining_condition::wait_for_genesis:
         break;
   }

   schedule_mining_loop();
   return result;
}

/**
 * Generates a Proof of Work Operation
 */
block_mining_condition::block_mining_condition_enum producer_plugin::maybe_mine_proof_of_work( fc::mutable_variant_object& capture )
{
   chain::database& db = database();
   fc::time_point now_fine = fc::time_point::now();
   fc::time_point now = now_fine + fc::microseconds( 500000 );

   if( !_production_enabled )
   {
      if( db.get_slot_time(1) >= now )
      {
         _production_enabled = true;
      }
      else
      {
         return block_mining_condition::not_synced;
      }
   }

   uint32_t slot = db.get_slot_at_time( now );
   if( slot == 0 )
   {
      capture("next_time", db.get_slot_time(1));
      return block_mining_condition::not_time_yet;
   }

   assert( now > db.head_block_time() );

   string scheduled_producer = db.get_scheduled_producer( slot );
   fc::time_point scheduled_time = db.get_slot_time( slot );
   uint32_t prate = db.producer_participation_rate();

   if( prate < _required_producer_participation )
   {
      capture("pct", uint32_t(100*uint64_t(prate) / PERCENT_1));
      return block_mining_condition::low_participation;
   }

   if( llabs((scheduled_time - now).count()) > fc::milliseconds( 500 ).count() )
   {
      capture("st", scheduled_time)("now", now)("lag", (scheduled_time - now).count()/1000 );
      return block_mining_condition::lag;
   }

   chain::x11_proof_of_work work;
   work.input.miner_account = _mining_producer;
   work.input.prev_block = db.head_block_id();
   mine_proof_of_work( work, db.pow_difficulty() );

   chain::proof_of_work_operation op;

   op.work = work;
   op.props = _mining_props;
   
   const auto& acct_idx  = db.get_index< chain::account_index >().indices().get< chain::by_name >();
   auto acct_it = acct_idx.find( work.input.miner_account );
   auto acct_auth = db.find< chain::account_authority_object, chain::by_account >( work.input.miner_account );

   fc::optional< fc::ecc::private_key > priv;

   if( !_private_keys.size() )
   {
      return block_mining_condition::no_private_key;
   }
   
   if( acct_it == acct_idx.end() )
   {
      chain::public_key_type pubkey = _private_keys.begin()->first;
      op.new_owner_key = pubkey;
      priv = _private_keys[ pubkey ];
   }
   else
   {
      FC_ASSERT( acct_auth->active_auth.key_auths.size() == 1,
         "Should not use multiple active key auths in a mining account." );
      chain::public_key_type pubkey = acct_auth->active_auth.key_auths.begin()->first;
      FC_ASSERT( _private_keys.find( pubkey ) != _private_keys.end(), 
         "Cannot find the Active key authority of the specifed mining producer.");
      op.new_owner_key = pubkey;
      priv = _private_keys[ pubkey ];
   }

   FC_ASSERT( priv.valid(), 
      "Must have access to active private key for mining producer: ${miner}", ("miner", work.input.miner_account) );

   chain::signed_transaction trx;

   op.validate();
   trx.operations.push_back(op);
   trx.ref_block_num = db.head_block_num();
   trx.ref_block_prefix = work.input.prev_block._hash[1];
   trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
   trx.sign( *priv, CHAIN_ID );

   int retry = 0;
   do
   {
      try
      {
         db.push_transaction( trx, 0 );
         fc::async( [this,trx](){ p2p_node().broadcast_transaction( trx ); } );
         capture("n", work.input.nonce)("id", work.input.prev_block)("c", now)("w",work.input.miner_account);
         return block_mining_condition::mined;
      }
      catch( fc::exception& e )
      {
         elog( "${e}", ("e",e.to_detail_string()) );
         elog( "Clearing pending transactions and attempting again" );
         db.clear_pending();
         retry++;
      }
   } while( retry < 2 );
   
   return block_mining_condition::exception_mining_block;
}


struct mining_state
{
   mining_state();
   virtual ~mining_state();

   std::string                    miner_account;          ///< Mining account of this producer node. 

   chain::block_id_type           prev_block;             ///< Recently received block. Must be at least Irreversible block num.

   uint128_t                      summary_target = 0;     ///< Current Mining Difficulty of the network.

   fc::promise< chain::x11_proof_of_work >::ptr work;     ///< Promise to be resolved when a valid Proof of Work is found.

   fc::mutex                      set_work_mutex;         ///< Mutex lock on resolving the Work promise to ensure thread coherence.
};

mining_state::mining_state() {}
mining_state::~mining_state() {}


void producer_plugin::mine_proof_of_work( chain::x11_proof_of_work& work, uint128_t summary_target )
{
   std::shared_ptr< mining_state > mine_state = std::make_shared< mining_state >();
   
   mine_state->miner_account = work.input.miner_account;
   mine_state->prev_block = work.input.prev_block;
   mine_state->summary_target = summary_target;
   mine_state->work = fc::promise< chain::x11_proof_of_work >::ptr( new fc::promise< chain::x11_proof_of_work >() );
   
   uint32_t thread_num = 0;
   uint32_t num_threads = _my->_mining_threads;

   wlog( "Mining for worker account ${a} on block ${b} with target ${t} using ${n} threads",
      ("a", work.input.miner_account) ("b", work.input.prev_block) ("c", summary_target) ("n", num_threads) ("t", summary_target) );

   uint32_t nonce_start = 0;

   for( auto& t : _my->_thread_pool )
   {
      uint32_t nonce_offset = nonce_start + thread_num;
      uint32_t nonce_stride = num_threads;

      wlog( "Launching thread ${i}", ("i", thread_num) );

      t->async( [ mine_state, nonce_offset, nonce_stride ]()
      {
         chain::x11_proof_of_work work;
         std::string miner_account = mine_state->miner_account;
         chain::block_id_type prev_block = mine_state->prev_block;
         uint128_t summary_target = mine_state->summary_target;

         wlog( "Starting thread mining at offset ${o}", ("o", nonce_offset) );

         work.input.prev_block = prev_block;
         work.input.miner_account = miner_account;
         work.input.nonce = nonce_offset;
         time_point thread_start = time_point::now();
         uint128_t hashrate = 0;

         while( !(mine_state->work->ready()) )
         {
            work.create( prev_block, miner_account, work.input.nonce );

            if( work.pow_summary < summary_target )
            {
               fc::scoped_lock< fc::mutex > lock(mine_state->set_work_mutex);     // Lock mutex to ensure other threads cannot overwrite data.

               if( !mine_state->work->ready() )
               {
                  mine_state->work->set_value( work );      // Resolve the Work promise value with the valid proof of work.
                  wlog( "||=======================================||");
                  wlog( "|| ===== Found Valid Proof of Work ===== ||" );
                  wlog( "||=======================================||" );
                  wlog( "Successful Nonce: ${n}", ("n", work.input.nonce) );
               }
               break;
            }
            else if( work.pow_summary < ( summary_target * 10 ) )
            {
               hashrate = work.input.nonce * fc::seconds(1).count() / ( time_point::now() - thread_start ).count();
               wlog( "--- [10%] Found High Intermediate Proof of Work with nonce: ${n}", ("n", work.input.nonce) );
               wlog( "Approximate Hashrate: ${h} Hashes Per Second.", ("h", hashrate ) );
            }
            else if( work.pow_summary < ( summary_target * 100 ) )
            {
               hashrate = work.input.nonce * fc::seconds(1).count() / ( time_point::now() - thread_start ).count();
               wlog( "-- [1%] Found Medium Intermediate Proof of Work with nonce: ${n}", ("n", work.input.nonce) );
               wlog( "Approximate Hashrate: ${h} Hashes Per Second.", ("h", hashrate ) );
            }
            else if( work.pow_summary < ( summary_target * 1000 ) )
            {
               hashrate = work.input.nonce * fc::seconds(1).count() / ( time_point::now() - thread_start ).count();
               wlog( "- [0.1%] Found Low Intermediate Proof of Work with nonce: ${n}", ("n", work.input.nonce) );
               wlog( "Approximate Hashrate: ${h} Hashes Per Second.", ("h", hashrate ) );
            }

            work.input.nonce += nonce_stride;
         }
         return;
      });
      
      ++thread_num;
   }

   work = mine_state->work->wait();

   wlog( "Finished: Proof of Work = ${w}", ("w", work) );
   return;
}



   //================================//
   // ===== Validation Methods ===== //
   //================================//


void producer_plugin::schedule_validation_loop()
{
   // Schedule for the next tick, If we would wait less than 20ms, wait for the whole Tick.

   fc::time_point fc_now = fc::time_point::now();

   fc::microseconds time_to_next_tick = VALIDATION_TICK_INTERVAL - fc::microseconds( fc_now.time_since_epoch().count() % VALIDATION_TICK_INTERVAL.count() );

   // Sleep for at least 20ms

   if( time_to_next_tick < fc::microseconds( 20000 ) )
   {
      time_to_next_tick += VALIDATION_TICK_INTERVAL;
   }   
      
   fc::time_point next_wakeup( fc_now + time_to_next_tick );

   _block_validation_task = fc::schedule([this]
   { 
      block_validation_loop(); 
   }, next_wakeup, "Producer Validation");
}

block_validation_condition::block_validation_condition_enum producer_plugin::block_validation_loop()
{
   if( fc::time_point::now() < fc::time_point(GENESIS_TIME) )
   {
      wlog( "waiting until genesis time to validate blocks: ${t}", ("t",GENESIS_TIME) );
      schedule_validation_loop();
      return block_validation_condition::wait_for_genesis;
   }

   block_validation_condition::block_validation_condition_enum result;
   fc::mutable_variant_object capture;
   
   try
   {
      result = maybe_validate_blocks(capture);
   }
   catch( const fc::canceled_exception& )
   {
      //We're trying to exit. Go ahead and let this one out.
      throw;
   }
   catch( const node::chain::unknown_hardfork_exception& e )
   {
      // Hit a hardfork that the current node know nothing about, stop validation and inform user
      elog( "${e}\nNode may be out of date...", ("e", e.to_detail_string()) );
      throw;
   }
   catch( const fc::exception& e )
   {
      elog("Got exception while generating block:\n${e}", ("e", e.to_detail_string()));
      result = block_validation_condition::exception_validating_block;
   }

   switch( result )
   {
      case block_validation_condition::fully_validated:
         ilog("Successfully Broadcasted Verification and Commit transactions for all active producers.");
         break;
      case block_validation_condition::not_synced:
         ilog("Not Validating Block: Production is disabled until we receive a recent block.");
         break;
      case block_validation_condition::not_my_turn:
         ilog("Not Validating Block: Awaiting Allocated production time slot.");
         break;
      case block_validation_condition::not_time_yet:
         ilog("Not Validating Block: Production Slot has not yet arrived.");
         break;
      case block_validation_condition::no_private_key:
         ilog("Not Validating Block: Node with Producer: ${sw} does not have access to the private key for Public key: ${sk}", (capture) );
         break;
      case block_validation_condition::low_participation:
         elog("Not Validating Block: Node appears to be on a minority fork with only ${pct}% producer participation.", (capture) );
         break;
      case block_validation_condition::lag:
         elog("Not Validating Block: Node unresponsive within 500ms of the allocated slot time. Actual response time: ${lag} ms", (capture));
         break;
      case block_validation_condition::consecutive:
         elog("Not Validating Block: Last block was generated by the same producer.\nThis node is probably disconnected from the network so block production has been disabled.\nDisable this check with --allow-consecutive option.");
         break;
      case block_validation_condition::exception_validating_block:
         elog("Failure when Validating");
         break;
      case block_validation_condition::wait_for_genesis:
         break;
   }

   schedule_validation_loop();
   return result;
}


block_validation_condition::block_validation_condition_enum producer_plugin::maybe_validate_blocks( fc::mutable_variant_object& capture )
{
   chain::database& db = database();
   fc::time_point now_fine = fc::time_point::now();
   fc::time_point now = now_fine + fc::microseconds( 500000 );
   const producer_schedule_object& pso = db.get_producer_schedule();
   const dynamic_global_property_object& dgpo = db.get_dynamic_global_properties();

   if( !_production_enabled )
   {
      if( db.get_slot_time(1) >= now )
      {
         _production_enabled = true;
      }
      else
      {
         return block_validation_condition::not_synced;
      }
   }

   uint32_t slot = db.get_slot_at_time( now );
   if( slot == 0 )
   {
      capture("next_time", db.get_slot_time(1));
      return block_validation_condition::not_time_yet;
   }

   assert( now > db.head_block_time() );

   string scheduled_producer = db.get_scheduled_producer( slot );
   fc::time_point scheduled_time = db.get_slot_time( slot );
   uint32_t prate = db.producer_participation_rate();

   if( prate < _required_producer_participation )
   {
      capture("pct", uint32_t(100*uint64_t(prate) / PERCENT_1));
      return block_validation_condition::low_participation;
   }

   if( llabs((scheduled_time - now).count()) > fc::milliseconds( 500 ).count() )
   {
      capture("st", scheduled_time)("now", now)("lag", (scheduled_time - now).count()/1000 );
      return block_validation_condition::lag;
   }

   vector< block_id_type > recent_block_ids;

   for( uint64_t i = dgpo.last_irreversible_block_num + 1; i < dgpo.head_block_number; i++ )
   {
      recent_block_ids.push_back( db.get_block_id_for_num( i ) );
   }

   const auto& validation_idx = db.get_index< chain::block_validation_index >().indices().get< chain::by_block_id >();
   auto validation_itr = validation_idx.begin();

   flat_map< block_id_type, flat_set< transaction_id_type > > verifying_transactions;

   size_t min_verifiers = ( IRREVERSIBLE_THRESHOLD * ( DPOS_VOTING_PRODUCERS + POW_MINING_PRODUCERS ) / PERCENT_100 );

   for( block_id_type id : recent_block_ids )
   {
      validation_itr = validation_idx.lower_bound( id );
      flat_set< transaction_id_type > txns;
      while( validation_itr != validation_idx.end() &&
         validation_itr->block_id == id )
      {
         txns.insert( validation_itr->verify_txn );
         ++validation_itr;
      }
      if( txns.size() >= min_verifiers )
      {
         verifying_transactions[ id ] = txns;
      }
   }
   
   const auto& producer_idx = db.get_index< chain::producer_index >().indices().get< chain::by_name >();
   auto producer_itr = producer_idx.begin();

   uint16_t verified_blocks;
   flat_set< account_name_type > completed_producers;

   for( auto p : _producers )
   {
      producer_itr = producer_idx.find( p );
      if( producer_itr != producer_idx.end() )
      {
         if( pso.is_top_producer( producer_itr->owner ) )
         {
            const account_authority_object& prod_auth = db.get_account_authority( producer_itr->owner );
            vector< public_key_type > prod_active_keys = prod_auth.active_auth.get_keys();
            signed_transaction trx;

            trx.ref_block_num = db.head_block_num();
            trx.ref_block_prefix = db.head_block_id()._hash[1];
            trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

            verify_block_operation verify;
            verify.signatory = producer_itr->owner;
            verify.producer = producer_itr->owner;

            commit_block_operation commit;
            commit.signatory = producer_itr->owner;
            commit.producer = producer_itr->owner;

            for( block_id_type block_id : recent_block_ids )
            {
               const block_validation_object* valid_ptr = db.find_block_validation( producer_itr->owner, block_header::num_from_id( block_id ) );
               if( valid_ptr == nullptr )
               {
                  verify.block_id = block_id;
                  verify.validate();
                  trx.operations.push_back( verify );
               }
            }

            for( auto txn_ids : verifying_transactions )
            {
               commit.block_id = txn_ids.first;
               commit.verifications = txn_ids.second;
               commit.commitment_stake = _commit_coin_amount;
               commit.validate();
               trx.operations.push_back( commit );
            }

            for( auto key : prod_active_keys )
            {
               auto private_key_itr = _private_keys.find( key );
               if( private_key_itr != _private_keys.end() )
               {
                  trx.sign( _private_keys[ key ], db.get_chain_id() );
               }
            }

            verified_blocks += trx.operations.size();

            int retry = 0;
            do
            {
               try
               {
                  db.push_transaction( trx, 0 );
                  fc::async( [this,trx](){ p2p_node().broadcast_transaction( trx ); } );
                  completed_producers.insert( producer_itr->owner );
                  break;
               }
               catch( fc::exception& e )
               {
                  elog( "${e}", ("e",e.to_detail_string()) );
                  elog( "Clearing pending transactions and attempting again" );
                  db.clear_pending();
                  verified_blocks -= trx.operations.size();
                  retry++;
               }
            } while( retry < 2 );
         }
      }
   }

   if( completed_producers.size() == _producers.size() )
   {
      return block_validation_condition::fully_validated;
   }
   else
   {
      return block_validation_condition::exception_validating_block;
   }
}

} } // node::producer

DEFINE_PLUGIN( producer, node::producer::producer_plugin )