
#pragma once

#include <eznode/chain/eznode_object_types.hpp>

#include <fc/api.hpp>

namespace eznode { namespace app {
   struct api_context;
} }

namespace eznode { namespace plugin { namespace raw_block {

namespace detail {
class raw_block_api_impl;
}

struct get_raw_block_args
{
   uint32_t block_num = 0;
};

struct get_raw_block_result
{
   chain::block_id_type          block_id;
   chain::block_id_type          previous;
   fc::time_point_sec            timestamp;
   std::string                   raw_block;
};

class raw_block_api
{
   public:
      raw_block_api( const eznode::app::api_context& ctx );

      void on_api_startup();

      get_raw_block_result get_raw_block( get_raw_block_args args );
      void push_raw_block( std::string block_b64 );

   private:
      std::shared_ptr< detail::raw_block_api_impl > my;
};

} } }

FC_REFLECT( eznode::plugin::raw_block::get_raw_block_args,
   (block_num)
   )

FC_REFLECT( eznode::plugin::raw_block::get_raw_block_result,
   (block_id)
   (previous)
   (timestamp)
   (raw_block)
   )

FC_API( eznode::plugin::raw_block::raw_block_api,
   (get_raw_block)
   (push_raw_block)
   )
