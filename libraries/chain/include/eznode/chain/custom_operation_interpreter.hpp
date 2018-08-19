
#pragma once

#include <memory>

namespace graphene { namespace schema {
   struct abstract_schema;
} }

namespace eznode { namespace protocol {
   struct customJson_operation;
} }

namespace eznode { namespace chain {

class custom_operation_interpreter
{
   public:
      virtual void apply( const protocol::customJson_operation& op ) = 0;
      virtual void apply( const protocol::custom_binary_operation & op ) = 0;
      virtual std::shared_ptr< graphene::schema::abstract_schema > get_operation_schema() = 0;
};

} } // eznode::chain
