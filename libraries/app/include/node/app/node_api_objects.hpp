#pragma once
#include <node/chain/account_object.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/comment_object.hpp>
#include <node/chain/global_property_object.hpp>
#include <node/chain/history_object.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/transaction_object.hpp>
#include <node/chain/witness_objects.hpp>

#include <node/tags/tags_plugin.hpp>

#include <node/witness/witness_objects.hpp>

namespace node { namespace app {

using namespace node::chain;

/*struct limit_order
{
   limit_order( chain::limit_order_object& o ):
      id( o.id ),
      created( o.created ),
      expiration( o.expiration ),
      seller( o.seller ),
      orderid( o.orderid ),
      for_sale( o.for_sale ),
      sell_price( o.sell_price )
   {}

   limit_order(){}

   chain::limit_order_id_type id;
   time_point_sec             created;
   time_point_sec             expiration;
   account_name_type          seller;
   uint32_t                   orderid = 0;
   share_type                 for_sale;
   price                      sell_price;
};*/

typedef chain::change_recoveryAccount_request_object  change_recoveryAccount_request_api_obj;
typedef chain::block_summary_object                    block_summary_api_obj;
typedef chain::comment_vote_object                     comment_vote_api_obj;
typedef chain::convert_request_object                  convert_request_api_obj;
typedef chain::escrow_object                           escrow_api_obj;
typedef chain::liquidity_reward_balance_object         liquidity_reward_balance_api_obj;
typedef chain::limit_order_object                      limit_order_api_obj;
typedef chain::withdrawESCOR_route_object           withdrawESCOR_route_api_obj;
typedef chain::decline_voting_rights_request_object    decline_voting_rights_request_api_obj;
typedef chain::witness_vote_object                     witness_vote_api_obj;
typedef chain::witness_schedule_object                 witness_schedule_api_obj;
typedef chain::ECO_fund_for_ESCOR_delegation_object               ECO_fund_for_ESCOR_delegation_api_obj;
typedef chain::ECO_fund_for_ESCOR_delegation_expiration_object    ECO_fund_for_ESCOR_delegation_expiration_api_obj;
typedef chain::reward_fund_object                      reward_fund_api_obj;
typedef witness::account_bandwidth_object       account_bandwidth_api_obj;

struct comment_api_obj
{
   comment_api_obj( const chain::comment_object& o ):
      id( o.id ),
      category( to_string( o.category ) ),
      parent_author( o.parent_author ),
      parent_permlink( to_string( o.parent_permlink ) ),
      author( o.author ),
      permlink( to_string( o.permlink ) ),
      title( to_string( o.title ) ),
      body( to_string( o.body ) ),
      json( to_string( o.json ) ),
      last_update( o.last_update ),
      created( o.created ),
      active( o.active ),
      last_payout( o.last_payout ),
      depth( o.depth ),
      children( o.children ),
      net_ESCORreward( o.net_ESCORreward ),
      abs_ESCORreward( o.abs_ESCORreward ),
      vote_ESCORreward( o.vote_ESCORreward ),
      children_abs_ESCORreward( o.children_abs_ESCORreward ),
      cashout_time( o.cashout_time ),
      max_cashout_time( o.max_cashout_time ),
      total_vote_weight( o.total_vote_weight ),
      reward_weight( o.reward_weight ),
      total_payout_value( o.total_payout_value ),
      curator_payout_value( o.curator_payout_value ),
      authorRewards( o.authorRewards ),
      net_votes( o.net_votes ),
      root_comment( o.root_comment ),
      max_accepted_payout( o.max_accepted_payout ),
      percent_EUSD( o.percent_EUSD ),
      allow_replies( o.allow_replies ),
      allow_votes( o.allow_votes ),
      allow_curationRewards( o.allow_curationRewards )
   {
      for( auto& route : o.beneficiaries )
      {
         beneficiaries.push_back( route );
      }
   }

   comment_api_obj(){}

   comment_id_type   id;
   string            category;
   account_name_type parent_author;
   string            parent_permlink;
   account_name_type author;
   string            permlink;

   string            title;
   string            body;
   string            json;
   time_point_sec    last_update;
   time_point_sec    created;
   time_point_sec    active;
   time_point_sec    last_payout;

   uint8_t           depth = 0;
   uint32_t          children = 0;

   share_type        net_ESCORreward;
   share_type        abs_ESCORreward;
   share_type        vote_ESCORreward;

   share_type        children_abs_ESCORreward;
   time_point_sec    cashout_time;
   time_point_sec    max_cashout_time;
   uint64_t          total_vote_weight = 0;

   uint16_t          reward_weight = 0;

   asset             total_payout_value;
   asset             curator_payout_value;

   share_type        authorRewards;

   int32_t           net_votes = 0;

   comment_id_type   root_comment;

   asset             max_accepted_payout;
   uint16_t          percent_EUSD = 0;
   bool              allow_replies = false;
   bool              allow_votes = false;
   bool              allow_curationRewards = false;
   vector< beneficiary_route_type > beneficiaries;
};

struct tag_api_obj
{
   tag_api_obj( const tags::tag_stats_object& o ) :
      name( o.tag ),
      total_payouts(o.total_payout),
      net_votes(o.net_votes),
      top_posts(o.top_posts),
      comments(o.comments),
      trending(o.total_trending) {}

   tag_api_obj() {}

   string               name;
   asset                total_payouts;
   int32_t              net_votes = 0;
   uint32_t             top_posts = 0;
   uint32_t             comments = 0;
   fc::uint128          trending = 0;
};

struct account_api_obj
{
   account_api_obj( const chain::account_object& a, const chain::database& db ) :
      id( a.id ),
      name( a.name ),
      memoKey( a.memoKey ),
      json( to_string( a.json ) ),
      proxy( a.proxy ),
      last_accountUpdate( a.last_accountUpdate ),
      created( a.created ),
      mined( a.mined ),
      owner_challenged( a.owner_challenged ),
      active_challenged( a.active_challenged ),
      last_owner_proved( a.last_owner_proved ),
      last_active_proved( a.last_active_proved ),
      recoveryAccount( a.recoveryAccount ),
      reset_account( a.reset_account ),
      last_account_recovery( a.last_account_recovery ),
      comment_count( a.comment_count ),
      lifetime_vote_count( a.lifetime_vote_count ),
      post_count( a.post_count ),
      can_vote( a.can_vote ),
      voting_power( a.voting_power ),
      last_vote_time( a.last_vote_time ),
      balance( a.balance ),
      ECOsavingsBalance( a.ECOsavingsBalance ),
      EUSDbalance( a.EUSDbalance ),
      EUSD_seconds( a.EUSD_seconds ),
      EUSD_seconds_last_update( a.EUSD_seconds_last_update ),
      EUSD_last_interest_payment( a.EUSD_last_interest_payment ),
      EUSDsavingsBalance( a.EUSDsavingsBalance ),
      savings_EUSD_seconds( a.savings_EUSD_seconds ),
      savings_EUSD_seconds_last_update( a.savings_EUSD_seconds_last_update ),
      savings_EUSD_last_interest_payment( a.savings_EUSD_last_interest_payment ),
      savings_withdraw_requests( a.savings_withdraw_requests ),
      EUSDrewardbalance( a.EUSDrewardbalance ),
      ECOrewardBalance( a.ECOrewardBalance ),
      ESCORrewardBalance( a.ESCORrewardBalance ),
      ESCORrewardBalanceInECO( a.ESCORrewardBalanceInECO ),
      curationRewards( a.curationRewards ),
      posting_rewards( a.posting_rewards ),
      ESCOR( a.ESCOR ),
      ESCORDelegated( a.ESCORDelegated ),
      ESCORreceived( a.ESCORreceived ),
      ESCORwithdrawRateInECO( a.ESCORwithdrawRateInECO ),
      nextESCORwithdrawalTime( a.nextESCORwithdrawalTime ),
      withdrawn( a.withdrawn ),
      to_withdraw( a.to_withdraw ),
      withdraw_routes( a.withdraw_routes ),
      witnesses_voted_for( a.witnesses_voted_for ),
      last_post( a.last_post ),
      last_root_post( a.last_root_post )
   {
      size_t n = a.proxied_ESCORfundECObalance_votes.size();
      proxied_ESCORfundECObalance_votes.reserve( n );
      for( size_t i=0; i<n; i++ )
         proxied_ESCORfundECObalance_votes.push_back( a.proxied_ESCORfundECObalance_votes[i] );

      const auto& auth = db.get< account_authority_object, by_account >( name );
      owner = authority( auth.owner );
      active = authority( auth.active );
      posting = authority( auth.posting );
      last_owner_update = auth.last_owner_update;

      if( db.has_index< witness::account_bandwidth_index >() )
      {
         auto forum_bandwidth = db.find< witness::account_bandwidth_object, witness::by_account_bandwidth_type >( boost::make_tuple( name, witness::bandwidth_type::forum ) );

         if( forum_bandwidth != nullptr )
         {
            average_bandwidth = forum_bandwidth->average_bandwidth;
            lifetime_bandwidth = forum_bandwidth->lifetime_bandwidth;
            last_bandwidth_update = forum_bandwidth->last_bandwidth_update;
         }

         auto market_bandwidth = db.find< witness::account_bandwidth_object, witness::by_account_bandwidth_type >( boost::make_tuple( name, witness::bandwidth_type::market ) );

         if( market_bandwidth != nullptr )
         {
            average_market_bandwidth = market_bandwidth->average_bandwidth;
            lifetime_market_bandwidth = market_bandwidth->lifetime_bandwidth;
            last_market_bandwidth_update = market_bandwidth->last_bandwidth_update;
         }
      }
   }


   account_api_obj(){}

   account_id_type   id;

   account_name_type name;
   authority         owner;
   authority         active;
   authority         posting;
   public_key_type   memoKey;
   string            json;
   account_name_type proxy;

   time_point_sec    last_owner_update;
   time_point_sec    last_accountUpdate;

   time_point_sec    created;
   bool              mined = false;
   bool              owner_challenged = false;
   bool              active_challenged = false;
   time_point_sec    last_owner_proved;
   time_point_sec    last_active_proved;
   account_name_type recoveryAccount;
   account_name_type reset_account;
   time_point_sec    last_account_recovery;
   uint32_t          comment_count = 0;
   uint32_t          lifetime_vote_count = 0;
   uint32_t          post_count = 0;

   bool              can_vote = false;
   uint16_t          voting_power = 0;
   time_point_sec    last_vote_time;

   asset             balance;
   asset             ECOsavingsBalance;

   asset             EUSDbalance;
   uint128_t         EUSD_seconds;
   time_point_sec    EUSD_seconds_last_update;
   time_point_sec    EUSD_last_interest_payment;

   asset             EUSDsavingsBalance;
   uint128_t         savings_EUSD_seconds;
   time_point_sec    savings_EUSD_seconds_last_update;
   time_point_sec    savings_EUSD_last_interest_payment;

   uint8_t           savings_withdraw_requests = 0;

   asset             EUSDrewardbalance;
   asset             ECOrewardBalance;
   asset             ESCORrewardBalance;
   asset             ESCORrewardBalanceInECO;

   share_type        curationRewards;
   share_type        posting_rewards;

   asset             ESCOR;
   asset             ESCORDelegated;
   asset             ESCORreceived;
   asset             ESCORwithdrawRateInECO;
   time_point_sec    nextESCORwithdrawalTime;
   share_type        withdrawn;
   share_type        to_withdraw;
   uint16_t          withdraw_routes = 0;

   vector< share_type > proxied_ESCORfundECObalance_votes;

   uint16_t          witnesses_voted_for;

   share_type        average_bandwidth = 0;
   share_type        lifetime_bandwidth = 0;
   time_point_sec    last_bandwidth_update;

   share_type        average_market_bandwidth = 0;
   share_type        lifetime_market_bandwidth = 0;
   time_point_sec    last_market_bandwidth_update;

   time_point_sec    last_post;
   time_point_sec    last_root_post;
};

struct owner_authority_history_api_obj
{
   owner_authority_history_api_obj( const chain::owner_authority_history_object& o ) :
      id( o.id ),
      account( o.account ),
      previous_owner_authority( authority( o.previous_owner_authority ) ),
      last_valid_time( o.last_valid_time )
   {}

   owner_authority_history_api_obj() {}

   owner_authority_history_id_type  id;

   account_name_type                account;
   authority                        previous_owner_authority;
   time_point_sec                   last_valid_time;
};

struct account_recovery_request_api_obj
{
   account_recovery_request_api_obj( const chain::account_recovery_request_object& o ) :
      id( o.id ),
      accountToRecover( o.accountToRecover ),
      new_owner_authority( authority( o.new_owner_authority ) ),
      expires( o.expires )
   {}

   account_recovery_request_api_obj() {}

   account_recovery_request_id_type id;
   account_name_type                accountToRecover;
   authority                        new_owner_authority;
   time_point_sec                   expires;
};

struct account_history_api_obj
{

};

struct savings_withdraw_api_obj
{
   savings_withdraw_api_obj( const chain::savings_withdraw_object& o ) :
      id( o.id ),
      from( o.from ),
      to( o.to ),
      memo( to_string( o.memo ) ),
      request_id( o.request_id ),
      amount( o.amount ),
      complete( o.complete )
   {}

   savings_withdraw_api_obj() {}

   savings_withdraw_id_type   id;
   account_name_type          from;
   account_name_type          to;
   string                     memo;
   uint32_t                   request_id = 0;
   asset                      amount;
   time_point_sec             complete;
};

struct feed_history_api_obj
{
   feed_history_api_obj( const chain::feed_history_object& f ) :
      id( f.id ),
      current_median_history( f.current_median_history ),
      price_history( f.price_history.begin(), f.price_history.end() )
   {}

   feed_history_api_obj() {}

   feed_history_id_type id;
   price                current_median_history;
   deque< price >       price_history;
};

struct witness_api_obj
{
   witness_api_obj( const chain::witness_object& w ) :
      id( w.id ),
      owner( w.owner ),
      created( w.created ),
      url( to_string( w.url ) ),
      total_missed( w.total_missed ),
      last_aslot( w.last_aslot ),
      last_confirmed_block_num( w.last_confirmed_block_num ),
      pow_worker( w.pow_worker ),
      signing_key( w.signing_key ),
      props( w.props ),
      EUSD_exchange_rate( w.EUSD_exchange_rate ),
      last_EUSD_exchange_update( w.last_EUSD_exchange_update ),
      votes( w.votes ),
      virtual_last_update( w.virtual_last_update ),
      virtual_position( w.virtual_position ),
      virtual_scheduled_time( w.virtual_scheduled_time ),
      last_work( w.last_work ),
      running_version( w.running_version ),
      hardfork_version_vote( w.hardfork_version_vote ),
      hardfork_time_vote( w.hardfork_time_vote )
   {}

   witness_api_obj() {}

   witness_id_type   id;
   account_name_type owner;
   time_point_sec    created;
   string            url;
   uint32_t          total_missed = 0;
   uint64_t          last_aslot = 0;
   uint64_t          last_confirmed_block_num = 0;
   uint64_t          pow_worker = 0;
   public_key_type   signing_key;
   chain_properties  props;
   price             EUSD_exchange_rate;
   time_point_sec    last_EUSD_exchange_update;
   share_type        votes;
   fc::uint128       virtual_last_update;
   fc::uint128       virtual_position;
   fc::uint128       virtual_scheduled_time;
   digest_type       last_work;
   version           running_version;
   hardfork_version  hardfork_version_vote;
   time_point_sec    hardfork_time_vote;
};

struct signed_block_api_obj : public signed_block
{
   signed_block_api_obj( const signed_block& block ) : signed_block( block )
   {
      block_id = id();
      signing_key = signee();
      transaction_ids.reserve( transactions.size() );
      for( const signed_transaction& tx : transactions )
         transaction_ids.push_back( tx.id() );
   }
   signed_block_api_obj() {}

   block_id_type                 block_id;
   public_key_type               signing_key;
   vector< transaction_id_type > transaction_ids;
};

struct dynamic_global_property_api_obj : public dynamic_global_property_object
{
   dynamic_global_property_api_obj( const dynamic_global_property_object& gpo, const chain::database& db ) :
      dynamic_global_property_object( gpo )
   {
      if( db.has_index< witness::reserve_ratio_index >() )
      {
         const auto& r = db.find( witness::reserve_ratio_id_type() );

         if( BOOST_LIKELY( r != nullptr ) )
         {
            current_reserve_ratio = r->current_reserve_ratio;
            average_block_size = r->average_block_size;
            max_virtual_bandwidth = r->max_virtual_bandwidth;
         }
      }
   }

   dynamic_global_property_api_obj( const dynamic_global_property_object& gpo ) :
      dynamic_global_property_object( gpo ) {}

   dynamic_global_property_api_obj() {}

   uint32_t    current_reserve_ratio = 0;
   uint64_t    average_block_size = 0;
   uint128_t   max_virtual_bandwidth = 0;
};

} } // node::app

FC_REFLECT( node::app::comment_api_obj,
             (id)(author)(permlink)
             (category)(parent_author)(parent_permlink)
             (title)(body)(json)(last_update)(created)(active)(last_payout)
             (depth)(children)
             (net_ESCORreward)(abs_ESCORreward)(vote_ESCORreward)
             (children_abs_ESCORreward)(cashout_time)(max_cashout_time)
             (total_vote_weight)(reward_weight)(total_payout_value)(curator_payout_value)(authorRewards)(net_votes)(root_comment)
             (max_accepted_payout)(percent_EUSD)(allow_replies)(allow_votes)(allow_curationRewards)
             (beneficiaries)
          )

FC_REFLECT( node::app::account_api_obj,
             (id)(name)(owner)(active)(posting)(memoKey)(json)(proxy)(last_owner_update)(last_accountUpdate)
             (created)(mined)
             (owner_challenged)(active_challenged)(last_owner_proved)(last_active_proved)(recoveryAccount)(last_account_recovery)(reset_account)
             (comment_count)(lifetime_vote_count)(post_count)(can_vote)(voting_power)(last_vote_time)
             (balance)
             (ECOsavingsBalance)
             (EUSDbalance)(EUSD_seconds)(EUSD_seconds_last_update)(EUSD_last_interest_payment)
             (EUSDsavingsBalance)(savings_EUSD_seconds)(savings_EUSD_seconds_last_update)(savings_EUSD_last_interest_payment)(savings_withdraw_requests)
             (EUSDrewardbalance)(ECOrewardBalance)(ESCORrewardBalance)(ESCORrewardBalanceInECO)
             (ESCOR)(ESCORDelegated)(ESCORreceived)(ESCORwithdrawRateInECO)(nextESCORwithdrawalTime)(withdrawn)(to_withdraw)(withdraw_routes)
             (curationRewards)
             (posting_rewards)
             (proxied_ESCORfundECObalance_votes)(witnesses_voted_for)
             (average_bandwidth)(lifetime_bandwidth)(last_bandwidth_update)
             (average_market_bandwidth)(lifetime_market_bandwidth)(last_market_bandwidth_update)
             (last_post)(last_root_post)
          )

FC_REFLECT( node::app::owner_authority_history_api_obj,
             (id)
             (account)
             (previous_owner_authority)
             (last_valid_time)
          )

FC_REFLECT( node::app::account_recovery_request_api_obj,
             (id)
             (accountToRecover)
             (new_owner_authority)
             (expires)
          )

FC_REFLECT( node::app::savings_withdraw_api_obj,
             (id)
             (from)
             (to)
             (memo)
             (request_id)
             (amount)
             (complete)
          )

FC_REFLECT( node::app::feed_history_api_obj,
             (id)
             (current_median_history)
             (price_history)
          )

FC_REFLECT( node::app::tag_api_obj,
            (name)
            (total_payouts)
            (net_votes)
            (top_posts)
            (comments)
            (trending)
          )

FC_REFLECT( node::app::witness_api_obj,
             (id)
             (owner)
             (created)
             (url)(votes)(virtual_last_update)(virtual_position)(virtual_scheduled_time)(total_missed)
             (last_aslot)(last_confirmed_block_num)(pow_worker)(signing_key)
             (props)
             (EUSD_exchange_rate)(last_EUSD_exchange_update)
             (last_work)
             (running_version)
             (hardfork_version_vote)(hardfork_time_vote)
          )

FC_REFLECT_DERIVED( node::app::signed_block_api_obj, (node::protocol::signed_block),
                     (block_id)
                     (signing_key)
                     (transaction_ids)
                  )

FC_REFLECT_DERIVED( node::app::dynamic_global_property_api_obj, (node::chain::dynamic_global_property_object),
                     (current_reserve_ratio)
                     (average_block_size)
                     (max_virtual_bandwidth)
                  )
