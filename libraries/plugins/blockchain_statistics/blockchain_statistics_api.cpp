#include <node/blockchain_statistics/blockchain_statistics_api.hpp>

namespace node { namespace blockchain_statistics {

namespace detail
{
   class blockchain_statistics_api_impl
   {
      public:
         blockchain_statistics_api_impl( node::app::application& app )
            :_app( app ) {}

         statistics get_stats_for_time( fc::time_point open, uint32_t interval )const;
         statistics get_stats_for_interval( fc::time_point start, fc::time_point end )const;
         statistics get_lifetime_stats()const;

         node::app::application& _app;
   };

   statistics blockchain_statistics_api_impl::get_stats_for_time( fc::time_point open, uint32_t interval )const
   {
      statistics result;
      const auto& bucket_idx = _app.chain_database()->get_index< bucket_index >().indices().get< by_bucket >();
      auto itr = bucket_idx.lower_bound( boost::make_tuple( interval, open ) );

      if( itr != bucket_idx.end() )
         result += *itr;

      return result;
   }

   statistics blockchain_statistics_api_impl::get_stats_for_interval( fc::time_point start, fc::time_point end )const
   {
      statistics result;
      const auto& bucket_itr = _app.chain_database()->get_index< bucket_index >().indices().get< by_bucket >();
      const auto& sizes = _app.get_plugin< blockchain_statistics_plugin >( BLOCKCHAIN_STATISTICS_PLUGIN_NAME )->get_tracked_buckets();
      auto size_itr = sizes.rbegin();
      time_point time = start;

      // This is a greedy algorithm, same as the ubiquitous "making change" problem.
      // So long as the bucket sizes share a common denominator, the greedy solution
      // has the same efficiency as the dynamic solution.
      while( size_itr != sizes.rend() && time < end )
      {
         auto itr = bucket_itr.find( boost::make_tuple( *size_itr, time ) );

         while( itr != bucket_itr.end() && 
            itr->seconds == *size_itr && 
            ( time + fc::seconds( itr->seconds ) ) <= end )
         {
            time += fc::seconds( *size_itr );
            result += *itr;
            itr++;
         }

         size_itr++;
      }

      return result;
   }

   statistics blockchain_statistics_api_impl::get_lifetime_stats()const
   {
      statistics result;
      result += _app.chain_database()->get( bucket_id_type() );

      return result;
   }
} // detail

blockchain_statistics_api::blockchain_statistics_api( const node::app::api_context& ctx )
{
   my = std::make_shared< detail::blockchain_statistics_api_impl >( ctx.app );
}

void blockchain_statistics_api::on_api_startup() {}

statistics blockchain_statistics_api::get_stats_for_time( fc::time_point open, uint32_t interval )const
{
   return my->_app.chain_database()->with_read_lock( [&]()
   {
      return my->get_stats_for_time( open, interval );
   });
}

statistics blockchain_statistics_api::get_stats_for_interval( fc::time_point start, fc::time_point end )const
{
   return my->_app.chain_database()->with_read_lock( [&]()
   {
      return my->get_stats_for_interval( start, end );
   });
}

statistics blockchain_statistics_api::get_lifetime_stats()const
{
   return my->_app.chain_database()->with_read_lock( [&]()
   {
      return my->get_lifetime_stats();
   });
}

statistics& statistics::operator +=( const bucket_object& b )
{
   this->blocks                                 += b.blocks;
   this->bandwidth                              += b.bandwidth;
   this->operations                             += b.operations;
   this->transactions                           += b.transactions;
   this->transfers                              += b.transfers;
   this->accounts_created                       += b.accounts_created;
   this->communities_created                    += b.communities_created;
   this->assets_created                         += b.assets_created;
   this->root_comments                          += b.root_comments;
   this->replies                                += b.replies;
   this->comment_votes                          += b.comment_votes;
   this->comment_views                          += b.comment_views;
   this->comment_shares                         += b.comment_shares;
   this->activity_rewards                       += b.activity_rewards;
   this->total_pow                              += b.total_pow;
   this->estimated_hashpower                    += b.estimated_hashpower;
   this->ad_bids_created                        += b.ad_bids_created;
   this->limit_orders                           += b.limit_orders;
   this->margin_orders                          += b.margin_orders;
   this->auction_orders                         += b.auction_orders;
   this->call_orders                            += b.call_orders;
   this->option_orders                          += b.option_orders;

   return ( *this );
}

} } // node::blockchain_statistics