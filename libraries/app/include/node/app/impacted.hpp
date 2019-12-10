
#pragma once

#include <fc/container/flat.hpp>
#include <node/protocol/operations.hpp>
#include <node/protocol/transaction.hpp>
#include <node/chain/node_object_types.hpp>

#include <fc/string.hpp>

namespace node { namespace app {

using namespace fc;

void operation_get_impacted_accounts(
   const node::protocol::operation& op,
   fc::flat_set<protocol::account_name_type>& result );

void transaction_get_impacted_accounts(
   const node::protocol::transaction& tx,
   fc::flat_set<protocol::account_name_type>& result
   );

} } // node::app
