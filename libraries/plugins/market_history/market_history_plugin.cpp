#include <node/market_history/market_history_api.hpp>

#include <node/chain/database.hpp>
#include <node/chain/index.hpp>
#include <node/chain/operation_notification.hpp>

namespace node { namespace market_history { namespace detail {

using node::protocol::fill_order_operation;

class market_history_plugin_impl
{
   public:
      market_history_plugin_impl( market_history_plugin& plugin )
         :_self( plugin ) {}
      virtual ~market_history_plugin_impl() {}

      /**
       * This method is called as a callback after a block is applied
       * and will process/index all operations that were applied in the block.
       */
      void update_market_histories( const operation_notification& o );

      market_history_plugin& _self;
      flat_set<uint32_t>     _tracked_durations = flat_set<uint32_t>  { 60, 300, 3600, 86400, 604800 }; // 1 min, 5 min, 60 min, 24h, 7d candles
      int32_t                _maximum_history_per_duration_size = 20000;
};

void market_history_plugin_impl::update_market_histories( const operation_notification& o )
{
   if( o.op.which() == operation::tag< fill_order_operation >::value )
   {
      fill_order_operation op = o.op.get< fill_order_operation >();

      auto& db = _self.database();
      const auto& duration_idx = db.get_index< market_history::market_duration_index >().indices().get< by_new_asset_pair >();

      db.create< order_history_object >( [&]( order_history_object& ho )
      {
         ho.time = db.head_block_time();
         ho.op = op;
      });

      if( !_maximum_history_per_duration_size ) return;
      if( !_tracked_durations.size() ) return;

      for( uint32_t duration : _tracked_durations )
      {
         time_point cutoff = db.head_block_time() - fc::seconds( duration * _maximum_history_per_duration_size );

         time_point open_time = fc::time_point( fc::microseconds( ( db.head_block_time().time_since_epoch().count() / duration ) * duration ) ); // round down to opening time point

         auto duration_itr = duration_idx.find( boost::make_tuple( op.symbol_a, op.symbol_b, duration, open_time ) );
         if( duration_itr == duration_idx.end() )
         {
            db.create< market_duration_object >( [&]( market_duration_object& mdo )
            {
               mdo.open_time = open_time;
               mdo.seconds = duration;

               if( op.open_pays.symbol == op.symbol_a )
               {
                  mdo.open_price = price( op.open_pays, op.current_pays);
                  mdo.high_price = price( op.open_pays, op.current_pays);
                  mdo.low_price = price( op.open_pays, op.current_pays);
                  mdo.close_price = price( op.open_pays, op.current_pays);
                  mdo.volume_a = op.open_pays;
                  mdo.volume_b = op.current_pays;
               }
               else
               {
                  mdo.open_price = price( op.current_pays, op.open_pays);
                  mdo.high_price = price( op.current_pays, op.open_pays);
                  mdo.low_price = price( op.current_pays, op.open_pays);
                  mdo.close_price = price( op.current_pays, op.open_pays);
                  mdo.volume_a = op.current_pays;
                  mdo.volume_b = op.open_pays;
               }
            });
         }
         else
         {
            db.modify( *duration_itr, [&]( market_duration_object& mdo )
            {
               if( op.open_pays.symbol == op.symbol_a )
               {
                  mdo.close_price = price( op.open_pays, op.current_pays);
                  mdo.volume_a += op.open_pays;
                  mdo.volume_b += op.current_pays;

                  if( mdo.high_price < price( op.open_pays, op.current_pays) )
                  {
                     mdo.high_price = price( op.open_pays, op.current_pays);
                  }
                  if( mdo.low_price > price( op.open_pays, op.current_pays) )
                  {
                     mdo.low_price = price( op.open_pays, op.current_pays);
                  }
               }
               else
               {
                  mdo.close_price = price( op.current_pays, op.open_pays);
                  mdo.volume_a += op.current_pays;
                  mdo.volume_b += op.open_pays;

                  if( mdo.high_price < price( op.current_pays, op.open_pays) )
                  {
                     mdo.high_price = price( op.current_pays, op.open_pays);
                  }
                  if( mdo.low_price > price( op.current_pays, op.open_pays) )
                  {
                     mdo.low_price = price( op.current_pays, op.open_pays);
                  }
               }
            });

            if( _maximum_history_per_duration_size > 0 )
            {
               open_time = fc::time_point();
               duration_itr = duration_idx.lower_bound( boost::make_tuple( op.symbol_a, op.symbol_b, duration, open_time ) );

               while( duration_itr->seconds == duration && duration_itr->open_time < cutoff )
               {
                  auto old_duration_itr = duration_itr;
                  ++duration_itr;
                  db.remove( *old_duration_itr );
               }
            }
         }
      }
   }
}

} // detail

market_history_plugin::market_history_plugin( application* app )
   : plugin( app ), _my( new detail::market_history_plugin_impl( *this ) ) {}
market_history_plugin::~market_history_plugin() {}

void market_history_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
)
{
   cli.add_options()
         ("market-history-duration-size", boost::program_options::value<string>()->default_value("[60,300,3600,86400,604800]"),
           "Track market history by grouping orders into durations of equal size measured in seconds specified as a JSON array of numbers")
         ("market-history-durations-per-size", boost::program_options::value<uint32_t>()->default_value(20000),
           "How far back in time to track history for each duration size, measured in the number of durations (default: 20000)")
         ;
   cfg.add(cli);
}

void market_history_plugin::plugin_initialize( const boost::program_options::variables_map& options )
{
   try
   {
      ilog( "market_history: plugin_initialize() begin" );
      chain::database& db = database();

      db.post_apply_operation.connect( [&]( const operation_notification& o ){ _my->update_market_histories( o ); } );
      add_plugin_index< market_duration_index      >(db);
      add_plugin_index< order_history_index        >(db);

      if( options.count("duration-size" ) )
      {
         std::string durations = options["duration-size"].as< string >();
         _my->_tracked_durations = fc::json::from_string( durations ).as< flat_set< uint32_t > >();
      }
      if( options.count("history-per-size" ) )
         _my->_maximum_history_per_duration_size = options["history-per-size"].as< uint32_t >();

      wlog( "duration-size ${b}", ("b", _my->_tracked_durations) );
      wlog( "history-per-size ${h}", ("h", _my->_maximum_history_per_duration_size) );

      ilog( "market_history: plugin_initialize() end" );
   } FC_CAPTURE_AND_RETHROW()
}

void market_history_plugin::plugin_startup()
{
   ilog( "market_history plugin: plugin_startup() begin" );

   app().register_api_factory< market_history_api >( "market_history_api" );

   ilog( "market_history plugin: plugin_startup() end" );
}

flat_set< uint32_t > market_history_plugin::get_tracked_durations() const
{
   return _my->_tracked_durations;
}

uint32_t market_history_plugin::get_max_history_per_duration() const
{
   return _my->_maximum_history_per_duration_size;
}

} } // node::market_history

DEFINE_PLUGIN( market_history, node::market_history::market_history_plugin )
