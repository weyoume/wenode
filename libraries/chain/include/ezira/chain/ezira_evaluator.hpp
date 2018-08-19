#pragma once

#include <eznode/protocol/eznode_operations.hpp>

#include <eznode/chain/evaluator.hpp>

namespace eznode{ namespace chain {

using namespace eznode::protocol;

DEFINE_EVALUATOR( accountCreate )
DEFINE_EVALUATOR( accountCreateWithDelegation )
DEFINE_EVALUATOR( accountUpdate )
DEFINE_EVALUATOR( transfer )
DEFINE_EVALUATOR( transferECOtoESCORfund )
DEFINE_EVALUATOR( witness_update )
DEFINE_EVALUATOR( accountWitnessVote )
DEFINE_EVALUATOR( account_witness_proxy )
DEFINE_EVALUATOR( withdraw_ESCOR )
DEFINE_EVALUATOR( setWithdrawESCORasECOroute )
DEFINE_EVALUATOR( comment )
DEFINE_EVALUATOR( comment_options )
DEFINE_EVALUATOR( deleteComment )
DEFINE_EVALUATOR( vote )
DEFINE_EVALUATOR( custom )
DEFINE_EVALUATOR( customJson )
DEFINE_EVALUATOR( custom_binary )
DEFINE_EVALUATOR( pow )
DEFINE_EVALUATOR( pow2 )
DEFINE_EVALUATOR( feed_publish )
DEFINE_EVALUATOR( convert )
DEFINE_EVALUATOR( limit_order_create )
DEFINE_EVALUATOR( limit_order_cancel )
DEFINE_EVALUATOR( report_over_production )
DEFINE_EVALUATOR( limit_order_create2 )
DEFINE_EVALUATOR( escrow_transfer )
DEFINE_EVALUATOR( escrow_approve )
DEFINE_EVALUATOR( escrow_dispute )
DEFINE_EVALUATOR( escrow_release )
DEFINE_EVALUATOR( challenge_authority )
DEFINE_EVALUATOR( prove_authority )
DEFINE_EVALUATOR( request_account_recovery )
DEFINE_EVALUATOR( recover_account )
DEFINE_EVALUATOR( change_recovery_account )
DEFINE_EVALUATOR( transferToSavings )
DEFINE_EVALUATOR( transferFromSavings )
DEFINE_EVALUATOR( cancelTransferFromSavings )
DEFINE_EVALUATOR( decline_voting_rights )
DEFINE_EVALUATOR( reset_account )
DEFINE_EVALUATOR( set_reset_account )
DEFINE_EVALUATOR( claimRewardBalance )
DEFINE_EVALUATOR( delegateESCOR )

} } // eznode::chain
