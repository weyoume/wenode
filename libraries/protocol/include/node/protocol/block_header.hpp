#pragma once
#include <node/protocol/base.hpp>

namespace node { namespace protocol {

   struct block_header
   {
      digest_type                   digest()const;
      block_id_type                 previous;
      uint64_t                      block_num()const { return num_from_id(previous) + 1; }
      fc::time_point                timestamp;
      string                        producer;
      checksum_type                 transaction_merkle_root;
      block_header_extensions_type  extensions;

      static uint64_t num_from_id(const block_id_type& id);
   };

   struct signed_block_header : public block_header
   {
      block_id_type              id()const;
      fc::ecc::public_key        signee()const;
      void                       sign( const fc::ecc::private_key& signer );
      bool                       validate_signee( const fc::ecc::public_key& expected_signee )const;

      signature_type             producer_signature;
   };


} } // node::protocol

FC_REFLECT( node::protocol::block_header, (previous)(timestamp)(producer)(transaction_merkle_root)(extensions) )
FC_REFLECT_DERIVED( node::protocol::signed_block_header, (node::protocol::block_header), (producer_signature) )
