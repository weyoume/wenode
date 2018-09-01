#include <node/account_statistics/account_statistics_api.hpp>

namespace node { namespace account_statistics {

namespace detail
{
   class account_statistics_api_impl
   {
      public:
         account_statistics_api_impl( node::app::application& app )
            :_app( app ) {}

         node::app::application& _app;
   };
} // detail

account_statistics_api::account_statistics_api( const node::app::api_context& ctx )
{
   _my= std::make_shared< detail::account_statistics_api_impl >( ctx.app );
}

void account_statistics_api::on_api_startup() {}

} } // node::account_statistics