#include <node/blockchain_statistics/blockchain_statistics_api.hpp>

#include <node/app/impacted.hpp>
#include <node/chain/account_object.hpp>
#include <node/chain/comment_object.hpp>
#include <node/chain/history_object.hpp>

#include <node/chain/database.hpp>
#include <node/chain/index.hpp>
#include <node/chain/operation_notification.hpp>

namespace node { namespace blockchain_statistics {

namespace detail
{

using namespace node::protocol;

class blockchain_statistics_plugin_impl
{
   public:
      blockchain_statistics_plugin_impl( blockchain_statistics_plugin& plugin )
         :_self( plugin ) {}
      virtual ~blockchain_statistics_plugin_impl() {}

      void on_block( const signed_block& b );
      void pre_operation( const operation_notification& o );
      void post_operation( const operation_notification& o );

      blockchain_statistics_plugin&       _self;
      flat_set< uint64_t >                _tracked_buckets = { 60, 3600, 21600, 86400, 604800, 2592000 };
      flat_set< bucket_id_type >          _current_buckets;
      uint64_t                            _maximum_history_per_bucket_size = 100;
};

struct operation_process
{
   const blockchain_statistics_plugin& _plugin;
   const bucket_object&                _bucket;
   chain::database&                    _db;

   operation_process( blockchain_statistics_plugin& bsp, const bucket_object& b )
      :_plugin( bsp ), _bucket( b ), _db( bsp.database() ) {}

   typedef void result_type;

   template< typename T >
   void operator()( const T& )const {}

   void operator()( const transfer_operation& op )const
   {
      _db.modify( _bucket, [&]( bucket_object& b )
      {
         b.transfers++;
      });
   }

   void operator()( const account_create_operation& op )const
   {
      _db.modify( _bucket, [&]( bucket_object& b )
      {
         b.accounts_created++;
      });
   }

   void operator()( const community_create_operation& op )const
   {
      _db.modify( _bucket, [&]( bucket_object& b )
      {
         b.communities_created++;
      });
   }

   void operator()( const asset_create_operation& op )const
   {
      _db.modify( _bucket, [&]( bucket_object& b )
      {
         b.assets_created++;
      });
   }

   void operator()( const activity_reward_operation& op )const
   {
      _db.modify( _bucket, [&]( bucket_object& b )
      {
         b.activity_rewards++;
      });
   }

   void operator()( const proof_of_work_operation& op )const
   {
      _db.modify( _bucket, [&]( bucket_object& b )
      {
         const account_object& worker = _db.get_account( op.work.get< x11_proof_of_work >().input.miner_account );

         if( worker.created == _db.head_block_time() )
         {
            b.accounts_created++;
         }
            
         uint128_t target = _db.pow_difficulty();
         uint128_t estimated_hashes = uint128_t::max_value() / target;
         uint32_t delta_t;

         if( b.seconds == 0 )
         {
            delta_t = _db.head_block_time().sec_since_epoch() - b.open.sec_since_epoch();
         }
         else
         {
            delta_t = b.seconds;
         }
         	
         b.estimated_hashpower = ( b.estimated_hashpower * delta_t + estimated_hashes ) / delta_t;
         b.total_pow++;
      });
   }

   void operator()( const comment_operation& op )const
   {
      _db.modify( _bucket, [&]( bucket_object& b )
      {
         const comment_object& comment = _db.get_comment( op.author, op.permlink );

         if( comment.created == _db.head_block_time() )
         {
            if( comment.parent_author.length() )
            {
               b.replies++;
            }
            else
            {
               b.root_comments++;
            }
         }
      });
   }

   void operator()( const vote_operation& op )const
   {
      _db.modify( _bucket, [&]( bucket_object& b )
      {
         const auto& cv_idx = _db.get_index< comment_vote_index >().indices().get< by_comment_voter >();
         const comment_object& comment = _db.get_comment( op.author, op.permlink );
         const account_object& voter = _db.get_account( op.voter );
         auto cv_itr = cv_idx.find( boost::make_tuple( comment.id, voter.name ) );

         if( cv_itr->created == _db.head_block_time() )
         {
            b.comment_votes++;
         }
      });
   }

   void operator()( const view_operation& op )const
   {
      _db.modify( _bucket, [&]( bucket_object& b )
      {
         const auto& cv_idx = _db.get_index< comment_view_index >().indices().get< by_comment_viewer >();
         const comment_object& comment = _db.get_comment( op.author, op.permlink );
         const account_object& viewer = _db.get_account( op.viewer );
         auto cv_itr = cv_idx.find( boost::make_tuple( comment.id, viewer.name ) );

         if( cv_itr->created == _db.head_block_time() )
         {
            b.comment_views++;
         }
      });
   }

   void operator()( const share_operation& op )const
   {
      _db.modify( _bucket, [&]( bucket_object& b )
      {
         const auto& cs_idx = _db.get_index< comment_share_index >().indices().get< by_comment_sharer >();
         const comment_object& comment = _db.get_comment( op.author, op.permlink );
         const account_object& sharer = _db.get_account( op.sharer );
         auto cs_itr = cs_idx.find( boost::make_tuple( comment.id, sharer.name ) );

         if( cs_itr->created == _db.head_block_time() )
         {
            b.comment_shares++;
         }
      });
   }

   void operator()( const limit_order_operation& op )const
   {
      const limit_order_object& order = _db.get_limit_order( op.owner, op.order_id );
      if( order.created == _db.head_block_time() )
      {
         _db.modify( _bucket, [&]( bucket_object& b )
         {
            b.limit_orders++;
         });
      }
   }

   void operator()( const margin_order_operation& op )const
   {
      const margin_order_object& order = _db.get_margin_order( op.owner, op.order_id );
      if( order.created == _db.head_block_time() )
      {
         _db.modify( _bucket, [&]( bucket_object& b )
         {
            b.margin_orders++;
         });
      }
   }

   void operator()( const option_order_operation& op )const
   {
      const option_order_object& order = _db.get_option_order( op.owner, op.order_id );
      if( order.created == _db.head_block_time() )
      {
         _db.modify( _bucket, [&]( bucket_object& b )
         {
            b.option_orders++;
         });
      }
   }

   void operator()( const auction_order_operation& op )const
   {
      const auction_order_object& order = _db.get_auction_order( op.owner, op.order_id );
      if( order.created == _db.head_block_time() )
      {
         _db.modify( _bucket, [&]( bucket_object& b )
         {
            b.auction_orders++;
         });
      }
   }

   void operator()( const call_order_operation& op )const
   {
      const call_order_object& order = _db.get_call_order( op.owner, op.debt.symbol );
      if( order.created == _db.head_block_time() )
      {
         _db.modify( _bucket, [&]( bucket_object& b )
         {
            b.call_orders++;
         });
      }
   }

   void operator()( const ad_bid_operation& op )const
   {
      const ad_bid_object& bid = _db.get_ad_bid( op.bidder, op.bid_id );
      if( bid.created == _db.head_block_time() )
      {
         _db.modify( _bucket, [&]( bucket_object& b )
         {
            b.ad_bids_created++;
         });
      }
   }

};

void blockchain_statistics_plugin_impl::on_block( const signed_block& b )
{
   auto& db = _self.database();

   if( b.block_num() == 1 )
   {
      db.create< bucket_object >( [&]( bucket_object& bo )
      {
         bo.open = b.timestamp;
         bo.seconds = 0;
         bo.blocks = 1;
      });
   }
   else
   {
      db.modify( db.get( bucket_id_type() ), [&]( bucket_object& bo )
      {
         bo.blocks++;
      });
   }

   _current_buckets.clear();
   _current_buckets.insert( bucket_id_type() );      // Add Lifetime statistics bucket

   const auto& bucket_idx = db.get_index< bucket_index >().indices().get< by_bucket >();

   uint32_t trx_size = 0;
   uint32_t num_trx = b.transactions.size();

   for( auto trx : b.transactions )
   {
      trx_size += fc::raw::pack_size( trx );
   }

   for( auto bucket : _tracked_buckets )
   {
      time_point open = fc::time_point( fc::seconds( ( db.head_block_time().sec_since_epoch() / bucket ) * bucket ) );
      auto itr = bucket_idx.find( boost::make_tuple( bucket, open ) );

      if( itr == bucket_idx.end() )
      {
         _current_buckets.insert(
            db.create< bucket_object >( [&]( bucket_object& bo )
            {
               bo.open = open;
               bo.seconds = bucket;
               bo.blocks = 1;
            }).id );

         if( _maximum_history_per_bucket_size > 0 )
         {
            time_point cutoff = time_point( fc::seconds( db.head_block_time().sec_since_epoch() - ( bucket * _maximum_history_per_bucket_size ) ) );

            itr = bucket_idx.lower_bound( boost::make_tuple( bucket, fc::time_point() ) );

            while( itr->seconds == bucket && itr->open < cutoff )
            {
               auto old_itr = itr;
               ++itr;
               db.remove( *old_itr );
            }
         }
      }
      else
      {
         db.modify( *itr, [&]( bucket_object& bo )
         {
            bo.blocks++;
         });

         _current_buckets.insert( itr->id );
      }

      db.modify( *itr, [&]( bucket_object& bo )
      {
         bo.transactions += num_trx;
         bo.bandwidth += trx_size;
      });
   }
}

void blockchain_statistics_plugin_impl::pre_operation( const operation_notification& o )
{
   //
}

void blockchain_statistics_plugin_impl::post_operation( const operation_notification& o )
{
   try
   {
   auto& db = _self.database();

   for( auto bucket_id : _current_buckets )
   {
      const auto& bucket = db.get(bucket_id);

      if( !is_virtual_operation( o.op ) )
      {
         db.modify( bucket, [&]( bucket_object& b )
         {
            b.operations++;
         });
      }
      o.op.visit( operation_process( _self, bucket ) );
   }
   } FC_CAPTURE_AND_RETHROW()
}

} // detail

blockchain_statistics_plugin::blockchain_statistics_plugin( application* app )
   :plugin( app ), _my( new detail::blockchain_statistics_plugin_impl( *this ) ) {}

blockchain_statistics_plugin::~blockchain_statistics_plugin() {}

void blockchain_statistics_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
)
{
   cli.add_options()
         ("chain-stats-bucket-size", boost::program_options::value<string>()->default_value("[60,3600,21600,86400,604800,2592000]"),
           "Track blockchain statistics by grouping orders into buckets of equal size measured in seconds specified as a JSON array of numbers")
         ("chain-stats-history-per-bucket", boost::program_options::value<uint32_t>()->default_value(100),
           "How far back in time to track history for each bucket size, measured in the number of buckets (default: 100)")
         ;
   cfg.add(cli);
}

void blockchain_statistics_plugin::plugin_initialize( const boost::program_options::variables_map& options )
{
   try
   {
      ilog( "chain_stats_plugin: plugin_initialize() begin" );
      chain::database& db = database();

      db.applied_block.connect( [&]( const signed_block& b ){ _my->on_block( b ); } );
      db.pre_apply_operation.connect( [&]( const operation_notification& o ){ _my->pre_operation( o ); } );
      db.post_apply_operation.connect( [&]( const operation_notification& o ){ _my->post_operation( o ); } );

      add_plugin_index< bucket_index >(db);

      if( options.count( "chain-stats-bucket-size" ) )
      {
         const std::string& buckets = options[ "chain-stats-bucket-size" ].as< string >();
         _my->_tracked_buckets = fc::json::from_string( buckets ).as< flat_set< uint64_t > >();
      }
      if( options.count( "chain-stats-history-per-bucket" ) )
         _my->_maximum_history_per_bucket_size = options[ "chain-stats-history-per-bucket" ].as< uint64_t >();

      wlog( "chain-stats-bucket-size: ${b}", ("b", _my->_tracked_buckets) );
      wlog( "chain-stats-history-per-bucket: ${h}", ("h", _my->_maximum_history_per_bucket_size) );

      ilog( "chain_stats_plugin: plugin_initialize() end" );
   } FC_CAPTURE_AND_RETHROW()
}

void blockchain_statistics_plugin::plugin_startup()
{
   ilog( "chain_stats plugin: plugin_startup() begin" );

   app().register_api_factory< blockchain_statistics_api >( "chain_stats_api" );

   ilog( "chain_stats plugin: plugin_startup() end" );
}

const flat_set< uint64_t >& blockchain_statistics_plugin::get_tracked_buckets() const
{
   return _my->_tracked_buckets;
}

uint64_t blockchain_statistics_plugin::get_max_history_per_bucket() const
{
   return _my->_maximum_history_per_bucket_size;
}

} } // node::blockchain_statistics

DEFINE_PLUGIN( blockchain_statistics, node::blockchain_statistics::blockchain_statistics_plugin );
