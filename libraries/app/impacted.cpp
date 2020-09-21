#include <node/protocol/authority.hpp>
#include <node/app/impacted.hpp>
#include <fc/utility.hpp>

namespace node { namespace app {

using namespace fc;
using namespace node::protocol;

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
   void operator()( const account_create_operation& op )
   {
      _impacted.insert( op.new_account_name );
      _impacted.insert( op.registrar );
   }

   void operator()( const comment_operation& op )
   {
      _impacted.insert( op.author );
      if( op.parent_author.size() )
         _impacted.insert( op.parent_author );
   }

   void operator()( const comment_vote_operation& op )
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
      _impacted.insert( op.account );
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const escrow_approve_operation& op )
   {
      _impacted.insert( op.account );
      _impacted.insert( op.mediator );
      _impacted.insert( op.escrow_from );
   }

   void operator()( const escrow_dispute_operation& op )
   {
      _impacted.insert( op.account );
      _impacted.insert( op.escrow_from );
   }

   void operator()( const escrow_release_operation& op )
   {
      _impacted.insert( op.account );
      _impacted.insert( op.escrow_from );
   }

   void operator()( const stake_asset_operation& op )
   {
      _impacted.insert( op.from );

      if ( op.to != account_name_type() && op.to != op.from )
      {
         _impacted.insert( op.to );
      }
   }

   void operator()( const unstake_asset_route_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const account_producer_vote_operation& op )
   {
      _impacted.insert( op.account );
      _impacted.insert( op.producer );
   }

   void operator()( const account_update_proxy_operation& op )
   {
      _impacted.insert( op.account );
      _impacted.insert( op.proxy );
   }

   struct proof_of_work_impacted_visitor
   {
      proof_of_work_impacted_visitor(){}

      typedef const account_name_type& result_type;

      template< typename WorkType >
      result_type operator()( const WorkType& work )const
      {
         return work.input.miner_account;
      }
   };

   void operator()( const proof_of_work_operation& op )
   {
      _impacted.insert( op.work.visit( proof_of_work_impacted_visitor() ) );
   }

   void operator()( const account_request_recovery_operation& op )
   {
      _impacted.insert( op.account_to_recover );
      _impacted.insert( op.recovery_account );
   }

   void operator()( const account_recover_operation& op )
   {
      _impacted.insert( op.account_to_recover );
   }

   void operator()( const account_recovery_update_operation& op )
   {
      _impacted.insert( op.account_to_recover );
   }

   void operator()( const transfer_to_savings_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const transfer_from_savings_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const delegate_asset_operation& op )
   {
      _impacted.insert( op.delegator );
      _impacted.insert( op.delegatee );
   }

   // vops

   void operator()( const author_reward_operation& op )
   {
      _impacted.insert( op.post_author );
   }

   void operator()( const vote_reward_operation& op )
   {
      _impacted.insert( op.voter );
   }

   void operator()( const view_reward_operation& op )
   {
      _impacted.insert( op.viewer );
   }

   void operator()( const share_reward_operation& op )
   {
      _impacted.insert( op.sharer );
   }

   void operator()( const comment_reward_operation& op )
   {
      _impacted.insert( op.comment_author );
   }

   void operator()( const moderation_reward_operation& op )
   {
      _impacted.insert( op.moderator );
   }

   void operator()( const supernode_reward_operation& op )
   {
      _impacted.insert( op.supernode );
   }

   void operator()( const interest_operation& op )
   {
      _impacted.insert( op.owner );
   }

   void operator()( const shutdown_producer_operation& op )
   {
      _impacted.insert( op.owner );
   }

   void operator()( const fill_order_operation& op )
   {
      _impacted.insert( op.current_owner );
      _impacted.insert( op.open_owner );
   }

   void operator()( const fill_transfer_from_savings_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const return_asset_delegation_operation& op )
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