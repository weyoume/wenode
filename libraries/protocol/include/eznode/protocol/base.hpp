#pragma once

#include <eznode/protocol/types.hpp>
#include <eznode/protocol/authority.hpp>
#include <eznode/protocol/version.hpp>

#include <fc/time.hpp>

namespace ezira { namespace protocol {

   struct base_operation
   {
      void get_required_authorities( vector<authority>& )const {}
      void get_required_active_authorities( flat_set<account_name_type>& )const {}
      void get_required_posting_authorities( flat_set<account_name_type>& )const {}
      void get_required_owner_authorities( flat_set<account_name_type>& )const {}

      bool is_virtual()const { return false; }
      void validate()const {}
   };

   struct virtual_operation : public base_operation
   {
      bool is_virtual()const { return true; }
      void validate()const { FC_ASSERT( false, "This is a virtual operation" ); }
   };

   typedef static_variant<
      void_t,
      version,              // Normal witness version reporting, for diagnostics and voting
      hardfork_version_vote // Voting for the next hardfork to trigger
      >                                block_header_extensions;

   typedef static_variant<
      void_t
      >                                future_extensions;

   typedef flat_set<block_header_extensions > block_header_extensions_type;
   typedef flat_set<future_extensions> extensions_type;


} } // eznode::protocol

FC_REFLECT_TYPENAME( eznode::protocol::block_header_extensions )
FC_REFLECT_TYPENAME( eznode::protocol::future_extensions )
