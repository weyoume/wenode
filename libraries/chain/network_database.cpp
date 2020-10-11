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

const network_officer_object& database::get_network_officer( const account_name_type& account )const
{ try {
	return get< network_officer_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const network_officer_object* database::find_network_officer( const account_name_type& account )const
{
   return find< network_officer_object, by_account >( account );
}

const network_officer_vote_object& database::get_network_officer_vote( const account_name_type& account, const account_name_type& officer )const
{ try {
   return get< network_officer_vote_object, by_account_officer >( boost::make_tuple( account, officer ) );
} FC_CAPTURE_AND_RETHROW( (account)(officer) ) }

const network_officer_vote_object* database::find_network_officer_vote( const account_name_type& account, const account_name_type& officer )const
{
   return find< network_officer_vote_object, by_account_officer >( boost::make_tuple( account, officer ) );
}

const executive_board_object& database::get_executive_board( const account_name_type& account )const
{ try {
	return get< executive_board_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const executive_board_object* database::find_executive_board( const account_name_type& account )const
{
   return find< executive_board_object, by_account >( account );
}

const executive_board_vote_object& database::get_executive_board_vote( const account_name_type& account, const account_name_type& executive )const
{ try {
   return get< executive_board_vote_object, by_account_executive >( boost::make_tuple( account, executive ) );
} FC_CAPTURE_AND_RETHROW( (account)(executive) ) }

const executive_board_vote_object* database::find_executive_board_vote( const account_name_type& account, const account_name_type& executive )const
{
   return find< executive_board_vote_object, by_account_executive >( boost::make_tuple( account, executive ) );
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

const mediator_object& database::get_mediator( const account_name_type& account )const
{ try {
	return get< mediator_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const mediator_object* database::find_mediator( const account_name_type& account )const
{
   return find< mediator_object, by_account >( account );
}

const governance_account_object& database::get_governance_account( const account_name_type& account )const
{ try {
	return get< governance_account_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const governance_account_object* database::find_governance_account( const account_name_type& account )const
{
   return find< governance_account_object, by_account >( account );
}

const enterprise_object& database::get_enterprise( const account_name_type& account, const shared_string& enterprise_id )const
{ try {
   return get< enterprise_object, by_enterprise_id >( boost::make_tuple( account, enterprise_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(enterprise_id) ) }

const enterprise_object* database::find_enterprise( const account_name_type& account, const shared_string& enterprise_id )const
{
   return find< enterprise_object, by_enterprise_id >( boost::make_tuple( account, enterprise_id ) );
}

const enterprise_object& database::get_enterprise( const account_name_type& account, const string& enterprise_id )const
{ try {
   return get< enterprise_object, by_enterprise_id >( boost::make_tuple( account, enterprise_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(enterprise_id) ) }

const enterprise_object* database::find_enterprise( const account_name_type& account, const string& enterprise_id )const
{
   return find< enterprise_object, by_enterprise_id >( boost::make_tuple( account, enterprise_id ) );
}

const enterprise_vote_object& database::get_enterprise_vote( const account_name_type& account, const shared_string& enterprise_id, const account_name_type& voter )const
{ try {
   return get< enterprise_vote_object, by_enterprise_id >( boost::make_tuple( account, enterprise_id, voter ) );
} FC_CAPTURE_AND_RETHROW( (account)(enterprise_id)(voter) ) }

const enterprise_vote_object* database::find_enterprise_vote( const account_name_type& account, const shared_string& enterprise_id, const account_name_type& voter )const
{
   return find< enterprise_vote_object, by_enterprise_id >( boost::make_tuple( account, enterprise_id, voter ) );
}

const enterprise_vote_object& database::get_enterprise_vote( const account_name_type& account, const string& enterprise_id, const account_name_type& voter )const
{ try {
   return get< enterprise_vote_object, by_enterprise_id >( boost::make_tuple( account, enterprise_id, voter ) );
} FC_CAPTURE_AND_RETHROW( (account)(enterprise_id)(voter) ) }

const enterprise_vote_object* database::find_enterprise_vote( const account_name_type& account, const string& enterprise_id, const account_name_type& voter )const
{
   return find< enterprise_vote_object, by_enterprise_id >( boost::make_tuple( account, enterprise_id, voter ) );
}


/**
 * Aligns network_officer votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::network_officer_update_votes( const account_object& account )
{
   const auto& vote_idx = get_index< network_officer_vote_index >().indices().get< by_account_type_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );
   time_point now = head_block_time();

   flat_map< network_officer_role_type, uint16_t > vote_rank;
   vote_rank[ network_officer_role_type::DEVELOPMENT ] = 1;
   vote_rank[ network_officer_role_type::MARKETING ] = 1;
   vote_rank[ network_officer_role_type::ADVOCACY ] = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const network_officer_vote_object& vote = *vote_itr;
      if( vote.vote_rank != vote_rank[ vote.officer_type ] )
      {
         modify( vote, [&]( network_officer_vote_object& v )
         {
            v.vote_rank = vote_rank[ vote.officer_type ];   // Updates vote rank to linear order of index retrieval.
            v.last_updated = now;
         });
      }
      vote_rank[ vote.officer_type ]++;
      ++vote_itr;
   }

   modify( account, [&]( account_object& a )
   {
      a.officer_vote_count = ( vote_rank[ network_officer_role_type::DEVELOPMENT ] + vote_rank[ network_officer_role_type::MARKETING ] + vote_rank[ network_officer_role_type::ADVOCACY ] - 3 );
   });
}

/**
 * Aligns network_officer votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::network_officer_update_votes( const account_object& account, const account_name_type& network_officer, 
   network_officer_role_type officer_type, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< network_officer_vote_index >().indices().get< by_account_type_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );
   time_point now = head_block_time();

   flat_map< network_officer_role_type, uint16_t > vote_rank;
   vote_rank[ network_officer_role_type::DEVELOPMENT ] = 1;
   vote_rank[ network_officer_role_type::MARKETING ] = 1;
   vote_rank[ network_officer_role_type::ADVOCACY ] = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const network_officer_vote_object& vote = *vote_itr;

      if( vote.vote_rank == input_vote_rank && vote.officer_type == officer_type )
      {
         vote_rank[ vote.officer_type ]++;
      }
      if( vote.vote_rank != vote_rank[ vote.officer_type ] )
      {
         modify( vote, [&]( network_officer_vote_object& v )
         {
            v.vote_rank = vote_rank[ vote.officer_type ];   // Updates vote rank to linear order of index retrieval.
            v.last_updated = now;
         });
      }
      vote_rank[ vote.officer_type ]++;
      ++vote_itr;
   }

   create< network_officer_vote_object >([&]( network_officer_vote_object& v )
   {
      v.account = account.name;
      v.network_officer = network_officer;
      v.officer_type = officer_type;
      v.vote_rank = input_vote_rank;
      v.last_updated = now;
      v.created = now;
   });

   modify( account, [&]( account_object& a )
   {
      a.officer_vote_count = ( vote_rank[ network_officer_role_type::DEVELOPMENT ] + vote_rank[ network_officer_role_type::MARKETING ] + vote_rank[ network_officer_role_type::ADVOCACY ] - 3 );
   });
}


/**
 * Aligns executive board votes in order of highest to lowest,
 * with continual ordering.
 */
void database::update_executive_board_votes( const account_object& account )
{
   const auto& vote_idx = get_index< executive_board_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );
   time_point now = head_block_time();

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const executive_board_vote_object& vote = *vote_itr;
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( executive_board_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
            v.last_updated = now;
         });
      }
      ++vote_itr;
      new_vote_rank++;
   }

   modify( account, [&]( account_object& a )
   {
      a.executive_board_vote_count = ( new_vote_rank - 1 );
   });
}

/**
 * Aligns executive board votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_executive_board_votes( const account_object& account, const account_name_type& executive, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< executive_board_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );
   time_point now = head_block_time();

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const executive_board_vote_object& vote = *vote_itr;
      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( executive_board_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
            v.last_updated = now;
         });
      }

      new_vote_rank++;
      ++vote_itr;
   }

   create< executive_board_vote_object >([&]( executive_board_vote_object& v )
   {
      v.account = account.name;
      v.executive_board = executive;
      v.vote_rank = input_vote_rank;
      v.last_updated = now;
      v.created = now;
   });

   modify( account, [&]( account_object& a )
   {
      a.executive_board_vote_count = ( new_vote_rank - 1 );
   });
}



/**
 * Aligns enterprise approval votes in order of highest to lowest,
 * with continual ordering.
 */
void database::update_enterprise_votes( const account_object& account )
{
   const auto& vote_idx = get_index< enterprise_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );
   time_point now = head_block_time();

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name )
   {
      const enterprise_vote_object& vote = *vote_itr;
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( enterprise_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
            v.last_updated = now;
         });
      }
      ++vote_itr;
      new_vote_rank++;
   }
}


/**
 * Aligns enterprise approval votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_enterprise_votes( const account_object& account, const account_name_type& voter, string enterprise_id, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< enterprise_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, 1 ) );
   time_point now = head_block_time();

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name )
   {
      const enterprise_vote_object& vote = *vote_itr;
      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( enterprise_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
            v.last_updated = now;
         });
      }
      new_vote_rank++;
      ++vote_itr;
   }

   create< enterprise_vote_object >([&]( enterprise_vote_object& v )
   {
      v.voter = voter;
      v.account = account.name;
      from_string( v.enterprise_id, enterprise_id );
      v.vote_rank = input_vote_rank;
      v.created = now;
      v.last_updated = now;
   });
}


void database::adjust_view_weight( const supernode_object& supernode, share_type delta, bool adjust = true )
{ try {
   const median_chain_property_object& median_props = get_median_chain_properties();
   time_point now = head_block_time();

   modify( supernode, [&]( supernode_object& s )
   {
      s.decay_weights( median_props, now );
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
   time_point now = head_block_time();
   modify( interface, [&]( interface_object& i )
   {
      i.decay_weights( now );
      if( adjust )
      {
         i.daily_active_users += PERCENT_100;
         i.monthly_active_users += PERCENT_100;
      }
   });
} FC_CAPTURE_AND_RETHROW() }



/**
 * Update a network officer's voting approval statisitics
 * and updates its approval if there are
 * sufficient votes from producers and other accounts.
 */
void database::network_officer_update( const network_officer_object& network_officer, 
   const producer_schedule_object& pso, const dynamic_global_property_object& props )
{ try {
   uint32_t vote_count = 0;
   share_type voting_power = 0;
   uint32_t producer_vote_count = 0;
   share_type producer_voting_power = 0;
   const asset_currency_data_object& currency = get_currency_data( network_officer.reward_currency );
   price equity_price = get_liquidity_pool( network_officer.reward_currency, currency.equity_asset ).hour_median_price;

   const auto& vote_idx = get_index< network_officer_vote_index >().indices().get< by_officer_account >();
   auto vote_itr = vote_idx.lower_bound( network_officer.account );

   while( vote_itr != vote_idx.end() && 
      vote_itr->network_officer == network_officer.account )
   {
      const network_officer_vote_object& vote = *vote_itr;
      const account_object& voter = get_account( vote.account );
      bool is_producer = pso.is_top_voting_producer( voter.name );
      vote_count++;
      share_type weight = 0;
      weight += get_voting_power( vote.account, equity_price );

      if( voter.proxied.size() )
      {
         weight += get_proxied_voting_power( voter, equity_price );
      }

      voting_power += share_type( weight.value >> vote.vote_rank );

      if( is_producer )
      {
         producer_vote_count++;
         const producer_object& producer = get_producer( voter.name );
         producer_voting_power += share_type( producer.voting_power.value >> vote.vote_rank );
      }
      ++vote_itr;
   }

   // Approve the network officer when a threshold of voting power and vote amount supports it.
   bool approve_officer = ( vote_count >= OFFICER_VOTE_THRESHOLD_AMOUNT ) &&
      ( producer_vote_count >= OFFICER_VOTE_THRESHOLD_PRODUCERS ) &&
      ( voting_power.value >= ( props.total_voting_power * OFFICER_VOTE_THRESHOLD_PERCENT ) / PERCENT_100 ) &&
      ( producer_voting_power.value >= ( pso.total_producer_voting_power * OFFICER_VOTE_THRESHOLD_PERCENT ) / PERCENT_100 );
   
   modify( network_officer, [&]( network_officer_object& noo )
   {
      noo.vote_count = vote_count;
      noo.voting_power = voting_power;
      noo.producer_vote_count = producer_vote_count;
      noo.producer_voting_power = producer_voting_power;
      noo.officer_approved = approve_officer;
   });

   ilog( "Updated Network Officer: ${n} Vote count: ${c} Approved: ${a}",
      ("n",network_officer.account)("c",vote_count)("a",approve_officer) );
   
} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays the network officer rewards to the 50 highest voted
 * developers, marketers and advocates on the network from
 * all currency asset reward funds once per day.
 */
void database::process_network_officer_rewards()
{ try {
   if( (head_block_num() % NETWORK_OFFICER_BLOCK_INTERVAL ) != 0 )    // Runs once per day.
      return;

   // ilog( "Process Network Officer Rewards" );

   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const producer_schedule_object& pso = get_producer_schedule();
   const auto& officer_idx = get_index< network_officer_index >().indices().get< by_symbol_type_voting_power >();
   auto officer_itr = officer_idx.begin();

   while( officer_itr != officer_idx.end() ) 
   {
      network_officer_update( *officer_itr, pso, props );
      ++officer_itr;
   }

   const auto& fund_idx = get_index< asset_reward_fund_index >().indices().get< by_symbol >();
   auto fund_itr = fund_idx.begin();

   while( fund_itr != fund_idx.end() )
   {
      const asset_reward_fund_object& reward_fund = *fund_itr;
      ++fund_itr;

      // ========== Development Officers ========== //

      auto development_itr = officer_idx.lower_bound( boost::make_tuple( reward_fund.symbol, network_officer_role_type::DEVELOPMENT ) );
      auto development_end = officer_idx.upper_bound( boost::make_tuple( reward_fund.symbol, network_officer_role_type::DEVELOPMENT ) );
      flat_map < account_name_type, share_type > development_map;
      share_type total_development_shares = 0;

      while( development_itr != development_end && 
         development_map.size() < NETWORK_OFFICER_ACTIVE_SET ) 
      {
         share_type development_shares = development_itr->voting_power;  // Get the development officer voting power

         if( development_shares > 0 && 
            development_itr->active && 
            development_itr->officer_approved )
         {
            total_development_shares += development_shares;
            development_map[ development_itr->account ] = development_shares;
         }
         ++development_itr;
      }

      // ========== Marketing Officers ========== //

      auto marketing_itr = officer_idx.lower_bound( boost::make_tuple( reward_fund.symbol, network_officer_role_type::MARKETING ) );
      auto marketing_end = officer_idx.upper_bound( boost::make_tuple( reward_fund.symbol, network_officer_role_type::MARKETING ) );
      flat_map < account_name_type, share_type > marketing_map;
      share_type total_marketing_shares = 0;
      
      while( marketing_itr != marketing_end && 
         marketing_map.size() < NETWORK_OFFICER_ACTIVE_SET ) 
      {
         share_type marketing_shares = marketing_itr->voting_power;  // Get the marketing officer voting power

         if( marketing_shares > 0 && 
            marketing_itr->active && 
            marketing_itr->officer_approved )
         {
            total_marketing_shares += marketing_shares;
            marketing_map[ marketing_itr->account ] = marketing_shares;
         }
         ++marketing_itr;
      }

      // ========== Advocacy Officers ========== //

      auto advocacy_itr = officer_idx.lower_bound( boost::make_tuple( reward_fund.symbol, network_officer_role_type::ADVOCACY ) );
      auto advocacy_end = officer_idx.upper_bound( boost::make_tuple( reward_fund.symbol, network_officer_role_type::ADVOCACY ) );
      flat_map < account_name_type, share_type > advocacy_map;
      share_type total_advocacy_shares = 0;
      
      while( advocacy_itr != advocacy_end && 
         advocacy_map.size() < NETWORK_OFFICER_ACTIVE_SET ) 
      {
         share_type advocacy_shares = advocacy_itr->voting_power;  // Get the advocacy officer voting power

         if( advocacy_shares > 0 && 
            advocacy_itr->active && 
            advocacy_itr->officer_approved )
         {
            total_advocacy_shares += advocacy_shares;
            advocacy_map[ advocacy_itr->account ] = advocacy_shares;
         }
         ++advocacy_itr;
      }

      asset development_reward = reward_fund.development_reward_balance;
      asset development_distributed = asset( 0, reward_fund.symbol );
      asset marketing_reward = reward_fund.marketing_reward_balance;
      asset marketing_distributed = asset( 0, reward_fund.symbol );
      asset advocacy_reward = reward_fund.advocacy_reward_balance;
      asset advocacy_distributed = asset( 0, reward_fund.symbol );

      for( auto b : development_map )
      {
         asset development_reward_split = ( development_reward * b.second ) / total_development_shares;
         adjust_reward_balance( b.first, development_reward_split );
         development_distributed += development_reward_split;
      }

      for( auto b : marketing_map )
      {
         asset marketing_reward_split = ( marketing_reward * b.second ) / total_marketing_shares;
         adjust_reward_balance( b.first, marketing_reward_split );
         marketing_distributed += marketing_reward_split;
      }

      for( auto b : advocacy_map )
      {
         asset advocacy_reward_split = ( advocacy_reward * b.second ) / total_advocacy_shares;
         adjust_reward_balance( b.first, advocacy_reward_split );
         advocacy_distributed += advocacy_reward_split;
      }

      modify( reward_fund, [&]( asset_reward_fund_object& r )
      {
         r.adjust_development_reward_balance( -development_distributed );   
         r.adjust_marketing_reward_balance( -marketing_distributed );   
         r.adjust_advocacy_reward_balance( -advocacy_distributed );  
      });

      asset total_distributed = development_distributed + marketing_distributed + advocacy_distributed;
      adjust_pending_supply( -total_distributed );   // Deduct distributed amount from pending supply.
   }
} FC_CAPTURE_AND_RETHROW() }



/**
 * Distributes Supernode rewards between all supernodes according to
 * stake weighted views on posts.
 */
void database::process_supernode_rewards()
{ try {
   if( (head_block_num() % SUPERNODE_BLOCK_INTERVAL ) != 0 )    // Runs once per day.
      return;

   ilog( "Process Supernode Rewards" );

   time_point now = head_block_time();
   const auto& supernode_idx = get_index< supernode_index >().indices().get< by_view_weight >();
   const auto& sn_acc_idx = get_index< supernode_index >().indices().get< by_account >();
   auto supernode_itr = supernode_idx.begin();
   flat_map < account_name_type, share_type > supernode_map;
   share_type total_supernode_shares = 0;
   
   while( supernode_itr != supernode_idx.end() ) 
   {
      share_type supernode_shares = supernode_itr->recent_view_weight;  // Get the supernode view weight for rewards

      if( supernode_shares > 0 && 
         supernode_itr->active && 
         now > ( supernode_itr->last_activation_time + fc::days(1) ) )
      {
         total_supernode_shares += supernode_shares;
         supernode_map[ supernode_itr->account ] = supernode_shares;
      }
      ++supernode_itr;
   }

   const auto& fund_idx = get_index< asset_reward_fund_index >().indices().get< by_symbol >();
   auto fund_itr = fund_idx.begin();

   while( fund_itr != fund_idx.end() && total_supernode_shares > 0 )
   {
      const asset_reward_fund_object& reward_fund = *fund_itr;
      asset supernode_reward = reward_fund.supernode_reward_balance;     // Record the opening balance of the supernode reward fund
      asset distributed = asset( 0, reward_fund.symbol );

      for( auto b : supernode_map )
      {
         asset supernode_reward_split = ( supernode_reward * b.second ) / total_supernode_shares; 
         adjust_reward_balance( b.first, supernode_reward_split );       // Pay supernode reward proportionally with view weight.
         auto sn_ptr = sn_acc_idx.find( b.first );
         if( sn_ptr != sn_acc_idx.end() )
         {
            modify( *sn_ptr, [&]( supernode_object& s )
            {
               s.storage_rewards += supernode_reward_split;     // Increment the lifetime storage earnings of the supernode
            }); 
         }
         distributed += supernode_reward_split;
      }

      modify( reward_fund, [&]( asset_reward_fund_object& r )
      {
         r.adjust_supernode_reward_balance( -distributed );     // Remove the distributed amount from the reward pool.
      });

      adjust_pending_supply( -distributed );               // Deduct distributed amount from pending supply.

      ++fund_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Update an executive board's voting approval statisitics
 * and update its approval if there are
 * sufficient votes from producers and other accounts.
 */
void database::update_executive_board( const executive_board_object& executive_board, 
   const producer_schedule_object& pso, const dynamic_global_property_object& props )
{ try {
   uint32_t vote_count = 0;
   share_type voting_power = 0;
   uint32_t producer_vote_count = 0;
   share_type producer_voting_power = 0;
   price equity_price = get_liquidity_pool(SYMBOL_COIN, SYMBOL_EQUITY).hour_median_price;

   const auto& vote_idx = get_index< executive_board_vote_index >().indices().get< by_executive_account >();
   auto vote_itr = vote_idx.lower_bound( executive_board.account );

   while( vote_itr != vote_idx.end() && 
      vote_itr->executive_board == executive_board.account )
   {
      const executive_board_vote_object& vote = *vote_itr;
      const account_object& voter = get_account( vote.account );
      bool is_producer = pso.is_top_voting_producer( voter.name );
      vote_count++;
      share_type weight = 0;
      weight += get_voting_power( vote.account, equity_price );
      if( voter.proxied.size() )
      {
         weight += get_proxied_voting_power( voter, equity_price );
      }
      voting_power += share_type( weight.value >> vote.vote_rank );

      if( is_producer )
      {
         producer_vote_count++;
         const producer_object& producer = get_producer( voter.name );
         producer_voting_power += share_type( producer.voting_power.value >> vote.vote_rank );
      }
      ++vote_itr;
   }

   // Approve the executive board when a threshold of accounts vote to support its budget.
   bool approve_board = ( vote_count >= EXECUTIVE_VOTE_THRESHOLD_AMOUNT ) &&
      ( producer_vote_count >= EXECUTIVE_VOTE_THRESHOLD_PRODUCERS ) &&
      ( voting_power.value >= ( props.total_voting_power * EXECUTIVE_VOTE_THRESHOLD_PERCENT ) / PERCENT_100 ) &&
      ( producer_voting_power.value >= ( pso.total_producer_voting_power * EXECUTIVE_VOTE_THRESHOLD_PERCENT ) / PERCENT_100 );
   
   modify( executive_board, [&]( executive_board_object& e )
   {
      e.vote_count = vote_count;
      e.voting_power = voting_power;
      e.producer_vote_count = producer_vote_count;
      e.producer_voting_power = producer_voting_power;
      e.board_approved = approve_board;
   });

   ilog( "Updated Executive Board: ${b} Vote count: ${v} Approved: ${a}", 
      ("b",executive_board.account)("v",vote_count)("a",approve_board));

} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays the requested budgets of the approved executive boards on the network.
 * 
 * Boards that have sufficient approval from accounts and producers paid once per day.
 * Price of network credit asset must be greater than $0.90 USD to issue new units, or 
 * executive budgets are suspended. 
 * Network credit is a credit currency that is issued to executive boards
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

   ilog( "Process Executive Board Budgets" );

   const producer_schedule_object& pso = get_producer_schedule();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   price credit_usd_price = get_liquidity_pool( SYMBOL_USD, SYMBOL_CREDIT ).hour_median_price;

   const auto& exec_idx = get_index< executive_board_index >().indices().get< by_voting_power >();
   auto exec_itr = exec_idx.begin();

   while( exec_itr != exec_idx.end() )   // update all executive board approvals and vote statistics. 
   {
      const executive_board_object& exec = *exec_itr;
      update_executive_board( exec, pso, props );
      ++exec_itr;
   }

   if( credit_usd_price > MIN_EXEC_CREDIT_PRICE )
   {
      auto exec_itr = exec_idx.begin(); // reset iterator;

      while( exec_itr != exec_idx.end() )   // Pay the budget requests of the approved executive boards.
      {
         const executive_board_object& exec = *exec_itr;

         if( exec.board_approved )
         {
            ilog( "Processed Executive Board Budget: ${a} \n ${b} \n", 
               ("b",exec) );
            adjust_liquid_balance( exec.account, exec.budget );     // Issues new supply of credit asset to pay executive board.
         }
         ++exec_itr;
      }
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Update a governance account's voting approval statisitics
 * and update its approval if there are
 * sufficient votes from producers and other accounts.
 */
void database::governance_update_account( const governance_account_object& governance_account, 
   const producer_schedule_object& pso, const dynamic_global_property_object& props )
{ try {
   uint32_t vote_count = 0;
   share_type voting_power = 0;
   uint32_t producer_vote_count = 0;
   share_type producer_voting_power = 0;
   price equity_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_EQUITY ).hour_median_price;

   const auto& vote_idx = get_index< governance_subscription_index >().indices().get< by_governance_account >();
   auto vote_itr = vote_idx.lower_bound( governance_account.account );

   while( vote_itr != vote_idx.end() && 
      vote_itr->governance_account == governance_account.account )
   {
      const governance_subscription_object& vote = *vote_itr;
      const account_object& voter = get_account( vote.account );
      bool is_producer = pso.is_top_voting_producer( voter.name );
      vote_count++;
      share_type weight = 0;
      weight += get_voting_power( vote.account, equity_price );
      if( voter.proxied.size() )
      {
         weight += get_proxied_voting_power( voter, equity_price );
      }
      voting_power += share_type( weight.value >> vote.vote_rank );

      if( is_producer )
      {
         producer_vote_count++;
         const producer_object& producer = get_producer( voter.name );
         producer_voting_power += share_type( producer.voting_power.value >> vote.vote_rank );
      }
      ++vote_itr;
   }

   // Approve the governance account when a threshold of votes to support its budget.
   bool approve_account = ( vote_count >= GOVERNANCE_VOTE_THRESHOLD_AMOUNT ) &&
      ( producer_vote_count >= GOVERNANCE_VOTE_THRESHOLD_PRODUCERS ) &&
      ( voting_power.value >= ( props.total_voting_power * GOVERNANCE_VOTE_THRESHOLD_PERCENT ) / PERCENT_100 ) &&
      ( producer_voting_power.value >= ( pso.total_producer_voting_power * GOVERNANCE_VOTE_THRESHOLD_PERCENT ) / PERCENT_100 );
   
   modify( governance_account, [&]( governance_account_object& g )
   {
      g.subscriber_count = vote_count;
      g.subscriber_power = voting_power;
      g.producer_subscriber_count = producer_vote_count;
      g.producer_subscriber_power = producer_voting_power;
      g.account_approved = approve_account;
   });

   ilog( "Update Governance Account: ${g} Subscribers: ${s} Approved: ${a}",
      ("g",governance_account.account)("s",vote_count)("a",approve_account));

} FC_CAPTURE_AND_RETHROW() }


void database::governance_update_account_set()
{ try { 
   if( (head_block_num() % SET_UPDATE_BLOCK_INTERVAL ) != 0 )    // Runs once per day
      return;

   // ilog( "Update Governance Account Set" );
   
   const producer_schedule_object& pso = get_producer_schedule();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const auto& g_idx = get_index< governance_account_index >().indices().get< by_subscriber_power >();
   auto g_itr = g_idx.begin();
   
   while( g_itr != g_idx.end() )
   {
      governance_update_account( *g_itr, pso, props );
      ++g_itr;
   }

} FC_CAPTURE_AND_RETHROW() }


/**
 * Update a community enterprise proposal's voting approval statisitics
 * and increment the approved milestone if there are
 * sufficient current approvals from producers and other accounts.
 */
void database::update_enterprise( const enterprise_object& enterprise, 
   const producer_schedule_object& pso, const dynamic_global_property_object& props )
{ try {
   uint32_t vote_count = 0;
   share_type voting_power = 0;
   uint32_t producer_vote_count = 0;
   share_type producer_voting_power = 0;
   uint32_t funder_count = 0;
   asset total_funding = asset( 0, enterprise.budget.symbol );
   uint128_t net_sqrt_voting_power = 0;
   uint128_t net_sqrt_funding = 0;
   const asset_currency_data_object& currency = get_currency_data( enterprise.budget_symbol() );
   price equity_price = get_liquidity_pool( enterprise.budget_symbol(), currency.equity_asset ).hour_median_price;

   const auto& vote_idx = get_index< enterprise_vote_index >().indices().get< by_enterprise_id >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( enterprise.account, enterprise.enterprise_id ) );

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == enterprise.account && 
      vote_itr->enterprise_id == enterprise.enterprise_id )
   {
      const enterprise_vote_object& vote = *vote_itr;
      ++vote_itr;

      const account_object& voter = get_account( vote.account );
      if( voter.active )
      {
         vote_count++;
         voting_power += get_voting_power( vote.account, equity_price );

         if( voter.proxied.size() )
         {
            voting_power += get_proxied_voting_power( voter, equity_price );
         }

         net_sqrt_voting_power += util::approx_sqrt( uint128_t( voting_power.value ) );

         if( pso.is_top_voting_producer( voter.name ) )
         {
            const producer_object& producer = get_producer( voter.name );
            if( producer.active )
            {
               producer_vote_count++;
               producer_voting_power += producer.voting_power.value;
            }
         }
      }
   }

   const auto& fund_idx = get_index< enterprise_fund_index >().indices().get< by_account_enterprise_funder >();
   auto fund_itr = fund_idx.lower_bound( boost::make_tuple( enterprise.account, enterprise.enterprise_id ) );

   while( fund_itr != fund_idx.end() && 
      fund_itr->account == enterprise.account && 
      fund_itr->enterprise_id == enterprise.enterprise_id )
   {
      const enterprise_fund_object& fund = *fund_itr;
      ++fund_itr;

      const account_object& funder = get_account( fund.account );
      if( funder.active )
      {
         funder_count++;
         total_funding += fund.amount;
         net_sqrt_funding += util::approx_sqrt( uint128_t( fund.amount.amount.value ) );
      }
   }

   // Approve the latest claimed milestone when a threshold of approvals support its release.

   bool approve_enterprise = ( vote_count >= ENTERPRISE_VOTE_THRESHOLD_AMOUNT ) &&
      ( producer_vote_count >= ENTERPRISE_VOTE_THRESHOLD_PRODUCERS ) &&
      ( voting_power.value >= ( props.total_voting_power * ENTERPRISE_VOTE_THRESHOLD_PERCENT ) / PERCENT_100 ) &&
      ( producer_voting_power.value >= ( pso.total_producer_voting_power * ENTERPRISE_VOTE_THRESHOLD_PERCENT ) / PERCENT_100 ) &&
      ( funder_count >= ENTERPRISE_FUND_THRESHOLD_AMOUNT ) &&
      ( total_funding >= ( enterprise.budget * ENTERPRISE_FUND_THRESHOLD_PERCENT ) / PERCENT_100 );

   modify( enterprise, [&]( enterprise_object& e )
   {
      e.vote_count = vote_count;
      e.voting_power = voting_power;
      e.producer_vote_count = producer_vote_count;
      e.producer_voting_power = producer_voting_power;
      e.funder_count = funder_count;
      e.total_funding = total_funding;
      e.net_sqrt_voting_power = net_sqrt_voting_power;
      e.net_sqrt_funding = net_sqrt_funding;
      e.approved = approve_enterprise;
   });

   ilog( "Updated Enterprise: \n ${e} \n",
      ("e",enterprise));

} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates all community enterprise proposals.
 * 
 * Checks if they have sufficient votes and funds from accounts on the network and producers.
 * Processes budget payments for all proposals that have milestone approvals.
 */
void database::process_enterprise_fund()
{ try {
   if( (head_block_num() % ENTERPRISE_BLOCK_INTERVAL ) != 0 )    // Runs once per day.
      return;

   const producer_schedule_object& pso = get_producer_schedule();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const auto& asset_reward_fund_idx = get_index< asset_reward_fund_index >().indices().get< by_symbol >();
   auto asset_reward_fund_itr = asset_reward_fund_idx.begin();

   const auto& enterprise_idx = get_index< enterprise_index >().indices().get< by_symbol >();
   auto enterprise_itr = enterprise_idx.begin();

   ilog( "Processing Enterprise Reward Funds" );

   while( enterprise_itr != enterprise_idx.end() )
   {
      update_enterprise( *enterprise_itr, pso, props );
      ++enterprise_itr;
   }

   while( asset_reward_fund_itr != asset_reward_fund_idx.end() )
   {
      const asset_reward_fund_object& reward_fund = *asset_reward_fund_itr;
      ++asset_reward_fund_itr;
      
      ilog( "Processing Enterprise Reward Fund for Currency: ${s}",
         ("s",reward_fund.symbol) );

      uint128_t total_sqrt_voting_power = 0;
      uint128_t total_sqrt_funding = 0;
      uint128_t fund_share_amount = 0;
      uint128_t vote_share_amount = 0;
      share_type d = 0;
      share_type rem = 0;
      asset distributed = asset( 0, reward_fund.symbol );
      asset total_distributed = asset( 0, reward_fund.symbol );
      asset enterprise_fund_balance = reward_fund.enterprise_fund_balance / 2;
      asset enterprise_vote_balance = reward_fund.enterprise_fund_balance - enterprise_fund_balance;

      enterprise_itr = enterprise_idx.lower_bound( reward_fund.symbol );

      while( enterprise_itr != enterprise_idx.end() && 
         enterprise_itr->budget_symbol() == reward_fund.symbol )
      {
         const enterprise_object& enterprise = *enterprise_itr;
         ++enterprise_itr;
         
         if( enterprise.enterprise_active() )
         {
            total_sqrt_voting_power += enterprise.net_sqrt_voting_power;
            total_sqrt_funding += enterprise.net_sqrt_funding;

            ilog( "Enterprise counted: \n ${s} \n",
               ("s",enterprise));
         }
      }

      enterprise_itr = enterprise_idx.lower_bound( reward_fund.symbol );

      while( enterprise_itr != enterprise_idx.end() &&
         enterprise_itr->budget_symbol() == reward_fund.symbol )
      {
         const enterprise_object& enterprise = *enterprise_itr;
         ++enterprise_itr;

         if( enterprise.enterprise_active() )
         {
            ilog( "Processing Enterprise Funding: \n ${e} \n",
               ("e",enterprise));

            rem = enterprise.budget.amount - enterprise.distributed.amount;
            fund_share_amount = ( uint128_t( enterprise_fund_balance.amount.value ) * enterprise.net_sqrt_funding ) / total_sqrt_funding;
            vote_share_amount = ( uint128_t( enterprise_vote_balance.amount.value ) * enterprise.net_sqrt_voting_power ) / total_sqrt_voting_power;

            d = std::min( share_type( ( fund_share_amount + vote_share_amount ).to_uint64() ), rem );
            distributed = asset( d, reward_fund.symbol );
            total_distributed += distributed;
            
            adjust_reward_balance( enterprise.account, distributed );

            modify( enterprise, [&]( enterprise_object& eo )
            {
               eo.distributed += distributed;
            });
         }
      }

      modify( reward_fund, [&]( asset_reward_fund_object& arfo )
      {
         arfo.adjust_enterprise_fund_balance( -total_distributed );
      });

      adjust_pending_supply( -total_distributed );

      ilog( "Processed Enterprise Reward Fund for Currency: ${s}",
         ("s",reward_fund.symbol) );
   }

   ilog( "Processed Enterprise Reward Funds" );
} FC_CAPTURE_AND_RETHROW() }



/**
 * Pays the fees to a network contibutor, and splits fees to the account's governance 
 * account subscriptions, and registrar and referrer.
 */
asset database::pay_fee_share( const account_object& payee, const asset& amount, bool recursive )
{ try {
   asset total_fees = amount;

   if( recursive )
   {
      flat_set<const account_object*> governance_subscriptions;

      const auto& g_idx = get_index< governance_subscription_index >().indices().get< by_account_governance >();
      auto g_itr = g_idx.lower_bound( payee.name );

      while( g_itr != g_idx.end() && g_itr->account == payee.name )
      {
         const governance_subscription_object& sub = *g_itr;
         const account_object* account_ptr = find_account( sub.governance_account );
         governance_subscriptions.insert( account_ptr );
         ++g_itr;
      }
      const account_object& registrar = get_account( payee.registrar );
      const account_object& referrer = get_account( payee.referrer );

      asset g_share = ( amount * GOVERNANCE_SHARE_PERCENT ) / PERCENT_100;
      asset registrar_share = ( amount * REFERRAL_SHARE_PERCENT ) / PERCENT_100;
      asset referrer_share = ( registrar_share * payee.referrer_rewards_percentage ) / PERCENT_100;
      registrar_share -= referrer_share;

      asset g_paid = pay_multi_fee_share( governance_subscriptions, g_share, false );
      asset registrar_paid = pay_fee_share( registrar, registrar_share, false );
      asset referrer_paid = pay_fee_share( referrer, referrer_share, false );

      asset distribution = total_fees - ( g_paid + registrar_paid + referrer_paid );
      adjust_reward_balance( payee.name, distribution );
   }
   else
   {
      adjust_reward_balance( payee.name, total_fees );
   }
   
   return total_fees;

} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays fees to a set of network contibutors, and splits fees to the account's governance 
 * account subscriptions, and registrar and referrer.
 */
asset database::pay_multi_fee_share( flat_set< const account_object* > payees, const asset& amount, bool recursive )
{ try {
   asset total_paid = asset( 0, amount.symbol );
   if( payees.size() )
   {
      asset fee_split = amount / payees.size();
      for( auto payee : payees )
      {
         total_paid += pay_fee_share( *payee, fee_split, recursive ); 
      }
   }
  
   return total_paid;

} FC_CAPTURE_AND_RETHROW() }



/**
 * Pays the network fee by burning the core asset into accumulated network revenue,
 * or by burning network credit assets or force settling USD assets if their price
 * falls below $1.00 USD.
 */
asset database::pay_network_fees( const asset& amount )
{ try {
   asset total_fees = amount;
   if( amount.symbol != SYMBOL_COIN )
   {
      total_fees = liquid_exchange( amount, SYMBOL_COIN, true, false );
   }
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = head_block_time();
   price credit_usd_price = get_liquidity_pool( SYMBOL_USD, SYMBOL_CREDIT ).hour_median_price;
   price usd_settlement_price = get_stablecoin_data( SYMBOL_USD ).settlement_price;
   price usd_market_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_USD ).base_hour_median_price( usd_settlement_price.base.symbol );

   if( usd_market_price < usd_settlement_price )   // If the market price of USD is below settlement price
   {
      asset usd_purchased = liquid_exchange( total_fees, SYMBOL_USD, true, false );   // Liquid Exchange into USD, without paying fees to avoid recursive fees. 

      create< asset_settlement_object >([&]( asset_settlement_object& fso )
      {
         fso.owner = NULL_ACCOUNT;
         fso.balance = usd_purchased;    // Settle USD purchased at below settlement price, to increase total Coin burned.
         fso.settlement_date = now + fc::minutes( 10 );
      });
   }
   else if( credit_usd_price < price(asset(1,SYMBOL_USD)/asset(1,SYMBOL_CREDIT)) )   // If price of credit is below $1.00 USD
   {
      liquid_exchange( total_fees, SYMBOL_CREDIT, true, false );  // Liquid Exchange into Credit asset, without paying fees to avoid recursive fees. 

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

   return total_fees;

} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays the network fee by burning the core asset into accumulated network revenue,
 * or by burning network credit assets or force settling USD assets if their price
 * falls below $1.00 USD. Splits revenue to registrar and referrer, and governance 
 * accounts that the user subscribes to.
 */
asset database::pay_network_fees( const account_object& payer, const asset& amount )
{ try {
   asset total_fees = amount;
   if( amount.symbol != SYMBOL_COIN )
   {
      total_fees = liquid_exchange( amount, SYMBOL_COIN, true, false );
   }
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = head_block_time();

   flat_set<const account_object*> governance_subscriptions;

   const auto& g_idx = get_index< governance_subscription_index >().indices().get< by_account_governance >();
   auto g_itr = g_idx.lower_bound( payer.name );

   while( g_itr != g_idx.end() && g_itr->account == payer.name )
   {
      const governance_subscription_object& sub = *g_itr;
      const account_object* account_ptr = find_account( sub.governance_account );
      governance_subscriptions.insert( account_ptr );
      ++g_itr;
   }
   const account_object& registrar = get_account( payer.registrar );
   const account_object& referrer = get_account( payer.referrer );

   asset g_share = ( total_fees * GOVERNANCE_SHARE_PERCENT ) / PERCENT_100;
   asset registrar_share = ( total_fees * REFERRAL_SHARE_PERCENT ) / PERCENT_100;
   asset referrer_share = ( registrar_share * payer.referrer_rewards_percentage ) / PERCENT_100;
   registrar_share -= referrer_share;

   asset g_paid = pay_multi_fee_share( governance_subscriptions, g_share, true );
   asset registrar_paid = pay_fee_share( registrar, registrar_share, true );
   asset referrer_paid = pay_fee_share( referrer, referrer_share, true );

   total_fees -= ( g_paid + registrar_paid + referrer_paid );

   price credit_usd_price = get_liquidity_pool( SYMBOL_USD, SYMBOL_CREDIT ).hour_median_price;
   price usd_settlement_price = get_stablecoin_data( SYMBOL_USD ).settlement_price;
   price usd_market_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_USD ).base_hour_median_price( usd_settlement_price.base.symbol );

   if( usd_market_price < usd_settlement_price )       // If the market price of USD is below settlement price
   {
      asset usd_purchased = liquid_exchange( total_fees, SYMBOL_USD, true, false );      // Liquid Exchange into USD, without paying fees to avoid recursive fees. 

      create< asset_settlement_object >([&]( asset_settlement_object& fso ) 
      {
         fso.owner = NULL_ACCOUNT;
         fso.balance = usd_purchased;        // Settle USD purchased at below settlement price, to increase total Coin burned.
         fso.settlement_date = now + fc::minutes( 10 );
      });
   }
   else if( credit_usd_price < price(asset(1,SYMBOL_USD)/asset(1,SYMBOL_CREDIT)) )       // If price of credit is below $1.00 USD
   {
      liquid_exchange( total_fees, SYMBOL_CREDIT, true, false );       // Liquid Exchange into Credit asset, without paying fees to avoid recursive fees. 

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

   return total_fees;

} FC_CAPTURE_AND_RETHROW() }



/**
 * Acquires a debt asset using network credit asset.
 * 
 * Issues new credit asset to the liquidity pool of coin
 * and purchases the debt asset using the coin proceeds
 */
asset database::network_credit_acquisition( const asset& amount, bool execute )
{ try {
   asset credit_acquired = asset( 0, SYMBOL_CREDIT );

   ilog( "Network Credit Debt Acquisition: ${a}",
      ("a",amount.to_string()) );

   const asset_object& asset_obj = get_asset( amount.symbol );
   FC_ASSERT( asset_obj.is_credit_enabled(), 
      "Cannot acquire assets that do not facilitate liquidity pools." );

   if( amount.symbol == SYMBOL_CREDIT )
   {
      credit_acquired = amount;
   }
   else
   {
      credit_acquired = liquid_acquire( amount, SYMBOL_CREDIT, true, true );
   }
   
   adjust_pending_supply( credit_acquired );

   ilog( "Credit Acquisition: ${a}",
      ("a",credit_acquired.to_string()) );
   
   return credit_acquired;
} FC_CAPTURE_AND_RETHROW() }






} } // node::chain