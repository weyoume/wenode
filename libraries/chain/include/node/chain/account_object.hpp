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

   using node::protocol::authority;

   class account_object : public object< account_object_type, account_object >
   {
      account_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         account_object( Constructor&& c, allocator< Allocator > a )
            :json( a )
         {
            c(*this);
         };

         id_type           id;

         account_name_type name;
         public_key_type   memoKey;
         shared_string     json;
         account_name_type proxy;

         time_point_sec    last_accountUpdate;

         time_point_sec    created;
         bool              mined = true;
         bool              owner_challenged = false;
         bool              active_challenged = false;
         time_point_sec    last_owner_proved = time_point_sec::min();
         time_point_sec    last_active_proved = time_point_sec::min();
         account_name_type recoveryAccount;
         account_name_type reset_account = NULL_ACCOUNT;
         time_point_sec    last_account_recovery;
         uint32_t          comment_count = 0;
         uint32_t          lifetime_vote_count = 0;
         uint32_t          post_count = 0;

         bool              can_vote = true;
         uint16_t          voting_power = PERCENT_100;   ///< current voting power of this account, it falls after every vote
         time_point_sec    last_vote_time; ///< used to increase the voting power of this account the longer it goes without voting.

         asset             balance = asset( 0, SYMBOL_TME );  ///< total liquid SCORE held by this account
         asset             TMEsavingsBalance = asset( 0, SYMBOL_TME );  ///< total liquid SCORE held by this account

         /**
          *  TSD Deposits pay interest based upon the interest rate set by witnesses. The purpose of these
          *  fields is to track the total (time * TSDbalance) that it is held. Then at the appointed time
          *  interest can be paid using the following equation:
          *
          *  interest = interest_rate * TSD_seconds / seconds_per_year
          *
          *  Every time the TSDbalance is updated the TSD_seconds is also updated. If at least
          *  MIN_COMPOUNDING_INTERVAL_SECONDS has past since TSD_last_interest_payment then
          *  interest is added to TSDbalance.
          *
          *  @defgroup TSD_data TSD Balance Data
          */
         ///@{
         asset             TSDbalance = asset( 0, SYMBOL_TSD ); /// total TSD balance
         uint128_t         TSD_seconds; ///< total TSD * how long it has been hel
         time_point_sec    TSD_seconds_last_update; ///< the last time the TSD_seconds was updated
         time_point_sec    TSD_last_interest_payment; ///< used to pay interest at most once per month


         asset             TSDsavingsBalance = asset( 0, SYMBOL_TSD ); /// total TSD balance
         uint128_t         savings_TSD_seconds; ///< total TSD * how long it has been hel
         time_point_sec    savings_TSD_seconds_last_update; ///< the last time the TSD_seconds was updated
         time_point_sec    savings_TSD_last_interest_payment; ///< used to pay interest at most once per month

         uint8_t           savings_withdraw_requests = 0;
         ///@}

         asset             SCORErewardBalanceInTME = asset( 0, SYMBOL_TME );
         asset             TMErewardBalance = asset( 0, SYMBOL_TME );
         asset             SCORErewardBalance = asset( 0, SYMBOL_SCORE );
         asset             TSDrewardBalance = asset( 0, SYMBOL_TSD );

         share_type        curationRewards = 0;
         share_type        posting_rewards = 0;

         asset             SCORE = asset( 0, SYMBOL_SCORE ); ///< total SCORE held by this account, controls its voting power
         asset             SCOREDelegated = asset( 0, SYMBOL_SCORE );
         asset             SCOREreceived = asset( 0, SYMBOL_SCORE );

         asset             SCOREwithdrawRateInTME = asset( 0, SYMBOL_SCORE ); ///< at the time this is updated it can be at most SCORE/104
         time_point_sec    nextSCOREwithdrawalTime = fc::time_point_sec::maximum(); ///< after every withdrawal this is incremented by 1 week
         share_type        withdrawn = 0; /// Track how many SCORE have been withdrawn
         share_type        to_withdraw = 0; /// Might be able to look this up with operation history.
         uint16_t          withdraw_routes = 0;

         fc::array<share_type, MAX_PROXY_RECURSION_DEPTH> proxied_SCOREfundTMEbalance_votes;// = std::vector<share_type>( MAX_PROXY_RECURSION_DEPTH, 0 ); ///< the total SCORE votes proxied to this account

         uint16_t          witnesses_voted_for = 0;

         time_point_sec    last_post;
         time_point_sec    last_root_post = fc::time_point_sec::min();
         uint32_t          post_bandwidth = 0;

         /// This function should be used only when the account votes for a witness directly
         share_type        witness_vote_weight()const {
            return std::accumulate( proxied_SCOREfundTMEbalance_votes.begin(),
                                    proxied_SCOREfundTMEbalance_votes.end(),
                                    SCORE.amount );
         }
         share_type        proxied_SCOREfundTMEbalance_votes_total()const {
            return std::accumulate( proxied_SCOREfundTMEbalance_votes.begin(),
                                    proxied_SCOREfundTMEbalance_votes.end(),
                                    share_type() );
         }

         asset effective_SCORE()const { return SCORE - SCOREDelegated + SCOREreceived; }
   };

   class account_authority_object : public object< account_authority_object_type, account_authority_object >
   {
      account_authority_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_authority_object( Constructor&& c, allocator< Allocator > a )
            : owner( a ), active( a ), posting( a )
         {
            c( *this );
         }

         id_type           id;

         account_name_type account;

         shared_authority  owner;   ///< used for backup control, can set owner or active
         shared_authority  active;  ///< used for all monetary operations, can set active or posting
         shared_authority  posting; ///< used for voting and posting

         time_point_sec    last_owner_update;
   };

   class TME_fund_for_SCORE_delegation_object : public object< TME_fund_for_SCORE_delegation_object_type, TME_fund_for_SCORE_delegation_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         TME_fund_for_SCORE_delegation_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         TME_fund_for_SCORE_delegation_object() {}

         id_type           id;
         account_name_type delegator;
         account_name_type delegatee;
         asset             SCORE;
         time_point_sec    min_delegation_time;
   };

   class TME_fund_for_SCORE_delegation_expiration_object : public object< TME_fund_for_SCORE_delegation_expiration_object_type, TME_fund_for_SCORE_delegation_expiration_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         TME_fund_for_SCORE_delegation_expiration_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         TME_fund_for_SCORE_delegation_expiration_object() {}

         id_type           id;
         account_name_type delegator;
         asset             SCORE;
         time_point_sec    expiration;
   };

   class owner_authority_history_object : public object< owner_authority_history_object_type, owner_authority_history_object >
   {
      owner_authority_history_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         owner_authority_history_object( Constructor&& c, allocator< Allocator > a )
            :previous_owner_authority( shared_authority::allocator_type( a.get_segment_manager() ) )
         {
            c( *this );
         }

         id_type           id;

         account_name_type account;
         shared_authority  previous_owner_authority;
         time_point_sec    last_valid_time;
   };

   class account_recovery_request_object : public object< account_recovery_request_object_type, account_recovery_request_object >
   {
      account_recovery_request_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_recovery_request_object( Constructor&& c, allocator< Allocator > a )
            :new_owner_authority( shared_authority::allocator_type( a.get_segment_manager() ) )
         {
            c( *this );
         }

         id_type           id;

         account_name_type accountToRecover;
         shared_authority  new_owner_authority;
         time_point_sec    expires;
   };

   class change_recoveryAccount_request_object : public object< change_recoveryAccount_request_object_type, change_recoveryAccount_request_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         change_recoveryAccount_request_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type           id;

         account_name_type accountToRecover;
         account_name_type recoveryAccount;
         time_point_sec    effective_on;
   };

   struct by_name;
   struct by_proxy;
   struct by_last_post;
   struct by_nextSCOREwithdrawalTime;
   struct by_TMEbalance;
   struct by_SCORE_balance;
   struct by_TSDbalance;
   struct by_post_count;
   struct by_vote_count;

   /**
    * @ingroup object_index
    */
   typedef multi_index_container<
      account_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< account_object, account_id_type, &account_object::id > >,
         ordered_unique< tag< by_name >,
            member< account_object, account_name_type, &account_object::name > >,
         ordered_unique< tag< by_proxy >,
            composite_key< account_object,
               member< account_object, account_name_type, &account_object::proxy >,
               member< account_object, account_id_type, &account_object::id >
            > /// composite key by proxy
         >,
         ordered_unique< tag< by_nextSCOREwithdrawalTime >,
            composite_key< account_object,
               member< account_object, time_point_sec, &account_object::nextSCOREwithdrawalTime >,
               member< account_object, account_id_type, &account_object::id >
            > /// composite key by_nextSCOREwithdrawalTime
         >,
         ordered_unique< tag< by_last_post >,
            composite_key< account_object,
               member< account_object, time_point_sec, &account_object::last_post >,
               member< account_object, account_id_type, &account_object::id >
            >,
            composite_key_compare< std::greater< time_point_sec >, std::less< account_id_type > >
         >,
         ordered_unique< tag< by_TMEbalance >,
            composite_key< account_object,
               member< account_object, asset, &account_object::balance >,
               member< account_object, account_id_type, &account_object::id >
            >,
            composite_key_compare< std::greater< asset >, std::less< account_id_type > >
         >,
         ordered_unique< tag< by_SCORE_balance >,
            composite_key< account_object,
               member< account_object, asset, &account_object::SCORE >,
               member< account_object, account_id_type, &account_object::id >
            >,
            composite_key_compare< std::greater< asset >, std::less< account_id_type > >
         >,
         ordered_unique< tag< by_TSDbalance >,
            composite_key< account_object,
               member< account_object, asset, &account_object::TSDbalance >,
               member< account_object, account_id_type, &account_object::id >
            >,
            composite_key_compare< std::greater< asset >, std::less< account_id_type > >
         >,
         ordered_unique< tag< by_post_count >,
            composite_key< account_object,
               member< account_object, uint32_t, &account_object::post_count >,
               member< account_object, account_id_type, &account_object::id >
            >,
            composite_key_compare< std::greater< uint32_t >, std::less< account_id_type > >
         >,
         ordered_unique< tag< by_vote_count >,
            composite_key< account_object,
               member< account_object, uint32_t, &account_object::lifetime_vote_count >,
               member< account_object, account_id_type, &account_object::id >
            >,
            composite_key_compare< std::greater< uint32_t >, std::less< account_id_type > >
         >
      >,
      allocator< account_object >
   > account_index;

   struct by_account;
   struct by_last_valid;

   typedef multi_index_container <
      owner_authority_history_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< owner_authority_history_object, owner_authority_history_id_type, &owner_authority_history_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< owner_authority_history_object,
               member< owner_authority_history_object, account_name_type, &owner_authority_history_object::account >,
               member< owner_authority_history_object, time_point_sec, &owner_authority_history_object::last_valid_time >,
               member< owner_authority_history_object, owner_authority_history_id_type, &owner_authority_history_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< time_point_sec >, std::less< owner_authority_history_id_type > >
         >
      >,
      allocator< owner_authority_history_object >
   > owner_authority_history_index;

   struct by_last_owner_update;

   typedef multi_index_container <
      account_authority_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_authority_object, account_authority_id_type, &account_authority_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< account_authority_object,
               member< account_authority_object, account_name_type, &account_authority_object::account >,
               member< account_authority_object, account_authority_id_type, &account_authority_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_authority_id_type > >
         >,
         ordered_unique< tag< by_last_owner_update >,
            composite_key< account_authority_object,
               member< account_authority_object, time_point_sec, &account_authority_object::last_owner_update >,
               member< account_authority_object, account_authority_id_type, &account_authority_object::id >
            >,
            composite_key_compare< std::greater< time_point_sec >, std::less< account_authority_id_type > >
         >
      >,
      allocator< account_authority_object >
   > account_authority_index;

   struct by_delegation;

   typedef multi_index_container <
      TME_fund_for_SCORE_delegation_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< TME_fund_for_SCORE_delegation_object, TME_fund_for_SCORE_delegation_id_type, &TME_fund_for_SCORE_delegation_object::id > >,
         ordered_unique< tag< by_delegation >,
            composite_key< TME_fund_for_SCORE_delegation_object,
               member< TME_fund_for_SCORE_delegation_object, account_name_type, &TME_fund_for_SCORE_delegation_object::delegator >,
               member< TME_fund_for_SCORE_delegation_object, account_name_type, &TME_fund_for_SCORE_delegation_object::delegatee >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_name_type > >
         >
      >,
      allocator< TME_fund_for_SCORE_delegation_object >
   > TME_fund_for_SCORE_delegation_index;

   struct by_expiration;
   struct by_account_expiration;

   typedef multi_index_container <
      TME_fund_for_SCORE_delegation_expiration_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< TME_fund_for_SCORE_delegation_expiration_object, TME_fund_for_SCORE_delegation_expiration_id_type, &TME_fund_for_SCORE_delegation_expiration_object::id > >,
         ordered_unique< tag< by_expiration >,
            composite_key< TME_fund_for_SCORE_delegation_expiration_object,
               member< TME_fund_for_SCORE_delegation_expiration_object, time_point_sec, &TME_fund_for_SCORE_delegation_expiration_object::expiration >,
               member< TME_fund_for_SCORE_delegation_expiration_object, TME_fund_for_SCORE_delegation_expiration_id_type, &TME_fund_for_SCORE_delegation_expiration_object::id >
            >,
            composite_key_compare< std::less< time_point_sec >, std::less< TME_fund_for_SCORE_delegation_expiration_id_type > >
         >,
         ordered_unique< tag< by_account_expiration >,
            composite_key< TME_fund_for_SCORE_delegation_expiration_object,
               member< TME_fund_for_SCORE_delegation_expiration_object, account_name_type, &TME_fund_for_SCORE_delegation_expiration_object::delegator >,
               member< TME_fund_for_SCORE_delegation_expiration_object, time_point_sec, &TME_fund_for_SCORE_delegation_expiration_object::expiration >,
               member< TME_fund_for_SCORE_delegation_expiration_object, TME_fund_for_SCORE_delegation_expiration_id_type, &TME_fund_for_SCORE_delegation_expiration_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< time_point_sec >, std::less< TME_fund_for_SCORE_delegation_expiration_id_type > >
         >
      >,
      allocator< TME_fund_for_SCORE_delegation_expiration_object >
   > TME_fund_for_SCORE_delegation_expiration_index;

   struct by_expiration;

   typedef multi_index_container <
      account_recovery_request_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_recovery_request_object, account_recovery_request_id_type, &account_recovery_request_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< account_recovery_request_object,
               member< account_recovery_request_object, account_name_type, &account_recovery_request_object::accountToRecover >,
               member< account_recovery_request_object, account_recovery_request_id_type, &account_recovery_request_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_recovery_request_id_type > >
         >,
         ordered_unique< tag< by_expiration >,
            composite_key< account_recovery_request_object,
               member< account_recovery_request_object, time_point_sec, &account_recovery_request_object::expires >,
               member< account_recovery_request_object, account_recovery_request_id_type, &account_recovery_request_object::id >
            >,
            composite_key_compare< std::less< time_point_sec >, std::less< account_recovery_request_id_type > >
         >
      >,
      allocator< account_recovery_request_object >
   > account_recovery_request_index;

   struct by_effective_date;

   typedef multi_index_container <
      change_recoveryAccount_request_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< change_recoveryAccount_request_object, change_recoveryAccount_request_id_type, &change_recoveryAccount_request_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< change_recoveryAccount_request_object,
               member< change_recoveryAccount_request_object, account_name_type, &change_recoveryAccount_request_object::accountToRecover >,
               member< change_recoveryAccount_request_object, change_recoveryAccount_request_id_type, &change_recoveryAccount_request_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< change_recoveryAccount_request_id_type > >
         >,
         ordered_unique< tag< by_effective_date >,
            composite_key< change_recoveryAccount_request_object,
               member< change_recoveryAccount_request_object, time_point_sec, &change_recoveryAccount_request_object::effective_on >,
               member< change_recoveryAccount_request_object, change_recoveryAccount_request_id_type, &change_recoveryAccount_request_object::id >
            >,
            composite_key_compare< std::less< time_point_sec >, std::less< change_recoveryAccount_request_id_type > >
         >
      >,
      allocator< change_recoveryAccount_request_object >
   > change_recoveryAccount_request_index;
} }

FC_REFLECT( node::chain::account_object,
             (id)(name)(memoKey)(json)(proxy)(last_accountUpdate)
             (created)(mined)
             (owner_challenged)(active_challenged)(last_owner_proved)(last_active_proved)(recoveryAccount)(last_account_recovery)(reset_account)
             (comment_count)(lifetime_vote_count)(post_count)(can_vote)(voting_power)(last_vote_time)
             (balance)
             (TMEsavingsBalance)
             (TSDbalance)(TSD_seconds)(TSD_seconds_last_update)(TSD_last_interest_payment)
             (TSDsavingsBalance)(savings_TSD_seconds)(savings_TSD_seconds_last_update)(savings_TSD_last_interest_payment)(savings_withdraw_requests)
             (TMErewardBalance)(TSDrewardBalance)(SCORErewardBalance)(SCORErewardBalanceInTME)
             (SCORE)(SCOREDelegated)(SCOREreceived)
             (SCOREwithdrawRateInTME)(nextSCOREwithdrawalTime)(withdrawn)(to_withdraw)(withdraw_routes)
             (curationRewards)
             (posting_rewards)
             (proxied_SCOREfundTMEbalance_votes)(witnesses_voted_for)
             (last_post)(last_root_post)(post_bandwidth)
          )
CHAINBASE_SET_INDEX_TYPE( node::chain::account_object, node::chain::account_index )

FC_REFLECT( node::chain::account_authority_object,
             (id)(account)(owner)(active)(posting)(last_owner_update)
)
CHAINBASE_SET_INDEX_TYPE( node::chain::account_authority_object, node::chain::account_authority_index )

FC_REFLECT( node::chain::TME_fund_for_SCORE_delegation_object,
            (id)(delegator)(delegatee)(SCORE)(min_delegation_time) )
CHAINBASE_SET_INDEX_TYPE( node::chain::TME_fund_for_SCORE_delegation_object, node::chain::TME_fund_for_SCORE_delegation_index )

FC_REFLECT( node::chain::TME_fund_for_SCORE_delegation_expiration_object,
            (id)(delegator)(SCORE)(expiration) )
CHAINBASE_SET_INDEX_TYPE( node::chain::TME_fund_for_SCORE_delegation_expiration_object, node::chain::TME_fund_for_SCORE_delegation_expiration_index )

FC_REFLECT( node::chain::owner_authority_history_object,
             (id)(account)(previous_owner_authority)(last_valid_time)
          )
CHAINBASE_SET_INDEX_TYPE( node::chain::owner_authority_history_object, node::chain::owner_authority_history_index )

FC_REFLECT( node::chain::account_recovery_request_object,
             (id)(accountToRecover)(new_owner_authority)(expires)
          )
CHAINBASE_SET_INDEX_TYPE( node::chain::account_recovery_request_object, node::chain::account_recovery_request_index )

FC_REFLECT( node::chain::change_recoveryAccount_request_object,
             (id)(accountToRecover)(recoveryAccount)(effective_on)
          )
CHAINBASE_SET_INDEX_TYPE( node::chain::change_recoveryAccount_request_object, node::chain::change_recoveryAccount_request_index )
