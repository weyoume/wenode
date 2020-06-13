#pragma once
#include <node/protocol/base.hpp>
#include <node/protocol/block_header.hpp>
#include <node/protocol/asset.hpp>

#include <fc/utf8.hpp>

namespace node { namespace protocol {

   /**
    * Measures distributed currency reward value denominated in USD for each post.
    */
   struct content_reward_operation : public virtual_operation 
   {
      content_reward_operation() {}
      content_reward_operation( 
         const account_name_type& a, 
         const string& p, 
         const asset& r ) : 
         post_author(a), 
         post_permlink(p), 
         reward_usd_value(r) {}

      account_name_type      post_author;

      string                 post_permlink;

      asset                  reward_usd_value;               ///< Denominated in USD

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( post_author ); }
   };


   /**
    * Measures distributed currency rewards to the account that is the author of a post.
    */
   struct author_reward_operation : public virtual_operation 
   {
      author_reward_operation() {}
      author_reward_operation( 
         const account_name_type& a, 
         const string& p, 
         const asset& r ) : 
         post_author(a), 
         post_permlink(p), 
         reward(r) {}

      account_name_type      post_author;

      string                 post_permlink;

      asset                  reward;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( post_author ); }
   };


   /**
    * Measures distributed currency rewards to accounts that vote on a post.
    */
   struct vote_reward_operation : public virtual_operation
   {
      vote_reward_operation() {}
      vote_reward_operation( 
         const string& v,  
         const string& a, 
         const string& p,
         const asset& r ) :
         voter(v),  
         post_author(a), 
         post_permlink(p),
         reward(r) {}

      account_name_type      voter;

      account_name_type      post_author;

      string                 post_permlink;

      asset                  reward;

      

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( voter ); }
   };


   /**
    * Measures distributed currency rewards to accounts that view a post.
    */
   struct view_reward_operation : public virtual_operation
   {
      view_reward_operation() {}
      view_reward_operation(
         const string& v, 
         const string& a, 
         const string& p,
         const asset& r ) :
         viewer(v),  
         post_author(a), 
         post_permlink(p),
         reward(r) {}

      account_name_type      viewer;

      account_name_type      post_author;

      string                 post_permlink;

      asset                  reward;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( viewer ); }
   };


   /**
    * Measures distributed currency rewards to the accounts that share a post.
    */
   struct share_reward_operation : public virtual_operation
   {
      share_reward_operation() {}
      share_reward_operation(
         const string& v, 
         const string& a, 
         const string& p,
         const asset& r ) :
         sharer(v),  
         post_author(a), 
         post_permlink(p),
         reward(r) {}

      account_name_type      sharer;

      account_name_type      post_author;

      string                 post_permlink;

      asset                  reward;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( sharer ); }
   };


   /**
    * Measures distributed currency rewards to creators of comments on a post.
    */
   struct comment_reward_operation : public virtual_operation
   {
      comment_reward_operation(){}
      comment_reward_operation( 
         const account_name_type& ca, 
         const string& cpl, 
         const account_name_type& pa, 
         const string& ppl, 
         const asset& r ) : 
         comment_author(ca),
         comment_permlink(cpl),
         post_author(pa),
         post_permlink(ppl),
         reward(r) {}

      account_name_type       comment_author;

      string                  comment_permlink;

      account_name_type       post_author;

      string                  post_permlink;

      asset                   reward;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( comment_author ); }
   };


   /**
    * Measures distributed currency rewards to the supernodes 
    * nominated as file hosts for viewers of post.
    */
   struct supernode_reward_operation : public virtual_operation
   {
      supernode_reward_operation() {}
      supernode_reward_operation(
         const string& s, 
         const string& a, 
         const string& p,
         const asset& r ) :
         supernode(s),  
         post_author(a), 
         post_permlink(p),
         reward(r) {}

      account_name_type      supernode;

      account_name_type      post_author;

      string                 post_permlink;

      asset                  reward;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( supernode ); }
   };


   /**
    * Measures distributed currency rewards to the moderators 
    * of the community that a post is created within.
    */
   struct moderation_reward_operation : public virtual_operation
   {
      moderation_reward_operation() {}
      moderation_reward_operation(
         const string& m, 
         const string& a, 
         const string& p,
         const asset& r ) :
         moderator(m),  
         post_author(a), 
         post_permlink(p),
         reward(r) {}

      account_name_type      moderator;

      account_name_type      post_author;

      string                 post_permlink;

      asset                  reward;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( moderator ); }
   };


   struct comment_payout_update_operation : public virtual_operation
   {
      comment_payout_update_operation() {}
      comment_payout_update_operation( 
         const account_name_type& a, 
         const string& p ) : 
         author( a ), 
         permlink( p ) {}

      account_name_type     author;

      string                permlink;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( author ); }
   };


   struct comment_benefactor_reward_operation : public virtual_operation
   {
      comment_benefactor_reward_operation() {}
      comment_benefactor_reward_operation( 
         const account_name_type& b, 
         const account_name_type& a, 
         const string& p, 
         const asset& r ) : 
         benefactor( b ), 
         author( a ), 
         permlink( p ), 
         reward( r ) {}

      account_name_type     benefactor;

      account_name_type     author;

      string                permlink;

      asset                 reward;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( benefactor ); }
   };


   struct interest_operation : public virtual_operation
   {
      interest_operation( const string& o = "", const asset& i = asset(0,SYMBOL_USD) )
         :owner(o),interest(i){}

      account_name_type        owner;

      asset                    interest;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( owner ); }
   };


   struct shutdown_producer_operation : public virtual_operation
   {
      shutdown_producer_operation(){}
      shutdown_producer_operation( const string& o ):owner(o) {}

      account_name_type    owner;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( owner ); }
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

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( current_owner ); }
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

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };


   struct hardfork_operation : public virtual_operation
   {
      hardfork_operation() {}
      hardfork_operation( uint32_t hf_id ) : hardfork_id( hf_id ) {}

      uint32_t         hardfork_id = 0;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( NULL_ACCOUNT ); }
   };


   struct return_asset_delegation_operation : public virtual_operation
   {
      return_asset_delegation_operation() {}
      return_asset_delegation_operation( 
         const account_name_type& a, 
         const asset& v ) : 
         account( a ), 
         amount( v ) {}

      account_name_type      account;

      asset                  amount;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( account ); }
   };


   struct producer_reward_operation : public virtual_operation
   {
      producer_reward_operation(){}
      producer_reward_operation( 
         const string& p, 
         const asset& r ) : 
         producer( p ), 
         reward( r ) {}

      account_name_type     producer;

      asset                 reward;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( producer ); }
   };


   struct execute_bid_operation : public virtual_operation
   {
      execute_bid_operation(){}
      execute_bid_operation( 
         account_name_type a, 
         asset d, 
         asset c ) : 
         bidder(a), 
         debt(d), 
         collateral(c) {}

      account_name_type   bidder;

      asset               debt;

      asset               collateral;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( bidder ); }
   };

   struct update_featured_feed_operation : public virtual_operation
   {
      update_featured_feed_operation() {}
      update_featured_feed_operation( 
         time_point featured_time ) : 
         featured_time( featured_time ) {}

      time_point       featured_time;

      void get_creator_name( flat_set<account_name_type>& a )const{ a.insert( NULL_ACCOUNT ); }
   };


} } //node::protocol

FC_REFLECT( node::protocol::content_reward_operation,
         (post_author)
         (post_permlink)
         (reward_usd_value) 
         );

FC_REFLECT( node::protocol::author_reward_operation,
         (post_author)
         (post_permlink)
         (reward) 
         );

FC_REFLECT( node::protocol::vote_reward_operation,
         (voter)
         (post_author)
         (post_permlink)
         (reward)
         );

FC_REFLECT( node::protocol::view_reward_operation,
         (viewer)
         (post_author)
         (post_permlink)
         (reward)
         );

FC_REFLECT( node::protocol::share_reward_operation,
         (sharer)
         (post_author)
         (post_permlink)
         (reward)
         );

FC_REFLECT( node::protocol::comment_reward_operation,
         (post_author)
         (post_permlink)
         (comment_author)
         (comment_permlink)
         (reward) 
         );

FC_REFLECT( node::protocol::supernode_reward_operation,
         (supernode)
         (post_author)
         (post_permlink)
         (reward)
         );

FC_REFLECT( node::protocol::moderation_reward_operation,
         (moderator)
         (post_author)
         (post_permlink)
         (reward)
         );

FC_REFLECT( node::protocol::comment_payout_update_operation,
         (author)
         (permlink) 
         );

FC_REFLECT( node::protocol::comment_benefactor_reward_operation,
         (benefactor)
         (author)
         (permlink)
         (reward) 
         );

FC_REFLECT( node::protocol::interest_operation,
         (owner)
         (interest) 
         );

FC_REFLECT( node::protocol::shutdown_producer_operation,
         (owner) 
         );

FC_REFLECT( node::protocol::fill_order_operation,
         (current_owner)
         (current_order_id)
         (current_pays)
         (open_owner)
         (open_order_id)
         (open_pays)
         (symbol_a)
         (symbol_b)
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

FC_REFLECT( node::protocol::return_asset_delegation_operation,
         (account)
         (amount) 
         );

FC_REFLECT( node::protocol::producer_reward_operation,
         (producer)
         (reward) 
         );

FC_REFLECT( node::protocol::execute_bid_operation,
         (bidder)
         (debt)
         (collateral)
         );

FC_REFLECT( node::protocol::update_featured_feed_operation,
         (featured_time)
         );
