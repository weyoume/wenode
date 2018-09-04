#pragma once
#include <steem/protocol/base.hpp>
#include <steem/protocol/block_header.hpp>
#include <steem/protocol/asset.hpp>
#include <steem/protocol/validation.hpp>
#include <steem/protocol/legacy_asset.hpp>

#include <fc/crypto/equihash.hpp>

namespace steem { namespace protocol {

   typedef base_operation base_action;

   struct required_action : public base_action
   {
      bool is_required()const { return true; }
   };

   struct optional_action : public base_action
   {
      bool is_required()const { return false; }
   };

   struct example_required_action : public required_action
   {
      account_name_type account;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert(account); }
   };

   struct example_optional_action : public optional_action
   {
      account_name_type account;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert(account); }
   };

} } // steem::protocol

FC_REFLECT( steem::protocol::example_required_action, (account) )
FC_REFLECT( steem::protocol::example_optional_action, (account) )