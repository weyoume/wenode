#pragma once

#include <node/protocol/operation_util.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/protocol/node_virtual_operations.hpp>

namespace node { namespace protocol {

   /** 
    * NOTE: Adjusting the order of any operations prior to the virtual operations will trigger a hardfork.
    */
   typedef static_variant<

            // Account Operations

            account_create_operation,
            account_update_operation,
            account_profile_operation,
            account_verification_operation,
            account_business_operation,
            account_membership_operation,
            account_vote_executive_operation,
            account_vote_officer_operation,
            account_member_request_operation,
            account_member_invite_operation,
            account_accept_request_operation,
            account_accept_invite_operation,
            account_remove_member_operation,
            account_update_list_operation,
            account_producer_vote_operation,
            account_update_proxy_operation,
            request_account_recovery_operation,
            recover_account_operation,
            reset_account_operation,
            set_reset_account_operation,
            change_recovery_account_operation,
            decline_voting_rights_operation,
            connection_request_operation,
            connection_accept_operation,
            account_follow_operation,
            tag_follow_operation,
            activity_reward_operation,

            // Network Operations

            update_network_officer_operation,
            network_officer_vote_operation,
            update_executive_board_operation,
            executive_board_vote_operation,
            update_governance_operation,
            subscribe_governance_operation,
            update_supernode_operation,
            update_interface_operation,
            update_mediator_operation,
            create_community_enterprise_operation,
            claim_enterprise_milestone_operation,
            approve_enterprise_milestone_operation,
            
            // Post and Comment operations

            comment_operation,
            message_operation,
            vote_operation,
            view_operation,
            share_operation,
            moderation_tag_operation,

            // Community Operations

            community_create_operation,
            community_update_operation,
            community_add_mod_operation,
            community_add_admin_operation,
            community_vote_mod_operation,
            community_transfer_ownership_operation,
            community_join_request_operation,
            community_join_accept_operation,
            community_join_invite_operation,
            community_invite_accept_operation,
            community_remove_member_operation,
            community_blacklist_operation,
            community_subscribe_operation,
            community_event_operation,
            community_event_attend_operation,

            // Advertising Operations

            ad_creative_operation,
            ad_campaign_operation,
            ad_inventory_operation,
            ad_audience_operation,
            ad_bid_operation,

            // Graph Data Operations

            graph_node_operation,
            graph_edge_operation,
            graph_node_property_operation,
            graph_edge_property_operation,
            
            // Transfer Operations

            transfer_operation,
            transfer_request_operation,
            transfer_accept_operation,
            transfer_recurring_operation,
            transfer_recurring_request_operation,
            transfer_recurring_accept_operation,
            transfer_confidential_operation,
            transfer_to_confidential_operation,
            transfer_from_confidential_operation,

            // Balance Operations

            claim_reward_balance_operation,
            stake_asset_operation,
            unstake_asset_operation,
            unstake_asset_route_operation,
            transfer_to_savings_operation,
            transfer_from_savings_operation,
            delegate_asset_operation,

            // Marketplace Operations

            product_update_operation,
            product_purchase_operation,
            escrow_transfer_operation,
            escrow_approve_operation,
            escrow_dispute_operation,
            escrow_release_operation,

            // Trading Operations

            limit_order_operation,
            margin_order_operation,
            auction_order_operation,
            call_order_operation,
            option_order_operation,
            
            // Pool Operations

            liquidity_pool_create_operation,
            liquidity_pool_exchange_operation,
            liquidity_pool_fund_operation,
            liquidity_pool_withdraw_operation,
            credit_pool_collateral_operation,
            credit_pool_borrow_operation,
            credit_pool_lend_operation,
            credit_pool_withdraw_operation,
            option_pool_create_operation,
            prediction_pool_create_operation,
            prediction_pool_exchange_operation,
            prediction_pool_resolve_operation,

            // Asset Operations

            asset_create_operation,
            asset_update_operation,
            asset_issue_operation,
            asset_reserve_operation,
            asset_update_issuer_operation,
            asset_distribution_operation,
            asset_distribution_fund_operation,
            asset_option_exercise_operation,
            asset_update_feed_producers_operation,
            asset_publish_feed_operation,
            asset_settle_operation,
            asset_global_settle_operation,
            asset_collateral_bid_operation,
            
            // Block producer operations

            producer_update_operation,
            proof_of_work_operation,
            verify_block_operation,
            commit_block_operation,
            producer_violation_operation,

            // Custom Operations

            custom_operation,
            custom_json_operation,

            /// virtual operations 

            content_reward_operation,
            author_reward_operation,
            vote_reward_operation,
            view_reward_operation,
            share_reward_operation,
            comment_reward_operation,
            supernode_reward_operation,
            moderation_reward_operation,
            comment_payout_update_operation,
            comment_benefactor_reward_operation,
            interest_operation,
            fill_order_operation,
            execute_bid_operation,
            shutdown_producer_operation,
            fill_transfer_from_savings_operation,
            hardfork_operation,
            return_asset_delegation_operation,
            producer_reward_operation
         > operation;

   bool is_market_operation( const operation& op );

   bool is_virtual_operation( const operation& op );

} } // node::protocol

DECLARE_OPERATION_TYPE( node::protocol::operation );

FC_REFLECT_TYPENAME( node::protocol::operation );