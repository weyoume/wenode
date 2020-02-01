#include <node/app/api_context.hpp>
#include <node/app/application.hpp>
#include <node/app/database_api.hpp>

#include <node/protocol/get_config.hpp>

#include <node/chain/util/reward.hpp>

#include <fc/bloom_filter.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/crypto/hex.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>


#include <cctype>

#include <cfenv>
#include <iostream>

#define GET_REQUIRED_FEES_MAX_RECURSION 4

namespace node { namespace app {

class database_api_impl;

applied_operation::applied_operation() {}

applied_operation::applied_operation( const operation_object& op_obj ): 
   trx_id( op_obj.trx_id ),
   block( op_obj.block ),
   trx_in_block( op_obj.trx_in_block ),
   op_in_trx( op_obj.op_in_trx ),
   virtual_op( op_obj.virtual_op ),
   timestamp( op_obj.timestamp )
{
   //fc::raw::unpack( op_obj.serialized_op, op );     // g++ refuses to compile this as ambiguous
   op = fc::raw::unpack< operation >( op_obj.serialized_op );
}

void find_accounts( set< string >& accounts, const discussion& d ) 
{
   accounts.insert( d.author );
}

class database_api_impl : public std::enable_shared_from_this<database_api_impl>
{
   public:
      database_api_impl( const node::app::api_context& ctx  );
      ~database_api_impl();


      //=================//
      // === Globals === //
      //=================//


      fc::variant_object                              get_config()const;

      dynamic_global_property_api_obj                 get_dynamic_global_properties()const;

      chain_properties                                get_median_chain_properties()const;

      producer_schedule_api_obj                       get_producer_schedule()const;

      hardfork_version                                get_hardfork_version()const;

      scheduled_hardfork                              get_next_scheduled_hardfork()const;

      reward_fund_api_obj                             get_reward_fund()const;


      //===================//
      // === Accounts ==== //
      //===================//


      vector< account_api_obj >                       get_accounts( vector< string > names )const;

      vector< account_api_obj >                       get_accounts_by_followers( string from, uint32_t limit )const;
      
      vector< account_concise_api_obj >               get_concise_accounts( vector< string > names )const;

      vector< extended_account >                      get_full_accounts( vector< string > names )const;

      map< uint32_t, applied_operation >              get_account_history( string account, uint64_t from, uint32_t limit )const;

      vector< message_state >                         get_messages( vector< string > names )const;

      vector< balance_state >                         get_balances( vector< string > names )const;

      vector< key_state >                             get_keychains( vector< string > names )const;

      set< string >                                   lookup_accounts( string lower_bound_name, uint32_t limit )const;

      uint64_t                                        get_account_count()const;

      vector< owner_authority_history_api_obj >       get_owner_history( string account )const;

      optional< account_recovery_request_api_obj >    get_recovery_request( string account )const;

      optional< account_bandwidth_api_obj >           get_account_bandwidth( string account, producer::bandwidth_type type )const;


      //================//
      // === Assets === //
      //================//


      vector< extended_asset >                        get_assets( vector< string > assets )const;

      uint64_t                                        get_asset_count()const;

      optional< escrow_api_obj >                      get_escrow( string from, string escrow_id )const;

      vector< withdraw_route >                        get_withdraw_routes( string account, withdraw_route_type type = outgoing )const;

      vector< savings_withdraw_api_obj >              get_savings_withdraw_from( string account )const;

      vector< savings_withdraw_api_obj >              get_savings_withdraw_to( string account )const;

      vector< asset_delegation_api_obj >              get_asset_delegations( string account, string from, uint32_t limit = 100 )const;

      vector< asset_delegation_expiration_api_obj >   get_expiring_asset_delegations( string account, time_point from, uint32_t limit = 100 )const;


      //================//
      // === Boards === //
      //================//


      vector< extended_board >                        get_boards( vector< string > boards )const;

      vector< extended_board >                        get_boards_by_subscribers( string from, uint32_t limit )const;

      uint64_t                                        get_board_count()const;
      

      //=================//
      // === Network === //
      //=================//


      vector< producer_api_obj >                get_producers_by_account( vector< string > names )const;

      vector< account_name_type >               get_active_producers()const;

      set< account_name_type >                  lookup_producer_accounts( string lower_bound_name, uint32_t limit )const;

      uint64_t                                  get_producer_count()const;

      vector< producer_api_obj >                get_producers_by_voting_power( string from, uint32_t limit )const;

      vector< producer_api_obj >                get_producers_by_mining_power( string from, uint32_t limit )const;

      vector< network_officer_api_obj >         get_network_officers_by_account( vector< string > names )const;

      vector< network_officer_api_obj >         get_development_officers_by_voting_power( string from, uint32_t limit )const;

      vector< network_officer_api_obj >         get_marketing_officers_by_voting_power( string from, uint32_t limit )const;

      vector< network_officer_api_obj >         get_advocacy_officers_by_voting_power( string from, uint32_t limit )const;

      vector< executive_board_api_obj >         get_executive_boards_by_account( vector< string > names )const;

      vector< executive_board_api_obj >         get_executive_boards_by_voting_power( string from, uint32_t limit )const;

      vector< supernode_api_obj >               get_supernodes_by_account( vector< string > names )const;

      vector< supernode_api_obj >               get_supernodes_by_view_weight( string from, uint32_t limit )const;

      vector< interface_api_obj >               get_interfaces_by_account( vector< string > names )const;

      vector< interface_api_obj >               get_interfaces_by_users( string from, uint32_t limit )const;

      vector< governance_account_api_obj >      get_governance_accounts_by_account( vector< string > names )const;

      vector< governance_account_api_obj >      get_governance_accounts_by_subscriber_power( string from, uint32_t limit )const;

      vector< community_enterprise_api_obj >    get_enterprise_by_voting_power( string from, string from_id, uint32_t limit )const;


      //================//
      // === Market === //
      //================//


      vector< order_state >                     get_open_orders( vector< string > names )const;

      market_limit_orders                       get_limit_orders( string buy_symbol, string sell_symbol, uint32_t limit )const;

      market_margin_orders                      get_margin_orders( string buy_symbol, string sell_symbol, uint32_t limit )const;

      market_call_orders                        get_call_orders( string buy_symbol, string sell_symbol, uint32_t limit )const;

      market_credit_loans                       get_credit_loans( string buy_symbol, string sell_symbol, uint32_t limit )const;

      vector< credit_pool_api_obj >             get_credit_pools( vector< string > assets )const;

      vector< liquidity_pool_api_obj >          get_liquidity_pools( string buy_symbol, string sell_symbol )const;

      market_state                              get_market_state( string buy_symbol, string sell_symbol )const;


      //=============//
      // === Ads === //
      //=============//


      vector< account_ad_state >                get_account_ads( vector< string > names )const;

      vector< ad_bid_state >                    get_interface_audience_bids( const ad_query& query )const;


      //================//
      // === Search === //
      //================//


      search_result_state                       get_search_query( const search_query& query )const;


      //=================================//
      // === Blocks and transactions === //
      //=================================//


      optional< block_header >                  get_block_header( uint64_t block_num )const;

      optional< signed_block_api_obj >          get_block( uint64_t block_num )const;

      vector< applied_operation >               get_ops_in_block( uint64_t block_num, bool only_virtual )const;

      std::string                               get_transaction_hex( const signed_transaction& trx)const;

      annotated_signed_transaction              get_transaction( transaction_id_type trx_id )const;

      set< public_key_type >                    get_required_signatures( const signed_transaction& trx, const flat_set< public_key_type >& available_keys )const;

      set< public_key_type >                    get_potential_signatures( const signed_transaction& trx )const;

      bool                                      verify_authority( const signed_transaction& trx )const;

      bool                                      verify_account_authority( const string& name_or_id, const flat_set< public_key_type >& signers )const;


      //======================//
      // === Posts + Tags === //
      //======================//


      vector< vote_state >                 get_active_votes( string author, string permlink )const;

      vector< view_state >                 get_active_views( string author, string permlink )const;

      vector< share_state >                get_active_shares( string author, string permlink )const;

      vector< moderation_state >           get_active_mod_tags( string author, string permlink )const;

      vector< account_vote >               get_account_votes( string account, string from_author, string from_permlink, uint32_t limit )const;

      vector< account_view >               get_account_views( string account, string from_author, string from_permlink, uint32_t limit )const;

      vector< account_share >              get_account_shares( string account, string from_author, string from_permlink, uint32_t limit )const;

      vector< account_moderation >         get_account_moderation( string account, string from_author, string from_permlink, uint32_t limit )const;

      vector< tag_following_api_obj >      get_tag_followings( vector< string > tags )const;

      vector< tag_api_obj >                get_top_tags( string after_tag, uint32_t limit )const;

      vector< pair< tag_name_type, uint32_t > >    get_tags_used_by_author( string author )const;


      //=====================//
      // === Discussions === //
      //=====================//


      discussion                           get_content( string author, string permlink )const;
      
      vector< discussion >                 get_content_replies( string parent, string parent_permlink )const;

      vector< discussion >                 get_replies_by_last_update( account_name_type start_author, string start_permlink, uint32_t limit )const;

      vector< discussion >                 get_discussions_by_sort_rank( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_feed( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_blog( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_recommended( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_comments( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_payout(const discussion_query& query )const;

      vector< discussion >                 get_post_discussions_by_payout( const discussion_query& query )const;

      vector< discussion >                 get_comment_discussions_by_payout( const discussion_query& query )const;

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


      //===============//
      // === State === //
      //===============//


      state                                get_state( string path )const;


      //=======================//
      // === Subscriptions === //
      //=======================//


      void                                 set_block_applied_callback( std::function<void( const variant& block_id )> cb );


      //=========================//
      // === Signal Handlers === //
      //=========================//


      void                                             on_applied_block( const chain::signed_block& b );

      std::function<void( const variant&)>             _block_applied_callback;

      node::chain::database&                          _db;

      std::shared_ptr< node::follow::follow_api >     _follow_api;

      boost::signals2::scoped_connection              _block_applied_connection;

      bool                                            _disable_get_block = false;

      void set_url( discussion& d )const;

      discussion get_discussion( comment_id_type, uint32_t truncate_body = 0 )const;

      static bool filter_default( const comment_api_obj& c ) { return false; }
      static bool exit_default( const comment_api_obj& c )   { return false; }
      static bool tag_exit_default( const tags::tag_object& c ) { return false; }

      template< typename Index, typename StartItr >
      vector< discussion > get_discussions( 
         const discussion_query& q,
         const string& board,
         const string& tag,
         comment_id_type parent,
         const Index& idx, 
         StartItr itr,
         uint32_t truncate_body = 0,
         const std::function< bool( const comment_api_obj& ) >& filter = &database_api::filter_default,
         const std::function< bool( const comment_api_obj& ) >& exit   = &database_api::exit_default,
         const std::function< bool( const tags::tag_object& ) >& tag_exit = &database_api::tag_exit_default,
         bool ignore_parent = false
         )const;
         
      comment_id_type get_parent( const discussion_query& q )const;

      void recursively_fetch_content( state& _state, discussion& root, set<string>& referenced_accounts )const;
};


   //======================//
   // === Constructors === //
   //======================//


database_api::database_api( const node::app::api_context& ctx )
   : my( new database_api_impl( ctx ) ) {}

database_api::~database_api() {}

database_api_impl::database_api_impl( const node::app::api_context& ctx )
   : _db( *ctx.app.chain_database() )
{
   wlog( "creating database api ${x}", ("x",int64_t(this)) );

   _disable_get_block = ctx.app._disable_get_block;

   try
   {
      ctx.app.get_plugin< follow::follow_plugin >( FOLLOW_PLUGIN_NAME );
      _follow_api = std::make_shared< node::follow::follow_api >( ctx );
   }
   catch( fc::assert_exception ) { ilog( "Follow Plugin not loaded" ); }
}

database_api_impl::~database_api_impl()
{
   elog( "freeing database api ${x}", ("x",int64_t(this)) );
}

void database_api::on_api_startup() {}

u256 to256( const fc::uint128& t )
{
   u256 results( t.high_bits() );
   results <<= 65;
   results += t.low_bits();
   return results;
}

void database_api::set_url( discussion& d )const
{
   const comment_api_obj root( my->_db.get< comment_object, by_id >( d.root_comment ) );
   d.url = "/" + root.category + "/@" + root.author + "/" + root.permlink;
   d.root_title = root.title;
   if( root.id != d.id )
      d.url += "#@" + d.author + "/" + d.permlink;
}


   //=================//
   // === Globals === //
   //=================//
   

fc::variant_object database_api::get_config()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_config();
   });
}

fc::variant_object database_api_impl::get_config()const
{
   return node::protocol::get_config();
}

dynamic_global_property_api_obj database_api::get_dynamic_global_properties()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_dynamic_global_properties();
   });
}

dynamic_global_property_api_obj database_api_impl::get_dynamic_global_properties()const
{
   return dynamic_global_property_api_obj( _db.get( dynamic_global_property_id_type() ), _db );
}

chain_properties database_api::get_median_chain_properties()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_median_chain_properties();
   });
}

chain_properties database_api_impl::get_median_chain_properties()const
{
   return _db.get_median_chain_properties();
}

producer_schedule_api_obj database_api::get_producer_schedule()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_producer_schedule();
   });
}

producer_schedule_api_obj database_api_impl::get_producer_schedule()const
{
   return producer_schedule_api_obj( _db.get_producer_schedule() );
}

hardfork_version database_api::get_hardfork_version()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_hardfork_version();
   });
}

hardfork_version database_api_impl::get_hardfork_version()const
{
   const hardfork_property_object& hpo = _db.get( hardfork_property_id_type() );
   return hpo.current_hardfork_version;
}

scheduled_hardfork database_api::get_next_scheduled_hardfork() const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_next_scheduled_hardfork();
   });
}

scheduled_hardfork database_api_impl::get_next_scheduled_hardfork() const
{
   scheduled_hardfork shf;
   const hardfork_property_object& hpo = _db.get( hardfork_property_id_type() );
   shf.hf_version = hpo.next_hardfork;
   shf.live_time = hpo.next_hardfork_time;
   return shf;
}

reward_fund_api_obj database_api::get_reward_fund()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_reward_fund();
   });
}

reward_fund_api_obj database_api_impl::get_reward_fund()const
{
   return reward_fund_api_obj( _db.get( reward_fund_id_type() ) );
}


   //===================//
   // === Accounts ==== //
   //===================//


vector< account_api_obj > database_api::get_accounts( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_accounts( names );
   });
}

vector< account_api_obj > database_api_impl::get_accounts( vector< string > names )const
{
   const auto& account_idx  = _db.get_index< account_index >().indices().get< by_name >();

   vector< account_api_obj > results;

   for( auto name: names )
   {
      auto account_itr = account_idx.find( name );
      if( account_itr != account_idx.end() )
      {
         results.push_back( account_api_obj( *account_itr, _db ) );
      }  
   }

   return results;
}


vector< account_api_obj > database_api::get_accounts_by_followers( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_accounts_by_followers( from, limit );
   });
}

vector< account_api_obj > database_api_impl::get_accounts_by_followers( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< account_api_obj > results;
   results.reserve( limit );

   const auto& account_idx  = _db.get_index< account_index >().indices().get< by_follower_count >();
   const auto& name_idx  = _db.get_index< account_index >().indices().get< by_name >();

   auto account_itr = account_idx.begin();
  
   if( from.size() )
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid Board name ${n}", ("n",from) );
      account_itr = account_idx.iterator_to( *name_itr );
   }

   while( account_itr != account_idx.end() && results.size() < limit )
   {
      results.push_back( account_api_obj( *account_itr, _db ) );
      ++account_itr;
   }
   return results;
}

vector< account_concise_api_obj > database_api::get_concise_accounts( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_concise_accounts( names );
   });
}

vector< account_concise_api_obj > database_api_impl::get_concise_accounts( vector< string > names )const
{
   const auto& account_idx  = _db.get_index< account_index >().indices().get< by_name >();

   vector< account_concise_api_obj > results;

   for( auto name: names )
   {
      auto account_itr = account_idx.find( name );
      if ( account_itr != account_idx.end() )
      {
         results.push_back( *account_itr );
      }  
   }

   return results;
}

vector< extended_account > database_api::get_full_accounts( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_full_accounts( names );
   });
}

/**
 * Retrieves all relevant state information regarding a list of specified accounts, including:
 * - Balances, active transfers and requests.
 * - Business Account details, invites and requests.
 * - Connection Details and requests.
 * - Incoming and Outgoing messages, and conversations with accounts.
 * - Board Moderation and ownership details, and invites and requests.
 * - Producer, network officer, and executive board details and votes.
 * - Interface and Supernode details.
 * - Advertising Campaigns, inventory, creative, bids, and audiences.
 */
vector< extended_account > database_api_impl::get_full_accounts( vector< string > names )const
{
   const auto& account_idx  = _db.get_index< account_index >().indices().get< by_name >();
   const auto& balance_idx = _db.get_index< account_balance_index >().indices().get< by_owner >();
   const auto& business_idx = _db.get_index< account_business_index >().indices().get< by_account >();
   const auto& bus_key_idx = _db.get_index< account_member_key_index >().indices().get< by_member_business >();
   const auto& board_key_idx = _db.get_index< board_member_key_index >().indices().get< by_member_board >();
   const auto& following_idx = _db.get_index< account_following_index >().indices().get< by_account >();
   const auto& connection_a_idx = _db.get_index< connection_index >().indices().get< by_account_a >();
   const auto& connection_b_idx = _db.get_index< connection_index >().indices().get< by_account_b >();
   const auto& inbox_idx = _db.get_index< message_index >().indices().get< by_account_inbox >();
   const auto& outbox_idx = _db.get_index< message_index >().indices().get< by_account_outbox >();

   const auto& limit_idx = _db.get_index< limit_order_index >().indices().get< by_account >();
   const auto& margin_idx = _db.get_index< margin_order_index >().indices().get< by_account >();
   const auto& call_idx = _db.get_index< call_order_index >().indices().get< by_account >();
   const auto& loan_idx = _db.get_index< credit_loan_index >().indices().get< by_owner >();
   const auto& collateral_idx = _db.get_index< credit_collateral_index >().indices().get< by_owner >();

   const auto& producer_vote_idx = _db.get_index< producer_vote_index >().indices().get< by_account_rank >();
   const auto& executive_vote_idx = _db.get_index< executive_board_vote_index >().indices().get< by_account_rank >();
   const auto& officer_vote_idx = _db.get_index< network_officer_vote_index >().indices().get< by_account_type_rank >();
   const auto& enterprise_vote_idx = _db.get_index< enterprise_approval_index >().indices().get< by_account_rank >();
   const auto& moderator_idx = _db.get_index< board_moderator_vote_index >().indices().get< by_account_board_rank >();
   const auto& account_officer_idx = _db.get_index< account_officer_vote_index >().indices().get< by_account_business_rank >();
   const auto& account_exec_idx = _db.get_index< account_executive_vote_index >().indices().get< by_account_business_role_rank >();

   const auto& connection_req_idx = _db.get_index< connection_request_index >().indices().get< by_req_account >();
   const auto& connection_acc_idx = _db.get_index< connection_request_index >().indices().get< by_account_req >();

   const auto& account_req_idx = _db.get_index< account_member_request_index >().indices().get< by_account_business >();
   const auto& bus_req_idx = _db.get_index< account_member_request_index >().indices().get< by_business_account >();
   const auto& account_inv_idx = _db.get_index< account_member_invite_index >().indices().get< by_account >();
   const auto& member_inv_idx = _db.get_index< account_member_invite_index >().indices().get< by_member >();
   const auto& bus_inv_idx = _db.get_index< account_member_invite_index >().indices().get< by_business >();

   const auto& board_req_idx = _db.get_index< board_join_request_index >().indices().get< by_account_board >();
   const auto& board_acc_inv_idx = _db.get_index< board_join_invite_index >().indices().get< by_account >();
   const auto& board_member_inv_idx = _db.get_index< board_join_invite_index >().indices().get< by_member >();
   const auto& board_member_idx = _db.get_index< board_member_index >().indices().get< by_name >();

   const auto& transfer_req_idx = _db.get_index< transfer_request_index >().indices().get< by_request_id >();
   const auto& transfer_from_req_idx = _db.get_index< transfer_request_index >().indices().get< by_from_account >();
   const auto& recurring_idx = _db.get_index< transfer_recurring_index >().indices().get< by_transfer_id >();
   const auto& recurring_to_idx = _db.get_index< transfer_recurring_index >().indices().get< by_to_account >();
   const auto& recurring_req_idx = _db.get_index< transfer_recurring_request_index >().indices().get< by_request_id >();
   const auto& recurring_from_req_idx = _db.get_index< transfer_recurring_request_index >().indices().get< by_from_account >();

   const auto& producer_idx = _db.get_index< producer_index >().indices().get< by_name >();
   const auto& executive_idx = _db.get_index< executive_board_index >().indices().get< by_account >();
   const auto& officer_idx = _db.get_index< network_officer_index >().indices().get< by_account >();
   const auto& enterprise_idx = _db.get_index< community_enterprise_index >().indices().get< by_creator >();
   const auto& interface_idx = _db.get_index< interface_index >().indices().get< by_account >();
   const auto& supernode_idx = _db.get_index< supernode_index >().indices().get< by_account >();
   const auto& governance_idx = _db.get_index< governance_account_index >().indices().get< by_account >();

   const auto& history_idx = _db.get_index< account_history_index >().indices().get< by_account >();

   vector< extended_account > results;

   for( auto name: names )
   {
      auto account_itr = account_idx.find( name );
      if ( account_itr != account_idx.end() )
      {
         results.push_back( extended_account( *account_itr, _db ) );

         auto balance_itr = balance_idx.lower_bound( name );
         while( balance_itr != balance_idx.end() && balance_itr->owner == name )
         {
            results.back().balances.balances[ balance_itr->symbol ] = account_balance_api_obj( *balance_itr );
            ++balance_itr;
         }
   
         auto limit_itr = limit_idx.lower_bound( name );
         while( limit_itr != limit_idx.end() && limit_itr->seller == name ) 
         {
            results.back().orders.limit_orders.push_back( limit_order_api_obj( *limit_itr ) );
            ++limit_itr;
         }

         auto margin_itr = margin_idx.lower_bound( name );
         while( margin_itr != margin_idx.end() && margin_itr->owner == name )
         {
            results.back().orders.margin_orders.push_back( margin_order_api_obj( *margin_itr ) );
            ++margin_itr;
         }

         auto call_itr = call_idx.lower_bound( name );
         while( call_itr != call_idx.end() && call_itr->borrower == name ) 
         {
            results.back().orders.call_orders.push_back( call_order_api_obj( *call_itr ) );
            ++call_itr;
         }

         auto loan_itr = loan_idx.lower_bound( name );
         while( loan_itr != loan_idx.end() && loan_itr->owner == name ) 
         {
            results.back().orders.loan_orders.push_back( credit_loan_api_obj( *loan_itr ) );
            ++loan_itr;
         }

         auto collateral_itr = collateral_idx.lower_bound( name );
         while( collateral_itr != collateral_idx.end() && collateral_itr->owner == name ) 
         {
            results.back().orders.collateral.push_back( credit_collateral_api_obj( *collateral_itr ) );
            ++collateral_itr;
         }
   
         auto following_itr = following_idx.find( name );
         if( following_itr != following_idx.end() )
         {
            results.back().following = *following_itr;
         }

         auto producer_itr = producer_idx.find( name );
         if( producer_itr != producer_idx.end() )
         {
            results.back().network.producer = producer_api_obj( *producer_itr );
         }

         auto executive_itr = executive_idx.find( name );
         if( executive_itr != executive_idx.end() )
         {
            results.back().network.executive_board = executive_board_api_obj( *executive_itr );
         }

         auto officer_itr = officer_idx.find( name );
         if( officer_itr != officer_idx.end() )
         {
            results.back().network.network_officer = network_officer_api_obj( *officer_itr );
         }

         auto interface_itr = interface_idx.find( name );
         if( interface_itr != interface_idx.end() )
         {
            results.back().network.interface = interface_api_obj( *interface_itr );
         }

         auto supernode_itr = supernode_idx.find( name );
         if( supernode_itr != supernode_idx.end() )
         {
            results.back().network.supernode = supernode_api_obj( *supernode_itr );
         }

         auto governance_itr = governance_idx.find( name );
         if( governance_itr != governance_idx.end() )
         {
            results.back().network.governance_account = governance_account_api_obj( *governance_itr );
         }

         auto enterprise_itr = enterprise_idx.lower_bound( name );
         while( enterprise_itr != enterprise_idx.end() && enterprise_itr->creator == name )
         {
            results.back().network.enterprise_proposals.push_back( community_enterprise_api_obj( *enterprise_itr ) );
            ++enterprise_itr;
         }

         auto producer_vote_itr = producer_vote_idx.lower_bound( name );
         while( producer_vote_itr != producer_vote_idx.end() && producer_vote_itr->account == name ) 
         {
            results.back().network.producer_votes[ producer_vote_itr->producer ] = producer_vote_itr->vote_rank;
            ++producer_vote_itr;
         }

         auto executive_vote_itr = executive_vote_idx.lower_bound( name );
         while( executive_vote_itr != executive_vote_idx.end() && executive_vote_itr->account == name )
         {
            results.back().network.executive_board_votes[ executive_vote_itr->executive_board ] = executive_vote_itr->vote_rank;
            ++executive_vote_itr;
         }

         auto officer_vote_itr = officer_vote_idx.lower_bound( name );
         while( officer_vote_itr != officer_vote_idx.end() && officer_vote_itr->account == name )
         {
            results.back().network.network_officer_votes[ network_officer_role_values[ officer_vote_itr->officer_type ] ][ officer_vote_itr->network_officer ] = officer_vote_itr->vote_rank;
            ++officer_vote_itr;
         }

         auto account_exec_itr = account_exec_idx.lower_bound( name );
         while( account_exec_itr != account_exec_idx.end() && account_exec_itr->account == name )
         {
            results.back().network.account_executive_votes[ account_exec_itr->business_account ][ executive_role_values[ account_exec_itr->role ] ] = std::make_pair( account_exec_itr->executive_account, account_exec_itr->vote_rank );
            ++account_exec_itr;
         }

         auto account_officer_itr = account_officer_idx.lower_bound( name );
         while( account_officer_itr != account_officer_idx.end() && account_officer_itr->account == name )
         {
            results.back().network.account_officer_votes[ account_officer_itr->business_account ][ account_officer_itr->officer_account ] = account_officer_itr->vote_rank;
            ++account_officer_itr;
         }

         auto enterprise_vote_itr = enterprise_vote_idx.lower_bound( name );
         while( enterprise_vote_itr != enterprise_vote_idx.end() && enterprise_vote_itr->account == name )
         {
            results.back().network.enterprise_approvals[ enterprise_itr->creator ][ to_string( enterprise_itr->enterprise_id ) ] = enterprise_vote_itr->vote_rank;
            ++enterprise_itr;
         }

         auto business_itr = business_idx.find( name );
         if( business_itr != business_idx.end() )
         {
            results.back().business.business = account_business_api_obj( *business_itr );
         }

         auto business_itr = business_idx.begin();

         while( business_itr != business_idx.end() )
         {
            if( business_itr->is_executive( name ) )
            {
               results.back().business.executive_businesses.push_back( business_itr->account );
            }
            else if( business_itr->is_officer( name ) )
            {
               results.back().business.officer_businesses.push_back( business_itr->account );
            }
            else if( business_itr->is_member( name ) )
            {
               results.back().business.member_businesses.push_back( business_itr->account );
            }
            
            ++business_itr;
         }

         auto account_req_itr = account_req_idx.lower_bound( name );
         auto bus_req_itr = bus_req_idx.lower_bound( name );

         while( account_req_itr != account_req_idx.end() && account_req_itr->account == name )
         {
            results.back().business.outgoing_requests[ account_req_itr->business_account ] = account_request_api_obj( *account_req_itr );
            ++account_req_itr;
         }

         while( bus_req_itr != bus_req_idx.end() && bus_req_itr->business_account == name )
         {
            results.back().business.incoming_requests[ bus_req_itr->account ] = account_request_api_obj( *bus_req_itr );
            ++bus_req_itr;
         }

         auto account_inv_itr = account_inv_idx.lower_bound( name );
         auto member_inv_itr = member_inv_idx.lower_bound( name );
         auto bus_inv_itr = bus_inv_idx.lower_bound( name );

         while( account_inv_itr != account_inv_idx.end() && account_inv_itr->account == name )
         {
            results.back().business.outgoing_invites[ account_inv_itr->member ] = account_invite_api_obj( *account_inv_itr );
            ++account_inv_itr;
         }

         while( member_inv_itr != member_inv_idx.end() && member_inv_itr->member == name )
         {
            results.back().business.incoming_invites[ member_inv_itr->business_account ] = account_invite_api_obj( *member_inv_itr );
            ++member_inv_itr;
         }

         while( bus_inv_itr != bus_inv_idx.end() && bus_inv_itr->business_account == name )
         {
            results.back().business.outgoing_invites[ bus_inv_itr->member ] = account_invite_api_obj( *bus_inv_itr );
            ++bus_inv_itr;
         }

         auto connection_a_itr = connection_a_idx.lower_bound( boost::make_tuple( name, CONNECTION ) );
         auto connection_b_itr = connection_b_idx.lower_bound( boost::make_tuple( name, CONNECTION ) );
         while( connection_a_itr != connection_a_idx.end() && 
            connection_a_itr->account_a == name &&
            connection_a_itr->connection_type == CONNECTION )
         {
            results.back().connections.connections[ connection_a_itr->account_b ] = connection_api_obj( *connection_a_itr );
            results.back().keychain.connection_keys[ connection_a_itr->account_b ] = connection_a_itr->encrypted_key_b;
            ++connection_a_itr;
         }
         while( connection_b_itr != connection_b_idx.end() && 
            connection_b_itr->account_b == name &&
            connection_b_itr->connection_type == CONNECTION )
         {
            results.back().connections.connections[ connection_b_itr->account_a ] = connection_api_obj( *connection_b_itr );
            results.back().keychain.connection_keys[ connection_b_itr->account_a ] = connection_b_itr->encrypted_key_a;
            ++connection_b_itr;
         }

         auto connection_a_itr = connection_a_idx.lower_bound( boost::make_tuple( name, FRIEND ) );
         auto connection_b_itr = connection_b_idx.lower_bound( boost::make_tuple( name, FRIEND ) );
         while( connection_a_itr != connection_a_idx.end() && 
            connection_a_itr->account_a == name &&
            connection_a_itr->connection_type == FRIEND )
         {
            results.back().connections.friends[ connection_a_itr->account_b ] = connection_api_obj( *connection_a_itr );
            results.back().keychain.friend_keys[ connection_a_itr->account_b ] = connection_a_itr->encrypted_key_b;
            ++connection_a_itr;
         }
         while( connection_b_itr != connection_b_idx.end() && 
            connection_b_itr->account_b == name &&
            connection_b_itr->connection_type == FRIEND )
         {
            results.back().connections.friends[ connection_b_itr->account_a ] = connection_api_obj( *connection_b_itr );
            results.back().keychain.friend_keys[ connection_b_itr->account_a ] = connection_b_itr->encrypted_key_a;
            ++connection_b_itr;
         }

         auto connection_a_itr = connection_a_idx.lower_bound( boost::make_tuple( name, COMPANION ) );
         auto connection_b_itr = connection_b_idx.lower_bound( boost::make_tuple( name, COMPANION ) );
         while( connection_a_itr != connection_a_idx.end() && 
            connection_a_itr->account_a == name &&
            connection_a_itr->connection_type == COMPANION )
         {
            results.back().connections.companions[ connection_a_itr->account_b ] = connection_api_obj( *connection_a_itr );
            results.back().keychain.companion_keys[ connection_a_itr->account_b ] = connection_a_itr->encrypted_key_b;
            ++connection_a_itr;
         }
         while( connection_b_itr != connection_b_idx.end() && 
            connection_b_itr->account_b == name &&
            connection_b_itr->connection_type == COMPANION )
         {
            results.back().connections.companions[ connection_b_itr->account_a ] = connection_api_obj( *connection_b_itr );
            results.back().keychain.companion_keys[ connection_b_itr->account_a ] = connection_b_itr->encrypted_key_a;
            ++connection_b_itr;
         }

         auto connection_req_itr = connection_req_idx.lower_bound( name );
         auto connection_acc_itr = connection_acc_idx.lower_bound( name );

         while( connection_req_itr != connection_req_idx.end() && connection_req_itr->requested_account == name )
         {
            results.back().connections.incoming_requests[ connection_req_itr->account ] = connection_request_api_obj( *connection_req_itr );
            ++connection_req_itr;
         }

         while( connection_acc_itr != connection_acc_idx.end() && connection_acc_itr->account == name )
         {
            results.back().connections.outgoing_requests[ connection_acc_itr->requested_account ] = connection_request_api_obj( *connection_acc_itr );
            ++connection_acc_itr;
         }

         auto board_itr = board_member_idx.begin();

         while( board_itr != board_member_idx.end() )
         {
            if( board_itr->founder == name )
            {
               results.back().boards.founded_boards.push_back( board_itr->name );
            }
            else if( board_itr->is_administrator( name ) )
            {
               results.back().boards.admin_boards.push_back( board_itr->name );
            }
            else if( board_itr->is_moderator( name ) )
            {
               results.back().boards.moderator_boards.push_back( board_itr->name );
            }
            else if( board_itr->is_member( name ) )
            {
               results.back().boards.member_boards.push_back( board_itr->name );
            }
            
            ++board_itr;
         }

         auto board_req_itr = board_req_idx.lower_bound( name );
         auto board_acc_inv_itr = board_acc_inv_idx.lower_bound( name );
         auto board_member_inv_itr = board_member_inv_idx.lower_bound( name );

         while( board_req_itr != board_req_idx.end() && board_req_itr->account == name )
         {
            results.back().boards.pending_requests[ board_req_itr->account ] = board_request_api_obj( *board_req_itr );
            ++board_req_itr;
         }

         while( board_acc_inv_itr != board_acc_inv_idx.end() && board_acc_inv_itr->account == name )
         {
            results.back().boards.outgoing_invites[ board_acc_inv_itr->member ] = board_invite_api_obj( *board_acc_inv_itr );
            ++board_acc_inv_itr;
         }

         while( board_member_inv_itr != board_member_inv_idx.end() && board_member_inv_itr->member == name )
         {
            results.back().boards.incoming_invites[ board_member_inv_itr->board ] = board_invite_api_obj( *board_member_inv_itr );
            ++board_member_inv_itr;
         }

         auto board_key_itr = board_key_idx.lower_bound( name );
         while( board_key_itr != board_key_idx.end() && board_key_itr->member == name )
         {
            results.back().keychain.board_keys[ board_key_itr->board ] = board_key_itr->encrypted_board_key;
            ++board_key_itr;
         }

         auto bus_key_itr = bus_key_idx.lower_bound( name );
         while( bus_key_itr != bus_key_idx.end() && bus_key_itr->member == name )
         {
            results.back().keychain.business_keys[ bus_key_itr->business_account ] = bus_key_itr->encrypted_business_key;
            ++bus_key_itr;
         }

         auto transfer_req_itr = transfer_req_idx.lower_bound( name );
         auto transfer_from_req_itr = transfer_from_req_idx.lower_bound( name );
         auto recurring_itr = recurring_idx.lower_bound( name );
         auto recurring_to_itr = recurring_to_idx.lower_bound( name );
         auto recurring_req_itr = recurring_req_idx.lower_bound( name );
         auto recurring_from_req_itr = recurring_from_req_idx.lower_bound( name );

         while( transfer_req_itr != transfer_req_idx.end() && transfer_req_itr->to == name )
         {
            results.back().transfers.outgoing_requests[ transfer_req_itr->from ] = transfer_request_api_obj( *transfer_req_itr );
            ++transfer_req_itr;
         }

         while( transfer_from_req_itr != transfer_from_req_idx.end() && transfer_from_req_itr->from == name )
         {
            results.back().transfers.incoming_requests[ transfer_from_req_itr->to ] = transfer_request_api_obj( *transfer_from_req_itr );
            ++transfer_from_req_itr;
         }

         while( recurring_itr != recurring_idx.end() && recurring_itr->from == name )
         {
            results.back().transfers.outgoing_recurring_transfers[ recurring_itr->to ] = transfer_recurring_api_obj( *recurring_itr );
            ++recurring_itr;
         }

         while( recurring_to_itr != recurring_to_idx.end() && recurring_to_itr->to == name )
         {
            results.back().transfers.incoming_recurring_transfers[ recurring_to_itr->from ] = transfer_recurring_api_obj( *recurring_to_itr );
            ++recurring_to_itr;
         }

         while( recurring_req_itr != recurring_req_idx.end() && recurring_req_itr->to == name )
         {
            results.back().transfers.outgoing_recurring_transfer_requests[ recurring_req_itr->from ] = transfer_recurring_request_api_obj( *recurring_req_itr );
            ++recurring_req_itr;
         }

         while( recurring_from_req_itr != recurring_from_req_idx.end() && recurring_from_req_itr->from == name )
         {
            results.back().transfers.incoming_recurring_transfer_requests[ recurring_from_req_itr->to ] = transfer_recurring_request_api_obj( *recurring_from_req_itr );
            ++recurring_from_req_itr;
         }

         auto inbox_itr = inbox_idx.lower_bound( name );
         auto outbox_itr = outbox_idx.lower_bound( name );
         vector< message_api_obj > inbox;
         vector< message_api_obj > outbox;
         map< account_name_type, vector< message_api_obj > > conversations;

         while( inbox_itr != inbox_idx.end() && inbox_itr->recipient == name )
         {
            inbox.push_back( message_api_obj( *inbox_itr ) );
         }

         while( outbox_itr != outbox_idx.end() && outbox_itr->sender == name )
         {
            outbox.push_back( message_api_obj( *outbox_itr ) );
         }

         for( auto message : inbox )
         {
            conversations[ message.sender ].push_back( message );
         }

         for( auto message : outbox )
         {
            conversations[ message.recipient ].push_back( message );
         }

         for( auto conv : conversations )
         {
            vector< message_api_obj > thread = conv.second;
            std::sort( thread.begin(), thread.end(), [&](auto a, auto b)
            {
               return a.created < b.created;
            });
            conversations[ conv.first ] = thread;
         }

         message_state mstate;
         mstate.inbox = inbox;
         mstate.outbox = outbox;
         mstate.conversations = conversations;
         results.back().messages = mstate;

         auto moderator_itr = moderator_idx.lower_bound( name );
         while( moderator_itr != moderator_idx.end() && moderator_itr->account == name )
         {
            results.back().boards.outgoing_moderator_votes[ moderator_itr->board ][ moderator_itr->moderator ] = moderator_itr->vote_rank;
            ++moderator_itr;
         }

         results.back().tags_usage = get_tags_used_by_author( name );
         if( _follow_api )
         {
            results.back().top_shared = _follow_api->get_blog_authors( name );
         }

         auto history_itr = history_idx.lower_bound( name );
   
         map< uint32_t, applied_operation> operation_history;
         
         while( history_itr != history_idx.end() && history_itr->account == name )
         {
            operation_history[ history_itr->sequence ] = _db.get( history_itr->op );
            ++history_itr;
         }

         for( auto& item : operation_history )
         {
            switch( item.second.op.which() )
            {
               case operation::tag<account_create_operation>::value:
               case operation::tag<account_update_operation>::value:
               case operation::tag<account_membership_operation>::value:
               case operation::tag<account_vote_executive_operation>::value:
               case operation::tag<account_vote_officer_operation>::value:
               case operation::tag<account_member_request_operation>::value:
               case operation::tag<account_member_invite_operation>::value:
               case operation::tag<account_accept_request_operation>::value:
               case operation::tag<account_accept_invite_operation>::value:
               case operation::tag<account_remove_member_operation>::value:
               case operation::tag<account_update_list_operation>::value:
               case operation::tag<request_account_recovery_operation>::value:
               case operation::tag<recover_account_operation>::value:
               case operation::tag<reset_account_operation>::value:
               case operation::tag<set_reset_account_operation>::value:
               case operation::tag<change_recovery_account_operation>::value:
               case operation::tag<decline_voting_rights_operation>::value:
               {
                  results.back().operations.account_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<connection_request_operation>::value:
               case operation::tag<connection_accept_operation>::value:
               {
                  results.back().operations.connection_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<account_follow_operation>::value:
               case operation::tag<tag_follow_operation>::value:
               {
                  results.back().operations.follow_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<activity_reward_operation>::value:
               {
                  results.back().operations.activity_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<comment_operation>::value:
               {
                  results.back().operations.post_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<message_operation>::value:
               {
                  results.back().operations.message_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<vote_operation>::value:
               {
                  results.back().operations.vote_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<view_operation>::value:
               {
                  results.back().operations.view_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<share_operation>::value:
               {
                  results.back().operations.share_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<moderation_tag_operation>::value:
               {
                  results.back().operations.moderation_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<board_create_operation>::value:
               case operation::tag<board_update_operation>::value:
               case operation::tag<board_add_mod_operation>::value:
               case operation::tag<board_add_admin_operation>::value:
               case operation::tag<board_vote_mod_operation>::value:
               case operation::tag<board_transfer_ownership_operation>::value:
               case operation::tag<board_join_request_operation>::value:
               case operation::tag<board_join_accept_operation>::value:
               case operation::tag<board_join_invite_operation>::value:
               case operation::tag<board_invite_accept_operation>::value:
               case operation::tag<board_remove_member_operation>::value:
               case operation::tag<board_blacklist_operation>::value:
               case operation::tag<board_subscribe_operation>::value:
               {
                  results.back().operations.board_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<ad_creative_operation>::value:
               case operation::tag<ad_campaign_operation>::value:
               case operation::tag<ad_inventory_operation>::value:
               case operation::tag<ad_audience_operation>::value:
               case operation::tag<ad_bid_operation>::value:
               {
                  results.back().operations.ad_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<transfer_operation>::value:
               case operation::tag<transfer_request_operation>::value:
               case operation::tag<transfer_accept_operation>::value:
               case operation::tag<transfer_recurring_operation>::value:
               case operation::tag<transfer_recurring_request_operation>::value:
               case operation::tag<transfer_recurring_accept_operation>::value:
               case operation::tag<author_reward_operation>::value:
               case operation::tag<curation_reward_operation>::value:
               case operation::tag<comment_benefactor_reward_operation>::value:
               {
                  results.back().operations.transfer_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<claim_reward_balance_operation>::value:
               case operation::tag<stake_asset_operation>::value:
               case operation::tag<unstake_asset_operation>::value:
               case operation::tag<transfer_to_savings_operation>::value:
               case operation::tag<transfer_from_savings_operation>::value:
               case operation::tag<delegate_asset_operation>::value:
               {
                  results.back().operations.balance_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<escrow_transfer_operation>::value:
               case operation::tag<escrow_approve_operation>::value:
               case operation::tag<escrow_dispute_operation>::value:
               case operation::tag<escrow_release_operation>::value:
               {
                  results.back().operations.escrow_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<limit_order_operation>::value:
               case operation::tag<margin_order_operation>::value:
               case operation::tag<call_order_operation>::value:
               case operation::tag<fill_order_operation>::value:
               {
                  results.back().operations.trading_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<liquidity_pool_create_operation>::value:
               case operation::tag<liquidity_pool_exchange_operation>::value:
               case operation::tag<liquidity_pool_fund_operation>::value:
               case operation::tag<liquidity_pool_withdraw_operation>::value:
               {
                  results.back().operations.liquidity_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<credit_pool_collateral_operation>::value:
               case operation::tag<credit_pool_borrow_operation>::value:
               case operation::tag<credit_pool_lend_operation>::value:
               case operation::tag<credit_pool_withdraw_operation>::value:
               {
                  results.back().operations.credit_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<asset_create_operation>::value:          
               case operation::tag<asset_update_operation>::value:
               case operation::tag<asset_issue_operation>::value: 
               case operation::tag<asset_reserve_operation>::value: 
               case operation::tag<asset_update_issuer_operation>::value:        
               case operation::tag<asset_update_feed_producers_operation>::value:         
               case operation::tag<asset_publish_feed_operation>::value: 
               case operation::tag<asset_settle_operation>::value:  
               case operation::tag<asset_global_settle_operation>::value:
               {
                  results.back().operations.asset_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<account_producer_vote_operation>::value:
               case operation::tag<account_update_proxy_operation>::value:
               case operation::tag<update_network_officer_operation>::value:
               case operation::tag<network_officer_vote_operation>::value:
               case operation::tag<update_executive_board_operation>::value:
               case operation::tag<executive_board_vote_operation>::value:
               case operation::tag<update_governance_operation>::value:
               case operation::tag<subscribe_governance_operation>::value:
               case operation::tag<update_supernode_operation>::value:
               case operation::tag<update_interface_operation>::value:
               case operation::tag<update_mediator_operation>::value:
               case operation::tag<create_community_enterprise_operation>::value:
               case operation::tag<claim_enterprise_milestone_operation>::value:
               case operation::tag<approve_enterprise_milestone_operation>::value:
               case operation::tag<producer_update_operation>::value:
               case operation::tag<proof_of_work_operation>::value:
               case operation::tag<producer_reward_operation>::value:
               case operation::tag<verify_block_operation>::value:
               case operation::tag<commit_block_operation>::value:
               case operation::tag<producer_violation_operation>::value:
               {
                  results.back().operations.network_history[ item.first ] = item.second;
               }
               break;
               case operation::tag<custom_operation>::value:
               case operation::tag<custom_json_operation>::value:
               default:
                  results.back().operations.other_history[ item.first ] = item.second;
            }
         }
      }
   }
   return results;
}

map< uint32_t, applied_operation > database_api::get_account_history( string account, uint64_t from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_history( account, from, limit );
   });
}

map< uint32_t, applied_operation > database_api_impl::get_account_history( string account, uint64_t from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 10000 ) );
   FC_ASSERT( from >= limit,
      "From must be greater than limit." );

   const auto& idx = _db.get_index< account_history_index >().indices().get< by_account >();
   auto itr = idx.lower_bound( boost::make_tuple( account, from ) );

   uint32_t n = 0;
   map<uint32_t, applied_operation> results;
   
   while( true )
   {
      if( itr == idx.end() )
         break;
      if( itr->account != account )
         break;
      if( n >= limit )
         break;
      results[ itr->sequence ] = _db.get( itr->op );
      ++itr;
      ++n;
   }
   return results;
}

vector< balance_state > database_api::get_balances( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_balances( names );
   });
}

vector< balance_state > database_api_impl::get_balances( vector< string > names )const
{
   const auto& balance_idx  = _db.get_index< account_balance_index >().indices().get< by_owner >();

   vector< balance_state > results;

   for( auto name: names )
   {
      balance_state bstate;
      auto balance_itr = balance_idx.lower_bound( name );
      while( balance_itr != balance_idx.end() && balance_itr->owner == name )
      {
         bstate.balances[ balance_itr->symbol ] = account_balance_api_obj( *balance_itr );
      }
      results.push_back( bstate );
   }

   return results;
}

vector< message_state > database_api::get_messages( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_messages( names );
   });
}

vector< message_state > database_api_impl::get_messages( vector< string > names )const
{
   const auto& inbox_idx = _db.get_index< message_index >().indices().get< by_account_inbox >();
   const auto& outbox_idx = _db.get_index< message_index >().indices().get< by_account_outbox >();

   vector< message_state > results;

   for( auto name: names )
   {
      auto inbox_itr = inbox_idx.lower_bound( name );
      auto outbox_itr = outbox_idx.lower_bound( name );
      vector< message_api_obj > inbox;
      vector< message_api_obj > outbox;
      map< account_name_type, vector< message_api_obj > > conversations;

      while( inbox_itr != inbox_idx.end() && inbox_itr->recipient == name )
      {
         inbox.push_back( message_api_obj( *inbox_itr ) );
      }

      while( outbox_itr != outbox_idx.end() && outbox_itr->sender == name )
      {
         outbox.push_back( message_api_obj( *outbox_itr ) );
      }

      for( auto message : inbox )
      {
         conversations[ message.sender ].push_back( message );
      }

      for( auto message : outbox )
      {
         conversations[ message.recipient ].push_back( message );
      }

      for( auto conv : conversations )
      {
         vector< message_api_obj > thread = conv.second;
         std::sort( thread.begin(), thread.end(), [&]( auto a, auto b )
         {
            return a.created < b.created;
         });
         conversations[ conv.first ] = thread;
      }

      message_state mstate;
      mstate.inbox = inbox;
      mstate.outbox = outbox;
      mstate.conversations = conversations;
      results.push_back( mstate );
   }

   return results;
}

vector< key_state > database_api::get_keychains( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_keychains( names );
   });
}

vector< key_state > database_api_impl::get_keychains( vector< string > names )const
{
   const auto& connection_a_idx = _db.get_index< connection_index >().indices().get< by_account_a >();
   const auto& connection_b_idx = _db.get_index< connection_index >().indices().get< by_account_b >();
   const auto& board_idx = _db.get_index< board_member_key_index >().indices().get< by_member_board >();
   const auto& business_idx = _db.get_index< account_member_key_index >().indices().get< by_member_business >();

   vector< key_state > results;

   for( auto name : names )
   {
      key_state kstate;

      auto connection_a_itr = connection_a_idx.lower_bound( boost::make_tuple( name, CONNECTION ) );
      auto connection_b_itr = connection_b_idx.lower_bound( boost::make_tuple( name, CONNECTION ) );
      while( connection_a_itr != connection_a_idx.end() && 
         connection_a_itr->account_a == name &&
         connection_a_itr->connection_type == CONNECTION )
      {
         kstate.connection_keys[ connection_a_itr->account_b ] = connection_a_itr->encrypted_key_b;
         ++connection_a_itr;
      }
      while( connection_b_itr != connection_b_idx.end() && 
         connection_b_itr->account_b == name &&
         connection_b_itr->connection_type == CONNECTION )
      {
         kstate.connection_keys[ connection_b_itr->account_a ] = connection_b_itr->encrypted_key_a;
         ++connection_b_itr;
      }

      auto connection_a_itr = connection_a_idx.lower_bound( boost::make_tuple( name, FRIEND ) );
      auto connection_b_itr = connection_b_idx.lower_bound( boost::make_tuple( name, FRIEND ) );
      while( connection_a_itr != connection_a_idx.end() && 
         connection_a_itr->account_a == name &&
         connection_a_itr->connection_type == FRIEND )
      {
         kstate.friend_keys[ connection_a_itr->account_b ] = connection_a_itr->encrypted_key_b;
         ++connection_a_itr;
      }
      while( connection_b_itr != connection_b_idx.end() && 
         connection_b_itr->account_b == name &&
         connection_b_itr->connection_type == FRIEND )
      {
         kstate.friend_keys[ connection_b_itr->account_a ] = connection_b_itr->encrypted_key_a;
         ++connection_b_itr;
      }

      auto connection_a_itr = connection_a_idx.lower_bound( boost::make_tuple( name, COMPANION ) );
      auto connection_b_itr = connection_b_idx.lower_bound( boost::make_tuple( name, COMPANION ) );
      while( connection_a_itr != connection_a_idx.end() && 
         connection_a_itr->account_a == name &&
         connection_a_itr->connection_type == COMPANION )
      {
         kstate.companion_keys[ connection_a_itr->account_b ] = connection_a_itr->encrypted_key_b;
         ++connection_a_itr;
      }
      while( connection_b_itr != connection_b_idx.end() && 
         connection_b_itr->account_b == name &&
         connection_b_itr->connection_type == COMPANION )
      {
         kstate.companion_keys[ connection_b_itr->account_a ] = connection_b_itr->encrypted_key_a;
         ++connection_b_itr;
      }

      auto board_itr = board_idx.lower_bound( name );
      while( board_itr != board_idx.end() && 
         board_itr->member == name )
      {
         kstate.board_keys[ board_itr->board ] = board_itr->encrypted_board_key;
         ++board_itr;
      }

      auto business_itr = business_idx.lower_bound( name );
      while( business_itr != business_idx.end() && 
         business_itr->member == name )
      {
         kstate.business_keys[ business_itr->business_account ] = business_itr->encrypted_business_key;
         ++business_itr;
      }
      results.push_back( kstate );
   }
   return results;
}

set< string > database_api::lookup_accounts( string lower_bound_name, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->lookup_accounts( lower_bound_name, limit );
   });
}

set< string > database_api_impl::lookup_accounts( string lower_bound_name, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   const auto& accounts_by_name = _db.get_index< account_index >().indices().get< by_name >();
   set< string > results;

   for( auto acc_itr = accounts_by_name.lower_bound( lower_bound_name );
      limit-- && acc_itr != accounts_by_name.end();
      ++acc_itr )
   {
      results.insert( acc_itr->name );
   }

   return results;
}

uint64_t database_api::get_account_count()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_count();
   });
}

uint64_t database_api_impl::get_account_count()const
{
   return _db.get_index< account_index >().indices().size();
}

vector< owner_authority_history_api_obj > database_api::get_owner_history( string account )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_owner_history( account );
   });
}

vector< owner_authority_history_api_obj > database_api_impl::get_owner_history( string account )const
{
   vector< owner_authority_history_api_obj > results;

   const auto& hist_idx = _db.get_index< owner_authority_history_index >().indices().get< by_account >();
   auto itr = hist_idx.lower_bound( account );

   while( itr != hist_idx.end() && itr->account == account )
   {
      results.push_back( owner_authority_history_api_obj( *itr ) );
      ++itr;
   }

   return results;
}

optional< account_recovery_request_api_obj > database_api::get_recovery_request( string account )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_recovery_request( account );
   });
}

optional< account_recovery_request_api_obj > database_api_impl::get_recovery_request( string account )const
{
   optional< account_recovery_request_api_obj > results;

   const auto& rec_idx = _db.get_index< account_recovery_request_index >().indices().get< by_account >();
   auto req = rec_idx.find( account );

   if( req != rec_idx.end() )
   {
      results = account_recovery_request_api_obj( *req );
   }

   return results;
}

optional< account_bandwidth_api_obj > database_api::get_account_bandwidth( string account, producer::bandwidth_type type )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_bandwidth( account, type );
   });
}

optional< account_bandwidth_api_obj > database_api_impl::get_account_bandwidth( string account, producer::bandwidth_type type )const
{
   optional< account_bandwidth_api_obj > results;
   if( _db.has_index< producer::account_bandwidth_index >() )
   {
      auto band = _db.find< producer::account_bandwidth_object, producer::by_account_bandwidth_type >( boost::make_tuple( account, type ) );
      if( band != nullptr )
      {
         results = *band;
      }  
   }
   return results;
}


   //================//
   // === Assets === //
   //================//


vector< extended_asset > database_api::get_assets( vector< string > assets )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_assets( assets );
   });
}

vector< extended_asset > database_api_impl::get_assets( vector< string > assets )const
{ 
   vector< extended_asset > results;

   for( auto asset : assets )
   {
      const auto& asset_idx = _db.get_index< asset_index >().indices().get< by_symbol >();
      const auto& asset_dyn_idx = _db.get_index< asset_dynamic_data_index >().indices().get< by_symbol >();
      
      auto asset_itr = asset_idx.find( asset );
      if( asset_itr != asset_idx.end() )
      {
         results.push_back( extended_asset( *asset_itr ) );
      }
      
      auto asset_dyn_itr = asset_dyn_idx.find( asset );
      if( asset_dyn_itr != asset_dyn_idx.end() )
      {
         results.back().total_supply = asset_dyn_itr->total_supply.value;
         results.back().liquid_supply = asset_dyn_itr->liquid_supply.value;
         results.back().reward_supply = asset_dyn_itr->reward_supply.value;
         results.back().savings_supply = asset_dyn_itr->savings_supply.value;
         results.back().delegated_supply = asset_dyn_itr->delegated_supply.value;
         results.back().receiving_supply = asset_dyn_itr->receiving_supply.value;
         results.back().pending_supply = asset_dyn_itr->pending_supply.value;
         results.back().confidential_supply = asset_dyn_itr->confidential_supply.value;
      }

      const auto& bitasset_idx = _db.get_index< asset_bitasset_data_index >().indices().get< by_symbol >();
      auto bitasset_itr = bitasset_idx.find( asset );
      if( bitasset_itr != bitasset_idx.end() )
      {
         results.back().bitasset = bitasset_data_api_obj( *bitasset_itr );
      }

      const auto& equity_idx = _db.get_index< asset_equity_data_index >().indices().get< by_symbol >();
      auto equity_itr = equity_idx.find( asset );
      if( equity_itr != equity_idx.end() )
      {
         results.back().equity = equity_data_api_obj( *equity_itr );
      }

      const auto& credit_idx = _db.get_index< asset_credit_data_index >().indices().get< by_symbol >();
      auto credit_itr = credit_idx.find( asset );
      if( credit_itr != credit_idx.end() )
      {
         results.back().credit = credit_data_api_obj( *credit_itr );
      }

      const auto& credit_pool_idx = _db.get_index< asset_credit_pool_index >().indices().get< by_base_symbol >();
      auto credit_pool_itr = credit_pool_idx.find( asset );
      if( credit_pool_itr != credit_pool_idx.end() )
      {
         results.back().credit_pool = credit_pool_api_obj( *credit_pool_itr );
      }

      const auto& pool_a_idx = _db.get_index< asset_liquidity_pool_index >().indices().get< by_symbol_a >();
      const auto& pool_b_idx = _db.get_index< asset_liquidity_pool_index >().indices().get< by_symbol_b >();
      auto pool_a_itr = pool_a_idx.lower_bound( asset );
      auto pool_b_itr = pool_b_idx.lower_bound( asset );

      while( pool_a_itr != pool_a_idx.end() && pool_a_itr->symbol_a == asset )
      {
         results.back().liquidity_pools[ pool_a_itr->symbol_b ] = liquidity_pool_api_obj( *pool_a_itr );
      }

      while( pool_b_itr != pool_b_idx.end() && pool_b_itr->symbol_b == asset )
      {
         results.back().liquidity_pools[ pool_b_itr->symbol_a ] = liquidity_pool_api_obj( *pool_b_itr );
      }
   }
   return results;
}

uint64_t database_api::get_asset_count()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_asset_count();
   });
}


uint64_t database_api_impl::get_asset_count()const
{
   return _db.get_index< asset_index >().indices().size();
}


optional< escrow_api_obj > database_api::get_escrow( string from, string escrow_id )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_escrow( from, escrow_id );
   });
}


optional< escrow_api_obj > database_api_impl::get_escrow( string from, string escrow_id )const
{
   optional< escrow_api_obj > results;
   shared_string eid;
   from_string( eid, escrow_id );
   try
   {
      results = _db.get_escrow( from, eid );
   }
   catch ( ... ) {}

   return results;
}

vector< withdraw_route > database_api::get_withdraw_routes( string account, withdraw_route_type type )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_withdraw_routes( account, type );
   });
}

vector< withdraw_route > database_api_impl::get_withdraw_routes( string account, withdraw_route_type type )const
{
   vector< withdraw_route > results;
   const account_object& acc = _db.get_account( account );

   if( type == outgoing || type == all )
   {
      const auto& by_route = _db.get_index< unstake_asset_route_index >().indices().get< by_withdraw_route >();
      auto route_itr = by_route.lower_bound( acc.name );

      while( route_itr != by_route.end() && route_itr->from == acc.name )
      {
         withdraw_route r;
         r.from = route_itr->from;
         r.to = route_itr->to;
         r.percent = route_itr->percent;
         r.auto_stake = route_itr->auto_stake;

         results.push_back( r );

         ++route_itr;
      }
   }

   if( type == incoming || type == all )
   {
      const auto& by_dest = _db.get_index< unstake_asset_route_index >().indices().get< by_destination >();
      auto route_itr = by_dest.lower_bound( acc.name );

      while( route_itr != by_dest.end() && route_itr->to == acc.name )
      {
         withdraw_route r;

         r.from = route_itr->from;
         r.to = route_itr->to;
         r.percent = route_itr->percent;
         r.auto_stake = route_itr->auto_stake;

         results.push_back( r );

         ++route_itr;
      }
   }

   return results;
}


vector< savings_withdraw_api_obj > database_api::get_savings_withdraw_from( string account )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_savings_withdraw_from( account );
   });
}

vector< savings_withdraw_api_obj > database_api_impl::get_savings_withdraw_from( string account )const
{
   vector< savings_withdraw_api_obj > results;
   const auto& from_rid_idx = _db.get_index< savings_withdraw_index >().indices().get< by_request_id >();
   auto itr = from_rid_idx.lower_bound( account );
   while( itr != from_rid_idx.end() && itr->from == account )
   {
      results.push_back( savings_withdraw_api_obj( *itr ) );
      ++itr;
   }
   return results;
}

vector< savings_withdraw_api_obj > database_api::get_savings_withdraw_to( string account )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_savings_withdraw_to( account );
   });
}

vector< savings_withdraw_api_obj > database_api_impl::get_savings_withdraw_to( string account )const
{
   vector< savings_withdraw_api_obj > results;
   const auto& to_complete_idx = _db.get_index< savings_withdraw_index >().indices().get< by_to_complete >();
   auto itr = to_complete_idx.lower_bound( account );
   while( itr != to_complete_idx.end() && itr->to == account ) {
      results.push_back( savings_withdraw_api_obj( *itr ) );
      ++itr;
   }
   return results;
}

vector< asset_delegation_api_obj > database_api::get_asset_delegations( string account, string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_asset_delegations( account, from, limit );
   });
}

vector< asset_delegation_api_obj > database_api_impl::get_asset_delegations( string account, string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< asset_delegation_api_obj > results;
   results.reserve( limit );

   const auto& delegation_idx = _db.get_index< asset_delegation_index, by_delegation >();
   auto itr = delegation_idx.lower_bound( boost::make_tuple( account, from ) );
   while( results.size() < limit && itr != delegation_idx.end() && itr->delegator == account )
   {
      results.push_back( *itr );
      ++itr;
   }

   return results;
}

vector< asset_delegation_expiration_api_obj > database_api::get_expiring_asset_delegations( string account, time_point from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_expiring_asset_delegations( account, from, limit );
   });
}


vector< asset_delegation_expiration_api_obj > database_api_impl::get_expiring_asset_delegations( string account, time_point from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< asset_delegation_expiration_api_obj > results;
   results.reserve( limit );

   const auto& exp_idx = _db.get_index< asset_delegation_expiration_index, by_account_expiration >();
   auto itr = exp_idx.lower_bound( boost::make_tuple( account, from ) );
   while( results.size() < limit && itr != exp_idx.end() && itr->delegator == account )
   {
      results.push_back( *itr );
      ++itr;
   }

   return results;
}



   //================//
   // === Boards === //
   //================//


vector< extended_board > database_api::get_boards( vector< string > boards )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_boards( boards );
   });
}

vector< extended_board > database_api_impl::get_boards( vector< string > boards )const
{
   vector< extended_board > results;
   const auto& board_idx = _db.get_index< board_index >().indices().get< by_name >();
   const auto& board_mem_idx = _db.get_index< board_member_index >().indices().get< by_name >();
   const auto& board_inv_idx = _db.get_index< board_join_invite_index >().indices().get< by_board >();
   const auto& board_req_idx = _db.get_index< board_join_request_index >().indices().get< by_board_account >();

   for( auto board : boards )
   {
      auto board_itr = board_idx.find( board );
      if( board_itr != board_idx.end() )
      results.push_back( extended_board( *board_itr ) );
      auto board_mem_itr = board_mem_idx.find( board );
      if( board_mem_itr != board_mem_idx.end() )
      {
         for( auto sub : board_mem_itr->subscribers )
         {
            results.back().subscribers.push_back( sub );
         }
         for( auto mem : board_mem_itr->members )
         {
            results.back().members.push_back( mem );
         }
         for( auto mod : board_mem_itr->moderators )
         {
            results.back().moderators.push_back( mod );
         }
         for( auto admin : board_mem_itr->administrators )
         {
            results.back().administrators.push_back( admin );
         }
         for( auto bl : board_mem_itr->blacklist )
         {
            results.back().blacklist.push_back( bl );
         }
         for( auto weight : board_mem_itr->mod_weight )
         {
            results.back().mod_weight[ weight.first ] = weight.second.value;
         }
         results.back().total_mod_weight = board_mem_itr->total_mod_weight.value;
      }

      auto board_inv_itr = board_inv_idx.lower_bound( board );
      auto board_req_itr = board_req_idx.lower_bound( board );

      while( board_inv_itr != board_inv_idx.end() && board_inv_itr->board == board )
      {
         results.back().invites[ board_inv_itr->member ] = board_invite_api_obj( *board_inv_itr );
         ++board_inv_itr;
      }

      while( board_req_itr != board_req_idx.end() && board_req_itr->board == board )
      {
         results.back().requests[ board_inv_itr->account ] = board_request_api_obj( *board_req_itr );
         ++board_req_itr;
      }
   }
   return results;
}

vector< extended_board > database_api::get_boards_by_subscribers( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_boards_by_subscribers( from, limit );
   });
}

vector< extended_board > database_api_impl::get_boards_by_subscribers( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< extended_board > results;
   results.reserve( limit );

   const auto& board_idx = _db.get_index< board_index >().indices().get< by_subscriber_count >();
   const auto& name_idx = _db.get_index< board_index >().indices().get< by_name >();
   const auto& board_mem_idx = _db.get_index< board_member_index >().indices().get< by_name >();
   const auto& board_inv_idx = _db.get_index< board_join_invite_index >().indices().get< by_board >();
   const auto& board_req_idx = _db.get_index< board_join_request_index >().indices().get< by_board_account >();

   auto board_itr = board_idx.begin();
  
   if( from.size() )
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid Board name ${n}", ("n",from) );
      board_itr = board_idx.iterator_to( *name_itr );
   }

   while( board_itr != board_idx.end() && results.size() < limit )
   {
      results.push_back( extended_board( *board_itr ) );
      board_name_type board = board_itr->name;
      auto board_mem_itr = board_mem_idx.find( board );
      if( board_mem_itr != board_mem_idx.end() )
      {
         for( auto sub : board_mem_itr->subscribers )
         {
            results.back().subscribers.push_back( sub );
         }
         for( auto mem : board_mem_itr->members )
         {
            results.back().members.push_back( mem );
         }
         for( auto mod : board_mem_itr->moderators )
         {
            results.back().moderators.push_back( mod );
         }
         for( auto admin : board_mem_itr->administrators )
         {
            results.back().administrators.push_back( admin );
         }
         for( auto bl : board_mem_itr->blacklist )
         {
            results.back().blacklist.push_back( bl );
         }
         for( auto weight : board_mem_itr->mod_weight )
         {
            results.back().mod_weight[ weight.first ] = weight.second.value;
         }
         results.back().total_mod_weight = board_mem_itr->total_mod_weight.value;
      }

      auto board_inv_itr = board_inv_idx.lower_bound( board );
      auto board_req_itr = board_req_idx.lower_bound( board );

      while( board_inv_itr != board_inv_idx.end() && board_inv_itr->board == board )
      {
         results.back().invites[ board_inv_itr->member ] = board_invite_api_obj( *board_inv_itr );
         ++board_inv_itr;
      }

      while( board_req_itr != board_req_idx.end() && board_req_itr->board == board )
      {
         results.back().requests[ board_inv_itr->account ] = board_request_api_obj( *board_req_itr );
         ++board_req_itr;
      }
   }
   return results;
}

uint64_t database_api::get_board_count()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_board_count();
   });
}

uint64_t database_api_impl::get_board_count()const
{
   return _db.get_index< board_index >().indices().size();
}


   //=================//
   // === Network === //
   //=================//


vector< account_name_type > database_api::get_active_producers()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_active_producers();
   });
}

vector< account_name_type > database_api_impl::get_active_producers()const
{
   const producer_schedule_object& pso = _db.get_producer_schedule();
   size_t n = pso.current_shuffled_producers.size();
   vector< account_name_type > results;
   results.reserve( n );
   for( size_t i=0; i<n; i++ )
   {
      results.push_back( pso.current_shuffled_producers[i] );
   }
      
   return results;
}

set< account_name_type > database_api::lookup_producer_accounts( string lower_bound_name, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->lookup_producer_accounts( lower_bound_name, limit );
   });
}

set< account_name_type > database_api_impl::lookup_producer_accounts( string lower_bound_name, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   const auto& producers_by_id = _db.get_index< producer_index >().indices().get< by_id >();

   set< account_name_type > producers_by_account_name;

   for( const producer_api_obj& producer : producers_by_id )
   {
      if( producer.owner >= lower_bound_name )
      {
         producers_by_account_name.insert( producer.owner );
      } 
   }
      
   auto end_iter = producers_by_account_name.begin();
   while ( end_iter != producers_by_account_name.end() && limit-- )
   {
      ++end_iter;
   }
   producers_by_account_name.erase( end_iter, producers_by_account_name.end() );
   return producers_by_account_name;
}

uint64_t database_api::get_producer_count()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_producer_count();
   });
}

uint64_t database_api_impl::get_producer_count()const
{
   return _db.get_index<producer_index>().indices().size();
}

vector< producer_api_obj > database_api::get_producers_by_account( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_producers_by_account( names );
   });
}

vector< producer_api_obj > database_api_impl::get_producers_by_account( vector< string > names )const
{
   vector< producer_api_obj > results;
   results.reserve( names.size() );

   const auto& producer_idx = _db.get_index< producer_index >().indices().get< by_name >();

   for( auto producer : names )
   {
      auto producer_itr = producer_idx.find( producer );
      if( producer_itr != producer_idx.end() )
      {
         results.push_back( producer_api_obj( *producer_itr ) );
      }
   }
   return results;
}

vector< producer_api_obj > database_api::get_producers_by_voting_power( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_producers_by_voting_power( from, limit );
   });
}

vector< producer_api_obj > database_api_impl::get_producers_by_voting_power( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< producer_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< producer_index >().indices().get< by_name >();
   const auto& vote_idx = _db.get_index< producer_index >().indices().get< by_voting_power >();

   auto itr = vote_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid producer name ${n}", ("n",from) );
      itr = vote_idx.iterator_to( *name_itr );
   }

   while( itr != vote_idx.end() && results.size() < limit && itr->vote_count > 0 )
   {
      results.push_back( producer_api_obj( *itr ) );
      ++itr;
   }
   return results;
}

vector< producer_api_obj > database_api::get_producers_by_mining_power( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_producers_by_mining_power( from, limit );
   });
}

vector< producer_api_obj > database_api_impl::get_producers_by_mining_power( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< producer_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< producer_index >().indices().get< by_name >();
   const auto& mining_idx = _db.get_index< producer_index >().indices().get< by_mining_power >();

   auto itr = mining_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid producer name ${n}", ("n",from) );
      itr = mining_idx.iterator_to( *name_itr );
   }

   while( itr != mining_idx.end() && results.size() < limit && itr->mining_count > 0 )
   {
      results.push_back( producer_api_obj( *itr ) );
      ++itr;
   }
   return results;
}

vector< network_officer_api_obj > database_api::get_network_officers_by_account( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_network_officers_by_account( names );
   });
}

vector< network_officer_api_obj > database_api_impl::get_network_officers_by_account( vector< string > names )const
{
   vector< network_officer_api_obj > results;
   results.reserve( names.size() );

   const auto& officer_idx = _db.get_index< network_officer_index >().indices().get< by_account >();

   for( auto officer : names )
   {
      auto officer_itr = officer_idx.find( officer );
      if( officer_itr != officer_idx.end() )
      {
         results.push_back( network_officer_api_obj( *officer_itr ) );
      }
   }
   return results;
}


vector< network_officer_api_obj > database_api::get_development_officers_by_voting_power( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_development_officers_by_voting_power( from, limit );
   });
}

vector< network_officer_api_obj > database_api_impl::get_development_officers_by_voting_power( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< network_officer_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< network_officer_index >().indices().get< by_account >();
   const auto& vote_idx = _db.get_index< network_officer_index >().indices().get< by_type_voting_power >();

   auto itr = vote_idx.lower_bound( DEVELOPMENT );

   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid network officer name ${n}", ("n",from) );
      itr = vote_idx.iterator_to( *name_itr );
   }

   while( itr != vote_idx.end() && results.size() < limit && itr->vote_count > 0 && itr->officer_type == DEVELOPMENT )
   {
      results.push_back( network_officer_api_obj( *itr ) );
      ++itr;
   }
   return results;
}

vector< network_officer_api_obj > database_api::get_marketing_officers_by_voting_power( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_marketing_officers_by_voting_power( from, limit );
   });
}

vector< network_officer_api_obj > database_api_impl::get_marketing_officers_by_voting_power( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< network_officer_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< network_officer_index >().indices().get< by_account >();
   const auto& vote_idx = _db.get_index< network_officer_index >().indices().get< by_type_voting_power >();

   auto itr = vote_idx.lower_bound( MARKETING );

   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(), "Invalid network officer name ${n}", ("n",from) );
      itr = vote_idx.iterator_to( *name_itr );
   }

   while( itr != vote_idx.end() && results.size() < limit && itr->vote_count > 0 && itr->officer_type == MARKETING )
   {
      results.push_back( network_officer_api_obj( *itr ) );
      ++itr;
   }
   return results;
}

vector< network_officer_api_obj > database_api::get_advocacy_officers_by_voting_power( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_advocacy_officers_by_voting_power( from, limit );
   });
}

vector< network_officer_api_obj > database_api_impl::get_advocacy_officers_by_voting_power( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< network_officer_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< network_officer_index >().indices().get< by_account >();
   const auto& vote_idx = _db.get_index< network_officer_index >().indices().get< by_type_voting_power >();

   auto itr = vote_idx.lower_bound( ADVOCACY );

   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid network officer name ${n}", ("n",from) );
      itr = vote_idx.iterator_to( *name_itr );
   }

   while( itr != vote_idx.end() && results.size() < limit && itr->vote_count > 0 && itr->officer_type == ADVOCACY )
   {
      results.push_back( network_officer_api_obj( *itr ) );
      ++itr;
   }
   return results;
}


vector< executive_board_api_obj > database_api::get_executive_boards_by_account( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_executive_boards_by_account( names );
   });
}

vector< executive_board_api_obj > database_api_impl::get_executive_boards_by_account( vector< string > names )const
{
   vector< executive_board_api_obj > results;
   results.reserve( names.size() );

   const auto& exec_idx = _db.get_index< executive_board_index >().indices().get< by_account >();

   for( auto exec : names )
   {
      auto exec_itr = exec_idx.find( exec );
      if( exec_itr != exec_idx.end() )
      {
         results.push_back( executive_board_api_obj( *exec_itr ) );
      }
   }
  
   return results;
}


vector< executive_board_api_obj > database_api::get_executive_boards_by_voting_power( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_executive_boards_by_voting_power( from, limit );
   });
}

vector< executive_board_api_obj > database_api_impl::get_executive_boards_by_voting_power( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< executive_board_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< executive_board_index >().indices().get< by_account >();
   const auto& vote_idx = _db.get_index< executive_board_index >().indices().get< by_voting_power >();

   auto itr = vote_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid executive board name ${n}", ("n",from) );
      itr = vote_idx.iterator_to( *name_itr );
   }

   while( itr != vote_idx.end() && results.size() < limit && itr->vote_count > 0 )
   {
      results.push_back( executive_board_api_obj( *itr ) );
      ++itr;
   }
   return results;
}

vector< supernode_api_obj > database_api::get_supernodes_by_account( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_supernodes_by_account( names );
   });
}

vector< supernode_api_obj > database_api_impl::get_supernodes_by_account( vector< string > names )const
{
   vector< supernode_api_obj > results;
   results.reserve( names.size() );

   const auto& sn_idx = _db.get_index< supernode_index >().indices().get< by_account >();

   for( auto sn : names )
   {
      auto sn_itr = sn_idx.find( sn );
      if( sn_itr != sn_idx.end() )
      {
         results.push_back( supernode_api_obj( *sn_itr ) );
      }
   }
  
   return results;
}

vector< supernode_api_obj > database_api::get_supernodes_by_view_weight( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_supernodes_by_view_weight( from, limit );
   });
}

vector< supernode_api_obj > database_api_impl::get_supernodes_by_view_weight( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );
   vector< supernode_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< supernode_index >().indices().get< by_account >();
   const auto& view_idx = _db.get_index< supernode_index >().indices().get< by_view_weight >();

   auto itr = view_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid supernode name ${n}", ("n",from) );
      itr = view_idx.iterator_to( *name_itr );
   }

   while( itr != view_idx.end() && results.size() < limit && itr->monthly_active_users > 0 )
   {
      results.push_back( supernode_api_obj( *itr ) );
      ++itr;
   }
   return results;
}

vector< interface_api_obj > database_api::get_interfaces_by_account( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_interfaces_by_account( names );
   });
}

vector< interface_api_obj > database_api_impl::get_interfaces_by_account( vector< string > names )const
{
   vector< interface_api_obj > results;
   results.reserve( names.size() );

   const auto& interface_idx = _db.get_index< interface_index >().indices().get< by_account >();

   for( auto interface : names )
   {
      auto interface_itr = interface_idx.find( interface );
      if( interface_itr != interface_idx.end() )
      {
         results.push_back( interface_api_obj( *interface_itr ) );
      }
   }
  
   return results;
}

vector< interface_api_obj > database_api::get_interfaces_by_users( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_interfaces_by_users( from, limit );
   });
}

vector< interface_api_obj > database_api_impl::get_interfaces_by_users( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );

   vector< interface_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< interface_index >().indices().get< by_account >();
   const auto& user_idx = _db.get_index< interface_index >().indices().get< by_monthly_active_users >();

   auto itr = user_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid interface name ${n}", ("n",from) );
      itr = user_idx.iterator_to( *name_itr );
   }

   while( itr != user_idx.end() && results.size() < limit && itr->monthly_active_users > 0 )
   {
      results.push_back( interface_api_obj( *itr ) );
      ++itr;
   }
   return results;
}

vector< governance_account_api_obj > database_api::get_governance_accounts_by_account( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_governance_accounts_by_account( names );
   });
}

vector< governance_account_api_obj > database_api_impl::get_governance_accounts_by_account( vector< string > names )const
{
   vector< governance_account_api_obj > results;
   results.reserve( names.size() );

   const auto& governance_account_idx = _db.get_index< governance_account_index >().indices().get< by_account >();

   for( auto governance_account : names )
   {
      auto governance_account_itr = governance_account_idx.find( governance_account );
      if( governance_account_itr != governance_account_idx.end() )
      {
         results.push_back( governance_account_api_obj( *governance_account_itr ) );
      }
   }
  
   return results;
}

vector< governance_account_api_obj > database_api::get_governance_accounts_by_subscriber_power( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_governance_accounts_by_subscriber_power( from, limit );
   });
}

vector< governance_account_api_obj > database_api_impl::get_governance_accounts_by_subscriber_power( string from, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );

   vector< governance_account_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< governance_account_index >().indices().get< by_account >();
   const auto& sub_idx = _db.get_index< governance_account_index >().indices().get< by_subscriber_power >();

   auto itr = sub_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( from );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid governance account name ${n}", ("n",from) );
      itr = sub_idx.iterator_to( *name_itr );
   }

   while( itr != sub_idx.end() && results.size() < limit && itr->subscriber_count > 0 )
   {
      results.push_back( governance_account_api_obj( *itr ) );
      ++itr;
   }
   return results;
}

vector< community_enterprise_api_obj > database_api::get_enterprise_by_voting_power( string from, string from_id, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_enterprise_by_voting_power( from, from_id, limit );
   });
}

vector< community_enterprise_api_obj > database_api_impl::get_enterprise_by_voting_power( string from, string from_id, uint32_t limit )const
{
   limit = std::min( limit, uint32_t( 1000 ) );

   vector< community_enterprise_api_obj > results;
   results.reserve( limit );

   const auto& name_idx = _db.get_index< community_enterprise_index >().indices().get< by_enterprise_id >();
   const auto& vote_idx = _db.get_index< community_enterprise_index >().indices().get< by_total_voting_power >();

   auto itr = vote_idx.begin();
   if( from.size() ) 
   {
      auto name_itr = name_idx.find( boost::make_tuple( from, from_id ) );
      FC_ASSERT( name_itr != name_idx.end(),
         "Invalid enterprise Creator: ${c} with enterprise_id: ${i}", ("c",from)("i",from_id) );
      itr = vote_idx.iterator_to( *name_itr );
   }

   while( itr != vote_idx.end() && results.size() < limit && itr->total_approvals > 0 )
   {
      results.push_back( community_enterprise_api_obj( *itr ) );
      ++itr;
   }
   return results;
}


   //================//
   // === Market === //
   //================//


vector< order_state > database_api::get_open_orders( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_open_orders( names );
   });
}

vector< order_state > database_api_impl::get_open_orders( vector< string > names )const
{
   vector< order_state > results;
   const auto& limit_idx = _db.get_index< limit_order_index >().indices().get< by_account >();
   const auto& margin_idx = _db.get_index< margin_order_index >().indices().get< by_account >();
   const auto& call_idx = _db.get_index< call_order_index >().indices().get< by_account >();
   const auto& loan_idx = _db.get_index< credit_loan_index >().indices().get< by_owner >();
   const auto& collateral_idx = _db.get_index< credit_collateral_index >().indices().get< by_owner >();

   for( auto name : names )
   {
      order_state ostate;
      auto limit_itr = limit_idx.lower_bound( name );
      while( limit_itr != limit_idx.end() && limit_itr->seller == name ) 
      {
         ostate.limit_orders.push_back( limit_order_api_obj( *limit_itr ) );
         ++limit_itr;
      }

      auto margin_itr = margin_idx.lower_bound( name );
      while( margin_itr != margin_idx.end() && margin_itr->owner == name ) 
      {
         ostate.margin_orders.push_back( margin_order_api_obj( *margin_itr ) );
         ++margin_itr;
      }

      auto call_itr = call_idx.lower_bound( name );
      while( call_itr != call_idx.end() && call_itr->borrower == name ) 
      {
         ostate.call_orders.push_back( call_order_api_obj( *call_itr ) );
         ++call_itr;
      }

      auto loan_itr = loan_idx.lower_bound( name );
      while( loan_itr != loan_idx.end() && loan_itr->owner == name ) 
      {
         ostate.loan_orders.push_back( credit_loan_api_obj( *loan_itr ) );
         ++loan_itr;
      }

      auto collateral_itr = collateral_idx.lower_bound( name );
      while( collateral_itr != collateral_idx.end() && collateral_itr->owner == name ) 
      {
         ostate.collateral.push_back( credit_collateral_api_obj( *collateral_itr ) );
         ++collateral_itr;
      }
   }
   return results;
}

market_limit_orders database_api::get_limit_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_limit_orders( buy_symbol, sell_symbol, limit );
   });
}

market_limit_orders database_api_impl::get_limit_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );
   limit = std::min( limit, uint32_t( 1000 ) );

   market_limit_orders results;

   const auto& limit_price_idx = _db.get_index< limit_order_index >().indices().get< by_price >();

   auto max_sell = price::max( asset_symbol_type( sell_symbol ), asset_symbol_type( buy_symbol ) );
   auto max_buy = price::max( asset_symbol_type( buy_symbol ), asset_symbol_type( sell_symbol ) );
   
   auto limit_sell_itr = limit_price_idx.lower_bound( max_sell );
   auto limit_buy_itr = limit_price_idx.lower_bound( max_buy );
   auto limit_end = limit_price_idx.end();

   while( limit_sell_itr != limit_end &&
      limit_sell_itr->sell_price.base.symbol == asset_symbol_type( sell_symbol ) && 
      results.limit_bids.size() < limit )
   {
      results.limit_bids.push_back( limit_order_api_obj( *limit_sell_itr ) );
      ++limit_sell_itr;
   }
   while( limit_buy_itr != limit_end && 
      limit_buy_itr->sell_price.base.symbol == asset_symbol_type( buy_symbol ) && 
      results.limit_asks.size() < limit )
   {
      results.limit_asks.push_back( limit_order_api_obj( *limit_buy_itr ) );
      ++limit_buy_itr;  
   }
   return results;
}

market_margin_orders database_api::get_margin_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_margin_orders( buy_symbol, sell_symbol, limit );
   });
}

market_margin_orders database_api_impl::get_margin_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );
   limit = std::min( limit, uint32_t( 1000 ) );

   market_margin_orders results;

   const auto& margin_price_idx = _db.get_index< margin_order_index >().indices().get< by_price >();

   auto max_sell = price::max( asset_symbol_type( sell_symbol ), asset_symbol_type( buy_symbol ) );
   auto max_buy = price::max( asset_symbol_type( buy_symbol ), asset_symbol_type( sell_symbol ) );
   
   auto margin_sell_itr = margin_price_idx.lower_bound( max_sell );
   auto margin_buy_itr = margin_price_idx.lower_bound( max_buy );
   auto margin_end = margin_price_idx.end();

   while( margin_sell_itr != margin_end &&
      margin_sell_itr->sell_price.base.symbol == asset_symbol_type( sell_symbol ) &&
      results.margin_bids.size() < limit )
   {
      results.margin_bids.push_back( margin_order_api_obj( *margin_sell_itr ) );
      ++margin_sell_itr;
   }
   while( margin_buy_itr != margin_end && 
      margin_buy_itr->sell_price.base.symbol == asset_symbol_type( buy_symbol ) &&
      results.margin_asks.size() < limit )
   {
      results.margin_asks.push_back( margin_order_api_obj( *margin_buy_itr ) );
      ++margin_buy_itr;  
   }
   return results;
}

market_call_orders database_api::get_call_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_call_orders( buy_symbol, sell_symbol, limit );
   });
}

market_call_orders database_api_impl::get_call_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );
   limit = std::min( limit, uint32_t( 1000 ) );

   market_call_orders results;

   const asset_object& buy_asset = _db.get_asset( buy_symbol );
   const asset_object& sell_asset = _db.get_asset( sell_symbol );

   asset_symbol_type bitasset_symbol;

   if( buy_asset.asset_type == BITASSET_ASSET )
   {
      bitasset_symbol = buy_asset.symbol;
   }
   else if( sell_asset.asset_type == BITASSET_ASSET )
   {
      bitasset_symbol = sell_asset.symbol;
   }
   else
   {
      return results;
   }

   const asset_bitasset_data_object& bit_obj = _db.get_bitasset_data( bitasset_symbol );
   results.settlement_price = bit_obj.current_feed.settlement_price;

   const auto& call_idx = _db.get_index< call_order_index >().indices().get< by_debt >();
   
   auto call_itr = call_idx.lower_bound( bitasset_symbol );
   auto call_end = call_idx.end();

   while( call_itr != call_end &&
      call_itr->debt_type() == bitasset_symbol && 
      results.calls.size() < limit )
   {
      results.calls.push_back( call_order_api_obj( *call_itr ) );
      ++call_itr;
   }
   
   return results;
}

market_credit_loans database_api::get_credit_loans( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_credit_loans( buy_symbol, sell_symbol, limit );
   });
}

market_credit_loans database_api_impl::get_credit_loans( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );
   limit = std::min( limit, uint32_t( 1000 ) );
   
   market_credit_loans results;

   const auto& loan_idx = _db.get_index<credit_loan_index>().indices().get< by_liquidation_spread >();
   
   auto loan_buy_itr = loan_idx.lower_bound( boost::make_tuple( asset_symbol_type( buy_symbol ), asset_symbol_type( sell_symbol ) ) );
   auto loan_sell_itr = loan_idx.lower_bound( boost::make_tuple( asset_symbol_type( sell_symbol ), asset_symbol_type( buy_symbol ) ) );
   auto loan_end = loan_idx.end();

   while( loan_sell_itr != loan_end &&
      loan_sell_itr->debt_asset() == asset_symbol_type( sell_symbol ) &&
      results.loan_bids.size() < limit )
   {
      results.loan_bids.push_back( credit_loan_api_obj( *loan_sell_itr ) );
      ++loan_sell_itr;
   }
   while( loan_buy_itr != loan_end &&
      loan_buy_itr->debt_asset() == asset_symbol_type( buy_symbol ) &&
      results.loan_asks.size() < limit )
   {
      results.loan_asks.push_back( credit_loan_api_obj( *loan_buy_itr ) );
      ++loan_buy_itr;
   }
   return results;
}

vector< credit_pool_api_obj> database_api::get_credit_pools( vector< string > assets )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_credit_pools( assets );
   });
}

vector< credit_pool_api_obj> database_api_impl::get_credit_pools( vector< string > assets )const
{
   vector< credit_pool_api_obj> results;

   const auto& pool_idx = _db.get_index< asset_credit_pool_index >().indices().get< by_base_symbol >();

   for( auto symbol : assets )
   {
      auto pool_itr = pool_idx.find( symbol );
      if( pool_itr != pool_idx.end() )
      {
         results.push_back( credit_pool_api_obj( *pool_itr ) );
      }
   }

   return results; 
}

vector< liquidity_pool_api_obj > database_api::get_liquidity_pools( string buy_symbol, string sell_symbol )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_liquidity_pools( buy_symbol, sell_symbol );
   });
}

vector< liquidity_pool_api_obj > database_api_impl::get_liquidity_pools( string buy_symbol, string sell_symbol )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );

   vector< liquidity_pool_api_obj > results;

   const asset_object& buy_asset = _db.get_asset( asset_symbol_type( buy_symbol ) );
   const asset_object& sell_asset = _db.get_asset( asset_symbol_type( sell_symbol ) );
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( buy_asset.id < sell_asset.id )
   {
      symbol_a = buy_asset.symbol;
      symbol_b = sell_asset.symbol;
   }
   else
   {
      symbol_b = buy_asset.symbol;
      symbol_a = sell_asset.symbol;
   }

   const auto& pool_idx = _db.get_index< asset_liquidity_pool_index >().indices().get< by_asset_pair >();

   liquidity_pool_api_obj buy_core_pool;
   liquidity_pool_api_obj sell_core_pool;
   liquidity_pool_api_obj buy_usd_pool;
   liquidity_pool_api_obj sell_usd_pool;
   liquidity_pool_api_obj direct_pool;

   auto pool_itr = pool_idx.find( boost::make_tuple( symbol_a, symbol_b ) );
   if( pool_itr != pool_idx.end() )
   {
      direct_pool = liquidity_pool_api_obj( *pool_itr );
      results.push_back( direct_pool );
   }

   if( asset_symbol_type( buy_symbol ) != SYMBOL_COIN )
   {
      auto pool_itr = pool_idx.find( boost::make_tuple( SYMBOL_COIN, asset_symbol_type( buy_symbol ) ) );
      if( pool_itr != pool_idx.end() )
      {
         buy_core_pool = liquidity_pool_api_obj( *pool_itr );
         results.push_back( buy_core_pool );
      }
   }

   if( asset_symbol_type( sell_symbol ) != SYMBOL_COIN )
   {
      auto pool_itr = pool_idx.find( boost::make_tuple( SYMBOL_COIN, asset_symbol_type( sell_symbol ) ) );
      if( pool_itr != pool_idx.end() )
      {
         sell_core_pool = liquidity_pool_api_obj( *pool_itr );
         results.push_back( sell_core_pool );
      }
   }

   if( asset_symbol_type( buy_symbol ) != SYMBOL_USD )
   {
      auto pool_itr = pool_idx.find( boost::make_tuple( SYMBOL_USD, asset_symbol_type( buy_symbol ) ) );
      if( pool_itr != pool_idx.end() )
      {
         buy_usd_pool = liquidity_pool_api_obj( *pool_itr );
         results.push_back( buy_usd_pool );
      }
   }

   if( asset_symbol_type( sell_symbol ) != SYMBOL_USD )
   {
      auto pool_itr = pool_idx.find( boost::make_tuple( SYMBOL_USD, asset_symbol_type( sell_symbol ) ) );
      if( pool_itr != pool_idx.end() )
      {
         sell_usd_pool = liquidity_pool_api_obj( *pool_itr );
         results.push_back( sell_usd_pool );
      }
   }
   
   return results; 
}

market_state database_api::get_market_state( string buy_symbol, string sell_symbol )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_market_state( buy_symbol, sell_symbol );
   });
}

market_state database_api_impl::get_market_state( string buy_symbol, string sell_symbol )const
{
   market_state results;

   results.limit_orders = get_limit_orders( buy_symbol, sell_symbol );
   results.margin_orders = get_margin_orders( buy_symbol, sell_symbol );

   const asset_object& buy_asset = _db.get_asset( buy_symbol );
   const asset_object& sell_asset = _db.get_asset( sell_symbol );
   if( buy_asset.is_market_issued )
   {
      const asset_bitasset_data_object& buy_bitasset = _db.get_bitasset_data( buy_symbol );
      if( buy_bitasset.backing_asset == sell_symbol )
      {
         results.call_orders = get_call_orders( buy_symbol, sell_symbol );
      }
   }
   if( sell_asset.is_market_issued )
   {
      const asset_bitasset_data_object& sell_bitasset = _db.get_bitasset_data( sell_symbol );
      if( sell_bitasset.backing_asset == buy_symbol )
      {
         results.call_orders = get_call_orders( buy_symbol, sell_symbol );
      }
   }

   results.liquidity_pools = get_liquidity_pools( buy_symbol, sell_symbol );
   vector< string > assets;
   assets.push_back( buy_symbol );
   assets.push_back( sell_symbol );
   results.credit_pools = get_credit_pools( assets );
   results.credit_loans = get_credit_loans( buy_symbol, sell_symbol );

   return results; 
}


   //=============//
   // === Ads === //
   //=============//


vector< account_ad_state > database_api::get_account_ads( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_ads( names );
   });
}

vector< account_ad_state > database_api_impl::get_account_ads( vector< string > names )const
{
   vector< account_ad_state > results;
   results.reserve( names.size() );

   const auto& creative_idx   = _db.get_index< ad_creative_index >().indices().get< by_latest >();
   const auto& campaign_idx   = _db.get_index< ad_campaign_index >().indices().get< by_latest >();
   const auto& audience_idx   = _db.get_index< ad_audience_index >().indices().get< by_latest >();
   const auto& inventory_idx  = _db.get_index< ad_inventory_index >().indices().get< by_latest >();
   const auto& bidder_idx     = _db.get_index< ad_bid_index >().indices().get< by_bidder_updated >();
   const auto& account_idx    = _db.get_index< ad_bid_index >().indices().get< by_account_updated >();
   const auto& author_idx     = _db.get_index< ad_bid_index >().indices().get< by_author_updated >();
   const auto& provider_idx   = _db.get_index< ad_bid_index >().indices().get< by_provider_metric_price >();

   for( auto name : names )
   {
      account_ad_state astate;

      auto creative_itr = creative_idx.lower_bound( name );
      while( creative_itr != creative_idx.end() && creative_itr->author == name )
      {
         astate.creatives.push_back( ad_creative_api_obj( *creative_itr ) );
         ++creative_itr;
      }

      auto campaign_itr = campaign_idx.lower_bound( name );
      while( campaign_itr != campaign_idx.end() && campaign_itr->account == name )
      {
         astate.campaigns.push_back( ad_campaign_api_obj( *campaign_itr ) );
         ++campaign_itr;
      }

      auto audience_itr = audience_idx.lower_bound( name );
      while( audience_itr != audience_idx.end() && audience_itr->account == name )
      {
         astate.audiences.push_back( ad_audience_api_obj( *audience_itr ) );
         ++audience_itr;
      }

      auto inventory_itr = inventory_idx.lower_bound( name );
      while( inventory_itr != inventory_idx.end() && inventory_itr->provider == name )
      {
         astate.inventories.push_back( ad_inventory_api_obj( *inventory_itr ) );
         ++inventory_itr;
      }

      auto bidder_itr = bidder_idx.lower_bound( name );
      while( bidder_itr != bidder_idx.end() && bidder_itr->bidder == name )
      {
         astate.created_bids.push_back( ad_bid_api_obj( *bidder_itr ) );
         ++bidder_itr;
      }

      auto account_itr = account_idx.lower_bound( name );
      while( account_itr != account_idx.end() && account_itr->account == name )
      {
         astate.account_bids.push_back( ad_bid_api_obj( *account_itr ) );
         ++account_itr;
      }

      auto author_itr = author_idx.lower_bound( name );
      while( author_itr != author_idx.end() && author_itr->author == name )
      {
         astate.creative_bids.push_back( ad_bid_api_obj( *author_itr ) );
         ++author_itr;
      }

      auto provider_itr = provider_idx.lower_bound( name );
      while( provider_itr != provider_idx.end() && provider_itr->provider == name )
      {
         astate.incoming_bids.push_back( ad_bid_state( *provider_itr ) );

         auto cr_itr = creative_idx.find( boost::make_tuple( provider_itr->author, provider_itr->creative_id ) );
         if( cr_itr != creative_idx.end() )
         {
            astate.incoming_bids.back().creative = ad_creative_api_obj( *cr_itr );
         }

         auto c_itr = campaign_idx.find( boost::make_tuple( provider_itr->account, provider_itr->campaign_id ) );
         if( c_itr != campaign_idx.end() )
         {
            astate.incoming_bids.back().campaign = ad_campaign_api_obj( *c_itr );
         }

         auto i_itr = inventory_idx.find( boost::make_tuple( provider_itr->provider, provider_itr->inventory_id ) );
         if( i_itr != inventory_idx.end() )
         {
            astate.incoming_bids.back().inventory = ad_inventory_api_obj( *i_itr );
         }

         auto a_itr = audience_idx.find( boost::make_tuple( provider_itr->bidder, provider_itr->audience_id ) );
         if( a_itr != audience_idx.end() )
         {
            astate.incoming_bids.back().audience = ad_audience_api_obj( *a_itr );
         }

         ++provider_itr;
      }

      results.push_back( astate );
   }

   return results;
}


vector< ad_bid_state > database_api::get_interface_audience_bids( const ad_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_interface_audience_bids( query );
   });
}

/**
 * Retrieves all bids for an interface that includes a specified
 * account in its audience set. 
 */
vector< ad_bid_state > database_api_impl::get_interface_audience_bids( const ad_query& query )const
{
   vector< ad_bid_state > results;
   account_name_type interface = query.interface;
   account_name_type viewer = query.viewer;

   ad_format_type format = STANDARD_FORMAT;
   for( auto i = 0; i < ad_format_values.size(); i++ )
   {
      if( query.format_type == ad_format_values[ i ] )
      {
         format = ad_format_type( i );
         break;
      }
   }
   
   const auto& creative_idx = _db.get_index< ad_creative_index >().indices().get< by_latest >();
   const auto& campaign_idx = _db.get_index< ad_campaign_index >().indices().get< by_latest >();
   const auto& audience_idx = _db.get_index< ad_audience_index >().indices().get< by_latest >();
   const auto& inventory_idx = _db.get_index< ad_inventory_index >().indices().get< by_latest >();
   const auto& provider_idx = _db.get_index< ad_bid_index >().indices().get< by_provider_metric_price >();

   auto provider_itr = provider_idx.lower_bound( boost::make_tuple( interface, format ) );
   while( provider_itr != provider_idx.end() && 
      provider_itr->provider == interface &&
      results.size() < query.limit )
   {
      auto a_itr = audience_idx.find( boost::make_tuple( provider_itr->bidder, provider_itr->audience_id ) );
      if( a_itr != audience_idx.end() )
      {
         const ad_audience_object& aud = *a_itr;
         if( aud.is_audience( viewer ) )
         {
            results.push_back( ad_bid_state( *provider_itr ) );
            results.back().audience = ad_audience_api_obj( *a_itr );
            auto cr_itr = creative_idx.find( boost::make_tuple( provider_itr->author, provider_itr->creative_id ) );
            if( cr_itr != creative_idx.end() )
            {
               results.back().creative = ad_creative_api_obj( *cr_itr );
            }
            auto c_itr = campaign_idx.find( boost::make_tuple( provider_itr->account, provider_itr->campaign_id ) );
            if( c_itr != campaign_idx.end() )
            {
               results.back().campaign = ad_campaign_api_obj( *c_itr );
            }
            auto i_itr = inventory_idx.find( boost::make_tuple( provider_itr->provider, provider_itr->inventory_id ) );
            if( i_itr != inventory_idx.end() )
            {
               results.back().inventory = ad_inventory_api_obj( *i_itr );
            }
         }
      }
      ++provider_itr;
   }
   return results;
}


   //================//
   // === Search === //
   //================//


search_result_state database_api::get_search_query( const search_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_search_query( query );
   });
}

/**
 * Retreives a series of accounts, boards, tags, assets and posts according to the 
 * lowest edit distance between the search query and the names of the objects.
 */
search_result_state database_api_impl::get_search_query( const search_query& query )const
{
   search_result_state results;
   account_name_type account = query.account;
   string q = query.query;
   size_t margin = ( ( q.size() * query.margin_percent ) / PERCENT_100 ) + 1;

   const auto& account_idx = _db.get_index< account_index >().indices().get< by_name >();
   const auto& board_idx = _db.get_index< board_index >().indices().get< by_name >();
   const auto& tag_idx = _db.get_index< tag_following_index >().indices().get< by_tag >();
   const auto& asset_idx = _db.get_index< asset_index >().indices().get< by_symbol >();
   const auto& post_idx = _db.get_index< comment_index >().indices().get< by_title >();

   auto account_itr = account_idx.begin();
   auto board_itr = board_idx.begin();
   auto tag_itr = tag_idx.begin();
   auto asset_itr = asset_idx.begin();
   auto post_itr = post_idx.upper_bound( shared_string() );    // Skip index posts with no title

   vector< pair< account_name_type, size_t > > accounts;
   vector< pair< board_name_type, size_t > > boards;
   vector< pair< tag_name_type, size_t > > tags;
   vector< pair< asset_symbol_type, size_t > > assets;
   vector< pair< shared_string, size_t > > posts;

   accounts.reserve( account_idx.size() );
   boards.reserve( board_idx.size() );
   tags.reserve( tag_idx.size() );
   assets.reserve( asset_idx.size() );
   posts.reserve( post_idx.size() );

   results.accounts.reserve( query.limit );
   results.boards.reserve( query.limit );
   results.tags.reserve( query.limit );
   results.assets.reserve( query.limit );
   results.posts.reserve( query.limit );

   // Finds items that are within the specified margin of error by edit distance from the search term.
   // Sort items in Ascending order, lowest edit distance first.

   if( query.include_accounts )
   {
      while( account_itr != account_idx.end() )
      {
         size_t edit_dist = edit_distance( string( account_itr->name ), q );
         if( edit_dist <= margin )
         {
            accounts.push_back( std::make_pair( account_itr->name, edit_dist  ) );
         }
         ++account_itr;
      }

      std::sort( accounts.begin(), accounts.end(), [&]( auto a, auto b )
      {
         return a.second > b.second;
      });

      for( auto item : accounts )
      {
         if( results.accounts.size() < query.limit )
         {
            account_itr = account_idx.find( item.first );
            results.accounts.push_back( account_api_obj( *account_itr, _db ) );
         }
         else
         {
            break;
         }
      }
   }

   if( query.include_boards )
   {
      while( board_itr != board_idx.end() )
      {
         size_t edit_dist = edit_distance( string( board_itr->name ), q );
         if( edit_dist <= margin )
         {
            boards.push_back( std::make_pair( board_itr->name, edit_dist ) );
         }
         ++board_itr;
      }

      std::sort( boards.begin(), boards.end(), [&]( auto a, auto b )
      {
         return a.second > b.second;
      });

      for( auto item : boards )
      {
         if( results.boards.size() < query.limit )
         {
            board_itr = board_idx.find( item.first );
            results.boards.push_back( board_api_obj( *board_itr ) );
         }
         else
         {
            break;
         }
      }
   }

   if( query.include_tags )
   {
      while( tag_itr != tag_idx.end() )
      {
         size_t edit_dist = edit_distance( string( tag_itr->tag ), q );
         if( edit_dist <= margin )
         {
            tags.push_back( std::make_pair( tag_itr->tag, edit_dist ) );
         }
         ++tag_itr;
      }

      std::sort( tags.begin(), tags.end(), [&]( auto a, auto b )
      {
         return a.second > b.second;
      });

      for( auto item : tags )
      {
         if( results.tags.size() < query.limit )
         {
            tag_itr = tag_idx.find( item.first );
            results.tags.push_back( tag_following_api_obj( *tag_itr ) );
         }
         else
         {
            break;
         }
      }
   }

   if( query.include_assets )
   {
      while( asset_itr != asset_idx.end() )
      {
         size_t edit_dist = edit_distance( string( asset_itr->symbol ), q );
         if( edit_dist <= margin )
         {
            assets.push_back( std::make_pair( asset_itr->symbol, edit_dist ) );
         }
         ++asset_itr;
      }

      std::sort( assets.begin(), assets.end(), [&]( auto a, auto b )
      {
         return a.second > b.second;
      });

      for( auto item : assets )
      {
         if( results.assets.size() < query.limit )
         {
            asset_itr = asset_idx.find( item.first );
            results.assets.push_back( asset_api_obj( *asset_itr ) );
         }
         else
         {
            break;
         }
      }
   }

   if( query.include_posts )
   {
      while( post_itr != post_idx.end() )
      {
         size_t edit_dist = edit_distance( to_string( post_itr->title ), q );
         if( edit_dist <= margin )
         {
            posts.push_back( std::make_pair( post_itr->title, edit_dist ) );
         }
         ++post_itr;
      }

      std::sort( posts.begin(), posts.end(), [&]( auto a, auto b )
      {
         return a.second > b.second;
      });

      for( auto item : posts )
      {
         if( results.posts.size() < query.limit )
         {
            post_itr = post_idx.find( item.first );
            results.posts.push_back( discussion( *post_itr ) );
         }
         else
         {
            break;
         }
      }
   }

   return results;
}


   //=================================//
   // === Blocks and Transactions === //
   //=================================//


optional< block_header > database_api::get_block_header( uint64_t block_num )const
{
   FC_ASSERT( !my->_disable_get_block,
      "get_block_header is disabled on this node." );

   return my->_db.with_read_lock( [&]()
   {
      return my->get_block_header( block_num );
   });
}


optional< block_header > database_api_impl::get_block_header( uint64_t block_num ) const
{
   auto results = _db.fetch_block_by_number( block_num );
   if( results )
   {
      return *results;
   }
      
   return {};
}


optional< signed_block_api_obj > database_api::get_block( uint64_t block_num )const
{
   FC_ASSERT( !my->_disable_get_block,
      "get_block is disabled on this node." );

   return my->_db.with_read_lock( [&]()
   {
      return my->get_block( block_num );
   });
}


optional< signed_block_api_obj > database_api_impl::get_block( uint64_t block_num )const
{
   return _db.fetch_block_by_number( block_num );
}


vector< applied_operation > database_api::get_ops_in_block( uint64_t block_num, bool only_virtual )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_ops_in_block( block_num, only_virtual );
   });
}


vector< applied_operation > database_api_impl::get_ops_in_block(uint64_t block_num, bool only_virtual)const
{
   const auto& idx = _db.get_index< operation_index >().indices().get< by_location >();
   auto itr = idx.lower_bound( block_num );
   vector< applied_operation> results;
   applied_operation temp;
   while( itr != idx.end() && itr->block == block_num )
   {
      temp = *itr;
      if( !only_virtual || is_virtual_operation(temp.op) )
         results.push_back(temp);
      ++itr;
   }
   return results;
}


std::string database_api::get_transaction_hex( const signed_transaction& trx )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_transaction_hex( trx );
   });
}

std::string database_api_impl::get_transaction_hex( const signed_transaction& trx )const
{
   return fc::to_hex( fc::raw::pack( trx ) );
}

annotated_signed_transaction database_api::get_transaction( transaction_id_type id )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_transaction( id );
   });
}

annotated_signed_transaction database_api_impl::get_transaction( transaction_id_type id )const
{
   const auto& idx = _db.get_index< operation_index >().indices().get< by_transaction_id >();
   auto itr = idx.lower_bound( id );

   annotated_signed_transaction results;

   if( itr != idx.end() && itr->trx_id == id )
   {
      auto blk = _db.fetch_block_by_number( itr->block );
      FC_ASSERT( blk.valid() );
      FC_ASSERT( blk->transactions.size() > itr->trx_in_block );
      results = blk->transactions[ itr->trx_in_block ];
      results.block_num = itr->block;
      results.transaction_num = itr->trx_in_block;
      return results;
   }
   else
   {
      FC_ASSERT( false, "Unknown Transaction ${t}", ("t",id));
   }
}

set< public_key_type > database_api::get_required_signatures( const signed_transaction& trx, const flat_set< public_key_type >& available_keys )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_required_signatures( trx, available_keys );
   });
}

set< public_key_type > database_api_impl::get_required_signatures( const signed_transaction& trx, const flat_set < public_key_type >& available_keys )const
{
   set< public_key_type > results = trx.get_required_signatures(
      CHAIN_ID,
      available_keys,
      [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).active  ); },
      [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).owner   ); },
      [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).posting ); },
      MAX_SIG_CHECK_DEPTH );
   return results;
}

set< public_key_type > database_api::get_potential_signatures( const signed_transaction& trx )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_potential_signatures( trx );
   });
}

set< public_key_type > database_api_impl::get_potential_signatures( const signed_transaction& trx )const
{
   set< public_key_type > results;
   trx.get_required_signatures(
      CHAIN_ID,
      flat_set< public_key_type >(),
      [&]( account_name_type account_name )
      {
         const auto& auth = _db.get< account_authority_object, by_account >(account_name).active;
         for( const auto& k : auth.get_keys() )
            results.insert(k);
         return authority( auth );
      },
      [&]( account_name_type account_name )
      {
         const auto& auth = _db.get< account_authority_object, by_account >(account_name).owner;
         for( const auto& k : auth.get_keys() )
            results.insert(k);
         return authority( auth );
      },
      [&]( account_name_type account_name )
      {
         const auto& auth = _db.get< account_authority_object, by_account >(account_name).posting;
         for( const auto& k : auth.get_keys() )
            results.insert(k);
         return authority( auth );
      },
      MAX_SIG_CHECK_DEPTH
   );

   return results;
}

bool database_api::verify_authority( const signed_transaction& trx ) const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->verify_authority( trx );
   });
}

bool database_api_impl::verify_authority( const signed_transaction& trx )const
{
   trx.verify_authority( 
      CHAIN_ID,
      [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).active  ); },
      [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).owner   ); },
      [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).posting ); },
      MAX_SIG_CHECK_DEPTH );
   return true;
}

bool database_api::verify_account_authority( const string& name_or_id, const flat_set< public_key_type >& signers )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->verify_account_authority( name_or_id, signers );
   });
}

bool database_api_impl::verify_account_authority( const string& name, const flat_set< public_key_type >& keys )const
{
   FC_ASSERT( name.size() > 0,
      "Verify requets must include account name.");
   const account_object* account = _db.find< account_object, by_name >( name );
   FC_ASSERT( account != nullptr, 
      "No such account" );
   signed_transaction trx;
   transfer_operation op;
   op.from = account->name;
   op.signatory = account->name;
   trx.operations.emplace_back( op );

   return verify_authority( trx );    // Confirm authority is able to sign a transfer operation
}


   //======================//
   // === Posts + Tags === //
   //======================//


vector< vote_state > database_api::get_active_votes( string author, string permlink )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_active_votes( author, permlink );
   });
}

vector< vote_state > database_api_impl::get_active_votes( string author, string permlink )const
{
   vector< vote_state > results;
   const comment_object& comment = _db.get_comment( author, permlink );
   const auto& idx = _db.get_index< comment_vote_index >().indices().get< by_comment_voter >();
   comment_id_type cid(comment.id);
   auto itr = idx.lower_bound( cid );
   while( itr != idx.end() && itr->comment == cid )
   {
      vote_state vstate;
      vstate.voter = itr->voter;
      vstate.weight = itr->weight;
      vstate.reward = itr->reward;
      vstate.percent = itr->vote_percent;
      vstate.time = itr->last_update;
      results.push_back( vstate );
      ++itr;
   }
   return results;
}

vector< view_state > database_api::get_active_views( string author, string permlink )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_active_views( author, permlink );
   });
}

vector< view_state > database_api_impl::get_active_views( string author, string permlink )const
{
   vector< view_state > results;
   const comment_object& comment = _db.get_comment( author, permlink );
   const auto& idx = _db.get_index< comment_view_index >().indices().get< by_comment_viewer >();
   comment_id_type cid(comment.id);
   auto itr = idx.lower_bound( cid );
   while( itr != idx.end() && itr->comment == cid )
   {
      view_state vstate;
      vstate.viewer = itr->viewer;
      vstate.weight = itr->weight;
      vstate.reward = itr->reward;
      vstate.time = itr->created;
      results.push_back( vstate );
      ++itr;
   }
   return results;
}

vector< share_state > database_api::get_active_shares( string author, string permlink )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_active_shares( author, permlink );
   });
}

vector< share_state > database_api_impl::get_active_shares( string author, string permlink )const
{
   vector< share_state > results;
   const comment_object& comment = _db.get_comment( author, permlink );
   const auto& idx = _db.get_index< comment_share_index >().indices().get< by_comment_sharer >();
   comment_id_type cid(comment.id);
   auto itr = idx.lower_bound( cid );
   while( itr != idx.end() && itr->comment == cid )
   {
      share_state sstate;
      sstate.sharer = itr->sharer;
      sstate.weight = itr->weight;
      sstate.reward = itr->reward;
      sstate.time = itr->created;

      results.push_back( sstate );
      ++itr;
   }
   return results;
}

vector< moderation_state > database_api::get_active_mod_tags( string author, string permlink )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_active_mod_tags( author, permlink );
   });
}

vector< moderation_state > database_api_impl::get_active_mod_tags( string author, string permlink )const
{
   vector< moderation_state > results;
   const comment_object& comment = _db.get_comment( author, permlink );
   const auto& idx = _db.get_index< moderation_tag_index >().indices().get< by_comment_moderator >();
   comment_id_type cid(comment.id);
   auto itr = idx.lower_bound( cid );

   while( itr != idx.end() && itr->comment == cid )
   {
      moderation_state mstate;
      mstate.moderator = itr->moderator;
      for( auto tag : itr->tags )
      {
         mstate.tags.push_back( tag );
      }
      mstate.rating = itr->rating;
      mstate.details = to_string( itr->details );
      mstate.filter = itr->filter;
      mstate.time = itr->last_update;

      results.push_back( mstate );
      ++itr;
   }
   return results;
}

vector< account_vote > database_api::get_account_votes( string account, string from_author, string from_permlink, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_votes( account, from_author, from_permlink, limit );
   });
}

vector< account_vote > database_api_impl::get_account_votes( string account, string from_author, string from_permlink, uint32_t limit )const
{
   vector< account_vote > results;
   limit = std::min( limit, uint32_t( 1000 ) );
   results.reserve( limit );

   const account_object& account = _db.get_account( account );
   const auto& idx = _db.get_index< comment_vote_index >().indices().get< by_voter_comment >();

   comment_id_type start;

   auto itr = idx.lower_bound( account );
   auto end = idx.upper_bound( account );

   if( from_author.length() && from_permlink.length() )
   {
      start = _db.get_comment( from_author, from_permlink ).id;
      auto from_itr = idx.find( account, start );
      if( from_itr != idx.end() )
      {
         itr = idx.iterator_to( *from_itr );
      }
   }

   while( itr != end && results.size() < limit )
   {
      const comment_object& comment = _db.get( itr->comment );
      account_vote avote;
      avote.author = comment.author;
      avote.permlink = to_string( comment.permlink );
      avote.weight = itr->weight;
      avote.reward = itr->reward;
      avote.percent = itr->vote_percent;
      avote.time = itr->last_update;
      results.push_back( avote );
      ++itr;
   }
   return results;
}

vector< account_view > database_api::get_account_views( string account, string from_author, string from_permlink, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_views( account, from_author, from_permlink, limit );
   });
}

vector< account_view > database_api_impl::get_account_views( string account, string from_author, string from_permlink, uint32_t limit )const
{
   vector< account_view > results;
   limit = std::min( limit, uint32_t( 1000 ) );
   results.reserve( limit );

   const account_object& account = _db.get_account( account );
   const auto& idx = _db.get_index< comment_view_index >().indices().get< by_viewer_comment >();

   comment_id_type start;

   auto itr = idx.lower_bound( account );
   auto end = idx.upper_bound( account );

   if( from_author.length() && from_permlink.length() )
   {
      start = _db.get_comment( from_author, from_permlink ).id;
      auto from_itr = idx.find( account, start );
      if( from_itr != idx.end() )
      {
         itr = idx.iterator_to( *from_itr );
      }
   }

   while( itr != end && results.size() < limit )
   {
      const comment_object& comment = _db.get( itr->comment );
      account_view aview;
      aview.author = comment.author;
      aview.permlink = to_string( comment.permlink );
      aview.weight = itr->weight;
      aview.reward = itr->reward;
      aview.time = itr->created;
      results.push_back( aview );
      ++itr;
   }
   return results;
}

vector< account_share > database_api::get_account_shares( string account, string from_author, string from_permlink, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_shares( account, from_author, from_permlink, limit );
   });
}

vector< account_share > database_api_impl::get_account_shares( string account, string from_author, string from_permlink, uint32_t limit )const
{
   vector< account_share > results;
   limit = std::min( limit, uint32_t( 1000 ) );
   results.reserve( limit );

   const account_object& account = _db.get_account( account );
   const auto& idx = _db.get_index< comment_share_index >().indices().get< by_sharer_comment >();

   comment_id_type start;

   auto itr = idx.lower_bound( account );
   auto end = idx.upper_bound( account );

   if( from_author.length() && from_permlink.length() )
   {
      start = _db.get_comment( from_author, from_permlink ).id;
      auto from_itr = idx.find( account, start );
      if( from_itr != idx.end() )
      {
         itr = idx.iterator_to( *from_itr );
      }
   }

   while( itr != end && results.size() < limit )
   {
      const comment_object& comment = _db.get( itr->comment );
      account_share ashare;
      ashare.author = comment.author;
      ashare.permlink = to_string( comment.permlink );
      ashare.weight = itr->weight;
      ashare.reward = itr->reward;
      ashare.time = itr->created;
      results.push_back( ashare );
      ++itr;
   }
   return results;
}

vector< account_moderation > database_api::get_account_moderation( string account, string from_author, string from_permlink, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_moderation( account, from_author, from_permlink, limit );
   });
}

vector< account_moderation > database_api_impl::get_account_moderation( string account, string from_author, string from_permlink, uint32_t limit )const
{
   vector< account_moderation > results;
   limit = std::min( limit, uint32_t( 1000 ) );
   results.reserve( limit );

   const account_object& account = _db.get_account( account );
   const auto& idx = _db.get_index< moderation_tag_index >().indices().get< by_moderator_comment >();

   comment_id_type start;

   auto itr = idx.lower_bound( account );
   auto end = idx.upper_bound( account );

   if( from_author.length() && from_permlink.length() )
   {
      start = _db.get_comment( from_author, from_permlink ).id;
      auto from_itr = idx.find( account, start );
      if( from_itr != idx.end() )
      {
         itr = idx.iterator_to( *from_itr );
      }
   }

   while( itr != end && results.size() < limit )
   {
      const comment_object& comment = _db.get( itr->comment );
      account_moderation amod;
      amod.author = comment.author;
      amod.permlink = to_string( comment.permlink );
      amod.tags = itr->tags;
      amod.rating = itr->rating;
      amod.details = to_string( itr->details );
      amod.filter = itr->filter;
      amod.time = itr->last_update;
      results.push_back( amod );
      ++itr;
   }
   return results;
}

vector< tag_following_api_obj > database_api::get_tag_followings( vector< string > tags )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_tag_followings( tags );
   });
}

vector< tag_following_api_obj > database_api_impl::get_tag_followings( vector< string > tags )const
{
   vector< tag_following_api_obj > results;
   const auto& tag_idx = _db.get_index< tag_following_index >().indices().get< by_tag >();
   
   for( auto tag : tags )
   {
      auto tag_itr = tag_idx.find( tag );

      if( tag_itr != tag_idx.end() )
      {
         results.push_back( tag_following_api_obj( *tag_itr ) );
      }
   }
   return results;
}

vector< pair< tag_name_type, uint32_t > >database_api::get_tags_used_by_author( string author )const 
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_tags_used_by_author( author );
   });
}

vector< pair< tag_name_type, uint32_t > > database_api_impl::get_tags_used_by_author( string author )const 
{
   if( !_db.has_index<tags::author_tag_stats_index>() )
   {
      return vector< pair< tag_name_type, uint32_t > >();
   }

   const account_object* acnt = _db.find_account( author );
   FC_ASSERT( acnt != nullptr,
      "Account not found." );

   const auto& tidx = _db.get_index< tags::author_tag_stats_index >().indices().get< tags::by_author_posts_tag >();
   auto itr = tidx.lower_bound( boost::make_tuple( acnt->id, 0 ) );
   vector< pair< tag_name_type, uint32_t > > results;

   while( itr != tidx.end() && itr->author == acnt->id && results.size() < 1000 )
   {
      results.push_back( std::make_pair( itr->tag, itr->total_posts ) );
      ++itr;
   }
   return results;
}

vector< tag_api_obj > database_api::get_top_tags( string after, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_top_tags( after, limit );
   });
}

vector< tag_api_obj > database_api_impl::get_top_tags( string after, uint32_t limit )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< tag_api_obj >();
   }
   
   vector< tag_api_obj > results;
   limit = std::min( limit, uint32_t( 1000 ) );
   results.reserve( limit );

   const auto& nidx = _db.get_index< tags::tag_stats_index >().indices().get< tags::by_tag >();
   const auto& ridx = _db.get_index< tags::tag_stats_index >().indices().get< tags::by_vote_power >();

   auto itr = ridx.begin();
   if( after != "" && nidx.size() )
   {
      auto nitr = nidx.lower_bound( after );
      if( nitr == nidx.end() )
      {
         itr = ridx.end();
      } 
      else
      {
         itr = ridx.iterator_to( *nitr );
      } 
   }

   while( itr != ridx.end() && results.size() < limit )
   {
      results.push_back( tag_api_obj( *itr ) );
      ++itr;
   }
   return results;
}


   //=====================//
   // === Discussions === //
   //=====================//


discussion database_api::get_content( string author, string permlink )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_content( author, permlink );
   });
}


discussion database_api_impl::get_content( string author, string permlink )const
{
   const auto& by_permlink_idx = _db.get_index< comment_index >().indices().get< by_permlink >();
   auto itr = by_permlink_idx.find( boost::make_tuple( author, permlink ) );
   if( itr != by_permlink_idx.end() )
   {
      discussion results(*itr);
      results.active_votes = get_active_votes( author, permlink );
      results.active_views = get_active_views( author, permlink );
      results.active_shares = get_active_shares( author, permlink );
      results.active_mod_tags = get_active_mod_tags( author, permlink );
      return results;
   }
   else
   {
      return discussion();
   }
}

vector< discussion > database_api::get_content_replies( string author, string permlink )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_content_replies( author, permlink );
   });
}

vector< discussion > database_api_impl::get_content_replies( string author, string permlink )const
{
   account_name_type acc_name = account_name_type( author );
   const auto& by_permlink_idx = _db.get_index< comment_index >().indices().get< by_parent >();
   auto itr = by_permlink_idx.find( boost::make_tuple( acc_name, permlink ) );
   vector< discussion > results;
   while( itr != by_permlink_idx.end() && 
      itr->parent_author == author && 
      to_string( itr->parent_permlink ) == permlink )
   {
      results.push_back( discussion( *itr ) );
      ++itr;
   }
   return results;
}

vector< discussion > database_api::get_replies_by_last_update( account_name_type start_parent_author, string start_permlink, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_replies_by_last_update( start_parent_author, start_permlink, limit);
   });
}


vector< discussion > database_api_impl::get_replies_by_last_update( account_name_type start_parent_author, string start_permlink, uint32_t limit )const
{
   vector< discussion > results;

   limit = std::min( limit, uint32_t( 1000 ) );
   const auto& last_update_idx = _db.get_index< comment_index >().indices().get< by_last_update >();
   auto itr = last_update_idx.begin();
   const account_name_type* parent_author = &start_parent_author;

   if( start_permlink.size() )
   {
      const comment_object& comment = _db.get_comment( start_parent_author, start_permlink );
      itr = last_update_idx.iterator_to( comment );
      parent_author = &comment.parent_author;
   }
   else if( start_parent_author.size() )
   {
      itr = last_update_idx.lower_bound( start_parent_author );
   }

   results.reserve( limit );

   while( itr != last_update_idx.end() && results.size() < limit && itr->parent_author == *parent_author )
   {
      results.push_back( *itr );
      results.back().active_votes = get_active_votes( itr->author, to_string( itr->permlink ) );
      ++itr;
   }
   
   return results;
}

discussion database_api::get_discussion( comment_id_type id, uint32_t truncate_body )const
{
   discussion d = my->_db.get( id );
   set_url( d );
   
   d.active_votes = get_active_votes( d.author, d.permlink );
   d.active_views = get_active_views( d.author, d.permlink );
   d.active_shares = get_active_shares( d.author, d.permlink );
   d.active_mod_tags = get_active_mod_tags( d.author, d.permlink );

   d.body_length = d.body.size();
   if( truncate_body ) 
   {
      d.body = d.body.substr( 0, truncate_body );

      if( !fc::is_utf8( d.body ) )
      {
         d.body = fc::prune_invalid_utf8( d.body );
      }  
   }
   return d;
}


template< typename Index, typename StartItr >
vector< discussion > database_api::get_discussions( 
   const discussion_query& query,
   const string& board,
   const string& tag,
   comment_id_type parent,
   const Index& tidx, 
   StartItr tidx_itr,
   uint32_t truncate_body,
   const std::function< bool( const comment_api_obj& ) >& filter,
   const std::function< bool( const comment_api_obj& ) >& exit,
   const std::function< bool( const tags::tag_object& ) >& tag_exit,
   bool ignore_parent
   )const
{
   vector< discussion > results;

   if( !my->_db.has_index<tags::tag_index>() )
   {
      return results;
   }
      
   const auto& cidx = my->_db.get_index< tags::tag_index >().indices().get< tags::by_comment >();
   comment_id_type start;

   if( query.start_author && query.start_permlink ) 
   {
      start = my->_db.get_comment( *query.start_author, *query.start_permlink ).id;
      auto itr = cidx.find( start );
      while( itr != cidx.end() && itr->comment == start )
      {
         if( itr->tag == tag && itr->board == board ) 
         {
            tidx_itr = tidx.iterator_to( *itr );
            break;
         }
         ++itr;
      }
   }

   uint32_t count = query.limit;
   uint64_t itr_count = 0;
   uint64_t filter_count = 0;
   uint64_t exc_count = 0;
   uint64_t max_itr_count = 10 * query.limit;
   while( count > 0 && tidx_itr != tidx.end() )
   {
      ++itr_count;
      if( itr_count > max_itr_count )
      {
         wlog( "Maximum iteration count exceeded serving query: ${q}", ("q", query) );
         wlog( "count=${count}   itr_count=${itr_count}   filter_count=${filter_count}   exc_count=${exc_count}",
               ("count", count)("itr_count", itr_count)("filter_count", filter_count)("exc_count", exc_count) );
         break;
      }
      if( tidx_itr->tag != tag || tidx_itr->board != board || ( !ignore_parent && tidx_itr->parent != parent ) )
      {
         break;
      } 
      try
      {
         if( !query.include_private )
         {
            if( tidx_itr->encrypted )
            {
               ++tidx_itr;
               continue;
            }
         }

         if( query.post_include_time.size() )
         {
            time_point now = my->_db.head_block_time();
            time_point created = tidx_itr->created;
            bool old_post = false;

            if( query.post_include_time == post_time_values[ ALL_TIME ] )
            {
               old_post = false;
            }
            else if( query.post_include_time == post_time_values[ LAST_HOUR ] )
            {
               if( created + fc::hours(1) > now )
               {
                  old_post = true;
               }
            }
            else if( query.post_include_time == post_time_values[ LAST_DAY ] )
            {
               if( created + fc::days(1) > now ) 
               {
                  old_post = true;
               }
            }
            else if( query.post_include_time == post_time_values[ LAST_WEEK ] )
            {
               if( created + fc::days(7) > now ) 
               {
                  old_post = true;
               }
            }
            else if( query.post_include_time == post_time_values[ LAST_MONTH ] )
            {
               if( created + fc::days(30) > now ) 
               {
                  old_post = true;
               }
            }
            else if( query.post_include_time == post_time_values[ LAST_YEAR ] )
            {
               if( created + fc::days(365) > now ) 
               {
                  old_post = true;
               }
            }

            if( old_post )
            {
               ++tidx_itr;
               continue;
            }
         }

         if( query.max_rating.size() )
         {
            bool over_rating = false;

            if( query.max_rating == post_rating_values[ FAMILY ] )
            {
               if( tidx_itr->rating == EXPLICIT || tidx_itr->rating == MATURE || tidx_itr->rating == GENERAL )
               {
                  over_rating = true;
               }
            }
            else if( query.max_rating == post_rating_values[ GENERAL ] )
            {
               if( tidx_itr->rating == EXPLICIT || tidx_itr->rating == MATURE )
               {
                  over_rating = true;
               }
            }
            else if( query.max_rating == post_rating_values[ MATURE ] )
            {
               if( tidx_itr->rating == EXPLICIT )
               {
                  over_rating = true;
               }
            }
            
            if( over_rating )
            {
               ++tidx_itr;
               continue;
            }
         }

         if( query.select_authors.size() && query.select_authors.find( tidx_itr->author ) == query.select_authors.end() )
         {
            ++tidx_itr;
            continue;
         }

         if( query.select_languages.size() && query.select_languages.find( tidx_itr->language ) == query.select_languages.end() )
         {
            ++tidx_itr;
            continue;
         }

         if( query.select_boards.size() )
         {
            auto tag_itr = tidx.lower_bound( tidx_itr->comment );

            bool found = false;
            while( tag_itr != tidx.end() && tag_itr->comment == tidx_itr->comment )
            {
               if( query.select_boards.find( tag_itr->board ) != query.select_boards.end() )
               {
                  found = true; 
                  break;
               }
               ++tag_itr;
            }
            if( !found )
            {
               ++tidx_itr;
               continue;
            }
         }

         if( query.select_tags.size() )
         {
            auto tag_itr = tidx.lower_bound( tidx_itr->comment );

            bool found = false;
            while( tag_itr != tidx.end() && tag_itr->comment == tidx_itr->comment )
            {
               if( query.select_tags.find( tag_itr->tag ) != query.select_tags.end() )
               {
                  found = true;
                  break;
               }
               ++tag_itr;
            }
            if( !found )
            {
               ++tidx_itr;
               continue;
            }
         }

         if( query.filter_authors.size() && query.filter_authors.find( tidx_itr->author ) != query.filter_authors.end() )
         {
            ++tidx_itr;
            continue;
         }

         if( query.filter_languages.size() && query.filter_languages.find( tidx_itr->language ) != query.filter_languages.end() )
         {
            ++tidx_itr;
            continue;
         }

         if( query.filter_boards.size() )
         {
            auto tag_itr = tidx_idx.lower_bound( tidx_itr->comment );

            bool found = false;
            while( tag_itr != tidx_idx.end() && tag_itr->comment == tidx_itr->comment )
            {
               if( query.filter_boards.find( tag_itr->board ) != query.filter_boards.end() )
               {
                  found = true; 
                  break;
               }
               ++tag_itr;
            }
            if( found ) 
            {
               ++tidx_itr;
               continue;
            }
         }

         if( query.filter_tags.size() )
         {
            auto tag_itr = tidx_idx.lower_bound( tidx_itr->comment );

            bool found = false;
            while( tag_itr != tag_idx.end() && tag_itr->comment == tidx_itr->comment )
            {
               if( query.filter_tags.find( tag_itr->tag ) != query.filter_tags.end() )
               {
                  found = true;
                  break;
               }
               ++tag_itr;
            }
            if( found ) 
            {
               ++tidx_itr;
               continue;
            }
         }

         results.push_back( get_discussion( tidx_itr->comment, truncate_body ) );

         if( filter( results.back() ) )
         {
            results.pop_back();
            ++filter_count;
         }
         else if( exit( results.back() ) || tag_exit( *tidx_itr ) )
         {
            results.pop_back();
            break;
         }
         else
         {
            --count;
         }  
      }
      catch ( const fc::exception& e )
      {
         ++exc_count;
         edump((e.to_detail_string()));
      }
      ++tidx_itr;
   }
   return results;
}

comment_id_type database_api::get_parent( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      comment_id_type parent;
      if( query.parent_author && query.parent_permlink ) 
      {
         parent = my->_db.get_comment( *query.parent_author, *query.parent_permlink ).id;
      }
      return parent;
   });
}

vector< discussion > database_api::get_discussions_by_sort_rank( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_sort_rank( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_sort_rank( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   auto board = fc::to_lower( query.board );
   auto tag = fc::to_lower( query.tag );
   auto parent = get_parent( query );

   string sort_type;
   string sort_time;

   if( query.sort_type.size() && query.sort_time.size() )
   {
      sort_type = query.sort_type;
      sort_time = query.sort_time;
   }

   const auto& tag_sort_index = _db.get_index< tags::tag_quality_sort_index >().indices().get< tags::by_parent_quality_standard >();

   if( sort_type == sort_option_values[ QUALITY_SORT ] )
   {
      if( sort_time == sort_time_values[ ACTIVE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_quality_sort_index >().indices().get< tags::by_parent_quality_active >();
      }
      else if( sort_time == sort_time_values[ RAPID_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_quality_sort_index >().indices().get< tags::by_parent_quality_rapid >();
      }
      else if( sort_time == sort_time_values[ STANDARD_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_quality_sort_index >().indices().get< tags::by_parent_quality_standard >();
      }
      else if( sort_time == sort_time_values[ TOP_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_quality_sort_index >().indices().get< tags::by_parent_quality_top >();
      }
      else if( sort_time == sort_time_values[ ELITE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_quality_sort_index >().indices().get< tags::by_parent_quality_elite >();
      }
   }
   else if( sort_type == sort_option_values[ VOTES_SORT ] )
   {
      if( sort_time == sort_time_values[ ACTIVE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_votes_sort_index >().indices().get< tags::by_parent_votes_active >();
      }
      else if( sort_time == sort_time_values[ RAPID_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_votes_sort_index >().indices().get< tags::by_parent_votes_rapid >();
      }
      else if( sort_time == sort_time_values[ STANDARD_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_votes_sort_index >().indices().get< tags::by_parent_votes_standard >();
      }
      else if( sort_time == sort_time_values[ TOP_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_votes_sort_index >().indices().get< tags::by_parent_votes_top >();
      }
      else if( sort_time == sort_time_values[ ELITE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_votes_sort_index >().indices().get< tags::by_parent_votes_elite >();
      }
   }
   else if( sort_type == sort_option_values[ VIEWS_SORT ] )
   {
      if( sort_time == sort_time_values[ ACTIVE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_views_sort_index >().indices().get< tags::by_parent_views_active >();
      }
      else if( sort_time == sort_time_values[ RAPID_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_views_sort_index >().indices().get< tags::by_parent_views_rapid >();
      }
      else if( sort_time == sort_time_values[ STANDARD_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_views_sort_index >().indices().get< tags::by_parent_views_standard >();
      }
      else if( sort_time == sort_time_values[ TOP_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_views_sort_index >().indices().get< tags::by_parent_views_top >();
      }
      else if( sort_time == sort_time_values[ ELITE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_views_sort_index >().indices().get< tags::by_parent_views_elite >();
      }
   }
   else if( sort_type == sort_option_values[ SHARES_SORT ] )
   {
      if( sort_time == sort_time_values[ ACTIVE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_shares_sort_index >().indices().get< tags::by_parent_shares_active >();
      }
      else if( sort_time == sort_time_values[ RAPID_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_shares_sort_index >().indices().get< tags::by_parent_shares_rapid >();
      }
      else if( sort_time == sort_time_values[ STANDARD_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_shares_sort_index >().indices().get< tags::by_parent_shares_standard >();
      }
      else if( sort_time == sort_time_values[ TOP_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_shares_sort_index >().indices().get< tags::by_parent_shares_top >();
      }
      else if( sort_time == sort_time_values[ ELITE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_shares_sort_index >().indices().get< tags::by_parent_shares_elite >();
      }
   }
   else if( sort_type == sort_option_values[ COMMENTS_SORT ] )
   {
      if( sort_time == sort_time_values[ ACTIVE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_comments_sort_index >().indices().get< tags::by_parent_comments_active >();
      }
      else if( sort_time == sort_time_values[ RAPID_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_comments_sort_index >().indices().get< tags::by_parent_comments_rapid >();
      }
      else if( sort_time == sort_time_values[ STANDARD_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_comments_sort_index >().indices().get< tags::by_parent_comments_standard >();
      }
      else if( sort_time == sort_time_values[ TOP_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_comments_sort_index >().indices().get< tags::by_parent_comments_top >();
      }
      else if( sort_time == sort_time_values[ ELITE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_comments_sort_index >().indices().get< tags::by_parent_comments_elite >();
      }
   }
   else if( sort_type == sort_option_values[ POPULAR_SORT ] )
   {
      if( sort_time == sort_time_values[ ACTIVE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_popular_sort_index >().indices().get< tags::by_parent_popular_active >();
      }
      else if( sort_time == sort_time_values[ RAPID_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_popular_sort_index >().indices().get< tags::by_parent_popular_rapid >();
      }
      else if( sort_time == sort_time_values[ STANDARD_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_popular_sort_index >().indices().get< tags::by_parent_popular_standard >();
      }
      else if( sort_time == sort_time_values[ TOP_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_popular_sort_index >().indices().get< tags::by_parent_popular_top >();
      }
      else if( sort_time == sort_time_values[ ELITE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_popular_sort_index >().indices().get< tags::by_parent_popular_elite >();
      }
   }
   else if( sort_type == sort_option_values[ VIRAL_SORT ] )
   {
      if( sort_time == sort_time_values[ ACTIVE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_viral_sort_index >().indices().get< tags::by_parent_viral_active >();
      }
      else if( sort_time == sort_time_values[ RAPID_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_viral_sort_index >().indices().get< tags::by_parent_viral_rapid >();
      }
      else if( sort_time == sort_time_values[ STANDARD_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_viral_sort_index >().indices().get< tags::by_parent_viral_standard >();
      }
      else if( sort_time == sort_time_values[ TOP_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_viral_sort_index >().indices().get< tags::by_parent_viral_top >();
      }
      else if( sort_time == sort_time_values[ ELITE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_viral_sort_index >().indices().get< tags::by_parent_viral_elite >();
      }
   }
   else if( sort_type == sort_option_values[ DISCUSSION_SORT ] )
   {
      if( sort_time == sort_time_values[ ACTIVE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_discussion_sort_index >().indices().get< tags::by_parent_discussion_active >();
      }
      else if( sort_time == sort_time_values[ RAPID_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_discussion_sort_index >().indices().get< tags::by_parent_discussion_rapid >();
      }
      else if( sort_time == sort_time_values[ STANDARD_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_discussion_sort_index >().indices().get< tags::by_parent_discussion_standard >();
      }
      else if( sort_time == sort_time_values[ TOP_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_discussion_sort_index >().indices().get< tags::by_parent_discussion_top >();
      }
      else if( sort_time == sort_time_values[ ELITE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_discussion_sort_index >().indices().get< tags::by_parent_discussion_elite >();
      }
   }
   else if( sort_type == sort_option_values[ PROMINENT_SORT ] )
   {
      if( sort_time == sort_time_values[ ACTIVE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_prominent_sort_index >().indices().get< tags::by_parent_prominent_active >();
      }
      else if( sort_time == sort_time_values[ RAPID_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_prominent_sort_index >().indices().get< tags::by_parent_prominent_rapid >();
      }
      else if( sort_time == sort_time_values[ STANDARD_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_prominent_sort_index >().indices().get< tags::by_parent_prominent_standard >();
      }
      else if( sort_time == sort_time_values[ TOP_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_prominent_sort_index >().indices().get< tags::by_parent_prominent_top >();
      }
      else if( sort_time == sort_time_values[ ELITE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_prominent_sort_index >().indices().get< tags::by_parent_prominent_elite >();
      }
   }
   else if( sort_type == sort_option_values[ CONVERSATION_SORT ] )
   {
      if( sort_time == sort_time_values[ ACTIVE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_conversation_sort_index >().indices().get< tags::by_parent_conversation_active >();
      }
      else if( sort_time == sort_time_values[ RAPID_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_conversation_sort_index >().indices().get< tags::by_parent_conversation_rapid >();
      }
      else if( sort_time == sort_time_values[ STANDARD_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_conversation_sort_index >().indices().get< tags::by_parent_conversation_standard >();
      }
      else if( sort_time == sort_time_values[ TOP_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_conversation_sort_index >().indices().get< tags::by_parent_conversation_top >();
      }
      else if( sort_time == sort_time_values[ ELITE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_conversation_sort_index >().indices().get< tags::by_parent_conversation_elite >();
      }
   }
   else if( sort_type == sort_option_values[ DISCOURSE_SORT ] )
   {
      if( sort_time == sort_time_values[ ACTIVE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_discourse_sort_index >().indices().get< tags::by_parent_discourse_active >();
      }
      else if( sort_time == sort_time_values[ RAPID_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_discourse_sort_index >().indices().get< tags::by_parent_discourse_rapid >();
      }
      else if( sort_time == sort_time_values[ STANDARD_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_discourse_sort_index >().indices().get< tags::by_parent_discourse_standard >();
      }
      else if( sort_time == sort_time_values[ TOP_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_discourse_sort_index >().indices().get< tags::by_parent_discourse_top >();
      }
      else if( sort_time == sort_time_values[ ELITE_TIME ] )
      {
         const auto& tag_sort_index = _db.get_index< tags::tag_discourse_sort_index >().indices().get< tags::by_parent_discourse_elite >();
      }
   }

   auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( board, tag, parent, std::numeric_limits<double>::max() ) );
   return get_discussions( query, board, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ) { return c.net_reward <= 0; } );
}


vector< discussion > database_api::get_discussions_by_feed( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_feed( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_feed( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   auto start_author = query.start_author ? *( query.start_author ) : "";
   auto start_permlink = query.start_permlink ? *( query.start_permlink ) : "";
   string account;
   if( query.account.size() )
   {
      account = query.account;
      const account_object& acc_obj = _db.get_account( account );
   }
   else
   {
      return vector< discussion >();
   }
   
   const auto& f_idx = _db.get_index< feed_index >().indices().get< by_new_account_type >();
   auto feed_itr = f_idx.lower_bound( account );
   feed_reach_type reach = FOLLOW_FEED;

   if( query.feed_type.size() )
   {
      for( auto i = 0; i < feed_reach_values.size(); i++ )
      {
         if( query.feed_type == feed_reach_values[ i ] )
         {
            reach = feed_reach_type( i );
            break;
         }
      }

      feed_itr = f_idx.lower_bound( boost::make_tuple( account, reach ) );
   }

   const auto& c_idx = _db.get_index< feed_index >().indices().get< by_comment >();
   if( start_author.size() || start_permlink.size() )
   {
      auto start_c = c_idx.find( boost::make_tuple( _db.get_comment( start_author, start_permlink ).id, account ) );
      FC_ASSERT( start_c != c_idx.end(),
         "Comment is not in account's feed" );
      feed_itr = f_idx.iterator_to( *start_c );
   }

   vector< discussion > results;
   results.reserve( query.limit );

   while( results.size() < query.limit && feed_itr != f_idx.end() )
   {
      if( feed_itr->account != account )
      {
         break;
      }
      try
      {
         results.push_back( get_discussion( feed_itr->comment ) );
         results.back().feed = feed_api_obj( *feed_itr );
      }
      catch ( const fc::exception& e )
      {
         edump((e.to_detail_string()));
      }

      ++feed_itr;
   }

   return results;
}

vector< discussion > database_api::get_discussions_by_blog( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_blog( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_blog( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();

   auto start_author = query.start_author ? *( query.start_author ) : "";
   auto start_permlink = query.start_permlink ? *( query.start_permlink ) : "";

   string account;
   string board;
   string tag;
   if( query.account.size() )
   {
      account = query.account;
      const account_object& acc_obj = _db.get_account( account );
   }

   if( query.board.size() )
   {
      board = query.board;
      const board_object& board_obj = _db.get_board( board );
   }

   if( query.tag.size() )
   {
      tag = query.tag;
      const tag_following_object& tag_obj = _db.get_tag_following( tag );
   }

   const auto& tag_idx = _db.get_index< tags::tag_index >().indices().get<tags::by_comment>();
   const auto& c_idx = _db.get_index< blog_index >().indices().get< by_comment >();
   const auto& b_idx = _db.get_index< blog_index >().indices().get< by_new_account_blog >();

   string subject = account;

   if( query.blog_type.size() )
   {
      if( query.blog_type == blog_reach_values[ ACCOUNT_BLOG ] )
      {
         const auto& b_idx = _db.get_index< blog_index >().indices().get< by_new_account_blog >();
      }
      else if( query.blog_type == blog_reach_values[ BOARD_BLOG ] )
      {
         const auto& b_idx = _db.get_index< blog_index >().indices().get< by_new_board_blog >();
         subject = board;
      }
      else if( query.blog_type == blog_reach_values[ TAG_BLOG ] )
      {
         const auto& b_idx = _db.get_index< blog_index >().indices().get< by_new_tag_blog >();
         subject = tag;
      }
   }

   auto blog_itr = b_idx.lower_bound( subject );

   if( start_author.size() || start_permlink.size() )
   {
      auto start_c = c_idx.find( boost::make_tuple( _db.get_comment( start_author, start_permlink ).id, account ) );
      FC_ASSERT( start_c != c_idx.end(),
         "Comment is not in account's blog" );
      blog_itr = b_idx.iterator_to( *start_c );
   }

   vector< discussion > results;
   results.reserve( query.limit );

   while( results.size() < query.limit && blog_itr != b_idx.end() )
   { 
      try
      {
         if( account.size() && blog_itr->account != account && query.blog_type == blog_reach_values[ ACCOUNT_BLOG ] )
         {
            break;
         }
         if( board.size() && blog_itr->board != board && query.blog_type == blog_reach_values[ BOARD_BLOG ] )
         {
            break;
         }
         if( tag.size() && blog_itr->tag != tag && query.blog_type == blog_reach_values[ TAG_BLOG ] )
         {
            break;
         }

         if( !query.include_private )
         {
            auto tag_itr = tag_idx.lower_bound( blog_itr->comment );
            if( tag_itr->encrypted )
            {
               ++blog_itr;
               continue;
            }
         }

         if( query.max_rating.size() )
         {
            auto tag_itr = tag_idx.lower_bound( blog_itr->comment );
            bool over_rating = false;

            if( query.max_rating == post_rating_values[ FAMILY ] )
            {
               if( tag_itr->rating == EXPLICIT || tag_itr->rating == MATURE || tag_itr->rating == GENERAL )
               {
                  over_rating = true;
               }
            }
            else if( query.max_rating == post_rating_values[ GENERAL ] )
            {
               if( tag_itr->rating == EXPLICIT || tag_itr->rating == MATURE )
               {
                  over_rating = true;
               }
            }
            else if( query.max_rating == post_rating_values[ MATURE ] )
            {
               if( tag_itr->rating == EXPLICIT )
               {
                  over_rating = true;
               }
            }
            
            if( over_rating )
            {
               ++blog_itr;
               continue;
            }
         }

         if( query.select_authors.size() && query.select_authors.find( blog_itr->account ) == query.select_authors.end() )
         {
            ++blog_itr;
            continue;
         }

         if( query.select_languages.size() ) 
         {
            auto tag_itr = tag_idx.lower_bound( blog_itr->comment );

            bool found = false;
            while( tag_itr != tag_idx.end() && tag_itr->comment == blog_itr->comment )
            {
               if( query.select_languages.find( tag_itr->language ) != query.select_languages.end() )
               {
                  found = true; 
                  break;
               }
               ++tag_itr;
            }
            if( !found ) 
            {
               ++blog_itr;
               continue;
            }
         }

         if( query.select_boards.size() ) 
         {
            auto tag_itr = tag_idx.lower_bound( blog_itr->comment );

            bool found = false;
            while( tag_itr != tag_idx.end() && tag_itr->comment == blog_itr->comment )
            {
               if( query.select_boards.find( tag_itr->board ) != query.select_boards.end() )
               {
                  found = true; 
                  break;
               }
               ++tag_itr;
            }
            if( !found ) 
            {
               ++blog_itr;
               continue;
            }
         }

         if( query.select_tags.size() ) 
         {
            auto tag_itr = tag_idx.lower_bound( blog_itr->comment );

            bool found = false;
            while( tag_itr != tag_idx.end() && tag_itr->comment == blog_itr->comment )
            {
               if( query.select_tags.find( tag_itr->tag ) != query.select_tags.end() )
               {
                  found = true; 
                  break;
               }
               ++tag_itr;
            }
            if( !found ) 
            {
               ++blog_itr;
               continue;
            }
         }

         if( query.filter_authors.size() && query.filter_authors.find( blog_itr->account ) != query.filter_authors.end() )
         {
            ++blog_itr;
            continue;
         }

         if( query.filter_languages.size() )
         {
            auto tag_itr = tag_idx.lower_bound( blog_itr->comment );

            bool found = false;
            while( tag_itr != tag_idx.end() && tag_itr->comment == blog_itr->comment )
            {
               if( query.filter_languages.find( tag_itr->language ) != query.filter_languages.end() )
               {
                  found = true; 
                  break;
               }
               ++tag_itr;
            }
            if( found ) 
            {
               ++blog_itr;
               continue;
            }
         }

         if( query.filter_boards.size() ) 
         {
            auto tag_itr = tag_idx.lower_bound( blog_itr->comment );

            bool found = false;
            while( tag_itr != tag_idx.end() && tag_itr->comment == blog_itr->comment )
            {
               if( query.filter_boards.find( tag_itr->board ) != query.filter_boards.end() )
               {
                  found = true; 
                  break;
               }
               ++tag_itr;
            }
            if( found ) 
            {
               ++blog_itr;
               continue;
            }
         }

         if( query.filter_tags.size() ) 
         {
            auto tag_itr = tag_idx.lower_bound( blog_itr->comment );

            bool found = false;
            while( tag_itr != tag_idx.end() && tag_itr->comment == blog_itr->comment )
            {
               if( query.filter_tags.find( tag_itr->tag ) != query.filter_tags.end() )
               {
                  found = true; 
                  break;
               }
               ++tag_itr;
            }
            if( found ) 
            {
               ++blog_itr;
               continue;
            }
         }
         
         results.push_back( get_discussion( blog_itr->comment, query.truncate_body ) );
         results.back().blog = blog_api_obj( *blog_itr );
      }
      catch ( const fc::exception& e )
      {
         edump((e.to_detail_string()));
      }

      ++blog_itr;
   }

   return results;
}

vector< discussion > database_api::get_discussions_by_recommended( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_recommended( query );
   });
}

/**
 * Recommended Feed is generated with a psudeorandom 
 * ordering of posts from the authors, 
 * boards, and tags that the account has previously 
 * interacted with.
 * Pulls the top posts from each sorting index
 * of each author blog, board, and tag that the
 * account has previously interacted with.
 * Adds the top posts by each index from the highest adjacency 
 * authors, boards and tags that are currently not followed by the account.
 * Randomly pulls the limit amount of posts from
 * this set of posts.
 */
vector< discussion > database_api_impl::get_discussions_by_recommended( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   vector< discussion > results;
   results.reserve( query.limit );

   query.validate();
   if( query.account.size() )
   {
      account_name_type account = query.account;
      const auto& recommend_idx = _db.get_index< tags::account_recommendations_index >().indices().get< tags::by_account >();
      auto recommend_itr = recommend_idx.find( account );

      if( recommend_itr != recommend_idx.end() )
      {
         const tags::account_recommendations_object& aro = *recommend_itr;
         vector< comment_id_type > recommended_posts;
         recommended_posts.reserve( aro.recommended_posts.size() );

         for( auto post : aro.recommended_posts )
         {
            recommended_posts.push_back( post );
         }

         auto now_hi = uint64_t( _db.head_block_time().time_since_epoch().count() ) << 32;
         for( uint32_t i = 0; i < query.limit; ++i )
         {
            uint64_t k = now_hi +      uint64_t(i)*26857571057736338717ULL;
            uint64_t l = now_hi >> 1 + uint64_t(i)*95198191871878293511ULL;
            uint64_t m = now_hi >> 2 + uint64_t(i)*58919729024841961988ULL;
            uint64_t n = now_hi >> 3 + uint64_t(i)*27137164109707054410ULL;
            
            k ^= (l >> 7);
            l ^= (m << 9);
            m ^= (n >> 5);
            n ^= (k << 3);

            k*= 14226572565896741612ULL;
            l*= 91985878658736871034ULL;
            m*= 30605588311672529089ULL;
            n*= 43069213742576315243ULL;

            k ^= (l >> 2);
            l ^= (m << 4);
            m ^= (n >> 1);
            n ^= (k << 9);

            k*= 79477756532752495704ULL;
            l*= 94908025588282034792ULL;
            m*= 26941980616458623416ULL;
            n*= 31902236862011382134ULL;

            uint64_t rand = (k ^ l) ^ (m ^ n) ; 
            uint32_t max = recommended_posts.size() - i;

            uint32_t j = i + rand % max;
            std::swap( recommended_posts[i], recommended_posts[j] );
            results.push_back( get_discussion( recommended_posts[i] ) );    // Returns randomly selected comments from the recommended posts collection
         }
      }
      else
      {
         return vector< discussion >();
      }
   }
   else
   {
      return vector< discussion >();
   }

   return results;
}

vector< discussion > database_api::get_discussions_by_comments( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_comments( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_comments( const discussion_query& query )const
{
   vector< discussion > results;
   query.validate();
   FC_ASSERT( query.start_author,
      "Must get comments for a specific author" );
   auto start_author = *( query.start_author );
   auto start_permlink = query.start_permlink ? *( query.start_permlink ) : "";

   const auto& c_idx = _db.get_index< comment_index >().indices().get< by_permlink >();
   const auto& t_idx = _db.get_index< comment_index >().indices().get< by_author_last_update >();
   auto comment_itr = t_idx.lower_bound( start_author );

   if( start_permlink.size() )
   {
      auto start_c = c_idx.find( boost::make_tuple( start_author, start_permlink ) );
      FC_ASSERT( start_c != c_idx.end(),
         "Comment is not in account's comments" );
      comment_itr = t_idx.iterator_to( *start_c );
   }

   results.reserve( query.limit );

   while( results.size() < query.limit && comment_itr != t_idx.end() )
   {
      if( comment_itr->author != start_author )
      {
         break;
      } 
      if( comment_itr->parent_author.size() > 0 )
      {
         try
         {
            results.push_back( get_discussion( comment_itr->id ) );
         }
         catch( const fc::exception& e )
         {
            edump( (e.to_detail_string() ) );
         }
      }

      ++comment_itr;
   }
   return results;
}


vector< discussion > database_api::get_discussions_by_payout( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_payout( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_payout( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string board = fc::to_lower( query.board );
   string tag = fc::to_lower( query.tag );
   auto parent = get_parent( query );

   const auto& tidx = _db.get_index< tags::tag_index >().indices().get< tags::by_net_reward >();
   auto tidx_itr = tidx.lower_bound( board, tag );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
}

vector< discussion > database_api::get_post_discussions_by_payout( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_post_discussions_by_payout( query );
   });
}

vector< discussion > database_api_impl::get_post_discussions_by_payout( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string board = fc::to_lower( query.board );
   string tag = fc::to_lower( query.tag );
   auto parent = comment_id_type();    // Root posts

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_reward_fund_net_reward>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( board, tag, true ) );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
}

vector< discussion > database_api::get_comment_discussions_by_payout( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_comment_discussions_by_payout( query );
   });
}

vector< discussion > database_api_impl::get_comment_discussions_by_payout( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   auto board = fc::to_lower( query.board );
   auto tag = fc::to_lower( query.tag );
   auto parent = comment_id_type(1);

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_reward_fund_net_reward>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( board, tag, false ) );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
}



vector< discussion > database_api::get_discussions_by_created( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_created( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_created( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   auto board = fc::to_lower( query.board );
   auto tag = fc::to_lower( query.tag );
   auto parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_created>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( board, tag, parent, fc::time_point::maximum() )  );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body );
}

vector< discussion > database_api::get_discussions_by_active( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_active( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_active( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   auto board = fc::to_lower( query.board );
   auto tag = fc::to_lower( query.tag );
   auto parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_active>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( board, tag, parent, fc::time_point::maximum() )  );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body );
}

vector< discussion > database_api::get_discussions_by_votes( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_votes( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_votes( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   auto board = fc::to_lower( query.board );
   auto tag = fc::to_lower( query.tag );
   auto parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_net_votes>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( board, tag, parent, std::numeric_limits<int32_t>::max() ) );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body );
}

vector< discussion > database_api::get_discussions_by_views( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_views( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_views( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   auto board = fc::to_lower( query.board );
   auto tag = fc::to_lower( query.tag );
   auto parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_view_count>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( board, tag, parent, std::numeric_limits<int32_t>::max() )  );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body );
}

vector< discussion > database_api::get_discussions_by_shares( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_shares( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_shares( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   auto board = fc::to_lower( query.board );
   auto tag = fc::to_lower( query.tag );
   auto parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_share_count>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( board, tag, parent, std::numeric_limits<int32_t>::max() )  );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body );
}

vector< discussion > database_api::get_discussions_by_children( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_children( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_children( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   auto board = fc::to_lower( query.board );
   auto tag = fc::to_lower( query.tag );
   auto parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_children>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( board, tag, parent, std::numeric_limits<int32_t>::max() )  );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body );
}

vector< discussion > database_api::get_discussions_by_vote_power( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_vote_power( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_vote_power( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string board = fc::to_lower( query.board );
   string tag = fc::to_lower( query.tag );
   auto parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_vote_power>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( board, tag, parent, std::numeric_limits<int32_t>::max() ) );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body );
}

vector< discussion > database_api::get_discussions_by_view_power( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_view_power( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_view_power( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string board = fc::to_lower( query.board );
   string tag = fc::to_lower( query.tag );
   auto parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_view_power>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( board, tag, parent, std::numeric_limits<int32_t>::max() ) );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body );
}


vector< discussion > database_api::get_discussions_by_share_power( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_share_power( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_share_power( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string board = fc::to_lower( query.board );
   string tag = fc::to_lower( query.tag );
   auto parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_share_power>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( board, tag, parent, std::numeric_limits<int32_t>::max() ) );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body );
}

vector< discussion > database_api::get_discussions_by_comment_power( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_comment_power( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_comment_power( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string board = fc::to_lower( query.board );
   string tag = fc::to_lower( query.tag );
   auto parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_comment_power>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( board, tag, parent, std::numeric_limits<int32_t>::max() ) );

   return get_discussions( query, board, tag, parent, tidx, tidx_itr, query.truncate_body );
}

/**
 * This call assumes root already stored as part of state, it will
 * modify root. Replies to contain links to the reply posts and then
 * add the reply discussions to the state. This method also fetches
 * any accounts referenced by authors.
 */
void database_api::recursively_fetch_content( state& _state, discussion& root, set< string >& referenced_accounts )const
{
   return my->_db.with_read_lock( [&]()
   {
      try
      {
         if( root.author.size() )
         {
            referenced_accounts.insert( root.author );
         }
         
         vector< discussion > replies = get_content_replies( root.author, root.permlink );

         for( auto& r : replies )
         {
            try
            {
               recursively_fetch_content( _state, r, referenced_accounts );
               root.replies.push_back( r.author + "/" + r.permlink );
               _state.content[ r.author + "/" + r.permlink ] = std::move(r);

               if( r.author.size() )
               {
                  referenced_accounts.insert( r.author );
               }
            }
            catch ( const fc::exception& e )
            {
               edump((e.to_detail_string()));
            }
         }
      }
      FC_CAPTURE_AND_RETHROW( (root.author)(root.permlink) )
   });
}

state database_api::get_state( string path )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_state( path );
   });
}

state database_api_impl::get_state( string path )const
{
   state _state;
   _state.props = _db.get_dynamic_global_properties();
   _state.current_route = path;

   try {
      if( path.size() && path[0] == '/' )
      {
         path = path.substr(1);   // remove '/' from front
      }
      
      vector< tag_api_obj > trending_tags = get_top_tags( std::string(), 50 );

      for( const auto& t : trending_tags )
      {
         _state.tag_idx.trending.push_back( string( t.tag ) );    // Trending tags record of highest voted tags
      }

      set< string > accounts;
      vector< string > part;
      part.reserve(4);
      boost::split( part, path, boost::is_any_of("/") );
      part.resize( std::max( part.size(), size_t(4) ) );
      string acnt;
      string board;
      string tag;
      string section;

      for( auto item : part )
      {
         if( item[0] == '@' )
         {
            string acnt = fc::to_lower( item.substr(1) );
            vector< string > accvec;
            accvec.push_back( acnt );
            _state.accounts[ acnt ] = get_full_accounts( accvec )[0];
         }
         else if( item[0] == '&' )
         {
            string board = fc::to_lower( item.substr(1) );
            vector< string > boardvec;
            boardvec.push_back( board );
            _state.boards[ board ] = get_boards( boardvec )[0];
         }
         else if( item[0] == '#' )
         {
            string tag = fc::to_lower( item.substr(1) );
            vector< string > tagvec;
            tagvec.push_back( tag );
            _state.tags[ tag ] = get_tag_followings( tagvec )[0];
         }
         else
         {
            string section = fc::to_lower( item.substr(1) );
         }
      }

      if( section == "recent-replies" )
      {
         vector< discussion > replies = get_replies_by_last_update( acnt, "", 50 );
         _state.recent_replies[ acnt ] = vector< string >();

         for( const auto& reply : replies )
         {
            string reply_ref = reply.author + "/" + reply.permlink;
            _state.content[ reply_ref ] = reply;
            _state.recent_replies[ acnt ].push_back( reply_ref );
         }
      }
      else if( section == "posts" || section == "comments" )
      {
         int count = 0;
         const auto& pidx = _db.get_index< comment_index >().indices().get< by_author_last_update >();
         auto itr = pidx.lower_bound( acnt );
         _state.comments[ acnt ] = vector< string >();

         while( itr != pidx.end() && itr->author == acnt && count < 20 )
         {
            if( itr->parent_author.size() )
            {
               string link = acnt + "/" + to_string( itr->permlink );
               _state.recent_replies[ acnt ].push_back( link );
               _state.content[ link ] = *itr;
               ++count;
            }
            ++itr;
         }
      }
      else if( section == "blog" )
      {
         discussion_query q;
         q.account = acnt;
         q.blog_type = ACCOUNT_BLOG;
         vector< discussion > blog_posts = get_discussions_by_blog( q );
         const auto& blog_idx = _db.get_index< blog_index >().indices().get< by_comment_account >();
         _state.blogs[ acnt ] = vector< string >();

         for( auto b : blog_posts )
         {
            string link = b.author + "/" + b.permlink;
            _state.blogs[ acnt ].push_back( link );
            _state.content[ link ] = b;
         }
      }
      else if( section == "feed" )
      {
         discussion_query q;
         q.account = acnt;
         q.feed_type = FOLLOW_FEED;
         vector< discussion > feed_posts = get_discussions_by_feed( q );
         const auto& feed_idx = _db.get_index< feed_index >().indices().get< by_new_account >();
         _state.blogs[ acnt ] = vector< string >();

         for( auto f: feed_posts )
         {
            string link = f.author + "/" + f.permlink;
            _state.feeds[ acnt ].push_back( link );
            _state.content[ link ] = f;
         }
      }
      else if( section == "voting_producers" || section == "~voting_producers")
      {
         vector< producer_api_obj > producers = get_producers_by_voting_power( "", 50 );
         for( const auto& p : producers )
         {
            _state.voting_producers[ p.owner ] = p;
         }
      }
      else if( section == "mining_producers" || section == "~mining_producers")
      {
         vector< producer_api_obj > producers = get_producers_by_mining_power( "", 50 );
         for( const auto& p : producers )
         {
            _state.mining_producers[ p.owner ] = p;
         }
      }
      else if( section == "boards" || section == "~boards")
      {
         vector< extended_board > boards = get_boards_by_subscribers( "", 50 );
         for( const auto& b : boards )
         {
            _state.boards[ b.name ] = b;
         }
      }
      else if( section == "payout" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         vector< discussion > trending_disc = get_post_discussions_by_payout( q );

         for( const auto& d : trending_disc )
         {
            string key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].payout.push_back( key );

            if( d.author.size() )
            {
               accounts.insert( d.author );
            } 
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "payout_comments" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         vector< discussion > trending_disc = get_comment_discussions_by_payout( q );

         for( const auto& d : trending_disc )
         {
            string key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].payout_comments.push_back( key );

            if( d.author.size() )
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "responses" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         vector< discussion > trending_disc = get_discussions_by_children( q );

         for( const auto& d : trending_disc )
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].responses.push_back( key );

            if( d.author.size() )
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "net_votes" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_votes( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].net_votes.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert (d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "view_count" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_views( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].view_count.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "share_count" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_shares( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].share_count.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "comment_count" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_children( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].comment_count.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "vote_power" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_vote_power( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].vote_power.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "view_power" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_view_power( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].view_power.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "share_power" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_share_power( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].share_power.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "comment_power" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_comment_power( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].comment_power.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "active" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_active( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].active.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "created" )
      {
         discussion_query q;
         q.board = board;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_created( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].created.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "tags" )
      {
         _state.tag_idx.trending.clear();
         auto trending_tags = get_top_tags( std::string(), 250 );
         for( const auto& t : trending_tags )
         {
            string name = t.tag;
            _state.tag_idx.trending.push_back( name );
            _state.tags[ name ] = tag_following_api_obj( _db.get_tag_following( name ) );
         }
      }
      else if( acnt.size() && section.size() )
      {
         string permlink = section;
         string key = acnt + "/" + permlink;
         discussion dis = get_content( acnt, permlink );
         recursively_fetch_content( _state, dis, accounts );
         _state.content[ key ] = std::move( dis );
      }

      for( const auto& a : accounts )
      {
         _state.accounts.erase("");
         _state.accounts[ a ] = extended_account( _db.get_account( a ), _db );
      }
      for( auto& d : _state.content ) 
      {
         d.second.active_votes = get_active_votes( d.second.author, d.second.permlink );
         d.second.active_views = get_active_views( d.second.author, d.second.permlink );
         d.second.active_shares = get_active_shares( d.second.author, d.second.permlink );
         d.second.active_mod_tags = get_active_mod_tags( d.second.author, d.second.permlink );
      }

      _state.producer_schedule = _db.get_producer_schedule();
   }
   catch ( const fc::exception& e ) 
   {
      _state.error = e.to_detail_string();
   }
   return _state;
}


   //=======================//
   // === Subscriptions === //
   //=======================//


void database_api::set_block_applied_callback( std::function<void( const variant& block_id)> cb )
{
   my->_db.with_read_lock( [&]()
   {
      my->set_block_applied_callback( cb );
   });
}

void database_api_impl::on_applied_block( const chain::signed_block& b )
{
   try
   {
      _block_applied_callback( fc::variant(signed_block_header(b) ) );
   }
   catch( ... )
   {
      _block_applied_connection.release();
   }
}

void database_api_impl::set_block_applied_callback( std::function<void( const variant& block_header)> cb )
{
   _block_applied_callback = cb;
   _block_applied_connection = connect_signal( _db.applied_block, *this, &database_api_impl::on_applied_block );
}

} } // node::app