#include <node/protocol/types.hpp>
#include <node/protocol/authority.hpp>
#include <node/protocol/transaction.hpp>
#include <node/chain/database.hpp>

#include <node/chain/node_object_types.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/custom_operation_interpreter.hpp>

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
#include <node/chain/producer_schedule.hpp>

#include <node/chain/util/asset.hpp>
#include <node/chain/util/reward.hpp>
#include <node/chain/util/uint256.hpp>
#include <node/chain/util/reward.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/uint128.hpp>

#include <fc/container/deque.hpp>

#include <fc/io/fstream.hpp>
#include <fc/io/json.hpp>

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
using node::protocol::share_type;

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
   chainbase::database::open( shared_mem_dir, chainbase_flags, shared_file_size );
   initialize_indexes();
   initialize_evaluators();

   if( chainbase_flags & chainbase::database::read_write )
   {
      if( !find< dynamic_global_property_object >() )
      {
         with_write_lock( [&]()
         {
            init_genesis();
         });
      }
         
      _block_log.open( data_dir / "block_log" );

      auto log_head = _block_log.head();

      // Rewind all undo state. This should return us to the state at the last irreversible block.
      with_write_lock( [&]()
      {
         undo_all();
         FC_ASSERT( revision() == int64_t( head_block_num() ), 
            "Chainbase revision does not match head block num",
            ("rev", revision())("head_block", head_block_num()) );
         validate_invariants();
      });

      if( head_block_num() )
      {
         auto head_block = _block_log.read_block_by_num( head_block_num() );
         // This assertion should be caught and a reindex should occur
         FC_ASSERT( head_block.valid() && head_block->id() == head_block_id(), 
            "Chain state does not match block log. Please reindex blockchain." );

         _fork_db.start_block( *head_block );
      }
   }

   with_read_lock( [&]()
   {
      init_hardforks(); // Writes to local state, but reads from db
   });
} FC_CAPTURE_LOG_AND_RETHROW( (data_dir)(shared_mem_dir)(shared_file_size) ) }


/**
 * Creates a new blockchain from the genesis block.
 *  
 * Generates initial assets, accounts, producers, communities, network objects,
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
   
   // Create the Global Dynamic Properties Object to track consensus critical network and chain information

   time_point now = GENESIS_TIME;

   ilog( "\n" );
   ilog( "======================================================" );
   ilog( "========== INIT GENESIS: STARTING NEW CHAIN ==========" );
   ilog( "====================================================== \n" );

   create< dynamic_global_property_object >( [&]( dynamic_global_property_object& dgpo )
   {
      dgpo.current_producer = GENESIS_ACCOUNT_BASE_NAME;
      dgpo.time = now;
      dgpo.recent_slots_filled = fc::uint128::max_value();
      dgpo.participation_count = 128;
   });

   create< median_chain_property_object >( [&]( median_chain_property_object& p ){});

   create< comment_metrics_object >( [&]( comment_metrics_object& o ) {});

   for( int i = 0; i < 0x10000; i++ )
   {
      create< block_summary_object >( [&]( block_summary_object& ) {});
   }

   create< hardfork_property_object >( [&]( hardfork_property_object& hpo )
   {
      hpo.processed_hardforks.push_back( now );
   });

   // Create the initial Reward fund object to contain the balances of the network reward funds and parameters

   create< asset_reward_fund_object >( [&]( asset_reward_fund_object& rfo )
   {
      rfo.symbol = SYMBOL_COIN;
      rfo.content_reward_balance = asset( 0, SYMBOL_COIN );
      rfo.validation_reward_balance = asset( 0, SYMBOL_COIN );
      rfo.txn_stake_reward_balance = asset( 0, SYMBOL_COIN );
      rfo.work_reward_balance = asset( 0, SYMBOL_COIN );
      rfo.producer_activity_reward_balance = asset( 0, SYMBOL_COIN );
      rfo.supernode_reward_balance = asset( 0, SYMBOL_COIN );
      rfo.power_reward_balance = asset( 0, SYMBOL_COIN );
      rfo.enterprise_fund_balance = asset( 0, SYMBOL_COIN );
      rfo.development_reward_balance = asset( 0, SYMBOL_COIN );
      rfo.marketing_reward_balance = asset( 0, SYMBOL_COIN );
      rfo.advocacy_reward_balance = asset( 0, SYMBOL_COIN );
      rfo.activity_reward_balance = asset( 0, SYMBOL_COIN );
      rfo.premium_partners_fund_balance = asset( 0, SYMBOL_COIN );
      rfo.recent_content_claims = 0;
      rfo.recent_activity_claims = 0;
      rfo.last_updated = now;
   });

   // Create initial blockchain accounts

   create< account_object >( [&]( account_object& a )
   {
      a.name = INIT_ACCOUNT;
      a.registrar = INIT_ACCOUNT;
      a.referrer = INIT_ACCOUNT;
      a.secure_public_key = get_public_key( a.name, SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.connection_public_key = get_public_key( a.name, CONNECTION_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.friend_public_key = get_public_key( a.name, FRIEND_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.companion_public_key = get_public_key( a.name, COMPANION_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.created = now;
      a.last_updated = now;
      a.last_vote_time = now;
      a.last_view_time = now;
      a.last_share_time = now;
      a.last_post = now;
      a.last_root_post = now;
      a.last_transfer_time = now;
      a.last_activity_reward = now;
      a.last_account_recovery = now;
      a.last_community_created = now;
      a.last_asset_created = now;
      from_string( a.json, "" );
      from_string( a.json_private, "" );
      from_string( a.details, INIT_DETAILS );
      from_string( a.url, INIT_URL );
      from_string( a.profile_image, INIT_IMAGE );
      a.membership = membership_tier_type::TOP_MEMBERSHIP;
      a.membership_expiration = time_point::maximum();
      a.mined = true;
      a.active = true;
      a.can_vote = true;
      a.revenue_share = true;
   });

   create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = INIT_ACCOUNT;
      auth.owner_auth.add_authority( get_public_key( INIT_ACCOUNT, OWNER_KEY_STR, INIT_ACCOUNT_PASSWORD ), 1 );
      auth.owner_auth.weight_threshold = 1;
      auth.active_auth.add_authority( get_public_key( INIT_ACCOUNT, ACTIVE_KEY_STR, INIT_ACCOUNT_PASSWORD ), 1 );
      auth.active_auth.weight_threshold = 1;
      auth.posting_auth.add_authority( get_public_key( INIT_ACCOUNT, POSTING_KEY_STR, INIT_ACCOUNT_PASSWORD ), 1 );
      auth.posting_auth.weight_threshold = 1;
   });

   create< account_permission_object >( [&]( account_permission_object& aao )
   {
      aao.account = INIT_ACCOUNT;
   });

   create< account_following_object >( [&]( account_following_object& afo )
   {
      afo.account = INIT_ACCOUNT;
      afo.last_updated = now;
   });

   create< account_object >( [&]( account_object& a )
   {
      a.name = INIT_CEO;
      a.registrar = INIT_ACCOUNT;
      a.referrer = INIT_ACCOUNT;
      a.secure_public_key = get_public_key( a.name, SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.connection_public_key = get_public_key( a.name, CONNECTION_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.friend_public_key = get_public_key( a.name, FRIEND_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.companion_public_key = get_public_key( a.name, COMPANION_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.created = now;
      a.last_updated = now;
      a.last_vote_time = now;
      a.last_view_time = now;
      a.last_share_time = now;
      a.last_post = now;
      a.last_root_post = now;
      a.last_transfer_time = now;
      a.last_activity_reward = now;
      a.last_account_recovery = now;
      a.last_community_created = now;
      a.last_asset_created = now;
      from_string( a.json, "" );
      from_string( a.json_private, "" );
      from_string( a.details, INIT_DETAILS );
      from_string( a.url, INIT_URL );
      from_string( a.profile_image, INIT_IMAGE );
      a.membership = membership_tier_type::TOP_MEMBERSHIP;
      a.membership_expiration = time_point::maximum();
      a.mined = true;
      a.active = true;
      a.can_vote = true;
      a.revenue_share = false;
   });

   create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = INIT_CEO;
      auth.owner_auth.add_authority( get_public_key( INIT_CEO, OWNER_KEY_STR, INIT_ACCOUNT_PASSWORD ), 1 );
      auth.owner_auth.weight_threshold = 1;
      auth.active_auth.add_authority( get_public_key( INIT_CEO, ACTIVE_KEY_STR, INIT_ACCOUNT_PASSWORD ), 1 );
      auth.active_auth.weight_threshold = 1;
      auth.posting_auth.add_authority( get_public_key( INIT_CEO, POSTING_KEY_STR, INIT_ACCOUNT_PASSWORD ), 1 );
      auth.posting_auth.weight_threshold = 1;
   });

   create< account_permission_object >( [&]( account_permission_object& aao )
   {
      aao.account = INIT_CEO;
   });

   create< account_following_object >( [&]( account_following_object& afo ) 
   {
      afo.account = INIT_CEO;
      afo.last_updated = now;
   });

   create< account_business_object >( [&]( account_business_object& abo )
   {
      abo.account = INIT_ACCOUNT;
      abo.business_type = business_structure_type::PUBLIC_BUSINESS;
      abo.executive_board.CHIEF_EXECUTIVE_OFFICER = INIT_CEO;
      abo.officer_vote_threshold = 1000 * BLOCKCHAIN_PRECISION;
      abo.business_public_key = get_public_key( abo.account, "business", INIT_ACCOUNT_PASSWORD );
      abo.members.insert( INIT_CEO );
      abo.officers.insert( INIT_CEO );
      abo.executives.insert( INIT_CEO );
      abo.equity_assets.insert( SYMBOL_EQUITY );
      abo.equity_revenue_shares[ SYMBOL_EQUITY ] = DIVIDEND_SHARE_PERCENT;
      abo.credit_assets.insert( SYMBOL_CREDIT );
      abo.credit_revenue_shares[ SYMBOL_CREDIT ] = BUYBACK_SHARE_PERCENT;
      abo.active = true;
      abo.created = now;
      abo.last_updated = now;
   });

   create< account_officer_vote_object >( [&]( account_officer_vote_object& aovo )
   {
      aovo.account = INIT_CEO;
      aovo.business_account = INIT_ACCOUNT;
      aovo.officer_account = INIT_CEO;
      aovo.vote_rank = 1;
   });

   create< account_executive_vote_object >( [&]( account_executive_vote_object& aevo )
   {
      aevo.account = INIT_CEO;
      aevo.business_account = INIT_ACCOUNT;
      aevo.executive_account = INIT_CEO;
      aevo.role = executive_role_type::CHIEF_EXECUTIVE_OFFICER;
      aevo.vote_rank = 1;
   });

   create< governance_account_object >( [&]( governance_account_object& gao )
   {
      gao.account = INIT_ACCOUNT;
      from_string( gao.url, INIT_URL );
      from_string( gao.details, INIT_DETAILS );
      gao.created = now;
      gao.active = true;
   });

   create< supernode_object >( [&]( supernode_object& s )
   {
      s.account = INIT_ACCOUNT;
      from_string( s.url, INIT_URL );
      from_string( s.details, INIT_DETAILS );
      from_string( s.node_api_endpoint, INIT_NODE_ENDPOINT );
      from_string( s.auth_api_endpoint, INIT_AUTH_ENDPOINT );
      from_string( s.notification_api_endpoint, INIT_NOTIFICATION_ENDPOINT );
      from_string( s.ipfs_endpoint, INIT_IPFS_ENDPOINT );
      from_string( s.bittorrent_endpoint, INIT_BITTORRENT_ENDPOINT );
      s.active = true;
      s.created = now;
      s.last_updated = now;
      s.last_activation_time = now;
   });

   create< network_officer_object >( [&]( network_officer_object& noo )
   {
      noo.account = INIT_CEO;
      noo.officer_type = network_officer_role_type::DEVELOPMENT;
      from_string( noo.url, INIT_URL );
      from_string( noo.details, INIT_DETAILS );
      noo.officer_approved = true;
      noo.reward_currency = SYMBOL_COIN;
      noo.created = now;
      noo.active = true;
   });

   create< executive_board_object >( [&]( executive_board_object& ebo )
   {
      ebo.account = INIT_ACCOUNT;
      ebo.budget = asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT );
      from_string( ebo.url, INIT_URL );
      from_string( ebo.details, INIT_DETAILS );
      ebo.active = true;
      ebo.created = now;
      ebo.board_approved = true;
   });

   create< interface_object >( [&]( interface_object& i )
   {
      i.account = INIT_ACCOUNT;
      from_string( i.url, INIT_URL );
      from_string( i.details, INIT_DETAILS );
      i.active = true;
      i.created = now;
      i.last_updated = now;
   });

   create< mediator_object >( [&]( mediator_object& i )
   {
      i.account = INIT_ACCOUNT;
      from_string( i.url, INIT_URL );
      from_string( i.details, INIT_DETAILS );
      i.active = true;
      i.created = now;
      i.last_updated = now;
   });

   // Create anonymous account for anonymous posting: password = "anonymouspassword"

   create< account_object >( [&]( account_object& a )
   {
      a.name = ANON_ACCOUNT;
      a.registrar = INIT_ACCOUNT;
      a.referrer = INIT_ACCOUNT;
      a.secure_public_key = get_public_key( a.name, SECURE_KEY_STR, ANON_ACCOUNT_PASSWORD );
      a.connection_public_key = get_public_key( a.name, CONNECTION_KEY_STR, ANON_ACCOUNT_PASSWORD );
      a.friend_public_key = get_public_key( a.name, FRIEND_KEY_STR, ANON_ACCOUNT_PASSWORD );
      a.companion_public_key = get_public_key( a.name, COMPANION_KEY_STR, ANON_ACCOUNT_PASSWORD );
      a.created = now;
      a.last_updated = now;
      a.last_vote_time = now;
      a.last_view_time = now;
      a.last_share_time = now;
      a.last_post = now;
      a.last_root_post = now;
      a.last_transfer_time = now;
      a.last_activity_reward = now;
      a.last_account_recovery = now;
      a.last_community_created = now;
      a.last_asset_created = now;
      from_string( a.json, "" );
      from_string( a.json_private, "" );
      from_string( a.details, INIT_DETAILS );
      from_string( a.url, INIT_URL );
      from_string( a.profile_image, INIT_IMAGE );
      a.membership = membership_tier_type::TOP_MEMBERSHIP;
      a.membership_expiration = time_point::maximum();
      a.mined = true;
      a.active = true;
      a.can_vote = true;
      a.revenue_share = false;
   });

   create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = ANON_ACCOUNT;
      auth.owner_auth.add_authority( get_public_key( ANON_ACCOUNT, OWNER_KEY_STR, ANON_ACCOUNT_PASSWORD ), 1 );
      auth.owner_auth.weight_threshold = 1;
      auth.active_auth.add_authority( get_public_key( ANON_ACCOUNT, ACTIVE_KEY_STR, ANON_ACCOUNT_PASSWORD ), 1 );
      auth.active_auth.weight_threshold = 1;
      auth.posting_auth.add_authority( get_public_key( ANON_ACCOUNT, POSTING_KEY_STR, ANON_ACCOUNT_PASSWORD ), 1 );
      auth.posting_auth.weight_threshold = 1;
   });

   create< account_permission_object >( [&]( account_permission_object& aao )
   {
      aao.account = ANON_ACCOUNT;
   });

   create< account_following_object >( [&]( account_following_object& afo )
   {
      afo.account = ANON_ACCOUNT;
      afo.last_updated = now;
   });

   create< account_object >( [&]( account_object& a )
   {
      a.name = PRODUCER_ACCOUNT;
      a.registrar = INIT_ACCOUNT;
      a.referrer = INIT_ACCOUNT;
      a.secure_public_key = get_public_key( a.name, SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.connection_public_key = get_public_key( a.name, CONNECTION_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.friend_public_key = get_public_key( a.name, FRIEND_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.companion_public_key = get_public_key( a.name, COMPANION_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.created = now;
      a.last_updated = now;
      a.last_vote_time = now;
      a.last_view_time = now;
      a.last_share_time = now;
      a.last_post = now;
      a.last_root_post = now;
      a.last_transfer_time = now;
      a.last_activity_reward = now;
      a.last_account_recovery = now;
      a.last_community_created = now;
      a.last_asset_created = now;
      from_string( a.json, "" );
      from_string( a.json_private, "" );
      from_string( a.details, INIT_DETAILS );
      from_string( a.url, INIT_URL );
      from_string( a.profile_image, INIT_IMAGE );
      a.membership = membership_tier_type::TOP_MEMBERSHIP;
      a.membership_expiration = time_point::maximum();
      a.mined = true;
      a.active = true;
      a.can_vote = true;
      a.revenue_share = false;
   });

   const account_authority_object& producer_auth = create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = PRODUCER_ACCOUNT;
      auth.owner_auth.weight_threshold = 1;
      auth.active_auth.weight_threshold = 1;
      auth.posting_auth.weight_threshold = 1;
   });

   create< account_permission_object >( [&]( account_permission_object& aao )
   {
      aao.account = PRODUCER_ACCOUNT;
   });

   create< account_following_object >( [&]( account_following_object& afo )
   {
      afo.account = PRODUCER_ACCOUNT;
      afo.last_updated = now;
   });

   // Create NULL account, which cannot make operations. 

   create< account_object >( [&]( account_object& a )
   {
      a.name = NULL_ACCOUNT;
      a.registrar = INIT_ACCOUNT;
      a.referrer = INIT_ACCOUNT;
      a.secure_public_key = get_public_key( a.name, SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.connection_public_key = get_public_key( a.name, CONNECTION_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.friend_public_key = get_public_key( a.name, FRIEND_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.companion_public_key = get_public_key( a.name, COMPANION_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.created = now;
      a.last_updated = now;
      a.last_vote_time = now;
      a.last_view_time = now;
      a.last_share_time = now;
      a.last_post = now;
      a.last_root_post = now;
      a.last_transfer_time = now;
      a.last_activity_reward = now;
      a.last_account_recovery = now;
      a.last_community_created = now;
      a.last_asset_created = now;
      from_string( a.json, "" );
      from_string( a.json_private, "" );
      from_string( a.details, INIT_DETAILS );
      from_string( a.url, INIT_URL );
      from_string( a.profile_image, INIT_IMAGE );
      a.membership = membership_tier_type::TOP_MEMBERSHIP;
      a.membership_expiration = time_point::maximum();
      a.mined = true;
      a.active = true;
      a.can_vote = false;
      a.revenue_share = false;
   });

   create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = NULL_ACCOUNT;
      auth.owner_auth.weight_threshold = 1;
      auth.active_auth.weight_threshold = 1;
      auth.posting_auth.weight_threshold = 1;
   });

   create< account_permission_object >( [&]( account_permission_object& aao )
   {
      aao.account = NULL_ACCOUNT;
   });

   create< account_following_object >( [&]( account_following_object& afo )
   {
      afo.account = NULL_ACCOUNT;
      afo.last_updated = now;
   });

   create< account_object >( [&]( account_object& a )
   {
      a.name = TEMP_ACCOUNT;
      a.registrar = INIT_ACCOUNT;
      a.referrer = INIT_ACCOUNT;
      a.secure_public_key = get_public_key( a.name, SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.connection_public_key = get_public_key( a.name, CONNECTION_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.friend_public_key = get_public_key( a.name, FRIEND_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.companion_public_key = get_public_key( a.name, COMPANION_KEY_STR, INIT_ACCOUNT_PASSWORD );
      a.created = now;
      a.last_updated = now;
      a.last_vote_time = now;
      a.last_view_time = now;
      a.last_share_time = now;
      a.last_post = now;
      a.last_root_post = now;
      a.last_transfer_time = now;
      a.last_activity_reward = now;
      a.last_account_recovery = now;
      a.last_community_created = now;
      a.last_asset_created = now;
      from_string( a.json, "" );
      from_string( a.json_private, "" );
      from_string( a.details, INIT_DETAILS );
      from_string( a.url, INIT_URL );
      from_string( a.profile_image, INIT_IMAGE );
      a.membership = membership_tier_type::TOP_MEMBERSHIP;
      a.membership_expiration = time_point::maximum();
      a.mined = true;
      a.active = true;
      a.can_vote = false;
      a.revenue_share = false;
   });

   create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = TEMP_ACCOUNT;
      auth.owner_auth.weight_threshold = 0;
      auth.active_auth.weight_threshold = 0;
   });

   create< account_permission_object >( [&]( account_permission_object& aao )
   {
      aao.account = TEMP_ACCOUNT;
   });

   create< account_following_object >( [&]( account_following_object& afo )
   {
      afo.account = TEMP_ACCOUNT;
      afo.last_updated = now;
   });

   // Create COIN asset
   
   create< asset_object >( [&]( asset_object& a ) 
   {
      a.symbol = SYMBOL_COIN;
      a.max_supply = MAX_ASSET_SUPPLY;
      a.asset_type = asset_property_type::CURRENCY_ASSET;
      a.flags = 0;
      a.issuer_permissions = 0;
      a.issuer = NULL_ACCOUNT;
      a.unstake_intervals = 4;
      a.stake_intervals = 0;
      from_string( a.json, "" );
      from_string( a.details, COIN_DETAILS );
      from_string( a.url, INIT_URL );
      a.created = now;
      a.last_updated = now;
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = SYMBOL_COIN;
   });

   create< asset_currency_data_object >( [&]( asset_currency_data_object& a )
   {
      a.symbol = SYMBOL_COIN;
      a.block_reward = BLOCK_REWARD;
      a.block_reward_reduction_percent = 0;
      a.block_reward_reduction_days = 0;
      a.content_reward_percent = CONTENT_REWARD_PERCENT;
      a.equity_asset = SYMBOL_EQUITY;
      a.equity_reward_percent = EQUITY_REWARD_PERCENT;
      a.producer_reward_percent = PRODUCER_REWARD_PERCENT;
      a.supernode_reward_percent = SUPERNODE_REWARD_PERCENT;
      a.power_reward_percent = POWER_REWARD_PERCENT;
      a.enterprise_fund_reward_percent = COMMUNITY_FUND_REWARD_PERCENT;
      a.development_reward_percent = DEVELOPMENT_REWARD_PERCENT;
      a.marketing_reward_percent = MARKETING_REWARD_PERCENT;
      a.advocacy_reward_percent = ADVOCACY_REWARD_PERCENT;
      a.activity_reward_percent = ACTIVITY_REWARD_PERCENT;
      a.producer_block_reward_percent = PRODUCER_BLOCK_PERCENT;
      a.validation_reward_percent = PRODUCER_VALIDATOR_PERCENT;
      a.txn_stake_reward_percent = PRODUCER_TXN_STAKE_PERCENT;
      a.work_reward_percent = PRODUCER_WORK_PERCENT;
      a.producer_activity_reward_percent = PRODUCER_ACTIVITY_PERCENT;
   });

   // Create Equity asset

   create< asset_object >( [&]( asset_object& a )
   {
      a.symbol = SYMBOL_EQUITY;
      a.max_supply = INIT_EQUITY_SUPPLY;
      a.asset_type = asset_property_type::EQUITY_ASSET;
      a.flags = 0;
      a.issuer_permissions = 0;
      a.issuer = INIT_ACCOUNT;
      a.unstake_intervals = 0;
      a.stake_intervals = 4;
      from_string( a.json, "" );
      from_string( a.details, EQUITY_DETAILS );
      from_string( a.url, INIT_URL );
      a.created = now;
      a.last_updated = now;
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = SYMBOL_EQUITY;
   });

   create< asset_equity_data_object >( [&]( asset_equity_data_object& a )
   {
      a.business_account = INIT_ACCOUNT;
      a.symbol = SYMBOL_EQUITY;
      a.dividend_share_percent = DIVIDEND_SHARE_PERCENT;
      a.min_active_time = EQUITY_ACTIVITY_TIME;
      a.min_balance = BLOCKCHAIN_PRECISION;
      a.min_producers = EQUITY_MIN_PRODUCERS;
      a.boost_balance = EQUITY_BOOST_BALANCE;
      a.boost_activity = EQUITY_BOOST_ACTIVITY;
      a.boost_producers = EQUITY_BOOST_PRODUCERS;
      a.boost_top = EQUITY_BOOST_TOP_PERCENT;
   });

   // Create Equity distribution

   asset_unit init_input = asset_unit( INIT_ACCOUNT, BLOCKCHAIN_PRECISION, account_balance_values[ 0 ], GENESIS_TIME );
   asset_unit sender_output = asset_unit( ASSET_UNIT_SENDER, 1, account_balance_values[ 0 ], GENESIS_TIME );

   create< asset_distribution_object >( [&]( asset_distribution_object& a )
   {
      a.distribution_asset = SYMBOL_EQUITY;
      a.fund_asset = SYMBOL_COIN;
      from_string( a.details, EQUITY_DETAILS );
      from_string( a.url, INIT_URL );
      from_string( a.json, "" );
      a.distribution_rounds = 250;
      a.distribution_interval_days = 7;
      a.max_intervals_missed = 250;
      a.input_fund_unit.insert( init_input );
      a.output_distribution_unit.insert( sender_output );
      a.min_input_fund_units = 1;
      a.max_input_fund_units = 1000000;
      a.min_unit_ratio = 200000;
      a.max_unit_ratio = 200000000000;
      a.min_input_balance_units = 1;
      a.max_input_balance_units = 1000000;
      a.total_distributed = asset( 0, SYMBOL_EQUITY );
      a.total_funded = asset( 0, SYMBOL_COIN );
      a.begin_time = now + fc::days(7);
      a.next_round_time = now + fc::days(7);
      a.created = now;
      a.last_updated = now;
   });

   // Create USD asset

   create< asset_object >( [&]( asset_object& a )
   {
      a.symbol = SYMBOL_USD;
      a.issuer = NULL_ACCOUNT;
      a.asset_type = asset_property_type::STABLECOIN_ASSET;
      a.max_supply = MAX_ASSET_SUPPLY;
      a.flags = int( asset_issuer_permission_flags::producer_fed_asset );
      a.issuer_permissions = 0;
      a.unstake_intervals = 4;
      a.stake_intervals = 0;
      from_string( a.json, "" );
      from_string( a.details, USD_DETAILS );
      from_string( a.url, INIT_URL );
      a.created = now;
      a.last_updated = now;
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = SYMBOL_USD;
   });

   create< asset_stablecoin_data_object >( [&]( asset_stablecoin_data_object& a )
   {
      a.symbol = SYMBOL_USD;
      a.backing_asset = SYMBOL_COIN;
      a.current_feed_publication_time = now;
      a.feed_lifetime = PRICE_FEED_LIFETIME;
      a.minimum_feeds = 1;
      a.asset_settlement_delay = ASSET_SETTLEMENT_DELAY;
      a.asset_settlement_offset_percent = ASSET_SETTLEMENT_OFFSET;
      a.maximum_asset_settlement_volume = ASSET_SETTLEMENT_MAX_VOLUME;

      price_feed feed;
      feed.settlement_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      a.feeds[ GENESIS_ACCOUNT_BASE_NAME ] = make_pair( now, feed );
      a.update_median_feeds( now );
   });

   // Create Credit asset
   
   create< asset_object >( [&]( asset_object& a )
   {
      a.symbol = SYMBOL_CREDIT;
      a.asset_type = asset_property_type::CREDIT_ASSET;
      a.flags = 0;
      a.issuer_permissions = 0;
      a.issuer = INIT_ACCOUNT;
      a.unstake_intervals = 4;
      a.stake_intervals = 0;
      from_string( a.json, "" );
      from_string( a.details, CREDIT_DETAILS );
      from_string( a.url, INIT_URL );
      a.created = now;
      a.last_updated = now;
   });

   create< asset_dynamic_data_object >([&](asset_dynamic_data_object& a) 
   {
      a.symbol = SYMBOL_CREDIT;
   });

   create< asset_credit_data_object >( [&]( asset_credit_data_object& a )
   {
      a.business_account = INIT_ACCOUNT;
      a.symbol = SYMBOL_CREDIT;
      a.buyback_asset = SYMBOL_USD;
      a.buyback_price = price( asset( 1, SYMBOL_USD ), asset( 1, SYMBOL_CREDIT ) );
      a.buyback_share_percent = BUYBACK_SHARE_PERCENT;
      a.liquid_fixed_interest_rate = LIQUID_FIXED_INTEREST_RATE;
      a.liquid_variable_interest_rate = LIQUID_VARIABLE_INTEREST_RATE;
      a.staked_fixed_interest_rate = STAKED_FIXED_INTEREST_RATE;
      a.staked_variable_interest_rate = STAKED_VARIABLE_INTEREST_RATE;
      a.savings_fixed_interest_rate = SAVINGS_FIXED_INTEREST_RATE;
      a.savings_variable_interest_rate = SAVINGS_VARIABLE_INTEREST_RATE;
      a.var_interest_range = VAR_INTEREST_RANGE;
   });

   const producer_schedule_object& pso = create< producer_schedule_object >( [&]( producer_schedule_object& pso )
   {
      pso.current_shuffled_producers.reserve( size_t( TOTAL_PRODUCERS ) );
      pso.num_scheduled_producers = TOTAL_PRODUCERS;
      pso.last_pow_update = now;
   });

   // Create accounts for genesis producers

   chain_properties chain_props;

   for( int i = 0; i < ( GENESIS_PRODUCER_AMOUNT + GENESIS_EXTRA_PRODUCERS ); ++i )
   {
      account_name_type producer_name = GENESIS_ACCOUNT_BASE_NAME + ( i ? fc::to_string( i ) : std::string() );

      create< account_object >( [&]( account_object& a )
      {
         a.name = producer_name;
         a.registrar = INIT_ACCOUNT;
         a.referrer = INIT_ACCOUNT;
         a.secure_public_key = get_public_key( a.name, SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
         a.connection_public_key = get_public_key( a.name, CONNECTION_KEY_STR, INIT_ACCOUNT_PASSWORD );
         a.friend_public_key = get_public_key( a.name, FRIEND_KEY_STR, INIT_ACCOUNT_PASSWORD );
         a.companion_public_key = get_public_key( a.name, COMPANION_KEY_STR, INIT_ACCOUNT_PASSWORD );
         a.created = now;
         a.last_updated = now;
         a.last_vote_time = now;
         a.last_view_time = now;
         a.last_share_time = now;
         a.last_post = now;
         a.last_root_post = now;
         a.last_transfer_time = now;
         a.last_activity_reward = now;
         a.last_account_recovery = now;
         a.last_community_created = now;
         a.last_asset_created = now;
         from_string( a.json, "" );
         from_string( a.json_private, "" );
         from_string( a.details, INIT_DETAILS );
         from_string( a.url, INIT_URL );
         from_string( a.profile_image, INIT_IMAGE );
         a.membership = membership_tier_type::TOP_MEMBERSHIP;
         a.membership_expiration = time_point::maximum();
         a.mined = true;
         a.active = true;
         a.can_vote = true;
         a.revenue_share = false;
      });

      create< account_authority_object >( [&]( account_authority_object& auth )
      {
         auth.account = producer_name;
         auth.owner_auth.add_authority( get_public_key( auth.account, OWNER_KEY_STR, INIT_ACCOUNT_PASSWORD ), 1 );
         auth.owner_auth.weight_threshold = 1;
         auth.active_auth.add_authority( get_public_key( auth.account, ACTIVE_KEY_STR, INIT_ACCOUNT_PASSWORD ), 1 );
         auth.active_auth.weight_threshold = 1;
         auth.posting_auth.add_authority( get_public_key( auth.account, POSTING_KEY_STR, INIT_ACCOUNT_PASSWORD ), 1 );
         auth.posting_auth.weight_threshold = 1;
      });

      create< account_permission_object >( [&]( account_permission_object& aao )
      {
         aao.account = producer_name;
      });

      create< account_following_object >( [&]( account_following_object& afo )
      {
         afo.account = producer_name;
         afo.last_updated = now;
      });

      create< producer_object >( [&]( producer_object& p )
      {
         p.owner = producer_name;
         p.props = chain_props;
         p.signing_key = get_public_key( p.owner, PRODUCER_KEY_STR, INIT_ACCOUNT_PASSWORD );
         p.schedule = producer_object::top_voting_producer;
         p.active = true;
         p.running_version = BLOCKCHAIN_VERSION;
         from_string( p.json, "" );
         from_string( p.details, INIT_DETAILS );
         from_string( p.url, INIT_URL );
         p.created = now;
         p.last_updated = now;
      });

      if( i < GENESIS_PRODUCER_AMOUNT )
      {
         modify( pso, [&]( producer_schedule_object& pso )
         {
            pso.current_shuffled_producers.push_back( producer_name );
         });

         modify( producer_auth, [&]( account_authority_object& a )
         {
            a.active_auth.add_authority( producer_name, 1 );  
         });
      }
   }

   create< community_object >( [&]( community_object& bo )
   {
      bo.name = INIT_COMMUNITY;
      bo.founder = INIT_ACCOUNT;
      
      from_string( bo.display_name, "General" );
      from_string( bo.details, INIT_DETAILS );
      from_string( bo.url, INIT_URL );
      from_string( bo.profile_image, INIT_IMAGE );
      from_string( bo.cover_image, INIT_IMAGE );
      from_string( bo.json, "" );
      from_string( bo.json_private, "" );

      bo.community_privacy = community_privacy_type::OPEN_PUBLIC_COMMUNITY;

      bo.community_member_key = get_public_key( INIT_COMMUNITY, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      bo.community_moderator_key = get_public_key( INIT_COMMUNITY, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      bo.community_admin_key = get_public_key( INIT_COMMUNITY, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );

      bo.max_rating = 9;
      
      bo.flags = 0;
      bo.permissions = COMMUNITY_PERMISSION_MASK;
      bo.created = now;
      bo.last_updated = now;
      bo.last_post = now;
      bo.last_root_post = now;
      bo.active = true;
   });

   create< community_member_object >( [&]( community_member_object& bmo )
   {
      bmo.name = INIT_COMMUNITY;
      bmo.founder = INIT_ACCOUNT;
      bmo.subscribers.insert( INIT_ACCOUNT );
      bmo.members.insert( INIT_ACCOUNT );
      bmo.moderators.insert( INIT_ACCOUNT );
      bmo.administrators.insert( INIT_ACCOUNT );
      bmo.community_privacy = community_privacy_type::OPEN_PUBLIC_COMMUNITY;
      bmo.last_updated = now;
   });

   create< community_moderator_vote_object >( [&]( community_moderator_vote_object& v )
   {
      v.moderator = INIT_ACCOUNT;
      v.account = INIT_ACCOUNT;
      v.community = INIT_COMMUNITY;
      v.vote_rank = 1;
   });

   // Allocate Genesis block reward to Init Account and create primary asset liquidity and credit pools.

   /**
    * Create Primary Liquidity Pools in Block 0.
    * 
    * [ coin/equity, coin/usd, coin/credit, equity/usd, equity/credit, usd/credit ]
    * Creates initial collateral positions of USD Asset, 
    * and rewards init account with small amount of Equity 
    * and Credit assets in liquidity and credit pools.
    */

   const asset_currency_data_object& currency = get_currency_data( SYMBOL_COIN );
   asset block_reward = currency.block_reward;
   FC_ASSERT( block_reward.symbol == SYMBOL_COIN && 
      block_reward.amount == 25 * BLOCKCHAIN_PRECISION, 
      "Block reward is not the correct symbol: ${s} or amount: ${a}",
      ("s",block_reward.symbol)("a",block_reward.amount));

   flat_set< asset_symbol_type > new_strikes;
   flat_set< asset_symbol_type > option_strikes;
   flat_set< date_type > new_dates;
   date_type next_date = date_type( now );

   for( int i = 0; i < 12; i++ )      // compile the next 12 months of expiration dates
   {
      if( next_date.month != 12 )
      {
         next_date = date_type( 1, next_date.month + 1, next_date.year );
      }
      else
      {
         next_date = date_type( 1, 1, next_date.year + 1 );
      }
      new_dates.insert( next_date );
   }

   adjust_liquid_balance( INIT_ACCOUNT, block_reward );
   adjust_liquid_balance( INIT_ACCOUNT, asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
   adjust_liquid_balance( INIT_ACCOUNT, asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );

   asset liquid_coin = get_liquid_balance( INIT_ACCOUNT, SYMBOL_COIN );

   FC_ASSERT( liquid_coin.symbol == SYMBOL_COIN && 
      liquid_coin.amount == 25 * BLOCKCHAIN_PRECISION, 
      "INIT_ACCOUNT does not have correct balance - symbol: ${s} or amount: ${a}", 
      ("s",liquid_coin.symbol)("a",liquid_coin.amount) );

   create< call_order_object >( [&]( call_order_object& coo )
   {
      coo.borrower = INIT_ACCOUNT;
      coo.collateral = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      coo.debt = asset( 5 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      coo.created = now;
      coo.last_updated = now;
   });

   adjust_liquid_balance( INIT_ACCOUNT, -asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
   adjust_pending_supply( asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
   adjust_liquid_balance( INIT_ACCOUNT, asset( 5 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

   asset_symbol_type coin_equity_symbol = string( LIQUIDITY_ASSET_PREFIX )+string( SYMBOL_COIN )+"."+string( SYMBOL_EQUITY );

   create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = NULL_ACCOUNT;
      a.symbol = coin_equity_symbol;
      a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = coin_equity_symbol;
   });

   create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
   {   
      a.symbol_a = SYMBOL_COIN;
      a.symbol_b = SYMBOL_EQUITY;
      a.symbol_liquid = coin_equity_symbol;
      a.balance_a = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      a.balance_b = asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY );
      a.balance_liquid = asset( BLOCKCHAIN_PRECISION, coin_equity_symbol );
      a.hour_median_price = price( a.balance_a, a.balance_b );
      a.day_median_price = price( a.balance_a, a.balance_b );
      a.price_history.push_back( price( a.balance_a, a.balance_b ) );
   });

   const asset_option_pool_object& coin_equity_option = create< asset_option_pool_object >( [&]( asset_option_pool_object& aopo )
   {   
      aopo.base_symbol = SYMBOL_COIN;
      aopo.quote_symbol = SYMBOL_EQUITY;
      aopo.add_strike_prices( asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) / asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ), new_dates );
   });

   new_strikes = coin_equity_option.get_strike_prices( asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) / asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ), new_dates );

   for( asset_symbol_type s : new_strikes )
   {
      option_strikes.insert( s );
   }
   
   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
   adjust_liquid_balance( INIT_ACCOUNT, asset( BLOCKCHAIN_PRECISION, coin_equity_symbol ) );

   asset_symbol_type coin_usd_symbol = string( LIQUIDITY_ASSET_PREFIX )+string( SYMBOL_COIN )+"."+string( SYMBOL_USD );

   create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = NULL_ACCOUNT;
      a.symbol = coin_usd_symbol;
      a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = coin_usd_symbol;
   });

   create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
   {   
      a.symbol_a = SYMBOL_COIN;
      a.symbol_b = SYMBOL_USD;
      a.symbol_liquid = coin_usd_symbol;
      a.balance_a = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      a.balance_b = asset( BLOCKCHAIN_PRECISION, SYMBOL_USD );
      a.balance_liquid = asset( BLOCKCHAIN_PRECISION, coin_usd_symbol );
      a.hour_median_price = price( a.balance_a, a.balance_b );
      a.day_median_price = price( a.balance_a, a.balance_b );
      a.price_history.push_back( price( a.balance_a, a.balance_b ) );
   });

   const asset_option_pool_object& coin_usd_option = create< asset_option_pool_object >( [&]( asset_option_pool_object& aopo )
   {   
      aopo.base_symbol = SYMBOL_COIN;
      aopo.quote_symbol = SYMBOL_USD;
      aopo.add_strike_prices( asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) / asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), new_dates );
   });

   new_strikes = coin_usd_option.get_strike_prices( asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) / asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), new_dates );

   for( asset_symbol_type s : new_strikes )
   {
      option_strikes.insert( s );
   }

   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
   adjust_liquid_balance( INIT_ACCOUNT, asset( BLOCKCHAIN_PRECISION, coin_usd_symbol ) );

   asset_symbol_type coin_credit_symbol = string( LIQUIDITY_ASSET_PREFIX )+string( SYMBOL_COIN )+"."+string( SYMBOL_CREDIT );

   create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = NULL_ACCOUNT;
      a.symbol = coin_credit_symbol;
      a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = coin_credit_symbol;
   });

   create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
   {   
      a.symbol_a = SYMBOL_COIN;
      a.symbol_b = SYMBOL_CREDIT;
      a.symbol_liquid = coin_credit_symbol;
      a.balance_a = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      a.balance_b = asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT );
      a.balance_liquid = asset( BLOCKCHAIN_PRECISION, coin_credit_symbol );
      a.hour_median_price = price( a.balance_a, a.balance_b );
      a.day_median_price = price( a.balance_a, a.balance_b );
      a.price_history.push_back( price( a.balance_a, a.balance_b ) );
   });

   const asset_option_pool_object& coin_credit_option = create< asset_option_pool_object >( [&]( asset_option_pool_object& aopo )
   {   
      aopo.base_symbol = SYMBOL_COIN;
      aopo.quote_symbol = SYMBOL_CREDIT;
      aopo.add_strike_prices( asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) / asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ), new_dates );
   });

   new_strikes = coin_credit_option.get_strike_prices( asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) / asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ), new_dates );

   for( asset_symbol_type s : new_strikes )
   {
      option_strikes.insert( s );
   }

   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
   adjust_liquid_balance( INIT_ACCOUNT, asset( BLOCKCHAIN_PRECISION, coin_credit_symbol ) );

   asset_symbol_type equity_usd_symbol = string( LIQUIDITY_ASSET_PREFIX )+string( SYMBOL_EQUITY )+"."+string( SYMBOL_USD );

   create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = NULL_ACCOUNT;
      a.symbol = equity_usd_symbol;
      a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = equity_usd_symbol;
   });
      
   create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
   {   
      a.symbol_a = SYMBOL_EQUITY;
      a.symbol_b = SYMBOL_USD;
      a.symbol_liquid = equity_usd_symbol;
      a.balance_a = asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY );
      a.balance_b = asset( BLOCKCHAIN_PRECISION, SYMBOL_USD );
      a.balance_liquid = asset( BLOCKCHAIN_PRECISION, equity_usd_symbol );
      a.hour_median_price = price( a.balance_a, a.balance_b );
      a.day_median_price = price( a.balance_a, a.balance_b );
      a.price_history.push_back( price( a.balance_a, a.balance_b ) );
   });

   const asset_option_pool_object& equity_usd_option = create< asset_option_pool_object >( [&]( asset_option_pool_object& aopo )
   {   
      aopo.base_symbol = SYMBOL_EQUITY;
      aopo.quote_symbol = SYMBOL_USD;
      aopo.add_strike_prices( asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) / asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), new_dates );
   });

   new_strikes = equity_usd_option.get_strike_prices( asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) / asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), new_dates );

   for( asset_symbol_type s : new_strikes )
   {
      option_strikes.insert( s );
   }

   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
   adjust_liquid_balance( INIT_ACCOUNT, asset( BLOCKCHAIN_PRECISION, equity_usd_symbol ) );

   asset_symbol_type equity_credit_symbol = string( LIQUIDITY_ASSET_PREFIX )+string( SYMBOL_EQUITY )+"."+string( SYMBOL_CREDIT );
   
   create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = NULL_ACCOUNT;
      a.symbol = equity_credit_symbol;
      a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = equity_credit_symbol;
   });
      
   create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
   {   
      a.symbol_a = SYMBOL_EQUITY;
      a.symbol_b = SYMBOL_CREDIT;
      a.symbol_liquid = equity_credit_symbol;
      a.balance_a = asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY );
      a.balance_b = asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT );
      a.balance_liquid = asset( BLOCKCHAIN_PRECISION, equity_credit_symbol );
      a.hour_median_price = price( a.balance_a, a.balance_b );
      a.day_median_price = price( a.balance_a, a.balance_b );
      a.price_history.push_back( price( a.balance_a, a.balance_b ) );
   });

   const asset_option_pool_object& equity_credit_option = create< asset_option_pool_object >( [&]( asset_option_pool_object& aopo )
   {   
      aopo.base_symbol = SYMBOL_EQUITY;
      aopo.quote_symbol = SYMBOL_CREDIT;
      aopo.add_strike_prices( asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) / asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ), new_dates );
   });

   new_strikes = equity_credit_option.get_strike_prices( asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) / asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ), new_dates );

   for( asset_symbol_type s : new_strikes )
   {
      option_strikes.insert( s );
   }

   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
   adjust_liquid_balance( INIT_ACCOUNT, asset( BLOCKCHAIN_PRECISION, equity_credit_symbol ) );

   asset_symbol_type usd_credit_symbol = string( LIQUIDITY_ASSET_PREFIX )+string( SYMBOL_USD )+"."+string( SYMBOL_CREDIT );

   create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = NULL_ACCOUNT;
      a.symbol = usd_credit_symbol;
      a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = usd_credit_symbol;
   });

   create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
   {   
      a.symbol_a = SYMBOL_USD;
      a.symbol_b = SYMBOL_CREDIT;
      a.symbol_liquid = usd_credit_symbol;
      a.balance_a = asset( BLOCKCHAIN_PRECISION, SYMBOL_USD );
      a.balance_b = asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT );
      a.balance_liquid = asset( BLOCKCHAIN_PRECISION, usd_credit_symbol );
      a.hour_median_price = price( a.balance_a, a.balance_b );
      a.day_median_price = price( a.balance_a, a.balance_b );
      a.price_history.push_back( price( a.balance_a, a.balance_b ) );
   });

   const asset_option_pool_object& usd_credit_option = create< asset_option_pool_object >( [&]( asset_option_pool_object& aopo )
   {   
      aopo.base_symbol = SYMBOL_USD;
      aopo.quote_symbol = SYMBOL_CREDIT;
      aopo.add_strike_prices( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ) / asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ), new_dates );
   });

   new_strikes = usd_credit_option.get_strike_prices( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ) / asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ), new_dates );

   for( asset_symbol_type s : new_strikes )
   {
      option_strikes.insert( s );
   }

   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
   adjust_liquid_balance( INIT_ACCOUNT, asset( BLOCKCHAIN_PRECISION, usd_credit_symbol ) );

   for( asset_symbol_type s : option_strikes )     // Create the new asset objects for the options.
   {
      option_strike strike = option_strike::from_string( s );
      const asset_object& base_asset = get_asset( strike.strike_price.base.symbol );
      const asset_object& quote_asset = get_asset( strike.strike_price.quote.symbol );

      create< asset_object >( [&]( asset_object& a )
      {
         a.symbol = s;
         a.asset_type = asset_property_type::OPTION_ASSET;
         a.issuer = NULL_ACCOUNT;
         from_string( a.display_symbol, strike.display_symbol() );
         from_string( 
            a.details, 
            strike.details( 
               to_string( quote_asset.display_symbol ), 
               to_string( quote_asset.details ), 
               to_string( base_asset.display_symbol ), 
               to_string( base_asset.details ) ) );
         
         from_string( a.json, "" );
         from_string( a.url, "" );
         a.max_supply = MAX_ASSET_SUPPLY;
         a.stake_intervals = 0;
         a.unstake_intervals = 0;
         a.market_fee_percent = 0;
         a.market_fee_share_percent = 0;
         a.issuer_permissions = 0;
         a.flags = 0;
         a.created = now;
         a.last_updated = now;
      });

      create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
      {
         a.symbol = s;
      });
   }

   // Create Primary asset credit pools [ coin, equity, usd, credit ]

   asset_symbol_type credit_asset_coin_symbol = string( CREDIT_ASSET_PREFIX )+string( SYMBOL_COIN );
   
   create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = NULL_ACCOUNT;
      a.symbol = credit_asset_coin_symbol;
      a.asset_type = asset_property_type::CREDIT_POOL_ASSET;
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = credit_asset_coin_symbol;
   });

   create< asset_credit_pool_object >( [&]( asset_credit_pool_object& a )
   {
      a.base_symbol = SYMBOL_COIN;
      a.credit_symbol = credit_asset_coin_symbol;
      a.base_balance = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      a.borrowed_balance = asset( 0, SYMBOL_COIN );
      a.credit_balance = asset( 100 * BLOCKCHAIN_PRECISION, credit_asset_coin_symbol );
      a.last_price = price( a.base_balance, a.credit_balance );
   });

   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
   adjust_liquid_balance( INIT_ACCOUNT, asset( 100 * BLOCKCHAIN_PRECISION, credit_asset_coin_symbol ) );

   asset_symbol_type credit_asset_equity_symbol = string( CREDIT_ASSET_PREFIX )+string( SYMBOL_EQUITY );
   
   create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = NULL_ACCOUNT;
      a.symbol = credit_asset_equity_symbol;
      a.asset_type = asset_property_type::CREDIT_POOL_ASSET; 
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = credit_asset_equity_symbol;
   });

   create< asset_credit_pool_object >( [&]( asset_credit_pool_object& a )
   {
      a.base_symbol = SYMBOL_EQUITY;
      a.credit_symbol = credit_asset_equity_symbol;
      a.base_balance = asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY );
      a.borrowed_balance = asset( 0, SYMBOL_EQUITY );
      a.credit_balance = asset( 100 * BLOCKCHAIN_PRECISION, credit_asset_equity_symbol );
      a.last_price = price( a.base_balance, a.credit_balance );
   });

   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
   adjust_liquid_balance( INIT_ACCOUNT, asset( 100 * BLOCKCHAIN_PRECISION, credit_asset_equity_symbol ) );

   asset_symbol_type credit_asset_usd_symbol = string( CREDIT_ASSET_PREFIX )+string( SYMBOL_USD );
   
   create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = NULL_ACCOUNT;
      a.symbol = credit_asset_usd_symbol;
      a.asset_type = asset_property_type::CREDIT_POOL_ASSET; 
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = credit_asset_usd_symbol;
   });

   create< asset_credit_pool_object >( [&]( asset_credit_pool_object& a )
   {
      a.base_symbol = SYMBOL_USD;
      a.credit_symbol = credit_asset_usd_symbol;
      a.base_balance = asset( BLOCKCHAIN_PRECISION, SYMBOL_USD );
      a.borrowed_balance = asset( 0, SYMBOL_USD );
      a.credit_balance = asset( 100 * BLOCKCHAIN_PRECISION, credit_asset_usd_symbol );
      a.last_price = price( a.base_balance, a.credit_balance );
   });

   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
   adjust_liquid_balance( INIT_ACCOUNT, asset( 100 * BLOCKCHAIN_PRECISION, credit_asset_usd_symbol ) );

   asset_symbol_type credit_asset_credit_symbol = string( CREDIT_ASSET_PREFIX )+string( SYMBOL_CREDIT );
   
   create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = NULL_ACCOUNT;
      a.symbol = credit_asset_credit_symbol;
      a.asset_type = asset_property_type::CREDIT_POOL_ASSET;
   });

   create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = credit_asset_credit_symbol;
   });

   create< asset_credit_pool_object >( [&]( asset_credit_pool_object& a )
   {
      a.base_symbol = SYMBOL_CREDIT;
      a.credit_symbol = credit_asset_credit_symbol;
      a.base_balance = asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT );
      a.borrowed_balance = asset( 0, SYMBOL_CREDIT );
      a.credit_balance = asset( 100 * BLOCKCHAIN_PRECISION, credit_asset_credit_symbol );
      a.last_price = price( a.base_balance, a.credit_balance );
   });

   adjust_liquid_balance( INIT_ACCOUNT, -asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
   adjust_pending_supply( asset( BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
   adjust_liquid_balance( INIT_ACCOUNT, asset( 100 * BLOCKCHAIN_PRECISION, credit_asset_credit_symbol ) );

} FC_CAPTURE_AND_RETHROW() }


void database::reindex( const fc::path& data_dir, const fc::path& shared_mem_dir, uint64_t shared_file_size )
{ try {
   ilog( "Reindexing Blockchain" );
   wipe( data_dir, shared_mem_dir, false );
   open( data_dir, shared_mem_dir, shared_file_size, chainbase::database::read_write );
   _fork_db.reset();    // override effect of _fork_db.start_block() call in open()

   auto start = fc::time_point::now();
   ASSERT( _block_log.head(), block_log_exception,
      "No blocks in block log. Cannot reindex an empty chain." );

   ilog( "Replaying blocks..." );

   uint64_t skip_flags =
      skip_producer_signature |
      skip_transaction_signatures |
      skip_transaction_dupe_check |
      skip_tapos_check |
      skip_merkle_check |
      skip_producer_schedule_check |
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


block_id_type database::find_block_id_for_num( uint64_t block_num )const
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

block_id_type database::get_block_id_for_num( uint64_t block_num )const
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

optional<signed_block> database::fetch_block_by_number( uint64_t block_num )const
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

   #ifndef SKIP_BY_TX_ID

   const auto& txn_idx = get_index< operation_index >().indices().get< by_transaction_id >();
   auto txn_itr = txn_idx.lower_bound( id );

   if( txn_itr != txn_idx.end() && 
      txn_itr->trx_id == id )
   {
      auto blk = fetch_block_by_number( txn_itr->block );
      FC_ASSERT( blk.valid(), 
         "Block not found at block height." );
      FC_ASSERT( blk->transactions.size() > txn_itr->trx_in_block,
         "Transaction in Block index too high." );

      annotated_signed_transaction result = blk->transactions[txn_itr->trx_in_block];

      result.block_num = txn_itr->block;
      result.transaction_num = txn_itr->trx_in_block;

      return result;
   }

   #endif

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
   return get< dynamic_global_property_object, by_id >( 0 );
} FC_CAPTURE_AND_RETHROW() }

time_point database::head_block_time()const
{
   return get_dynamic_global_properties().time;
}

uint64_t database::head_block_num()const
{
   return get_dynamic_global_properties().head_block_number;
}

block_id_type database::head_block_id()const
{
   return get_dynamic_global_properties().head_block_id;
}


const hardfork_property_object& database::get_hardfork_property_object()const
{ try {
   return get< hardfork_property_object >();
} FC_CAPTURE_AND_RETHROW() }

node_property_object& database::node_properties()
{
   return _node_property_object;
}

const node_property_object& database::get_node_properties() const
{
   return _node_property_object;
}

uint64_t database::last_non_undoable_block_num() const
{
   return get_dynamic_global_properties().last_irreversible_block_num;
}

const transaction_id_type& database::get_current_transaction_id()const
{
   return _current_trx_id;
}

uint16_t database::get_current_op_in_trx()const
{
   return _current_op_in_trx;
}


/**
 * Returns a Shuffled copy of a specified vector of accounts
 * High performance random generator using 256 bits of internal state.
 * http://xorshift.di.unimi.it/
 */
vector< account_name_type > database::shuffle_accounts( vector< account_name_type > accounts )const
{
   vector< account_name_type > set = accounts;
   auto now_hi = uint64_t( head_block_time().time_since_epoch().count() ) << 32;
   for( uint64_t i = 0; i < accounts.size(); ++i )
   {
      uint64_t k = now_hi +      uint64_t(i)*2685757105773633871ULL;
      uint64_t l = ( now_hi >> 1 ) + uint64_t(i)*9519819187187829351ULL;
      uint64_t m = ( now_hi >> 2 ) + uint64_t(i)*5891972902484196198ULL;
      uint64_t n = ( now_hi >> 3 ) + uint64_t(i)*2713716410970705441ULL;
      
      k ^= (l >> 7);
      l ^= (m << 9);
      m ^= (n >> 5);
      n ^= (k << 3);

      k*= 1422657256589674161ULL;
      l*= 9198587865873687103ULL;
      m*= 3060558831167252908ULL;
      n*= 4306921374257631524ULL;

      k ^= (l >> 2);
      l ^= (m << 4);
      m ^= (n >> 1);
      n ^= (k << 9);

      k*= 7947775653275249570ULL;
      l*= 9490802558828203479ULL;
      m*= 2694198061645862341ULL;
      n*= 3190223686201138213ULL;

      uint64_t rand = (k ^ l) ^ (m ^ n) ; 
      uint64_t max = set.size() - i;

      uint64_t j = i + rand % max;
      std::swap( set[i], set[j] );
   }
   return set;
};


void database::add_checkpoints( const flat_map< uint64_t, block_id_type >& checkpts )
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
bool database::push_block( const signed_block& new_block, uint32_t skip )
{
   fc::time_point begin_time = fc::time_point::now();

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

   fc::time_point end_time = fc::time_point::now();
   fc::microseconds dt = end_time - begin_time;
   if( ( new_block.block_num() % 10000 ) == 0 )
   {
      ilog( "Push_block ${b} took ${t} microseconds", ("b", new_block.block_num())("t", dt.count()) );
   }
 
   return result;
}

void database::_maybe_warn_multiple_production( uint64_t height )const
{
   vector< item_ptr > blocks = _fork_db.fetch_block_by_number( height );

   if( blocks.size() > 1 )
   {
      vector< signed_block > block_list;

      for( const auto& b : blocks )
      {
         block_list.push_back( b->data );
      }

      ilog( "Encountered block num collision at block ${n} due to a fork.", ("n", height) );
   }
   return;
}

bool database::_push_block( const signed_block& new_block )
{ try {
   uint32_t skip = get_node_properties().skip_flags;
   // uint32_t skip_undo_db = skip & skip_undo_block;

   if( !(skip&skip_fork_db) )
   {
      shared_ptr< fork_item > new_head = _fork_db.push_block( new_block );
      _maybe_warn_multiple_production( new_head->num );

      // If the head block from the longest chain does not build off of the current head, we need to switch forks.

      if( new_head->data.previous != head_block_id() )
      {
         // If the newly pushed block is the same height as head, we get head back in new_head
         // Only switch forks if new_head is actually higher than head

         if( new_head->data.block_num() > head_block_num() )
         {
            wlog( "Switching to fork: ${id}", ("id",new_head->data.id()) );
            auto branches = _fork_db.fetch_branch_from( new_head->data.id(), head_block_id() );

            // pop blocks until we hit the forked block
            while( head_block_id() != branches.second.back()->data.previous )
            { 
               pop_block();
            }

            // push all blocks on the new fork
            for( auto ritr = branches.first.rbegin(); ritr != branches.first.rend(); ++ritr )
            {
               ilog( "Pushing blocks from fork ${n} ${id}", ("n",(*ritr)->data.block_num())("id",(*ritr)->data.id()) );
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
                  wlog( "exception thrown while switching forks ${e}", ("e",except->to_detail_string() ) );
                  // remove the rest of branches.first from the fork_db, those blocks are invalid
                  while( ritr != branches.first.rend() )
                  {
                     _fork_db.remove( (*ritr)->data.id() );
                     ++ritr;
                  }
                  _fork_db.set_head( branches.second.front() );

                  // pop all blocks from the bad fork
                  while( head_block_id() != branches.second.back()->data.previous )
                  {
                     pop_block();
                  }
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
         {
            return false;
         }
      }
   }

   try
   {
      auto session = start_undo_session( true );
      apply_block( new_block, skip );
      session.push();
   }
   catch( const fc::exception& e )
   {
      elog("Failed to push new block: \n ${e}", ("e", e.to_detail_string()));
      _fork_db.remove(new_block.id());
      throw;
   }

   return false;
} FC_CAPTURE_AND_RETHROW( ( new_block ) ) }


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
   const median_chain_property_object& median_props = get_median_chain_properties();

   FC_ASSERT( fc::raw::pack_size( trx ) <= ( median_props.maximum_block_size - 256 ),
      "Transaction size must be less than maximum block size." );

   set_producing( true );

   detail::with_skip_flags( *this, skip, [&]()
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

   // The transaction applied successfully. Merge its changes into the pending block session.
   temp_session.squash();

   // notify anyone listening to pending transactions
   notify_on_pending_transaction( trx );
}


/** 
 * Creates a new block using the keys provided to the producer node, 
 * when the producer is scheduled and syncronised.
 */
signed_block database::generate_block(
   time_point when,
   const account_name_type& producer_owner,
   const fc::ecc::private_key& block_signing_private_key,
   uint32_t skip )
{
   signed_block result;
   detail::with_skip_flags( *this, skip, [&]()
   {
      try
      {
         result = _generate_block( when, producer_owner, block_signing_private_key );
      }
      FC_CAPTURE_AND_RETHROW( (producer_owner) )
   });
   return result;
}

signed_block database::_generate_block( fc::time_point when, const account_name_type& producer_owner, 
   const fc::ecc::private_key& block_signing_private_key )
{
   uint32_t skip = get_node_properties().skip_flags;
   uint64_t slot_num = get_slot_at_time( when );
   const median_chain_property_object& median_props = get_median_chain_properties();

   FC_ASSERT( slot_num > 0,
      "Slot number must be greater than zero." );
   string scheduled_producer = get_scheduled_producer( slot_num );
   FC_ASSERT( scheduled_producer == producer_owner,
      "Scheduled producer must be the same as producer owner." );

   const producer_object& producer = get_producer( producer_owner );

   if( !(skip & skip_producer_signature) )
   {
      FC_ASSERT( producer.signing_key == block_signing_private_key.get_public_key(),
         "Block signing key must be equal to the producers block signing key." );
   }
      
   signed_block pending_block;

   pending_block.previous = head_block_id();
   pending_block.timestamp = when;
   pending_block.producer = producer_owner;
   
   auto blockchainVersion = BLOCKCHAIN_VERSION;
   if( producer.running_version != BLOCKCHAIN_VERSION )
   {
      pending_block.extensions.insert( block_header_extensions( BLOCKCHAIN_VERSION ) );
   }
      
   const hardfork_property_object& hfp = get_hardfork_property_object();

   auto blockchainHardforkVersion = BLOCKCHAIN_HARDFORK_VERSION;
   if( hfp.current_hardfork_version < BLOCKCHAIN_HARDFORK_VERSION // Binary is newer hardfork than has been applied
      && ( producer.hardfork_version_vote != _hardfork_versions[ hfp.last_hardfork + 1 ] || 
      producer.hardfork_time_vote != _hardfork_times[ hfp.last_hardfork + 1 ] ) ) // producer vote does not match binary configuration
   {
      // Make vote match binary configuration
      pending_block.extensions.insert( block_header_extensions( hardfork_version_vote( _hardfork_versions[ hfp.last_hardfork + 1 ], _hardfork_times[ hfp.last_hardfork + 1 ] ) ) );
   }
   else if( hfp.current_hardfork_version == BLOCKCHAIN_HARDFORK_VERSION     // Binary does not know of a new hardfork
      && producer.hardfork_version_vote > BLOCKCHAIN_HARDFORK_VERSION )      // Voting for hardfork in the future, that we do not know of...
   {
      // Make vote match binary configuration. This is vote to not apply the new hardfork.
      pending_block.extensions.insert( block_header_extensions( hardfork_version_vote( _hardfork_versions[ hfp.last_hardfork ], _hardfork_times[ hfp.last_hardfork ] ) ) );
   }
   
   size_t total_block_size = fc::raw::pack_size( pending_block ) + 4;       // The 4 is for the max size of the transaction vector length
   uint64_t maximum_block_size = median_props.maximum_block_size;     // MAX_BLOCK_SIZE;

   with_write_lock( [&]()
   {
      // The following code throws away existing pending_tx_session and
      // rebuilds it by re-applying pending transactions.
      // This rebuild is necessary because pending transactions' validity
      // and semantics may have changed since they were received, because
      // time-based semantics are evaluated based on the current block
      // time.  These changes can only be reflected in the database when
      // the value of the "when" variable is known, which means we need to
      // re-apply pending transactions in this method.

      _pending_tx_session.reset();
      _pending_tx_session = start_undo_session( true );

      uint64_t postponed_tx_count = 0;   // pop pending state (reset to head block state)
      
      for( const signed_transaction& tx : _pending_tx )
      {
         if( tx.expiration < when )    // Only include transactions that have not expired yet for currently generating block.
            continue;

         uint64_t new_total_size = total_block_size + fc::raw::pack_size( tx );

         if( new_total_size >= maximum_block_size )
         {
            postponed_tx_count++;     // postpone transaction if it would make block too big
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
         catch ( const fc::exception& e )      // Do nothing, transaction will not be re-applied
         {
            wlog( "Transaction was not processed while generating block due to ${e}", ("e", e) );
            wlog( "The transaction was ${t}", ("t", tx) );
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

   if( !(skip & skip_producer_signature) )
   {
      pending_block.sign( block_signing_private_key );
   }

   if( !(skip & skip_block_size_check) )
   {
      FC_ASSERT( fc::raw::pack_size( pending_block ) <= MAX_BLOCK_SIZE );
   }

   // ilog( "Generated Block ID: ${i} at block height: ${h}",("i", pending_block.id() )("h", pending_block.block_num() ) );

   push_block( pending_block, skip );

   return pending_block;
}


/**
 * Removes the most recent block from the database and undoes any changes it made.
 */
void database::pop_block()
{ try {
   ilog( "Popping Block" );

   _pending_tx_session.reset();
   auto head_id = head_block_id();

   optional<signed_block> head_block = fetch_block_by_id( head_id );   // save the head block so we can recover its transactions
   ASSERT( head_block.valid(), pop_empty_chain,
      "There are no blocks to pop." );

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

void database::initialize_evaluators()
{
   // Account Evaluators

   _my->_evaluator_registry.register_evaluator< account_create_evaluator                 >();
   _my->_evaluator_registry.register_evaluator< account_update_evaluator                 >();
   _my->_evaluator_registry.register_evaluator< account_verification_evaluator           >();
   _my->_evaluator_registry.register_evaluator< account_business_evaluator               >();
   _my->_evaluator_registry.register_evaluator< account_membership_evaluator             >();
   _my->_evaluator_registry.register_evaluator< account_vote_executive_evaluator         >();
   _my->_evaluator_registry.register_evaluator< account_vote_officer_evaluator           >();
   _my->_evaluator_registry.register_evaluator< account_member_request_evaluator         >();
   _my->_evaluator_registry.register_evaluator< account_member_invite_evaluator          >();
   _my->_evaluator_registry.register_evaluator< account_accept_request_evaluator         >();
   _my->_evaluator_registry.register_evaluator< account_accept_invite_evaluator          >();
   _my->_evaluator_registry.register_evaluator< account_remove_member_evaluator          >();
   _my->_evaluator_registry.register_evaluator< account_update_list_evaluator            >();
   _my->_evaluator_registry.register_evaluator< account_producer_vote_evaluator          >();
   _my->_evaluator_registry.register_evaluator< account_update_proxy_evaluator           >();
   _my->_evaluator_registry.register_evaluator< account_request_recovery_evaluator       >();
   _my->_evaluator_registry.register_evaluator< account_recover_evaluator                >();
   _my->_evaluator_registry.register_evaluator< account_reset_evaluator                  >();
   _my->_evaluator_registry.register_evaluator< account_reset_update_evaluator           >();
   _my->_evaluator_registry.register_evaluator< account_recovery_update_evaluator        >();
   _my->_evaluator_registry.register_evaluator< account_decline_voting_evaluator         >();
   _my->_evaluator_registry.register_evaluator< account_connection_evaluator             >();
   _my->_evaluator_registry.register_evaluator< account_follow_evaluator                 >();
   _my->_evaluator_registry.register_evaluator< account_follow_tag_evaluator             >();
   _my->_evaluator_registry.register_evaluator< account_activity_evaluator               >();

   // Network Evaluators

   _my->_evaluator_registry.register_evaluator< network_officer_update_evaluator         >();
   _my->_evaluator_registry.register_evaluator< network_officer_vote_evaluator           >();
   _my->_evaluator_registry.register_evaluator< executive_board_update_evaluator         >();
   _my->_evaluator_registry.register_evaluator< executive_board_vote_evaluator           >();
   _my->_evaluator_registry.register_evaluator< governance_update_evaluator              >();
   _my->_evaluator_registry.register_evaluator< governance_subscribe_evaluator           >();
   _my->_evaluator_registry.register_evaluator< supernode_update_evaluator               >();
   _my->_evaluator_registry.register_evaluator< interface_update_evaluator               >();
   _my->_evaluator_registry.register_evaluator< mediator_update_evaluator                >();
   _my->_evaluator_registry.register_evaluator< enterprise_update_evaluator              >();
   _my->_evaluator_registry.register_evaluator< enterprise_fund_evaluator                >();
   _my->_evaluator_registry.register_evaluator< enterprise_vote_evaluator                >();

   // Comment Evaluators

   _my->_evaluator_registry.register_evaluator< comment_evaluator                        >();
   _my->_evaluator_registry.register_evaluator< message_evaluator                        >();
   _my->_evaluator_registry.register_evaluator< comment_vote_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< comment_view_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< comment_share_evaluator                  >();
   _my->_evaluator_registry.register_evaluator< comment_moderation_evaluator             >();
   _my->_evaluator_registry.register_evaluator< list_evaluator                           >();
   _my->_evaluator_registry.register_evaluator< poll_evaluator                           >();
   _my->_evaluator_registry.register_evaluator< poll_vote_evaluator                      >();
   _my->_evaluator_registry.register_evaluator< premium_purchase_evaluator               >();
   _my->_evaluator_registry.register_evaluator< premium_release_evaluator                >();

   // Community Evaluators

   _my->_evaluator_registry.register_evaluator< community_create_evaluator               >();
   _my->_evaluator_registry.register_evaluator< community_update_evaluator               >();
   _my->_evaluator_registry.register_evaluator< community_add_mod_evaluator              >();
   _my->_evaluator_registry.register_evaluator< community_add_admin_evaluator            >();
   _my->_evaluator_registry.register_evaluator< community_vote_mod_evaluator             >();
   _my->_evaluator_registry.register_evaluator< community_transfer_ownership_evaluator   >();
   _my->_evaluator_registry.register_evaluator< community_join_request_evaluator         >();
   _my->_evaluator_registry.register_evaluator< community_join_accept_evaluator          >();
   _my->_evaluator_registry.register_evaluator< community_join_invite_evaluator          >();
   _my->_evaluator_registry.register_evaluator< community_invite_accept_evaluator        >();
   _my->_evaluator_registry.register_evaluator< community_remove_member_evaluator        >();
   _my->_evaluator_registry.register_evaluator< community_blacklist_evaluator            >();
   _my->_evaluator_registry.register_evaluator< community_subscribe_evaluator            >();
   _my->_evaluator_registry.register_evaluator< community_federation_evaluator           >();
   _my->_evaluator_registry.register_evaluator< community_event_evaluator                >();
   _my->_evaluator_registry.register_evaluator< community_event_attend_evaluator         >();

   // Advertising Evaluators

   _my->_evaluator_registry.register_evaluator< ad_creative_evaluator                    >();
   _my->_evaluator_registry.register_evaluator< ad_campaign_evaluator                    >();
   _my->_evaluator_registry.register_evaluator< ad_inventory_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< ad_audience_evaluator                    >();
   _my->_evaluator_registry.register_evaluator< ad_bid_evaluator                         >();

   // Graph Data Evaluators

   _my->_evaluator_registry.register_evaluator< graph_node_evaluator                     >();
   _my->_evaluator_registry.register_evaluator< graph_edge_evaluator                     >();
   _my->_evaluator_registry.register_evaluator< graph_node_property_evaluator            >();
   _my->_evaluator_registry.register_evaluator< graph_edge_property_evaluator            >();

   // Transfer Evaluators

   _my->_evaluator_registry.register_evaluator< transfer_evaluator                       >();
   _my->_evaluator_registry.register_evaluator< transfer_request_evaluator               >();
   _my->_evaluator_registry.register_evaluator< transfer_accept_evaluator                >();
   _my->_evaluator_registry.register_evaluator< transfer_recurring_evaluator             >();
   _my->_evaluator_registry.register_evaluator< transfer_recurring_request_evaluator     >();
   _my->_evaluator_registry.register_evaluator< transfer_recurring_accept_evaluator      >();
   _my->_evaluator_registry.register_evaluator< transfer_confidential_evaluator          >();
   _my->_evaluator_registry.register_evaluator< transfer_to_confidential_evaluator       >();
   _my->_evaluator_registry.register_evaluator< transfer_from_confidential_evaluator     >();

   // Balance Evaluators

   _my->_evaluator_registry.register_evaluator< claim_reward_balance_evaluator           >();
   _my->_evaluator_registry.register_evaluator< stake_asset_evaluator                    >();
   _my->_evaluator_registry.register_evaluator< unstake_asset_evaluator                  >();
   _my->_evaluator_registry.register_evaluator< unstake_asset_route_evaluator            >();
   _my->_evaluator_registry.register_evaluator< transfer_to_savings_evaluator            >();
   _my->_evaluator_registry.register_evaluator< transfer_from_savings_evaluator          >();
   _my->_evaluator_registry.register_evaluator< delegate_asset_evaluator                 >();
   
   // Marketplace Evaluators
   
   _my->_evaluator_registry.register_evaluator< product_sale_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< product_purchase_evaluator               >();
   _my->_evaluator_registry.register_evaluator< product_auction_sale_evaluator           >();
   _my->_evaluator_registry.register_evaluator< product_auction_bid_evaluator            >();
   _my->_evaluator_registry.register_evaluator< escrow_transfer_evaluator                >();
   _my->_evaluator_registry.register_evaluator< escrow_approve_evaluator                 >();
   _my->_evaluator_registry.register_evaluator< escrow_dispute_evaluator                 >();
   _my->_evaluator_registry.register_evaluator< escrow_release_evaluator                 >();
   
   // Trading Evaluators

   _my->_evaluator_registry.register_evaluator< limit_order_evaluator                    >();
   _my->_evaluator_registry.register_evaluator< margin_order_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< auction_order_evaluator                  >();
   _my->_evaluator_registry.register_evaluator< call_order_evaluator                     >();
   _my->_evaluator_registry.register_evaluator< option_order_evaluator                   >();

   // Pool Evaluators
   
   _my->_evaluator_registry.register_evaluator< liquidity_pool_create_evaluator          >();
   _my->_evaluator_registry.register_evaluator< liquidity_pool_exchange_evaluator        >();
   _my->_evaluator_registry.register_evaluator< liquidity_pool_fund_evaluator            >();
   _my->_evaluator_registry.register_evaluator< liquidity_pool_withdraw_evaluator        >();
   _my->_evaluator_registry.register_evaluator< credit_pool_collateral_evaluator         >();
   _my->_evaluator_registry.register_evaluator< credit_pool_borrow_evaluator             >();
   _my->_evaluator_registry.register_evaluator< credit_pool_lend_evaluator               >();
   _my->_evaluator_registry.register_evaluator< credit_pool_withdraw_evaluator           >();
   _my->_evaluator_registry.register_evaluator< option_pool_create_evaluator             >();
   _my->_evaluator_registry.register_evaluator< prediction_pool_create_evaluator         >();
   _my->_evaluator_registry.register_evaluator< prediction_pool_exchange_evaluator       >();
   _my->_evaluator_registry.register_evaluator< prediction_pool_resolve_evaluator        >();
   
   // Asset Evaluators

   _my->_evaluator_registry.register_evaluator< asset_create_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< asset_update_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< asset_issue_evaluator                    >();
   _my->_evaluator_registry.register_evaluator< asset_reserve_evaluator                  >();
   _my->_evaluator_registry.register_evaluator< asset_update_issuer_evaluator            >();
   _my->_evaluator_registry.register_evaluator< asset_distribution_evaluator             >();
   _my->_evaluator_registry.register_evaluator< asset_distribution_fund_evaluator        >();
   _my->_evaluator_registry.register_evaluator< asset_option_exercise_evaluator          >();
   _my->_evaluator_registry.register_evaluator< asset_stimulus_fund_evaluator            >();
   _my->_evaluator_registry.register_evaluator< asset_update_feed_producers_evaluator    >();
   _my->_evaluator_registry.register_evaluator< asset_publish_feed_evaluator             >();
   _my->_evaluator_registry.register_evaluator< asset_settle_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< asset_global_settle_evaluator            >();
   _my->_evaluator_registry.register_evaluator< asset_collateral_bid_evaluator           >();
   
   // Block Producer Evaluators

   _my->_evaluator_registry.register_evaluator< producer_update_evaluator                >();
   _my->_evaluator_registry.register_evaluator< proof_of_work_evaluator                  >();
   _my->_evaluator_registry.register_evaluator< verify_block_evaluator                   >();
   _my->_evaluator_registry.register_evaluator< commit_block_evaluator                   >();
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
   add_core_index< median_chain_property_index             >(*this);
   add_core_index< transaction_index                       >(*this);
   add_core_index< operation_index                         >(*this);
   add_core_index< account_history_index                   >(*this);
   add_core_index< block_summary_index                     >(*this);
   add_core_index< hardfork_property_index                 >(*this);
   
   // Account Indexes

   add_core_index< account_index                           >(*this);
   add_core_index< account_authority_index                 >(*this);
   add_core_index< account_permission_index                >(*this);
   add_core_index< account_verification_index              >(*this);
   add_core_index< account_business_index                  >(*this);
   add_core_index< account_executive_vote_index            >(*this);
   add_core_index< account_officer_vote_index              >(*this);
   add_core_index< account_member_request_index            >(*this);
   add_core_index< account_member_invite_index             >(*this);
   add_core_index< account_member_key_index                >(*this);
   add_core_index< account_following_index                 >(*this);
   add_core_index< account_tag_following_index             >(*this);
   add_core_index< account_connection_index                >(*this);
   add_core_index< account_authority_history_index         >(*this);
   add_core_index< account_recovery_request_index          >(*this);
   add_core_index< account_recovery_update_request_index   >(*this);
   add_core_index< account_decline_voting_request_index    >(*this);

   // Network Indexes

   add_core_index< network_officer_index                   >(*this);
   add_core_index< network_officer_vote_index              >(*this);
   add_core_index< executive_board_index                   >(*this);
   add_core_index< executive_board_vote_index              >(*this);
   add_core_index< governance_account_index                >(*this);
   add_core_index< governance_subscription_index           >(*this);
   add_core_index< supernode_index                         >(*this);
   add_core_index< interface_index                         >(*this);
   add_core_index< mediator_index                          >(*this);
   add_core_index< enterprise_index                        >(*this);
   add_core_index< enterprise_vote_index                   >(*this);
   add_core_index< enterprise_fund_index                   >(*this);

   // Comment Indexes

   add_core_index< comment_index                           >(*this);
   add_core_index< comment_blog_index                      >(*this);
   add_core_index< comment_feed_index                      >(*this);
   add_core_index< comment_vote_index                      >(*this);
   add_core_index< comment_view_index                      >(*this);
   add_core_index< comment_share_index                     >(*this);
   add_core_index< comment_moderation_index                >(*this);
   add_core_index< comment_metrics_index                   >(*this);
   add_core_index< message_index                           >(*this);
   add_core_index< list_index                              >(*this);
   add_core_index< poll_index                              >(*this);
   add_core_index< poll_vote_index                         >(*this);
   add_core_index< premium_purchase_index                  >(*this);
   add_core_index< premium_purchase_key_index              >(*this);

   // Community Indexes

   add_core_index< community_index                         >(*this);
   add_core_index< community_member_index                  >(*this);
   add_core_index< community_member_key_index              >(*this);
   add_core_index< community_moderator_vote_index          >(*this);
   add_core_index< community_join_request_index            >(*this);
   add_core_index< community_join_invite_index             >(*this);
   add_core_index< community_federation_index              >(*this);
   add_core_index< community_event_index                   >(*this);
   add_core_index< community_event_attend_index            >(*this);

   // Advertising Indexes

   add_core_index< ad_creative_index                       >(*this);
   add_core_index< ad_campaign_index                       >(*this);
   add_core_index< ad_inventory_index                      >(*this);
   add_core_index< ad_audience_index                       >(*this);
   add_core_index< ad_bid_index                            >(*this);

   // Graph Data Indexes

   add_core_index< graph_node_index                        >(*this);
   add_core_index< graph_edge_index                        >(*this);
   add_core_index< graph_node_property_index               >(*this);
   add_core_index< graph_edge_property_index               >(*this);

   // Transfer Indexes

   add_core_index< transfer_request_index                  >(*this);
   add_core_index< transfer_recurring_index                >(*this);
   add_core_index< transfer_recurring_request_index        >(*this);

   // Balance Indexes

   add_core_index< account_balance_index                   >(*this);
   add_core_index< account_vesting_balance_index           >(*this);
   add_core_index< confidential_balance_index              >(*this);
   add_core_index< unstake_asset_route_index               >(*this);
   add_core_index< savings_withdraw_index                  >(*this);
   add_core_index< asset_delegation_index                  >(*this);
   add_core_index< asset_delegation_expiration_index       >(*this);
   
   // Marketplace Indexes

   add_core_index< product_sale_index                      >(*this);
   add_core_index< product_purchase_index                  >(*this);
   add_core_index< product_auction_sale_index              >(*this);
   add_core_index< product_auction_bid_index               >(*this);
   add_core_index< escrow_index                            >(*this);

   // Trading Indexes

   add_core_index< limit_order_index                       >(*this);
   add_core_index< margin_order_index                      >(*this);
   add_core_index< auction_order_index                     >(*this);
   add_core_index< call_order_index                        >(*this);
   add_core_index< option_order_index                      >(*this);

   // Pool Indexes

   add_core_index< asset_liquidity_pool_index              >(*this);
   add_core_index< asset_credit_pool_index                 >(*this);
   add_core_index< credit_collateral_index                 >(*this);
   add_core_index< credit_loan_index                       >(*this);
   add_core_index< asset_option_pool_index                 >(*this);
   add_core_index< asset_prediction_pool_index             >(*this);
   add_core_index< asset_prediction_pool_resolution_index  >(*this);
   
   // Asset Indexes

   add_core_index< asset_index                             >(*this);
   add_core_index< asset_dynamic_data_index                >(*this);
   add_core_index< asset_currency_data_index               >(*this);
   add_core_index< asset_reward_fund_index                 >(*this);
   add_core_index< asset_stablecoin_data_index             >(*this);
   add_core_index< asset_settlement_index                  >(*this);
   add_core_index< asset_collateral_bid_index              >(*this);
   add_core_index< asset_equity_data_index                 >(*this);
   add_core_index< asset_bond_data_index                   >(*this);
   add_core_index< asset_credit_data_index                 >(*this);
   add_core_index< asset_stimulus_data_index               >(*this);
   add_core_index< asset_unique_data_index                 >(*this);
   add_core_index< asset_distribution_index                >(*this);
   add_core_index< asset_distribution_balance_index        >(*this);

   // Block Producer Objects 

   add_core_index< producer_index                          >(*this);
   add_core_index< producer_schedule_index                 >(*this);
   add_core_index< producer_vote_index                     >(*this);
   add_core_index< block_validation_index                  >(*this);
   add_core_index< commit_violation_index                  >(*this);

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
      {
         FC_ASSERT( next_block.id() == itr->second, "Block did not match checkpoint", ("checkpoint",*itr)("block_id",next_block.id()) );
      }
         

      if( _checkpoints.rbegin()->first >= block_num )
      {
         skip = skip_producer_signature
              | skip_transaction_signatures
              | skip_transaction_dupe_check
              | skip_fork_db
              | skip_block_size_check
              | skip_tapos_check
              | skip_authority_check
              | skip_merkle_check //While blockchain is being downloaded, txs need to be validated against block headers
              | skip_undo_history_check
              | skip_producer_schedule_check
              | skip_validate
              | skip_validate_invariants
              ;
      }  
   }

   detail::with_skip_flags( *this, skip, [&]()
   {
      _apply_block( next_block );
   });

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
         uint64_t lep = block_num + 1 + _flush_blocks * 9 / 10;
         uint64_t rep = block_num + 1 + _flush_blocks;

         // use time_point::now() as RNG source to pick block randomly between lep and rep
         uint64_t span = rep - lep;
         uint64_t x = lep;
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
   uint64_t free_gb = uint64_t( get_free_memory() / (1024*1024*1024) );
   if( force || (free_gb < _last_free_gb_printed) || (free_gb > _last_free_gb_printed+1) )
   {
      ilog( "Free memory is now ${n} GB", ("n", free_gb) );
      _last_free_gb_printed = free_gb;
   }

   if( free_gb == 0 )
   {
      uint64_t free_mb = uint64_t( get_free_memory() / (1024*1024) );

      if( free_mb <= 50 && head_block_num() % 1000 == 0 )
      {
         elog( "Free memory is now ${n} MB. Shared Memory Capacity is insufficient, and may cause a node failure when depleted. Please increase shared file size.", 
         ("n", free_mb) );
      } 
   }
}

void database::_apply_block( const signed_block& next_block )
{ try {
   // ilog( "Begin Applying Block: ${b}", ("b", next_block.id() ) );
   notify_pre_apply_block( next_block );
   uint64_t next_block_num = next_block.block_num();
   uint32_t skip = get_node_properties().skip_flags;

   if( !( skip & skip_merkle_check ) )
   {
      auto merkle_root = next_block.calculate_merkle_root();

      try
      {
         FC_ASSERT( next_block.transaction_merkle_root == merkle_root, 
            "Merkle check failed", ("next_block.transaction_merkle_root",next_block.transaction_merkle_root)("calc",merkle_root)("next_block",next_block)("id",next_block.id()) );
      }
      catch( fc::assert_exception& e )
      {
         const auto& merkle_map = get_shared_db_merkle();
         auto itr = merkle_map.find( next_block_num );

         if( itr == merkle_map.end() || itr->second != merkle_root )
         {
            throw e;
         }  
      }
   }

   const producer_object& signing_producer = validate_block_header( skip, next_block );

   _current_block_num = next_block_num;
   _current_trx_in_block = 0;
   _current_trx_stake_weight = 0;

   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const median_chain_property_object& median_props = get_median_chain_properties();
   
   size_t block_size = fc::raw::pack_size( next_block );

   FC_ASSERT( block_size <= median_props.maximum_block_size, 
      "Block Size is too large.", 
      ("next_block_num",next_block_num)("block_size", block_size)("max",median_props.maximum_block_size ) );
   
   if( block_size < MIN_BLOCK_SIZE )
   {
      elog( "Block size is too small",
         ("next_block_num",next_block_num)("block_size", block_size)("min",MIN_BLOCK_SIZE)
      );
   }

   // Modify current producer so transaction evaluators can know who included the transaction,
   // this is mostly for POW operations which must pay the current_producer.

   modify( props, [&]( dynamic_global_property_object& dgp )
   {
      dgp.current_producer = next_block.producer;
   });

   process_header_extensions( next_block );     // parse producer version reporting

   const producer_object& producer = get_producer( next_block.producer );
   const hardfork_property_object& hardfork_state = get_hardfork_property_object();

   FC_ASSERT( producer.running_version >= hardfork_state.current_hardfork_version,
      "Block produced by producer: ${p} that is not running current hardfork.",
      ("p",producer)("next_block.producer",next_block.producer)("hardfork_state", hardfork_state)
   );
   
   /** 
    * We do not need to push the undo state for each transaction
    * because they either all apply and are valid or the
    * entire block fails to apply. We only need an "undo" state
    * for transactions when validating broadcast transactions or
    * when building a block.
    */
   for( const auto& trx : next_block.transactions )
   {
      apply_transaction( trx, skip );
      ++_current_trx_in_block;
   }

   update_global_dynamic_data( next_block );
   update_signing_producer( signing_producer, next_block );
   update_producer_schedule(*this);
   update_last_irreversible_block();
   update_transaction_stake( signing_producer, _current_trx_stake_weight );
   create_block_summary( next_block );

   clear_expired_transactions();
   clear_expired_operations();
   clear_expired_delegations();
   
   update_producer_set();
   governance_update_account_set();
   update_community_moderator_set();
   process_community_membership_fees();
   update_business_account_set();
   update_comment_metrics();
   update_message_counter();
   update_median_liquidity();
   update_proof_of_work_target();
   update_account_reputations();
   
   process_funds();

   process_asset_staking();
   process_stablecoins();
   process_savings_withdraws();
   process_recurring_transfers();
   process_equity_rewards();
   process_power_rewards();
   process_bond_interest();
   process_bond_assets();
   process_credit_updates();
   process_credit_buybacks();
   process_margin_updates();
   process_credit_interest();
   process_stimulus_assets();

   process_auction_orders();
   process_option_pools();
   process_prediction_pools();
   process_unique_assets();
   process_asset_distribution();
   process_product_auctions();

   process_membership_updates();
   process_txn_stake_rewards();
   process_validation_rewards();
   process_producer_activity_rewards();
   process_network_officer_rewards();
   process_executive_board_budgets();
   process_supernode_rewards();
   process_enterprise_fund();

   process_comment_cashout();
   
   account_recovery_processing();
   process_escrow_transfers();
   process_account_decline_voting();
   process_hardforks();

   notify_applied_block( next_block );      // notify observers that the block has been applied
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
            const auto& signing_producer = get_producer( next_block.producer );
            //idump( (next_block.producer)(signing_producer.running_version)(reported_version) );

            if( reported_version != signing_producer.running_version )
            {
               modify( signing_producer, [&]( producer_object& p )
               {
                  p.running_version = reported_version;
               });
            }
            break;
         }
         case 2: // hardfork_version vote
         {
            auto hfv = itr->get< hardfork_version_vote >();
            const auto& signing_producer = get_producer( next_block.producer );
            //idump( (next_block.producer)(signing_producer.running_version)(hfv) );

            if( hfv.hf_version != signing_producer.hardfork_version_vote || hfv.hf_time != signing_producer.hardfork_time_vote )
               modify( signing_producer, [&]( producer_object& p )
               {
                  p.hardfork_version_vote = hfv.hf_version;
                  p.hardfork_time_vote = hfv.hf_time;
               });

            break;
         }
         default:
            FC_ASSERT( false, "Unknown extension in block header" );
      }

      ++itr;
   }
}

void database::apply_transaction( const signed_transaction& trx, uint32_t skip )
{
   detail::with_skip_flags( *this, skip, [&]() { _apply_transaction(trx); });
   notify_on_applied_transaction( trx );
}

void database::_apply_transaction( const signed_transaction& trx )
{ try {
   _current_trx_id = trx.id();
   uint32_t skip = get_node_properties().skip_flags;

   if( !(skip&skip_validate) )
   {
      trx.validate();
   }

   auto& trx_idx = get_index< transaction_index >();
   const chain_id_type& chain_id = CHAIN_ID;
   auto trx_id = trx.id();

   // idump((trx_id)(skip&skip_transaction_dupe_check));

   FC_ASSERT( (skip & skip_transaction_dupe_check) ||
      trx_idx.indices().get< by_trx_id >().find( trx_id ) == trx_idx.indices().get< by_trx_id >().end(),
      "Duplicate transaction check failed", ("trx_ix", trx_id) );

   if( !(skip & (skip_transaction_signatures | skip_authority_check) ) )
   {
      auto get_active  = [&]( const string& name ) { return authority( get< account_authority_object, by_account >( name ).active_auth ); };
      auto get_owner   = [&]( const string& name ) { return authority( get< account_authority_object, by_account >( name ).owner_auth );  };
      auto get_posting = [&]( const string& name ) { return authority( get< account_authority_object, by_account >( name ).posting_auth );  };

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

   // Skip all manner of expiration and TaPoS checking if we're on block 1; It's impossible that the transaction is
   // expired, and TaPoS makes no sense as no blocks exist yet.
   if( BOOST_LIKELY( head_block_num() > 0 ) )
   {
      if( !(skip & skip_tapos_check) )
      {
         const auto& tapos_block_summary = get< block_summary_object >( trx.ref_block_num );

         // Verify TaPoS block summary has correct ID prefix, and that this block's time is not past the expiration

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
      create< transaction_object >([&]( transaction_object& transaction )
      {
         transaction.trx_id = trx_id;
         transaction.expiration = trx.expiration;
         fc::raw::pack( transaction.packed_trx, trx );
      });
   }

   notify_on_pre_apply_transaction( trx );

   _current_op_in_trx = 0;
   for( const auto& op : trx.operations )
   { 
      try 
      {
         apply_operation( op );   // process the operations
         ++_current_op_in_trx;
      } FC_CAPTURE_AND_RETHROW( (op) );
   }

   check_flash_loans();       // Ensure no unresolved flash loans.
   update_stake( trx );       // Apply stake weight to the block producer.
   _current_trx_id = transaction_id_type();

} FC_CAPTURE_AND_RETHROW( (trx) ) }


void database::update_stake( const signed_transaction& trx )
{
   if( trx.operations.size() )
   {
      flat_set< account_name_type > creators;
      for( const operation& op : trx.operations )
      {
         operation_creator_name( op, creators );
      }
      share_type voting_power;
      for( account_name_type name : creators )
      {
         voting_power += get_voting_power( name, SYMBOL_COIN );
      }
      size_t size = fc::raw::pack_size(trx);
      uint128_t stake_weight = approx_sqrt( uint128_t( ( voting_power.value / BLOCKCHAIN_PRECISION.value ) * size ) );
      _current_trx_stake_weight += stake_weight;
   }
}

/**
 * Decays and increments the current producer 
 * according to the stake weight of all the 
 * transactions in the block they have created.
 */
void database::update_transaction_stake( const producer_object& signing_producer, const uint128_t& transaction_stake )
{ try {
   const median_chain_property_object& median_props = get_median_chain_properties();
   const time_point now = head_block_time();
   fc::microseconds decay_time = median_props.txn_stake_decay_time;

   modify( signing_producer, [&]( producer_object& p )
   {
      p.recent_txn_stake_weight -= ( p.recent_txn_stake_weight * ( now - p.last_txn_stake_weight_update).to_seconds() ) / decay_time.to_seconds();
      p.recent_txn_stake_weight += transaction_stake;
      p.last_txn_stake_weight_update = now;
   });

} FC_CAPTURE_AND_RETHROW() }

void database::apply_operation(const operation& op)
{
   operation_notification note(op);
   notify_pre_apply_operation( note );
   _my->_evaluator_registry.get_evaluator( op ).apply( op );
   notify_post_apply_operation( note );
}

const producer_object& database::validate_block_header( uint32_t skip, const signed_block& next_block )const
{ try {
   FC_ASSERT( head_block_id() == next_block.previous,
      "Head Block ID must equal previous block header in new block.", 
      ("head_block_id",head_block_id())("next.prev",next_block.previous) );
   FC_ASSERT( head_block_time() < next_block.timestamp,
      "Head Block time must be less than timestamp of new block.",
      ("head_block_time",head_block_time())("next",next_block.timestamp)("blocknum",next_block.block_num()) );

   const producer_object& producer = get_producer( next_block.producer );

   if( !( skip&skip_producer_signature ) )
   {
      FC_ASSERT( next_block.validate_signee( producer.signing_key ) );
   }  

   if( !( skip&skip_producer_schedule_check ) )
   {
      uint64_t slot_num = get_slot_at_time( next_block.timestamp );
      FC_ASSERT( slot_num > 0,
         "slot number must be greater than 0." );

      string scheduled_producer = get_scheduled_producer( slot_num );

      FC_ASSERT( producer.owner == scheduled_producer, "producer produced block at wrong time",
         ("block producer",next_block.producer)("scheduled",scheduled_producer)("slot_num",slot_num) );
   }

   return producer;
} FC_CAPTURE_AND_RETHROW() }


void database::create_block_summary(const signed_block& next_block)
{ try {
   block_summary_id_type sid( next_block.block_num() & 0xffff );
   
   modify( get< block_summary_object >( sid ), [&]( block_summary_object& p ) 
   {
      p.block_id = next_block.id();
   });
} FC_CAPTURE_AND_RETHROW() }


void database::update_global_dynamic_data( const signed_block& b )
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   
   uint32_t missed_blocks = 0;
   price equity_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_EQUITY ).hour_median_price;
   price usd_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_USD ).hour_median_price;

   if( head_block_time() != fc::time_point() )
   {
      missed_blocks = get_slot_at_time( b.timestamp );
      assert( missed_blocks != 0 );
      missed_blocks--;
      for( uint32_t i = 0; i < missed_blocks; ++i )
      {
         const auto& producer_missed = get_producer( get_scheduled_producer( i + 1 ) );
         if( producer_missed.owner != b.producer )
         {
            modify( producer_missed, [&]( producer_object& p )
            {
               p.total_missed++;
               if( ( head_block_num() - p.last_confirmed_block_num ) > BLOCKS_PER_DAY )
               {
                  p.active = false;
                  push_virtual_operation( shutdown_producer_operation( p.owner ) );
               }
            } );
         }
      }
   }
   
   modify( props, [&]( dynamic_global_property_object& dgpo )  
   {
      // Dynamic global properties updating, constant time assuming 100% participation. It is O(B) otherwise (B = Num blocks between update)

      for( uint32_t i = 0; i < missed_blocks + 1; i++ )
      {
         dgpo.participation_count -= dgpo.recent_slots_filled.hi & 0x8000000000000000ULL ? 1 : 0;
         dgpo.recent_slots_filled = ( dgpo.recent_slots_filled << 1 ) + ( i == 0 ? 1 : 0 );
         dgpo.participation_count += ( i == 0 ? 1 : 0 );
      }

      dgpo.head_block_number = b.block_num();
      dgpo.head_block_id = b.id();
      dgpo.time = b.timestamp;
      dgpo.current_aslot += (missed_blocks+1);
      dgpo.current_median_equity_price = equity_price;
      dgpo.current_median_usd_price = usd_price;
   });

   // ilog( "Updated Global Dynamic Data: Head Block Number: ${n} Head block ID: ${i} ASlot: ${s} Time: ${t}", 
   // ("n", props.head_block_number)("i", props.head_block_id)("s",props.current_aslot)("t",props.time) );

   if( !( get_node_properties().skip_flags & skip_undo_history_check ) )
   {
      ASSERT( props.head_block_number - props.last_irreversible_block_num  < MAX_UNDO_HISTORY, undo_database_exception,
         "The database does not have enough undo history to support a blockchain with so many missed blocks. "
         "Please add a checkpoint if you would like to continue applying blocks beyond this point.",
         ("last_irreversible_block_num",props.last_irreversible_block_num)("head", props.head_block_number)
         ("max_undo",MAX_UNDO_HISTORY) );
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the last irreversible and last committed block numbers and IDs,
 * enabling nodes to add the block history to their block logs, when consensus finality 
 * is achieved by block producers.
 */
void database::update_last_irreversible_block()
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const producer_schedule_object& pso = get_producer_schedule();

   vector< const producer_object* > producer_objs;

   producer_objs.reserve( pso.num_scheduled_producers );

   for( size_t i = 0; i < pso.current_shuffled_producers.size(); i++ )
   {
      producer_objs.push_back( &get_producer( pso.current_shuffled_producers[ i ] ) );
   }

   static_assert( IRREVERSIBLE_THRESHOLD > 0, "irreversible threshold must be nonzero" );

   // 1 1 1 2 2 2 2 2 2 2 -> 2     .7*10 = 7
   // 1 1 1 1 1 1 1 2 2 2 -> 1
   // 3 3 3 3 3 3 3 3 3 3 -> 3

   size_t offset = ( ( PERCENT_100 - IRREVERSIBLE_THRESHOLD ) * producer_objs.size() / PERCENT_100 );

   std::nth_element( producer_objs.begin(), producer_objs.begin() + offset, producer_objs.end(),
      []( const producer_object* a, const producer_object* b )
      {
         return a->last_confirmed_block_num < b->last_confirmed_block_num;
      });

   uint64_t new_last_irreversible_block_num = producer_objs[offset]->last_confirmed_block_num;

   std::nth_element( producer_objs.begin(), producer_objs.begin() + offset, producer_objs.end(),
      []( const producer_object* a, const producer_object* b )
      {
         return a->last_commit_height < b->last_commit_height;
      });

   uint64_t new_last_committed_block_num = producer_objs[offset]->last_commit_height;

   if( new_last_irreversible_block_num > props.last_irreversible_block_num )
   {
      block_id_type irreversible_id = get_block_id_for_num( new_last_irreversible_block_num );

      modify( props, [&]( dynamic_global_property_object& d )
      {
         d.last_irreversible_block_num = new_last_irreversible_block_num;
         d.last_irreversible_block_id = irreversible_id;
      });
   }

   if( new_last_committed_block_num > props.last_committed_block_num )
   {
      block_id_type commit_id = get_block_id_for_num( new_last_committed_block_num );

      modify( props, [&]( dynamic_global_property_object& d )
      {
         d.last_committed_block_num = new_last_committed_block_num;
         d.last_committed_block_id = commit_id;
      });
   }

   // Take the highest of last committed and irreversible blocks, and commit it to the local database.
   uint64_t commit_height = std::max( props.last_committed_block_num, props.last_irreversible_block_num );
   
   commit( commit_height );  // Node will not reverse blocks after they have been committed or produced on by two thirds of producers.

   if( !( get_node_properties().skip_flags & skip_block_log ) )  // Output to block log based on new committed and last irreversiible block numbers.
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

   _fork_db.set_max_size( props.head_block_number - commit_height + 1 );

} FC_CAPTURE_AND_RETHROW() }


void database::init_hardforks()
{
   _hardfork_times[ 0 ] = fc::time_point( GENESIS_TIME );
   _hardfork_versions[ 0 ] = hardfork_version( 0, 0 );
   // FC_ASSERT( HARDFORK_0_1 == 1, "Invalid hardfork configuration" );
   // _hardfork_times[ HARDFORK_0_1 ] = fc::time_point( HARDFORK_0_1_TIME );
   // _hardfork_versions[ HARDFORK_0_1 ] = HARDFORK_0_1_VERSION;

   const auto& hardforks = get_hardfork_property_object();
   FC_ASSERT( hardforks.last_hardfork <= NUM_HARDFORKS,
      "Chain knows of more hardforks than configuration.", ("hardforks.last_hardfork",hardforks.last_hardfork)("NUM_HARDFORKS",NUM_HARDFORKS) );
   FC_ASSERT( _hardfork_versions[ hardforks.last_hardfork ] <= BLOCKCHAIN_VERSION,
      "Blockchain version is older than last applied hardfork." );
   FC_ASSERT( BLOCKCHAIN_HARDFORK_VERSION == _hardfork_versions[ NUM_HARDFORKS ] );
}


/**
 * Expire all orders that have exceeded their expiration time.
 */
void database::clear_expired_operations()
{ try {
   time_point now = head_block_time();

   const auto& limit_index = get_index< limit_order_index >().indices().get< by_expiration >();
   while( !limit_index.empty() && limit_index.begin()->expiration <= now )
   {
      const limit_order_object& order = *limit_index.begin();
      cancel_limit_order( order );
   }

   const auto& margin_index = get_index< margin_order_index >().indices().get< by_expiration >();
   while( !margin_index.empty() && margin_index.begin()->expiration <= now )
   {
      const margin_order_object& order = *margin_index.begin();
      close_margin_order( order );
   }

   const auto& transfer_req_index = get_index< transfer_request_index >().indices().get< by_expiration >();
   while( !transfer_req_index.empty() && transfer_req_index.begin()->expiration <= now )
   {
      const transfer_request_object& req = *transfer_req_index.begin();
      ilog( "Removed: ${v}",("v",req));
      remove( req );
   }

   const auto& transfer_rec_index = get_index< transfer_recurring_request_index >().indices().get< by_expiration >();
   while( !transfer_rec_index.empty() && transfer_rec_index.begin()->expiration <= now )
   {
      const transfer_recurring_request_object& rec = *transfer_rec_index.begin();
      ilog( "Removed: ${v}",("v",rec));
      remove( rec );
   }

   const auto& account_member_request_idx = get_index< account_member_request_index >().indices().get< by_expiration >();
   while( !account_member_request_idx.empty() && account_member_request_idx.begin()->expiration <= now )
   {
      const account_member_request_object& req = *account_member_request_idx.begin();
      ilog( "Removed: ${v}",("v",req));
      remove( req );
   }

   const auto& account_member_invite_idx = get_index< account_member_invite_index >().indices().get< by_expiration >();
   while( !account_member_invite_idx.empty() && account_member_invite_idx.begin()->expiration <= now )
   {
      const account_member_invite_object& inv = *account_member_invite_idx.begin();
      ilog( "Removed: ${v}",("v",inv));
      remove( inv );
   }

   const auto& community_join_request_idx = get_index< community_join_request_index >().indices().get< by_expiration >();
   while( !community_join_request_idx.empty() && community_join_request_idx.begin()->expiration <= now )
   {
      const community_join_request_object& req = *community_join_request_idx.begin();
      ilog( "Removed: ${v}",("v",req));
      remove( req );
   }

   const auto& community_join_invite_idx = get_index< community_join_invite_index >().indices().get< by_expiration >();
   while( !community_join_invite_idx.empty() && community_join_invite_idx.begin()->expiration <= now )
   {
      const community_join_invite_object& inv = *community_join_invite_idx.begin();
      ilog( "Removed: ${v}",("v",inv));
      remove( inv );
   }

   const auto& bid_idx = get_index< ad_bid_index >().indices().get< by_expiration >();
   while( !bid_idx.empty() && bid_idx.begin()->expiration <= now )
   {
      const ad_bid_object& bid = *bid_idx.begin();
      cancel_ad_bid( bid );
   }

   const auto& premium_purchase_idx = get_index< premium_purchase_index >().indices().get< by_expiration >();
   while( !premium_purchase_idx.empty() && premium_purchase_idx.begin()->expiration <= now )
   {
      const premium_purchase_object& premium_purchase = *premium_purchase_idx.begin();
      cancel_premium_purchase( premium_purchase );
   }

   // Process expired force settlement orders
   const auto& settlement_index = get_index< asset_settlement_index >().indices().get< by_expiration >();
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
      for( auto itr = settlement_index.lower_bound( current_asset );
         itr != settlement_index.end();
         itr = settlement_index.lower_bound( current_asset ) )
      {
         ++count;
         const asset_settlement_object& order = *itr;
         auto order_id = order.id;
         current_asset = order.settlement_asset_symbol();
         const asset_object& mia_object = get_asset( current_asset );
         const asset_stablecoin_data_object& mia_stablecoin = get_stablecoin_data( mia_object.symbol );

         extra_dump = ( (count >= 1000) && (count <= 1020) );

         if( extra_dump )
         {
            wlog( "clear_expired_operations() dumping extra data for iteration ${c}", ("c", count) );
            ilog( "head_block_num is ${hb} current_asset is ${a}", ("hb", head_block_num())("a", current_asset) );
         }

         if( mia_stablecoin.has_settlement() )
         {
            ilog( "Canceling a force settlement because of black swan" );
            cancel_settle_order( order );
            continue;
         }

         // Has this order not reached its settlement date?
         if( order.settlement_date > now )
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
         
         if( mia_stablecoin.current_feed.settlement_price.is_null() )
         {
            ilog("Canceling a force settlement in ${asset} because settlement price is null",
                 ("asset", mia_object.symbol));

            cancel_settle_order( order );
            continue;
         }
         
         if( max_settlement_volume.symbol != current_asset ) // only calculate once per asset
         {  
            const asset_dynamic_data_object& dyn_data = get_dynamic_data( mia_object.symbol );
            max_settlement_volume = asset( mia_stablecoin.max_asset_settlement_volume(dyn_data.get_total_supply().amount), mia_object.symbol );
         }

         if( mia_stablecoin.force_settled_volume >= max_settlement_volume.amount || current_asset_finished )
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
            uint16_t offset = mia_stablecoin.asset_settlement_offset_percent;
            settlement_fill_price = mia_stablecoin.current_feed.settlement_price / ratio_type( PERCENT_100 - offset, PERCENT_100 );
         }
            
         if( settlement_price.base.symbol != current_asset )  // only calculate once per asset
         {
            settlement_price = settlement_fill_price;
         }

         auto& call_index = get_index< call_order_index >().indices().get< by_collateral >();
         asset settled = asset( mia_stablecoin.force_settled_volume , mia_object.symbol);
         // Match against the least collateralized short until the settlement is finished or we reach max settlements
         while( settled < max_settlement_volume && find(order_id) )
         {
            auto itr = call_index.lower_bound( boost::make_tuple( price::min( mia_stablecoin.backing_asset, mia_object.symbol )));
            // There should always be a call order, since asset exists
            FC_ASSERT( itr != call_index.end() && 
               itr->debt_type() == mia_object.symbol, 
               "Call order asset must be the same as market issued asset." );
            asset max_settlement = max_settlement_volume - settled;

            if( order.balance.amount == 0 )
            {
               wlog( "0 settlement detected" );
               cancel_settle_order( order );
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
         if( mia_stablecoin.force_settled_volume != settled.amount )
         {
            modify(mia_stablecoin, [settled](asset_stablecoin_data_object& b) 
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
         if( hardforks.last_hardfork < NUM_HARDFORKS ) 
         {
            apply_hardfork( hardforks.last_hardfork + 1 );
         }
         else
         {
            throw unknown_hardfork_exception();
         }
      }

      // ilog( "Processed Hardforks" );
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
 * Look for expired transactions in the deduplication list, and remove them.
 * Transactions must have expired by at least two forking windows in order to be removed.
 */
void database::clear_expired_transactions()
{
   // ilog( "Clear Expired Transactions." );

   auto& transaction_idx = get_index< transaction_index >();
   const auto& dedupe_index = transaction_idx.indices().get< by_expiration >();
   while( ( !dedupe_index.empty() ) && ( head_block_time() > dedupe_index.begin()->expiration ) )
   {
      const transaction_object& txn = *dedupe_index.begin();
      // ilog( "Removed: ${v}",("v",txn));
      remove( txn );
   }
}


/**
 * Verifies all supply invariants.
 * 
 * Ensures that the Supply of every asset is equal to the amount of the asset in account
 * balances
 */
void database::validate_invariants()const
{ try {
   // ilog( "Validate Invariants" );
   const auto& asset_idx = get_index< asset_dynamic_data_index >().indices().get< by_symbol >();
   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol >();
   
   auto asset_itr = asset_idx.begin();
   while( asset_itr != asset_idx.end() )
   {
      const asset_dynamic_data_object& addo = *asset_itr;
      asset total_account_balance_supply =  addo.get_account_balance_supply();
      
      FC_ASSERT( addo.get_delegated_supply() == addo.get_receiving_supply(),
         "Asset Supply error: Delegated supply not equal to receiving supply",
         ("Asset", addo.symbol ) );

      asset total_account_balances = asset( 0, addo.symbol );

      auto balance_itr = balance_idx.lower_bound( addo.symbol );

      while( balance_itr != balance_idx.end() && 
         balance_itr->symbol == addo.symbol )
      {
         const account_balance_object& abo = *balance_itr;
         total_account_balances += abo.get_total_balance();
         ++balance_itr;
      }

      FC_ASSERT( total_account_balances == total_account_balance_supply,
         "Account Balance Error: Balance of asset ${s} account balance sum: ${b} not equal to total account balance supply: ${t}.", 
         ("s", addo.symbol)("b", total_account_balances)("t", total_account_balance_supply) );

      ++asset_itr;
   }

} FC_CAPTURE_LOG_AND_RETHROW( (head_block_num()) ) }


} } // node::chain