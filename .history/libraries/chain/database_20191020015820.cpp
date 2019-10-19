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

struct reward_fund_context
{
   uint128_t   recent_content_claims = 0;
   asset       content_reward_balance = asset( 0, SYMBOL_COIN );
   share_type  reward_distributed = 0;
};

uint8_t find_msb( const uint128_t& u )
{
   uint64_t x;
   uint8_t places;
   x      = (u.lo ? u.lo : 1);
   places = (u.hi ?   64 : 0);
   x      = (u.hi ? u.hi : x);
   return uint8_t( boost::multiprecision::detail::find_msb(x) + places );
}

uint64_t approx_sqrt( const uint128_t& x )
{
   if( (x.lo == 0) && (x.hi == 0) )
      return 0;

   uint8_t msb_x = find_msb(x);
   uint8_t msb_z = msb_x >> 1;

   uint128_t msb_x_bit = uint128_t(1) << msb_x;
   uint64_t  msb_z_bit = uint64_t (1) << msb_z;

   uint128_t mantissa_mask = msb_x_bit - 1;
   uint128_t mantissa_x = x & mantissa_mask;
   uint64_t mantissa_z_hi = (msb_x & 1) ? msb_z_bit : 0;
   uint64_t mantissa_z_lo = (mantissa_x >> (msb_x - msb_z)).lo;
   uint64_t mantissa_z = (mantissa_z_hi | mantissa_z_lo) >> 1;
   uint64_t result = msb_z_bit | mantissa_z;

   return result;
}

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
{
   try
   {
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
   }
   FC_CAPTURE_LOG_AND_RETHROW( (data_dir)(shared_mem_dir)(shared_file_size) )
}

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
      rfo.equity_reward_balance = asset(0, SYMBOL_COIN);
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
{
   try
   {
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
   }
   FC_CAPTURE_AND_RETHROW( (data_dir)(shared_mem_dir) )

}

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
{
   try
   {
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
   }
   FC_CAPTURE_AND_RETHROW( (block_num) )
}

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

uint32_t database::pow_difficulty()const
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

const feed_history_object& database::get_feed_history()const
{ try {
   return get< feed_history_object >();
} FC_CAPTURE_AND_RETHROW() }

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

/**
 * Pays an account its activity reward shares from the reward pool.
 */
asset database::claim_activity_reward( const account_object& account, const witness_object& witness)
{ try {
   const account_balance_object& abo = get_account_balance(account.name, SYMBOL_EQUITY);
   auto decay_rate = RECENT_REWARD_DECAY_RATE;
   const reward_fund_object& reward_fund = get_reward_fund();
   const asset_dynamic_data_object& core_asset_dynamic_data = get_core_dynamic_data();
   share_type activity_shares = BLOCKCHAIN_PRECISION;
   if( abo.staked_balance >= 10 * BLOCKCHAIN_PRECISION) 
   {
      activity_shares *= 2;
   }

   if( account.membership == STANDARD_MEMBERSHIP) 
   {
      activity_shares = (activity_shares * ACTIVITY_BOOST_STANDARD_PERCENT) / PERCENT_100;
   }
   else if( account.membership == MID_MEMBERSHIP) 
   {
      activity_shares = (activity_shares * ACTIVITY_BOOST_MID_PERCENT) / PERCENT_100;
   }
   else if( account.membership == TOP_MEMBERSHIP) 
   {
      activity_shares = (activity_shares * ACTIVITY_BOOST_TOP_PERCENT) / PERCENT_100;
   }

   // Decay recent claims of activity reward fund and add new shares of this claim.
   modify( reward_fund, [&]( reward_fund_object& rfo )   
   {
      rfo.recent_activity_claims -= ( rfo.recent_activity_claims * ( head_block_time() - rfo.last_update ).to_seconds() ) / decay_rate.to_seconds();
      rfo.last_update = head_block_time();
      rfo.recent_activity_claims += activity_shares.value;
   }); 

   asset activity_reward = ( reward_fund.activity_reward_balance * activity_shares ) / reward_fund.recent_activity_claims;

   modify( reward_fund, [&]( reward_fund_object& rfo )
   {
      rfo.adjust_activity_reward_balance( -activity_reward );    // Deduct activity reward from reward fund.
   });

   adjust_pending_supply( -activity_reward );                    // Deduct activity reward from pending supply.
   
   // Update recent activity claims on account
   modify( account, [&]( account_object& a ) 
   {
      a.recent_activity_claims -= ( a.recent_activity_claims * ( head_block_time() - a.last_activity_reward ).to_seconds() ) / decay_rate.to_seconds();
      a.last_activity_reward = head_block_time();                // Update activity reward time.
      a.recent_activity_claims += BLOCKCHAIN_PRECISION;          // Increments rolling activity average by one claim.
   });

   adjust_reward_balance( account, activity_reward );            // Add activity reward to reward balance of claiming account. 

   uint128_t voting_power = get_voting_power( account.name ).value;

   modify( witness, [&]( witness_object& w ) 
   {
      w.accumulated_activity_stake += voting_power;
   });
auto& mem_inv_idx = get_index<account_member_invite_index>().indices().get<by_expiration>();
   while( !mem_inv_idx.empty() && mem_inv_idx.begin()->expiration <= head_time )
   {
      const account_member_invite_object& invite = *mem_inv_idx.begin();
      remove( invite );   // Remove expired account member invites
   }