#pragma once
#include <eznode/protocol/base.hpp>
#include <eznode/protocol/block_header.hpp>
#include <eznode/protocol/asset.hpp>

#include <fc/utf8.hpp>

namespace eznode { namespace protocol {

   struct authorReward_operation : public virtual_operation {
      authorReward_operation(){}
      authorReward_operation( const account_name_type& a, const string& p, const asset& s, const asset& st, const asset& v )
         :author(a), permlink(p), EUSDpayout(s), ECOpayout(st), ESCORpayout(v){}

      account_name_type author;
      string            permlink;
      asset             EUSDpayout;
      asset             ECOpayout;
      asset             ESCORpayout;
   };


   struct curationReward_operation : public virtual_operation
   {
      curationReward_operation(){}
      curationReward_operation( const string& c, const asset& r, const string& a, const string& p )
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


   struct liquidity_reward_operation : public virtual_operation
   {
      liquidity_reward_operation( string o = string(), asset p = asset() )
      :owner(o), payout(p) {}

      account_name_type owner;
      asset             payout;
   };


   struct interest_operation : public virtual_operation
   {
      interest_operation( const string& o = "", const asset& i = asset(0,SYMBOL_EUSD) )
         :owner(o),interest(i){}

      account_name_type owner;
      asset             interest;
   };


   struct fill_convert_request_operation : public virtual_operation
   {
      fill_convert_request_operation(){}
      fill_convert_request_operation( const string& o, const uint32_t id, const asset& in, const asset& out )
         :owner(o), requestid(id), amount_in(in), amount_out(out) {}

      account_name_type owner;
      uint32_t          requestid = 0;
      asset             amount_in;
      asset             amount_out;
   };


   struct fillESCORWithdraw_operation : public virtual_operation
   {
      fillESCORWithdraw_operation(){}
      fillESCORWithdraw_operation( const string& f, const string& t, const asset& w, const asset& d )
         :from_account(f), to_account(t), withdrawn(w), deposited(d) {}

      account_name_type from_account;
      account_name_type to_account;
      asset             withdrawn;
      asset             deposited;
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
      fill_order_operation( const string& c_o, uint32_t c_id, const asset& c_p, const string& o_o, uint32_t o_id, const asset& o_p )
      :current_owner(c_o), current_orderid(c_id), current_pays(c_p), open_owner(o_o), open_orderid(o_id), open_pays(o_p) {}

      account_name_type current_owner;
      uint32_t          current_orderid = 0;
      asset             current_pays;
      account_name_type open_owner;
      uint32_t          open_orderid = 0;
      asset             open_pays;
   };


   struct fill_transferFromSavings_operation : public virtual_operation
   {
      fill_transferFromSavings_operation() {}
      fill_transferFromSavings_operation( const account_name_type& f, const account_name_type& t, const asset& a, const uint32_t r, const string& m )
         :from(f), to(t), amount(a), request_id(r), memo(m) {}

      account_name_type from;
      account_name_type to;
      asset             amount;
      uint32_t          request_id = 0;
      string            memo;
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

   struct return_ESCOR_delegation_operation : public virtual_operation
   {
      return_ESCOR_delegation_operation() {}
      return_ESCOR_delegation_operation( const account_name_type& a, const asset& v ) : account( a ), ESCOR( v ) {}

      account_name_type account;
      asset             ESCOR;
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
      producer_reward_operation( const string& p, const asset& v ) : producer( p ), ESCOR( v ) {}

      account_name_type producer;
      asset             ESCOR;

   };

} } //eznode::protocol

FC_REFLECT( eznode::protocol::authorReward_operation, (author)(permlink)(EUSDpayout)(ECOpayout)(ESCORpayout) )
FC_REFLECT( eznode::protocol::curationReward_operation, (curator)(reward)(comment_author)(comment_permlink) )
FC_REFLECT( eznode::protocol::comment_reward_operation, (author)(permlink)(payout) )
FC_REFLECT( eznode::protocol::fill_convert_request_operation, (owner)(requestid)(amount_in)(amount_out) )
FC_REFLECT( eznode::protocol::liquidity_reward_operation, (owner)(payout) )
FC_REFLECT( eznode::protocol::interest_operation, (owner)(interest) )
FC_REFLECT( eznode::protocol::fillESCORWithdraw_operation, (from_account)(to_account)(withdrawn)(deposited) )
FC_REFLECT( eznode::protocol::shutdown_witness_operation, (owner) )
FC_REFLECT( eznode::protocol::fill_order_operation, (current_owner)(current_orderid)(current_pays)(open_owner)(open_orderid)(open_pays) )
FC_REFLECT( eznode::protocol::fill_transferFromSavings_operation, (from)(to)(amount)(request_id)(memo) )
FC_REFLECT( eznode::protocol::hardfork_operation, (hardfork_id) )
FC_REFLECT( eznode::protocol::comment_payout_update_operation, (author)(permlink) )
FC_REFLECT( eznode::protocol::return_ESCOR_delegation_operation, (account)(ESCOR) )
FC_REFLECT( eznode::protocol::comment_benefactor_reward_operation, (benefactor)(author)(permlink)(reward) )
FC_REFLECT( eznode::protocol::producer_reward_operation, (producer)(ESCOR) )