#pragma once
#include <node/protocol/block_header.hpp>
#include <node/protocol/transaction.hpp>

namespace node { namespace protocol {

   struct signed_block : public signed_block_header
   {
      checksum_type calculate_merkle_root()const;
      vector<signed_transaction> transactions;
   };

} } // node::protocol

FC_REFLECT_DERIVED( node::protocol::signed_block, (node::protocol::signed_block_header), (transactions) )
