#pragma once
#include <node/protocol/types.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/chain/evaluator.hpp>

namespace node { namespace chain {

using namespace node::protocol;

// Account Evaluators

DEFINE_EVALUATOR( account_create )
DEFINE_EVALUATOR( account_update )
DEFINE_EVALUATOR( account_verification )
DEFINE_EVALUATOR( account_membership )
DEFINE_EVALUATOR( account_update_list )
DEFINE_EVALUATOR( account_producer_vote )
DEFINE_EVALUATOR( account_update_proxy )
DEFINE_EVALUATOR( account_request_recovery )
DEFINE_EVALUATOR( account_recover )
DEFINE_EVALUATOR( account_reset )
DEFINE_EVALUATOR( account_reset_update )
DEFINE_EVALUATOR( account_recovery_update )
DEFINE_EVALUATOR( account_decline_voting )
DEFINE_EVALUATOR( account_connection )
DEFINE_EVALUATOR( account_follow )
DEFINE_EVALUATOR( account_follow_tag )
DEFINE_EVALUATOR( account_activity )

// Business Evaluators

DEFINE_EVALUATOR( business_create )
DEFINE_EVALUATOR( business_update )
DEFINE_EVALUATOR( business_executive )
DEFINE_EVALUATOR( business_executive_vote )
DEFINE_EVALUATOR( business_director )
DEFINE_EVALUATOR( business_director_vote )

// Governance Evaluators

DEFINE_EVALUATOR( governance_create )
DEFINE_EVALUATOR( governance_update )
DEFINE_EVALUATOR( governance_executive )
DEFINE_EVALUATOR( governance_executive_vote )
DEFINE_EVALUATOR( governance_director )
DEFINE_EVALUATOR( governance_director_vote )
DEFINE_EVALUATOR( governance_member )
DEFINE_EVALUATOR( governance_member_request )
DEFINE_EVALUATOR( governance_resolution )
DEFINE_EVALUATOR( governance_resolution_vote )

// Network Evaluators

DEFINE_EVALUATOR( network_officer_update )
DEFINE_EVALUATOR( network_officer_vote )
DEFINE_EVALUATOR( supernode_update )
DEFINE_EVALUATOR( interface_update )
DEFINE_EVALUATOR( mediator_update )
DEFINE_EVALUATOR( enterprise_update )
DEFINE_EVALUATOR( enterprise_fund )
DEFINE_EVALUATOR( enterprise_vote )

// Post and Comment Evaluators

DEFINE_EVALUATOR( comment )
DEFINE_EVALUATOR( message )
DEFINE_EVALUATOR( comment_vote )
DEFINE_EVALUATOR( comment_view )
DEFINE_EVALUATOR( comment_share )
DEFINE_EVALUATOR( comment_moderation )
DEFINE_EVALUATOR( list )
DEFINE_EVALUATOR( poll )
DEFINE_EVALUATOR( poll_vote )
DEFINE_EVALUATOR( premium_purchase )
DEFINE_EVALUATOR( premium_release )

// Community Evaluators

DEFINE_EVALUATOR( community_create )
DEFINE_EVALUATOR( community_update )
DEFINE_EVALUATOR( community_member )
DEFINE_EVALUATOR( community_member_request )
DEFINE_EVALUATOR( community_member_vote )
DEFINE_EVALUATOR( community_subscribe )
DEFINE_EVALUATOR( community_blacklist )
DEFINE_EVALUATOR( community_federation )
DEFINE_EVALUATOR( community_event )
DEFINE_EVALUATOR( community_event_attend )
DEFINE_EVALUATOR( community_directive )
DEFINE_EVALUATOR( community_directive_vote )
DEFINE_EVALUATOR( community_directive_member )
DEFINE_EVALUATOR( community_directive_member_vote )

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
DEFINE_EVALUATOR( transfer_confidential )
DEFINE_EVALUATOR( transfer_to_confidential )
DEFINE_EVALUATOR( transfer_from_confidential )

// Balance Evaluators

DEFINE_EVALUATOR( claim_reward_balance )
DEFINE_EVALUATOR( stake_asset )
DEFINE_EVALUATOR( unstake_asset )
DEFINE_EVALUATOR( unstake_asset_route )
DEFINE_EVALUATOR( transfer_to_savings )
DEFINE_EVALUATOR( transfer_from_savings )
DEFINE_EVALUATOR( delegate_asset )

// Marketplace Evaluators

DEFINE_EVALUATOR( product_sale )
DEFINE_EVALUATOR( product_purchase )
DEFINE_EVALUATOR( product_auction_sale )
DEFINE_EVALUATOR( product_auction_bid )
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

// Pool Evaluators

DEFINE_EVALUATOR( liquidity_pool_create )
DEFINE_EVALUATOR( liquidity_pool_exchange )
DEFINE_EVALUATOR( liquidity_pool_fund )
DEFINE_EVALUATOR( liquidity_pool_withdraw )
DEFINE_EVALUATOR( credit_pool_collateral )
DEFINE_EVALUATOR( credit_pool_borrow )
DEFINE_EVALUATOR( credit_pool_lend )
DEFINE_EVALUATOR( credit_pool_withdraw )
DEFINE_EVALUATOR( option_pool_create )
DEFINE_EVALUATOR( prediction_pool_create )
DEFINE_EVALUATOR( prediction_pool_exchange )
DEFINE_EVALUATOR( prediction_pool_resolve )

// Asset Evaluators

DEFINE_EVALUATOR( asset_create )
DEFINE_EVALUATOR( asset_update )
DEFINE_EVALUATOR( asset_issue )
DEFINE_EVALUATOR( asset_reserve )
DEFINE_EVALUATOR( asset_update_issuer )
DEFINE_EVALUATOR( asset_distribution )
DEFINE_EVALUATOR( asset_distribution_fund )
DEFINE_EVALUATOR( asset_option_exercise )
DEFINE_EVALUATOR( asset_stimulus_fund )
DEFINE_EVALUATOR( asset_update_feed_producers )
DEFINE_EVALUATOR( asset_publish_feed )
DEFINE_EVALUATOR( asset_settle )
DEFINE_EVALUATOR( asset_global_settle )
DEFINE_EVALUATOR( asset_collateral_bid )

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
