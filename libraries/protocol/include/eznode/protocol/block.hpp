#pragma once
#include <eznode/protocol/block_header.hpp>
#include <eznode/protocol/transaction.hpp>

namespace ezira { namespace protocol {

   struct signed_block : public signed_block_header
   {
      checksum_type calculate_merkle_root()const;
      vector<signed_transaction> transactions;
   };

} } // eznode::protocol

FC_REFLECT_DERIVED( eznode::protocol::signed_block, (eznode::protocol::signed_block_header), (transactions) )
