#pragma once
#include <node/protocol/types.hpp>
#include <node/protocol/node_operations.hpp>

#include <node/chain/evaluator.hpp>

namespace node { namespace chain {

using namespace node::protocol;

// Account Evaluators

DEFINE_EVALUATOR( account_create )
DEFINE_EVALUATOR( account_update )
DEFINE_EVALUATOR( account_profile )
DEFINE_EVALUATOR( account_verification )
DEFINE_EVALUATOR( account_business )
DEFINE_EVALUATOR( account_membership )
DEFINE_EVALUATOR( account_vote_executive )
DEFINE_EVALUATOR( account_vote_officer )
DEFINE_EVALUATOR( account_member_request )
DEFINE_EVALUATOR( account_member_invite )
DEFINE_EVALUATOR( account_accept_request )
DEFINE_EVALUATOR( account_accept_invite )
DEFINE_EVALUATOR( account_remove_member )
DEFINE_EVALUATOR( account_update_list )
DEFINE_EVALUATOR( account_producer_vote )
DEFINE_EVALUATOR( account_update_proxy )
DEFINE_EVALUATOR( request_account_recovery )
DEFINE_EVALUATOR( recover_account )
DEFINE_EVALUATOR( reset_account )
DEFINE_EVALUATOR( set_reset_account )
DEFINE_EVALUATOR( change_recovery_account )
DEFINE_EVALUATOR( decline_voting_rights )
DEFINE_EVALUATOR( connection_request )
DEFINE_EVALUATOR( connection_accept )
DEFINE_EVALUATOR( account_follow )
DEFINE_EVALUATOR( tag_follow )
DEFINE_EVALUATOR( activity_reward )

// Network Evaluators

DEFINE_EVALUATOR( update_network_officer )
DEFINE_EVALUATOR( network_officer_vote )
DEFINE_EVALUATOR( update_executive_board )
DEFINE_EVALUATOR( executive_board_vote )
DEFINE_EVALUATOR( update_governance )
DEFINE_EVALUATOR( subscribe_governance )
DEFINE_EVALUATOR( update_supernode )
DEFINE_EVALUATOR( update_interface )
DEFINE_EVALUATOR( update_mediator )
DEFINE_EVALUATOR( create_community_enterprise )
DEFINE_EVALUATOR( claim_enterprise_milestone )
DEFINE_EVALUATOR( approve_enterprise_milestone )

// Post and Comment Evaluators

DEFINE_EVALUATOR( comment )
DEFINE_EVALUATOR( message )
DEFINE_EVALUATOR( vote )
DEFINE_EVALUATOR( view )
DEFINE_EVALUATOR( share )
DEFINE_EVALUATOR( moderation_tag )

// Community Evaluators

DEFINE_EVALUATOR( community_create )
DEFINE_EVALUATOR( community_update )
DEFINE_EVALUATOR( community_add_mod )
DEFINE_EVALUATOR( community_add_admin )
DEFINE_EVALUATOR( community_vote_mod )
DEFINE_EVALUATOR( community_transfer_ownership )
DEFINE_EVALUATOR( community_join_request )
DEFINE_EVALUATOR( community_join_accept )
DEFINE_EVALUATOR( community_join_invite )
DEFINE_EVALUATOR( community_invite_accept )
DEFINE_EVALUATOR( community_remove_member )
DEFINE_EVALUATOR( community_blacklist )
DEFINE_EVALUATOR( community_subscribe )
DEFINE_EVALUATOR( community_event )
DEFINE_EVALUATOR( community_event_attend )

// Advertising Evaluators

DEFINE_EVALUATOR( ad_creative )
DEFINE_EVALUATOR( ad_campaign )
DEFINE_EVALUATOR( ad_inventory )
DEFINE_EVALUATOR( ad_audience )
DEFINE_EVALUATOR( ad_bid )

// Graph Data Evaluators

DEFINE_EVALUATOR( graph_node )
DEFINE_EVALUATOR( graph_edge )
DEFINE_EVALUATOR( graph_node_property )
DEFINE_EVALUATOR( graph_edge_property )

// Transfer Evaluators

DEFINE_EVALUATOR( transfer )
DEFINE_EVALUATOR( transfer_request )
DEFINE_EVALUATOR( transfer_accept )
DEFINE_EVALUATOR( transfer_recurring )
DEFINE_EVALUATOR( transfer_recurring_request )
DEFINE_EVALUATOR( transfer_recurring_accept )

// Balance Evaluators

DEFINE_EVALUATOR( claim_reward_balance )
DEFINE_EVALUATOR( stake_asset )
DEFINE_EVALUATOR( unstake_asset )
DEFINE_EVALUATOR( unstake_asset_route )
DEFINE_EVALUATOR( transfer_to_savings )
DEFINE_EVALUATOR( transfer_from_savings )
DEFINE_EVALUATOR( delegate_asset )

// Marketplace Evaluators

DEFINE_EVALUATOR( product_update )
DEFINE_EVALUATOR( product_purchase )
DEFINE_EVALUATOR( escrow_transfer )
DEFINE_EVALUATOR( escrow_approve )
DEFINE_EVALUATOR( escrow_dispute )
DEFINE_EVALUATOR( escrow_release )

// Trading Evaluators

DEFINE_EVALUATOR( limit_order )
DEFINE_EVALUATOR( margin_order )
DEFINE_EVALUATOR( auction_order )
DEFINE_EVALUATOR( call_order )
DEFINE_EVALUATOR( option_order )
DEFINE_EVALUATOR( bid_collateral )

// Liquidity Pool Evaluators

DEFINE_EVALUATOR( liquidity_pool_create )
DEFINE_EVALUATOR( liquidity_pool_exchange )
DEFINE_EVALUATOR( liquidity_pool_fund )
DEFINE_EVALUATOR( liquidity_pool_withdraw )
DEFINE_EVALUATOR( credit_pool_collateral )
DEFINE_EVALUATOR( credit_pool_borrow )
DEFINE_EVALUATOR( credit_pool_lend )
DEFINE_EVALUATOR( credit_pool_withdraw )

// Asset Evaluators

DEFINE_EVALUATOR( asset_create )
DEFINE_EVALUATOR( asset_update )
DEFINE_EVALUATOR( asset_issue )
DEFINE_EVALUATOR( asset_reserve )
DEFINE_EVALUATOR( asset_update_issuer )
DEFINE_EVALUATOR( asset_update_feed_producers )
DEFINE_EVALUATOR( asset_publish_feed )
DEFINE_EVALUATOR( asset_settle )
DEFINE_EVALUATOR( asset_global_settle )

// Block Producer Evaluators

DEFINE_EVALUATOR( producer_update )
DEFINE_EVALUATOR( proof_of_work )
DEFINE_EVALUATOR( verify_block )
DEFINE_EVALUATOR( commit_block )
DEFINE_EVALUATOR( producer_violation )

// Custom Evaluators

DEFINE_EVALUATOR( custom )
DEFINE_EVALUATOR( custom_json )

} } // node::chain
