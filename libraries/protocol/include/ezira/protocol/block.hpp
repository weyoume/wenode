#pragma once
#include <ezira/protocol/block_header.hpp>
#include <ezira/protocol/transaction.hpp>

namespace ezira { namespace protocol {

   struct signed_block : public signed_block_header
   {
      checksum_type calculate_merkle_root()const;
      vector<signed_transaction> transactions;
   };

} } // ezira::protocol

FC_REFLECT_DERIVED( ezira::protocol::signed_block, (ezira::protocol::signed_block_header), (transactions) )
