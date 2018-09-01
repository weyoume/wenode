#pragma once

#include <node/protocol/operation_util.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/protocol/node_virtual_operations.hpp>

namespace node { namespace protocol {

   /** NOTE: do not change the order of any operations prior to the virtual operations
    * or it will trigger a hardfork.
    */
   typedef fc::static_variant<
            vote_operation,
            comment_operation,

            transfer_operation,
            transferECOtoESCORfund_operation,
            withdrawESCOR_operation,

            limit_order_create_operation,
            limit_order_cancel_operation,

            feed_publish_operation,
            convert_operation,

            accountCreate_operation,
            accountUpdate_operation,

            witness_update_operation,
            accountWitnessVote_operation,
            account_witness_proxy_operation,

            pow_operation,

            custom_operation,

            report_over_production_operation,

            deleteComment_operation,
            customJson_operation,
            comment_options_operation,
            setWithdrawESCORasECOroute_operation,
            limit_order_create2_operation,
            challenge_authority_operation,
            prove_authority_operation,
            request_account_recovery_operation,
            recover_account_operation,
            change_recoveryAccount_operation,
            escrow_transfer_operation,
            escrow_dispute_operation,
            escrow_release_operation,
            pow2_operation,
            escrow_approve_operation,
            transferToSavings_operation,
            transferFromSavings_operation,
            cancelTransferFromSavings_operation,
            custom_binary_operation,
            decline_voting_rights_operation,
            reset_account_operation,
            set_reset_account_operation,
            claimRewardBalance_operation,
            delegateESCOR_operation,
            accountCreateWithDelegation_operation,

            /// virtual operations below this point
            fill_convert_request_operation,
            authorReward_operation,
            curationReward_operation,
            comment_reward_operation,
            liquidity_reward_operation,
            interest_operation,
            fillESCORWithdraw_operation,
            fill_order_operation,
            shutdown_witness_operation,
            fill_transferFromSavings_operation,
            hardfork_operation,
            comment_payout_update_operation,
            return_ESCOR_delegation_operation,
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

/*namespace fc {
    void to_variant( const node::protocol::operation& var,  fc::variant& vo );
    void from_variant( const fc::variant& var,  node::protocol::operation& vo );
}*/

DECLARE_OPERATION_TYPE( node::protocol::operation )
FC_REFLECT_TYPENAME( node::protocol::operation )
