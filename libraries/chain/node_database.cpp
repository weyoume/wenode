#include <node/protocol/node_operations.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/custom_operation_interpreter.hpp>
#include <node/chain/database.hpp>
#include <node/chain/database_exceptions.hpp>
#include <node/chain/db_with.hpp>
#include <node/chain/evaluator_registry.hpp>
#include <node/chain/global_property_object.hpp>
#include <node/chain/history_object.hpp>
#include <node/chain/index.hpp>
#include <node/chain/node_evaluator.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/transaction_object.hpp>
#include <node/chain/shared_db_merkle.hpp>
#include <node/chain/operation_notification.hpp>
#include <node/chain/witness_schedule.hpp>
#include <node/witness/witness_objects.hpp>

#include <node/chain/util/asset.hpp>
#include <node/chain/util/reward.hpp>
#include <node/chain/util/uint256.hpp>
#include <node/chain/util/reward.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/uint128.hpp>

#include <fc/container/deque.hpp>

#include <fc/io/fstream.hpp>

#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>

namespace node { namespace chain {

//namespace db2 = graphene::db2;

struct object_schema_repr
{
   std::pair< uint16_t, uint16_t > space_type;
   std::string type;
};

struct operation_schema_repr
{
   std::string id;
   std::string type;
};

struct db_schema
{
   std::map< std::string, std::string > types;
   std::vector< object_schema_repr > object_types;
   std::string operation_type;
   std::vector< operation_schema_repr > custom_operation_types;
};

} }

FC_REFLECT( node::chain::object_schema_repr, (space_type)(type) )
FC_REFLECT( node::chain::operation_schema_repr, (id)(type) )
FC_REFLECT( node::chain::db_schema, (types)(object_types)(operation_type)(custom_operation_types) )

namespace node { namespace chain {

using boost::container::flat_set;

class database_impl
{
   public:
      database_impl( database& self );

      database&                              _self;
      evaluator_registry< operation >        _evaluator_registry;
};

database_impl::database_impl( database& self )
   : _self(self), _evaluator_registry(self) {}

database::database()
   : _my( new database_impl(*this) ) {}

database::~database()
{
   clear_pending();
}

void database::open( const fc::path& data_dir, const fc::path& shared_mem_dir, uint64_t shared_file_size, uint32_t chainbase_flags )
{ try {
   init_schema();
   chainbase::database::open( shared_mem_dir, chainbase_flags, shared_file_size );

   initialize_indexes();
   initialize_evaluators();

   if( chainbase_flags & chainbase::database::read_write )
   {
      if( !find< dynamic_global_property_object >() )
         with_write_lock( [&]()
         {
            init_genesis();
         });

      _block_log.open( data_dir / "block_log" );

      auto log_head = _block_log.head();

      // Rewind all undo state. This should return us to the state at the last irreversible block.
      with_write_lock( [&]()
      {
         undo_all();
         FC_ASSERT( revision() == head_block_num(), "Chainbase revision does not match head block num",
            ("rev", revision())("head_block", head_block_num()) );
         validate_invariants();
      });

      if( head_block_num() )
      {
         auto head_block = _block_log.read_block_by_num( head_block_num() );
         // This assertion should be caught and a reindex should occur
         FC_ASSERT( head_block.valid() && head_block->id() == head_block_id(), "Chain state does not match block log. Please reindex blockchain." );

         _fork_db.start_block( *head_block );
      }
   }

   with_read_lock( [&]()
   {
      init_hardforks(); // Writes to local state, but reads from db
   });
} FC_CAPTURE_LOG_AND_RETHROW( (data_dir)(shared_mem_dir)(shared_file_size) ) }

/**
 * Creates a new blockchain from the genesis block, 
 * creates all necessary network objects.
 * Generates initial assets, accounts, witnesses, boards
 * and sets initial global dynamic properties. 
 */
void database::init_genesis()
{ try {
   struct auth_inhibitor
   {
      auth_inhibitor(database& db) : db(db), old_flags(db.node_properties().skip_flags)
      { db.node_properties().skip_flags |= skip_authority_check; }
      ~auth_inhibitor()
      { db.node_properties().skip_flags = old_flags; }
   private:
      database& db;
      uint32_t old_flags;
   } inhibitor(*this);

   // Create blockchain accounts
   public_key_type      init_public_key(INIT_PUBLIC_KEY);

   time_point now = head_block_time();

   create< account_object >( [&]( account_object& a )
   {
      a.name = INIT_ACCOUNT;
      a.account_type = BUSINESS; 
      a.secure_public_key = init_public_key;
      a.created = now;
      a.last_account_update = now;
      a.last_vote_time = now;
      a.last_post = now;
      a.last_root_post = now;
      a.last_transfer = now;
      a.last_owner_proved = now;
      a.last_active_proved = now;
      a.last_activity_reward = now;
      a.last_account_recovery = now;
      a.membership = TOP_MEMBERSHIP;
      a.membership_expiration = time_point::maximum();
      a.mined = true;

   });
   create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = INIT_ACCOUNT;
      auth.owner.add_authority( init_public_key, 1 );
      auth.owner.weight_threshold = 1;
      auth.active  = auth.owner;
      auth.posting = auth.active;
      auth.active.weight_threshold = 1;
   });

   create< account_object >( [&]( account_object& a )
   {
      a.name = WITNESS_ACCOUNT;
   });
   create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = WITNESS_ACCOUNT;
      auth.owner.weight_threshold = 1;
      auth.active.weight_threshold = 1;
   });

   create< account_object >( [&]( account_object& a )
   {
      a.name = NULL_ACCOUNT;
   });
   create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = NULL_ACCOUNT;
      auth.owner.weight_threshold = 1;
      auth.active.weight_threshold = 1;
   });

   create< account_object >( [&]( account_object& a )
   {
      a.name = TEMP_ACCOUNT;
   });
   create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = TEMP_ACCOUNT;
      auth.owner.weight_threshold = 0;
      auth.active.weight_threshold = 0;
   });

   // Create core asset
   
   create<asset_object>( []( asset_object& a ) 
   {
      a.symbol = SYMBOL_COIN;
      a.options.max_supply = MAX_ASSET_SUPPLY;
      a.asset_type = CURRENCY_ASSET;
      a.options.flags = 0;
      a.options.issuer_permissions = 0;
      a.issuer = NULL_ACCOUNT;
      a.options.core_exchange_rate.base.amount = 1;
      a.options.core_exchange_rate.base.symbol = SYMBOL_COIN;
      a.options.core_exchange_rate.quote.amount = 1;
      a.options.core_exchange_rate.quote.symbol = SYMBOL_COIN;
   });

   create<asset_dynamic_data_object>([](asset_dynamic_data_object& a) 
   {
      a.symbol = SYMBOL_COIN;
      a.total_supply = INIT_COIN_SUPPLY;
   });

   // Create Equity asset

   create<asset_object>( []( asset_object& a ) 
   {
      a.symbol = SYMBOL_EQUITY;
      a.options.max_supply = INIT_EQUITY_SUPPLY;
      a.asset_type = EQUITY_ASSET;
      a.options.flags = 0;
      a.options.issuer_permissions = 0;
      a.issuer = INIT_ACCOUNT;
      a.options.core_exchange_rate.base.amount = 10;
      a.options.core_exchange_rate.base.symbol = SYMBOL_COIN;
      a.options.core_exchange_rate.quote.amount = 1;
      a.options.core_exchange_rate.quote.symbol = SYMBOL_EQUITY;
      a.options.unstake_intervals = 0;
      a.options.stake_intervals = 4;

   });

   create<asset_dynamic_data_object>([](asset_dynamic_data_object& a) 
   {
      a.symbol = SYMBOL_EQUITY;
      a.staked_supply = INIT_EQUITY_SUPPLY;
      a.total_supply = INIT_EQUITY_SUPPLY;
   });

   // Create USD asset

   create<asset_object>( []( asset_object& a ) 
   {
      a.symbol = SYMBOL_USD;
      a.options.max_supply = MAX_ASSET_SUPPLY;
      a.asset_type = BITASSET_ASSET;
      a.options.flags = 0;
      a.options.issuer_permissions = 0;
      a.issuer = GENESIS_ACCOUNT_BASE_NAME;
      a.options.core_exchange_rate.base.amount = 1;
      a.options.core_exchange_rate.base.symbol = SYMBOL_COIN;
      a.options.core_exchange_rate.quote.amount = 1;
      a.options.core_exchange_rate.quote.symbol = SYMBOL_USD;
   });

   create<asset_bitasset_data_object>( [&]( asset_bitasset_data_object& a ) 
   {
      a.symbol = SYMBOL_USD;
      a.backing_asset = SYMBOL_COIN;
   });

   // Create Credit asset
   
   create<asset_object>( []( asset_object& a ) 
   {
      a.symbol = SYMBOL_CREDIT;
      a.asset_type = CREDIT_ASSET;
      a.options.flags = 0;
      a.options.issuer_permissions = 0;
      a.issuer = INIT_ACCOUNT;
      a.options.core_exchange_rate.base.amount = 1;
      a.options.core_exchange_rate.base.symbol = SYMBOL_COIN;
      a.options.core_exchange_rate.quote.amount = 1;
      a.options.core_exchange_rate.quote.symbol = SYMBOL_CREDIT;
   });

   create<asset_dynamic_data_object>([](asset_dynamic_data_object& a) 
   {
      a.symbol = SYMBOL_CREDIT;
   });

   for( int i = 0; i < (GENESIS_WITNESS_AMOUNT + GENESIS_EXTRA_WITNESSES); ++i )
   {
      // Create account for genesis witness
      create< account_object >( [&]( account_object& a )
      {
         a.name = GENESIS_ACCOUNT_BASE_NAME + ( i ? fc::to_string( i ) : std::string() );
         a.secure_public_key = init_public_key;
      } );

      // Create Core asset balance object for witness
      create< account_balance_object >( [&]( account_balance_object& abo )
      {
         abo.owner = GENESIS_ACCOUNT_BASE_NAME + ( i ? fc::to_string( i ) : std::string() );
         abo.symbol = SYMBOL_COIN;
         abo.liquid_balance  = GENESIS_ACCOUNT_COIN;
         abo.staked_balance = GENESIS_ACCOUNT_COIN_STAKE;
      } );

      create< account_authority_object >( [&]( account_authority_object& auth )
      {
         auth.account = GENESIS_ACCOUNT_BASE_NAME + ( i ? fc::to_string( i ) : std::string() );
         auth.owner.add_authority( init_public_key, 1 );
         auth.owner.weight_threshold = 1;
         auth.active  = auth.owner;
         auth.posting = auth.active;
      });
      if(i < GENESIS_WITNESS_AMOUNT)
      {
         create< witness_object >( [&]( witness_object& w )
         {
            w.owner        = GENESIS_ACCOUNT_BASE_NAME + ( i ? fc::to_string(i) : std::string() );
            w.signing_key  = init_public_key;
            w.schedule = witness_object::top_miner;
         } );
      }
   }

   // Create Equity asset balance object for initial account
      create< account_balance_object >( [&]( account_balance_object& abo )
      {
         abo.owner = INIT_ACCOUNT;
         abo.symbol = SYMBOL_EQUITY;
         abo.liquid_balance = 0;
         abo.staked_balance = INIT_EQUITY_SUPPLY;
      } );

   // Create the initial Reward fund object to contain the balances of the network reward funds and parameters

   create< reward_fund_object >( [&]( reward_fund_object& rfo )
   {
      rfo.last_update = now;
      rfo.content_constant = CONTENT_CONSTANT;
      rfo.content_reward_balance = asset(0, SYMBOL_COIN);
      rfo.validation_reward_balance = asset(0, SYMBOL_COIN);
      rfo.txn_stake_reward_balance = asset(0, SYMBOL_COIN);
      rfo.work_reward_balance = asset(0, SYMBOL_COIN);
      rfo.activity_reward_balance = asset(0, SYMBOL_COIN);
      rfo.supernode_reward_balance = asset(0, SYMBOL_COIN);
      rfo.power_reward_balance = asset(0, SYMBOL_COIN);
      rfo.community_fund_balance = asset(0, SYMBOL_COIN);
      rfo.development_reward_balance = asset(0, SYMBOL_COIN);
      rfo.marketing_reward_balance = asset(0, SYMBOL_COIN);
      rfo.advocacy_reward_balance = asset(0, SYMBOL_COIN);
      rfo.activity_reward_balance = asset(0, SYMBOL_COIN);
      rfo.recent_content_claims = 0;
      rfo.author_reward_curve = curve_id::convergent_semi_quadratic;
      rfo.curation_reward_curve = curve_id::convergent_semi_quadratic;
   });

   // Create the Global Dynamic Properties Object to track consensus critical network and chain information
   
   create< dynamic_global_property_object >( [&]( dynamic_global_property_object& p )
   {
      p.current_producer = GENESIS_ACCOUNT_BASE_NAME;
      p.time = GENESIS_TIME;
      p.recent_slots_filled = fc::uint128::max_value();
      p.participation_count = 128;
      p.maximum_block_size = MAX_BLOCK_SIZE;
   });

   create< comment_metrics_object >( [&]( comment_metrics_object& o ) {});

   // Nothing to do
   create< feed_history_object >( [&]( feed_history_object& o ) {});
   for( int i = 0; i < 0x10000; i++ )
      create< block_summary_object >( [&]( block_summary_object& ) {});
   create< hardfork_property_object >( [&]( hardfork_property_object& hpo )
   {
      hpo.processed_hardforks.push_back( GENESIS_TIME );

   });

   // Create witness scheduler
   create< witness_schedule_object >( [&]( witness_schedule_object& wso )
   {
      wso.current_shuffled_producers[0] = GENESIS_ACCOUNT_BASE_NAME;
   });
} FC_CAPTURE_AND_RETHROW() }

void database::reindex( const fc::path& data_dir, const fc::path& shared_mem_dir, uint64_t shared_file_size )
{ try {
   ilog( "Reindexing Blockchain" );
   wipe( data_dir, shared_mem_dir, false );
   open( data_dir, shared_mem_dir, shared_file_size, chainbase::database::read_write );
   _fork_db.reset();    // override effect of _fork_db.start_block() call in open()

   auto start = fc::time_point::now();
   ASSERT( _block_log.head(), block_log_exception, "No blocks in block log. Cannot reindex an empty chain." );

   ilog( "Replaying blocks..." );


   uint64_t skip_flags =
      skip_witness_signature |
      skip_transaction_signatures |
      skip_transaction_dupe_check |
      skip_tapos_check |
      skip_merkle_check |
      skip_witness_schedule_check |
      skip_authority_check |
      skip_validate | /// no need to validate operations
      skip_validate_invariants |
      skip_block_log;

   with_write_lock( [&]()
   {
      auto itr = _block_log.read_block( 0 );
      auto last_block_num = _block_log.head()->block_num();

      while( itr.first.block_num() != last_block_num )
      {
         auto cur_block_num = itr.first.block_num();
         if( cur_block_num % 100000 == 0 )
            std::cerr << "   " << double( cur_block_num * 100 ) / last_block_num << "%   " << cur_block_num << " of " << last_block_num <<
            "   (" << (get_free_memory() / (1024*1024)) << "M free)\n";
         apply_block( itr.first, skip_flags );
         itr = _block_log.read_block( itr.second );
      }

      apply_block( itr.first, skip_flags );
      set_revision( head_block_num() );
   });

   if( _block_log.head()->block_num() )
      _fork_db.start_block( *_block_log.head() );

   auto end = fc::time_point::now();
   ilog( "Done reindexing, elapsed time: ${t} sec", ("t",double((end-start).count())/1000000.0 ) );
} FC_CAPTURE_AND_RETHROW( (data_dir)(shared_mem_dir) ) }

void database::wipe( const fc::path& data_dir, const fc::path& shared_mem_dir, bool include_blocks)
{
   close();
   chainbase::database::wipe( shared_mem_dir );
   if( include_blocks )
   {
      fc::remove_all( data_dir / "block_log" );
      fc::remove_all( data_dir / "block_log.index" );
   }
}

void database::close(bool rewind)
{
   try
   {
      // Since pop_block() will move tx's in the popped blocks into pending,
      // we have to clear_pending() after we're done popping to get a clean
      // DB state (issue #336).
      clear_pending();

      chainbase::database::flush();
      chainbase::database::close();

      _block_log.close();

      _fork_db.reset();
   }
   FC_CAPTURE_AND_RETHROW()
}

bool database::is_known_block( const block_id_type& id )const
{ try {
   return fetch_block_by_id( id ).valid();
} FC_CAPTURE_AND_RETHROW() }

/**
 * Only return true *if* the transaction has not expired or been invalidated. If this
 * method is called with a VERY old transaction we will return false, they should
 * query things by blocks if they are that old.
 */
bool database::is_known_transaction( const transaction_id_type& id )const
{ try {
   const auto& trx_idx = get_index<transaction_index>().indices().get<by_trx_id>();
   return trx_idx.find( id ) != trx_idx.end();
} FC_CAPTURE_AND_RETHROW() }

block_id_type database::find_block_id_for_num( uint32_t block_num )const
{ try {
   if( block_num == 0 )
      return block_id_type();

   // Reversible blocks are *usually* in the TAPOS buffer.  Since this
   // is the fastest check, we do it first.
   block_summary_id_type bsid = block_num & 0xFFFF;
   const block_summary_object* bs = find< block_summary_object, by_id >( bsid );
   if( bs != nullptr )
   {
      if( protocol::block_header::num_from_id(bs->block_id) == block_num )
         return bs->block_id;
   }

   // Next we query the block log.   Irreversible blocks are here.
   auto b = _block_log.read_block_by_num( block_num );
   if( b.valid() )
      return b->id();

   // Finally we query the fork DB.
   shared_ptr< fork_item > fitem = _fork_db.fetch_block_on_main_branch_by_number( block_num );
   if( fitem )
      return fitem->id;

   return block_id_type();
} FC_CAPTURE_AND_RETHROW( (block_num) ) }

block_id_type database::get_block_id_for_num( uint32_t block_num )const
{
   block_id_type bid = find_block_id_for_num( block_num );
   FC_ASSERT( bid != block_id_type() );
   return bid;
}

optional<signed_block> database::fetch_block_by_id( const block_id_type& id )const
{ try {
   auto b = _fork_db.fetch_block( id );
   if( !b )
   {
      auto tmp = _block_log.read_block_by_num( protocol::block_header::num_from_id( id ) );

      if( tmp && tmp->id() == id )
         return tmp;

      tmp.reset();
      return tmp;
   }

   return b->data;
} FC_CAPTURE_AND_RETHROW() }

optional<signed_block> database::fetch_block_by_number( uint32_t block_num )const
{ try {
   optional< signed_block > b;

   auto results = _fork_db.fetch_block_by_number( block_num );
   if( results.size() == 1 )
      b = results[0]->data;
   else
      b = _block_log.read_block_by_num( block_num );

   return b;
} FC_LOG_AND_RETHROW() }

const signed_transaction database::get_recent_transaction( const transaction_id_type& trx_id ) const
{ try {
   auto& index = get_index<transaction_index>().indices().get<by_trx_id>();
   auto itr = index.find(trx_id);
   FC_ASSERT(itr != index.end());
   signed_transaction trx;
   fc::raw::unpack( itr->packed_trx, trx );
   return trx;;
} FC_CAPTURE_AND_RETHROW() }


annotated_signed_transaction database::get_transaction( const transaction_id_type& id )const
{ try {
   const auto& idx = get_index< operation_index >().indices().get<by_transaction_id>();
   auto itr = idx.lower_bound( id );
   if( itr != idx.end() && itr->trx_id == id ) 
   {
      auto blk = fetch_block_by_number( itr->block );
      FC_ASSERT( blk.valid() );
      FC_ASSERT( blk->transactions.size() > itr->trx_in_block );
      annotated_signed_transaction result = blk->transactions[itr->trx_in_block];
      result.block_num       = itr->block;
      result.transaction_num = itr->trx_in_block;
      return result;
   }
   FC_ASSERT( false, "Unknown Transaction ${t}", ("t",id));

} FC_CAPTURE_AND_RETHROW( (id) ) }

std::vector< block_id_type > database::get_block_ids_on_fork( block_id_type head_of_fork ) const
{ try {
   pair<fork_database::branch_type, fork_database::branch_type> branches = _fork_db.fetch_branch_from(head_block_id(), head_of_fork);
   if( !((branches.first.back()->previous_id() == branches.second.back()->previous_id())) )
   {
      edump( (head_of_fork)
             (head_block_id())
             (branches.first.size())
             (branches.second.size()) );
      assert(branches.first.back()->previous_id() == branches.second.back()->previous_id());
   }
   std::vector< block_id_type > result;
   for( const item_ptr& fork_block : branches.second )
      result.emplace_back(fork_block->id);
   result.emplace_back(branches.first.back()->previous_id());
   return result;
} FC_CAPTURE_AND_RETHROW() }

chain_id_type database::get_chain_id() const
{
   return CHAIN_ID;
}

const dynamic_global_property_object& database::get_dynamic_global_properties()const
{ try {
   return get< dynamic_global_property_object >();
} FC_CAPTURE_AND_RETHROW() }

time_point database::head_block_time()const
{
   return get_dynamic_global_properties().time;
}

time_point database::next_maintenance_time()const
{
   return get_dynamic_global_properties().next_maintenance_time;
}

uint32_t database::head_block_num()const
{
   return get_dynamic_global_properties().head_block_number;
}

block_id_type database::head_block_id()const
{
   return get_dynamic_global_properties().head_block_id;
}

const witness_schedule_object& database::get_witness_schedule()const
{ try {
   return get< witness_schedule_object >();
} FC_CAPTURE_AND_RETHROW() }

// Gets the median chain properties object from the Witness Schedule object.
const chain_properties& database::chain_properties()const
{
   return get_witness_schedule().median_props;
}

uint128_t database::pow_difficulty()const
{
   return get_witness_schedule().pow_target_difficulty;
}

node_property_object& database::node_properties()
{
   return _node_property_object;
}

const node_property_object& database::get_node_properties() const
{
   return _node_property_object;
}

uint32_t database::last_non_undoable_block_num() const
{
   return get_dynamic_global_properties().last_irreversible_block_num;
}

const asset_object& database::get_core_asset() const
{
   return get< asset_object, by_id >( 0 );
}

const asset_object* database::find_core_asset() const
{
   return find< asset_object, by_id >( 0 );
}

const price& database::get_usd_price() const
{
   get_bitasset_data( SYMBOL_USD ).current_feed.settlement_price;
}

const asset_object& database::get_asset( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_object, by_symbol >( symbol );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_object* database::find_asset( const asset_symbol_type& symbol ) const
{
   return find< asset_object, by_symbol >( symbol );
}

const asset_dynamic_data_object& database::get_core_dynamic_data() const
{
   return get< asset_dynamic_data_object, by_id >( 0 );
}

const asset_dynamic_data_object* database::find_core_dynamic_data() const
{
   return find< asset_dynamic_data_object, by_id >( 0 );
}

const asset_dynamic_data_object& database::get_dynamic_data( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_dynamic_data_object, by_symbol >( (symbol) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_dynamic_data_object* database::find_dynamic_data( const asset_symbol_type& symbol ) const
{
   return find< asset_dynamic_data_object, by_symbol >( (symbol) );
}

const asset_bitasset_data_object& database::get_bitasset_data( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_bitasset_data_object, by_symbol >( (symbol) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_bitasset_data_object* database::find_bitasset_data( const asset_symbol_type& symbol ) const
{
   return find< asset_bitasset_data_object, by_symbol >( (symbol) );
}

const asset_equity_data_object& database::get_equity_data( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_equity_data_object, by_symbol >( (symbol) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_equity_data_object* database::find_equity_data( const asset_symbol_type& symbol ) const
{
   return find< asset_equity_data_object, by_symbol >( (symbol) );
}

const asset_credit_data_object& database::get_credit_data( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_credit_data_object, by_symbol >( (symbol) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_credit_data_object* database::find_credit_data( const asset_symbol_type& symbol ) const
{
   return find< asset_credit_data_object, by_symbol >( (symbol) );
}

const witness_object& database::get_witness( const account_name_type& name ) const
{ try {
   return get< witness_object, by_name >( name );
} FC_CAPTURE_AND_RETHROW( (name) ) }

const witness_object* database::find_witness( const account_name_type& name ) const
{
   return find< witness_object, by_name >( name );
}

const block_validation_object& database::get_block_validation( const account_name_type& producer, uint32_t height ) const
{ try {
   return get< block_validation_object, by_producer_height >( boost::make_tuple( producer, height) );
} FC_CAPTURE_AND_RETHROW( (producer)(height) ) }

const block_validation_object* database::find_block_validation( const account_name_type& producer, uint32_t height ) const
{
   return find< block_validation_object, by_producer_height >( boost::make_tuple( producer, height) );
}

const account_object& database::get_account( const account_name_type& name )const
{ try {
	return get< account_object, by_name >( name );
} FC_CAPTURE_AND_RETHROW( (name) ) }

const account_object* database::find_account( const account_name_type& name )const
{
   return find< account_object, by_name >( name );
}

const account_following_object& database::get_account_following( const account_name_type& account )const
{ try {
	return get< account_following_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const account_following_object* database::find_account_following( const account_name_type& account )const
{
   return find< account_following_object, by_account >( account );
}

const tag_following_object& database::get_tag_following( const tag_name_type& tag )const
{ try {
	return get< tag_following_object, by_tag >( tag );
} FC_CAPTURE_AND_RETHROW( (tag) ) }

const tag_following_object* database::find_tag_following( const tag_name_type& tag )const
{
   return find< tag_following_object, by_tag >( tag );
}

const account_business_object& database::get_account_business( const account_name_type& account )const
{ try {
	return get< account_business_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const account_business_object* database::find_account_business( const account_name_type& account )const
{
   return find< account_business_object, by_account >( account );
}

const account_executive_vote_object& database::get_account_executive_vote( const account_name_type& account, const account_name_type& business, const account_name_type& executive )const
{ try {
	return get< account_executive_vote_object, by_account_business_executive >( boost::make_tuple( account, business, executive ) );
} FC_CAPTURE_AND_RETHROW( (account)(business)(executive) ) }

const account_executive_vote_object* database::find_account_executive_vote( const account_name_type& account, const account_name_type& business, const account_name_type& executive )const
{
   return find< account_executive_vote_object, by_account_business_executive >( boost::make_tuple( account, business, executive ) );
}

const account_officer_vote_object& database::get_account_officer_vote( const account_name_type& account, const account_name_type& business, const account_name_type& officer )const
{ try {
	return get< account_officer_vote_object, by_account_business_officer >( boost::make_tuple( account, business, officer ) );
} FC_CAPTURE_AND_RETHROW( (account)(business)(officer) ) }

const account_officer_vote_object* database::find_account_officer_vote( const account_name_type& account, const account_name_type& business, const account_name_type& officer )const
{
   return find< account_officer_vote_object, by_account_business_officer >( boost::make_tuple( account, business, officer ) );
}

const account_member_request_object& database::get_account_member_request( const account_name_type& account, const account_name_type& business )const
{ try {
	return get< account_member_request_object, by_account_business >( boost::make_tuple( account, business) );
} FC_CAPTURE_AND_RETHROW( (account)(business) ) }

const account_member_request_object* database::find_account_member_request( const account_name_type& account, const account_name_type& business )const
{
   return find< account_member_request_object, by_account_business >( boost::make_tuple( account, business) );
}

const account_member_invite_object& database::get_account_member_invite( const account_name_type& member, const account_name_type& business )const
{ try {
	return get< account_member_invite_object, by_member_business >( boost::make_tuple( member, business) );
} FC_CAPTURE_AND_RETHROW( (member)(business) ) }

const account_member_invite_object* database::find_account_member_invite( const account_name_type& member, const account_name_type& business )const
{
   return find< account_member_invite_object, by_member_business >( boost::make_tuple( member, business) );
}

const account_member_key_object& database::get_account_member_key( const account_name_type& account )const
{ try {
	return get< account_member_key_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const account_member_key_object* database::find_account_member_key( const account_name_type& account )const
{
   return find< account_member_key_object, by_account >( account );
}

const account_balance_object& database::get_account_balance( const account_name_type& owner, const asset_symbol_type& symbol )const
{ try {
	return get< account_balance_object, by_owner_symbol >( boost::make_tuple( owner, symbol ) );
} FC_CAPTURE_AND_RETHROW( (owner) (symbol) ) }

const account_balance_object* database::find_account_balance( const account_name_type& owner, const asset_symbol_type& symbol )const
{
   return find< account_balance_object, by_owner_symbol >( boost::make_tuple( owner, symbol ) );
}

const account_permission_object& database::get_account_permissions( const account_name_type& account )const
{ try {
	return get< account_permission_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const account_permission_object* database::find_account_permissions( const account_name_type& account )const
{
   return find< account_permission_object, by_account >( account );
}

const account_authority_object& database::get_account_authority( const account_name_type& account )const
{ try {
	return get< account_authority_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const account_authority_object* database::find_account_authority( const account_name_type& account )const
{
   return find< account_authority_object, by_account >( account );
}

const network_officer_object& database::get_network_officer( const account_name_type& account )const
{ try {
	return get< network_officer_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const network_officer_object* database::find_network_officer( const account_name_type& account )const
{
   return find< network_officer_object, by_account >( account );
}

const executive_board_object& database::get_executive_board( const account_name_type& account )const
{ try {
	return get< executive_board_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const executive_board_object* database::find_executive_board( const account_name_type& account )const
{
   return find< executive_board_object, by_account >( account );
}

const supernode_object& database::get_supernode( const account_name_type& account )const
{ try {
	return get< supernode_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const supernode_object* database::find_supernode( const account_name_type& account )const
{
   return find< supernode_object, by_account >( account );
}

const interface_object& database::get_interface( const account_name_type& account )const
{ try {
	return get< interface_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const interface_object* database::find_interface( const account_name_type& account )const
{
   return find< interface_object, by_account >( account );
}

const governance_account_object& database::get_governance_account( const account_name_type& account )const
{ try {
	return get< governance_account_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const governance_account_object* database::find_governance_account( const account_name_type& account )const
{
   return find< governance_account_object, by_account >( account );
}

const community_enterprise_object& database::get_community_enterprise( const account_name_type& creator, const shared_string& enterprise_id )const
{ try {
   return get< community_enterprise_object, by_enterprise_id >( boost::make_tuple( creator, enterprise_id ) );
} FC_CAPTURE_AND_RETHROW( (creator)(enterprise_id) ) }

const community_enterprise_object* database::find_community_enterprise( const account_name_type& creator, const shared_string& enterprise_id )const
{
   return find< community_enterprise_object, by_enterprise_id >( boost::make_tuple( creator, enterprise_id ) );
}

const enterprise_approval_object& database::get_enterprise_approval( const account_name_type& creator, const shared_string& enterprise_id, const account_name_type& account )const
{ try {
   return get< enterprise_approval_object, by_enterprise_id >( boost::make_tuple( creator, enterprise_id, account ) );
} FC_CAPTURE_AND_RETHROW( (creator)(enterprise_id)(account) ) }

const enterprise_approval_object* database::find_enterprise_approval( const account_name_type& creator, const shared_string& enterprise_id, const account_name_type& account )const
{
   return find< enterprise_approval_object, by_enterprise_id >( boost::make_tuple( creator, enterprise_id, account ) );
}

const board_object& database::get_board( const board_name_type& board )const
{ try {
	return get< board_object, by_name >( board );
} FC_CAPTURE_AND_RETHROW( (board) ) }

const board_object* database::find_board( const board_name_type& board )const
{
   return find< board_object, by_name >( board );
}

const board_member_object& database::get_board_member( const board_name_type& board )const
{ try {
	return get< board_member_object, by_name >( board );
} FC_CAPTURE_AND_RETHROW( (board) ) }

const board_member_object* database::find_board_member( const board_name_type& board )const
{
   return find< board_member_object, by_name >( board );
}

const board_member_key_object& database::get_board_member_key( const account_name_type& member, const board_name_type& board )const
{ try {
	return get< board_member_key_object, by_member_board >( boost::make_tuple( member, board) );
} FC_CAPTURE_AND_RETHROW( (board) ) }

const board_member_key_object* database::find_board_member_key( const account_name_type& member, const board_name_type& board )const
{
   return find< board_member_key_object, by_member_board >( boost::make_tuple( member, board) );
}

const comment_object& database::get_comment( const account_name_type& author, const shared_string& permlink )const
{ try {
   return get< comment_object, by_permlink >( boost::make_tuple( author, permlink ) );
} FC_CAPTURE_AND_RETHROW( (author)(permlink) ) }

const comment_object* database::find_comment( const account_name_type& author, const shared_string& permlink )const
{
   return find< comment_object, by_permlink >( boost::make_tuple( author, permlink ) );
}

const comment_object& database::get_comment( const account_name_type& author, const string& permlink )const
{ try {
   return get< comment_object, by_permlink >( boost::make_tuple( author, permlink) );
} FC_CAPTURE_AND_RETHROW( (author)(permlink) ) }

const comment_object* database::find_comment( const account_name_type& author, const string& permlink )const
{
   return find< comment_object, by_permlink >( boost::make_tuple( author, permlink ) );
}

const comment_vote_object& database::get_comment_vote( const account_name_type& voter, const comment_id_type& vote_id )const
{ try {
   return get< comment_vote_object, by_voter_comment >( boost::make_tuple( voter, vote_id ) );
} FC_CAPTURE_AND_RETHROW( (voter)(vote_id) ) }

const comment_vote_object* database::find_comment_vote( const account_name_type& voter, const comment_id_type& vote_id )const
{
   return find< comment_vote_object, by_voter_comment >( boost::make_tuple( voter, vote_id ) );
}

const comment_view_object& database::get_comment_view( const account_name_type& viewer, const comment_id_type& view_id )const
{ try {
   return get< comment_view_object, by_viewer_comment >( boost::make_tuple( viewer, view_id ) );
} FC_CAPTURE_AND_RETHROW( (viewer)(view_id) ) }

const comment_view_object* database::find_comment_view( const account_name_type& viewer, const comment_id_type& view_id )const
{
   return find< comment_view_object, by_viewer_comment >( boost::make_tuple( viewer, view_id ) );
}

const comment_share_object& database::get_comment_share( const account_name_type& sharer, const comment_id_type& share_id )const
{ try {
   return get< comment_share_object, by_sharer_comment >( boost::make_tuple( sharer, share_id ) );
} FC_CAPTURE_AND_RETHROW( (sharer)(share_id) ) }

const comment_share_object* database::find_comment_share( const account_name_type& sharer, const comment_id_type& share_id )const
{
   return find< comment_share_object, by_sharer_comment >( boost::make_tuple( sharer, share_id ) );
}

const ad_creative_object& database::get_ad_creative( const account_name_type& account, const string& creative_id )const
{ try {
   return get< ad_creative_object, by_creative_id >( boost::make_tuple( account, creative_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(creative_id) ) }

const ad_creative_object* database::find_ad_creative( const account_name_type& account, const string& creative_id )const
{
   return find< ad_creative_object, by_creative_id >( boost::make_tuple( account, creative_id ) );
}

const ad_campaign_object& database::get_ad_campaign( const account_name_type& account, const string& campaign_id )const
{ try {
   return get< ad_campaign_object, by_campaign_id >( boost::make_tuple( account, campaign_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(campaign_id) ) }

const ad_campaign_object* database::find_ad_campaign( const account_name_type& account, const string& campaign_id )const
{
   return find< ad_campaign_object, by_campaign_id >( boost::make_tuple( account, campaign_id ) );
}

const ad_inventory_object& database::get_ad_inventory( const account_name_type& account, const string& inventory_id )const
{ try {
   return get< ad_inventory_object, by_inventory_id >( boost::make_tuple( account, inventory_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(inventory_id) ) }

const ad_inventory_object* database::find_ad_inventory( const account_name_type& account, const string& inventory_id )const
{
   return find< ad_inventory_object, by_inventory_id >( boost::make_tuple( account, inventory_id ) );
}

const ad_audience_object& database::get_ad_audience( const account_name_type& account, const string& audience_id )const
{ try {
   return get< ad_audience_object, by_audience_id >( boost::make_tuple( account, audience_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(audience_id) ) }

const ad_audience_object* database::find_ad_audience( const account_name_type& account, const string& audience_id )const
{
   return find< ad_audience_object, by_audience_id >( boost::make_tuple( account, audience_id ) );
}

const ad_bid_object& database::get_ad_bid( const account_name_type& account, const string& bid_id )const
{ try {
   return get< ad_bid_object, by_bid_id >( boost::make_tuple( account, bid_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(bid_id) ) }

const ad_bid_object* database::find_ad_bid( const account_name_type& account, const string& bid_id )const
{
   return find< ad_bid_object, by_bid_id >( boost::make_tuple( account, bid_id ) );
}

const asset_liquidity_pool_object& database::get_liquidity_pool( const asset_symbol_type& symbol_a, const asset_symbol_type& symbol_b )const
{ try {
   return get< asset_liquidity_pool_object, by_asset_pair >( boost::make_tuple( symbol_a, symbol_b ) );
} FC_CAPTURE_AND_RETHROW( (symbol_a)(symbol_b) ) }

const asset_liquidity_pool_object* database::find_liquidity_pool( const asset_symbol_type& symbol_a, const asset_symbol_type& symbol_b )const
{
   return find< asset_liquidity_pool_object, by_asset_pair >( boost::make_tuple( symbol_a, symbol_b ) );
}

const asset_liquidity_pool_object& database::get_liquidity_pool( const asset_symbol_type& symbol )const
{ try {
   return get< asset_liquidity_pool_object, by_asset_pair >( boost::make_tuple( SYMBOL_COIN, symbol ) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_liquidity_pool_object* database::find_liquidity_pool( const asset_symbol_type& symbol )const
{
   return find< asset_liquidity_pool_object, by_asset_pair >( boost::make_tuple( SYMBOL_COIN, symbol ) );
}

const asset_credit_pool_object& database::get_credit_pool( const asset_symbol_type& symbol, bool credit_asset)const
{ try {
   if( credit_asset)
   {
      return get< asset_credit_pool_object, by_credit_symbol >( symbol );
   }
   else
   {
      return get< asset_credit_pool_object, by_base_symbol >( symbol );
   }
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_credit_pool_object* database::find_credit_pool( const asset_symbol_type& symbol, bool credit_asset)const
{
   if( credit_asset )
   {
      return find< asset_credit_pool_object, by_credit_symbol >( symbol );
   }
   else
   {
      return find< asset_credit_pool_object, by_base_symbol >( symbol );
   }
}

const credit_collateral_object& database::get_collateral( const account_name_type& owner, const asset_symbol_type& symbol  )const
{ try {
   return get< credit_collateral_object, by_owner_symbol >( boost::make_tuple( owner, symbol ) );
} FC_CAPTURE_AND_RETHROW( (owner)(symbol) ) }

const credit_collateral_object* database::find_collateral( const account_name_type& owner, const asset_symbol_type& symbol )const
{
   return find< credit_collateral_object, by_owner_symbol >( boost::make_tuple( owner, symbol ) );
}

const credit_loan_object& database::get_loan( const account_name_type& owner, string& loan_id  )const
{ try {
   return get< credit_loan_object, by_loan_id >( boost::make_tuple( owner, loan_id ) );
} FC_CAPTURE_AND_RETHROW( (owner)(loan_id) ) }

const credit_loan_object* database::find_loan( const account_name_type& owner, string& loan_id )const
{
   return find< credit_loan_object, by_loan_id >( boost::make_tuple( owner, loan_id ) );
}

const escrow_object& database::get_escrow( const account_name_type& name, uint32_t escrow_id )const
{ try {
   return get< escrow_object, by_from_id >( boost::make_tuple( name, escrow_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(escrow_id) ) }

const escrow_object* database::find_escrow( const account_name_type& name, uint32_t escrow_id )const
{
   return find< escrow_object, by_from_id >( boost::make_tuple( name, escrow_id ) );
}

const limit_order_object& database::get_limit_order( const account_name_type& name, uint32_t orderid )const
{ try {
   return get< limit_order_object, by_account >( boost::make_tuple( name, orderid ) );
} FC_CAPTURE_AND_RETHROW( (name)(orderid) ) }

const limit_order_object* database::find_limit_order( const account_name_type& name, uint32_t orderid )const
{
   return find< limit_order_object, by_account >( boost::make_tuple( name, orderid ) );
}

const call_order_object& database::get_call_order( const account_name_type& name, const asset_symbol_type& symbol )const
{ try {
   return get< call_order_object, by_account >( boost::make_tuple( name, symbol ) );
} FC_CAPTURE_AND_RETHROW( (name)(symbol) ) }

const call_order_object* database::find_call_order( const account_name_type& name, const asset_symbol_type& symbol )const
{
   return find< call_order_object, by_account >( boost::make_tuple( name, symbol ) );
}

const margin_order_object& database::get_margin_order( const account_name_type& name, string& margin_id )const
{ try {
   return get< margin_order_object, by_account >( boost::make_tuple( name, margin_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(margin_id) ) }

const margin_order_object* database::find_margin_order( const account_name_type& name, string& margin_id )const
{
   return find< margin_order_object, by_account >( boost::make_tuple( name, margin_id ) );
}

const savings_withdraw_object& database::get_savings_withdraw( const account_name_type& owner, uint32_t request_id )const
{ try {
   return get< savings_withdraw_object, by_from_rid >( boost::make_tuple( owner, request_id ) );
} FC_CAPTURE_AND_RETHROW( (owner)(request_id) ) }

const savings_withdraw_object* database::find_savings_withdraw( const account_name_type& owner, uint32_t request_id )const
{
   return find< savings_withdraw_object, by_from_rid >( boost::make_tuple( owner, request_id ) );
}

const reward_fund_object& database::get_reward_fund() const
{ try {
   return get< reward_fund_object>();
} FC_CAPTURE_AND_RETHROW() }

const comment_metrics_object& database::get_comment_metrics() const
{ try {
   return get< comment_metrics_object>();
} FC_CAPTURE_AND_RETHROW() }

const asset& database::asset_to_USD( const asset& a) const
{
   if( a.symbol != SYMBOL_COIN )
   {
      price rate = get_liquidity_pool( SYMBOL_COIN, a.symbol ).current_price;
      return util::asset_to_USD( get_usd_price(), a * rate );
   }
   else
   {
      return util::asset_to_USD( get_usd_price(), a);
   }
}

const asset& database::USD_to_asset( const asset& a) const
{
   return util::USD_to_asset( get_usd_price(), a);
}

const hardfork_property_object& database::get_hardfork_property_object()const
{ try {
   return get< hardfork_property_object >();
} FC_CAPTURE_AND_RETHROW() }

const time_point database::calculate_discussion_payout_time( const comment_object& comment )const
{
   return comment.cashout_time;
}

uint32_t database::witness_participation_rate()const
{
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();
   return uint64_t(PERCENT_100) * dpo.recent_slots_filled.popcount() / 128;
}

void database::add_checkpoints( const flat_map< uint32_t, block_id_type >& checkpts )
{
   for( const auto& i : checkpts )
      _checkpoints[i.first] = i.second;
}

bool database::before_last_checkpoint()const
{
   return (_checkpoints.size() > 0) && (_checkpoints.rbegin()->first >= head_block_num());
}

/**
 * Push block "may fail" in which case every partial change is unwound.  After
 * push block is successful the block is appended to the chain database on disk.
 *
 * @return true if we switched forks as a result of this push.
 */
bool database::push_block(const signed_block& new_block, uint32_t skip)
{
   //fc::time_point begin_time = fc::time_point::now();

   bool result;
   detail::with_skip_flags( *this, skip, [&]()
   {
      with_write_lock( [&]()
      {
         detail::without_pending_transactions( *this, std::move(_pending_tx), [&]()
         {
            try
            {
               result = _push_block(new_block);
            }
            FC_CAPTURE_AND_RETHROW( (new_block) )
         });
      });
   });

   //fc::time_point end_time = fc::time_point::now();
   //fc::microseconds dt = end_time - begin_time;
   //if( ( new_block.block_num() % 10000 ) == 0 )
   //   ilog( "push_block ${b} took ${t} microseconds", ("b", new_block.block_num())("t", dt.count()) );
   return result;
}

void database::_maybe_warn_multiple_production( uint32_t height )const
{
   auto blocks = _fork_db.fetch_block_by_number( height );
   if( blocks.size() > 1 )
   {
      vector< std::pair< account_name_type, fc::time_point > > witness_time_pairs;
      for( const auto& b : blocks )
      {
         witness_time_pairs.push_back( std::make_pair( b->data.witness, b->data.timestamp ) );
      }

      ilog( "Encountered block num collision at block ${n} due to a fork, witnesses are: ${w}", ("n", height)("w", witness_time_pairs) );
   }
   return;
}

bool database::_push_block(const signed_block& new_block)
{ try {
   uint32_t skip = get_node_properties().skip_flags;
   //uint32_t skip_undo_db = skip & skip_undo_block;

   if( !(skip&skip_fork_db) )
   {
      shared_ptr<fork_item> new_head = _fork_db.push_block(new_block);
      _maybe_warn_multiple_production( new_head->num );

      //If the head block from the longest chain does not build off of the current head, we need to switch forks.
      if( new_head->data.previous != head_block_id() )
      {
         //If the newly pushed block is the same height as head, we get head back in new_head
         //Only switch forks if new_head is actually higher than head
         if( new_head->data.block_num() > head_block_num() )
         {
            // wlog( "Switching to fork: ${id}", ("id",new_head->data.id()) );
            auto branches = _fork_db.fetch_branch_from(new_head->data.id(), head_block_id());

            // pop blocks until we hit the forked block
            while( head_block_id() != branches.second.back()->data.previous )
               pop_block();

            // push all blocks on the new fork
            for( auto ritr = branches.first.rbegin(); ritr != branches.first.rend(); ++ritr )
            {
                // ilog( "pushing blocks from fork ${n} ${id}", ("n",(*ritr)->data.block_num())("id",(*ritr)->data.id()) );
                optional<fc::exception> except;
                try
                {
                   auto session = start_undo_session( true );
                   apply_block( (*ritr)->data, skip );
                   session.push();
                }
                catch ( const fc::exception& e ) { except = e; }
                if( except )
                {
                   // wlog( "exception thrown while switching forks ${e}", ("e",except->to_detail_string() ) );
                   // remove the rest of branches.first from the fork_db, those blocks are invalid
                   while( ritr != branches.first.rend() )
                   {
                      _fork_db.remove( (*ritr)->data.id() );
                      ++ritr;
                   }
                   _fork_db.set_head( branches.second.front() );

                   // pop all blocks from the bad fork
                   while( head_block_id() != branches.second.back()->data.previous )
                      pop_block();

                   // restore all blocks from the good fork
                   for( auto ritr = branches.second.rbegin(); ritr != branches.second.rend(); ++ritr )
                   {
                      auto session = start_undo_session( true );
                      apply_block( (*ritr)->data, skip );
                      session.push();
                   }
                   throw *except;
                }
            }
            return true;
         }
         else
            return false;
      }
   }

   try
   {
      auto session = start_undo_session( true );
      apply_block(new_block, skip);
      session.push();
   }
   catch( const fc::exception& e )
   {
      elog("Failed to push new block:\n${e}", ("e", e.to_detail_string()));
      _fork_db.remove(new_block.id());
      throw;
   }

   return false;
} FC_CAPTURE_AND_RETHROW() }

/**
 * Attempts to push the transaction into the pending queue
 *
 * When called to push a locally generated transaction, set the skip_block_size_check bit on the skip argument. This
 * will allow the transaction to be pushed even if it causes the pending block size to exceed the maximum block size.
 * Although the transaction will probably not propagate further now, as the peers are likely to have their pending
 * queues full as well, it will be kept in the queue to be propagated later when a new block flushes out the pending
 * queues.
 */
void database::push_transaction( const signed_transaction& trx, uint32_t skip )
{ try { try {
   FC_ASSERT( fc::raw::pack_size(trx) <= (get_dynamic_global_properties().maximum_block_size - 256) );
   set_producing( true );
   detail::with_skip_flags( *this, skip,
      [&]()
      {
         with_write_lock( [&]()
         {
            _push_transaction( trx );
         });
      });
   set_producing( false );
   }
   catch( ... )
   {
      set_producing( false );
      throw;
   }
}
FC_CAPTURE_AND_RETHROW( (trx) ) }

void database::_push_transaction( const signed_transaction& trx )
{
   // If this is the first transaction pushed after applying a block, start a new undo session.
   // This allows us to quickly rewind to the clean state of the head block, in case a new block arrives.
   if( !_pending_tx_session.valid() )
      _pending_tx_session = start_undo_session( true );

   // Create a temporary undo session as a child of _pending_tx_session.
   // The temporary session will be discarded by the destructor if
   // _apply_transaction fails.  If we make it to merge(), we
   // apply the changes.

   auto temp_session = start_undo_session( true );
   _apply_transaction( trx );
   _pending_tx.push_back( trx );

   notify_changed_objects();
   // The transaction applied successfully. Merge its changes into the pending block session.
   temp_session.squash();

   // notify anyone listening to pending transactions
   notify_on_pending_transaction( trx );
}


/** Creates a new block using the keys provided to the witness node, 
 * when the witness is scheduled and syncronised.
 */
signed_block database::generate_block(
   time_point when,
   const account_name_type& witness_owner,
   const fc::ecc::private_key& block_signing_private_key,
   uint32_t skip /* = 0 */
   )
{
   signed_block result;
   detail::with_skip_flags( *this, skip, [&]()
   {
      try
      {
         result = _generate_block( when, witness_owner, block_signing_private_key );
      }
      FC_CAPTURE_AND_RETHROW( (witness_owner) )
   });
   return result;
}

signed_block database::_generate_block( fc::time_point when, const account_name_type& witness_owner, const fc::ecc::private_key& block_signing_private_key )
{
   uint32_t skip = get_node_properties().skip_flags;
   uint32_t slot_num = get_slot_at_time( when );
   FC_ASSERT( slot_num > 0 );
   string scheduled_witness = get_scheduled_witness( slot_num );
   FC_ASSERT( scheduled_witness == witness_owner );

   const auto& witness_obj = get_witness( witness_owner );

   if( !(skip & skip_witness_signature) )
      FC_ASSERT( witness_obj.signing_key == block_signing_private_key.get_public_key() );

   signed_block pending_block;

   pending_block.previous = head_block_id();
   pending_block.timestamp = when;
   pending_block.witness = witness_owner;
   
   const auto& witness = get_witness( witness_owner );
   auto blockchainVersion = BLOCKCHAIN_VERSION;
   if( witness.running_version != BLOCKCHAIN_VERSION )
      pending_block.extensions.insert( block_header_extensions( BLOCKCHAIN_VERSION ) );

   const auto& hfp = get_hardfork_property_object();
   auto blockchainHardforkVersion = BLOCKCHAIN_HARDFORK_VERSION;
   if( hfp.current_hardfork_version < BLOCKCHAIN_HARDFORK_VERSION // Binary is newer hardfork than has been applied
      && ( witness.hardfork_version_vote != _hardfork_versions[ hfp.last_hardfork + 1 ] || witness.hardfork_time_vote != _hardfork_times[ hfp.last_hardfork + 1 ] ) ) // Witness vote does not match binary configuration
   {
      // Make vote match binary configuration
      pending_block.extensions.insert( block_header_extensions( hardfork_version_vote( _hardfork_versions[ hfp.last_hardfork + 1 ], _hardfork_times[ hfp.last_hardfork + 1 ] ) ) );
   }
   else if( hfp.current_hardfork_version == BLOCKCHAIN_HARDFORK_VERSION // Binary does not know of a new hardfork
      && witness.hardfork_version_vote > BLOCKCHAIN_HARDFORK_VERSION ) // Voting for hardfork in the future, that we do not know of...
   {
      // Make vote match binary configuration. This is vote to not apply the new hardfork.
      pending_block.extensions.insert( block_header_extensions( hardfork_version_vote( _hardfork_versions[ hfp.last_hardfork ], _hardfork_times[ hfp.last_hardfork ] ) ) );
   }
   
   // The 4 is for the max size of the transaction vector length
   size_t total_block_size = fc::raw::pack_size( pending_block ) + 4;
   auto maximum_block_size = get_dynamic_global_properties().maximum_block_size; //MAX_BLOCK_SIZE;

   with_write_lock( [&]()
   {
      //
      // The following code throws away existing pending_tx_session and
      // rebuilds it by re-applying pending transactions.
      //
      // This rebuild is necessary because pending transactions' validity
      // and semantics may have changed since they were received, because
      // time-based semantics are evaluated based on the current block
      // time.  These changes can only be reflected in the database when
      // the value of the "when" variable is known, which means we need to
      // re-apply pending transactions in this method.
      //
      _pending_tx_session.reset();
      _pending_tx_session = start_undo_session( true );

      uint64_t postponed_tx_count = 0;
      // pop pending state (reset to head block state)
      for( const signed_transaction& tx : _pending_tx )
      {
         // Only include transactions that have not expired yet for currently generating block,
         // this should clear problem transactions and allow block production to continue

         if( tx.expiration < when )
            continue;

         uint64_t new_total_size = total_block_size + fc::raw::pack_size( tx );

         // postpone transaction if it would make block too big
         if( new_total_size >= maximum_block_size )
         {
            postponed_tx_count++;
            continue;
         }

         try
         {
            auto temp_session = start_undo_session( true );
            _apply_transaction( tx );
            temp_session.squash();

            total_block_size += fc::raw::pack_size( tx );
            pending_block.transactions.push_back( tx );
         }
         catch ( const fc::exception& e )
         {
            // Do nothing, transaction will not be re-applied
            //wlog( "Transaction was not processed while generating block due to ${e}", ("e", e) );
            //wlog( "The transaction was ${t}", ("t", tx) );
         }
      }
      if( postponed_tx_count > 0 )
      {
         wlog( "Postponed ${n} transactions due to block size limit", ("n", postponed_tx_count) );
      }

      _pending_tx_session.reset();
   });

   // We have temporarily broken the invariant that
   // _pending_tx_session is the result of applying _pending_tx, as
   // _pending_tx now consists of the set of postponed transactions.
   // However, the push_block() call below will re-create the
   // _pending_tx_session.

   pending_block.transaction_merkle_root = pending_block.calculate_merkle_root();

   if( !(skip & skip_witness_signature) )
      pending_block.sign( block_signing_private_key );

   // TODO:  Move this to _push_block() so session is restored.
   if( !(skip & skip_block_size_check) )
   {
      FC_ASSERT( fc::raw::pack_size(pending_block) <= MAX_BLOCK_SIZE );
   }

   push_block( pending_block, skip );

   return pending_block;
}

/**
 * Removes the most recent block from the database and
 * undoes any changes it made.
 */
void database::pop_block()
{ try {
      _pending_tx_session.reset();
      auto head_id = head_block_id();

      /// save the head block so we can recover its transactions
      optional<signed_block> head_block = fetch_block_by_id( head_id );
      ASSERT( head_block.valid(), pop_empty_chain, "there are no blocks to pop" );

      _fork_db.pop_block();
      undo();

      _popped_tx.insert( _popped_tx.begin(), head_block->transactions.begin(), head_block->transactions.end() );

} FC_CAPTURE_AND_RETHROW() }

void database::clear_pending()
{ try {
   assert( (_pending_tx.size() == 0) || _pending_tx_session.valid() );
   _pending_tx.clear();
   _pending_tx_session.reset();
} FC_CAPTURE_AND_RETHROW() }

void database::notify_pre_apply_operation( operation_notification& note )
{
   note.trx_id       = _current_trx_id;
   note.block        = _current_block_num;
   note.trx_in_block = _current_trx_in_block;
   note.op_in_trx    = _current_op_in_trx;

   TRY_NOTIFY( pre_apply_operation, note )
}

void database::notify_post_apply_operation( const operation_notification& note )
{
   TRY_NOTIFY( post_apply_operation, note )
}

void database::push_virtual_operation( const operation& op, bool force )
{
   if( !force )
   {
      #if defined( IS_LOW_MEM ) && ! defined( IS_TEST_NET )
      return;
      #endif
   }
   FC_ASSERT( is_virtual_operation( op ) );
   operation_notification note(op);
   notify_pre_apply_operation( note );
   notify_post_apply_operation( note );
}

void database::notify_applied_block( const signed_block& block )
{
   TRY_NOTIFY( applied_block, block )
}

void database::notify_pre_apply_block( const signed_block& block )
{
   TRY_NOTIFY( pre_apply_block, block )
}

void database::notify_on_pending_transaction( const signed_transaction& tx )
{
   TRY_NOTIFY( on_pending_transaction, tx )
}

void database::notify_on_pre_apply_transaction( const signed_transaction& tx )
{
   TRY_NOTIFY( on_pre_apply_transaction, tx )
}

void database::notify_on_applied_transaction( const signed_transaction& tx )
{
   TRY_NOTIFY( on_applied_transaction, tx )
}

account_name_type database::get_scheduled_witness( uint32_t slot_num )const
{
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();
   const witness_schedule_object& wso = get_witness_schedule();
   uint64_t current_aslot = dpo.current_aslot + slot_num;
   return wso.current_shuffled_producers[ current_aslot % wso.num_scheduled_producers ];
}

fc::time_point database::get_slot_time(uint32_t slot_num)const
{
   if( slot_num == 0 )
      return fc::time_point();

   int64_t interval_micsec = BLOCK_INTERVAL.count();
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();

   if( head_block_num() == 0 )
   {
      // n.b. first block is at genesis_time plus one block interval
      fc::time_point genesis_time = dpo.time;
      return genesis_time + fc::microseconds(slot_num * interval_micsec);
   }

   // "slot 0" is head_slot_time
   // "slot 1" is head_slot_time,
   //   plus maint interval if head block is a maint block
   //   plus block interval if head block is not a maint block

   int64_t head_block_abs_slot = (head_block_time().time_since_epoch().count() / interval_micsec);
   return fc::time_point( fc::microseconds(head_block_abs_slot * interval_micsec + slot_num * interval_micsec));
}

uint32_t database::get_slot_at_time(fc::time_point when)const
{
   fc::time_point first_slot_time = get_slot_time( 1 );
   if( when < first_slot_time ) {
      return 0;
   }
   uint32_t slot_number = ((when.time_since_epoch().count() - first_slot_time.time_since_epoch().count()) / BLOCK_INTERVAL.count()) + 1;
   return slot_number;
}


void database::update_witness_set()
{ try {
   if( (head_block_num() % SET_UPDATE_BLOCK_INTERVAL) != 0 )    // Runs once per day
      return;

   const witness_schedule_object& wso = get_witness_schedule();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const auto& wit_idx = get_index< witness_index >().indices().get< by_voting_power >();
   auto wit_itr = wit_idx.begin();
   uint128_t total_witness_voting_power = 0;
   
   while( wit_itr != wit_idx.end() )
   {
      total_witness_voting_power += update_witness( *wit_itr, wso, props).value;
      ++wit_itr;
   }

   modify( wso, [&]( witness_schedule_object& w )
   {
      w.total_witness_voting_power = total_witness_voting_power;
   });

} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the voting power and vote count of a witness
 * and returns the total voting power supporting the witness.
 */
share_type database::update_witness( const witness_object& witness, const witness_schedule_object& wso, 
   const dynamic_global_property_object& props )
{ try {
   const auto& wit_vote_idx = get_index< witness_vote_index >().indices().get< by_witness_account >();
   auto wit_vote_itr = wit_vote_idx.lower_bound( witness.owner );
   price equity_price = props.current_median_equity_price;
   time_point now = props.time;
   share_type voting_power = 0;
   uint32_t vote_count = 0;

   while( wit_vote_itr != wit_vote_idx.end() && wit_vote_itr->witness == witness.owner )
   {
      const witness_vote_object& vote = *wit_vote_itr;
      const account_object& voter = get_account( vote.account );
      share_type weight = get_voting_power( wit_vote_itr->account, equity_price );
      if( voter.proxied.size() )
      {
         weight += get_proxied_voting_power( voter, equity_price );
      }
      // divides voting weight by 2^vote_rank, limiting total voting weight -> total voting power as votes increase.
      voting_power += ( weight.value >> vote.vote_rank );    
      vote_count++;
      ++wit_vote_itr;
   }

   modify( witness, [&]( witness_object& w )
   {
      w.voting_power = voting_power;
      w.vote_count = vote_count;
      auto delta_pos = w.voting_power.value * (wso.current_witness_virtual_time - w.witness_virtual_last_update);
      w.witness_virtual_position += delta_pos;
      w.witness_virtual_scheduled_time = w.witness_virtual_last_update + (VIRTUAL_SCHEDULE_LAP_LENGTH - w.witness_virtual_position)/(w.voting_power.value+1);
      /** witnesses with a low number of votes could overflow the time field and end up with a scheduled time in the past */
      if( w.witness_virtual_scheduled_time < wso.current_witness_virtual_time ) 
      {
         w.witness_virtual_scheduled_time = fc::uint128::max_value();
      }
      w.decay_weights( now, wso );
      w.witness_virtual_last_update = wso.current_witness_virtual_time;
   });

   return voting_power;

} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the voting power map of the moderators
 * in a board, which determines the distribution of the
 * moderation rewards for the board.
 */
void database::update_board_moderators( const board_member_object& board )
{ try {
   price equity_price = get_liquidity_pool(SYMBOL_COIN, SYMBOL_EQUITY).hour_median_price;
   const auto& vote_idx = get_index< board_moderator_vote_index >().indices().get< by_board_moderator >();
   flat_map<account_name_type, share_type> mod_weight;

   auto vote_itr = vote_idx.lower_bound( board.name );

   while( vote_itr != vote_idx.end() && vote_itr->board == board.name )
   {
      const board_moderator_vote_object& vote = *vote_itr;
      const account_object& voter = get_account( vote.account );
      share_type weight = get_voting_power( vote_itr->account );
      if( voter.proxied.size() )
      {
         weight += get_proxied_voting_power( voter, equity_price );
      }
      // divides voting weight by 2^vote_rank, limiting total voting weight -> total voting power as votes increase.
      mod_weight[ vote.moderator ] += ( weight.value >> vote.vote_rank );
      ++vote_itr;
   }
   
   modify( board, [&]( board_member_object& b )
   {
      b.mod_weight = mod_weight;
   });

} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the voting power map of the moderators
 * in a board, which determines the distribution of t
 * moderation rewards for the board.
 */
void database::update_board_moderator_set()
{ try {
   if( (head_block_num() % SET_UPDATE_BLOCK_INTERVAL) != 0 )    // Runs once per day
      return;
   
   price equity_price = get_liquidity_pool(SYMBOL_COIN, SYMBOL_EQUITY).hour_median_price;
   const auto& board_idx = get_index< board_member_index >().indices().get< by_name >();
   auto board_itr = board_idx.begin();

   while( board_itr != board_idx.end() )
   {
      update_board_moderators( *board_itr );
      ++board_itr;
   }

} FC_CAPTURE_AND_RETHROW() }

/**
 * Updates the voting statistics, executive board, and officer set of a business
 * account.
 */
void database::update_business_account( const account_business_object& business, const dynamic_global_property_object& props )
{ try {
   const auto& bus_officer_vote_idx = get_index< account_officer_vote_index >().indices().get< by_business_account_rank >();
   const auto& bus_executive_vote_idx = get_index< account_executive_vote_index >().indices().get< by_business_role_executive >();

   flat_map< account_name_type, share_type > officers;
   flat_map< account_name_type, flat_map< executive_types, share_type > > exec_map;
   vector< pair< account_name_type, pair< executive_types, share_type > > > role_rank;
   role_rank.reserve( props.executive_types_amount * officers.size() );
   flat_map< account_name_type, pair< executive_types, share_type > > executives;
   executive_officer_set exec_set;

   auto bus_officer_vote_itr = bus_officer_vote_idx.lower_bound( business.account );
   share_type voting_power = 0;

   while( bus_officer_vote_itr != bus_officer_vote_idx.end() && 
      bus_officer_vote_itr->business_account == business.account )
   {
      const account_object& voter = get_account( bus_officer_vote_itr->account );
      share_type weight = get_equity_voting_power( bus_officer_vote_itr->account, business );

      while( bus_officer_vote_itr != bus_officer_vote_idx.end() && 
         bus_officer_vote_itr->business_account == business.account &&
         bus_officer_vote_itr->account == voter.name )
      {
         const account_officer_vote_object& vote = *bus_officer_vote_itr;
         officers[ vote.officer_account ] += ( weight.value >> vote.vote_rank );
         // divides voting weight by 2^vote_rank, limiting total voting weight -> total voting power as votes increase.
         ++bus_officer_vote_itr;
      }
   }

   // Remove officers from map that do not meet voting requirement
   for( auto itr = officers.begin(); itr != officers.end(); )
   {
      if( itr->second < business.officer_vote_threshold )
      {
         itr = officers.erase( itr );   
      }
      else
      {
         ++itr;
      }
   }

   flat_map< executive_types, share_type > role_map;
   auto bus_executive_vote_itr = bus_executive_vote_idx.lower_bound( business.account );

   while( bus_executive_vote_itr != bus_executive_vote_idx.end() && 
      bus_executive_vote_itr->business_account == business.account )
   {
      const account_object& voter = get_account( bus_executive_vote_itr->account );
      share_type weight = get_equity_voting_power( bus_executive_vote_itr->account, business );

      while( bus_executive_vote_itr != bus_executive_vote_idx.end() && 
         bus_executive_vote_itr->business_account == business.account &&
         bus_executive_vote_itr->account == voter.name )
      {
         const account_executive_vote_object& vote = *bus_executive_vote_itr;

         exec_map[ vote.executive_account ][ vote.role ] += ( weight.value >> vote.vote_rank );
         // divides voting weight by 2^vote_rank, limiting total voting weight -> total voting power as votes increase.
         ++bus_executive_vote_itr;
      }
   }
      
   for( auto exec_votes : exec_map )
   {
      for( auto role_votes : exec_votes.second )   // Copy all exec role votes into sorting vector
      {
         role_rank.push_back( std::make_pair( exec_votes.first, std::make_pair( role_votes.first, role_votes.second ) ) );
      }
   }
   
   std::sort( role_rank.begin(), role_rank.end(), [&](auto a, auto b)
   {
      return a.second.second < b.second.second;   // Ordered vector of all executives, for each role. 
   });

   auto role_rank_itr = role_rank.begin();

   while( !exec_set.allocated && role_rank_itr != role_rank.end() )
   {
      pair< account_name_type, pair< executive_types, share_type > > rank = *role_rank_itr;
      
      account_name_type executive = rank.first;
      executive_types role = rank.second.first;
      share_type votes = rank.second.second;

      switch( role )
      {
         case CHIEF_EXECUTIVE_OFFICER:
         {
            executives[executive] = std::make_pair( role, votes );
            exec_set.CHIEF_EXECUTIVE_OFFICER = executive;
         }
         break;
         case CHIEF_OPERATING_OFFICER:
         {
            executives[executive] = std::make_pair( role, votes );
            exec_set.CHIEF_OPERATING_OFFICER = executive;
         }
         break;
         case CHIEF_FINANCIAL_OFFICER:
         {
            executives[executive] = std::make_pair( role, votes );
            exec_set.CHIEF_FINANCIAL_OFFICER = executive;
         }
         break;
         case CHIEF_TECHNOLOGY_OFFICER:
         {
            executives[executive] = std::make_pair( role, votes );
            exec_set.CHIEF_TECHNOLOGY_OFFICER = executive;
         }
         break;
         case CHIEF_DEVELOPMENT_OFFICER:
         {
            executives[executive] = std::make_pair( role, votes );
            exec_set.CHIEF_DEVELOPMENT_OFFICER = executive;
         }
         break;
         case CHIEF_SECURITY_OFFICER:
         {
            executives[executive] = std::make_pair( role, votes );
            exec_set.CHIEF_SECURITY_OFFICER = executive;
         }
         break;
         case CHIEF_ADVOCACY_OFFICER:
         {
            executives[executive] = std::make_pair( role, votes );
            exec_set.CHIEF_ADVOCACY_OFFICER = executive;
         }
         break;
         case CHIEF_GOVERNANCE_OFFICER:
         {
            executives[executive] = std::make_pair( role, votes );
            exec_set.CHIEF_GOVERNANCE_OFFICER = executive;
         }
         break;
         case CHIEF_MARKETING_OFFICER:
         {
            executives[executive] = std::make_pair( role, votes );
            exec_set.CHIEF_MARKETING_OFFICER = executive;
         }
         break;
         case CHIEF_DESIGN_OFFICER:
         {
            executives[executive] = std::make_pair( role, votes );
            exec_set.CHIEF_DESIGN_OFFICER = executive;
         }
         break;
      }
   }

   modify( business, [&]( account_business_object& b )
   {
      b.officers = officers;
      b.executives = executives;
      b.executive_board = exec_set; 
   });

} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the executive board votes and positions of officers
 * in a business account.
 */
void database::update_business_account_set()
{ try {
   if( (head_block_num() % SET_UPDATE_BLOCK_INTERVAL) != 0 )    // Runs once per day
      return;

   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const auto& business_idx = get_index< account_business_index >().indices().get< by_account >();
   auto business_itr = business_idx.begin();

   while( business_itr != business_idx.end() )
   {
      update_business_account( *business_itr, props );
      ++business_itr;
   }
   
} FC_CAPTURE_AND_RETHROW() }


/**
 * Allocates equity asset dividends from each dividend reward pool,
 * according to proportional balances.
 */
void database::process_power_rewards()
{ try {
   if( (head_block_num() % EQUITY_INTERVAL_BLOCKS) != 0 )    // Runs once per week
      return;

   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   
   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol_stake >();
   const reward_fund_object& rfo = get_reward_fund();
   asset power_reward_balance = rfo.power_reward_balance;       // Record the opening balance of the power reward fund
   auto balance_itr = balance_idx.lower_bound( SYMBOL_COIN );
   flat_map < account_name_type, share_type > power_map;
   share_type total_power_shares = 0;
   asset distributed = asset( 0, SYMBOL_COIN );

   while( balance_itr != balance_idx.end() && 
      balance_itr->symbol == SYMBOL_COIN && 
      balance_itr->staked_balance >= BLOCKCHAIN_PRECISION )
   {
      share_type power_shares = balance_itr->staked_balance;  // Get the staked balance for each stakeholder

      if( power_shares > 0 )
      {
         total_power_shares += power_shares;
         power_map[ balance_itr->owner] = power_shares;
      }
      ++balance_itr;
   }

   for( auto b : power_map )
   {
      asset power_reward = ( power_reward_balance * b.second ) / total_power_shares; 
      adjust_staked_balance( b.first, power_reward );       // Pay equity dividend to each stakeholder account proportionally.
      distributed += power_reward;
   }

   modify( rfo, [&]( reward_fund_object& r )
   {
      r.adjust_power_reward_balance( -distributed ); 
      
   });

   adjust_pending_supply( -distributed );   // Deduct distributed amount from pending supply.
      
} FC_CAPTURE_AND_RETHROW() }


/**
 * Calaulates the relative share of equity reward dividend distribution that an account should recieve
 * based on its balances, and account activity.
 */
share_type database::get_equity_shares( const account_balance_object& balance, const asset_equity_data_object& equity )
{
   const account_object& account = get_account( balance.owner );
   time_point now = head_block_time();
   if( ( account.witnesses_voted_for < equity.options.min_witnesses ) || 
      ( now > (account.last_activity_reward + equity.options.min_active_time ) ) )
   {
      return 0;  // Account does not recieve equity reward when witness votes or last activity are insufficient.
   }

   share_type equity_shares = 0;
   equity_shares += ( equity.options.liquid_dividend_percent * balance.liquid_balance ) / PERCENT_100;
   equity_shares += ( equity.options.staked_dividend_percent * balance.staked_balance ) / PERCENT_100;
   equity_shares += ( equity.options.savings_dividend_percent * balance.savings_balance ) / PERCENT_100; 

   if( (balance.staked_balance >= equity.options.boost_balance ) &&
      (account.witnesses_voted_for >= equity.options.boost_witnesses ) &&
      (account.recent_activity_claims >= equity.options.boost_activity ) )
   {
      equity_shares *= 2;    // Doubles equity reward when 10+ WYM balance, 50+ witness votes, and 15+ Activity rewards in last 30 days
   }

   if( account.membership == TOP_MEMBERSHIP ) 
   {
      equity_shares = (equity_shares * equity.options.boost_top ) / PERCENT_100;
   }

   return equity_shares;
}


/**
 * Allocates equity asset dividends from each dividend reward pool,
 * according to proportional balances.
 */
void database::process_equity_rewards()
{ try {
   if( (head_block_num() % EQUITY_INTERVAL_BLOCKS) != 0 )    // Runs once per week
      return;
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   const auto& equity_idx = get_index< asset_equity_data_index >().indices().get< by_symbol >();
   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol_stake >();
   auto equity_itr = equity_idx.begin();

   while( equity_itr != equity_idx.end() )
   {
      const asset_equity_data_object& equity = *equity_itr;
      
      if( equity.dividend_pool.amount > 0 )
      {
         asset equity_reward_balance = equity.dividend_pool;  // Record the opening balance of the equity reward fund
         auto balance_itr = balance_idx.lower_bound( equity.symbol );
         flat_map < account_name_type, share_type > equity_map;
         share_type total_equity_shares = 0;
         asset distributed = asset( 0, equity.dividend_asset );

         while( balance_itr != balance_idx.end() &&
            balance_itr->symbol == equity.symbol ) 
         {
            share_type equity_shares = get_equity_shares( *balance_itr , equity );  // Get the equity shares for each stakeholder

            if( equity_shares > 0 )
            {
               total_equity_shares += equity_shares;
               equity_map[ balance_itr->owner] = equity_shares;
            }
            ++balance_itr;
         }

         for( auto b : equity_map )
         {
            asset equity_reward = ( equity_reward_balance * b.second ) / total_equity_shares; 
            adjust_reward_balance( b.first, equity_reward );       // Pay equity dividend to each stakeholder account proportionally.
            distributed += equity_reward;
         }

         modify( equity, [&]( asset_equity_data_object& e )
         {
            e.adjust_pool( -distributed ); 
            e.last_dividend = now;        // Remove the distributed amount from the dividend pool.
         });

         adjust_pending_supply( -distributed );   // Deduct distributed amount from pending supply.
      }
      ++equity_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the amount of proof of work required for a proof 
 * to be valid, targets a mining time interval of 10 minutes.
 * using a moving average of 7 days.
 */
void database::update_proof_of_work_target()
{ try {
   if( (head_block_num() % POW_UPDATE_BLOCK_INTERVAL) != 0 )    // Runs once per week
      return;

   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const witness_schedule_object& wso = get_witness_schedule();
   uint128_t recent_pow = wso.recent_pow;        // Amount of proofs of work, times block precision, decayed over 7 days
   uint128_t target_pow = ( BLOCKCHAIN_PRECISION * wso.pow_decay_time.to_seconds() ) / wso.pow_target_time.to_seconds();
   uint128_t new_difficulty = ( wso.pow_target_difficulty * target_pow ) / recent_pow;
   time_point now = props.time;

   modify( wso, [&]( witness_schedule_object& w )
   {
      w.pow_target_difficulty = new_difficulty;
      w.decay_pow( now );
   });

} FC_CAPTURE_AND_RETHROW() }


void database::claim_proof_of_work_reward( const account_name_type& miner )
{ try {
   const reward_fund_object& rfo = get_reward_fund();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   const witness_schedule_object& wso = get_witness_schedule();
   asset pow_reward = rfo.work_reward_balance;
   const witness_object& witness = get_witness( miner );

   modify( witness, [&]( witness_object& w )
   {
      w.mining_power += BLOCKCHAIN_PRECISION;
      w.mining_count ++;
      w.last_pow_time = now;
      w.decay_weights( now, wso );
   });

   modify( wso, [&]( witness_schedule_object& w )
   {
      w.recent_pow += BLOCKCHAIN_PRECISION;
      w.decay_pow( now );
   });

   modify( rfo, [&]( reward_fund_object& r )
   {
      r.adjust_work_reward_balance( -pow_reward );
   });

   adjust_reward_balance( miner, pow_reward );
   adjust_pending_supply( -pow_reward );

} FC_CAPTURE_AND_RETHROW() }

/**
 * Distributes the transaction stake reward to all block producers
 * according to the amount of stake weighted transactions included in 
 * blocks. Each transaction included in a block adds the size of the transaction
 * multipled 
 */
void database::process_txn_stake_rewards()
{ try {
   if( (head_block_num() % TXN_STAKE_BLOCK_INTERVAL) != 0 )    // Runs once per week
      return;

   const reward_fund_object& rfo = get_reward_fund();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   const witness_schedule_object& wso = get_witness_schedule();
   asset txn_stake_reward = rfo.txn_stake_reward_balance;     // Record the opening balance of the transaction stake reward fund
   const auto& witness_idx = get_index< witness_index >().indices().get< by_txn_stake_weight >();
   auto witness_itr = witness_idx.begin();
    
   flat_map < account_name_type, share_type > stake_map;
   share_type total_stake_shares = 0;
   asset distributed = asset( 0, SYMBOL_COIN );

   while( witness_itr != witness_idx.end() &&
      witness_itr->recent_txn_stake_weight > BLOCKCHAIN_PRECISION ) 
   {
      share_type stake_shares = witness_itr->recent_txn_stake_weight;  // Get the recent txn stake for each witness

      if( stake_shares > 0 )
      {
         total_stake_shares += stake_shares;
         stake_map[ witness_itr->owner ] = stake_shares;
      }
      ++witness_itr;
   }

   for( auto b : stake_map )
   {
      asset stake_reward = ( txn_stake_reward * b.second ) / total_stake_shares; 
      adjust_reward_balance( b.first, stake_reward );       // Pay transaction stake reward to each block producer proportionally.
      distributed += stake_reward;
   }

   modify( rfo, [&]( reward_fund_object& r )
   {
      r.adjust_txn_stake_reward_balance( -distributed );     // Remove the distributed amount from the reward pool.
   });

   adjust_pending_supply( -distributed );   // Deduct distributed amount from pending supply.

} FC_CAPTURE_AND_RETHROW() }


/**
 * Distributes the block reward for validating blocks to witnesses
 * and miners according to the stake weight of their commitment transactions
 * upon the block becoming irreversible after majority of producers have created
 * a block on top of it.
 * This enables nodes to have a lower finality time in
 * cases where producers a majority of producers commit to a newly 
 * created block before it becomes irreversible.
 * Nodes will treat the blocks that they commit to as irreversible when
 * greater than two third of producers also commit to the same block.
 */
void database::process_validation_rewards()
{ try {
   const reward_fund_object& rfo = get_reward_fund();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   const witness_schedule_object& wso = get_witness_schedule();
   asset validation_reward = rfo.validation_reward_balance;     // Record the opening balance of the validation reward fund
   const auto& valid_idx = get_index< block_validation_index >().indices().get< by_height_stake >();
   auto valid_itr = valid_idx.lower_bound( props.last_irreversible_block_num );
    
   flat_map < account_name_type, share_type > validation_map;
   share_type total_validation_shares = 0;
   asset distributed = asset( 0, SYMBOL_COIN );

   while( valid_itr != valid_idx.end() &&
      valid_itr->height == props.last_irreversible_block_num &&
      valid_itr->stake.amount >= BLOCKCHAIN_PRECISION ) 
   {
      share_type validation_shares = valid_itr->stake;  // Get the validation stake on each commitment for each witness

      if( validation_shares > 0 )
      {
         total_validation_shares += validation_shares;
         validation_map[ valid_itr->producer ] = validation_shares;
      }
      ++valid_itr;
   }

   for( auto b : validation_map )
   {
      asset validation_reward = ( validation_reward * b.second ) / total_validation_shares; 
      adjust_reward_balance( b.first, validation_reward );       // Pay transaction validation reward to each block producer proportionally.
      distributed += validation_reward;
   }

   modify( rfo, [&]( reward_fund_object& r )
   {
      r.adjust_validation_reward_balance( -distributed );     // Remove the distributed amount from the reward pool.
   });

   adjust_pending_supply( -distributed );   // Deduct distributed amount from pending supply.

} FC_CAPTURE_AND_RETHROW() }


/** 
 * Rewards witnesses when they have the current highest accumulated 
 * activity stake. Each time an account produces an activity reward transaction, 
 * they implicitly nominate thier highest voted witness to receive a daily vote as their Prime Witness.
 * Award is distributed every eight hours to the leader by activity stake.
 * This incentivizes witnesses to campign to achieve prime witness designation with 
 * high stake, active accounts, in a competitive manner against other block producers. 
 */
void database::process_producer_activity_rewards()
{ try {
   if( (head_block_num() % POA_BLOCK_INTERVAL ) != 0 )    // Runs once per 8 hours.
      return;
   
   const reward_fund_object& rfo = get_reward_fund();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   const witness_schedule_object& wso = get_witness_schedule();
   asset poa_reward = rfo.producer_activity_reward_balance;          // Record the opening balance of the witness activity reward fund.
   const auto& witness_idx = get_index< witness_index >().indices().get< by_activity_stake >();
   auto witness_itr = witness_idx.begin();

   if( witness_itr != witness_idx.end() )
   {
      const witness_object& prime_witness = *witness_itr;

      modify( prime_witness, [&]( witness_object& w )
      {
         w.accumulated_activity_stake = 0;     // Reset activity stake for top witness.
      });

      modify( rfo, [&]( reward_fund_object& r )
      {
         r.adjust_producer_activity_reward_balance( -poa_reward );     // Remove the distributed amount from the reward pool.
      });

      adjust_reward_balance( prime_witness.owner, poa_reward );   // Pay witness activity reward to the witness with the highest accumulated activity stake.
      adjust_pending_supply( -poa_reward );        // Deduct distributed amount from pending supply.
   }

} FC_CAPTURE_AND_RETHROW() }



void database::process_supernode_rewards()
{ try {
   if( (head_block_num() % SUPERNODE_BLOCK_INTERVAL ) != 0 )    // Runs once per day.
      return;

   const reward_fund_object& rfo = get_reward_fund();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   asset supernode_reward = rfo.supernode_reward_balance;     // Record the opening balance of the supernode reward fund

   const auto& supernode_idx = get_index< supernode_index >().indices().get< by_view_weight >();
   auto supernode_itr = supernode_idx.begin();
    
   flat_map < account_name_type, share_type > supernode_map;
   share_type total_supernode_shares = 0;
   asset distributed = asset( 0, SYMBOL_COIN );

   while( supernode_itr != supernode_idx.end() ) 
   {
      share_type supernode_shares = supernode_itr->recent_view_weight;  // Get the supernode view weight for rewards

      if( supernode_shares > 0 && supernode_itr->active && now > ( supernode_itr->last_activation_time + fc::days(1) ) )
      {
         total_supernode_shares += supernode_shares;
         supernode_map[ supernode_itr->account ] = supernode_shares;
      }
      ++supernode_itr;
   }

   for( auto b : supernode_map )
   {
      asset supernode_reward_split = ( supernode_reward * b.second ) / total_supernode_shares; 
      adjust_reward_balance( b.first, supernode_reward_split );       // Pay supernode reward proportionally with view weight.
      distributed += supernode_reward_split;
   }

   modify( rfo, [&]( reward_fund_object& r )
   {
      r.adjust_supernode_reward_balance( -distributed );     // Remove the distributed amount from the reward pool.
   });

   adjust_pending_supply( -distributed );   // Deduct distributed amount from pending supply.

} FC_CAPTURE_AND_RETHROW() }



/**
 * Update an network officer;'s voting approval statisitics
 * and updates its approval if there are
 * sufficient votes from witnesses and other accounts.
 */
void database::update_network_officer( const network_officer_object& network_officer, 
   const witness_schedule_object& wso, const dynamic_global_property_object& props )
{ try {
   uint32_t vote_count = 0;
   share_type voting_power = 0;
   uint32_t witness_vote_count = 0;
   share_type witness_voting_power = 0;
   price equity_price = get_liquidity_pool(SYMBOL_COIN, SYMBOL_EQUITY).hour_median_price;

   const auto& vote_idx = get_index< network_officer_vote_index >().indices().get< by_officer_account >();
   auto vote_itr = vote_idx.lower_bound( network_officer.account );

   while( vote_itr != vote_idx.end() && 
      vote_itr->network_officer == network_officer.account )
   {
      const network_officer_vote_object& vote = *vote_itr;
      const account_object& voter = get_account( vote.account );
      bool is_witness = wso.is_top_witness( voter.name );
      vote_count++;
      share_type weight = 0;
      weight += get_voting_power( vote.account, equity_price );
      if( voter.proxied.size() )
      {
         weight += get_proxied_voting_power( voter, equity_price );
      }
      voting_power += ( weight.value >> vote.vote_rank );

      if( is_witness )
      {
         witness_vote_count++;
         const witness_object& witness = get_witness( voter.name );
         witness_voting_power += ( witness.voting_power.value >> vote.vote_rank );
      }
      ++vote_itr;
   }

   // Approve the network officer when a threshold of voting power and vote amount supports it.
   bool approve_officer = ( vote_count >= VOTE_THRESHOLD_AMOUNT * 4 ) &&
      ( witness_vote_count >= VOTE_THRESHOLD_AMOUNT ) &&
      ( voting_power >= ( props.total_voting_power * VOTE_THRESHOLD_PERCENT ) / PERCENT_100 ) &&
      ( witness_voting_power >= ( wso.total_witness_voting_power * VOTE_THRESHOLD_PERCENT ) / PERCENT_100 );
   
   modify( network_officer, [&]( network_officer_object& n )
   {
      n.vote_count = vote_count;
      n.voting_power = voting_power;
      n.witness_vote_count = witness_vote_count;
      n.witness_voting_power = witness_voting_power;
      n.officer_approved = approve_officer;
   });
} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays the network officer rewards to the 50 highest voted
 * developers, marketers and advocates on the network from
 * the reward fund, once per day.
 */
void database::process_network_officer_rewards()
{ try {
   if( (head_block_num() % NETWORK_OFFICER_BLOCK_INTERVAL ) != 0 )    // Runs once per day.
      return;

   const reward_fund_object& rfo = get_reward_fund();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const witness_schedule_object& wso = get_witness_schedule();
   const auto& officer_idx = get_index< network_officer_index >().indices().get< by_type_voting_power >();
   time_point now = props.time;
   auto officer_itr = officer_idx.begin();

   while( officer_itr != officer_idx.end() ) 
   {
      update_network_officer( *officer_itr, wso, props );
      ++officer_itr;
   }

   // ========== Development Officers ========== //

   asset development_reward = rfo.development_reward_balance;     // Record the opening balance of the development reward fund
   auto development_itr = officer_idx.lower_bound( DEVELOPMENT );
   auto development_end = officer_idx.upper_bound( DEVELOPMENT );
   flat_map < account_name_type, share_type > development_map;
   share_type total_development_shares = 0;
   asset development_distributed = asset( 0, SYMBOL_COIN );

   while( development_itr != development_end && development_map.size() < NETWORK_OFFICER_ACTIVE_SET ) 
   {
      share_type development_shares = development_itr->voting_power;  // Get the development officer voting power

      if( development_shares > 0 && development_itr->active && development_itr->officer_approved )
      {
         total_development_shares += development_shares;
         development_map[ development_itr->account ] = development_shares;
      }
      ++development_itr;
   }

   for( auto b : development_map )
   {
      asset development_reward_split = ( development_reward * b.second ) / total_development_shares; 
      adjust_reward_balance( b.first, development_reward_split );       // Pay development reward proportionally with voting power.
      development_distributed += development_reward_split;
   }

   // ========== Marketing Officers ========== //

   asset marketing_reward = rfo.marketing_reward_balance;     // Record the opening balance of the marketing reward fund
   auto marketing_itr = officer_idx.lower_bound( MARKETING );
   auto marketing_end = officer_idx.upper_bound( MARKETING );
   flat_map < account_name_type, share_type > marketing_map;
   share_type total_marketing_shares = 0;
   asset marketing_distributed = asset( 0, SYMBOL_COIN );

   while( marketing_itr != marketing_end && marketing_map.size() < NETWORK_OFFICER_ACTIVE_SET ) 
   {
      share_type marketing_shares = marketing_itr->voting_power;  // Get the marketing officer voting power

      if( marketing_shares > 0 && marketing_itr->active && marketing_itr->officer_approved )
      {
         total_marketing_shares += marketing_shares;
         marketing_map[ marketing_itr->account ] = marketing_shares;
      }
      ++marketing_itr;
   }

   for( auto b : marketing_map )
   {
      asset marketing_reward_split = ( marketing_reward * b.second ) / total_marketing_shares; 
      adjust_reward_balance( b.first, marketing_reward_split );       // Pay marketing reward proportionally with voting power.
      marketing_distributed += marketing_reward_split;
   }

   // ========== Advocacy Officers ========== //

   asset advocacy_reward = rfo.advocacy_reward_balance;     // Record the opening balance of the advocacy reward fund
   auto advocacy_itr = officer_idx.lower_bound( ADVOCACY );
   auto advocacy_end = officer_idx.upper_bound( ADVOCACY );
   flat_map < account_name_type, share_type > advocacy_map;
   share_type total_advocacy_shares = 0;
   asset advocacy_distributed = asset( 0, SYMBOL_COIN );

   while( advocacy_itr != advocacy_end && advocacy_map.size() < NETWORK_OFFICER_ACTIVE_SET ) 
   {
      share_type advocacy_shares = advocacy_itr->voting_power;  // Get the advocacy officer voting power

      if( advocacy_shares > 0 && advocacy_itr->active && advocacy_itr->officer_approved )
      {
         total_advocacy_shares += advocacy_shares;
         advocacy_map[ advocacy_itr->account ] = advocacy_shares;
      }
      ++advocacy_itr;
   }

   for( auto b : advocacy_map )
   {
      asset advocacy_reward_split = ( advocacy_reward * b.second ) / total_advocacy_shares; 
      adjust_reward_balance( b.first, advocacy_reward_split );       // Pay advocacy reward proportionally with voting power.
      advocacy_distributed += advocacy_reward_split;
   }

   modify( rfo, [&]( reward_fund_object& r )
   {
      r.adjust_development_reward_balance( -development_distributed );   
      r.adjust_marketing_reward_balance( -marketing_distributed );   
      r.adjust_advocacy_reward_balance( -advocacy_distributed );  
   });
   asset total_distributed = development_distributed + marketing_distributed + advocacy_distributed;
   adjust_pending_supply( -total_distributed );   // Deduct distributed amount from pending supply.

} FC_CAPTURE_AND_RETHROW() }


/**
 * Update an executive board's voting approval statisitics
 * and update its approval if there are
 * sufficient votes from witnesses and other accounts.
 */
void database::update_executive_board( const executive_board_object& executive_board, 
   const witness_schedule_object& wso, const dynamic_global_property_object& props )
{ try {
   uint32_t vote_count = 0;
   share_type voting_power = 0;
   uint32_t witness_vote_count = 0;
   share_type witness_voting_power = 0;
   price equity_price = get_liquidity_pool(SYMBOL_COIN, SYMBOL_EQUITY).hour_median_price;

   const auto& vote_idx = get_index< executive_board_vote_index >().indices().get< by_executive_account >();
   auto vote_itr = vote_idx.lower_bound( executive_board.account );

   while( vote_itr != vote_idx.end() && 
      vote_itr->executive_board == executive_board.account )
   {
      const executive_board_vote_object& vote = *vote_itr;
      const account_object& voter = get_account( vote.account );
      bool is_witness = wso.is_top_witness( voter.name );
      vote_count++;
      share_type weight = 0;
      weight += get_voting_power( vote.account, equity_price );
      if( voter.proxied.size() )
      {
         weight += get_proxied_voting_power( voter, equity_price );
      }
      voting_power += ( weight.value >> vote.vote_rank );

      if( is_witness )
      {
         witness_vote_count++;
         const witness_object& witness = get_witness( voter.name );
         witness_voting_power += ( witness.voting_power.value >> vote.vote_rank );
      }
      ++vote_itr;
   }

   // Approve the executive board when a threshold of votes to support its budget.
   bool approve_board = ( vote_count >= VOTE_THRESHOLD_AMOUNT * 8 ) &&
      ( witness_vote_count >= VOTE_THRESHOLD_AMOUNT * 2 ) &&
      ( voting_power >= ( props.total_voting_power * VOTE_THRESHOLD_PERCENT * 2 ) / PERCENT_100 ) &&
      ( witness_voting_power >= ( wso.total_witness_voting_power * VOTE_THRESHOLD_PERCENT * 2 ) / PERCENT_100 );
   
   modify( executive_board, [&]( executive_board_object& e )
   {
      e.vote_count = vote_count;
      e.voting_power = voting_power;
      e.witness_vote_count = witness_vote_count;
      e.witness_voting_power = witness_voting_power;
      e.board_approved = approve_board;
   });
} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays the requested budgets fo the top 5 voted executive boards on the network, that have
 * sufficient approval from accounts and witnesses once per day.
 * Price of network credit asset must be greater than $0.90 USD to issue new units, or 
 * executive budgets are suspended. 
 * Network credit is a digital fiat currency that is issued to executive boards
 * for expenses of managing a network development team. Its value is derived from
 * buybacks from network revenue, up to a face value of $1.00 USD
 * per credit, and interest payments for balance holders.
 * Holding Credit assets are economically equivalent to holding bonds
 * for debt lent to the network. 
 */
void database::process_executive_board_budgets()
{ try {
   if( (head_block_num() % EXECUTIVE_BOARD_BLOCK_INTERVAL ) != 0 )    // Runs once per day.
      return;

   const witness_schedule_object& wso = get_witness_schedule();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   price credit_usd_price = get_liquidity_pool( SYMBOL_USD, SYMBOL_CREDIT ).hour_median_price;

   const auto& exec_idx = get_index< executive_board_index >().indices().get< by_voting_power >();
   auto exec_itr = exec_idx.begin();
   uint16_t board_count = 0;

   while( exec_itr != exec_idx.end() )   // update all executive board approvals and vote statistics. 
   {
      const executive_board_object& exec = *exec_itr;
      update_executive_board( exec, wso, props );
      ++exec_itr;
   }

   if( credit_usd_price > MIN_EXEC_CREDIT_PRICE )
   {
      auto exec_itr = exec_idx.begin(); // reset iterator;

      while( exec_itr != exec_idx.end() && board_count < EXECUTIVE_BOARD_ACTIVE_SET )   // Pay the budget requests of the top 5 approved boards.
      {
         const executive_board_object& exec = *exec_itr;

         if( exec.board_approved )
         {
            FC_ASSERT( exec.budget.symbol == SYMBOL_CREDIT , 
               "Executive Budget must be in the network credit asset." );
            adjust_liquid_balance( exec.account, exec.budget );     // Issues new supply of credit asset to pay executive board.
            board_count++;
         }
         ++exec_itr;
      }
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Update a governance account's voting approval statisitics
 * and update its approval if there are
 * sufficient votes from witnesses and other accounts.
 */
void database::update_governance_account( const governance_account_object& governance_account, 
   const witness_schedule_object& wso, const dynamic_global_property_object& props )
{ try {
   uint32_t vote_count = 0;
   share_type voting_power = 0;
   uint32_t witness_vote_count = 0;
   share_type witness_voting_power = 0;
   price equity_price = get_liquidity_pool(SYMBOL_COIN, SYMBOL_EQUITY).hour_median_price;

   const auto& vote_idx = get_index< governance_subscription_index >().indices().get< by_governance_account >();
   auto vote_itr = vote_idx.lower_bound( governance_account.account );

   while( vote_itr != vote_idx.end() && 
      vote_itr->governance_account == governance_account.account )
   {
      const governance_subscription_object& vote = *vote_itr;
      const account_object& voter = get_account( vote.account );
      bool is_witness = wso.is_top_witness( voter.name );
      vote_count++;
      share_type weight = 0;
      weight += get_voting_power( vote.account, equity_price );
      if( voter.proxied.size() )
      {
         weight += get_proxied_voting_power( voter, equity_price );
      }
      voting_power += ( weight.value >> vote.vote_rank );

      if( is_witness )
      {
         witness_vote_count++;
         const witness_object& witness = get_witness( voter.name );
         witness_voting_power += ( witness.voting_power.value >> vote.vote_rank );
      }
      ++vote_itr;
   }

   // Approve the executive board when a threshold of votes to support its budget.
   bool approve_account = ( vote_count >= VOTE_THRESHOLD_AMOUNT * 4 ) &&
      ( witness_vote_count >= VOTE_THRESHOLD_AMOUNT ) &&
      ( voting_power >= ( props.total_voting_power * VOTE_THRESHOLD_PERCENT ) / PERCENT_100 ) &&
      ( witness_voting_power >= ( wso.total_witness_voting_power * VOTE_THRESHOLD_PERCENT ) / PERCENT_100 );
   
   modify( governance_account, [&]( governance_account_object& g )
   {
      g.subscriber_count = vote_count;
      g.subscriber_power = voting_power;
      g.witness_subscriber_count = witness_vote_count;
      g.witness_subscriber_power = witness_voting_power;
      g.account_approved = approve_account;
   });
} FC_CAPTURE_AND_RETHROW() }


void database::update_governance_account_set()
{ try { 
   if( (head_block_num() % SET_UPDATE_BLOCK_INTERVAL ) != 0 )    // Runs once per day
      return;
   
   const witness_schedule_object& wso = get_witness_schedule();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const auto& gov_idx = get_index< governance_account_index >().indices().get< by_subscriber_power >();
   auto gov_itr = gov_idx.begin();
   
   while( gov_itr != gov_idx.end() )
   {
      update_governance_account( *gov_itr, wso, props );
      ++gov_itr;
   }

} FC_CAPTURE_AND_RETHROW() }


/**
 * Update a community enterprise proposal's voting approval statisitics
 * and increment the approved milestone if there are
 * sufficient current approvals from witnesses and other accounts.
 */
void database::update_enterprise( const community_enterprise_object& enterprise, 
   const witness_schedule_object& wso, const dynamic_global_property_object& props )
{ try {
   uint32_t total_approvals = 0;
   share_type total_voting_power = 0;
   uint32_t total_witness_approvals = 0;
   share_type total_witness_voting_power = 0;
   uint32_t current_approvals = 0;
   share_type current_voting_power = 0;
   uint32_t current_witness_approvals = 0;
   share_type current_witness_voting_power = 0;
   price equity_price = get_liquidity_pool(SYMBOL_COIN, SYMBOL_EQUITY).hour_median_price;

   const auto& approval_idx = get_index< enterprise_approval_index >().indices().get< by_enterprise_id >();
   auto approval_itr = approval_idx.lower_bound( boost::make_tuple( enterprise.creator, enterprise.enterprise_id ) );

   while( approval_itr != approval_idx.end() && 
      approval_itr->creator == enterprise.creator && 
      approval_itr->enterprise_id == enterprise.enterprise_id )
   {
      const enterprise_approval_object& approval = *approval_itr;
      const account_object& voter = get_account( approval.account );
      bool is_witness = wso.is_top_witness( voter.name );
      total_approvals++;
      total_voting_power += get_voting_power( approval.account, equity_price );
      if( voter.proxied.size() )
      {
         total_voting_power += get_proxied_voting_power( voter, equity_price );
      }

      if( is_witness )
      {
         total_witness_approvals++;
         const witness_object& witness = get_witness( voter.name );
         total_witness_voting_power += witness.voting_power;
      }

      if( approval.milestone == enterprise.claimed_milestones ) // approval is current 
      {
         current_approvals++;
         current_voting_power += get_voting_power( approval.account, equity_price );
         if( voter.proxied.size() )
         {
            current_voting_power += get_proxied_voting_power( voter, equity_price );
         }

         if( is_witness )
         {
            current_witness_approvals++;
            const witness_object& witness = get_witness( voter.name );
            current_witness_voting_power += witness.voting_power;
         }
      }
      ++approval_itr;
   }

   // Approve the latest claimed milestone when a threshold of approvals support its release.
   bool approve_milestone = ( current_approvals >= VOTE_THRESHOLD_AMOUNT * 4 ) &&
      ( current_witness_approvals >= VOTE_THRESHOLD_AMOUNT ) &&
      ( current_voting_power >= ( props.total_voting_power * VOTE_THRESHOLD_PERCENT ) / PERCENT_100 ) &&
      ( current_witness_voting_power >= ( wso.total_witness_voting_power * VOTE_THRESHOLD_PERCENT ) / PERCENT_100 );

   modify( enterprise, [&]( community_enterprise_object& e )
   {
      e.total_approvals = total_approvals;
      e.total_voting_power = total_voting_power;
      e.total_witness_approvals = total_witness_approvals;
      e.total_witness_voting_power = total_witness_voting_power;
      e.current_approvals = current_approvals;
      e.current_voting_power = current_voting_power;
      e.current_witness_approvals = current_witness_approvals;
      e.current_witness_voting_power = current_witness_voting_power;
      if( approve_milestone )
      {
         e.approved_milestones = e.claimed_milestones;
      }
   });
} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates all community enterprise proposals, by checking 
 * if they have sufficient approvals from accounts on the network
 * and witnesses.
 * Processes budget payments for all proposals that have milestone approvals.
 */
void database::process_community_enterprise_fund()
{ try {
   if( (head_block_num() % ENTERPRISE_BLOCK_INTERVAL ) != 0 )    // Runs once per day.
      return;

   const reward_fund_object& rfo = get_reward_fund();
   const witness_schedule_object& wso = get_witness_schedule();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   const auto& enterprise_idx = get_index< community_enterprise_index >().indices().get< by_total_voting_power >();
   auto enterprise_itr = enterprise_idx.begin();

   while( enterprise_itr != enterprise_idx.end() ) 
   {
      update_enterprise( *enterprise_itr, wso, props );
      ++enterprise_itr;
   }

   auto enterprise_itr = enterprise_idx.begin();

   while( enterprise_itr != enterprise_idx.end() && 
      rfo.community_fund_balance.amount > 0 )   // Enterprise objects in order of highest total voting power.
   {
      if( enterprise_itr->approved_milestones >= 0 )  // Processed when they have inital approval.
      {
         const community_enterprise_object& enterprise = *enterprise_itr;
         asset available_budget = std::min( rfo.community_fund_balance, enterprise.daily_budget );

         if( enterprise.duration > enterprise.days_paid )
         {
            modify( rfo, [&]( reward_fund_object& r )
            {
               r.adjust_community_fund_balance( available_budget );     // Remove the distributed amount from the reward pool.
            });

            modify( enterprise, [&]( community_enterprise_object& e )
            {
               e.adjust_pending_budget( available_budget );     // Pay daily budget to enterprise proposal
               e.days_paid++;
            });
         }

         uint16_t percent_released = 0;

         for( auto i = 0; i <= enterprise.approved_milestones; i++ )
         {
            percent_released += enterprise.milestones[ i ].second;  // Accumulate all approved milestone percentages
         }

         asset release_limit = ( enterprise.total_budget() * percent_released ) / PERCENT_100;
         asset to_release = std::min( enterprise.pending_budget, release_limit - enterprise.total_distributed );
         
         if( to_release.amount > 0)
         {
            asset distributed = asset( 0, SYMBOL_COIN );

            for( auto b : enterprise.beneficiaries )
            {
               asset release_split = ( to_release * b.second ) / PERCENT_100; 
               adjust_liquid_balance( b.first, release_split );       // Pay proposal beneficiaries according to percentage split.
               distributed += release_split;
            }

            adjust_pending_supply( -distributed );   // Deduct distributed amount from pending supply.

            modify( enterprise, [&]( community_enterprise_object& e )
            {
               e.adjust_pending_budget( -distributed );
               e.total_distributed += distributed;
            });
         }
      }
      ++enterprise_itr;
   }
} FC_CAPTURE_AND_RETHROW() }



void database::adjust_view_weight( const supernode_object& supernode, share_type delta, bool adjust = true )
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();

   modify( supernode, [&]( supernode_object& s )
   {
      s.decay_weights( props );
      s.recent_view_weight += delta;

      if( adjust )
      {
         s.daily_active_users += PERCENT_100;
         s.monthly_active_users += PERCENT_100;
      }
   });

} FC_CAPTURE_AND_RETHROW() }



void database::adjust_interface_users( const interface_object& interface, bool adjust = true )
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();

   modify( interface, [&]( interface_object& i )
   {
      i.decay_weights( props );

      if( adjust )
      {
         i.daily_active_users += PERCENT_100;
         i.monthly_active_users += PERCENT_100;
      }
   });

} FC_CAPTURE_AND_RETHROW() }


/**
 *  Overall the network has a MeCoin Issuance rate of one Billion per year
 *   25% of issuance is directed to Content Creator rewards
 *   20% of issuance is directed to Equity Holder rewards
 *   20% of issuance is directed to Block producers (Witnesses + Miners)
 *   10% of issuance is directed to Supernode Operator rewards
 *   10% of issuance is directed to Staked MeCoin Holder rewards
 *    5% of issuance is directed to The Community Enterprise fund
 *  2.5% of issuance is directed to The Development reward pool
 *  2.5% of issuance is directed to The Marketing reward pool
 *  2.5% of issuance is directed to The Advocacy reward pool
 *  2.5% of issuance is directed to The Activity reward pool
 *  This method pays out Staked and liquid COIN every block to all network contributors
 */
void database::process_funds()
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const witness_schedule_object& wso = get_witness_schedule();
   const witness_object& current_producer = get_witness( props.current_producer );
   const account_object& witness_account = get_account( props.current_producer );

   asset block_reward = BLOCK_REWARD;

   asset content_reward            = ( block_reward * CONTENT_REWARD_PERCENT          ) / PERCENT_100;
   asset equity_reward             = ( block_reward * EQUITY_REWARD_PERCENT           ) / PERCENT_100;
   asset producer_reward           = ( block_reward * PRODUCER_REWARD_PERCENT         ) / PERCENT_100;
   asset supernode_reward          = ( block_reward * SUPERNODE_REWARD_PERCENT        ) / PERCENT_100;
   asset power_reward              = ( block_reward * POWER_REWARD_PERCENT            ) / PERCENT_100;
   asset community_fund_reward     = ( block_reward * COMMUNITY_FUND_PERCENT          ) / PERCENT_100;
   asset development_reward        = ( block_reward * DEVELOPMENT_REWARD_PERCENT      ) / PERCENT_100;
   asset marketing_reward          = ( block_reward * MARKETING_REWARD_PERCENT        ) / PERCENT_100;
   asset advocacy_reward           = ( block_reward * ADVOCACY_REWARD_PERCENT         ) / PERCENT_100;
   asset activity_reward           = ( block_reward * ACTIVITY_REWARD_PERCENT         ) / PERCENT_100;

   asset producer_block_reward     = ( producer_reward * PRODUCER_BLOCK_PERCENT       ) / PERCENT_100;
   asset validation_reward         = ( producer_reward * PRODUCER_VALIDATOR_PERCENT   ) / PERCENT_100;
   asset txn_stake_reward          = ( producer_reward * PRODUCER_TXN_STAKE_PERCENT   ) / PERCENT_100;
   asset work_reward               = ( producer_reward * PRODUCER_WORK_PERCENT        ) / PERCENT_100;
   asset producer_activity_reward  = ( producer_reward * PRODUCER_ACTIVITY_PERCENT    ) / PERCENT_100;

   asset producer_pending = validation_reward + txn_stake_reward + work_reward + producer_activity_reward;
   asset pending_issuance = content_reward + equity_reward + supernode_reward + power_reward + community_fund_reward + development_reward + marketing_reward + advocacy_reward + activity_reward;

   asset reward_checksum = content_reward + equity_reward + validation_reward + txn_stake_reward + work_reward + producer_activity_reward + producer_block_reward + supernode_reward + power_reward + community_fund_reward + development_reward + marketing_reward + advocacy_reward + activity_reward;
   FC_ASSERT( reward_checksum == BLOCK_REWARD, 
      "Block reward issuance checksum failed, allocation is invalid");
   
   const reward_fund_object& reward_fund = get_reward_fund();
   const asset_equity_data_object& equity = get_equity_data( SYMBOL_EQUITY );

   modify( reward_fund, [&]( reward_fund_object& rfo )
   {
      rfo.adjust_content_reward_balance(content_reward);
      rfo.adjust_validation_reward_balance( validation_reward );
      rfo.adjust_txn_stake_reward_balance( txn_stake_reward );
      rfo.adjust_work_reward_balance( work_reward );
      rfo.adjust_producer_activity_reward_balance( producer_activity_reward );
      rfo.adjust_supernode_reward_balance( supernode_reward );
      rfo.adjust_power_reward_balance( power_reward );
      rfo.adjust_community_fund_balance( community_fund_reward );
      rfo.adjust_development_reward_balance( development_reward );
      rfo.adjust_marketing_reward_balance( marketing_reward );
      rfo.adjust_advocacy_reward_balance( advocacy_reward );
      rfo.adjust_activity_reward_balance( activity_reward );
   });

   modify( equity, [&](asset_equity_data_object& aedo) 
   {
      aedo.adjust_pool( equity_reward );
   });

   adjust_reward_balance( witness_account, producer_block_reward );

   adjust_pending_supply( pending_issuance + producer_pending );
   
   push_virtual_operation( producer_reward_operation( witness_account.name, producer_block_reward ) );
   
} FC_CAPTURE_AND_RETHROW() }


void database::initialize_evaluators()
{
   // Account Evaluators

   _my->_evaluator_registry.register_evaluator< account_create_evaluator                 >();
   _my->_evaluator_registry.register_evaluator< account_update_evaluator                 >();
   _my->_evaluator_registry.register_evaluator< account_membership_evaluator             >();
   _my->_evaluator_registry.register_evaluator< account_vote_executive_evaluator         >();
   _my->_evaluator_registry.register_evaluator< account_vote_officer_evaluator           >();
   _my->_evaluator_registry.register_evaluator< account_member_request_evaluator         >();
   _my->_evaluator_registry.register_evaluator< account_member_invite_evaluator          >();
   _my->_evaluator_registry.register_evaluator< account_accept_request_evaluator         >();
   _my->_evaluator_registry.register_evaluator< account_accept_invite_evaluator          >();
   _my->_evaluator_registry.register_evaluator< account_remove_member_evaluator          >();
   _my->_evaluator_registry.register_evaluator< account_update_list_evaluator            >();
   _my->_evaluator_registry.register_evaluator< account_witness_vote_evaluator           >();
   _my->_evaluator_registry.register_evaluator< account_update_proxy_evaluator           >();
   _my->_evaluator_registry.register_evaluator< request_account_recovery_evaluator       >();
   _my->_evaluator_registry.register_evaluator< recover_account_evaluator                >();
   _my->_evaluator_registry.register_evaluator< reset_account_evaluator                  >();
   _my->_evaluator_registry.register_evaluator< set_reset_account_evaluator              >();
   _my->_evaluator_registry.register_evaluator< change_recovery_account_evaluator        >();
   _my->_evaluator_registry.register_evaluator< decline_voting_rights_evaluator          >();
   _my->_evaluator_registry.register_evaluator< connection_request_evaluator             >();
   _my->_evaluator_registry.register_evaluator< connection_accept_evaluator              >();
   _my->_evaluator_registry.register_evaluator< account_follow_evaluator                 >();
   _my->_evaluator_registry.register_evaluator< tag_follow_evaluator                     >();
   _my->_evaluator_registry.register_evaluator< activity_reward_evaluator                >();

   // Network Evaluators

   _my->_evaluator_registry.register_evaluator< update_network_officer_evaluator         >();
   _my->_evaluator_registry.register_evaluator< network_officer_vote_evaluator           >();
   _my->_evaluator_registry.register_evaluator< update_executive_board_evaluator         >();
   _my->_evaluator_registry.register_evaluator< executive_board_vote_evaluator           >();
   _my->_evaluator_registry.register_evaluator< update_governance_evaluator              >();
   _my->_evaluator_registry.register_evaluator< subscribe_governance_evaluator           >();
   _my->_evaluator_registry.register_evaluator< update_supernode_evaluator               >();
   _my->_evaluator_registry.register_evaluator< update_interface_evaluator               >();
   _my->_evaluator_registry.register_evaluator< create_community_enterprise_evaluator    >();
   _my->_evaluator_registry.register_evaluator< claim_enterprise_milestone_evaluator     >();
   _my->_evaluator_registry.register_evaluator< approve_enterprise_milestone_evaluator   >();

   // Comment Evaluators

   _my->_evaluator_registry.register_evaluator< comment_evaluator                        >();
   _my->_evaluator_registry.register_evaluator< comment_options_evaluator                >();
   _my->_evaluator_registry.register_evaluator< message_evaluator                        >();
   _my->_evaluator_registry.register_evaluator< vote_evaluator                           >();
   _my->_evaluator_registry.register_evaluator< view_evaluator                           >();
   _my->_evaluator_registry.register_evaluator< share_evaluator                          >();
   _my->_evaluator_registry.register_evaluator< moderation_tag_evaluator                 >();

   // Board Evaluators

   _my->_evaluator_registry.register_evaluator< board_create_operation                   >();
   _my->_evaluator_registry.register_evaluator< board_update_operation                   >();
   _my->_evaluator_registry.register_evaluator< board_add_mod_operation                  >();
   _my->_evaluator_registry.register_evaluator< board_add_admin_operation                >();
   _my->_evaluator_registry.register_evaluator< board_vote_mod_operation                 >();
   _my->_evaluator_registry.register_evaluator< board_transfer_ownership_operation       >();
   _my->_evaluator_registry.register_evaluator< board_join_request_operation             >();
   _my->_evaluator_registry.register_evaluator< board_join_accept_operation              >();
   _my->_evaluator_registry.register_evaluator< board_join_invite_operation              >();
   _my->_evaluator_registry.register_evaluator< board_invite_accept_operation            >();
   _my->_evaluator_registry.register_evaluator< board_remove_member_operation            >();
   _my->_evaluator_registry.register_evaluator< board_blacklist_operation                >();
   _my->_evaluator_registry.register_evaluator< board_subscribe_operation                >();

   // Advertising Evaluators

   _my->_evaluator_registry.register_evaluator< ad_creative_evaluator                    >();
   _my->_evaluator_registry.register_evaluator< ad_campaign_evaluator                    >();
   _my->_evaluator_registry.register_evaluator< ad_inventory_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< ad_audience_evaluator                    >();
   _my->_evaluator_registry.register_evaluator< ad_bid_evaluator                         >();
   _my->_evaluator_registry.register_evaluator< ad_deliver_evaluator                     >();

   // Transfer Evaluators

   _my->_evaluator_registry.register_evaluator< transfer_evaluator                       >();
   _my->_evaluator_registry.register_evaluator< transfer_request_evaluator               >();
   _my->_evaluator_registry.register_evaluator< transfer_accept_evaluator                >();
   _my->_evaluator_registry.register_evaluator< transfer_recurring_evaluator             >();
   _my->_evaluator_registry.register_evaluator< transfer_recurring_request_evaluator     >();
   _my->_evaluator_registry.register_evaluator< transfer_recurring_accept_evaluator      >();

   // Balance Evaluators

   _my->_evaluator_registry.register_evaluator< claim_reward_balance_evaluator           >();
   _my->_evaluator_registry.register_evaluator< stake_asset_evaluator                    >();
   _my->_evaluator_registry.register_evaluator< unstake_asset_evaluator                  >();
   _my->_evaluator_registry.register_evaluator< unstake_asset_route_evaluator            >();
   _my->_evaluator_registry.register_evaluator< transfer_to_savings_evaluator            >();
   _my->_evaluator_registry.register_evaluator< transfer_from_savings_evaluator          >();
   _my->_evaluator_registry.register_evaluator< cancel_transfer_from_savings_evaluator   >();
   _my->_evaluator_registry.register_evaluator< delegate_asset_evaluator                 >();
   
   // Escrow Evaluators
   
   _my->_evaluator_registry.register_evaluator< escrow_transfer_evaluator                >();
   _my->_evaluator_registry.register_evaluator< escrow_approve_evaluator                 >();
   _my->_evaluator_registry.register_evaluator< escrow_dispute_evaluator                 >();
   _my->_evaluator_registry.register_evaluator< escrow_release_evaluator                 >();
   
   // Trading Evaluators

   _my->_evaluator_registry.register_evaluator< limit_order_create_evaluator             >();
   _my->_evaluator_registry.register_evaluator< limit_order_cancel_evaluator             >();
   _my->_evaluator_registry.register_evaluator< margin_order_create_evaluator            >();
   _my->_evaluator_registry.register_evaluator< margin_order_close_evaluator             >();
   _my->_evaluator_registry.register_evaluator< call_order_update_evaluator              >();
   _my->_evaluator_registry.register_evaluator< bid_collateral_evaluator                 >();

   // Pool Evaluators
   
   _my->_evaluator_registry.register_evaluator< liquidity_pool_create_evaluator          >();
   _my->_evaluator_registry.register_evaluator< liquidity_pool_exchange_evaluator        >();
   _my->_evaluator_registry.register_evaluator< liquidity_pool_fund_evaluator            >();
   _my->_evaluator_registry.register_evaluator< liquidity_pool_withdraw_evaluator        >();
   _my->_evaluator_registry.register_evaluator< credit_pool_collateral_evaluator         >();
   _my->_evaluator_registry.register_evaluator< credit_pool_borrow_evaluator             >();
   _my->_evaluator_registry.register_evaluator< credit_pool_lend_evaluator               >();
   _my->_evaluator_registry.register_evaluator< credit_pool_withdraw_evaluator           >();

   // Asset Evaluators

   _my->_evaluator_registry.register_evaluator< asset_create_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< asset_update_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< asset_issue_evaluator                    >();
   _my->_evaluator_registry.register_evaluator< asset_reserve_evaluator                  >();
   _my->_evaluator_registry.register_evaluator< asset_claim_fees_evaluator               >();
   _my->_evaluator_registry.register_evaluator< asset_claim_pool_evaluator               >();
   _my->_evaluator_registry.register_evaluator< asset_fund_fee_pool_evaluator            >();
   _my->_evaluator_registry.register_evaluator< asset_update_issuer_evaluator            >();
   _my->_evaluator_registry.register_evaluator< asset_update_bitasset_evaluator          >();
   _my->_evaluator_registry.register_evaluator< asset_update_feed_producers_evaluator    >();
   _my->_evaluator_registry.register_evaluator< asset_publish_feed_evaluator             >();
   _my->_evaluator_registry.register_evaluator< asset_settle_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< asset_global_settle_evaluator            >();
   
   // Block Producer Evaluators

   _my->_evaluator_registry.register_evaluator< witness_update_evaluator                 >();
   _my->_evaluator_registry.register_evaluator< proof_of_work_evaluator                  >();
   _my->_evaluator_registry.register_evaluator< verify_block_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< commit_block_evaluator                  >();
   _my->_evaluator_registry.register_evaluator< producer_violation_evaluator             >();

   // Custom Evaluators

   _my->_evaluator_registry.register_evaluator< custom_evaluator                         >();
   _my->_evaluator_registry.register_evaluator< custom_json_evaluator                    >();
}


void database::set_custom_operation_interpreter( const std::string& id, std::shared_ptr< custom_operation_interpreter > registry )
{
   bool inserted = _custom_operation_interpreters.emplace( id, registry ).second;
   // This assert triggering means we're mis-configured (multiple registrations of custom JSON evaluator for same ID)
   FC_ASSERT( inserted );
}

std::shared_ptr< custom_operation_interpreter > database::get_custom_json_evaluator( const std::string& id )
{
   auto it = _custom_operation_interpreters.find( id );
   if( it != _custom_operation_interpreters.end() )
      return it->second;
   return std::shared_ptr< custom_operation_interpreter >();
}

void database::initialize_indexes()
{
   // Global Indexes

   add_core_index< dynamic_global_property_index           >(*this);
   add_core_index< transaction_index                       >(*this);
   add_core_index< operation_index                         >(*this);
   add_core_index< reward_fund_index                       >(*this);
   add_core_index< block_summary_index                     >(*this);
   add_core_index< hardfork_property_index                 >(*this);
   
   // Account Indexes

   add_core_index< account_index                           >(*this);
   add_core_index< account_business_index                  >(*this);
   add_core_index< account_executive_vote_index            >(*this);
   add_core_index< account_officer_vote_index              >(*this);
   add_core_index< account_member_request_index            >(*this);
   add_core_index< account_member_invite_index             >(*this);
   add_core_index< account_member_key_index                >(*this);
   add_core_index< account_authority_index                 >(*this);
   add_core_index< account_permission_index                >(*this);
   add_core_index< account_following_index                 >(*this);
   add_core_index< account_balance_index                   >(*this);
   add_core_index< account_history_index                   >(*this);
   add_core_index< tag_following_index                     >(*this);
   add_core_index< connection_index                        >(*this);
   add_core_index< connection_request_index                >(*this);
   add_core_index< owner_authority_history_index           >(*this);
   add_core_index< account_recovery_request_index          >(*this);
   add_core_index< change_recovery_account_request_index   >(*this);
   add_core_index< decline_voting_rights_request_index     >(*this);

   // Network Indexes

   add_core_index< network_officer_index                   >(*this);
   add_core_index< network_officer_vote_index              >(*this);
   add_core_index< executive_board_index                   >(*this);
   add_core_index< executive_board_vote_index              >(*this);
   add_core_index< governance_account_index                >(*this);
   add_core_index< governance_subscription_index           >(*this);
   add_core_index< supernode_index                         >(*this);
   add_core_index< interface_index                         >(*this);
   add_core_index< community_enterprise_index              >(*this);
   add_core_index< enterprise_approval_index               >(*this);

   // Comment Indexes

   add_core_index< comment_index                           >(*this);
   add_core_index< comment_vote_index                      >(*this);
   add_core_index< comment_view_index                      >(*this);
   add_core_index< comment_share_index                     >(*this);
   add_core_index< moderation_tag_index                    >(*this);
   add_core_index< comment_metrics_index                   >(*this);
   add_core_index< message_index                           >(*this);
   add_core_index< blog_index                              >(*this);
   add_core_index< feed_index                              >(*this);

   // Board Indexes

   add_core_index< board_index                             >(*this);
   add_core_index< board_member_index                      >(*this);
   add_core_index< board_member_key_index                  >(*this);
   add_core_index< board_moderator_vote_index              >(*this);
   add_core_index< board_join_request_index                >(*this);
   add_core_index< board_join_invite_index                 >(*this);

   // Advertising Indexes

   add_core_index< ad_creative_index                       >(*this);
   add_core_index< ad_campaign_index                       >(*this);
   add_core_index< ad_inventory_index                      >(*this);
   add_core_index< ad_audience_index                       >(*this);
   add_core_index< ad_bid_index                            >(*this);

   // Transfer Indexes

   add_core_index< transfer_request_index                  >(*this);
   add_core_index< transfer_recurring_index                >(*this);
   add_core_index< transfer_recurring_request_index        >(*this);

   // Balance Indexes

   add_core_index< unstake_asset_route_index               >(*this);
   add_core_index< savings_withdraw_index                  >(*this);
   add_core_index< asset_delegation_index                  >(*this);
   add_core_index< asset_delegation_expiration_index       >(*this);

   // Escrow Indexes

   add_core_index< escrow_index                            >(*this);

   // Trading Indexes

   add_core_index< limit_order_index                       >(*this);
   add_core_index< margin_order_index                       >(*this);
   add_core_index< call_order_index                        >(*this);
   add_core_index< force_settlement_index                  >(*this);
   add_core_index< collateral_bid_index                    >(*this);

   // Asset Indexes

   add_core_index< asset_index                             >(*this);
   add_core_index< asset_dynamic_data_index                >(*this);
   add_core_index< asset_bitasset_data_index               >(*this);
   add_core_index< asset_equity_data_index                 >(*this);
   add_core_index< asset_credit_data_index                 >(*this);
   add_core_index< asset_liquidity_pool_index              >(*this);
   add_core_index< asset_credit_pool_index                 >(*this);

   // Credit Indexes

   add_core_index< credit_collateral_index                 >(*this);
   add_core_index< credit_loan_index                       >(*this);

   // Block Producer Objects 

   add_core_index< witness_index                           >(*this);
   add_core_index< witness_schedule_index                  >(*this);
   add_core_index< witness_vote_index                      >(*this);
   add_core_index< block_validation_index                  >(*this);

   _plugin_index_signal();
}

const std::string& database::get_json_schema()const
{
   return _json_schema;
}


void database::validate_transaction( const signed_transaction& trx )
{
   database::with_write_lock( [&]()
   {
      auto session = start_undo_session( true );
      _apply_transaction( trx );
      session.undo();
   });
}

void database::set_flush_interval( uint32_t flush_blocks )
{
   _flush_blocks = flush_blocks;
   _next_flush_block = 0;
}

//////////////////// private methods ////////////////////

void database::apply_block( const signed_block& next_block, uint32_t skip )
{ try {
   //fc::time_point begin_time = fc::time_point::now();

   auto block_num = next_block.block_num();
   if( _checkpoints.size() && _checkpoints.rbegin()->second != block_id_type() )
   {
      auto itr = _checkpoints.find( block_num );
      if( itr != _checkpoints.end() )
         FC_ASSERT( next_block.id() == itr->second, "Block did not match checkpoint", ("checkpoint",*itr)("block_id",next_block.id()) );

      if( _checkpoints.rbegin()->first >= block_num )
         skip = skip_witness_signature
              | skip_transaction_signatures
              | skip_transaction_dupe_check
              | skip_fork_db
              | skip_block_size_check
              | skip_tapos_check
              | skip_authority_check
              | skip_merkle_check //While blockchain is being downloaded, txs need to be validated against block headers
              | skip_undo_history_check
              | skip_witness_schedule_check
              | skip_validate
              | skip_validate_invariants
              ;
   }

   detail::with_skip_flags( *this, skip, [&]()
   {
      _apply_block( next_block );
   } );

   /*try
   {
   /// check invariants
   if( is_producing() || !( skip & skip_validate_invariants ) )
      validate_invariants();
   }
   FC_CAPTURE_AND_RETHROW( (next_block) );*/

   //fc::time_point end_time = fc::time_point::now();
   //fc::microseconds dt = end_time - begin_time;
   if( _flush_blocks != 0 )
   {
      if( _next_flush_block == 0 )
      {
         uint32_t lep = block_num + 1 + _flush_blocks * 9 / 10;
         uint32_t rep = block_num + 1 + _flush_blocks;

         // use time_point::now() as RNG source to pick block randomly between lep and rep
         uint32_t span = rep - lep;
         uint32_t x = lep;
         if( span > 0 )
         {
            uint64_t now = uint64_t( fc::time_point::now().time_since_epoch().count() );
            x += now % span;
         }
         _next_flush_block = x;
         //ilog( "Next flush scheduled at block ${b}", ("b", x) );
      }

      if( _next_flush_block == block_num )
      {
         _next_flush_block = 0;
         //ilog( "Flushing database shared memory at block ${b}", ("b", block_num) );
         chainbase::database::flush();
      }
   }

   show_free_memory( false );

} FC_CAPTURE_AND_RETHROW( (next_block) ) }

void database::show_free_memory( bool force )
{
   uint32_t free_gb = uint32_t( get_free_memory() / (1024*1024*1024) );
   if( force || (free_gb < _last_free_gb_printed) || (free_gb > _last_free_gb_printed+1) )
   {
      ilog( "Free memory is now ${n}G", ("n", free_gb) );
      _last_free_gb_printed = free_gb;
   }

   if( free_gb == 0 )
   {
      uint32_t free_mb = uint32_t( get_free_memory() / (1024*1024) );

      if( free_mb <= 100 && head_block_num() % 10 == 0 )
         elog( "Free memory is now ${n}M. Increase shared file size immediately!" , ("n", free_mb) );
   }
}

void database::_apply_block( const signed_block& next_block )
{ try {
   notify_pre_apply_block( next_block );

   uint32_t next_block_num = next_block.block_num();
   //block_id_type next_block_id = next_block.id();

   uint32_t skip = get_node_properties().skip_flags;

   if( !( skip & skip_merkle_check ) )
   {
      auto merkle_root = next_block.calculate_merkle_root();

      try
      {
         FC_ASSERT( next_block.transaction_merkle_root == merkle_root, "Merkle check failed", ("next_block.transaction_merkle_root",next_block.transaction_merkle_root)("calc",merkle_root)("next_block",next_block)("id",next_block.id()) );
      }
      catch( fc::assert_exception& e )
      {
         const auto& merkle_map = get_shared_db_merkle();
         auto itr = merkle_map.find( next_block_num );

         if( itr == merkle_map.end() || itr->second != merkle_root )
            throw e;
      }
   }

   const witness_object& signing_witness = validate_block_header(skip, next_block);

   _current_block_num    = next_block_num;
   _current_trx_in_block = 0;
   _current_trx_stake_weight = 0;

   const auto& gprops = get_dynamic_global_properties();
   auto block_size = fc::raw::pack_size( next_block );

   FC_ASSERT( block_size <= gprops.maximum_block_size, "Block Size is too Big", ("next_block_num",next_block_num)("block_size", block_size)("max",gprops.maximum_block_size) );
   
   if( block_size < MIN_BLOCK_SIZE )
   {
      elog( "Block size is too small",
         ("next_block_num",next_block_num)("block_size", block_size)("min",MIN_BLOCK_SIZE)
      );
   }

   /// modify current witness so transaction evaluators can know who included the transaction,
   /// this is mostly for POW operations which must pay the current_producer
   modify( gprops, [&]( dynamic_global_property_object& dgp ){
      dgp.current_producer = next_block.witness;
   });

   /// parse witness version reporting
   process_header_extensions( next_block );

   const auto& witness = get_witness( next_block.witness );
   const auto& hardfork_state = get_hardfork_property_object();
   FC_ASSERT( witness.running_version >= hardfork_state.current_hardfork_version,
      "Block produced by witness that is not running current hardfork",
      ("witness",witness)("next_block.witness",next_block.witness)("hardfork_state", hardfork_state)
   );
   
   for( const auto& trx : next_block.transactions )
   {
      /* We do not need to push the undo state for each transaction
       * because they either all apply and are valid or the
       * entire block fails to apply.  We only need an "undo" state
       * for transactions when validating broadcast transactions or
       * when building a block.
       */
      apply_transaction( trx, skip );
      ++_current_trx_in_block;
   }

   update_global_dynamic_data(next_block);
   update_signing_witness(signing_witness, next_block);

   update_last_irreversible_block();
   update_transaction_stake(signing_witness, _current_trx_stake_weight);
   create_block_summary(next_block);
   clear_expired_transactions();
   clear_expired_operations();
   clear_expired_delegations();
   update_witness_schedule(*this);

   update_witness_set();
   update_governance_account_set();
   update_board_moderator_set();
   update_business_account_set();
   update_comment_metrics();
   update_median_liquidity();
   update_proof_of_work_target();
   
   process_funds();

   process_asset_staking();
   process_savings_withdraws();
   process_recurring_transfers();
   process_equity_rewards();
   process_power_rewards();
   process_credit_updates();
   process_credit_buybacks();
   process_margin_updates();
   process_credit_interest();
   process_membership_updates();
   process_txn_stake_rewards();
   process_validation_rewards();
   process_producer_activity_rewards();
   process_network_officer_rewards();
   process_executive_board_budgets();
   process_supernode_rewards();
   process_community_enterprise_fund();

   process_comment_cashout();
   
   account_recovery_processing();
   expire_escrow_ratification();
   process_decline_voting_rights();

   process_hardforks();

   // notify observers that the block has been applied
   notify_applied_block( next_block );

   notify_changed_objects();
} FC_CAPTURE_LOG_AND_RETHROW( (next_block.block_num()) ) }

void database::process_header_extensions( const signed_block& next_block )
{
   auto itr = next_block.extensions.begin();

   while( itr != next_block.extensions.end() )
   {
      switch( itr->which() )
      {
         case 0: // void_t
            break;
         case 1: // version
         {
            auto reported_version = itr->get< version >();
            const auto& signing_witness = get_witness( next_block.witness );
            //idump( (next_block.witness)(signing_witness.running_version)(reported_version) );

            if( reported_version != signing_witness.running_version )
            {
               modify( signing_witness, [&]( witness_object& wo )
               {
                  wo.running_version = reported_version;
               });
            }
            break;
         }
         case 2: // hardfork_version vote
         {
            auto hfv = itr->get< hardfork_version_vote >();
            const auto& signing_witness = get_witness( next_block.witness );
            //idump( (next_block.witness)(signing_witness.running_version)(hfv) );

            if( hfv.hf_version != signing_witness.hardfork_version_vote || hfv.hf_time != signing_witness.hardfork_time_vote )
               modify( signing_witness, [&]( witness_object& wo )
               {
                  wo.hardfork_version_vote = hfv.hf_version;
                  wo.hardfork_time_vote = hfv.hf_time;
               });

            break;
         }
         default:
            FC_ASSERT( false, "Unknown extension in block header" );
      }

      ++itr;
   }
}

void database::apply_transaction(const signed_transaction& trx, uint32_t skip)
{
   detail::with_skip_flags( *this, skip, [&]() { _apply_transaction(trx); });
   notify_on_applied_transaction( trx );
}

void database::_apply_transaction(const signed_transaction& trx)
{ try {
   _current_trx_id = trx.id();
   uint32_t skip = get_node_properties().skip_flags;

   if( !(skip&skip_validate) )   /* issue #505 explains why this skip_flag is disabled */
      trx.validate();

   auto& trx_idx = get_index<transaction_index>();
   const chain_id_type& chain_id = CHAIN_ID;
   auto trx_id = trx.id();
   // idump((trx_id)(skip&skip_transaction_dupe_check));
   FC_ASSERT( (skip & skip_transaction_dupe_check) ||
              trx_idx.indices().get<by_trx_id>().find(trx_id) == trx_idx.indices().get<by_trx_id>().end(),
              "Duplicate transaction check failed", ("trx_ix", trx_id) );

   if( !(skip & (skip_transaction_signatures | skip_authority_check) ) )
   {
      auto get_active  = [&]( const string& name ) { return authority( get< account_authority_object, by_account >( name ).active ); };
      auto get_owner   = [&]( const string& name ) { return authority( get< account_authority_object, by_account >( name ).owner );  };
      auto get_posting = [&]( const string& name ) { return authority( get< account_authority_object, by_account >( name ).posting );  };

      try
      {
         trx.verify_authority( chain_id, get_active, get_owner, get_posting, MAX_SIG_CHECK_DEPTH );
      }
      catch( protocol::tx_missing_active_auth& e )
      {
         if( get_shared_db_merkle().find( head_block_num() + 1 ) == get_shared_db_merkle().end() )
            throw e;
      }
   }

   //Skip all manner of expiration and TaPoS checking if we're on block 1; It's impossible that the transaction is
   //expired, and TaPoS makes no sense as no blocks exist.
   if( BOOST_LIKELY(head_block_num() > 0) )
   {
      if( !(skip & skip_tapos_check) )
      {
         const auto& tapos_block_summary = get< block_summary_object >( trx.ref_block_num );
         //Verify TaPoS block summary has correct ID prefix, and that this block's time is not past the expiration
         ASSERT( trx.ref_block_prefix == tapos_block_summary.block_id._hash[1], transaction_tapos_exception,
                    "", ("trx.ref_block_prefix", trx.ref_block_prefix)
                    ("tapos_block_summary",tapos_block_summary.block_id._hash[1]));
      }

      fc::time_point now = head_block_time();

      ASSERT( trx.expiration <= now + fc::seconds(MAX_TIME_UNTIL_EXPIRATION), transaction_expiration_exception,
                  "", ("trx.expiration",trx.expiration)("now",now)("max_til_exp",MAX_TIME_UNTIL_EXPIRATION));
      
      ASSERT( now < trx.expiration, transaction_expiration_exception, "", ("now",now)("trx.exp",trx.expiration) );
   }

   //Insert transaction into unique transactions database.
   if( !(skip & skip_transaction_dupe_check) )
   {
      create<transaction_object>([&](transaction_object& transaction) 
      {
         transaction.trx_id = trx_id;
         transaction.expiration = trx.expiration;
         fc::raw::pack( transaction.packed_trx, trx );
      });
   }

   notify_on_pre_apply_transaction( trx );

   //Finally process the operations
   _current_op_in_trx = 0;
   for( const auto& op : trx.operations )
   { try {
      apply_operation(op);
      ++_current_op_in_trx;
     } FC_CAPTURE_AND_RETHROW( (op) );
   }

   update_stake( trx );

   _current_trx_id = transaction_id_type();

} FC_CAPTURE_AND_RETHROW( (trx) ) }

void database::update_stake( const signed_transaction& trx)
{
   if(trx.operations.size() )
   {
      const operation& op = trx.operations[0];
      share_type voting_power = get_voting_power( operation_creator_name(op) );
      size_t size = fc::raw::pack_size(trx);
      _current_trx_stake_weight += uint128_t( voting_power.value * size );
   }
}

/**
 * Decays and increments the current witness according to the stake
 * weight of all the transactions in the block they have created.
 */
void database::update_transaction_stake(const witness_object& signing_witness, const uint128_t& transaction_stake)
{
   const witness_schedule_object& wso = get_witness_schedule();
   const time_point now = head_block_time();
   fc::microseconds decay_time = wso.txn_stake_decay_time;
   modify( signing_witness, [&]( witness_object& w ) 
   {
      w.recent_txn_stake_weight -= ( w.recent_txn_stake_weight * ( now - w.last_txn_stake_weight_update).to_seconds() ) / decay_time.to_seconds();
      w.recent_txn_stake_weight += transaction_stake;
      w.last_txn_stake_weight_update = now;
   });
}

void database::apply_operation(const operation& op)
{
   operation_notification note(op);
   notify_pre_apply_operation( note );
   _my->_evaluator_registry.get_evaluator( op ).apply( op );
   notify_post_apply_operation( note );
}

const witness_object& database::validate_block_header( uint32_t skip, const signed_block& next_block )const
{ try {
   FC_ASSERT( head_block_id() == next_block.previous, "", ("head_block_id",head_block_id())("next.prev",next_block.previous) );
   FC_ASSERT( head_block_time() < next_block.timestamp, "", ("head_block_time",head_block_time())("next",next_block.timestamp)("blocknum",next_block.block_num()) );
   const witness_object& witness = get_witness( next_block.witness );

   if( !(skip&skip_witness_signature) )
      FC_ASSERT( next_block.validate_signee( witness.signing_key ) );

   if( !(skip&skip_witness_schedule_check) )
   {
      uint32_t slot_num = get_slot_at_time( next_block.timestamp );
      FC_ASSERT( slot_num > 0 );

      string scheduled_witness = get_scheduled_witness( slot_num );

      FC_ASSERT( witness.owner == scheduled_witness, "Witness produced block at wrong time",
                 ("block witness",next_block.witness)("scheduled",scheduled_witness)("slot_num",slot_num) );
   }

   return witness;
} FC_CAPTURE_AND_RETHROW() }

void database::create_block_summary(const signed_block& next_block)
{ try {
   block_summary_id_type sid( next_block.block_num() & 0xffff );
   modify( get< block_summary_object >( sid ), [&](block_summary_object& p) 
   {
      p.block_id = next_block.id();
   });
} FC_CAPTURE_AND_RETHROW() }

void database::update_global_dynamic_data( const signed_block& b )
{ try {
   const dynamic_global_property_object& _dgp = get_dynamic_global_properties();
   uint32_t missed_blocks = 0;
   price equity_price = get_liquidity_pool(SYMBOL_COIN, SYMBOL_EQUITY).hour_median_price;
   price usd_price = get_liquidity_pool(SYMBOL_COIN, SYMBOL_USD).hour_median_price;

   if( head_block_time() != fc::time_point() )
   {
      missed_blocks = get_slot_at_time( b.timestamp );
      assert( missed_blocks != 0 );
      missed_blocks--;
      for( uint32_t i = 0; i < missed_blocks; ++i )
      {
         const auto& witness_missed = get_witness( get_scheduled_witness( i + 1 ) );
         if(  witness_missed.owner != b.witness )
         {
            modify( witness_missed, [&]( witness_object& w )
            {
               w.total_missed++;
               if( head_block_num() - w.last_confirmed_block_num  > BLOCKS_PER_DAY )
               {
                  w.signing_key = public_key_type();
                  push_virtual_operation( shutdown_witness_operation( w.owner ) );
               }
            } );
         }
      }
   }

   // dynamic global properties updating
   modify( _dgp, [&]( dynamic_global_property_object& dgp )
   {
      // This is constant time assuming 100% participation. It is O(B) otherwise (B = Num blocks between update)
      for( uint32_t i = 0; i < missed_blocks + 1; i++ )
      {
         dgp.participation_count -= dgp.recent_slots_filled.hi & 0x8000000000000000ULL ? 1 : 0;
         dgp.recent_slots_filled = ( dgp.recent_slots_filled << 1 ) + ( i == 0 ? 1 : 0 );
         dgp.participation_count += ( i == 0 ? 1 : 0 );
      }

      dgp.head_block_number = b.block_num();
      dgp.head_block_id = b.id();
      dgp.time = b.timestamp;
      dgp.current_aslot += missed_blocks+1;
      dgp.current_median_equity_price = equity_price;
      dgp.current_median_usd_price = usd_price;
   } );

   if( !(get_node_properties().skip_flags & skip_undo_history_check) )
   {
      ASSERT( _dgp.head_block_number - _dgp.last_irreversible_block_num  < MAX_UNDO_HISTORY, undo_database_exception,
                 "The database does not have enough undo history to support a blockchain with so many missed blocks. "
                 "Please add a checkpoint if you would like to continue applying blocks beyond this point.",
                 ("last_irreversible_block_num",_dgp.last_irreversible_block_num)("head", _dgp.head_block_number)
                 ("max_undo",MAX_UNDO_HISTORY) );
   }
} FC_CAPTURE_AND_RETHROW() }

void database::update_signing_witness(const witness_object& signing_witness, const signed_block& new_block)
{ try {
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();
   uint64_t new_block_aslot = dpo.current_aslot + get_slot_at_time( new_block.timestamp );

   modify( signing_witness, [&]( witness_object& _wit )
   {
      _wit.last_aslot = new_block_aslot;
      _wit.last_confirmed_block_num = new_block.block_num();
   } );
} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the last irreversible and last committed block numbers and IDs,
 * enabling nodes to add the block history to their block logs, when consensus finality 
 * is achieved by block producers.
 */
void database::update_last_irreversible_block()
{ try {
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();

   const witness_schedule_object& wso = witness_schedule_object();

   vector< const witness_object* > wit_objs;
   wit_objs.reserve( wso.num_scheduled_producers );
   for( int i = 0; i < wso.num_scheduled_producers; i++ )
      wit_objs.push_back( &get_witness( wso.current_shuffled_producers[i] ) );

   static_assert( IRREVERSIBLE_THRESHOLD > 0, "irreversible threshold must be nonzero" );

   // 1 1 1 2 2 2 2 2 2 2 -> 2     .7*10 = 7
   // 1 1 1 1 1 1 1 2 2 2 -> 1
   // 3 3 3 3 3 3 3 3 3 3 -> 3

   size_t offset = ((PERCENT_100 - IRREVERSIBLE_THRESHOLD) * wit_objs.size() / PERCENT_100);

   std::nth_element( wit_objs.begin(), wit_objs.begin() + offset, wit_objs.end(),
      []( const witness_object* a, const witness_object* b )
      {
         return a->last_confirmed_block_num < b->last_confirmed_block_num;
      } );

   uint32_t new_last_irreversible_block_num = wit_objs[offset]->last_confirmed_block_num;

   std::nth_element( wit_objs.begin(), wit_objs.begin() + offset, wit_objs.end(),
      []( const witness_object* a, const witness_object* b )
      {
         return a->last_commit_height < b->last_commit_height;
      });

   uint32_t new_last_committed_block_num = wit_objs[offset]->last_commit_height;

   if( new_last_irreversible_block_num > dpo.last_irreversible_block_num )
   {
      block_id_type irreversible_id = get_block_id_for_num( new_last_irreversible_block_num );

      modify( dpo, [&]( dynamic_global_property_object& _dpo )
      {
         _dpo.last_irreversible_block_num = new_last_irreversible_block_num;
         _dpo.last_irreversible_block_id = irreversible_id;
      });
   }

   if( new_last_committed_block_num > dpo.last_committed_block_num )
   {
      block_id_type commit_id = get_block_id_for_num( new_last_committed_block_num );

      modify( dpo, [&]( dynamic_global_property_object& _dpo )
      {
         _dpo.last_committed_block_num = new_last_committed_block_num;
         _dpo.last_committed_block_id = commit_id;
      });
   }

   // Take the highest of last committed and irreverisble blocks, and commit it to the local database.
   uint32_t commit_height = std::max( dpo.last_committed_block_num, dpo.last_irreversible_block_num );   
   
   commit( commit_height );  // Node will not reverse blocks after they have been committed or produced on by two thirds of producers.

   if( !( get_node_properties().skip_flags & skip_block_log ) )  // Output to block log based on new committed and last irreverisible block numbers.
   {
      const auto& tmp_head = _block_log.head();
      uint64_t log_head_num = 0;

      if( tmp_head )
      {
         log_head_num = tmp_head->block_num();
      }

      if( log_head_num < commit_height )
      {
         while( log_head_num < commit_height )
         {
            shared_ptr< fork_item > block = _fork_db.fetch_block_on_main_branch_by_number( log_head_num+1 );
            FC_ASSERT( block, "Current fork in the fork database does not contain the last_irreversible_block" );
            _block_log.append( block->data );
            log_head_num++;
         }

         _block_log.flush();
      }
   }

   _fork_db.set_max_size( dpo.head_block_number - commit_height + 1 );

} FC_CAPTURE_AND_RETHROW() }


asset database::calculate_issuer_fee( const asset_object& trade_asset, const asset& trade_amount )
{ try {
   FC_ASSERT( trade_asset.symbol == trade_amount.symbol );

   if( !trade_asset.charges_market_fees() )
      return asset(0, trade_asset.symbol);
   if( trade_asset.options.market_fee_percent == 0 )
      return asset(0, trade_asset.symbol);

   share_type value = (( trade_amount.amount * trade_asset.options.market_fee_percent ) / PERCENT_100  );
   asset percent_fee = asset( value, trade_asset.symbol );

   if( percent_fee.amount > trade_asset.options.max_market_fee )
      percent_fee.amount = trade_asset.options.max_market_fee;

   return percent_fee;
} FC_CAPTURE_AND_RETHROW() }

asset database::pay_issuer_fees( const asset_object& recv_asset, const asset& receives )
{ try {
   asset issuer_fees = calculate_issuer_fee( recv_asset, receives );
   FC_ASSERT( issuer_fees <= receives, "Market fee shouldn't be greater than receives");

   //Don't dirty undo state if not actually collecting any fees
   if( issuer_fees.amount > 0 )
   {
      const asset_dynamic_data_object& recv_dyn_data = get_dynamic_data(recv_asset.symbol);
      modify( recv_dyn_data, [&]( asset_dynamic_data_object& addo )
      {
         addo.adjust_accumulated_fees( issuer_fees.amount );
      });
   }

   return issuer_fees;
} FC_CAPTURE_AND_RETHROW() }

asset database::pay_issuer_fees(const account_object& seller, const asset_object& recv_asset, const asset& receives )
{ try {
   const auto issuer_fees = calculate_issuer_fee( recv_asset, receives );
   FC_ASSERT( issuer_fees <= receives, 
      "Market fee shouldn't be greater than receives");
   
   if ( issuer_fees.amount > 0 ) 
   {
      asset reward = asset(0, recv_asset.symbol);

      const auto reward_percent = recv_asset.options.market_fee_share_percent; 
      if ( reward_percent > 0 ) // calculate and pay rewards
      {
         const account_object& registrar_account = get_account( seller.registrar );
         const account_object& referrer_account = get_account( seller.referrer );
         const account_permission_object& registrar_permissions = get_account_permissions(seller.registrar);
         const account_permission_object& referrer_permissions = get_account_permissions(seller.referrer);

         const auto reward_value = ( issuer_fees.amount * reward_percent) / PERCENT_100;
         if ( reward_value > 0 && registrar_permissions.is_authorized_asset( recv_asset) )
         {
            asset reward = asset( reward_value, recv_asset.symbol);
            FC_ASSERT( reward < issuer_fees, 
               "Market reward should be less than issuer fees");
            
            auto registrar_reward = reward;
            if( seller.referrer != seller.registrar )
            {
               const auto referrer_rewards_value = ( reward.amount * seller.referrer_rewards_percentage ) / PERCENT_100;

               if ( referrer_rewards_value > 0 && referrer_permissions.is_authorized_asset( recv_asset) )
               {
                  FC_ASSERT ( referrer_rewards_value <= reward.amount.value, 
                     "Referrer reward shouldn't be greater than total reward" );
                  const asset referrer_reward = asset( referrer_rewards_value, recv_asset.symbol);
                  registrar_reward -= referrer_reward;    // cut referrer percent from reward
                  adjust_reward_balance( seller.referrer, referrer_reward );
               }
            }
            adjust_reward_balance(seller.registrar, registrar_reward);
         }
      }

      const asset_dynamic_data_object& recv_dyn_data = get_dynamic_data(recv_asset.symbol);
      modify( recv_dyn_data, [&]( asset_dynamic_data_object& obj )
      {
         obj.adjust_accumulated_fees(issuer_fees.amount - reward.amount);
      });
   }

   return issuer_fees;
} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays the network fee by burning the core asset into accumulated network revenue,
 * or by burning network credit assets or force settling USD assets if their price
 * falls below $1.00 USD.
 */
asset database::pay_network_fees( const asset& amount )
{ try {
   FC_ASSERT(amount.symbol == SYMBOL_COIN);
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   asset total_fees = amount;

   price credit_usd_price = get_liquidity_pool( SYMBOL_USD, SYMBOL_CREDIT ).hour_median_price;
   price usd_settlement_price = get_bitasset_data( SYMBOL_USD ).settlement_price;
   price usd_market_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_USD ).base_hour_median_price( usd_settlement_price.base.symbol );

   if( usd_market_price < usd_settlement_price )   // If the market price of USD is below settlement price
   {
      asset usd_purchased = liquid_exchange( total_fees, SYMBOL_USD, true, false );   // Liquid Exchange into USD, without paying fees to avoid recursive fees. 

      create< force_settlement_object >([&]( force_settlement_object& fso ) 
      {
         fso.owner = NULL_ACCOUNT;
         fso.balance = usd_purchased;    // Settle USD purchased at below settlement price, to increase total Coin burned.
         fso.settlement_date = now + fc::minutes( 10 );
      });
   }
   else if( credit_usd_price < price(asset(1,SYMBOL_USD)/asset(1,SYMBOL_CREDIT)) )   // If price of credit is below $1.00 USD
   {
      asset credit_purchased = liquid_exchange( total_fees, SYMBOL_CREDIT, true, false );   // Liquid Exchange into Credit asset, without paying fees to avoid recursive fees. 

      modify( props, [&]( dynamic_global_property_object& gpo ) 
      {
         gpo.accumulated_network_revenue += total_fees;
      });
   }
   else   // Remove Coin from Supply and increment network revenue. 
   {
      modify( props, [&]( dynamic_global_property_object& gpo ) 
      {
         gpo.accumulated_network_revenue += total_fees;
      });
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays the network fee by burning the core asset into accumulated network revenue,
 * or by burning network credit assets or force settling USD assets if their price
 * falls below $1.00 USD. Splits revenue to registrar and referrer, and governance 
 * accounts that the user subscribes to.
 */
asset database::pay_network_fees( const account_object& payer, const asset& amount )
{ try {
   FC_ASSERT(amount.symbol == SYMBOL_COIN);
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   asset total_fees = amount;

   flat_set<const account_object*> governance_subscriptions;

   const auto& gov_idx = get_index< governance_subscription_index >().indices().get< by_account_governance >();
   auto gov_itr = gov_idx.lower_bound( payer.name );

   while( gov_itr != gov_idx.end() && gov_itr->account == payer.name )
   {
      const governance_subscription_object& sub = *gov_itr;
      const account_object* account_ptr = find_account( sub.governance_account );
      governance_subscriptions.insert( account_ptr );
      ++gov_itr;
   }
   const account_object& registrar = get_account( payer.registrar );
   const account_object& referrer = get_account( payer.referrer );

   asset gov_share = ( amount * GOVERNANCE_SHARE_PERCENT ) / PERCENT_100;
   asset registrar_share = ( amount * REFERRAL_SHARE_PERCENT ) / PERCENT_100;
   asset referrer_share = ( registrar_share * payer.referrer_rewards_percentage ) / PERCENT_100;
   registrar_share -= referrer_share;

   asset gov_paid = pay_multi_fee_share( governance_subscriptions, gov_share );
   asset registrar_paid = pay_fee_share( registrar, registrar_share );
   asset referrer_paid = pay_fee_share( referrer, referrer_share );

   total_fees -= ( gov_paid + registrar_paid + referrer_paid );

   price credit_usd_price = get_liquidity_pool( SYMBOL_USD, SYMBOL_CREDIT ).hour_median_price;
   price usd_settlement_price = get_bitasset_data( SYMBOL_USD ).settlement_price;
   price usd_market_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_USD ).base_hour_median_price( usd_settlement_price.base.symbol );

   if( usd_market_price < usd_settlement_price )   // If the market price of USD is below settlement price
   {
      asset usd_purchased = liquid_exchange( total_fees, SYMBOL_USD, true, false );   // Liquid Exchange into USD, without paying fees to avoid recursive fees. 

      create< force_settlement_object >([&]( force_settlement_object& fso ) 
      {
         fso.owner = NULL_ACCOUNT;
         fso.balance = usd_purchased;    // Settle USD purchased at below settlement price, to increase total Coin burned.
         fso.settlement_date = now + fc::minutes( 10 );
      });
   }
   else if( credit_usd_price < price(asset(1,SYMBOL_USD)/asset(1,SYMBOL_CREDIT)) )   // If price of credit is below $1.00 USD
   {
      asset credit_purchased = liquid_exchange( total_fees, SYMBOL_CREDIT, true, false );   // Liquid Exchange into Credit asset, without paying fees to avoid recursive fees. 

      modify( props, [&]( dynamic_global_property_object& gpo ) 
      {
         gpo.accumulated_network_revenue += total_fees;
      });
   }
   else   // Remove Coin from Supply and increment network revenue. 
   {
      modify( props, [&]( dynamic_global_property_object& gpo ) 
      {
         gpo.accumulated_network_revenue += total_fees;
      });
   }
} FC_CAPTURE_AND_RETHROW() }


/** 
 * Pays protocol trading fees on taker orders.
 * taker: The account that is the taker on the trade
 * receives: The asset object being received from the trade
 * maker_int: The owner account of the interface of the maker of the trade
 * taker_int: The owner account of the interface of the taker of the trade
 */
asset database::pay_trading_fees( const account_object& taker, const asset& receives, const account_name_type& maker_int, const account_name_type& taker_int ) 
{ try {
   asset total_fees = ( receives * TRADING_FEE_PERCENT ) / PERCENT_100;
   const account_object& m_interface = get_account(maker_int);
   const account_object& t_interface = get_account(taker_int);
   
   asset maker_interface_share = ( total_fees * MAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset taker_interface_share = ( total_fees * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset network_fee = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;

   asset maker_paid = pay_fee_share( m_interface, maker_interface_share );
   asset taker_paid = pay_fee_share( t_interface, taker_interface_share );
   asset network_paid = pay_network_fees( taker, network_fee );

   asset total_paid = network_paid + maker_paid + taker_paid;
   return total_paid;
} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays an advertising delivery operation to the provider
 * and pays a fee split to the demand side interface, the delivery provider
 * the bidder account, the audience members, and the network.
 */
asset database::pay_advertising_delivery( const account_object& provider, const account_object& demand, 
   const account_object& bidder, const account_object& delivery, flat_set< const account_object* > audience, const asset& value )
{ try {
   asset total_fees = ( value * ADVERTISING_FEE_PERCENT ) / PERCENT_100;
   
   asset demand_share     = ( total_fees * DEMAND_ADVERTISING_FEE_PERCENT ) / PERCENT_100;
   asset audience_share   = ( total_fees * AUDIENCE_ADVERTISING_FEE_PERCENT ) / PERCENT_100;
   asset bidder_share     = ( total_fees * BIDDER_ADVERTISING_FEE_PERCENT ) / PERCENT_100;
   asset delivery_share   = ( total_fees * DELIVERY_ADVERTISING_FEE_PERCENT ) / PERCENT_100;
   asset network_fee      = ( total_fees * NETWORK_ADVERTISING_FEE_PERCENT ) / PERCENT_100;

   asset demand_paid = pay_fee_share( demand, demand_share );
   asset audience_paid = pay_multi_fee_share( audience, audience_share );
   asset bidder_paid = pay_fee_share( bidder, bidder_share );
   asset delivery_paid = pay_fee_share( delivery, delivery_share );
   asset network_paid = pay_network_fees( provider, network_fee );

   asset fees_paid = network_paid + demand_paid + audience_paid + bidder_paid + delivery_paid;

   adjust_liquid_balance( provider.name, value - fees_paid );

   return value;
} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays the fees to a network contibutor, and splits fees to the account's governance 
 * account subscriptions, and registrar and referrer.
 */
asset database::pay_fee_share( const account_object& payee, const asset& amount )
{ try {
   asset total_fees = amount;

   flat_set<const account_object*> governance_subscriptions;

   const auto& gov_idx = get_index< governance_subscription_index >().indices().get< by_account_governance >();
   auto gov_itr = gov_idx.lower_bound( payee.name );

   while( gov_itr != gov_idx.end() && gov_itr->account == payee.name )
   {
      const governance_subscription_object& sub = *gov_itr;
      const account_object* account_ptr = find_account( sub.governance_account );
      governance_subscriptions.insert( account_ptr );
      ++gov_itr;
   }
   const account_object& registrar = get_account( payee.registrar );
   const account_object& referrer = get_account( payee.referrer );

   asset gov_share = ( amount * GOVERNANCE_SHARE_PERCENT ) / PERCENT_100;
   asset registrar_share = ( amount * REFERRAL_SHARE_PERCENT ) / PERCENT_100;
   asset referrer_share = ( registrar_share * payee.referrer_rewards_percentage ) / PERCENT_100;
   registrar_share -= referrer_share;

   asset gov_paid = pay_multi_fee_share( governance_subscriptions, gov_share );
   asset registrar_paid = pay_fee_share( registrar, registrar_share );
   asset referrer_paid = pay_fee_share( referrer, referrer_share );

   asset distribution = total_fees - ( gov_paid + registrar_paid + referrer_paid );

   adjust_reward_balance( payee.name, distribution );

   return total_fees;

} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays fees to a set of network contibutors, and splits fees to the account's governance 
 * account subscriptions, and registrar and referrer.
 */
asset database::pay_multi_fee_share( flat_set< const account_object* > payees, const asset& amount )
{ try {
   asset total_paid = asset( 0, amount.symbol );
   asset fee_split = amount / payees.size();

   for( auto payee : payees )
   {
      total_paid += pay_fee_share( *payee, fee_split ); 
   }
   return total_paid;

} FC_CAPTURE_AND_RETHROW() }




void database::init_hardforks()
{
   _hardfork_times[ 0 ] = fc::time_point( GENESIS_TIME );
   _hardfork_versions[ 0 ] = hardfork_version( 0, 0 );
   // FC_ASSERT( HARDFORK_0_1 == 1, "Invalid hardfork configuration" );
   // _hardfork_times[ HARDFORK_0_1 ] = fc::time_point( HARDFORK_0_1_TIME );
   // _hardfork_versions[ HARDFORK_0_1 ] = HARDFORK_0_1_VERSION;

   const auto& hardforks = get_hardfork_property_object();
   FC_ASSERT( hardforks.last_hardfork <= NUM_HARDFORKS, "Chain knows of more hardforks than configuration", ("hardforks.last_hardfork",hardforks.last_hardfork)("NUM_HARDFORKS",NUM_HARDFORKS) );
   FC_ASSERT( _hardfork_versions[ hardforks.last_hardfork ] <= BLOCKCHAIN_VERSION, "Blockchain version is older than last applied hardfork" );
   FC_ASSERT( BLOCKCHAIN_HARDFORK_VERSION == _hardfork_versions[ NUM_HARDFORKS ] );
}

void database::clear_expired_operations()
{ try {
         //Cancel expired limit orders
         auto head_time = head_block_time();
         auto maint_time = get_dynamic_global_properties().next_maintenance_time;

         auto& limit_index = get_index<limit_order_index>().indices().get<by_expiration>();
         while( !limit_index.empty() && limit_index.begin()->expiration <= head_time )
         {
            const limit_order_object& order = *limit_index.begin();
            auto base_asset = order.sell_price.base.symbol;
            auto quote_asset = order.sell_price.quote.symbol;
            cancel_limit_order( order );
         }

   //Process expired force settlement orders
   auto& settlement_index = get_index<force_settlement_index>().indices().get<by_expiration>();
   if( !settlement_index.empty() )
   {
      asset_symbol_type current_asset = settlement_index.begin()->settlement_asset_symbol();
      asset max_settlement_volume;
      price settlement_fill_price;
      price settlement_price;
      bool current_asset_finished = false;
      bool extra_dump = false;

      auto next_asset = [&current_asset, &current_asset_finished, &settlement_index, &extra_dump] {
         auto bound = settlement_index.upper_bound(current_asset);
         if( bound == settlement_index.end() )
         {
            if( extra_dump )
            {
               ilog( "next_asset() returning false" );
            }
            return false;
         }
         if( extra_dump )
         {
            ilog( "next_asset returning true, bound is ${b}", ("b", *bound) );
         }
         current_asset = bound->settlement_asset_symbol();
         current_asset_finished = false;
         return true;
      };

      uint32_t count = 0;

      // At each iteration, we either consume the current order and remove it, or we move to the next asset
      for( auto itr = settlement_index.lower_bound(current_asset);
           itr != settlement_index.end();
           itr = settlement_index.lower_bound(current_asset) )
      {
         ++count;
         const force_settlement_object& order = *itr;
         auto order_id = order.id;
         current_asset = order.settlement_asset_symbol();
         const asset_object& mia_object = get_asset(current_asset);
         const asset_bitasset_data_object& mia_bitasset = get_bitasset_data(mia_object.symbol);

         extra_dump = ((count >= 1000) && (count <= 1020));

         if( extra_dump )
         {
            wlog( "clear_expired_operations() dumping extra data for iteration ${c}", ("c", count) );
            ilog( "head_block_num is ${hb} current_asset is ${a}", ("hb", head_block_num())("a", current_asset) );
         }

         if( mia_bitasset.has_settlement() )
         {
            ilog( "Canceling a force settlement because of black swan" );
            cancel_settle_order( order, true );
            continue;
         }

         // Has this order not reached its settlement date?
         if( order.settlement_date > head_time )
         {
            if( next_asset() )
            {
               if( extra_dump )
               {
                  ilog( "next_asset() returned true when order.settlement_date > head_block_time()" );
               }
               continue;
            }
            break;
         }
         // Can we still settle in this asset?
         if( mia_bitasset.current_feed.settlement_price.is_null() )
         {
            ilog("Canceling a force settlement in ${asset} because settlement price is null",
                 ("asset", mia_object.symbol));
            cancel_settle_order(order, true);
            continue;
         }
         
         if( max_settlement_volume.symbol != current_asset ) // only calculate once per asset
         {  
            const asset_dynamic_data_object& dyn_data = get_dynamic_data( mia_object.symbol );
            max_settlement_volume = asset( mia_bitasset.max_force_settlement_volume(dyn_data.total_supply), mia_object.symbol );
         }

         if( mia_bitasset.force_settled_volume >= max_settlement_volume.amount || current_asset_finished )
         {
            if( next_asset() )
            {
               if( extra_dump )
               {
                  ilog( "next_asset() returned true when mia.force_settled_volume >= max_settlement_volume.amount" );
               }
               continue;
            }
            break;
         }

         if( settlement_fill_price.base.symbol != current_asset )  // only calculate once per asset
         {
            bitasset_options options = *mia_bitasset.options;
            uint16_t offset = options.force_settlement_offset_percent;
            settlement_fill_price = mia_bitasset.current_feed.settlement_price / ratio_type( PERCENT_100 - offset, PERCENT_100 );
         }
            
         if( settlement_price.base.symbol != current_asset )  // only calculate once per asset
         {
            settlement_price = settlement_fill_price;
         }

         auto& call_index = get_index<call_order_index>().indices().get<by_collateral>();
         asset settled = asset( mia_bitasset.force_settled_volume , mia_object.symbol);
         // Match against the least collateralized short until the settlement is finished or we reach max settlements
         while( settled < max_settlement_volume && find(order_id) )
         {
            auto itr = call_index.lower_bound(boost::make_tuple( price::min( mia_bitasset.backing_asset, mia_object.symbol )));
            // There should always be a call order, since asset exists!
            FC_ASSERT(itr != call_index.end() && itr->debt_type() == mia_object.symbol);
            asset max_settlement = max_settlement_volume - settled;

            if( order.balance.amount == 0 )
            {
               wlog( "0 settlement detected" );
               cancel_settle_order( order, true );
               break;
            }
            asset new_settled = match(*itr, order, settlement_price, max_settlement, settlement_fill_price);
            if( new_settled.amount == 0 ) // unable to fill this settle order
            {
               if( find( order_id ) ) // the settle order hasn't been cancelled
                  current_asset_finished = true;
               break;
            }
            settled += new_settled;
         }
         if( mia_bitasset.force_settled_volume != settled.amount )
         {
            modify(mia_bitasset, [settled](asset_bitasset_data_object& b) 
            {
               b.force_settled_volume = settled.amount;
            });
         }
      }
   }
} FC_CAPTURE_AND_RETHROW() }

void database::process_hardforks()
{
   try
   {
      // If there are upcoming hardforks and the next one is later, do nothing
      const auto& hardforks = get_hardfork_property_object();

      while( _hardfork_versions[ hardforks.last_hardfork ] < hardforks.next_hardfork
         && hardforks.next_hardfork_time <= head_block_time() )
      {
         if( hardforks.last_hardfork < NUM_HARDFORKS ) {
            apply_hardfork( hardforks.last_hardfork + 1 );
         }
         else
            throw unknown_hardfork_exception();
      }
   }
   FC_CAPTURE_AND_RETHROW()
}

bool database::has_hardfork( uint32_t hardfork )const
{
   return get_hardfork_property_object().processed_hardforks.size() > hardfork;
}

void database::set_hardfork( uint32_t hardfork, bool apply_now )
{
   auto const& hardforks = get_hardfork_property_object();

   for( uint32_t i = hardforks.last_hardfork + 1; i <= hardfork && i <= NUM_HARDFORKS; i++ )
   {
      modify( hardforks, [&]( hardfork_property_object& hpo )
      {
         hpo.next_hardfork = _hardfork_versions[i];
         hpo.next_hardfork_time = head_block_time();
      } );

      if( apply_now )
         apply_hardfork( i );
   }
}

void database::apply_hardfork( uint32_t hardfork )
{
   if( _log_hardforks )
      elog( "HARDFORK ${hf} at block ${b}", ("hf", hardfork)("b", head_block_num()) );

   switch( hardfork )
   {
      case HARDFORK_0_1:
         break;
      case HARDFORK_0_2:
         break;
      case HARDFORK_0_3:
         break;
   }

   modify( get_hardfork_property_object(), [&]( hardfork_property_object& hfp )
   {
      FC_ASSERT( hardfork == hfp.last_hardfork + 1, "Hardfork being applied out of order", ("hardfork",hardfork)("hfp.last_hardfork",hfp.last_hardfork) );
      FC_ASSERT( hfp.processed_hardforks.size() == hardfork, "Hardfork being applied out of order" );
      hfp.processed_hardforks.push_back( _hardfork_times[ hardfork ] );
      hfp.last_hardfork = hardfork;
      hfp.current_hardfork_version = _hardfork_versions[ hardfork ];
      FC_ASSERT( hfp.processed_hardforks[ hfp.last_hardfork ] == _hardfork_times[ hfp.last_hardfork ], "Hardfork processing failed sanity check..." );
   } );

   push_virtual_operation( hardfork_operation( hardfork ), true );
}

/**
 * Verifies all supply invariants
 */
void database::validate_invariants()const
{ try {
   const dynamic_global_property_object& gprops = get_dynamic_global_properties();
   const auto& asset_idx = get_index<asset_dynamic_data_index>().indices().get<by_id>();
   vector< asset > asset_checksum;
   const asset_dynamic_data_object& final_asset = *asset_idx.end();
   auto asset_count = final_asset.id;
   asset_checksum.reserve(gprops.asset_count);

   for( auto itr = asset_idx.begin(); itr != asset_idx.end(); ++itr ) // ASSERT ALL ASSET OBJECTS HAVE CORRECT SUPPLY ACCOUNTING;
   {
      asset total_supply_checksum = asset(0, itr-> symbol);

      total_supply_checksum += itr->get_liquid_supply();
      total_supply_checksum += itr->get_staked_supply();
      total_supply_checksum += itr->get_savings_supply();
      total_supply_checksum += itr->get_reward_supply();
      total_supply_checksum += itr->get_pending_supply();

      FC_ASSERT(itr->get_delegated_supply() == itr->get_receiving_supply(), "Asset Supply error: Delegated supply not equal to receiving supply", ("Asset", itr->symbol));
      FC_ASSERT(total_supply_checksum == itr->get_total_supply(), "Asset Supply error: Supply values Sum not equal to total supply", ("Asset", itr->symbol));
      asset_checksum.push_back(itr->get_total_supply()); // Build a vector to record all asset total supply values for verification
   }

   const auto& account_balance_idx = get_index<account_balance_index>().indices().get<by_name>();

   for( auto itr = account_balance_idx.begin(); itr != account_balance_idx.end(); ++itr ) // ASSERT ALL ACCOUNT BALANCE OBJECTS HAVE CORRECT SUPPLY ACCOUNTING;
   {
      asset total_balance_checksum = asset(0, itr-> symbol);
      uint64_t balance_asset_id = get_asset(itr->symbol).id._id;

      total_balance_checksum += itr->get_liquid_balance();
      total_balance_checksum += itr->get_staked_balance();
      total_balance_checksum += itr->get_savings_balance();
      total_balance_checksum += itr->get_reward_balance();

      FC_ASSERT(total_balance_checksum == itr->get_total_balance(), "Account Balance error: Balance value sum not equal to total balance", ("Asset", itr->symbol)("Account", itr->owner.name));
      asset_checksum[balance_asset_id] -= itr->get_total_balance(); //decrement the asset checksum by the total of all account balance objects
   }

   const auto& witness_idx = get_index< witness_index >().indices();

   for( auto itr = witness_idx.begin(); itr != witness_idx.end(); ++itr )
   {
      FC_ASSERT( itr->voting_power <= gprops.total_voting_power, "", ("itr",*itr) );  // verify no witness has too many votes
   }

   const auto& limit_order_idx = get_index< limit_order_index >().indices();

   for( auto itr = limit_order_idx.begin(); itr != limit_order_idx.end(); ++itr )
   {
      uint64_t order_asset_id = get_asset(itr->sell_price.base.symbol).id._id;
      asset_checksum[order_asset_id] -= itr->sell_price.base; // Decrement the asset checksum by the total of all pending limit orders
   }

   const auto& call_order_idx = get_index< call_order_index >().indices();

   for( auto itr = call_order_idx.begin(); itr != call_order_idx.end(); ++itr )
   {
      uint64_t order_asset_id = get_asset(itr->collateral.symbol).id._id;
      asset_checksum[order_asset_id] -= itr->collateral; // Decrement the asset checksum by the total of all outstanding call orders
   }

   const auto& escrow_idx = get_index< escrow_index >().indices().get< by_id >();

   for( auto itr = escrow_idx.begin(); itr != escrow_idx.end(); ++itr )
   {
      uint64_t escrow_asset_id = get_asset(itr->balance.symbol).id._id;
      uint64_t pending_fee_id = get_asset(itr->pending_fee.symbol).id._id;
      asset_checksum[escrow_asset_id] -= itr->balance; // Decrement the asset checksum by the total of all pending escrow transfer balances
      asset_checksum[pending_fee_id] -= itr->pending_fee; // Decrement the asset checksum by the total of all pending escrow fees
   }

   const auto& savings_withdraw_idx = get_index< savings_withdraw_index >().indices().get< by_id >();

   for( auto itr = savings_withdraw_idx.begin(); itr != savings_withdraw_idx.end(); ++itr )
   {
      uint64_t savings_asset_id = get_asset(itr->amount.symbol).id._id;
      asset_checksum[savings_asset_id] -= itr->amount;
   }
   fc::uint128_t total_reward_curve;

   const auto& comment_idx = get_index< comment_index >().indices();

   for( auto itr = comment_idx.begin(); itr != comment_idx.end(); ++itr )
   {
      if( itr->net_reward > 0 )
      {
         auto delta = util::evaluate_reward_curve( itr->net_reward );
         total_reward_curve += delta;
      }
   }

   const reward_fund_object& reward_fund = get_reward_fund();

   asset total_reward_balance = asset(0, SYMBOL_COIN);

   total_reward_balance += reward_fund.content_reward_balance;
   total_reward_balance += reward_fund.validation_reward_balance;
   total_reward_balance += reward_fund.txn_stake_reward_balance;
   total_reward_balance += reward_fund.work_reward_balance;
   total_reward_balance += reward_fund.producer_activity_reward_balance;
   total_reward_balance += reward_fund.supernode_reward_balance;
   total_reward_balance += reward_fund.power_reward_balance;
   total_reward_balance += reward_fund.community_fund_balance;
   total_reward_balance += reward_fund.development_reward_balance;
   total_reward_balance += reward_fund.marketing_reward_balance;
   total_reward_balance += reward_fund.advocacy_reward_balance;
   total_reward_balance += reward_fund.activity_reward_balance;

   FC_ASSERT(total_reward_balance == reward_fund.total_pending_reward_balance, 
      "Reward Fund Error: Balance sums not equal to total balance value");

   asset_checksum[0] -= total_reward_balance;

   for( auto itr = asset_idx.begin(); itr != asset_idx.end(); ++itr ) // ASSERT ALL ASSET CHECKSUMS ARE ZERO;
   {
      FC_ASSERT(asset_checksum[itr->id._id].amount == 0, 
         "Asset Supply Invariant Error: Supply checksum not equal to zero.", ("Asset", itr->symbol));
   }
} FC_CAPTURE_LOG_AND_RETHROW( (head_block_num()) ) }


} } //node::chain