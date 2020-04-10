#pragma once

#include <node/app/plugin.hpp>
#include <node/chain/database.hpp>

#include <fc/thread/future.hpp>

#define RESERVE_RATIO_PRECISION ((int64_t)10000)
#define RESERVE_RATIO_MIN_INCREMENT ((int64_t)5000)

namespace node { namespace producer {

using std::string;
using protocol::public_key_type;
using app::application;
using node::protocol::block_id_type;
using node::protocol::asset;

namespace block_production_condition
{
   enum block_production_condition_enum
   {
      produced = 0,
      not_synced = 1,
      not_my_turn = 2,
      not_time_yet = 3,
      no_private_key = 4,
      low_participation = 5,
      lag = 6,
      consecutive = 7,
      wait_for_genesis = 8,
      exception_producing_block = 9
   };
}

namespace block_mining_condition
{
   enum block_mining_condition_enum
   {
      mined = 0,
      not_synced = 1,
      not_my_turn = 2,
      not_time_yet = 3,
      no_private_key = 4,
      low_participation = 5,
      lag = 6,
      consecutive = 7,
      wait_for_genesis = 8,
      exception_mining_block = 9
   };
}

namespace block_validation_condition
{
   enum block_validation_condition_enum
   {
      fully_validated = 0,
      not_synced = 1,
      not_my_turn = 2,
      not_time_yet = 3,
      no_private_key = 4,
      low_participation = 5,
      lag = 6,
      consecutive = 7,
      wait_for_genesis = 8,
      exception_validating_block = 9
   };
}

namespace detail
{
   class producer_plugin_impl;
}

class producer_plugin : public node::app::plugin
{
public:
   producer_plugin( application* app );
   virtual ~producer_plugin();

   std::string plugin_name()const override;

   virtual void plugin_set_program_options(
      boost::program_options::options_description &command_line_options,
      boost::program_options::options_description &config_file_options
      ) override;

   void set_block_production(bool allow) { _production_enabled = allow; }

   virtual void plugin_initialize( const boost::program_options::variables_map& options ) override;
   virtual void plugin_startup() override;
   virtual void plugin_shutdown() override;

private:
   void schedule_production_loop();
   block_production_condition::block_production_condition_enum block_production_loop();
   block_production_condition::block_production_condition_enum maybe_produce_block( fc::mutable_variant_object& capture );

   void schedule_mining_loop();
   block_mining_condition::block_mining_condition_enum block_mining_loop();
   block_mining_condition::block_mining_condition_enum maybe_mine_proof_of_work( fc::mutable_variant_object& capture );
   void mine_proof_of_work( chain::x11_proof_of_work& work, uint128_t summary_target );

   void schedule_validation_loop();
   block_validation_condition::block_validation_condition_enum block_validation_loop();
   block_validation_condition::block_validation_condition_enum maybe_validate_blocks( fc::mutable_variant_object& capture );

   boost::program_options::variables_map _options;
   bool _production_enabled = false;
   uint32_t _required_producer_participation = 33 * PERCENT_1;
   uint32_t _production_skip_flags = node::chain::database::skip_nothing;

   block_id_type    _head_block_id        = block_id_type();
   fc::time_point   _hash_start_time;

   std::map< public_key_type, fc::ecc::private_key >     _private_keys;

   std::set< string >                                    _producers;

   string                                                _mining_producer;

   fc::future< void >                                    _block_production_task;

   fc::future< void >                                    _block_mining_task;

   fc::future< void >                                    _block_validation_task;

   asset                                                 _commit_coin_amount;

   chain::chain_properties                               _mining_props;

   friend class detail::producer_plugin_impl;
   std::unique_ptr< detail::producer_plugin_impl > _my;
};

} } //node::producer
