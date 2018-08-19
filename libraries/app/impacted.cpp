/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <eznode/protocol/authority.hpp>

#include <eznode/app/impacted.hpp>

#include <fc/utility.hpp>

namespace eznode { namespace app {

using namespace fc;
using namespace eznode::protocol;

// TODO:  Review all of these, especially no-ops
struct get_impacted_account_visitor
{
   flat_set<account_name_type>& _impacted;
   get_impacted_account_visitor( flat_set<account_name_type>& impact ):_impacted( impact ) {}
   typedef void result_type;

   template<typename T>
   void operator()( const T& op )
   {
      op.get_required_posting_authorities( _impacted );
      op.get_required_active_authorities( _impacted );
      op.get_required_owner_authorities( _impacted );
   }

   // ops
   void operator()( const accountCreate_operation& op )
   {
      _impacted.insert( op.newAccountName );
      _impacted.insert( op.creator );
   }

   void operator()( const accountCreateWithDelegation_operation& op )
   {
      _impacted.insert( op.newAccountName );
      _impacted.insert( op.creator );
   }

   void operator()( const comment_operation& op )
   {
      _impacted.insert( op.author );
      if( op.parent_author.size() )
         _impacted.insert( op.parent_author );
   }

   void operator()( const challenge_authority_operation& op )
   {
      _impacted.insert( op.challenger );
      _impacted.insert( op.challenged );
   }

   void operator()( const vote_operation& op )
   {
      _impacted.insert( op.voter );
      _impacted.insert( op.author );
   }

   void operator()( const transfer_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const escrow_transfer_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
      _impacted.insert( op.agent );
   }

   void operator()( const escrow_approve_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
      _impacted.insert( op.agent );
   }

   void operator()( const escrow_dispute_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
      _impacted.insert( op.agent );
   }

   void operator()( const escrow_release_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
      _impacted.insert( op.agent );
   }

   void operator()( const transferECOtoESCORfund_operation& op )
   {
      _impacted.insert( op.from );

      if ( op.to != account_name_type() && op.to != op.from )
      {
         _impacted.insert( op.to );
      }
   }

   void operator()( const setWithdrawESCORasECOroute_operation& op )
   {
      _impacted.insert( op.from_account );
      _impacted.insert( op.to_account );
   }

   void operator()( const accountWitnessVote_operation& op )
   {
      _impacted.insert( op.account );
      _impacted.insert( op.witness );
   }

   void operator()( const account_witness_proxy_operation& op )
   {
      _impacted.insert( op.account );
      _impacted.insert( op.proxy );
   }

   void operator()( const feed_publish_operation& op )
   {
      _impacted.insert( op.publisher );
   }

   void operator()( const pow_operation& op )
   {
      _impacted.insert( op.worker_account );
   }

   struct pow2_impacted_visitor
   {
      pow2_impacted_visitor(){}

      typedef const account_name_type& result_type;

      template< typename WorkType >
      result_type operator()( const WorkType& work )const
      {
         return work.input.worker_account;
      }
   };

   void operator()( const pow2_operation& op )
   {
      _impacted.insert( op.work.visit( pow2_impacted_visitor() ) );
   }

   void operator()( const request_account_recovery_operation& op )
   {
      _impacted.insert( op.accountToRecover );
      _impacted.insert( op.recoveryAccount );
   }

   void operator()( const recover_account_operation& op )
   {
      _impacted.insert( op.accountToRecover );
   }

   void operator()( const change_recoveryAccount_operation& op )
   {
      _impacted.insert( op.accountToRecover );
   }

   void operator()( const transferToSavings_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const transferFromSavings_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const delegateESCOR_operation& op )
   {
      _impacted.insert( op.delegator );
      _impacted.insert( op.delegatee );
   }


   // vops

   void operator()( const authorReward_operation& op )
   {
      _impacted.insert( op.author );
   }

   void operator()( const curationReward_operation& op )
   {
      _impacted.insert( op.curator );
   }

   void operator()( const liquidity_reward_operation& op )
   {
      _impacted.insert( op.owner );
   }

   void operator()( const interest_operation& op )
   {
      _impacted.insert( op.owner );
   }

   void operator()( const fill_convert_request_operation& op )
   {
      _impacted.insert( op.owner );
   }

   void operator()( const fillESCORWithdraw_operation& op )
   {
      _impacted.insert( op.from_account );
      _impacted.insert( op.to_account );
   }

   void operator()( const shutdown_witness_operation& op )
   {
      _impacted.insert( op.owner );
   }

   void operator()( const fill_order_operation& op )
   {
      _impacted.insert( op.current_owner );
      _impacted.insert( op.open_owner );
   }

   void operator()( const fill_transferFromSavings_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const return_ESCOR_delegation_operation& op )
   {
      _impacted.insert( op.account );
   }

   void operator()( const comment_benefactor_reward_operation& op )
   {
      _impacted.insert( op.benefactor );
      _impacted.insert( op.author );
   }

   void operator()( const producer_reward_operation& op )
   {
      _impacted.insert( op.producer );
   }

   //void operator()( const operation& op ){}
};

void operation_get_impacted_accounts( const operation& op, flat_set<account_name_type>& result )
{
   get_impacted_account_visitor vtor = get_impacted_account_visitor( result );
   op.visit( vtor );
}

void transaction_get_impacted_accounts( const transaction& tx, flat_set<account_name_type>& result )
{
   for( const auto& op : tx.operations )
      operation_get_impacted_accounts( op, result );
}

} }
