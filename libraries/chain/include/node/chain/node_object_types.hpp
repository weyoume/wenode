#pragma once
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

//#include <graphene/db2/database.hpp>
#include <chainbase/chainbase.hpp>

#include <node/protocol/types.hpp>
#include <node/protocol/authority.hpp>


namespace node { namespace chain {

namespace bip = chainbase::bip;
using namespace boost::multi_index;

using boost::multi_index_container;

using chainbase::object;
using chainbase::oid;
using chainbase::allocator;
using chainbase::strcmp_less;
using chainbase::strcmp_equal;
using chainbase::shared_vector;

using node::protocol::chain_id_type;
using node::protocol::digest_type;
using node::protocol::public_key_type;
using node::protocol::share_type;
using node::protocol::ratio_type;
using node::protocol::block_id_type;
using node::protocol::transaction_id_type;
using node::protocol::signature_type;

using node::protocol::account_name_type;
using node::protocol::community_name_type;
using node::protocol::tag_name_type;
using node::protocol::asset_symbol_type;
using node::protocol::graph_node_name_type;
using node::protocol::graph_edge_name_type;
using node::protocol::fixed_string_32;
using node::protocol::encrypted_keypair_type;
using node::protocol::date_type;

using node::protocol::community_privacy_type;
using node::protocol::community_federation_type;
using node::protocol::business_structure_type;
using node::protocol::membership_tier_type;
using node::protocol::network_officer_role_type;
using node::protocol::executive_role_type;
using node::protocol::product_auction_type;
using node::protocol::asset_property_type;
using node::protocol::ad_format_type;
using node::protocol::post_format_type;
using node::protocol::ad_metric_type;
using node::protocol::connection_tier_type;
using node::protocol::feed_reach_type;
using node::protocol::blog_reach_type;
using node::protocol::sort_time_type;
using node::protocol::sort_option_type;
using node::protocol::post_time_type;
using node::protocol::asset_issuer_permission_flags;
using node::protocol::community_permission_flags;


typedef bip::basic_string< char, std::char_traits< char >, allocator< char > > shared_string;
typedef bip::allocator< shared_string, bip::managed_mapped_file::segment_manager > basic_string_allocator;
inline std::string to_string( const shared_string& str ) { return std::string( str.begin(), str.end() ); }
inline void from_string( shared_string& out, const string& in ){ out.assign( in.begin(), in.end() ); }

typedef bip::vector< char, allocator< char > > buffer_type;

struct by_id;

enum object_type
{
   // Global Objects

   dynamic_global_property_object_type,
   median_chain_property_object_type,
   transaction_object_type,
   operation_object_type,
   account_history_object_type,
   block_summary_object_type,
   hardfork_property_object_type,
   
   // Account Objects

   account_object_type,
   account_authority_object_type,
   account_permission_object_type,
   account_verification_object_type,
   account_business_object_type,
   account_executive_vote_object_type,
   account_officer_vote_object_type,
   account_member_request_object_type,
   account_member_invite_object_type,
   account_member_key_object_type,
   account_following_object_type,
   account_tag_following_object_type,
   account_connection_object_type,
   account_authority_history_object_type,
   account_recovery_request_object_type,
   account_recovery_update_request_object_type,
   account_decline_voting_request_object_type,

   // Network Objects
   
   network_officer_object_type,
   network_officer_vote_object_type,
   executive_board_object_type,
   executive_board_vote_object_type,
   governance_account_object_type,
   governance_subscription_object_type,
   supernode_object_type,
   interface_object_type,
   mediator_object_type,
   enterprise_object_type,
   enterprise_vote_object_type,
   enterprise_fund_object_type,

   // Comment Objects

   comment_object_type,
   comment_blog_object_type,
   comment_feed_object_type,
   comment_vote_object_type,
   comment_view_object_type,
   comment_share_object_type,
   comment_moderation_object_type,
   comment_metrics_object_type,
   message_object_type,
   list_object_type,
   poll_object_type,
   poll_vote_object_type,
   premium_purchase_object_type,
   premium_purchase_key_object_type,

   // Community Objects

   community_object_type,
   community_member_object_type,
   community_member_key_object_type,
   community_moderator_vote_object_type,
   community_join_request_object_type,
   community_join_invite_object_type,
   community_federation_object_type,
   community_event_object_type,
   community_event_attend_object_type,

   // Advertising Objects

   ad_creative_object_type,
   ad_campaign_object_type,
   ad_inventory_object_type,
   ad_audience_object_type,
   ad_bid_object_type,

   // Graph Data Objects

   graph_node_object_type,
   graph_edge_object_type,
   graph_node_property_object_type,
   graph_edge_property_object_type,

   // Transfer Objects

   transfer_request_object_type,
   transfer_recurring_object_type,
   transfer_recurring_request_object_type,

   // Balance Objects

   account_balance_object_type,
   account_vesting_balance_object_type,
   confidential_balance_object_type,
   unstake_asset_route_object_type,
   savings_withdraw_object_type,
   asset_delegation_object_type,
   asset_delegation_expiration_object_type,
   
   // Marketplace Objects

   product_sale_object_type,
   product_purchase_object_type,
   product_auction_sale_object_type,
   product_auction_bid_object_type,
   escrow_object_type,

   // Trading Objects
   
   limit_order_object_type,
   margin_order_object_type,
   auction_order_object_type,
   call_order_object_type,
   option_order_object_type,

   // Pool Objects

   asset_liquidity_pool_object_type,
   asset_credit_pool_object_type,
   credit_collateral_object_type,
   credit_loan_object_type,
   asset_option_pool_object_type,
   asset_prediction_pool_object_type,
   asset_prediction_pool_resolution_object_type,

   // Asset Objects

   asset_object_type,
   asset_dynamic_data_object_type,
   asset_currency_data_object_type,
   asset_reward_fund_object_type,
   asset_stablecoin_data_object_type,
   asset_settlement_object_type,
   asset_collateral_bid_object_type,
   asset_equity_data_object_type,
   asset_bond_data_object_type,
   asset_credit_data_object_type,
   asset_stimulus_data_object_type,
   asset_unique_data_object_type,
   asset_distribution_object_type,
   asset_distribution_balance_object_type,

   // Block Producer objects

   producer_object_type,
   producer_schedule_object_type,
   producer_vote_object_type,
   block_validation_object_type,
   commit_violation_object_type
};

//========================//
// === Object Classes === //
//========================//

// Global objects

class dynamic_global_property_object;
class median_chain_property_object;
class transaction_object;
class operation_object;
class account_history_object;
class block_summary_object;
class hardfork_property_object;

// Account Objects

class account_object;
class account_authority_object;
class account_permission_object;
class account_verification_object;
class account_business_object;
class account_executive_vote_object;
class account_officer_vote_object;
class account_member_request_object;
class account_member_invite_object;
class account_member_key_object;
class account_following_object;
class account_tag_following_object;
class account_connection_object;
class account_authority_history_object;
class account_recovery_request_object;
class account_recovery_update_request_object;
class account_decline_voting_request_object;

// Network objects

class network_officer_object;
class network_officer_vote_object;
class executive_board_object;
class executive_board_vote_object;
class governance_account_object;
class governance_subscription_object;
class supernode_object;
class interface_object;
class mediator_object;
class enterprise_object;
class enterprise_vote_object;
class enterprise_fund_object;

// Comment Objects

class comment_object;
class comment_blog_object;
class comment_feed_object;
class comment_vote_object;
class comment_view_object;
class comment_share_object;
class comment_moderation_object;
class comment_metrics_object;
class message_object;
class list_object;
class poll_object;
class poll_vote_object;
class premium_purchase_object;
class premium_purchase_key_object;

// Community Objects

class community_object;
class community_member_object;
class community_member_key_object;
class community_moderator_vote_object;
class community_join_request_object;
class community_join_invite_object;
class community_federation_object;
class community_event_object;
class community_event_attend_object;

// Advertising Objects

class ad_creative_object;
class ad_campaign_object;
class ad_inventory_object;
class ad_audience_object;
class ad_bid_object;

// Graph Data Objects

class graph_node_object;
class graph_edge_object;
class graph_node_property_object;
class graph_edge_property_object;

// Transfer Objects

class transfer_request_object;
class transfer_recurring_object;
class transfer_recurring_request_object;

// Balance Objects

class account_balance_object;
class account_vesting_balance_object;
class confidential_balance_object;
class unstake_asset_route_object;
class savings_withdraw_object;
class asset_delegation_object;
class asset_delegation_expiration_object;

// Marketplace Objects

class product_sale_object;
class product_purchase_object;
class product_auction_sale_object;
class product_auction_bid_object;
class escrow_object;

// Trading Objects

class limit_order_object;
class margin_order_object;
class auction_order_object;
class call_order_object;
class option_order_object;

// Pool Objects

class asset_liquidity_pool_object;
class asset_credit_pool_object;
class credit_collateral_object;
class credit_loan_object;
class asset_option_pool_object;
class asset_prediction_pool_object;
class asset_prediction_pool_resolution_object;

// Asset objects

class asset_object;
class asset_dynamic_data_object;
class asset_currency_data_object;
class asset_reward_fund_object;
class asset_stablecoin_data_object;
class asset_settlement_object;
class asset_collateral_bid_object;
class asset_equity_data_object;
class asset_bond_data_object;
class asset_credit_data_object;
class asset_stimulus_data_object;
class asset_unique_data_object;
class asset_distribution_object;
class asset_distribution_balance_object;

// Block producer objects

class producer_object;
class producer_schedule_object;
class producer_vote_object;
class block_validation_object;
class commit_violation_object;

//=========================//
// === Object ID Types === //
//=========================//

// Global Objects

typedef oid< dynamic_global_property_object             > dynamic_global_property_id_type;
typedef oid< median_chain_property_object               > median_chain_property_id_type;
typedef oid< transaction_object                         > transaction_object_id_type;        // Includes object to avoid collision with transaction_id_type
typedef oid< operation_object                           > operation_id_type;
typedef oid< account_history_object                     > account_history_id_type;
typedef oid< block_summary_object                       > block_summary_id_type;
typedef oid< hardfork_property_object                   > hardfork_property_id_type;

// Account Objects

typedef oid< account_object                             > account_id_type;
typedef oid< account_authority_object                   > account_authority_id_type;
typedef oid< account_permission_object                  > account_permission_id_type;
typedef oid< account_verification_object                > account_verification_id_type;
typedef oid< account_business_object                    > account_business_id_type;
typedef oid< account_executive_vote_object              > account_executive_vote_id_type;
typedef oid< account_officer_vote_object                > account_officer_vote_id_type;
typedef oid< account_member_request_object              > account_member_request_id_type;
typedef oid< account_member_invite_object               > account_member_invite_id_type;
typedef oid< account_member_key_object                  > account_member_key_id_type;
typedef oid< account_following_object                   > account_following_id_type;
typedef oid< account_tag_following_object               > account_tag_following_id_type;
typedef oid< account_connection_object                  > account_connection_id_type;
typedef oid< account_authority_history_object           > account_authority_history_id_type;
typedef oid< account_recovery_request_object            > account_recovery_request_id_type;
typedef oid< account_recovery_update_request_object     > account_recovery_update_request_id_type;
typedef oid< account_decline_voting_request_object      > account_decline_voting_request_id_type;

// Network objects

typedef oid< network_officer_object                     > network_officer_id_type;
typedef oid< network_officer_vote_object                > network_officer_vote_id_type;
typedef oid< executive_board_object                     > executive_board_id_type;
typedef oid< executive_board_vote_object                > executive_board_vote_id_type;
typedef oid< governance_account_object                  > governance_account_id_type;
typedef oid< governance_subscription_object             > governance_subscription_id_type;
typedef oid< supernode_object                           > supernode_id_type;
typedef oid< interface_object                           > interface_id_type;
typedef oid< mediator_object                            > mediator_id_type;
typedef oid< enterprise_object                          > enterprise_id_type;
typedef oid< enterprise_vote_object                     > enterprise_vote_id_type;
typedef oid< enterprise_fund_object                     > enterprise_fund_id_type;

// Comment Objects

typedef oid< comment_object                             > comment_id_type;
typedef oid< comment_blog_object                        > comment_blog_id_type;
typedef oid< comment_feed_object                        > comment_feed_id_type;
typedef oid< comment_vote_object                        > comment_vote_id_type;
typedef oid< comment_view_object                        > comment_view_id_type;
typedef oid< comment_share_object                       > comment_share_id_type;
typedef oid< comment_moderation_object                  > comment_moderation_id_type;
typedef oid< comment_metrics_object                     > comment_metrics_id_type;
typedef oid< message_object                             > message_id_type;
typedef oid< list_object                                > list_id_type;
typedef oid< poll_object                                > poll_id_type;
typedef oid< poll_vote_object                           > poll_vote_id_type;
typedef oid< premium_purchase_object                    > premium_purchase_id_type;
typedef oid< premium_purchase_key_object                > premium_purchase_key_id_type;

// Community Objects

typedef oid< community_object                           > community_id_type;
typedef oid< community_member_object                    > community_member_id_type;
typedef oid< community_member_key_object                > community_member_key_id_type;
typedef oid< community_moderator_vote_object            > community_moderator_vote_id_type;
typedef oid< community_join_request_object              > community_join_request_id_type;
typedef oid< community_join_invite_object               > community_join_invite_id_type;
typedef oid< community_federation_object                > community_federation_id_type;
typedef oid< community_event_object                     > community_event_id_type;
typedef oid< community_event_attend_object              > community_event_attend_id_type;

// Advertising Objects

typedef oid< ad_creative_object                         > ad_creative_id_type;
typedef oid< ad_campaign_object                         > ad_campaign_id_type;
typedef oid< ad_inventory_object                        > ad_inventory_id_type;
typedef oid< ad_audience_object                         > ad_audience_id_type;
typedef oid< ad_bid_object                              > ad_bid_id_type;

// Graph Data Objects

typedef oid< graph_node_object                          > graph_node_id_type;
typedef oid< graph_edge_object                          > graph_edge_id_type;
typedef oid< graph_node_property_object                 > graph_node_property_id_type;
typedef oid< graph_edge_property_object                 > graph_edge_property_id_type;

// Transfer Objects

typedef oid< transfer_request_object                    > transfer_request_id_type;
typedef oid< transfer_recurring_object                  > transfer_recurring_id_type;
typedef oid< transfer_recurring_request_object          > transfer_recurring_request_id_type;

// Balance Objects

typedef oid< account_balance_object                     > account_balance_id_type;
typedef oid< account_vesting_balance_object             > account_vesting_balance_id_type;
typedef oid< unstake_asset_route_object                 > unstake_asset_route_id_type;
typedef oid< savings_withdraw_object                    > savings_withdraw_id_type;
typedef oid< asset_delegation_object                    > asset_delegation_id_type;
typedef oid< asset_delegation_expiration_object         > asset_delegation_expiration_id_type;
typedef oid< confidential_balance_object                > confidential_balance_id_type;

// Marketplace Objects

typedef oid< product_sale_object                        > product_sale_id_type;
typedef oid< product_purchase_object                    > product_purchase_id_type;
typedef oid< product_auction_sale_object                > product_auction_sale_id_type;
typedef oid< product_auction_bid_object                 > product_auction_bid_id_type;
typedef oid< escrow_object                              > escrow_id_type;

// Trading Objects

typedef oid< limit_order_object                         > limit_order_id_type;
typedef oid< margin_order_object                        > margin_order_id_type;
typedef oid< auction_order_object                       > auction_order_id_type;
typedef oid< call_order_object                          > call_order_id_type;
typedef oid< option_order_object                        > option_order_id_type;

// Pool Objects

typedef oid< asset_liquidity_pool_object                > asset_liquidity_pool_id_type;
typedef oid< asset_credit_pool_object                   > asset_credit_pool_id_type;
typedef oid< credit_collateral_object                   > credit_collateral_id_type;
typedef oid< credit_loan_object                         > credit_loan_id_type;
typedef oid< asset_option_pool_object                   > asset_option_pool_id_type;
typedef oid< asset_prediction_pool_object               > asset_prediction_pool_id_type;
typedef oid< asset_prediction_pool_resolution_object    > asset_prediction_pool_resolution_id_type;

// Asset objects

typedef oid< asset_object                               > asset_id_type;
typedef oid< asset_dynamic_data_object                  > asset_dynamic_data_id_type;
typedef oid< asset_currency_data_object                 > asset_currency_data_id_type;
typedef oid< asset_reward_fund_object                   > asset_reward_fund_id_type;
typedef oid< asset_stablecoin_data_object               > asset_stablecoin_data_id_type;
typedef oid< asset_settlement_object                    > asset_settlement_id_type;
typedef oid< asset_collateral_bid_object                > asset_collateral_bid_id_type;
typedef oid< asset_equity_data_object                   > asset_equity_data_id_type;
typedef oid< asset_bond_data_object                     > asset_bond_data_id_type;
typedef oid< asset_credit_data_object                   > asset_credit_data_id_type;
typedef oid< asset_stimulus_data_object                 > asset_stimulus_data_id_type;
typedef oid< asset_unique_data_object                   > asset_unique_data_id_type;
typedef oid< asset_distribution_object                  > asset_distribution_id_type;
typedef oid< asset_distribution_balance_object          > asset_distribution_balance_id_type;

// Block producer objects

typedef oid< producer_object                            > producer_id_type;
typedef oid< producer_schedule_object                   > producer_schedule_id_type;
typedef oid< producer_vote_object                       > producer_vote_id_type;
typedef oid< block_validation_object                    > block_validation_id_type;
typedef oid< commit_violation_object                    > commit_violation_id_type;

} } //node::chain

namespace fc
{
   class variant;
   inline void to_variant( const node::chain::shared_string& s, variant& var )
   {
      var = fc::string( node::chain::to_string( s ) );
   }

   inline void from_variant( const variant& var, node::chain::shared_string& s )
   {
      auto str = var.as_string();
      s.assign( str.begin(), str.end() );
   }

   template<typename T>
   inline void to_variant( const chainbase::oid<T>& var, variant& vo )
   {
      vo = var._id;
   }

   template<typename T>
   inline void from_variant( const variant& vo, chainbase::oid<T>& var )
   {
      var._id = vo.as_int64();
   }

   template<typename T>
   inline void to_variant( const chainbase::shared_vector<T>& var, variant& vo )
   {
      chainbase::shared_vector< variant > vars;
      vars.resize( var.size() );
      for( size_t i = 0; i < var.size(); ++i )
      {
         vars[i] = variant( var[i] );
      }
      vo = std::move( vars );
   }

   template<typename T>
   inline void from_variant( const variant& vo, chainbase::shared_vector<T>& var )
   {
      const variants& vars = vo.get_array();
      var.clear();
      var.resize( vars.size() );
      for( auto itr = vars.begin(); itr != vars.end(); ++itr )
      {
         var.push_back( itr->as<T>() );
      }
   }

   namespace raw {
      template<typename Stream, typename T>
      inline void pack( Stream& s, const chainbase::oid<T>& id )
      {
         s.write( (const char*)&id._id, sizeof(id._id) );
      }
      template<typename Stream, typename T>
      inline void unpack( Stream& s, chainbase::oid<T>& id )
      {
         s.read( (char*)&id._id, sizeof(id._id));
      }
   }

   namespace raw
   {
      namespace bip = chainbase::bip;
      using chainbase::allocator;

      template< typename T > inline void pack( node::chain::buffer_type& raw, const T& v )
      {
         auto size = pack_size( v );
         raw.resize( size );
         datastream< char* > ds( raw.data(), size );
         pack( ds, v );
      }

      template< typename T > inline void unpack( const node::chain::buffer_type& raw, T& v )
      {
         datastream< const char* > ds( raw.data(), raw.size() );
         unpack( ds, v );
      }

      template< typename T > inline T unpack( const node::chain::buffer_type& raw )
      {
         T v;
         datastream< const char* > ds( raw.data(), raw.size() );
         unpack( ds, v );
         return v;
      }
   }
}

FC_REFLECT_ENUM( node::chain::object_type,

         // Node objects
         
         (dynamic_global_property_object_type)
         (median_chain_property_object_type)
         (transaction_object_type)
         (operation_object_type)
         (account_history_object_type)
         (block_summary_object_type)
         (hardfork_property_object_type)

         // Account Objects

         (account_object_type)
         (account_authority_object_type)
         (account_permission_object_type)
         (account_verification_object_type)
         (account_business_object_type)
         (account_executive_vote_object_type)
         (account_officer_vote_object_type)
         (account_member_request_object_type)
         (account_member_invite_object_type)
         (account_member_key_object_type)
         (account_following_object_type)
         (account_tag_following_object_type)
         (account_connection_object_type)
         (account_authority_history_object_type)
         (account_recovery_request_object_type)
         (account_recovery_update_request_object_type)
         (account_decline_voting_request_object_type)

         // Network objects

         (network_officer_object_type)
         (network_officer_vote_object_type)
         (executive_board_object_type)
         (executive_board_vote_object_type)
         (governance_account_object_type)
         (governance_subscription_object_type)
         (supernode_object_type)
         (interface_object_type)
         (mediator_object_type)
         (enterprise_object_type)
         (enterprise_vote_object_type)
         (enterprise_fund_object_type)

         // Comment Objects

         (comment_object_type)
         (comment_blog_object_type)
         (comment_feed_object_type)
         (comment_vote_object_type)
         (comment_view_object_type)
         (comment_share_object_type)
         (comment_moderation_object_type)
         (comment_metrics_object_type)
         (message_object_type)
         (list_object_type)
         (poll_object_type)
         (poll_vote_object_type)
         (premium_purchase_object_type)
         (premium_purchase_key_object_type)
         
         // Community Objects

         (community_object_type)
         (community_member_object_type)
         (community_member_key_object_type)
         (community_moderator_vote_object_type)
         (community_join_request_object_type)
         (community_join_invite_object_type)
         (community_federation_object_type)
         (community_event_object_type)
         (community_event_attend_object_type)

         // Advertising Objects

         (ad_creative_object_type)
         (ad_campaign_object_type)
         (ad_inventory_object_type)
         (ad_audience_object_type)
         (ad_bid_object_type)

         // Graph Data Objects

         (graph_node_object_type)
         (graph_edge_object_type)
         (graph_node_property_object_type)
         (graph_edge_property_object_type)

         // Transfer Objects

         (transfer_request_object_type)
         (transfer_recurring_object_type)
         (transfer_recurring_request_object_type)

         // Balance Objects

         (account_balance_object_type)
         (account_vesting_balance_object_type)
         (confidential_balance_object_type)
         (unstake_asset_route_object_type)
         (savings_withdraw_object_type)
         (asset_delegation_object_type)
         (asset_delegation_expiration_object_type)
         
         // Marketplace Objects

         (product_sale_object_type)
         (product_purchase_object_type)
         (product_auction_sale_object_type)
         (product_auction_bid_object_type)
         (escrow_object_type)

         // Trading Objects

         (limit_order_object_type)
         (margin_order_object_type)
         (auction_order_object_type)
         (call_order_object_type)
         (option_order_object_type)

         // Pool Objects

         (asset_liquidity_pool_object_type)
         (asset_credit_pool_object_type)
         (credit_collateral_object_type)
         (credit_loan_object_type)
         (asset_option_pool_object_type)
         (asset_prediction_pool_object_type)
         (asset_prediction_pool_resolution_object_type)

         // Asset objects

         (asset_object_type)
         (asset_dynamic_data_object_type)
         (asset_currency_data_object_type)
         (asset_reward_fund_object_type)
         (asset_stablecoin_data_object_type)
         (asset_settlement_object_type)
         (asset_collateral_bid_object_type)
         (asset_equity_data_object_type)
         (asset_bond_data_object_type)
         (asset_credit_data_object_type)
         (asset_stimulus_data_object_type)
         (asset_unique_data_object_type)
         (asset_distribution_object_type)
         (asset_distribution_balance_object_type)

         // Block producer objects
         
         (producer_object_type)
         (producer_schedule_object_type)
         (producer_vote_object_type)
         (block_validation_object_type)
         (commit_violation_object_type)
         );

FC_REFLECT_TYPENAME( node::chain::shared_string );
FC_REFLECT_TYPENAME( node::chain::buffer_type );