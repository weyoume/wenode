#pragma once
#include <fc/fixed_string.hpp>

#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>

#include <node/chain/node_object_types.hpp>
#include <node/chain/witness_objects.hpp>
#include <node/chain/shared_authority.hpp>

#include <boost/multi_index/composite_key.hpp>

#include <numeric>

namespace node { namespace chain {

   struct strcmp_less
   {
      bool operator()( const shared_string& a, const shared_string& b )const
      {
         return less( a.c_str(), b.c_str() );
      }

      bool operator()( const shared_string& a, const string& b )const
      {
         return less( a.c_str(), b.c_str() );
      }

      bool operator()( const string& a, const shared_string& b )const
      {
         return less( a.c_str(), b.c_str() );
      }

      private:
         inline bool less( const char* a, const char* b )const
         {
            return std::strcmp( a, b ) < 0;
         }
   };

   class ad_creative_object : public object< ad_creative_object_type, ad_creative_object >
   {
      ad_creative_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         ad_creative_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                     id;

         account_name_type           account;           // Name of the account creating the creative.

         shared_string               creative_id;       // The uuidv4 of the creative for reference

         format_types                format_type;       // The type of formatting used for the ad, determines the interpretation of the creative and objective.

         account_name_type           author;            // Name of the account that created the objective.

         shared_string               objective;         // The name of the object being advertised, the link and CTA destination of the creative.

         shared_string               creative;          // IPFS link to the Media to be displayed, image or video.

         shared_string               json;              // Public plaintext json information about the board, its topic and rules.

         time_point                  created;           // Time creative was made.

         time_point                  last_updated;      // Time creative's details were last updated.

         bool                        active = true;     // True when the creative is active for use in campaigns, false to deactivate.
   };


   class ad_campaign_object : public object< ad_campaign_object_type, ad_campaign_object >
   {
      ad_campaign_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         ad_campaign_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                          id;

         account_name_type                account;           // Account creating the ad campaign.

         shared_string                    campaign_id;       // uuidv4 to refer to the campaign.

         asset                            budget;            // Total expenditure of the campaign.

         asset                            total_bids;        // Total amount of expenditure in active bids. Cannot exceed AD_RESERVE_RATIO times the campaign budget.

         time_point                       begin;             // Beginning time of the campaign. Bids cannot be created before this time.

         time_point                       end;               // Ending time of the campaign. Remaining campaign budget will be refunded after this time.

         shared_string                    json;              // json metadata for the campaign.

         flat_set<account_name_type>      agents;            // Set of Accounts authorized to create bids for the campaign.

         account_name_type                interface;         // Interface that facilitated the purchase of the advertising campaign.
         
         time_point                       created;           // Time campaign was created.

         time_point                       last_updated;      // Time campaigns's details were last updated or inventory was delivered.

         bool                             active = true;     // True when active for bidding and delivery, false to deactivate.

         bool is_agent( const account_name_type& account )const  
         {
            if( agents.find( account ) != agents.end() )
            {
               return true; // The account is in the agents list.
            }
            else 
            {
               return false; // The account is not in the agents list.
            }
         };
   };


   class ad_inventory_object : public object< ad_inventory_object_type, ad_inventory_object >
   {
      ad_inventory_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         ad_inventory_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                          id;

         account_name_type                provider;          // Account creating the ad inventory.

         shared_string                    inventory_id;      // uuidv4 to refer to the inventory.

         metric_types                     metric;            // Type of expense metric used.

         shared_string                    audience_id;       // ad audience_id, containing a set of usernames of viewing accounts in their userbase.

         asset                            min_price;         // Minimum bidding price per metric.

         uint32_t                         inventory;         // Total metrics available.

         uint32_t                         remaining;         // Current amount of inventory remaining. Decrements when delivered.

         shared_string                    json;              // json metadata for the inventory.

         flat_set<account_name_type>      agents;            // Set of Accounts authorized to create delivery transactions for the inventory.
         
         time_point                       created;           // Time inventory was created.

         time_point                       last_updated;      // Time inventorys's details were last updated or inventory was delivered.

         time_point                       expiration;        // Time that the inventory offering expires. All outstanding bids for the inventory also expire at this time. 

         bool                             active = true;     // True when active for bidding and delivery, false to deactivate.

         bool is_agent( const account_name_type& account )const  
         {
            if( agents.find( account ) != agents.end() )
            {
               return true; // The account is in the agents list.
            }
            else 
            {
               return false; // The account is not in the agents list.
            }
         };
   };

   class ad_audience_object : public object< ad_audience_object_type, ad_audience_object >
   {
      ad_audience_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         ad_audience_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                          id;

         account_name_type                account;           // Account creating the ad audience.

         shared_string                    audience_id;       // uuidv4 to refer to the audience.

         shared_string                    json;              // json metadata for the audience.

         flat_set< account_name_type >    audience;          // List of usernames within the audience for campaigns and inventory.
         
         time_point                       created;           // Time audience was created.

         time_point                       last_updated;      // Time audiences's details were last updated.

         bool                             active = true;     // True when active for bidding and delivery, false to deactivate.

         bool is_audience( const account_name_type& account )const  
         {
            if( audience.find( account ) != audience.end() )
            {
               return true; // The account is in the audience list.
            }
            else 
            {
               return false; // The account is not in the audience list.
            }
         };
   };

   class ad_bid_object : public object< ad_bid_object_type, ad_bid_object >
   {
      ad_bid_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         ad_bid_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                          id;

         account_name_type                bidder;            // Account that created the ad budget, or an agent of the campaign.

         shared_string                    bid_id;            // Bid uuidv4 for referring to the bid and updating it or cancelling it..

         account_name_type                account;           // Account that created the campaign that this bid is directed towards.  

         shared_string                    creative_id;       // Desired creative for display. 

         shared_string                    campaign_id;       // Ad campaign uuidv4 to utilise for the bid.

         account_name_type                provider;          // Account offering inventory supply.

         shared_string                    inventory_id;      // Inventory uuidv4 offering to bid on.

         shared_string                    audience_id;       // Desired audience for display acceptance. Audience must include only members of the inventory audience.

         asset                            bid_price;         // Price offered per metric. Asset symbol must be the same as the inventory price.

         uint32_t                         requested;         // Maximum total metrics requested.

         uint32_t                         remaining;         // Current amount of inventory remaining. Decrements when delivered.
         
         time_point                       created;           // Time audience was created.

         time_point                       last_updated;      // Time audiences's details were last updated or inventory was delivered.

         time_point                       expiration;        // Time audience was created.

   };

   struct by_creative_id;

   typedef multi_index_container<
      ad_creative_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< ad_creative_object, ad_creative_id_type, &ad_creative_object::id > >,
         ordered_unique< tag< by_creative_id >,
            composite_key< ad_creative_object,
               member< ad_creative_object, account_name_type, &ad_creative_object::author>,
               member< ad_creative_object, shared_string, &ad_creative_object::creative_id >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less >
         > 
      >,
      allocator< ad_creative_object >
   > ad_creative_index;

   struct by_campaign_id;

   typedef multi_index_container<
      ad_campaign_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< ad_campaign_object, ad_campaign_id_type, &ad_campaign_object::id > >,
         ordered_unique< tag< by_campaign_id >,
            composite_key< ad_campaign_object,
               member< ad_campaign_object, account_name_type, &ad_campaign_object::account >,
               member< ad_campaign_object, shared_string, &ad_campaign_object::campaign_id >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less >
         > 
      >,
      allocator< ad_campaign_object >
   > ad_campaign_index;

   struct by_inventory_id;
   struct by_metric;

   typedef multi_index_container<
      ad_inventory_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< ad_inventory_object, ad_inventory_id_type, &ad_inventory_object::id > >,
         ordered_unique< tag< by_inventory_id >,
            composite_key< ad_inventory_object,
               member< ad_inventory_object, account_name_type, &ad_inventory_object::provider >,
               member< ad_inventory_object, shared_string, &ad_inventory_object::inventory_id >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less >
         >,
         ordered_unique< tag< by_metric >,
            composite_key< ad_inventory_object,
               member< ad_inventory_object, metric_types, &ad_inventory_object::metric >,
               member< ad_inventory_object, asset, &ad_inventory_object::min_price >,
               member< ad_inventory_object, ad_inventory_id_type, &ad_inventory_object::id >
            >,
            composite_key_compare< std::less< metric_types >, std::less< asset >, std::less< ad_inventory_id_type > >
         > 
      >,
      allocator< ad_inventory_object >
   > ad_inventory_index;

   struct by_audience_id;

   typedef multi_index_container<
      ad_audience_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< ad_audience_object, ad_audience_id_type, &ad_audience_object::id > >,
         ordered_unique< tag< by_audience_id >,
            composite_key< ad_audience_object,
               member< ad_audience_object, account_name_type, &ad_audience_object::account >,
               member< ad_audience_object, shared_string, &ad_audience_object::audience_id >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less >
         > 
      >,
      allocator< ad_audience_object >
   > ad_audience_index;

   struct by_expiration;
   struct by_bidder_inventory;
   struct by_bidder_campaign;
   struct by_provider_inventory;

   struct by_bidder_updated;
   struct by_provider_updated;
   struct by_bid_id;

   typedef multi_index_container<
      ad_bid_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< ad_bid_object, ad_bid_id_type, &ad_bid_object::id > >,
         ordered_non_unique< tag< by_expiration >, member< ad_bid_object, time_point, &ad_bid_object::expiration > >,
         ordered_unique< tag< by_bid_id >,
            composite_key< ad_bid_object,
               member< ad_bid_object, account_name_type, &ad_bid_object::bidder >,
               member< ad_bid_object, shared_string, &ad_bid_object::bid_id >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less  >
         >,
         ordered_unique< tag< by_bidder_campaign >,
            composite_key< ad_bid_object,
               member< ad_bid_object, account_name_type, &ad_bid_object::bidder >,
               member< ad_bid_object, shared_string, &ad_bid_object::campaign_id >,
               member< ad_bid_object, asset, &ad_bid_object::bid_price >,
               member< ad_bid_object, ad_bid_id_type, &ad_bid_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less, std::greater< asset >, std::less< ad_bid_id_type >  >
         >,
         ordered_unique< tag< by_provider_inventory >,
            composite_key< ad_bid_object,
               member< ad_bid_object, account_name_type, &ad_bid_object::provider >,
               member< ad_bid_object, shared_string, &ad_bid_object::inventory_id >,
               member< ad_bid_object, asset, &ad_bid_object::bid_price >,
               member< ad_bid_object, ad_bid_id_type, &ad_bid_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less, std::greater< asset >, std::less< ad_bid_id_type >  >
         >,
         ordered_unique< tag< by_bidder_updated >,
            composite_key< ad_bid_object,
               member< ad_bid_object, account_name_type, &ad_bid_object::bidder >,
               member< ad_bid_object, time_point, &ad_bid_object::last_updated >,
               member< ad_bid_object, ad_bid_id_type, &ad_bid_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::greater< time_point >, std::less< ad_bid_id_type > >
         >,
         ordered_unique< tag< by_provider_updated >,
            composite_key< ad_bid_object,
               member< ad_bid_object, account_name_type, &ad_bid_object::provider >,
               member< ad_bid_object, time_point, &ad_bid_object::last_updated >,
               member< ad_bid_object, ad_bid_id_type, &ad_bid_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::greater< time_point >, std::less< ad_bid_id_type > >
         >  
      >,
      allocator< ad_bid_object >
   > ad_bid_index;

} } // node::chain

FC_REFLECT( node::chain::ad_creative_object,
         (id)
         (author)
         (creative_id)
         (format_type)  
         (objective)
         (creative)
         (json)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::ad_creative_object, node::chain::ad_creative_index );

FC_REFLECT( node::chain::ad_campaign_object,
         (id)
         (account)
         (campaign_id)  
         (budget)
         (begin)
         (end)
         (json)
         (agents)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::ad_campaign_object, node::chain::ad_campaign_index );

FC_REFLECT( node::chain::ad_inventory_object,
         (id)
         (provider)
         (inventory_id)  
         (metric)
         (min_price)
         (inventory)
         (remaining)
         (json)
         (audience)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::ad_inventory_object, node::chain::ad_inventory_index );

FC_REFLECT( node::chain::ad_audience_object,
         (id)
         (account)
         (audience_id)
         (json)
         (audience)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::ad_audience_object, node::chain::ad_audience_index );

FC_REFLECT( node::chain::ad_bid_object,
         (id)
         (bidder)
         (provider)
         (campaign_id)
         (inventory_id)
         (metric)
         (bid_price)
         (requested)
         (remaining)
         (audience)
         (created)
         (last_updated)
         (expiration)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::ad_bid_object, node::chain::ad_bid_index );