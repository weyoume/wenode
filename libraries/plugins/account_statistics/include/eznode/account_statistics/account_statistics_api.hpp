#pragma once

#include <eznode/account_statistics/account_statistics_plugin.hpp>

#include <fc/api.hpp>

namespace eznode{ namespace app {
   struct api_context;
} }

namespace eznode { namespace account_statistics {

namespace detail
{
   class account_statistics_api_impl;
}

class account_statistics_api
{
   public:
      account_statistics_api( const eznode::app::api_context& ctx );

      void on_api_startup();

   private:
      std::shared_ptr< detail::account_statistics_api_impl > _my;
};

} } // eznode::account_statistics

FC_API( eznode::account_statistics::account_statistics_api, )