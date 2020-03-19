#pragma once

#include <node/blockchain_statistics/blockchain_statistics_plugin.hpp>

#include <fc/api.hpp>

namespace node { namespace app {
   struct api_context;
} }

namespace node { namespace blockchain_statistics {

namespace detail
{
   class blockchain_statistics_api_impl;
}

struct statistics
{
   uint64_t             blocks = 0;                                  ///< Blocks produced

   uint64_t             bandwidth = 0;                               ///< Bandwidth in bytes

   uint64_t             operations = 0;                              ///< Operations evaluated

   uint64_t             transactions = 0;                            ///< Transactions processed

   uint64_t             transfers = 0;                               ///< Account to account transfers

   uint64_t             accounts_created = 0;                        ///< Accounts created

   uint64_t             communities_created = 0;                     ///< Communities created

   uint64_t             assets_created = 0;                          ///< Assets created

   uint64_t             root_comments = 0;                           ///< Top level root comments

   uint64_t             replies = 0;                                 ///< Replies to comments

   uint64_t             comment_votes = 0;                           ///< Votes on comments

   uint64_t             comment_views = 0;                           ///< Views on comments

   uint64_t             comment_shares = 0;                          ///< Shares on comments

   uint64_t             activity_rewards = 0;                        ///< Activity rewards claimed    
   
   uint64_t             total_pow = 0;                               ///< POW submitted

   uint128_t            estimated_hashpower = 0;                     ///< Estimated average hashpower over interval

   uint64_t             ad_bids_created = 0;                         ///< Ad bids

   uint64_t             limit_orders = 0;                            ///< Trading limit orders

   uint64_t             margin_orders = 0;                           ///< Trading margin orders

   uint64_t             auction_orders = 0;                          ///< Trading auction orders

   uint64_t             call_orders = 0;                             ///< Trading call orders

   uint64_t             option_orders = 0;                           ///< Trading option orders

   statistics& operator += ( const bucket_object& b );
};

class blockchain_statistics_api
{
   public:
      blockchain_statistics_api( const node::app::api_context& ctx );

      void on_api_startup();

      /**
      * @brief Gets statistics over the time window length, interval, that contains time, open.
      * @param open The opening time, or a time contained within the window.
      * @param interval The size of the window for which statistics were aggregated.
      * @returns Statistics for the window.
      */
      statistics get_stats_for_time( fc::time_point open, uint32_t interval )const;

      /**
      * @brief Aggregates statistics over a time interval.
      * @param start The beginning time of the window.
      * @param stop The end time of the window. stop must take place after start.
      * @returns Aggregated statistics over the interval.
      */
      statistics get_stats_for_interval( fc::time_point start, fc::time_point end )const;

      /**
       * @brief Returns lifetime statistics.
       */
      statistics get_lifetime_stats()const;

   private:
      std::shared_ptr< detail::blockchain_statistics_api_impl > my;
};

} } // node::blockchain_statistics

FC_REFLECT( node::blockchain_statistics::statistics,
         (blocks)
         (bandwidth)
         (operations)
         (transactions)
         (transfers)
         (accounts_created)
         (communities_created)
         (assets_created)
         (root_comments)
         (replies)
         (comment_votes)
         (comment_views)
         (comment_shares)
         (activity_rewards)  
         (total_pow)
         (estimated_hashpower)
         (ad_bids_created)
         (limit_orders)
         (margin_orders)
         (auction_orders)
         (call_orders)
         (option_orders)
         );

FC_API( node::blockchain_statistics::blockchain_statistics_api,
         (get_stats_for_time)
         (get_stats_for_interval)
         (get_lifetime_stats)
         );