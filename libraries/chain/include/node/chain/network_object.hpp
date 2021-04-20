#pragma once
#include <fc/fixed_string.hpp>
#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/chain/node_object_types.hpp>
#include <node/chain/shared_authority.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <numeric>

namespace node { namespace chain {


   /**
    * Operates a Network officer for a currency asset.
    * 
    * Recieves ongoing funding and is placed in an ongoing role as a
    * Developer, Marketer, or Advocate.
    */
   class network_officer_object : public object< network_officer_object_type, network_officer_object >
   {
      network_officer_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         network_officer_object( Constructor&& c, allocator< Allocator > a ) :
            details(a), 
            url(a), 
            json(a)
            {
               c( *this );
            }

         id_type                        id;

         account_name_type              account;                          ///< The name of the account that owns the network officer.

         network_officer_role_type      officer_type;                     ///< The type of network officer that the account serves as.

         shared_string                  details;                          ///< The officer's details description.

         shared_string                  url;                              ///< The officer's reference URL.

         shared_string                  json;                             ///< Json metadata of the officer.

         asset_symbol_type              reward_currency;                  ///< Symbol of the currency asset that the network officer requests.

         uint32_t                       vote_count = 0;                   ///< The number of accounts that support the officer.

         share_type                     voting_power = 0;                 ///< The amount of voting power that votes for the officer.

         uint32_t                       producer_vote_count = 0;          ///< The number of producer accounts that support the officer.

         share_type                     producer_voting_power = 0;        ///< The amount of producer voting power that votes for the officer.

         time_point                     last_updated;                     ///< Time the vote was last updated.

         time_point                     created;                          ///< The time the officer was created.

         bool                           active = true;                    ///< True if the officer is active, set false to deactivate.

         bool                           officer_approved = false;         ///< True when the network officer has received sufficient voting approval to earn funds.
   };


   /**
    * Creates a vote for a network officer.
    * 
    * Weighted by stake in the currency that the Network Officer is operating for.
    */
   class network_officer_vote_object : public object< network_officer_vote_object_type, network_officer_vote_object >
   {
      network_officer_vote_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         network_officer_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                        id;

         account_name_type              account;                    ///< The name of the account voting for the officer.

         account_name_type              network_officer;            ///< The name of the network officer being voted for.

         network_officer_role_type      officer_type;               ///< the type of network officer that is being voted for.

         uint16_t                       vote_rank = 1;              ///< the ranking of the vote for the officer.

         time_point                     last_updated;               ///< Time the vote was last updated.

         time_point                     created;                    ///< Time the vote was created.
   };


   /**
    * Supernodes combine a Full node, and IPFS node, an Auth API node, a Notification Server, and a Bittorrent Endpoint.
    * 
    * Supernodes are a Second layer architecture node, 
    * and combine into a cohesive Blockchain object for infrastructural baseload.
    * Supernodes earn a share of all content rewards for views 
    * that reference them as the viewing supernode.
    * They earn a share of premium content fees when 
    * they serve decryption keys to purchasers.
    */
   class supernode_object : public object< supernode_object_type, supernode_object >
   {
      supernode_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         supernode_object( Constructor&& c, allocator< Allocator > a ) :
            details(a), 
            url(a), 
            node_api_endpoint(a), 
            notification_api_endpoint(a), 
            auth_api_endpoint(a), 
            ipfs_endpoint(a), 
            bittorrent_endpoint(a), 
            json(a)
            {
               c( *this );
            }

         id_type                        id;

         account_name_type              account;                          ///< The name of the account that owns the supernode.

         shared_string                  details;                          ///< The supernode's details description. 

         shared_string                  url;                              ///< The supernode's reference URL. 

         shared_string                  node_api_endpoint;                ///< The Full Archive node public API endpoint of the supernode.

         shared_string                  notification_api_endpoint;        ///< The Notification API endpoint of the Supernode. 

         shared_string                  auth_api_endpoint;                ///< The Transaction signing authentication API endpoint of the supernode.

         shared_string                  ipfs_endpoint;                    ///< The IPFS file storage API endpoint of the supernode.

         shared_string                  bittorrent_endpoint;              ///< The Bittorrent Seed Box endpoint URL of the Supernode. 

         shared_string                  json;                             ///< JSON metadata of the supernode, including additional outside of consensus APIs and services. 
         
         asset                          storage_rewards;                  ///< Amount of core asset earned from storage.

         uint64_t                       daily_active_users = 0;           ///< The average number of accounts (X percent 100) that have used files from the node in the prior 24h.

         uint64_t                       monthly_active_users = 0;         ///< The average number of accounts (X percent 100) that have used files from the node in the prior 30 days.

         share_type                     recent_view_weight = 0;           ///< The rolling 7 day average of daily accumulated voting power of viewers.

         time_point                     last_activation_time;             ///< The time the Supernode was last reactivated, must be at least 24h ago to claim rewards.

         time_point                     last_updated;                     ///< The time the file weight and active users was last decayed.

         time_point                     created;                          ///< The time the supernode was created.

         bool                           active = true;                    ///< True if the supernode is active, set false to deactivate.

         void                           decay_weights( const median_chain_property_object& props, time_point now )
         {
            recent_view_weight -= ( ( recent_view_weight * ( now - last_updated ).to_seconds() ) / props.supernode_decay_time.to_seconds() );
            daily_active_users -= ( ( daily_active_users * ( now - last_updated ).to_seconds() ) / fc::days(1).to_seconds() );
            monthly_active_users -= ( ( monthly_active_users * ( now - last_updated ).to_seconds() ) / fc::days(30).to_seconds() );
            last_updated = now;
         }
   };


   /**
    * Interfaces enable a developer to create advertising inventory and be nominated for views.
    * 
    * Interfaces tract the details of off-chain domain properties and 
    * determine the source application of transactions that reference the interface.
    * 
    * Note that Interface assignment may not be completely accurate, 
    * as transaction may specify false interfaces
    * and no transaction assignment of an interface should be
    * used to determine content responsbility. 
    */
   class interface_object : public object< interface_object_type, interface_object >
   {
      interface_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         interface_object( Constructor&& c, allocator< Allocator > a ) :
            details(a), 
            url(a), 
            json(a)
            {
               c( *this );
            }

         id_type                        id;

         account_name_type              account;                           ///< The name of the account that owns the interface.

         shared_string                  details;                           ///< The interface's details description. 

         shared_string                  url;                               ///< The interface's reference URL. 

         shared_string                  json;                              ///< JSON metadata of the interface. 

         uint64_t                       daily_active_users = 0;            ///< The average number of accounts (X percent 100) that have signed a transaction from the interface in the prior 24h.

         uint64_t                       monthly_active_users = 0;          ///< The average number of accounts (X percent 100) that have signed a transaction from the interface in the prior 30 days.

         time_point                     last_updated;                      ///< The time the user counts were last updated.

         time_point                     created;                           ///< The time the interface was created.

         bool                           active = true;                     ///< True if the interface is active, set false to deactivate.

         void                           decay_weights( time_point now )
         {
            daily_active_users -= ( ( daily_active_users * ( now - last_updated ).to_seconds() ) / fc::days(1).to_seconds() );
            monthly_active_users -= ( ( monthly_active_users * ( now - last_updated ).to_seconds() ) / fc::days(30).to_seconds() );
            last_updated = now;
         }
   };


   /**
    * Mediators are assigned to Marketplace tranactions to act as dispute resolving agents.
    * 
    * Each Marketplace transaction is assigned 2 mediators, one from each party.
    * In a dispute, 5 random mediators are added from a pool, and vote on the 
    * allocation of disputed funds.
    * 
    * Mediators earn a share of the marketplace fees for 
    * escrow transfers that they are assigned to. In a dispute, the fee is increased.
    * All mediators place a mediation bond to secure the trustworthyness of thier account,
    * and include a bond in every transfer that they are a party to.
    * 
    * If the vote that they determine deviates from the median vote, 
    * they may lose deposit funds to the other mediators in the dispute.
    */
   class mediator_object : public object< mediator_object_type, mediator_object >
   {
      mediator_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         mediator_object( Constructor&& c, allocator< Allocator > a ) :
            details(a), 
            url(a), 
            json(a)
            {
               c( *this );
            }

         id_type                        id;

         account_name_type              account;                              ///< The name of the account that owns the mediator.

         shared_string                  details;                              ///< The mediator's details description.

         shared_string                  url;                                  ///< The mediator's reference URL.

         shared_string                  json;                                 ///< Json metadata of the mediator.

         asset                          mediator_bond;                        ///< Core Asset staked in mediation bond for selection.

         uint128_t                      mediation_virtual_position = 0;       ///< Quantitative ranking of selection for mediation.

         time_point                     last_updated;                         ///< The time the mediator was last updated.

         time_point                     created;                              ///< The time the mediator was created.

         bool                           active = true;                        ///< True if the mediator is active, set false to deactivate.
   };


   /**
    * Contains an Enterprise Proposal that earns funding from the network.
    * 
    * Enterprise Proposals can recieve funding from contributors, and votes from
    * stakeholders of the currency it earns for it's budget.
    * 
    * 50% of the Funding is divided by voting power, 
    * and the other 50% is divided by funding contributons under quadratic funding.
    */
   class enterprise_object : public object< enterprise_object_type, enterprise_object >
   {
      enterprise_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         enterprise_object( Constructor&& c, allocator< Allocator > a ) :
            enterprise_id(a),
            details(a),
            url(a), 
            json(a)
            {
               c( *this );
            }

         id_type                         id;

         account_name_type               account;                           ///< The name of the account that created the enterprise proposal.

         shared_string                   enterprise_id;                     ///< UUIDv4 referring to the proposal.

         shared_string                   details;                           ///< The proposals's details and planning for funding deployment and deliverables.

         shared_string                   url;                               ///< The proposals's reference URL.

         shared_string                   json;                              ///< JSON metadata of the proposal.

         asset                           budget;                            ///< Amount of Currency Asset requested for project funding.

         asset                           distributed;                       ///< Total amount of funds distributed to the proposal.

         uint32_t                        vote_count = 0;                    ///< The number of accounts that support the enterprise proposal.

         share_type                      voting_power = 0;                  ///< The amount of voting power that supports the enterprise proposal.

         uint32_t                        producer_vote_count = 0;           ///< The overall number of top 50 producers that support the enterprise proposal.

         share_type                      producer_voting_power = 0;         ///< The overall amount of producer voting power that supports the enterprise proposal.

         uint32_t                        funder_count = 0;                  ///< The number of accounts that have sent direct funding to the enterprise proposal.

         asset                           total_funding;                     ///< The overall amount of producer voting power that supports the enterprise proposal.

         uint128_t                       net_sqrt_voting_power = 0;         ///< Sum of all of the square roots of the voting power of each vote.

         uint128_t                       net_sqrt_funding = 0;              ///< Sum of all of the square roots of the total funding amount of each vote.
         
         time_point                      last_updated;                      ///< Time that the Enterprise was last updated.

         time_point                      created;                           ///< Time the Enterprise was created.

         bool                            active = true;                     ///< True if the Enterprise is active, set false to deactivate.

         bool                            approved = false;                  ///< True when the Enterprise proposal has reached approval status.

         asset_symbol_type               budget_symbol()const { return budget.symbol; }      ///< The Asset symbol of the enterprise budget.

         bool                            enterprise_active()const { return active && approved && ( budget.amount > distributed.amount ); }    ///< True when the Enterprise should be receiving budget.
   };
   

   /**
    * Creates a vote for an Enterprise Proposal.
    * 
    * The voting power in the budget currency is used to
    * determine the voting stake of the vote.
    */
   class enterprise_vote_object : public object< enterprise_vote_object_type, enterprise_vote_object >
   {
      enterprise_vote_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         enterprise_vote_object( Constructor&& c, allocator< Allocator > a ) :
            enterprise_id(a)
            {
               c( *this );
            }

         id_type                        id;

         account_name_type              voter;                ///< Account voting for the enterprise.

         account_name_type              account;              ///< The name of the account that created the community enterprise proposal.

         shared_string                  enterprise_id;        ///< UUIDv4 referring to the proposal being claimed.

         uint16_t                       vote_rank = 1;        ///< The vote rank of the approval for enterprise.

         time_point                     last_updated;         ///< Time that the vote was last updated.

         time_point                     created;              ///< The time the vote was created.
   };


   /**
    * Creates a funding contribution to an Enterprise Proposal.
    * 
    * The funds are sent to the project owner, and the
    * square root of the funding amount is added to the total funding share.
    */
   class enterprise_fund_object : public object< enterprise_fund_object_type, enterprise_fund_object >
   {
      enterprise_fund_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         enterprise_fund_object( Constructor&& c, allocator< Allocator > a ) :
            enterprise_id(a)
            {
               c( *this );
            }

         id_type                        id;

         account_name_type              funder;               ///< Account voting for the enterprise.

         account_name_type              account;              ///< The name of the account that created the community enterprise proposal.

         shared_string                  enterprise_id;        ///< UUIDv4 referring to the proposal being claimed.

         asset                          amount;               ///< The total amount of funding sent to the enterprise proposal.

         time_point                     last_updated;         ///< Time that the funding was last updated.

         time_point                     created;              ///< The time the funding was created.
   };



   struct by_name;
   struct by_account;
   struct by_subscribers;
   struct by_subscriber_power;
   struct by_voting_power;
   struct by_vote_count;
   struct by_symbol_type_voting_power;
   struct by_symbol_type_vote_count;


   typedef multi_index_container <
      network_officer_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< network_officer_object, network_officer_id_type, &network_officer_object::id > >,
         ordered_unique< tag< by_account >,
            member< network_officer_object, account_name_type, &network_officer_object::account > >,
         ordered_unique< tag< by_symbol_type_vote_count >,
            composite_key< network_officer_object,
               member< network_officer_object, asset_symbol_type, &network_officer_object::reward_currency >,
               member< network_officer_object, network_officer_role_type, &network_officer_object::officer_type >,
               member< network_officer_object, uint32_t, &network_officer_object::vote_count >,
               member< network_officer_object, network_officer_id_type, &network_officer_object::id >
            >,
            composite_key_compare<
               std::less< asset_symbol_type >,
               std::less< network_officer_role_type >,
               std::greater< uint32_t >,
               std::less< network_officer_id_type >
            >
         >,
         ordered_unique< tag< by_symbol_type_voting_power >,
            composite_key< network_officer_object,
               member< network_officer_object, asset_symbol_type, &network_officer_object::reward_currency >,
               member< network_officer_object, network_officer_role_type, &network_officer_object::officer_type >,
               member< network_officer_object, share_type, &network_officer_object::voting_power >,
               member< network_officer_object, network_officer_id_type, &network_officer_object::id >
            >,
            composite_key_compare<
               std::less< asset_symbol_type >,
               std::less< network_officer_role_type >,
               std::greater< share_type >,
               std::less< network_officer_id_type >
            >
         >
      >,
      allocator< network_officer_object >
   > network_officer_index;


   struct by_account_officer;
   struct by_officer_account;
   struct by_account_type_rank;


   typedef multi_index_container<
      network_officer_vote_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< network_officer_vote_object, network_officer_vote_id_type, &network_officer_vote_object::id > >,
         ordered_unique< tag< by_account_officer >,
            composite_key< network_officer_vote_object,
               member< network_officer_vote_object, account_name_type, &network_officer_vote_object::account >,
               member< network_officer_vote_object, account_name_type, &network_officer_vote_object::network_officer >,
               member< network_officer_vote_object, uint16_t, &network_officer_vote_object::vote_rank >
            >
         >,
         ordered_unique< tag< by_account_type_rank >,
            composite_key< network_officer_vote_object,
               member< network_officer_vote_object, account_name_type, &network_officer_vote_object::account >,
               member< network_officer_vote_object, network_officer_role_type, &network_officer_vote_object::officer_type >,
               member< network_officer_vote_object, uint16_t, &network_officer_vote_object::vote_rank >
            >
         >,
         ordered_unique< tag< by_officer_account >,
            composite_key< network_officer_vote_object,
               member< network_officer_vote_object, account_name_type, &network_officer_vote_object::network_officer >,
               member< network_officer_vote_object, account_name_type, &network_officer_vote_object::account >,
               member< network_officer_vote_object, uint16_t, &network_officer_vote_object::vote_rank >
            >
         >
      >,
      allocator< network_officer_vote_object >
   > network_officer_vote_index;


   struct by_view_weight;
   struct by_daily_active_users;
   struct by_monthly_active_users;


   typedef multi_index_container <
      supernode_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< supernode_object, supernode_id_type, &supernode_object::id > >,
         ordered_unique< tag< by_account >,
            member< supernode_object, account_name_type, &supernode_object::account > >,
         ordered_unique< tag< by_daily_active_users >,
            composite_key< supernode_object,
               member< supernode_object, uint64_t, &supernode_object::daily_active_users >,
               member< supernode_object, supernode_id_type, &supernode_object::id >
            >,
            composite_key_compare< 
               std::greater< uint64_t >, 
               std::less< supernode_id_type > 
            >
         >,
         ordered_unique< tag< by_monthly_active_users >,
            composite_key< supernode_object,
               member< supernode_object, uint64_t, &supernode_object::monthly_active_users >,
               member< supernode_object, supernode_id_type, &supernode_object::id >
            >,
            composite_key_compare< 
               std::greater< uint64_t >, 
               std::less< supernode_id_type > 
            >
         >,
         ordered_unique< tag< by_view_weight >,
            composite_key< supernode_object,
               member< supernode_object, share_type, &supernode_object::recent_view_weight >,
               member< supernode_object, supernode_id_type, &supernode_object::id >
            >,
            composite_key_compare< 
               std::greater< share_type >, 
               std::less< supernode_id_type > 
            >
         >
      >,
      allocator< supernode_object >
   > supernode_index;


   typedef multi_index_container <
      interface_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< interface_object, interface_id_type, &interface_object::id > >,
         ordered_unique< tag< by_account >,
            member< interface_object, account_name_type, &interface_object::account > >,
         ordered_unique< tag< by_daily_active_users >,
            composite_key< interface_object,
               member< interface_object, uint64_t, &interface_object::daily_active_users >,
               member< interface_object, interface_id_type, &interface_object::id >
            >,
            composite_key_compare< 
               std::greater< uint64_t >, 
               std::less< interface_id_type > 
            >
         >,
         ordered_unique< tag< by_monthly_active_users >,
            composite_key< interface_object,
               member< interface_object, uint64_t, &interface_object::monthly_active_users >,
               member< interface_object, interface_id_type, &interface_object::id >
            >,
            composite_key_compare< 
               std::greater< uint64_t >, 
               std::less< interface_id_type > 
            >
         >
      >,
      allocator< interface_object >
   > interface_index;


   struct by_virtual_position;


   typedef multi_index_container <
      mediator_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< mediator_object, mediator_id_type, &mediator_object::id > >,
         ordered_unique< tag< by_account >,
            member< mediator_object, account_name_type, &mediator_object::account > >,
         ordered_unique< tag< by_virtual_position >,
            composite_key< mediator_object,
               member< mediator_object, uint128_t, &mediator_object::mediation_virtual_position >,
               member< mediator_object, mediator_id_type, &mediator_object::id >
            >,
            composite_key_compare< 
               std::greater< uint128_t >, 
               std::less< mediator_id_type > 
            >
         >
      >,
      allocator< mediator_object >
   > mediator_index;


   struct by_enterprise_id;
   struct by_total_budget;
   struct by_begin_time;
   struct by_end_time;
   struct by_expiration;
   struct by_next_payment;
   struct by_total_voting_power;
   struct by_total_producer_approvals;
   struct by_symbol;


   typedef multi_index_container <
      enterprise_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< enterprise_object, enterprise_id_type, &enterprise_object::id > >,
         ordered_unique< tag< by_enterprise_id >,
            composite_key< enterprise_object,
               member< enterprise_object, account_name_type, &enterprise_object::account >,
               member< enterprise_object, shared_string, &enterprise_object::enterprise_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< enterprise_object,
               member< enterprise_object, account_name_type, &enterprise_object::account >,
               member< enterprise_object, enterprise_id_type, &enterprise_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< enterprise_id_type >
            >
         >,
         ordered_unique< tag< by_symbol >,
            composite_key< enterprise_object,
               const_mem_fun< enterprise_object, asset_symbol_type, &enterprise_object::budget_symbol >,
               member< enterprise_object, enterprise_id_type, &enterprise_object::id >
            >,
            composite_key_compare<
               std::less< asset_symbol_type >,
               std::less< enterprise_id_type >
            >
         >,
         ordered_unique< tag< by_total_voting_power >,
            composite_key< enterprise_object,
               member< enterprise_object, share_type, &enterprise_object::voting_power >,
               member< enterprise_object, enterprise_id_type, &enterprise_object::id >
            >,
            composite_key_compare<
               std::greater< share_type >,
               std::less< enterprise_id_type >
            >
         >
      >,
      allocator< enterprise_object >
   > enterprise_index;


   struct by_account_rank;
   struct by_account_enterprise;


   typedef multi_index_container <
      enterprise_vote_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< enterprise_vote_object, enterprise_vote_id_type, &enterprise_vote_object::id > >,
         ordered_unique< tag< by_enterprise_id >,
            composite_key< enterprise_vote_object,
               member< enterprise_vote_object, account_name_type, &enterprise_vote_object::account >,
               member< enterprise_vote_object, shared_string, &enterprise_vote_object::enterprise_id >,
               member< enterprise_vote_object, account_name_type, &enterprise_vote_object::voter >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less, 
               std::less< account_name_type >
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< enterprise_vote_object,
               member< enterprise_vote_object, account_name_type, &enterprise_vote_object::voter >,
               member< enterprise_vote_object, enterprise_vote_id_type, &enterprise_vote_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< enterprise_vote_id_type > 
            >
         >,
         ordered_unique< tag< by_account_enterprise >,
            composite_key< enterprise_vote_object,
               member< enterprise_vote_object, account_name_type, &enterprise_vote_object::voter >,
               member< enterprise_vote_object, account_name_type, &enterprise_vote_object::account >,
               member< enterprise_vote_object, shared_string, &enterprise_vote_object::enterprise_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >,
         ordered_unique< tag< by_account_rank >,
            composite_key< enterprise_vote_object, 
               member< enterprise_vote_object, account_name_type, &enterprise_vote_object::voter >,
               member< enterprise_vote_object, uint16_t, &enterprise_vote_object::vote_rank >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< uint16_t > 
            >
         >
      >,
      allocator< enterprise_vote_object >
   > enterprise_vote_index;


   struct by_funder_account_enterprise;
   struct by_account_enterprise_funder;


   typedef multi_index_container <
      enterprise_fund_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< enterprise_fund_object, enterprise_fund_id_type, &enterprise_fund_object::id > >,
         ordered_unique< tag< by_account_enterprise_funder >,
            composite_key< enterprise_fund_object,
               member< enterprise_fund_object, account_name_type, &enterprise_fund_object::account >,
               member< enterprise_fund_object, shared_string, &enterprise_fund_object::enterprise_id >,
               member< enterprise_fund_object, account_name_type, &enterprise_fund_object::funder >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               strcmp_less,
               std::less< account_name_type >
            >
         >,
         ordered_unique< tag< by_funder_account_enterprise >,
            composite_key< enterprise_fund_object,
               member< enterprise_fund_object, account_name_type, &enterprise_fund_object::funder >,
               member< enterprise_fund_object, account_name_type, &enterprise_fund_object::account >,
               member< enterprise_fund_object, shared_string, &enterprise_fund_object::enterprise_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< account_name_type >,
               strcmp_less
            >
         >
      >,
      allocator< enterprise_fund_object >
   > enterprise_fund_index;



} }         ///< node::chain

FC_REFLECT( node::chain::network_officer_object,
         (id)
         (account)
         (officer_type)
         (details)
         (url)
         (json)
         (reward_currency)
         (vote_count)
         (voting_power)
         (producer_vote_count)
         (producer_voting_power)
         (last_updated)
         (created)
         (active)
         (officer_approved)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::network_officer_object, node::chain::network_officer_index );

FC_REFLECT( node::chain::network_officer_vote_object,
         (id)
         (account)
         (network_officer)
         (officer_type)
         (vote_rank)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::network_officer_vote_object, node::chain::network_officer_vote_index );

FC_REFLECT( node::chain::supernode_object,
         (id)
         (account)
         (details)
         (url)
         (node_api_endpoint)
         (notification_api_endpoint)
         (auth_api_endpoint)
         (ipfs_endpoint)
         (bittorrent_endpoint)
         (json)
         (storage_rewards)
         (daily_active_users)
         (monthly_active_users)
         (recent_view_weight)
         (last_activation_time)
         (last_updated)
         (created)
         (active)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::supernode_object, node::chain::supernode_index );

FC_REFLECT( node::chain::interface_object,
         (id)
         (account)
         (active)
         (details)
         (url)
         (json)
         (daily_active_users)
         (monthly_active_users)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::interface_object, node::chain::interface_index );

FC_REFLECT( node::chain::mediator_object,
         (id)
         (account)
         (details)
         (url)
         (json)
         (mediator_bond)
         (mediation_virtual_position)
         (last_updated)
         (created)
         (active)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::mediator_object, node::chain::mediator_index );

FC_REFLECT( node::chain::enterprise_object,
         (id)
         (account)
         (enterprise_id)
         (details)
         (url)
         (json)
         (budget)
         (distributed)
         (vote_count)
         (voting_power)
         (producer_vote_count)
         (producer_voting_power)
         (funder_count)
         (total_funding)
         (net_sqrt_voting_power)
         (net_sqrt_funding)
         (last_updated)
         (created)
         (active)
         (approved)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::enterprise_object, node::chain::enterprise_index );

FC_REFLECT( node::chain::enterprise_vote_object,
         (id)
         (voter)
         (account)
         (enterprise_id)
         (vote_rank)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::enterprise_vote_object, node::chain::enterprise_vote_index );

FC_REFLECT( node::chain::enterprise_fund_object,
         (id)
         (funder)
         (account)
         (enterprise_id)
         (amount)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::enterprise_fund_object, node::chain::enterprise_fund_index );