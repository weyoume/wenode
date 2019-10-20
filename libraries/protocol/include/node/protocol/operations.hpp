#pragma once

#include <node/protocol/operation_util.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/protocol/node_virtual_operations.hpp>

namespace node { namespace protocol {

   /** NOTE: do not change the order of any operations prior to the virtual operations
    * or it will trigger a hardfork.
    */
   typedef static_variant<

            // Account Operations

            account_create_operation,
            account_update_operation,
            account_membership_operation,
            account_vote_executive_operation,
            account_vote_officer_operation,
            account_member_request_operation,
            account_member_invite_operation,
            account_accept_request_operation,
            account_accept_invite_operation,
            account_remove_member_operation,
            account_update_list_operation,
            account_witness_vote_operation,
            account_update_proxy_operation,
            request_account_recovery_operation,
            recover_account_operation,
            reset_account_operation,
            set_reset_account_operation,
            change_recovery_account_operation,
            challenge_authority_operation,
            prove_authority_operation,
            decline_voting_rights_operation,
            connection_request_operation,
            connection_accept_operation,
            account_follow_operation,
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
            create_community_enterprise_operation,
            claim_enterprise_milestone_operation,
            approve_enterprise_milestone_operation,
            
            // Post and Comment operations

            comment_operation,
            comment_options_operation,
            message_operation,
            vote_operation,
            view_operation,
            share_operation,
            moderation_tag_operation,

            // Board Operations

            board_create_operation,
            board_update_operation,
            board_add_mod_operation,
            board_add_admin_operation,
            board_vote_mod_operation,
            board_transfer_ownership_operation,
            board_join_request_operation,
            board_join_accept_operation,
            board_join_invite_operation,
            board_invite_accept_operation,
            board_remove_member_operation,
            board_blacklist_operation,
            board_subscribe_operation,

            // Advertising Operations

            ad_creative_operation,
            ad_campaign_operation,
            ad_inventory_operation,
            ad_audience_operation,
            ad_bid_operation,
            ad_deliver_operation,
            
            // Transfer Operations

            transfer_operation,
            transfer_request_operation,
            transfer_accept_operation,
            transfer_recurring_operation,
            transfer_recurring_request_operation,
            transfer_recurring_accept_operation,

            // Balance Operations

            claim_reward_balance_operation,
            stake_asset_operation,
            unstake_asset_operation,
            unstake_asset_route_operation,
            transfer_to_savings_operation,
            transfer_from_savings_operation,
            cancel_transfer_from_savings_operation,
            delegate_asset_operation,

            // Escrow Operations

            escrow_transfer_operation,
            escrow_approve_operation,
            escrow_dispute_operation,
            escrow_release_operation,

            // Trading Operations

            limit_order_create_operation,
            limit_order_cancel_operation,
            margin_order_create_operation,
            margin_order_close_operation,
            call_order_update_operation,
            bid_collateral_operation,

            // Pool Operations

            liquidity_pool_create_operation,
            liquidity_pool_exchange_operation,
            liquidity_pool_fund_operation,
            liquidity_pool_withdraw_operation,
            credit_pool_collateral_operation,
            credit_pool_borrow_operation,
            credit_pool_lend_operation,
            credit_pool_withdraw_operation,

            // Asset Operations

            asset_create_operation,          
            asset_update_operation,
            asset_issue_operation, 
            asset_reserve_operation, 
            asset_claim_fees_operation,
            asset_claim_pool_operation, 
            asset_fund_fee_pool_operation,
            asset_update_issuer_operation,        
            asset_update_bitasset_operation,         
            asset_update_feed_producers_operation,         
            asset_publish_feed_operation, 
            asset_settle_operation,  
            asset_global_settle_operation,
            
            // Block producer operations

            witness_update_operation,
            proof_of_work_operation,
            verify_block_operation,
            commit_block_operation,
            producer_violation_operation,

            // Custom Operations

            custom_operation,
            custom_json_operation,

            /// virtual operations 

            author_reward_operation,
            curation_reward_operation,
            comment_reward_operation,
            interest_operation,
            fill_order_operation,
            asset_settle_cancel_operation,
            shutdown_witness_operation,
            fill_transfer_from_savings_operation,
            hardfork_operation,
            comment_payout_update_operation,
            return_asset_delegation_operation,
            comment_benefactor_reward_operation,
            producer_reward_operation
         > operation;

   /*void operation_get_required_authorities( const operation& op,
                                            flat_set<string>& active,
                                            flat_set<string>& owner,
                                            flat_set<string>& posting,
                                            vector<authority>&  other );

   void operation_validate( const operation& op );*/

   bool is_market_operation( const operation& op );

   bool is_virtual_operation( const operation& op );

} } // node::protocol

DECLARE_OPERATION_TYPE( node::protocol::operation );

FC_REFLECT_TYPENAME( node::protocol::operation );
