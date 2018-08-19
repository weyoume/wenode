
#pragma once

#include <eznode/protocol/base.hpp>
#include <eznode/protocol/types.hpp>

#include <fc/reflect/reflect.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace eznode { namespace private_message {

struct private_message_operation : public eznode::protocol::base_operation
{
    protocol::account_name_type  from;
    protocol::account_name_type  to;
    protocol::public_key_type    from_memoKey;
    protocol::public_key_type    to_memoKey;
    uint64_t                     sent_time = 0; /// used as seed to secret generation
    uint32_t                     checksum = 0;
    std::vector<char>            encrypted_message;
};

typedef fc::static_variant< private_message_operation > private_message_plugin_operation;

} }

FC_REFLECT( eznode::private_message::private_message_operation, (from)(to)(from_memoKey)(to_memoKey)(sent_time)(checksum)(encrypted_message) )

DECLARE_OPERATION_TYPE( eznode::private_message::private_message_plugin_operation )
FC_REFLECT_TYPENAME( eznode::private_message::private_message_plugin_operation )
