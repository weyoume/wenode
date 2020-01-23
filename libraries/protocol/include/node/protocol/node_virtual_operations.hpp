#pragma once
#include <node/protocol/base.hpp>
#include <node/protocol/block_header.hpp>
#include <node/protocol/asset.hpp>

#include <fc/utf8.hpp>

namespace node { namespace protocol {

   struct author_reward_operation : public virtual_operation 
   {
      author_reward_operation(){}
      author_reward_operation( const account_name_type& a, const string& p, const asset& r )
         :author(a), permlink(p), reward(r){}

      account_name_type author;
      string            permlink;
      asset             reward;
   };


   struct curation_reward_operation : public virtual_operation
   {
      curation_reward_operation(){}
      curation_reward_operation( const string& c, const asset& r, const string& a, const string& p )
         :curator(c), reward(r), comment_author(a), comment_permlink(p) {}

      account_name_type curator;
      asset             reward;
      account_name_type comment_author;
      string            comment_permlink;
   };


   struct comment_reward_operation : public virtual_operation
   {
      comment_reward_operation(){}
      comment_reward_operation( const account_name_type& a, const string& pl, const asset& p )
         :author(a), permlink(pl), payout(p){}

      account_name_type author;
      string            permlink;
      asset             payout;
   };


   struct interest_operation : public virtual_operation
   {
      interest_operation( const string& o = "", const asset& i = asset(0,SYMBOL_USD) )
         :owner(o),interest(i){}

      account_name_type owner;
      asset             interest;
   };


   struct shutdown_witness_operation : public virtual_operation
   {
      shutdown_witness_operation(){}
      shutdown_witness_operation( const string& o ):owner(o) {}

      account_name_type owner;
   };


   struct fill_order_operation : public virtual_operation
   {
      fill_order_operation(){}
      fill_order_operation( 
         const account_name_type& c_o, 
         const string& c_id, 
         const asset& c_p, 
         const account_name_type& o_o, 
         const string& o_id, 
         const asset& o_p,
         const asset_symbol_type& sym_a,
         const asset_symbol_type& sym_b ):
         current_owner(c_o),
         current_order_id(c_id),
         current_pays(c_p),
         open_owner(o_o),
         open_order_id(o_id),
         open_pays(o_p),
         symbol_a(sym_a),
         symbol_b(sym_b){}

      account_name_type      current_owner;
      string                 current_order_id;
      asset                  current_pays;
      account_name_type      open_owner;
      string                 open_order_id;
      asset                  open_pays;
      asset_symbol_type      symbol_a;
      asset_symbol_type      symbol_b;
   };

   struct asset_settle_cancel_operation : public virtual_operation
   {
      account_name_type             account;        // Account requesting the force settlement. This account pays the fee
      
      asset                         amount;         // Amount of asset to force settle. This must be a market-issued asset

      force_settlement_id_type      settlement; 

      extensions_type               extensions;

      void            validate()const;
      void                  get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert(account); }
   };


   struct fill_transfer_from_savings_operation : public virtual_operation
   {
      fill_transfer_from_savings_operation() {}
      fill_transfer_from_savings_operation( 
         const account_name_type& f, 
         const account_name_type& t, 
         const asset& a, 
         const string& r, 
         const string& m ):
         from(f), 
         to(t), 
         amount(a), 
         request_id(r), 
         memo(m){}

      account_name_type    from;
      account_name_type    to;
      asset                amount;
      string               request_id;
      string               memo;
   };

   struct hardfork_operation : public virtual_operation
   {
      hardfork_operation() {}
      hardfork_operation( uint32_t hf_id ) : hardfork_id( hf_id ) {}

      uint32_t         hardfork_id = 0;
   };

   struct comment_payout_update_operation : public virtual_operation
   {
      comment_payout_update_operation() {}
      comment_payout_update_operation( const account_name_type& a, const string& p ) : author( a ), permlink( p ) {}

      account_name_type author;
      string            permlink;
   };

   struct return_asset_delegation_operation : public virtual_operation
   {
      return_asset_delegation_operation() {}
      return_asset_delegation_operation( const account_name_type& a, const asset& v ) : account( a ), amount( v ) {}

      account_name_type account;
      asset             amount;
   };

   struct comment_benefactor_reward_operation : public virtual_operation
   {
      comment_benefactor_reward_operation() {}
      comment_benefactor_reward_operation( const account_name_type& b, const account_name_type& a, const string& p, const asset& r )
         : benefactor( b ), author( a ), permlink( p ), reward( r ) {}

      account_name_type benefactor;
      account_name_type author;
      string            permlink;
      asset             reward;
   };

   struct producer_reward_operation : public virtual_operation
   {
      producer_reward_operation(){}
      producer_reward_operation( const string& p, const asset& r ) : producer( p ), reward( r ) {}

      account_name_type producer;
      asset             reward;

   };

   struct execute_bid_operation : public virtual_operation
   {
      
      execute_bid_operation(){}
      execute_bid_operation( account_name_type a, asset d, asset c )
         : bidder(a), debt(d), collateral(c) {}

      account_name_type   bidder;

      asset               debt;

      asset               collateral;

      void            validate()const { FC_ASSERT( !"virtual operation" ); }   
   };

} } //node::protocol

FC_REFLECT( node::protocol::author_reward_operation, 
         (author)
         (permlink)
         (reward) 
         );

FC_REFLECT( node::protocol::curation_reward_operation, 
         (curator)
         (reward)
         (comment_author)
         (comment_permlink) 
         );

FC_REFLECT( node::protocol::comment_reward_operation, 
         (author)
         (permlink)
         (payout) 
         );

FC_REFLECT( node::protocol::interest_operation, 
         (owner)
         (interest) 
         );

FC_REFLECT( node::protocol::shutdown_witness_operation, 
         (owner) 
         );

FC_REFLECT( node::protocol::fill_order_operation, 
         (current_owner)
         (current_order_id)
         (current_pays)
         (open_owner)
         (open_order_id)
         (open_pays) 
         );

FC_REFLECT( node::protocol::fill_transfer_from_savings_operation, 
         (from)
         (to)
         (amount)
         (request_id)
         (memo) 
         );

FC_REFLECT( node::protocol::hardfork_operation, 
         (hardfork_id) 
         );

FC_REFLECT( node::protocol::comment_payout_update_operation, 
         (author)
         (permlink) 
         );

FC_REFLECT( node::protocol::return_asset_delegation_operation, 
         (account)
         (asset) 
         );

FC_REFLECT( node::protocol::comment_benefactor_reward_operation, 
         (benefactor)
         (author)
         (permlink)
         (reward) 
         );

FC_REFLECT( node::protocol::producer_reward_operation, 
         (producer)
         (reward) 
         );
