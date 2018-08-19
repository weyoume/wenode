#include <eznode/blockchain_statistics/blockchain_statistics_api.hpp>

namespace eznode { namespace blockchain_statistics {

namespace detail
{
   class blockchain_statistics_api_impl
   {
      public:
         blockchain_statistics_api_impl( eznode::app::application& app )
            :_app( app ) {}

         statistics get_stats_for_time( fc::time_point_sec open, uint32_t interval )const;
         statistics get_stats_for_interval( fc::time_point_sec start, fc::time_point_sec end )const;
         statistics get_lifetime_stats()const;

         eznode::app::application& _app;
   };

   statistics blockchain_statistics_api_impl::get_stats_for_time( fc::time_point_sec open, uint32_t interval )const
   {
      statistics result;
      const auto& bucket_idx = _app.chain_database()->get_index< bucket_index >().indices().get< by_bucket >();
      auto itr = bucket_idx.lower_bound( boost::make_tuple( interval, open ) );

      if( itr != bucket_idx.end() )
         result += *itr;

      return result;
   }

   statistics blockchain_statistics_api_impl::get_stats_for_interval( fc::time_point_sec start, fc::time_point_sec end )const
   {
      statistics result;
      const auto& bucket_itr = _app.chain_database()->get_index< bucket_index >().indices().get< by_bucket >();
      const auto& sizes = _app.get_plugin< blockchain_statistics_plugin >( BLOCKCHAIN_STATISTICS_PLUGIN_NAME )->get_tracked_buckets();
      auto size_itr = sizes.rbegin();
      auto time = start;

      // This is a greedy algorithm, same as the ubiquitous "making change" problem.
      // So long as the bucket sizes share a common denominator, the greedy solution
      // has the same efficiency as the dynamic solution.
      while( size_itr != sizes.rend() && time < end )
      {
         auto itr = bucket_itr.find( boost::make_tuple( *size_itr, time ) );

         while( itr != bucket_itr.end() && itr->seconds == *size_itr && time + itr->seconds <= end )
         {
            time += *size_itr;
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

blockchain_statistics_api::blockchain_statistics_api( const eznode::app::api_context& ctx )
{
   my = std::make_shared< detail::blockchain_statistics_api_impl >( ctx.app );
}

void blockchain_statistics_api::on_api_startup() {}

statistics blockchain_statistics_api::get_stats_for_time( fc::time_point_sec open, uint32_t interval )const
{
   return my->_app.chain_database()->with_read_lock( [&]()
   {
      return my->get_stats_for_time( open, interval );
   });
}

statistics blockchain_statistics_api::get_stats_for_interval( fc::time_point_sec start, fc::time_point_sec end )const
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
   this->ECO_transferred                        += b.ECO_transferred;
   this->EUSD_transferred                        += b.EUSD_transferred;
   this->EUSD_paid_as_interest                   += b.EUSD_paid_as_interest;
   this->accounts_created                       += b.paid_accounts_created + b.mined_accounts_created;
   this->paid_accounts_created                  += b.paid_accounts_created;
   this->mined_accounts_created                 += b.mined_accounts_created;
   this->total_comments                         += b.root_comments + b.replies;
   this->total_comment_edits                    += b.root_comment_edits + b.reply_edits;
   this->total_comments_deleted                 += b.root_comments_deleted + b.replies_deleted;
   this->root_comments                          += b.root_comments;
   this->root_comment_edits                     += b.root_comment_edits;
   this->root_comments_deleted                  += b.root_comments_deleted;
   this->replies                                += b.replies;
   this->reply_edits                            += b.reply_edits;
   this->replies_deleted                        += b.replies_deleted;
   this->total_votes                            += b.new_root_votes + b.changed_root_votes + b.new_reply_votes + b.changed_reply_votes;
   this->new_votes                              += b.new_root_votes + b.new_reply_votes;
   this->changed_votes                          += b.changed_root_votes + b.changed_reply_votes;
   this->total_root_votes                       += b.new_root_votes + b.changed_root_votes;
   this->new_root_votes                         += b.new_root_votes;
   this->changed_root_votes                     += b.changed_root_votes;
   this->total_reply_votes                      += b.new_reply_votes + b.changed_reply_votes;
   this->new_reply_votes                        += b.new_reply_votes;
   this->changed_reply_votes                    += b.changed_reply_votes;
   this->payouts                                += b.payouts;
   this->EUSD_paid_to_authors                    += b.EUSD_paid_to_authors;
   this->ESCOR_paid_to_authors                  += b.ESCOR_paid_to_authors;
   this->ESCOR_paid_to_curators                 += b.ESCOR_paid_to_curators;
   this->liquidity_rewards_paid                 += b.liquidity_rewards_paid;
   this->transfers_to_ECO_fund_for_ESCOR                   += b.transfers_to_ECO_fund_for_ESCOR;
   this->ECO_value_of_ESCOR                           += b.ECO_value_of_ESCOR;
   this->new_ESCOR_ECO_fund_withdrawal_requests        += b.new_ESCOR_ECO_fund_withdrawal_requests;
   this->ESCORwithdrawRateInECO_delta            += b.ESCORwithdrawRateInECO_delta;
   this->modified_ESCOR_ECO_fund_withdrawal_requests   += b.modified_ESCOR_ECO_fund_withdrawal_requests;
   this->ECO_fund_for_ESCOR_withdrawals_processed          += b.ECO_fund_for_ESCOR_withdrawals_processed;
   this->finished_ECO_fund_for_ESCOR_withdrawals           += b.finished_ECO_fund_for_ESCOR_withdrawals;
   this->ESCOR_withdrawn                        += b.ESCOR_withdrawn;
   this->ESCOR_transferred                      += b.ESCOR_transferred;
   this->EUSD_conversion_requests_created        += b.EUSD_conversion_requests_created;
   this->EUSD_to_be_converted                    += b.EUSD_to_be_converted;
   this->EUSD_conversion_requests_filled         += b.EUSD_conversion_requests_filled;
   this->ECO_converted                        += b.ECO_converted;
   this->limit_orders_created                   += b.limit_orders_created;
   this->limit_orders_filled                    += b.limit_orders_filled;
   this->limit_orders_cancelled                 += b.limit_orders_cancelled;
   this->total_pow                              += b.total_pow;
   this->estimated_hashpower                    += b.estimated_hashpower;

   return ( *this );
}

} } // eznode::blockchain_statistics
