#pragma once
#include <node/app/plugin.hpp>

#include <node/chain/node_object_types.hpp>

#include <boost/multi_index/composite_key.hpp>

//
// Plugins should #define their SPACE_ID's so plugins with
// conflicting SPACE_ID assignments can be compiled into the
// same binary (by simply re-assigning some of the conflicting #defined
// SPACE_ID's in a build script).
//
// Assignment of SPACE_ID's cannot be done at run-time because
// various template automagic depends on them being known at compile
// time.
//
#ifndef BLOCKCHAIN_STATISTICS_SPACE_ID
#define BLOCKCHAIN_STATISTICS_SPACE_ID 9
#endif

#ifndef BLOCKCHAIN_STATISTICS_PLUGIN_NAME
#define BLOCKCHAIN_STATISTICS_PLUGIN_NAME "chain_stats"
#endif

namespace node { namespace blockchain_statistics {

using namespace node::chain;
using app::application;
using fc::time_point;

enum blockchain_statistics_object_type
{
   bucket_object_type = ( BLOCKCHAIN_STATISTICS_SPACE_ID << 8 )
};

namespace detail
{
   class blockchain_statistics_plugin_impl;
}

class blockchain_statistics_plugin : public node::app::plugin
{
   public:
      blockchain_statistics_plugin( application* app );
      virtual ~blockchain_statistics_plugin();

      virtual std::string plugin_name()const override { return BLOCKCHAIN_STATISTICS_PLUGIN_NAME; }
      virtual void plugin_set_program_options(
         boost::program_options::options_description& cli,
         boost::program_options::options_description& cfg ) override;
      virtual void plugin_initialize( const boost::program_options::variables_map& options ) override;
      virtual void plugin_startup() override;

      const flat_set< uint64_t >& get_tracked_buckets() const;
      uint64_t get_max_history_per_bucket() const;

   private:
      friend class detail::blockchain_statistics_plugin_impl;
      std::unique_ptr< detail::blockchain_statistics_plugin_impl > _my;
};

struct bucket_object : public object< bucket_object_type, bucket_object >
{
   template< typename Constructor, typename Allocator >
   bucket_object( Constructor&& c, allocator< Allocator > a )
   {
      c( *this );
   }

   id_type              id;

   time_point           open;                                        ///< Open time of the bucket

   uint64_t             seconds = 0;                                 ///< Seconds accounted for in the bucket

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
};

typedef oid< bucket_object > bucket_id_type;

struct by_id;
struct by_bucket;
typedef multi_index_container<
   bucket_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< bucket_object, bucket_id_type, &bucket_object::id > >,
      ordered_unique< tag< by_bucket >,
         composite_key< bucket_object,
            member< bucket_object, uint64_t, &bucket_object::seconds >,
            member< bucket_object, time_point, &bucket_object::open >
         >
      >
   >,
   allocator< bucket_object >
> bucket_index;

} } // node::blockchain_statistics

FC_REFLECT( node::blockchain_statistics::bucket_object,
   (id)
   (open)
   (seconds)
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
)
CHAINBASE_SET_INDEX_TYPE( node::blockchain_statistics::bucket_object, node::blockchain_statistics::bucket_index )
