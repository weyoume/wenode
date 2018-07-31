#pragma once

#include <ezira/account_statistics/account_statistics_plugin.hpp>

#include <fc/api.hpp>

namespace ezira{ namespace app {
   struct api_context;
} }

namespace ezira { namespace account_statistics {

namespace detail
{
   class account_statistics_api_impl;
}

class account_statistics_api
{
   public:
      account_statistics_api( const ezira::app::api_context& ctx );

      void on_api_startup();

   private:
      std::shared_ptr< detail::account_statistics_api_impl > _my;
};

} } // ezira::account_statistics

FC_API( ezira::account_statistics::account_statistics_api, )