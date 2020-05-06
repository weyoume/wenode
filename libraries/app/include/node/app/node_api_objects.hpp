#pragma once
#include <node/chain/node_objects.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/global_property_object.hpp>
#include <node/chain/history_object.hpp>
#include <node/chain/transaction_object.hpp>
#include <node/tags/tags_plugin.hpp>
#include <node/producer/producer_objects.hpp>

namespace node { namespace app {

using namespace node::chain;

typedef chain::change_recovery_account_request_object  change_recovery_account_request_api_obj;
typedef chain::block_summary_object                    block_summary_api_obj;
typedef chain::comment_vote_object                     comment_vote_api_obj;
typedef chain::comment_view_object                     comment_view_api_obj;
typedef chain::comment_share_object                    comment_share_api_obj;
typedef chain::unstake_asset_route_object              unstake_asset_route_api_obj;
typedef chain::decline_voting_rights_request_object    decline_voting_rights_request_api_obj;
typedef chain::producer_vote_object                    producer_vote_api_obj;
typedef chain::producer_schedule_object                producer_schedule_api_obj;
typedef chain::asset_delegation_object                 asset_delegation_api_obj;
typedef chain::asset_delegation_expiration_object      asset_delegation_expiration_api_obj;
typedef producer::account_bandwidth_object             account_bandwidth_api_obj;



//================================//
// ===== Global API Objects ===== //
//================================//



struct dynamic_global_property_api_obj : public dynamic_global_property_object
{
   dynamic_global_property_api_obj( const dynamic_global_property_object& gpo, const chain::database& db ) :
      dynamic_global_property_object( gpo )
   {
      if( db.has_index< producer::reserve_ratio_index >() )
      {
         const auto& r = db.find( producer::reserve_ratio_id_type() );

         if( BOOST_LIKELY( r != nullptr ) )
         {
            current_reserve_ratio = r->current_reserve_ratio;
            average_block_size = r->average_block_size;
            max_virtual_bandwidth = r->max_virtual_bandwidth;
         }
      }
   }

   dynamic_global_property_api_obj( const dynamic_global_property_object& gpo ) :
      dynamic_global_property_object( gpo ){}

   dynamic_global_property_api_obj(){}

   uint32_t    current_reserve_ratio = 0;
   uint64_t    average_block_size = 0;
   uint128_t   max_virtual_bandwidth = 0;
};



struct median_chain_property_api_obj
{
   median_chain_property_api_obj( const chain::median_chain_property_object& o ) :
      id( o.id ),
      account_creation_fee( o.account_creation_fee ),
      asset_coin_liquidity( o.asset_coin_liquidity ),
      asset_usd_liquidity( o.asset_usd_liquidity ),
      maximum_block_size( o.maximum_block_size ),
      pow_target_time( o.pow_target_time ),
      pow_decay_time( o.pow_decay_time ),
      txn_stake_decay_time( o.txn_stake_decay_time ),
      escrow_bond_percent( o.escrow_bond_percent ),
      credit_interest_rate( o.credit_interest_rate ),
      credit_open_ratio( o.credit_open_ratio ),
      credit_liquidation_ratio( o.credit_liquidation_ratio ),
      credit_min_interest( o.credit_min_interest ),
      credit_variable_interest( o.credit_variable_interest ),
      market_max_credit_ratio( o.market_max_credit_ratio ),
      margin_open_ratio( o.margin_open_ratio ),
      margin_liquidation_ratio( o.margin_liquidation_ratio ),
      maximum_asset_feed_publishers( o.maximum_asset_feed_publishers ),
      membership_base_price( o.membership_base_price ),
      membership_mid_price( o.membership_mid_price ),
      membership_top_price( o.membership_top_price ),
      author_reward_percent( o.author_reward_percent ),
      vote_reward_percent( o.vote_reward_percent ),
      view_reward_percent( o.view_reward_percent ),
      share_reward_percent( o.share_reward_percent ),
      comment_reward_percent( o.comment_reward_percent ),
      storage_reward_percent( o.storage_reward_percent ),
      moderator_reward_percent( o.moderator_reward_percent ),
      content_reward_decay_rate( o.content_reward_decay_rate ),
      content_reward_interval( o.content_reward_interval ),
      vote_reserve_rate( o.vote_reserve_rate ),
      view_reserve_rate( o.view_reserve_rate ),
      share_reserve_rate( o.share_reserve_rate ),
      comment_reserve_rate( o.comment_reserve_rate ),
      vote_recharge_time( o.vote_recharge_time ),
      view_recharge_time( o.view_recharge_time ),
      share_recharge_time( o.share_recharge_time ),
      comment_recharge_time( o.comment_recharge_time ),
      vote_curation_decay( o.vote_curation_decay ),
      view_curation_decay( o.view_curation_decay ),
      share_curation_decay( o.share_curation_decay ),
      comment_curation_decay( o.comment_curation_decay ),
      supernode_decay_time( o.supernode_decay_time ),
      enterprise_vote_percent_required( o.enterprise_vote_percent_required ),
      maximum_asset_whitelist_authorities( o.maximum_asset_whitelist_authorities ),
      max_stake_intervals( o.max_stake_intervals ),
      max_unstake_intervals( o.max_unstake_intervals ),
      max_exec_budget( o.max_exec_budget ){}

   median_chain_property_api_obj(){}

   median_chain_property_id_type         id;
   asset                                 account_creation_fee;                 ///< Minimum fee required to create a new account by staking.
   asset                                 asset_coin_liquidity;                 ///< Coin liquidity required to create a new asset.
   asset                                 asset_usd_liquidity;                  ///< USD liquidity required to create a new asset.
   uint64_t                              maximum_block_size;                   ///< The maximum block size of the network in bytes. No Upper bound on block size limit.
   fc::microseconds                      pow_target_time;                      ///< The targeted time for each proof of work
   fc::microseconds                      pow_decay_time;                       ///< Time over which proof of work output is averaged over
   fc::microseconds                      txn_stake_decay_time;                 ///< Time over which transaction stake is averaged over
   uint16_t                              escrow_bond_percent;                  ///< Percentage of an escrow transfer that is deposited for dispute resolution
   uint16_t                              credit_interest_rate;                 ///< The credit interest rate paid to holders of network credit assets.
   uint16_t                              credit_open_ratio;                    ///< The minimum required collateralization ratio for a credit loan to be opened. 
   uint16_t                              credit_liquidation_ratio;             ///< The minimum permissible collateralization ratio before a loan is liquidated. 
   uint16_t                              credit_min_interest;                  ///< The minimum component of credit pool interest rates. 
   uint16_t                              credit_variable_interest;             ///< The variable component of credit pool interest rates, applied at equal base and borrowed balances.
   uint16_t                              market_max_credit_ratio;              ///< The maximum percentage of core asset liquidity balances that can be loaned.
   uint16_t                              margin_open_ratio;                    ///< The minimum required collateralization ratio for a credit loan to be opened. 
   uint16_t                              margin_liquidation_ratio;             ///< The minimum permissible collateralization ratio before a loan is liquidated. 
   uint16_t                              maximum_asset_feed_publishers;        ///< The maximum number of accounts that can publish price feeds for a stablecoin.
   asset                                 membership_base_price;                ///< The price for standard membership per month.
   asset                                 membership_mid_price;                 ///< The price for Mezzanine membership per month.
   asset                                 membership_top_price;                 ///< The price for top level membership per month.
   uint32_t                              author_reward_percent;                ///< The percentage of content rewards distributed to post authors.
   uint32_t                              vote_reward_percent;                  ///< The percentage of content rewards distributed to post voters.
   uint32_t                              view_reward_percent;                  ///< The percentage of content rewards distributed to post viewers.
   uint32_t                              share_reward_percent;                 ///< The percentage of content rewards distributed to post sharers.
   uint32_t                              comment_reward_percent;               ///< The percentage of content rewards distributed to post commenters.
   uint32_t                              storage_reward_percent;               ///< The percentage of content rewards distributed to viewing supernodes.
   uint32_t                              moderator_reward_percent;             ///< The percentage of content rewards distributed to community moderators.
   fc::microseconds                      content_reward_decay_rate;            ///< The time over which content rewards are distributed
   fc::microseconds                      content_reward_interval;              ///< Time taken per distribution of content rewards.
   uint32_t                              vote_reserve_rate;                    ///< The number of votes regenerated per day.
   uint32_t                              view_reserve_rate;                    ///< The number of views regenerated per day.
   uint32_t                              share_reserve_rate;                   ///< The number of shares regenerated per day.
   uint32_t                              comment_reserve_rate;                 ///< The number of comments regenerated per day.
   fc::microseconds                      vote_recharge_time;                   ///< Time taken to fully recharge voting power.
   fc::microseconds                      view_recharge_time;                   ///< Time taken to fully recharge viewing power.
   fc::microseconds                      share_recharge_time;                  ///< Time taken to fully recharge sharing power.
   fc::microseconds                      comment_recharge_time;                ///< Time taken to fully recharge commenting power.
   fc::microseconds                      curation_auction_decay_time;          ///< time of curation reward decay after a post is created. 
   double                                vote_curation_decay;                  ///< Number of votes for the half life of voting curation reward decay.
   double                                view_curation_decay;                  ///< Number of views for the half life of viewer curation reward decay.
   double                                share_curation_decay;                 ///< Number of shares for the half life of sharing curation reward decay.
   double                                comment_curation_decay;               ///< Number of comments for the half life of comment curation reward decay.
   fc::microseconds                      supernode_decay_time;                 ///< Amount of time to average the supernode file weight over. 
   uint16_t                              enterprise_vote_percent_required;     ///< Percentage of total voting power required to approve enterprise milestones. 
   uint64_t                              maximum_asset_whitelist_authorities;  ///< The maximum amount of whitelisted or blacklisted authorities for user issued assets 
   uint8_t                               max_stake_intervals;                  ///< Maximum weeks that an asset can stake over.
   uint8_t                               max_unstake_intervals;                ///< Maximum weeks that an asset can unstake over.
   asset                                 max_exec_budget;                      ///< Maximum budget that an executive board can claim.
};



struct reward_fund_api_obj
{
   reward_fund_api_obj( const chain::reward_fund_object& o ) :
      id( o.id ),
      symbol( o.symbol ),
      content_reward_balance( o.content_reward_balance ),
      validation_reward_balance( o.validation_reward_balance ),
      txn_stake_reward_balance( o.txn_stake_reward_balance ),
      work_reward_balance( o.work_reward_balance ),
      producer_activity_reward_balance( o.producer_activity_reward_balance ),
      supernode_reward_balance( o.supernode_reward_balance ),
      power_reward_balance( o.power_reward_balance ),
      community_fund_balance( o.community_fund_balance ),
      development_reward_balance( o.development_reward_balance ),
      marketing_reward_balance( o.marketing_reward_balance ),
      advocacy_reward_balance( o.advocacy_reward_balance ),
      activity_reward_balance( o.activity_reward_balance ),
      premium_partners_fund_balance( o.premium_partners_fund_balance ),
      total_pending_reward_balance( o.total_pending_reward_balance ),
      recent_content_claims( o.recent_content_claims ),
      recent_activity_claims( o.recent_activity_claims ),
      content_constant( o.content_constant ),
      content_reward_decay_rate( o.content_reward_decay_rate ),
      content_reward_interval( o.content_reward_interval ),
      author_reward_curve( o.author_reward_curve ),
      curation_reward_curve( o.curation_reward_curve ),
      last_updated( o.last_updated ){}

   reward_fund_api_obj(){}

   reward_fund_id_type     id;
   asset_symbol_type       symbol;                                                    ///< Currency symbol of the asset that the reward fund issues.
   asset                   content_reward_balance;                                    ///< Balance awaiting distribution to content creators.
   asset                   validation_reward_balance;                                 ///< Balance distributed to block validating producers. 
   asset                   txn_stake_reward_balance;                                  ///< Balance distributed to block producers based on the stake weighted transactions in each block.
   asset                   work_reward_balance;                                       ///< Balance distributed to each proof of work block producer. 
   asset                   producer_activity_reward_balance;                          ///< Balance distributed to producers that receive activity reward votes.
   asset                   supernode_reward_balance;                                  ///< Balance distributed to supernodes, based on stake weighted comment views.
   asset                   power_reward_balance;                                      ///< Balance distributed to staked units of the currency.
   asset                   community_fund_balance;                                    ///< Balance distributed to community proposal funds on the currency. 
   asset                   development_reward_balance;                                ///< Balance distributed to elected developers. 
   asset                   marketing_reward_balance;                                  ///< Balance distributed to elected marketers. 
   asset                   advocacy_reward_balance;                                   ///< Balance distributed to elected advocates. 
   asset                   activity_reward_balance;                                   ///< Balance distributed to content creators that are active each day. 
   asset                   premium_partners_fund_balance;                             ///< Receives income from memberships, distributed to premium creators. 
   asset                   total_pending_reward_balance;                              ///< Total of all reward balances. 
   uint128_t               recent_content_claims;                                     ///< Recently claimed content reward balance shares.
   uint128_t               recent_activity_claims;                                    ///< Recently claimed activity reward balance shares.
   uint128_t               content_constant;                                          ///< Contstant added to content claim shares.
   fc::microseconds        content_reward_decay_rate;                                 ///< Time taken to distribute all content rewards.
   fc::microseconds        content_reward_interval;                                   ///< Time between each individual distribution of content rewards. 
   curve_id                author_reward_curve;                                       ///< Type of reward curve used for author content reward calculation. 
   curve_id                curation_reward_curve;                                     ///< Type of reward curve used for curation content reward calculation.
   time_point              last_updated;                                              ///< Time that the reward fund was last updated. 
};



//=================================//
// ===== Account API Objects ===== //
//=================================//



struct account_api_obj
{
   account_api_obj( const chain::account_object& a, const database& db ) :
      id( a.id ),
      name( a.name ),
      details( to_string( a.details ) ),
      url( to_string( a.url ) ),
      image( to_string( a.image ) ),
      json( to_string( a.json ) ),
      json_private( to_string( a.json_private ) ),
      first_name( to_string( a.first_name ) ),
      last_name( to_string( a.last_name ) ),
      gender( to_string( a.json_private ) ),
      date_of_birth( to_string( a.date_of_birth ) ),
      email( to_string( a.email ) ),
      phone( to_string( a.phone ) ),
      nationality( to_string( a.nationality ) ),
      pinned_permlink( to_string( a.pinned_permlink ) ),
      membership( membership_tier_values[ int( a.membership ) ] ),
      secure_public_key( a.secure_public_key ),
      connection_public_key( a.connection_public_key ),
      friend_public_key( a.friend_public_key ),
      companion_public_key( a.companion_public_key ),
      proxy( a.proxy ),
      registrar( a.registrar ),
      referrer( a.referrer ),
      recovery_account( a.recovery_account ),
      reset_account( a.reset_account ),
      membership_interface( a.membership_interface ),
      reset_account_delay_days( a.reset_account_delay_days ),
      referrer_rewards_percentage( a.referrer_rewards_percentage ),
      comment_count( a.comment_count ),
      follower_count( a.follower_count ),
      following_count( a.following_count ),
      post_vote_count( a.post_vote_count ),
      post_count( a.post_count ),
      voting_power( a.voting_power ),
      viewing_power( a.viewing_power ),
      sharing_power( a.sharing_power ),
      commenting_power( a.commenting_power ),
      savings_withdraw_requests( a.savings_withdraw_requests ),
      withdraw_routes( a.withdraw_routes ),
      posting_rewards( a.posting_rewards.value ),
      curation_rewards( a.curation_rewards.value ),
      moderation_rewards( a.moderation_rewards.value ),
      total_rewards( a.total_rewards.value ),
      author_reputation( a.author_reputation.value ),
      loan_default_balance( a.loan_default_balance ),
      recent_activity_claims( a.recent_activity_claims.value ),
      producer_vote_count( a.producer_vote_count ),
      officer_vote_count( a.officer_vote_count ),
      executive_board_vote_count( a.executive_board_vote_count ),
      governance_subscriptions( a.governance_subscriptions ),
      enterprise_approval_count( a.enterprise_approval_count ),
      recurring_membership( a.recurring_membership ),
      created( a.created ),
      membership_expiration( a.membership_expiration ),
      last_account_update( a.last_account_update ),
      last_vote_time( a.last_vote_time ),
      last_view_time( a.last_view_time ),
      last_share_time( a.last_share_time ),
      last_post( a.last_post ),
      last_root_post( a.last_root_post ),
      last_transfer_time( a.last_transfer_time ),
      last_activity_reward( a.last_activity_reward ),
      last_account_recovery( a.last_account_recovery ),
      last_community_created( a.last_community_created ),
      last_asset_created( a.last_asset_created ),
      mined( a.mined ),
      revenue_share( a.revenue_share ),
      can_vote( a.can_vote ),
      active( a.active )
      {
         const auto& auth = db.get< account_authority_object, by_account >( name );
         owner_auth = authority( auth.owner_auth );
         active_auth = authority( auth.active_auth );
         posting_auth = authority( auth.posting_auth );
         last_owner_update = auth.last_owner_update;

         if( db.has_index< producer::account_bandwidth_index >() )
         {
            auto bandwidth_ptr = db.find< producer::account_bandwidth_object, producer::by_account >( name );

            if( bandwidth_ptr != nullptr )
            {
               average_bandwidth = (bandwidth_ptr->average_bandwidth).value;
               lifetime_bandwidth = (bandwidth_ptr->lifetime_bandwidth).value;
               last_bandwidth_update = bandwidth_ptr->last_bandwidth_update;
            }

         }
         for( auto name : a.proxied )
         {
            proxied.push_back( name );
         }
      }

   account_api_obj(){}

   account_id_type                  id;
   account_name_type                name;                                  ///< Username of the account, lowercase letter and numbers and hyphens only.
   string                           details;                               ///< The account's Public details string.
   string                           url;                                   ///< The account's Public personal URL.
   string                           image;                                 ///< IPFS Reference of the Public profile image of the account.
   string                           json;                                  ///< The JSON string of additional Public profile information.
   string                           json_private;                          ///< The JSON string of additional encrypted profile information. Encrypted with connection key.
   string                           first_name;                            ///< Encrypted First name of the user. Encrypted with connection key.
   string                           last_name;                             ///< Encrypted Last name of the user. Encrypted with connection key.
   string                           gender;                                ///< Encrypted Gender of the user. Encrypted with connection key.
   string                           date_of_birth;                         ///< Encrypted Date of birth of the user. Format: DD-MM-YYYY. Encrypted with connection key.
   string                           email;                                 ///< Encrypted Email address of the user. Encrypted with connection key.
   string                           phone;                                 ///< Encrypted Phone Number of the user. Encrypted with connection key.
   string                           nationality;                           ///< Encrypted Country of user's residence. Encrypted with connection key.
   string                           pinned_permlink;                       ///< Post pinned to the top of the account's profile. 
   string                           membership;                            ///< Level of account membership.
   public_key_type                  secure_public_key;                     ///< Key used for receiving incoming encrypted direct messages and key exchanges.
   public_key_type                  connection_public_key;                 ///< Key used for encrypting posts for connection level visibility. 
   public_key_type                  friend_public_key;                     ///< Key used for encrypting posts for friend level visibility.
   public_key_type                  companion_public_key;                  ///< Key used for encrypting posts for companion level visibility.
   authority                        owner_auth;
   authority                        active_auth;
   authority                        posting_auth;
   account_name_type                proxy;                                 ///< Account that votes on behalf of this account
   vector< account_name_type>       proxied;                               ///< Accounts that have set this account to be their proxy voter.
   account_name_type                registrar;                             ///< The name of the account that created the account;
   account_name_type                referrer;                              ///< The name of the account that originally referred the account to be created;
   account_name_type                recovery_account;                      ///< Account that can request recovery using a recent owner key if compromised.  
   account_name_type                reset_account;                         ///< Account that has the ability to reset owner authority after specified days of inactivity.
   account_name_type                membership_interface;                  ///< Account of the last interface to sell a membership to the account.
   uint16_t                         reset_account_delay_days;
   uint16_t                         referrer_rewards_percentage;           ///< The percentage of registrar rewards that are directed to the referrer.
   uint32_t                         comment_count;
   uint32_t                         follower_count;
   uint32_t                         following_count;
   uint32_t                         post_vote_count;
   uint32_t                         post_count;
   uint16_t                         voting_power;                          ///< current voting power of this account, falls after every vote, recovers over time.
   uint16_t                         viewing_power;                         ///< current viewing power of this account, falls after every view, recovers over time.
   uint16_t                         sharing_power;                         ///< current sharing power of this account, falls after every share, recovers over time.
   uint16_t                         commenting_power;                      ///< current commenting power of this account, falls after every comment, recovers over time.
   uint8_t                          savings_withdraw_requests;
   uint16_t                         withdraw_routes;
   int64_t                          posting_rewards;                       ///< Rewards in core asset earned from author rewards.
   int64_t                          curation_rewards;                      ///< Rewards in core asset earned from voting, shares, views, and commenting
   int64_t                          moderation_rewards;                    ///< Rewards in core asset from moderation rewards. 
   int64_t                          total_rewards;                         ///< Rewards in core asset earned from all reward sources.
   int64_t                          author_reputation;                     ///< 0 to BLOCKCHAIN_PRECISION rating of the account, based on relative total rewards
   asset                            loan_default_balance;
   int64_t                          recent_activity_claims;
   uint16_t                         producer_vote_count;
   uint16_t                         officer_vote_count;                    ///< Number of network officers that the account has voted for.
   uint16_t                         executive_board_vote_count;            ///< Number of Executive boards that the account has voted for.
   uint16_t                         governance_subscriptions;              ///< Number of governance accounts that the account subscribes to.   
   uint16_t                         enterprise_approval_count;             ///< Number of Enterprise proposals that the account votes for. 
   uint16_t                         recurring_membership;                  ///< Amount of months membership should be automatically renewed for on expiration
   time_point                       created;                               ///< Time that the account was created.
   time_point                       membership_expiration;                 ///< Time that the account has its current membership subscription until.
   time_point                       last_account_update;                   ///< Time that the account's details were last updated.
   time_point                       last_vote_time;                        ///< Time that the account last voted on a comment.
   time_point                       last_view_time;                        ///< Time that the account last viewed a post.
   time_point                       last_share_time;                       ///< Time that the account last viewed a post.
   time_point                       last_post;                             ///< Time that the user most recently created a comment 
   time_point                       last_root_post;                        ///< Time that the account last created a post.
   time_point                       last_transfer_time;                    ///< Time that the account last sent a transfer or created a trading txn. 
   time_point                       last_activity_reward;
   time_point                       last_account_recovery;
   time_point                       last_community_created;
   time_point                       last_asset_created;
   time_point                       last_owner_update;
   int64_t                          average_bandwidth;
   int64_t                          lifetime_bandwidth;
   time_point                       last_bandwidth_update;
   bool                             mined;
   bool                             revenue_share;
   bool                             can_vote;
   bool                             active;
};



struct account_concise_api_obj
{
   account_concise_api_obj( const chain::account_object& a ):
      id( a.id ),
      name( a.name ),
      details( to_string( a.details ) ),
      image( to_string( a.image ) ),
      membership( membership_tier_values[ int( a.membership ) ] ),
      follower_count( a.follower_count ),
      following_count( a.following_count ),
      total_rewards( a.total_rewards.value ),
      author_reputation( a.author_reputation.value ),
      created( a.created ),
      active( a.active ){}
      
   account_concise_api_obj(){}

   account_id_type                  id;
   account_name_type                name;                                  ///< Username of the account, lowercase letter and numbers and hyphens only.
   string                           details;                               ///< User's account details.
   string                           image;                                 ///< Account's public profile image IPFS reference.
   string                           membership;                            ///< Level of account membership.
   uint32_t                         follower_count;                        ///< Number of account followers.
   uint32_t                         following_count;                       ///< Number of accounts that the account follows. 
   int64_t                          total_rewards;                         ///< Rewards in core asset earned from all reward sources.
   int64_t                          author_reputation;                     ///< 0 to BLOCKCHAIN_PRECISION rating of the account, based on relative total rewards
   time_point                       created;                               ///< Time that the account was created.
   bool                             active;
};


struct account_verification_api_obj
{
   account_verification_api_obj( const chain::account_verification_object& a ):
      id( a.id ),
      verifier_account( a.verifier_account ),
      verified_account( a.verified_account ),
      shared_image( to_string( a.shared_image ) ),
      created( a.created ),
      last_updated( a.last_updated ){}
      
   account_verification_api_obj(){}

   account_verification_id_type          id;
   account_name_type                     verifier_account;              ///< Name of the Account with the profile.
   account_name_type                     verified_account;              ///< Name of the account being verifed.
   string                                shared_image;                  ///< IPFS reference to an image containing both people and the current
   signature_type                        image_signature;               ///< Signature of shared_image, that validates to verified_profile_public_key.
   time_point                            created;                       ///< Time of verification.
   time_point                            last_updated;                  ///< Time that the verifcation was last updated. 
};


struct account_business_api_obj
{
   account_business_api_obj( const chain::account_business_object& a ):
      id( a.id ),
      account( a.account ),
      business_type( business_structure_values[ int( a.business_type ) ] ),
      business_public_key( a.business_public_key ),
      executive_board( a.executive_board ),
      officer_vote_threshold( a.officer_vote_threshold.value )
      {
         for( auto name : a.executive_votes )
         {
            executive_votes.push_back( std::make_pair( name.first, std::make_pair( executive_role_values[ int( name.second.first ) ] , name.second.second.value ) ) );
         }
         for( auto name : a.executives )
         {
            executives.push_back( name );
         }
         for( auto name : a.officer_votes )
         {
            officer_votes.push_back( std::make_pair( name.first, name.second.value ) );
         }
         for( auto name : a.officers )
         {
            officers.push_back( name );
         }
         for( auto name : a.members )
         {
            members.push_back( name );
         }
         for( auto symbol : a.equity_assets )
         {
            equity_assets.push_back( symbol );
         }
         for( auto symbol : a.credit_assets )
         {
            credit_assets.push_back( symbol );
         }
         for( auto symbol : a.equity_revenue_shares )
         {
            equity_revenue_shares.push_back( std::make_pair( symbol.first, symbol.second ) );
         }
         for( auto symbol : a.credit_revenue_shares )
         {
            credit_revenue_shares.push_back( std::make_pair( symbol.first, symbol.second ) );
         }
      }

   account_business_api_obj(){}

   account_business_id_type                                        id;
   account_name_type                                               account;                    ///< Username of the business account, lowercase letters only.
   string                                                          business_type;              ///< Type of business account, controls authorizations for transactions of different types.
   public_key_type                                                 business_public_key;        ///< Type of business account, controls authorizations for transactions of different types.
   executive_officer_set                                           executive_board;            ///< Set of highest voted executive accounts for each role.
   vector< pair< account_name_type, pair< string, int64_t > > >    executive_votes;            ///< Set of all executive names.
   vector< account_name_type >                                     executives;                 ///< Set of accounts voted as executives.
   vector< pair< account_name_type, int64_t > >                    officer_votes;              ///< Set of all officers in the business, and their supporting voting power.
   vector< account_name_type >                                     officers;                   ///< Set of accounts voted as officers.
   vector< account_name_type >                                     members;                    ///< Set of all members of the business.
   int64_t                                                         officer_vote_threshold;     ///< Amount of voting power required for an officer to be active. 
   vector<asset_symbol_type >                                      equity_assets;              ///< Set of equity assets that offer dividends and voting power over the business account's structure
   vector<asset_symbol_type >                                      credit_assets;              ///< Set of credit assets that offer interest and buybacks from the business account
   vector< pair < asset_symbol_type,uint16_t > >                   equity_revenue_shares;      ///< Holds a map of all equity assets that the account shares incoming revenue with, and percentages.
   vector< pair < asset_symbol_type,uint16_t > >                   credit_revenue_shares;      ///< Holds a map of all equity assets that the account shares incoming revenue with, and percentages.
};


struct account_executive_vote_api_obj
{
   account_executive_vote_api_obj( const chain::account_executive_vote_object& a ):
      id( a.id ),
      account( a.account ),
      business_account( a.business_account ),
      executive_account( a.executive_account ),
      role( executive_role_values[ int( a.role ) ] ),
      vote_rank( a.vote_rank ){}
      
   account_executive_vote_api_obj(){}

   account_executive_vote_id_type        id;
   account_name_type                     account;               ///< Username of the account, voting for the executive.
   account_name_type                     business_account;      ///< Name of the referred business account.
   account_name_type                     executive_account;     ///< Name of the executive account.
   string                                role;                  ///< Role voted in favor of.
   uint16_t                              vote_rank;             ///< The rank of the executive vote.
};


struct account_officer_vote_api_obj
{
   account_officer_vote_api_obj( const chain::account_officer_vote_object& a ):
      id( a.id ),
      account( a.account ),
      business_account( a.business_account ),
      officer_account( a.officer_account ),
      vote_rank( a.vote_rank ){}
      
   account_officer_vote_api_obj(){}

   account_officer_vote_id_type          id;
   account_name_type                     account;               ///< Username of the account, voting for the officer.
   account_name_type                     business_account;      ///< Name of the referred business account.
   account_name_type                     officer_account;       ///< Name of the officer account.
   uint16_t                              vote_rank;             ///< The rank of the officer vote.
};


struct account_permission_api_obj
{
   account_permission_api_obj( const chain::account_permission_object& a ):
      id( a.id )
      {
         whitelisted_accounts.reserve( a.whitelisted_accounts.size() );
         for( auto name : a.whitelisted_accounts )
         {
            whitelisted_accounts.push_back( name );
         }
         blacklisted_accounts.reserve( a.blacklisted_accounts.size() );
         for( auto name : a.blacklisted_accounts )
         {
            blacklisted_accounts.push_back( name );
         }
         whitelisted_assets.reserve( a.whitelisted_assets.size() );
         for( auto name : a.whitelisted_assets )
         {
            whitelisted_assets.push_back( name );
         }
         blacklisted_assets.reserve( a.blacklisted_assets.size() );
         for( auto name : a.blacklisted_assets )
         {
            blacklisted_assets.push_back( name );
         }
      }

   account_permission_api_obj(){}

   account_permission_id_type               id;
   account_name_type                        account;                       ///< Name of the account with permissions set.
   vector< account_name_type >              whitelisted_accounts;          ///< List of accounts that are able to send transfers to this account.
   vector< account_name_type >              blacklisted_accounts;          ///< List of accounts that are not able to receive transfers from this account.
   vector< asset_symbol_type >              whitelisted_assets;            ///< List of assets that the account has whitelisted to receieve transfers of. 
   vector< asset_symbol_type >              blacklisted_assets;            ///< List of assets that the account has blacklisted against incoming transfers.
};


struct account_request_api_obj
{
   account_request_api_obj( const chain::account_member_request_object& o ):
      id( o.id ),
      account( o.account ),
      business_account( o.business_account ),
      message( to_string( o.message ) ),
      expiration( o.expiration ){}

   account_request_api_obj(){}

   account_member_request_id_type          id;
   account_name_type                       account;
   account_name_type                       business_account;
   string                                  message;
   time_point                              expiration;
};



struct account_invite_api_obj
{
   account_invite_api_obj( const chain::account_member_invite_object& o ):
      id( o.id ),
      account( o.account ),
      business_account( o.business_account ),
      member( o.member ),
      message( to_string( o.message ) ),
      expiration( o.expiration ){}

   account_invite_api_obj(){}

   account_member_invite_id_type           id;
   account_name_type                       account;
   account_name_type                       business_account;
   account_name_type                       member;
   string                                  message;
   time_point                              expiration;
};



struct account_vesting_balance_api_obj
{
   account_vesting_balance_api_obj( const chain::account_vesting_balance_object& b ):
      id( b.id ),
      owner( b.owner ),
      symbol( b.symbol ),
      vesting_balance( b.vesting_balance.value ),
      vesting_time( b.vesting_time ){}

   account_vesting_balance_api_obj(){}

   account_vesting_balance_id_type        id;
   account_name_type                      owner;                      ///< Account that owns the vesting balance.
   asset_symbol_type                      symbol;                     ///< Symbol of the asset that the balance corresponds to.
   int64_t                                vesting_balance;            ///< Balance that is locked until the vesting time. Cannot be used for voting.
   time_point                             vesting_time;               ///< Time at which the vesting balance will become liquid.
};



struct account_balance_api_obj
{
   account_balance_api_obj( const chain::account_balance_object& b ):
      id( b.id ),
      owner( b.owner ),
      symbol( b.symbol ),
      liquid_balance( b.liquid_balance.value ),
      staked_balance( b.staked_balance.value ),
      reward_balance( b.reward_balance.value ),
      savings_balance( b.savings_balance.value ),
      delegated_balance( b.delegated_balance.value ),
      receiving_balance( b.receiving_balance.value ),
      total_balance( b.total_balance.value ),
      stake_rate( b.stake_rate.value ),
      next_stake_time( b.next_stake_time ),
      to_stake( b.to_stake.value ),
      total_staked( b.total_staked.value ),
      unstake_rate( b.unstake_rate.value ),
      next_unstake_time( b.next_unstake_time ),
      to_unstake( b.to_unstake.value ),
      total_unstaked( b.total_unstaked.value ),
      last_interest_time( b.last_interest_time ){}

   account_balance_api_obj(){}

   account_balance_id_type        id;
   account_name_type              owner;
   asset_symbol_type              symbol;
   int64_t                        liquid_balance;             ///< Balance that can be freely transferred.
   int64_t                        staked_balance;             ///< Balance that cannot be transferred, and is vested in the account for a period of time.
   int64_t                        reward_balance;             ///< Balance that is newly issued from the network.
   int64_t                        savings_balance;            ///< Balance that cannot be transferred, and must be withdrawn after a delay period. 
   int64_t                        delegated_balance;          ///< Balance that is delegated to other accounts for voting power.
   int64_t                        receiving_balance;          ///< Balance that has been delegated to the account by other delegators. 
   int64_t                        total_balance;              ///< The total of all balances
   int64_t                        stake_rate;                 ///< Amount of liquid balance that is being staked from the liquid balance to the staked balance.  
   time_point                     next_stake_time;            ///< time at which the stake rate will be transferred from liquid to staked. 
   int64_t                        to_stake;                   ///< total amount to stake over the staking period. 
   int64_t                        total_staked;               ///< total amount that has been staked so far. 
   int64_t                        unstake_rate;               ///< Amount of staked balance that is being unstaked from the staked balance to the liquid balance.  
   time_point                     next_unstake_time;          ///< time at which the unstake rate will be transferred from staked to liquid. 
   int64_t                        to_unstake;                 ///< total amount to unstake over the withdrawal period. 
   int64_t                        total_unstaked;             ///< total amount that has been unstaked so far. 
   time_point                     last_interest_time;         ///< Last time that interest was compounded.
};



struct account_following_api_obj
{
   account_following_api_obj( const chain::account_following_object& a ):
      id( a.id ),
      account( a.account ),
      last_updated( a.last_updated )
      { 
         for( auto name : a.followers )
         {
            followers.push_back( name );
         }
         for( auto name : a.following )
         {
            following.push_back( name );
         }
         for( auto name : a.mutual_followers )
         {
            mutual_followers.push_back( name );
         }
         for( auto name : a.connections )
         {
            connections.push_back( name );
         }
         for( auto name : a.friends )
         {
            friends.push_back( name );
         }
         for( auto name : a.companions )
         {
            companions.push_back( name );
         }
         for( auto community : a.followed_communities )
         {
            followed_communities.push_back( community );
         }
         for( auto tag : a.followed_tags )
         {
            followed_tags.push_back( tag );
         }
         for( auto name : a.filtered )
         {
            filtered.push_back( name );
         }
         for( auto name : a.filtered_communities )
         {
            filtered_communities.push_back( name );
         }
         for( auto name : a.filtered_tags )
         {
            filtered_tags.push_back( name );
         }
      }

   account_following_api_obj(){}

   account_following_id_type       id;
   account_name_type               account;                ///< Name of the account.
   vector< account_name_type >     followers;              ///< Accounts that follow this account.
   vector< account_name_type >     following;              ///< Accounts that this account follows.
   vector< account_name_type >     mutual_followers;       ///< Accounts that are both following and followers of this account.
   vector< account_name_type >     connections;            ///< Accounts that are connections of this account.
   vector< account_name_type >     friends;                ///< Accounts that are friends of this account.
   vector< account_name_type >     companions;             ///< Accounts that are companions of this account.
   vector< community_name_type >   followed_communities;   ///< Communities that the account subscribes to.
   vector< tag_name_type >         followed_tags;          ///< Tags that the account follows.
   vector< account_name_type >     filtered;               ///< Accounts that this account has filtered. Interfaces should not show posts by these users.
   vector< community_name_type >   filtered_communities;   ///< Communities that this account has filtered. Posts will not display if they are in these communities.
   vector< tag_name_type >         filtered_tags;          ///< Tags that this account has filtered. Posts will not display if they have any of these tags. 
   time_point                      last_updated;           ///< Last time that the account changed its following sets.
};



struct tag_following_api_obj
{
   tag_following_api_obj( const chain::tag_following_object& t ):
      id( t.id ),
      tag( t.tag ),
      last_updated( t.last_updated )
      {
         followers.reserve( t.followers.size() );
         for( auto name : t.followers )
         {
            followers.push_back( name );
         }
      }

   tag_following_api_obj(){}

   tag_following_id_type             id;
   tag_name_type                     tag;                  ///< Name of the account.
   vector< account_name_type >       followers;            ///< Accounts that follow this account. 
   time_point                        last_updated;          ///< Last time that the tag changed its following sets.
};


struct connection_api_obj
{
   connection_api_obj( const chain::connection_object& c ):
      id( c.id ),
      account_a( c.account_a ),
      encrypted_key_a( c.encrypted_key_a ),
      account_b( c.account_b ),
      encrypted_key_b( c.encrypted_key_b ),
      connection_type( connection_tier_values[ int( c.connection_type ) ] ),
      connection_id( to_string( c.connection_id ) ),
      message_count( c.message_count ),
      consecutive_days( c.consecutive_days ),
      last_message_time_a( c.last_message_time_a ),
      last_message_time_b( c.last_message_time_b ),
      last_updated( c.last_updated ),
      created( c.created ){}

   connection_api_obj(){}

   connection_id_type           id;                 
   account_name_type            account_a;                ///< Account with the lower ID.
   encrypted_keypair_type       encrypted_key_a;          ///< A's private connection key, encrypted with the public secure key of account B.
   account_name_type            account_b;                ///< Account with the greater ID.
   encrypted_keypair_type       encrypted_key_b;          ///< B's private connection key, encrypted with the public secure key of account A.
   string                       connection_type;          ///< The connection level shared in this object
   string                       connection_id;            ///< Unique uuidv4 for the connection, for local storage of decryption key.
   uint32_t                     message_count;      ///< Number of total messages sent between connections
   uint32_t                     consecutive_days;         ///< Number of consecutive days that the connected accounts have both sent a message.
   time_point                   last_message_time_a;      ///< Time since the account A last sent a message
   time_point                   last_message_time_b;      ///< Time since the account B last sent a message
   time_point                   last_updated;             ///< Time that the connection was lat updated.
   time_point                   created;                  ///< Time the connection was created. 
};


struct connection_request_api_obj
{
   connection_request_api_obj( const chain::connection_request_object& c ):
      id( c.id ),
      account( c.account ),
      requested_account( c.requested_account ),
      connection_type( connection_tier_values[ int( c.connection_type ) ] ),
      message( to_string( c.message ) ),
      expiration( c.expiration ){}

   connection_request_api_obj(){}

   connection_request_id_type              id;                 
   account_name_type                       account;               ///< Account that created the request
   account_name_type                       requested_account;  
   string                                  connection_type;
   string                                  message;
   time_point                              expiration;   
};


struct owner_authority_history_api_obj
{
   owner_authority_history_api_obj( const chain::owner_authority_history_object& o ) :
      id( o.id ),
      account( o.account ),
      previous_owner_authority( authority( o.previous_owner_authority ) ),
      last_valid_time( o.last_valid_time ){}
   
   owner_authority_history_api_obj(){}

   owner_authority_history_id_type      id;
   account_name_type                    account;
   authority                            previous_owner_authority;
   time_point                           last_valid_time;
};



struct account_recovery_request_api_obj
{
   account_recovery_request_api_obj( const chain::account_recovery_request_object& o ) :
      id( o.id ),
      account_to_recover( o.account_to_recover ),
      new_owner_authority( authority( o.new_owner_authority ) ),
      expiration( o.expiration ){}

   account_recovery_request_api_obj() {}

   account_recovery_request_id_type     id;
   account_name_type                    account_to_recover;
   authority                            new_owner_authority;
   time_point                           expiration;
};



//=================================//
// ===== Network API Objects ===== //
//=================================//



struct network_officer_api_obj
{
   network_officer_api_obj( const chain::network_officer_object& o ):
      id( o.id ),
      account( o.account ),
      active( o.active ),
      officer_approved( o.officer_approved ),
      officer_type( network_officer_role_values[ int( o.officer_type ) ] ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      created( o.created ),
      vote_count( o.vote_count ),
      voting_power( o.voting_power.value ),
      producer_vote_count( o.producer_vote_count ),
      producer_voting_power( o.producer_voting_power.value ){}
      
   network_officer_api_obj(){}

   network_officer_id_type        id;
   account_name_type              account;                 ///< The name of the account that owns the network officer.
   bool                           active;                  ///< True if the officer is active, set false to deactivate.
   bool                           officer_approved;        ///< True when the network officer has received sufficient voting approval to earn funds.
   string                         officer_type;            ///< The type of network officer that the account serves as. 
   string                         details;                 ///< The officer's details description. 
   string                         url;                     ///< The officer's reference URL. 
   string                         json;                    ///< Json metadata of the officer. 
   time_point                     created;                 ///< The time the officer was created.
   uint32_t                       vote_count;              ///< The number of accounts that support the officer.
   int64_t                        voting_power;            ///< The amount of voting power that votes for the officer.
   uint32_t                       producer_vote_count;      ///< The number of accounts that support the officer.
   int64_t                        producer_voting_power;    ///< The amount of voting power that votes for the officer.
};


struct network_officer_vote_api_obj
{
   network_officer_vote_api_obj( const chain::network_officer_vote_object& o ):
      id( o.id ),
      account( o.account ),
      network_officer( o.network_officer ),
      officer_type( network_officer_role_values[ int( o.officer_type ) ] ),
      vote_rank( o.vote_rank ){}

   network_officer_vote_api_obj(){}

   network_officer_vote_id_type        id;
   account_name_type                   account;                    ///< The name of the account voting for the officer.
   account_name_type                   network_officer;            ///< The name of the network officer being voted for.
   string                              officer_type;               ///< the type of network officer that is being voted for.
   uint16_t                            vote_rank;                  ///< the ranking of the vote for the officer.
};


struct executive_board_api_obj
{
   executive_board_api_obj( const chain::executive_board_object& o ):
      id( o.id ),
      account( o.account ),
      active( o.active ),
      board_approved( o.board_approved ),
      budget( o.budget ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      created( o.created ),
      vote_count( o.vote_count ),
      voting_power( o.voting_power.value ),
      producer_vote_count( o.producer_vote_count ),
      producer_voting_power( o.producer_voting_power.value ){}

   executive_board_api_obj(){}

   executive_board_id_type        id;
   account_name_type              account;                    ///< The name of the governance account that created the executive team.
   bool                           active;                     ///< True if the executive team is active, set false to deactivate.
   bool                           board_approved;             ///< True when the board has reach sufficient voting support to receive budget.
   asset                          budget;                     ///< Total amount of Credit asset requested for team compensation and funding.
   string                         details;                    ///< The executive team's details description. 
   string                         url;                        ///< The executive team's reference URL. 
   string                         json;                       ///< Json metadata of the executive team. 
   time_point                     created;                    ///< The time the executive team was created.
   uint32_t                       vote_count;                 ///< The number of accounts that support the executive team.
   int64_t                        voting_power;               ///< The amount of voting power that votes for the executive team. 
   uint32_t                       producer_vote_count;        ///< The number of accounts that support the executive team.
   int64_t                        producer_voting_power;      ///< The amount of voting power that votes for the executive team.
};


struct executive_board_vote_api_obj
{
   executive_board_vote_api_obj( const chain::executive_board_vote_object& o ):
      id( o.id ),
      account( o.account ),
      executive_board( o.executive_board ),
      vote_rank( o.vote_rank ){}

   executive_board_vote_api_obj(){}

   executive_board_vote_id_type        id;
   account_name_type                   account;               ///< The name of the account that voting for the executive board.
   account_name_type                   executive_board;       ///< The name of the executive board being voted for.
   uint16_t                            vote_rank;             ///< The rank the rank of the vote for the executive board. 
};


struct governance_account_api_obj
{
   governance_account_api_obj( const chain::governance_account_object& o ):
      id( o.id ),
      account( o.account ),
      active( o.active ),
      account_approved( o.account_approved ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      created( o.created ),
      subscriber_count( o.subscriber_count ),
      subscriber_power( o.subscriber_power.value ),
      producer_subscriber_count( o.producer_subscriber_count ),
      producer_subscriber_power( o.producer_subscriber_power.value ){}

   governance_account_api_obj(){}

   governance_account_id_type     id;
   account_name_type              account;                     ///< The name of the governance account that created the governance account.
   bool                           active;                      ///< True if the governance account is active, set false to deactivate.
   bool                           account_approved;            ///< True when the governance account has reach sufficient voting support to receive budget.
   string                         details;                     ///< The governance account's details description. 
   string                         url;                         ///< The governance account's reference URL. 
   string                         json;                        ///< Json metadata of the governance account. 
   time_point                     created;                     ///< The time the governance account was created.
   uint32_t                       subscriber_count;            ///< The number of accounts that support the governance account.
   int64_t                        subscriber_power;            ///< The amount of voting power that votes for the governance account. 
   uint32_t                       producer_subscriber_count;   ///< The number of accounts that support the governance account.
   int64_t                        producer_subscriber_power;   ///< The amount of voting power that votes for the governance account.
};


struct governance_subscription_api_obj
{
   governance_subscription_api_obj( const chain::governance_subscription_object& o ):
      id( o.id ),
      account( o.account ),
      governance_account( o.governance_account ),
      vote_rank( o.vote_rank ){}

   governance_subscription_api_obj(){}

   governance_subscription_id_type     id;
   account_name_type                   account;                    ///< The name of the account that subscribes to the governance account.
   account_name_type                   governance_account;         ///< The name of the governance account being subscribed to.
   uint16_t                            vote_rank;                  ///< The preference rank of subscription for fee splitting.
};


struct supernode_api_obj
{
   supernode_api_obj( const chain::supernode_object& o ):
      id( o.id ),
      account( o.account ),
      active( o.active ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      node_api_endpoint( to_string( o.node_api_endpoint ) ),
      notification_api_endpoint( to_string( o.notification_api_endpoint ) ),
      auth_api_endpoint( to_string( o.auth_api_endpoint ) ),
      ipfs_endpoint( to_string( o.ipfs_endpoint ) ),
      bittorrent_endpoint( to_string( o.bittorrent_endpoint ) ),
      json( to_string( o.json ) ),
      created( o.created ),
      storage_rewards( o.storage_rewards ),
      daily_active_users( o.daily_active_users / PERCENT_100 ),
      monthly_active_users( o.monthly_active_users / PERCENT_100 ),
      recent_view_weight( o.recent_view_weight.value ),
      last_updated( o.last_updated ),
      last_activation_time( o.last_activation_time ){}
   
   supernode_api_obj(){}

   supernode_id_type       id;
   account_name_type       account;                     ///< The name of the account that owns the supernode.
   bool                    active;                      ///< True if the supernode is active, set false to deactivate.
   string                  details;                     ///< The supernode's details description. 
   string                  url;                         ///< The supernode's reference URL. 
   string                  node_api_endpoint;           ///< The Full Archive node public API endpoint of the supernode.
   string                  notification_api_endpoint;   ///< The Notification API endpoint of the Supernode. 
   string                  auth_api_endpoint;           ///< The Transaction signing authentication API endpoint of the supernode.
   string                  ipfs_endpoint;               ///< The IPFS file storage API endpoint of the supernode.
   string                  bittorrent_endpoint;         ///< The Bittorrent Seed Box endpoint URL of the Supernode. 
   string                  json;                        ///< Json metadata of the supernode, including additional outside of consensus APIs and services. 
   time_point              created;                     ///< The time the supernode was created.
   asset                   storage_rewards;             ///< Amount of core asset earned from storage.
   uint64_t                daily_active_users;          ///< The average number of accounts (X percent 100) that have used files from the node in the prior 24h.
   uint64_t                monthly_active_users;        ///< The average number of accounts (X percent 100) that have used files from the node in the prior 30 days.
   int64_t                 recent_view_weight;          ///< The rolling 7 day average of daily accumulated voting power of viewers. 
   time_point              last_updated;                ///< The time the file weight and active users was last decayed.
   time_point              last_activation_time;        ///< The time the Supernode was last reactivated, must be at least 24h ago to claim rewards.
};


struct interface_api_obj
{
   interface_api_obj( const chain::interface_object& o ):
      id( o.id ),
      account( o.account ),
      active( o.active ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      created( o.created ),
      daily_active_users( o.daily_active_users / PERCENT_100 ),
      monthly_active_users( o.monthly_active_users / PERCENT_100 ),
      last_updated( o.last_updated ){}
   
   interface_api_obj(){}

   interface_id_type       id;
   account_name_type       account;                     ///< The name of the account that owns the interface.
   bool                    active;                      ///< True if the interface is active, set false to deactivate.
   string                  details;                     ///< The interface's details description. 
   string                  url;                         ///< The interface's reference URL. 
   string                  json;                        ///< Json metadata of the interface, including additional outside of consensus APIs and services. 
   time_point              created;                     ///< The time the interface was created.
   uint64_t                daily_active_users;          ///< The average number of accounts (X percent 100) that have used files from the node in the prior 24h.
   uint64_t                monthly_active_users;        ///< The average number of accounts (X percent 100) that have used files from the node in the prior 30 days.
   time_point              last_updated;                ///< The time the file weight and active users was last decayed.
};


struct mediator_api_obj
{
   mediator_api_obj( const chain::mediator_object& o ):
      id( o.id ),
      account( o.account ),
      active( o.active ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      mediator_bond( o.mediator_bond ),
      mediation_virtual_position( o.mediation_virtual_position ),
      last_escrow_from( o.last_escrow_from ),
      last_escrow_id( to_string( o.last_escrow_id ) ),
      created( o.created ),
      last_updated( o.last_updated ){}
   
   mediator_api_obj(){}

   mediator_id_type        id;
   account_name_type       account;                     ///< The name of the account that owns the mediator.
   bool                    active;                      ///< True if the mediator is active, set false to deactivate.
   string                  details;                     ///< The mediator's details description. 
   string                  url;                         ///< The mediator's reference URL. 
   string                  json;                        ///< Json metadata of the mediator, including additional outside of consensus APIs and services. 
   asset                   mediator_bond;               ///< Core Asset staked in mediation bond for selection.
   uint128_t               mediation_virtual_position;  ///< Quantitative ranking of selection for mediation.
   account_name_type       last_escrow_from;            ///< The sender of the most recently allocated escrow
   string                  last_escrow_id;              ///< Escrow uuidv4 of the most recently allocated escrow.
   time_point              created;                     ///< The time the mediator was created.
   time_point              last_updated;                ///< The time the mediator was last updated.
};


struct community_enterprise_api_obj
{
   community_enterprise_api_obj( const chain::community_enterprise_object& o ):
      id( o.id ),
      creator( o.creator ),
      enterprise_id( to_string( o.enterprise_id ) ),
      active( o.active ),
      proposal_type( proposal_distribution_values[ int( o.proposal_type ) ] ),
      approved_milestones( o.approved_milestones ),
      claimed_milestones( o.claimed_milestones ),
      investment( *o.investment ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      begin( o.begin ),
      end( o.end ),
      expiration( o.expiration ),
      daily_budget( o.daily_budget ),
      duration( o.duration ),
      pending_budget( o.pending_budget ),
      total_distributed( o.total_distributed ),
      days_paid( o.days_paid ),
      total_approvals( o.total_approvals ),
      total_voting_power( o.total_voting_power.value ),
      total_producer_approvals( o.total_producer_approvals ),
      total_producer_voting_power( o.total_producer_voting_power.value ),
      current_approvals( o.current_approvals ),
      current_voting_power( o.current_voting_power.value ),
      current_producer_approvals( o.current_producer_approvals ),
      current_producer_voting_power( o.current_producer_voting_power.value ),
      created( o.created )
      {
         for( auto beneficiary : o.beneficiaries )
         {
            beneficiaries[ beneficiary.first ] = beneficiary.second;
         }
         for( auto milestone : o.milestone_shares )
         {
            milestone_shares.push_back( milestone );
         }
         for( auto milestone : o.milestone_details )
         {
            milestone_details.push_back( to_string( milestone) );
         }
         for( auto milestone : o.milestone_history )
         {
            milestone_history.push_back( to_string( milestone ) );
         }
      }

   community_enterprise_api_obj(){}

   community_enterprise_id_type       id;
   account_name_type                  creator;                                    ///< The name of the governance account that created the community enterprise proposal.
   string                             enterprise_id;                              ///< UUIDv4 for referring to the proposal.
   bool                               active;                                     ///< True if the project is active, set false to deactivate.
   string                             proposal_type;                              ///< The type of proposal, determines release schedule.
   map< account_name_type, uint16_t > beneficiaries;                              ///< Map of account names and percentages of budget value.
   vector< uint16_t >                 milestone_shares;                           ///< Ordered vector of release milestone descriptions.
   vector< string >                   milestone_details;                          ///< Ordered vector of release milestone percentages of budget value.
   vector< string >                   milestone_history;                          ///< Ordered vector of the details of every claimed milestone.
   int16_t                            approved_milestones;                        ///< Number of the last approved milestone by the community.
   int16_t                            claimed_milestones;                         ///< Number of milestones claimed for release.  
   asset_symbol_type                  investment;                                 ///< Symbol of the asset to be purchased with the funding if the proposal is investment type. 
   string                             details;                                    ///< The proposals's details description. 
   string                             url;                                        ///< The proposals's reference URL. 
   string                             json;                                       ///< Json metadata of the proposal. 
   time_point                         begin;                                      ///< Enterprise proposal start time. If the proposal is not approved by the start time, it is rejected. 
   time_point                         end;                                        ///< Enterprise proposal end time. Determined by start plus remaining interval number of days.
   time_point                         expiration;                                 ///< Time that the proposal expires, and transfers all remaining pending budget back to the community fund. 
   asset                              daily_budget;                               ///< Daily amount of Core asset requested for project compensation and funding.
   uint16_t                           duration;                                   ///< Number of days that the proposal lasts for. 
   asset                              pending_budget;                             ///< Funds held in the proposal for release. 
   asset                              total_distributed;                          ///< Total amount of funds distributed for the proposal. 
   uint16_t                           days_paid;                                  ///< Number of days that the proposal has been paid for. 
   uint32_t                           total_approvals;                            ///< The overall number of accounts that support the enterprise proposal.
   int64_t                            total_voting_power;                         ///< The oveall amount of voting power that supports the enterprise proposal.
   uint32_t                           total_producer_approvals;                   ///< The overall number of top 50 producers that support the enterprise proposal.
   int64_t                            total_producer_voting_power;                ///< The overall amount of producer voting power that supports the enterprise proposal.
   uint32_t                           current_approvals;                          ///< The number of accounts that support the latest claimed milestone.
   int64_t                            current_voting_power;                       ///< The amount of voting power that supports the latest claimed milestone.
   uint32_t                           current_producer_approvals;                 ///< The number of top 50 producers that support the latest claimed milestone.
   int64_t                            current_producer_voting_power;              ///< The amount of producer voting power that supports the latest claimed milestone.
   time_point                         created;                                    ///< The time the proposal was created.
};

class enterprise_approval_object : public object< enterprise_approval_object_type, enterprise_approval_object >
   {
      enterprise_approval_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         enterprise_approval_object( Constructor&& c, allocator< Allocator > a ) :
         enterprise_id(a)
         {
            c( *this );
         }

         id_type                        id;

         account_name_type              account;                   ///< Account approving the enterprise Milestone.

         account_name_type              creator;                   ///< The name of the account that created the community enterprise proposal.

         shared_string                  enterprise_id;             ///< UUIDv4 referring to the proposal being claimed.

         uint16_t                       vote_rank;                 ///< The vote rank of the approval for enterprise.

         int16_t                        milestone;                 ///< Number of the milestone being approved for release.
   };

struct enterprise_approval_api_obj
{
   enterprise_approval_api_obj( const chain::enterprise_approval_object& o ):
      id( o.id ),
      account( o.account ),
      creator( o.creator ),
      enterprise_id( to_string( o.enterprise_id ) ),
      vote_rank( o.vote_rank ),
      milestone( o.milestone ){}

   enterprise_approval_api_obj(){}

   enterprise_approval_id_type       id;
   account_name_type                  account;                   ///< Account approving the enterprise Milestone.
   account_name_type                  creator;                   ///< The name of the account that created the community enterprise proposal.
   string                             enterprise_id;             ///< UUIDv4 referring to the proposal being claimed.
   uint16_t                           vote_rank;                 ///< The vote rank of the approval for enterprise.
   int16_t                            milestone;                 ///< Number of the milestone being approved for release.
};



//=================================//
// ===== Comment API Objects ===== //
//=================================//



struct comment_api_obj
{
   comment_api_obj( const chain::comment_object& o ) :
      id( o.id ),
      author( o.author ),
      permlink( to_string( o.permlink ) ),
      title( to_string( o.title ) ),
      post_type( post_format_values[ int( o.post_type ) ]),
      public_key( o.public_key ),
      encrypted( o.is_encrypted() ),
      reach( feed_reach_values[ int( o.reach ) ] ),
      reply_connection( connection_tier_values[ int( o.reply_connection ) ] ),
      community( o.community ),
      body( to_string( o.body ) ),
      url( to_string( o.url ) ),
      interface( o.interface ),
      rating( o.rating ),
      language( to_string( o.language ) ),
      root_comment( o.root_comment ),
      parent_author( o.parent_author ),
      parent_permlink( to_string( o.parent_permlink ) ),
      json( to_string( o.json ) ),    
      category( to_string( o.category ) ),
      latitude( o.latitude ),
      longitude( o.longitude ),
      comment_price( o.comment_price ),
      reply_price( o.reply_price ),
      premium_price( o.premium_price ),
      last_updated( o.last_updated ),
      created( o.created ),
      active( o.active ),
      last_payout( o.last_payout ),
      author_reputation( o.author_reputation.value ),
      depth( o.depth ),
      children( o.children ),
      net_votes( o.net_votes ),
      view_count( o.view_count ),
      share_count( o.share_count ),
      net_reward( o.net_reward.value ),
      vote_power( o.vote_power.value ),
      view_power( o.view_power.value ),
      share_power( o.share_power.value ),
      comment_power( o.comment_power.value ),
      cashout_time( o.cashout_time ),
      cashouts_received( o.cashouts_received ),
      total_vote_weight( o.total_vote_weight ),
      total_view_weight( o.total_view_weight ),
      total_share_weight( o.total_share_weight ),
      total_comment_weight( o.total_comment_weight ),
      total_payout_value( o.total_payout_value ),
      curator_payout_value( o.curator_payout_value ),
      beneficiary_payout_value( o.beneficiary_payout_value ),
      content_rewards( o.content_rewards ),
      percent_liquid( o.percent_liquid ),
      reward( o.reward.value ),
      weight( o.weight ),
      max_weight( o.max_weight ),
      max_accepted_payout( o.max_accepted_payout ),
      author_reward_percent( o.author_reward_percent ),
      vote_reward_percent( o.vote_reward_percent ),
      share_reward_percent( o.share_reward_percent ),
      comment_reward_percent( o.comment_reward_percent ),
      storage_reward_percent( o.storage_reward_percent ),
      moderator_reward_percent( o.moderator_reward_percent ),
      allow_replies( o.allow_replies ),
      allow_votes( o.allow_votes ),
      allow_views( o.allow_views ),
      allow_shares( o.allow_shares ),
      allow_curation_rewards( o.allow_curation_rewards ),
      root( o.root ),
      deleted( o.deleted )
      {
         for( auto i : o.ipfs )
         {
            ipfs.push_back( to_string( i ) );
         }
         for( auto m : o.magnet )
         {
            magnet.push_back( to_string( m ) );
         }
         for( auto tag: o.tags )
         {
            tags.push_back( tag );
         }
         for( auto pr: o.payments_received )
         {
            for( auto as : pr.second )
            {
               payments_received.push_back( std::make_pair( pr.first, as.second ) );
            }
         }
         for( auto& route : o.beneficiaries )
         {
            beneficiaries.push_back( route );
         }
      }
      
   comment_api_obj(){}

   comment_id_type                id;
   account_name_type              author;                       ///< Name of the account that created the post.
   string                         permlink;                     ///< Unique identifing string for the post.
   string                         title;                        ///< Name of the post for reference.
   string                         post_type;                    ///< The type of post that is being created, image, text, article, video etc. 
   public_key_type                public_key;                   ///< The public key used to encrypt the post, holders of the private key may decrypt.
   bool                           encrypted;                    ///< True if the post is encrypted for a specific audience. 
   string                         reach;                        ///< The reach of the post across followers, connections, friends and companions.
   string                         reply_connection;             ///< Level of connection that can reply to the comment. 
   community_name_type            community;                    ///< The name of the community to which the post is uploaded to. Null string if no community. 
   vector< tag_name_type >        tags;                         ///< Set of string tags for sorting the post by.
   string                         body;                         ///< String containing text for display when the post is opened.
   vector< string >               ipfs;                         ///< String containing a display image or video file as an IPFS file hash.
   vector< string >               magnet;                       ///< String containing a bittorrent magnet link to a file swarm.
   string                         url;                          ///< String containing a URL for the post to link to.
   account_name_type              interface;                    ///< Name of the interface account that was used to broadcast the transaction and view the post.
   uint16_t                       rating;                       ///< User nominated rating as to the maturity of the content, and display sensitivity. 
   string                         language;                     ///< String containing a two letter language code that the post is broadcast in.
   comment_id_type                root_comment;                 ///< The root post that the comment is an ancestor of. 
   account_name_type              parent_author;                ///< Account that created the post this post is replying to, empty if root post. 
   string                         parent_permlink;              ///< permlink of the post this post is replying to, empty if root post. 
   string                         json;                         ///< IPFS link to file containing - Json metadata of the Title, Link, and additional interface specific data relating to the post.
   string                         category;
   double                         latitude;                     ///< Latitude co-ordinates of the comment.
   double                         longitude;                    ///< Longitude co-ordinates of the comment.
   asset                          comment_price;                ///< The price paid to create a comment
   asset                          reply_price;                  ///< Price paid to create a reply to the post
   asset                          premium_price;                ///< Price paid to unlock premium post. 
   vector< pair< account_name_type, asset > >  payments_received;    ///< Map of all transfers received that referenced this comment. 
   vector< beneficiary_route_type > beneficiaries;
   time_point                     last_updated;                 ///< The time the comment was last edited by the author
   time_point                     created;                      ///< Time that the comment was created.
   time_point                     active;                       ///< The last time this post was replied to.
   time_point                     last_payout;                  ///< The last time that the post received a content reward payout
   int64_t                        author_reputation;            ///< Used to measure author lifetime rewards, relative to other accounts.
   uint16_t                       depth;                        ///< used to track max nested depth
   uint32_t                       children;                     ///< The total number of children, grandchildren, posts with this as root comment.
   int32_t                        net_votes;                    ///< The amount of upvotes, minus downvotes on the post.
   uint32_t                       view_count;                   ///< The amount of views on the post.
   uint32_t                       share_count;                  ///< The amount of shares on the post.
   int64_t                        net_reward;                   ///< Net reward is the sum of all vote, view, share and comment power.
   int64_t                        vote_power;                   ///< Sum of weighted voting power from votes.
   int64_t                        view_power;                   ///< Sum of weighted voting power from viewers.
   int64_t                        share_power;                  ///< Sum of weighted voting power from shares.
   int64_t                        comment_power;                ///< Sum of weighted voting power from comments.
   time_point                     cashout_time;                 ///< 24 hours from the weighted average of vote time
   uint32_t                       cashouts_received;            ///< Number of times that the comment has received content rewards
   uint128_t                      total_vote_weight;            ///< the total weight of votes, used to calculate pro-rata share of curation payouts
   uint128_t                      total_view_weight;            ///< the total weight of views, used to calculate pro-rata share of curation payouts
   uint128_t                      total_share_weight;           ///< the total weight of shares, used to calculate pro-rata share of curation payouts
   uint128_t                      total_comment_weight;         ///< the total weight of comments, used to calculate pro-rata share of curation payouts
   asset                          total_payout_value;           ///< The total payout this comment has received over time, measured in USD
   asset                          curator_payout_value;         
   asset                          beneficiary_payout_value;
   asset                          content_rewards;
   int64_t                        percent_liquid;
   int64_t                        reward;                       ///< The amount of reward_curve this comment is responsible for in its root post.
   uint128_t                      weight;                       ///< Used to define the comment curation reward this comment receives.
   uint128_t                      max_weight;                   ///< Used to define relative contribution of this comment to rewards.
   asset                          max_accepted_payout;          ///< USD value of the maximum payout this post will receive
   uint32_t                       author_reward_percent;
   uint32_t                       vote_reward_percent;
   uint32_t                       view_reward_percent;
   uint32_t                       share_reward_percent;
   uint32_t                       comment_reward_percent;
   uint32_t                       storage_reward_percent;
   uint32_t                       moderator_reward_percent;
   bool                           allow_replies;               ///< allows a post to receive replies.
   bool                           allow_votes;                 ///< allows a post to receive votes.
   bool                           allow_views;                 ///< allows a post to receive views.
   bool                           allow_shares;                ///< allows a post to receive shares.
   bool                           allow_curation_rewards;      ///< Allows a post to distribute curation rewards.
   bool                           root;                        ///< True if post is a root post. 
   bool                           deleted;                     ///< True if author selects to remove from display in all interfaces, removed from API node distribution, cannot be interacted with.
};



struct moderation_tag_api_obj
{
   moderation_tag_api_obj( const chain::moderation_tag_object& m ) :
      id( m.id ),
      moderator( m.moderator ),
      comment( m.comment ),
      community( m.community ),
      rating( m.rating ),
      details( to_string( m.details ) ),
      interface( m.interface ),
      filter( m.filter ),
      last_updated( m.last_updated ),
      created( m.created )
      {
         for( auto tag : m.tags )
         {
            tags.push_back( tag );
         }
      }

   moderation_tag_api_obj(){}

   moderation_tag_id_type         id;
   account_name_type              moderator;        ///< Name of the moderator or goverance account that created the comment tag.
   comment_id_type                comment;          ///< ID of the comment.
   community_name_type            community;        ///< The name of the community to which the post is uploaded to.
   vector< tag_name_type >        tags;             ///< Set of string tags for sorting the post by
   uint16_t                       rating;           ///< Moderator updated rating as to the maturity of the content, and display sensitivity. 
   string                         details;          ///< Explaination as to what rule the post is in contravention of and why it was tagged.
   account_name_type              interface;        ///< Name of the interface application that broadcasted the transaction.
   bool                           filter;           ///< True if the post should be filtered by the community or governance address subscribers. 
   time_point                     last_updated;     ///< Time the comment tag was last edited by the author.
   time_point                     created;          ///< Time that the comment tag was created.
};


struct message_api_obj
{
   message_api_obj( const chain::message_object& m ):
      id( m.id ),
      sender( m.sender ),
      recipient( m.recipient ),
      sender_public_key( m.sender_public_key ),
      recipient_public_key( m.recipient_public_key ),
      message( to_string( m.message ) ),
      json( to_string( m.json ) ),
      uuid( to_string( m.uuid ) ),
      last_updated( m.last_updated ),
      created( m.created ){}

   message_api_obj(){}

   message_id_type         id;
   account_name_type       sender;                   ///< Name of the message sender.
   account_name_type       recipient;                ///< Name of the intended message recipient.
   public_key_type         sender_public_key;        ///< Public secure key of the sender.
   public_key_type         recipient_public_key;     ///< Public secure key of the recipient.
   string                  message;                  ///< Encrypted private message ciphertext.
   string                  json;                     ///< Encrypted Message metadata.
   string                  uuid;                     ///< uuidv4 uniquely identifying the message for local storage.
   time_point              last_updated;             ///< Time the message was last changed, used to reload encrypted message storage.
   time_point              created;                  ///< Time the message was sent.
};


struct list_api_obj
{
   list_api_obj( const chain::list_object& o ) :
      id( o.id ),
      creator( o.creator ),
      list_id( to_string( o.list_id ) ),
      name( to_string( o.name ) ),
      last_updated( o.last_updated ),
      created( o.created )
      {
         for( auto id : o.accounts )
         {
            accounts.insert( id._id );
         }
         for( auto id : o.comments )
         {
            comments.insert( id._id );
         }
         for( auto id : o.communities )
         {
            communities.insert( id._id );
         }
         for( auto id : o.assets )
         {
            assets.insert( id._id );
         }
         for( auto id : o.products )
         {
            products.insert( id._id );
         }
         for( auto id : o.auctions )
         {
            auctions.insert( id._id );
         }
         for( auto id : o.nodes )
         {
            nodes.insert( id._id );
         }
         for( auto id : o.edges )
         {
            edges.insert( id._id );
         }
         for( auto id : o.node_types )
         {
            node_types.insert( id._id );
         }
         for( auto id : o.edge_types )
         {
            edge_types.insert( id._id );
         }
      }

   list_api_obj(){}

   list_id_type                   id;
   account_name_type              creator;             ///< Name of the account that created the list.
   string                         list_id;             ///< uuidv4 referring to the list.
   string                         name;                ///< Name of the list, unique for each account.
   set< int64_t >                 accounts;            ///< Account IDs within the list.
   set< int64_t >                 comments;            ///< Comment IDs within the list.
   set< int64_t >                 communities;         ///< Community IDs within the list.
   set< int64_t >                 assets;              ///< Asset IDs within the list.
   set< int64_t >                 products;            ///< Product IDs within the list.
   set< int64_t >                 auctions;            ///< Auction IDs within the list.
   set< int64_t >                 nodes;               ///< Graph node IDs within the list.
   set< int64_t >                 edges;               ///< Graph edge IDs within the list.
   set< int64_t >                 node_types;          ///< Graph node property IDs within the list.
   set< int64_t >                 edge_types;          ///< Graph edge property IDs within the list.
   time_point                     last_updated;        ///< Time the list was last edited by the creator.
   time_point                     created;             ///< Time that the comment tag was created.
};


struct poll_api_obj
{
   poll_api_obj( const chain::poll_object& o ) :
      id( o.id ),
      creator( o.creator ),
      poll_id( to_string( o.poll_id ) ),
      details( to_string( o.details ) ),
      completion_time( o.completion_time ),
      last_updated( o.last_updated ),
      created( o.created )
      {
         for( auto p : o.poll_options )
         {
            poll_options.push_back( to_string( p ) );
         }
      }

   poll_api_obj(){}

   poll_id_type                   id;
   account_name_type              creator;             ///< Name of the account that created the poll.
   string                         poll_id;             ///< uuidv4 referring to the list.
   string                         details;             ///< Text describing the question being asked.
   vector< string >               poll_options;        ///< Available poll voting options.
   time_point                     completion_time;     ///< Time the poll voting completes.
   time_point                     last_updated;        ///< Time the list was last edited by the creator.
   time_point                     created;             ///< Time that the comment tag was created.
};


struct poll_vote_api_obj
{
   poll_vote_api_obj( const chain::poll_vote_object& o ) :
      id( o.id ),
      voter( o.voter ),
      creator( o.creator ),
      poll_id( to_string( o.poll_id ) ),
      poll_option( to_string( o.poll_option ) ),
      last_updated( o.last_updated ),
      created( o.created ){}

   poll_vote_api_obj(){}

   poll_vote_id_type           id;
   account_name_type           voter;               ///< Name of the account that created the vote.
   account_name_type           creator;             ///< Name of the account that created the poll.
   string                      poll_id;             ///< uuidv4 referring to the poll.
   string                      poll_option;         ///< Poll option chosen.
   time_point                  last_updated;        ///< Time the vote was last edited by the voter.
   time_point                  created;             ///< Time that the vote was created.
};


struct blog_api_obj
{
   blog_api_obj( const chain::blog_object& o ):
      id( o.id ),
      account( o.account ),
      community( o.community ),
      tag( o.tag ),
      comment( o.comment ),
      blog_type( blog_reach_values[ int( o.blog_type ) ] ),
      first_shared_by( o.first_shared_by ),
      shares( o.shares ),
      blog_time( o.blog_time )
      {
         for( auto s : o.shared_by )
         {
            shared_by[ s.first ] =  s.second;
         }
      }
      
   blog_api_obj(){}

   blog_id_type                            id;
   account_name_type                       account;               ///< Blog or sharing account for account type blogs, null for other types.
   community_name_type                     community;             ///< Community posted or shared to for community type blogs.
   tag_name_type                           tag;                   ///< Tag posted or shared to for tag type blogs.          
   comment_id_type                         comment;               ///< Comment ID.
   map< account_name_type, time_point >    shared_by;             ///< Map of the times that accounts that have shared the comment in the blog.
   string                                  blog_type;             ///< Account, Community, or Tag blog.
   account_name_type                       first_shared_by;       ///< First account that shared the comment with the account, community or tag.
   uint32_t                                shares;                ///< Number of accounts that have shared the comment with account, community or tag.
   time_point                              blog_time;             ///< Latest time that the comment was shared on the account, community or tag.
};


struct feed_api_obj
{
   feed_api_obj( const chain::feed_object& o ):
      id( o.id ),
      account( o.account ),
      comment( o.comment ),
      feed_type( feed_reach_values[ int( o.feed_type ) ] ),
      first_shared_by( o.first_shared_by ),
      shares( o.shares ),
      feed_time( o.feed_time )
      {
         for( auto s : o.shared_by )
         {
            shared_by[ s.first ] = s.second;
         }
         for( auto b : o.communities )
         {
            for( auto s : b.second )
            {
               communities[ b.first ][ s.first ] = s.second;
            }
         }
         for( auto t : o.tags )
         {
            for( auto s : t.second )
            {
               tags[ t.first ][ s.first ] = s.second;
            }
         }
      }

   feed_api_obj(){}

   feed_id_type                                id;
   account_name_type                           account;               ///< Account that should see comment in their feed.
   comment_id_type                             comment;               ///< ID of comment being shared
   string                                      feed_type;             ///< Type of feed, follow, connection, community, tag etc. 
   map< account_name_type, time_point >        shared_by;             ///< Map of the times that accounts that have shared the comment.
   map< community_name_type, map< account_name_type, time_point > >   communities;  ///< Map of all communities that the comment has been shared with
   map< tag_name_type, map< account_name_type, time_point > >     tags;    ///< Map of all tags that the comment has been shared with.
   account_name_type                           first_shared_by;       ///< First account that shared the comment with account. 
   uint32_t                                    shares;                ///< Number of accounts that have shared the comment with account.
   time_point                                  feed_time;             ///< Time that the comment was added or last shared with account. 
};



//===================================//
// ===== Community API Objects ===== //
//===================================//



struct community_api_obj
{
   community_api_obj( const chain::community_object& b ):
      id( b.id ),
      name( b.name ),
      founder( b.founder ),
      community_privacy( community_privacy_values[ int( b.community_privacy ) ] ),
      community_public_key( b.community_public_key ),
      json( to_string( b.json ) ),
      json_private( to_string( b.json_private ) ),
      pinned_author( b.pinned_author ),
      pinned_permlink( to_string( b.pinned_permlink ) ),
      subscriber_count( b.subscriber_count ),
      post_count( b.post_count ),
      comment_count( b.comment_count ),
      vote_count( b.vote_count ),
      view_count( b.view_count ),
      share_count( b.share_count ),
      reward_currency( b.reward_currency ),
      created( b.created ),
      last_community_update( b.last_community_update ),
      last_post( b.last_post ),
      last_root_post( b.last_root_post ),
      active( b.active ){}

   community_api_obj(){}

   community_id_type                  id;
   community_name_type                name;                       ///< Name of the community, lowercase letters, numbers and hyphens only.
   account_name_type                  founder;                    ///< The account that created the community, able to add and remove administrators.
   string                             community_privacy;          ///< Community privacy level, open, public, private, or exclusive
   public_key_type                    community_public_key;       ///< Key used for encrypting and decrypting posts. Private key shared with accepted members.
   string                             json;                       ///< Public plaintext json information about the community, its topic and rules.
   string                             json_private;               ///< Private ciphertext json information about the community.
   account_name_type                  pinned_author;              ///< Author of the Post pinned to the top of the community's page. 
   string                             pinned_permlink;            ///< Permlink of the Post pinned to the top of the community's page. 
   uint32_t                           subscriber_count;           ///< number of accounts that are subscribed to the community
   uint32_t                           post_count;                 ///< number of posts created in the community
   uint32_t                           comment_count;              ///< number of comments on posts in the community
   uint32_t                           vote_count;                 ///< accumulated number of votes received by all posts in the community
   uint32_t                           view_count;                 ///< accumulated number of views on posts in the community 
   uint32_t                           share_count;                ///< accumulated number of shares on posts in the community 
   asset_symbol_type                  reward_currency;            ///< The Currency asset used for content rewards in the community. 
   time_point                         created;                    ///< Time that the community was created.
   time_point                         last_community_update;      ///< Time that the community's details were last updated.
   time_point                         last_post;                  ///< Time that the user most recently created a comment.
   time_point                         last_root_post;             ///< Time that the community last created a post. 
   bool                               active;                     ///< True when community is active.
};


struct community_request_api_obj
{
   community_request_api_obj( const chain::community_join_request_object& o ):
      id( o.id ),
      account( o.account ),
      community( o.community),
      message( to_string( o.message ) ),
      expiration( o.expiration ){}

   community_request_api_obj(){}

   community_join_request_id_type          id;                 
   account_name_type                       account;        
   community_name_type                     community;  
   string                                  message;
   time_point                              expiration;   
};


struct community_invite_api_obj
{
   community_invite_api_obj( const chain::community_join_invite_object& o ):
      id( o.id ),
      account( o.account ),
      community( o.community ),
      member( o.member ),
      message( to_string( o.message ) ),
      expiration( o.expiration ){}

   community_invite_api_obj(){}

   community_join_invite_id_type           id;                 
   account_name_type                       account;             
   community_name_type                     community; 
   account_name_type                       member;      
   string                                  message;
   time_point                              expiration;   
};


struct community_event_api_obj
{
   community_event_api_obj( const chain::community_event_object& o ):
      id( o.id ),
      account( o.account ),
      community( o.community),
      event_name( to_string( o.event_name ) ),
      location( to_string( o.location ) ),
      latitude( o.latitude ),
      longitude( o.longitude ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      event_start_time( o.event_start_time ),
      event_end_time( o.event_end_time ),
      last_updated( o.last_updated ),
      created( o.created )
      {
         for( auto name : o.interested )
         {
            interested.push_back( name );
         }
         for( auto name : o.attending )
         {
            attending.push_back( name );
         }
         for( auto name : o.not_attending )
         {
            not_attending.push_back( name );
         }
      }

   community_event_api_obj(){}

   community_event_id_type               id;                 
   account_name_type                     account;                ///< Account that created the event.
   community_name_type                   community;              ///< Community being invited to join.
   string                                event_name;             ///< The Name of the event.
   string                                location;               ///< Address location of the event.
   double                                latitude;               ///< Latitude co-ordinates of the event.
   double                                longitude;              ///< Longitude co-ordinates of the event.
   string                                details;                ///< Event details describing the purpose of the event.
   string                                url;                    ///< Reference URL for the event.
   string                                json;                   ///< Additional Event JSON data.
   vector< account_name_type >           invited;                ///< Members that are invited to the event, all members if empty.
   vector< account_name_type >           interested;             ///< Members that are interested in the event.
   vector< account_name_type >           attending;              ///< Members that have confirmed that they will be attending the event.
   vector< account_name_type >           not_attending;          ///< Members that have confirmed that they will not be attending the event.
   time_point                            event_start_time;       ///< Time that the Event will begin.
   time_point                            event_end_time;         ///< Time that the event will end.
   time_point                            last_updated;           ///< Time that the event was last updated.
   time_point                            created;                ///< Time that the event was created.
};



//=====================================//
// ===== Advertising API Objects ===== //
//=====================================//



struct ad_creative_api_obj
{
   ad_creative_api_obj( const chain::ad_creative_object& o ):
      id( o.id ),
      account( o.account ),
      creative_id( to_string( o.creative_id ) ),
      format_type( ad_format_values[ int( o.format_type ) ] ),
      author( o.author ),
      objective( to_string( o.objective ) ),
      creative( to_string( o.creative ) ),
      json( to_string( o.json ) ),
      created( o.created ),
      last_updated( o.last_updated ),
      active( o.active ){}

   ad_creative_api_obj(){}

   ad_creative_id_type         id;
   account_name_type           account;           ///< Name of the account creating the creative.
   string                      creative_id;       ///< The uuidv4 of the creative for reference
   string                      format_type;       ///< The type of formatting used for the ad, determines the interpretation of the creative and objective.
   account_name_type           author;            ///< Name of the account that created the objective.
   string                      objective;         ///< The name of the object being advertised, the link and CTA destination of the creative.
   string                      creative;          ///< IPFS link to the Media to be displayed, image or video.
   string                      json;              ///< Public plaintext json information about the creative, its topic and rules.
   time_point                  created;           ///< Time creative was made.
   time_point                  last_updated;      ///< Time creative's details were last updated.
   bool                        active;            ///< True when the creative is active for use in campaigns, false to deactivate.
};



struct ad_campaign_api_obj
{
   ad_campaign_api_obj( const chain::ad_campaign_object& o ):
      id( o.id ),
      account( o.account ),
      campaign_id( to_string( o.campaign_id ) ),
      budget( o.budget ),
      total_bids( o.total_bids ),
      begin( o.begin ),
      end( o.end ),
      json( to_string( o.json ) ),
      interface( o.interface ),
      created( o.created ),
      last_updated( o.last_updated ),
      active( o.active )
      {
         for( auto agent : o.agents )
         {
            agents.push_back( agent );
         }
      }

   ad_campaign_api_obj(){}

   ad_campaign_id_type              id;
   account_name_type                account;           ///< Account creating the ad campaign.
   string                           campaign_id;       ///< uuidv4 to refer to the campaign.
   asset                            budget;            ///< Total expenditure of the campaign.
   asset                            total_bids;        ///< Total amount of expenditure in active bids.
   time_point                       begin;             ///< Beginning time of the campaign. Bids cannot be created before this time.
   time_point                       end;               ///< Ending time of the campaign. Remaining campaign budget will be refunded after this time.
   string                           json;              ///< json metadata for the campaign.
   vector<account_name_type>        agents;            ///< Set of Accounts authorized to create bids for the campaign.
   account_name_type                interface;         ///< Interface that facilitated the purchase of the advertising campaign.
   time_point                       created;           ///< Time campaign was created.
   time_point                       last_updated;      ///< Time campaigns's details were last updated or inventory was delivered.
   bool                             active;            ///< True when active for bidding and delivery, false to deactivate.
};



struct ad_inventory_api_obj
{
   ad_inventory_api_obj( const chain::ad_inventory_object& o ):
      id( o.id ),
      provider( o.provider ),
      inventory_id( to_string( o.inventory_id ) ),
      metric( ad_metric_values[ int( o.metric ) ] ),
      audience_id( to_string( o.audience_id ) ),
      min_price( o.min_price ),
      inventory( o.inventory ),
      remaining( o.remaining ),
      json( to_string( o.json ) ),
      created( o.created ),
      last_updated( o.last_updated ),
      expiration( o.expiration ),
      active( o.active ){}
   
   ad_inventory_api_obj(){}

   ad_inventory_id_type             id;
   account_name_type                provider;          ///< Account creating the ad inventory.
   string                           inventory_id;      ///< uuidv4 to refer to the inventory.
   string                           metric;            ///< Type of expense metric used.
   string                           audience_id;       ///< ad audience_id, containing a set of usernames of viewing accounts in their userbase.
   asset                            min_price;         ///< Minimum bidding price per metric.
   uint32_t                         inventory;         ///< Total metrics available.
   uint32_t                         remaining;         ///< Current amount of inventory remaining. Decrements when delivered.
   string                           json;              ///< json metadata for the inventory.
   time_point                       created;           ///< Time inventory was created.
   time_point                       last_updated;      ///< Time inventorys's details were last updated or inventory was delivered.
   time_point                       expiration;        ///< Time that the inventory offering expires. All outstanding bids for the inventory also expire at this time. 
   bool                             active;            ///< True when active for bidding and delivery, false to deactivate.
};



struct ad_audience_api_obj
{
   ad_audience_api_obj( const chain::ad_audience_object& o ):
      id( o.id ),
      account( o.account ),
      audience_id( to_string( o.audience_id ) ),
      json( to_string( o.json ) ),
      created( o.created ),
      last_updated( o.last_updated ),
      active( o.active )
      {
         for( auto aud : o.audience )
         {
            audience.push_back( aud );
         }
      }

   ad_audience_api_obj(){}

   ad_audience_id_type              id;
   account_name_type                account;           ///< Account creating the ad audience.
   string                           audience_id;       ///< uuidv4 to refer to the audience.
   string                           json;              ///< json metadata for the audience.
   vector< account_name_type >      audience;          ///< List of usernames within the audience for campaigns and inventory.
   time_point                       created;           ///< Time audience was created.
   time_point                       last_updated;      ///< Time audiences's details were last updated.
   bool                             active;            ///< True when active for bidding and delivery, false to deactivate.
};



struct ad_bid_api_obj
{
   ad_bid_api_obj( const chain::ad_bid_object& o ):
      id( o.id ),
      bidder( o.bidder ),
      bid_id( to_string( o.bid_id ) ),
      audience_id( to_string( o.audience_id ) ),
      account( o.account ),
      campaign_id( to_string( o.campaign_id ) ),
      author( o.author ),
      creative_id( to_string( o.creative_id ) ),
      provider( o.provider ),
      inventory_id( to_string( o.inventory_id ) ),
      bid_price( o.bid_price ),
      format( ad_format_values[ int( o.format ) ] ),
      metric( ad_metric_values[ int( o.metric ) ] ),
      objective( to_string( o.objective ) ),
      requested( o.requested ),
      remaining( o.remaining ),
      json( to_string( o.json ) ),
      created( o.created ),
      last_updated( o.last_updated ),
      expiration( o.expiration )
      {
         for( auto d : o.delivered )
         {
            delivered.push_back( d );
         }
      }

   ad_bid_api_obj(){}

   ad_bid_id_type                   id;
   account_name_type                bidder;            ///< Account that created the ad budget, or an agent of the campaign.
   string                           bid_id;            ///< Bid uuidv4 for referring to the bid and updating it or cancelling it.
   string                           audience_id;       ///< Desired audience for display acceptance. Audience must include only members of the inventory audience.
   account_name_type                account;           ///< Account that created the campaign that this bid is directed towards.
   string                           campaign_id;       ///< Ad campaign uuidv4 to utilise for the bid.
   account_name_type                author;            ///< Account that created the creative that is being bidded on.
   string                           creative_id;       ///< Desired creative for display.
   account_name_type                provider;          ///< Account offering inventory supply.
   string                           inventory_id;      ///< Inventory uuidv4 offering to bid on.
   asset                            bid_price;         ///< Price offered per metric. Asset symbol must be the same as the inventory price.
   string                           format;            ///< Ad Creative format.
   string                           metric;            ///< Type of expense metric used.
   string                           objective;         ///< Creative Objective for bid rank ordering.
   uint32_t                         requested;         ///< Maximum total metrics requested.
   uint32_t                         remaining;         ///< Current amount of inventory remaining. Decrements when delivered.
   vector< account_name_type >      delivered;         ///< List of audience accounts that have been delivered creative.
   string                           json;              ///< JSON Metadata of the ad bid.
   time_point                       created;           ///< Time that the bid was created.
   time_point                       last_updated;      ///< Time that the bid's details were last updated or inventory was delivered.
   time_point                       expiration;        ///< Time that the bid was will expire.
};



//====================================//
// ===== Graph Data API Objects ===== //
//====================================//



struct graph_node_api_obj
{
   graph_node_api_obj( const chain::graph_node_object& o ):
      id( o.id ),
      account( o.account ),
      node_id( to_string( o.node_id ) ),
      name( to_string( o.name ) ),
      details( to_string( o.details ) ),
      json( to_string( o.json ) ),
      json_private( to_string( o.json_private ) ),
      node_public_key( o.node_public_key ),
      interface( o.interface ),
      created( o.created ),
      last_updated( o.last_updated )
      {
         for( auto t : o.node_types )
         {
            node_types.push_back( t );
         }
         for( auto t : o.attributes )
         {
            attributes.push_back( to_string( t ) );
         }
         for( auto t : o.attribute_values )
         {
            attribute_values.push_back( to_string( t ) );
         }
      }

   graph_node_api_obj(){}

   graph_node_id_type                      id;
   account_name_type                       account;                     ///< Name of the account that created the node.
   vector< graph_node_name_type >          node_types;                  ///< Set of Types of node being created, determines the required attributes.
   string                                  node_id;                     ///< uuidv4 identifying the node. Unique for each account.
   string                                  name;                        ///< Name of the node.
   string                                  details;                     ///< Describes the additional details of the node.
   vector< string >                        attributes;                  ///< List of attributes types for this node.
   vector< string >                        attribute_values;            ///< List of attribute values for this node.
   string                                  json;                        ///< Public plaintext JSON node attribute information.
   string                                  json_private;                ///< Private encrypted ciphertext JSON node attribute information.
   public_key_type                         node_public_key;             ///< Key used for encrypting and decrypting private node JSON data and attribute values.
   account_name_type                       interface;                   ///< Name of the application that facilitated the creation of the node.
   time_point                              created;                     ///< Time the node was created.
   time_point                              last_updated;                ///< Time that the node was last updated by its creator.
};



struct graph_edge_api_obj
{
   graph_edge_api_obj( const chain::graph_edge_object& o ):
      id( o.id ),
      account( o.account ),
      edge_id( to_string( o.edge_id ) ),
      from_node( o.from_node ),
      to_node( o.to_node ),
      name( to_string( o.name ) ),
      details( to_string( o.details ) ),
      json( to_string( o.json ) ),
      json_private( to_string( o.json_private ) ),
      edge_public_key( o.edge_public_key ),
      interface( o.interface ),
      created( o.created ),
      last_updated( o.last_updated )
      {
         for( auto t : o.edge_types )
         {
            edge_types.push_back( t );
         }
         for( auto t : o.attributes )
         {
            attributes.push_back( to_string( t ) );
         }
         for( auto t : o.attribute_values )
         {
            attribute_values.push_back( to_string( t ) );
         }
      }

   graph_edge_api_obj(){}

   graph_edge_id_type                       id;
   account_name_type                        account;                     ///< Name of the account that created the edge.
   vector< graph_edge_name_type >           edge_types;                  ///< Types of the edge being created.
   string                                   edge_id;                     ///< uuidv4 identifying the edge.
   graph_node_id_type                       from_node;                   ///< The Base connecting node.
   graph_node_id_type                       to_node;                     ///< The Node being connected to.
   string                                   name;                        ///< Name of the edge.
   string                                   details;                     ///< Describes the edge.
   vector< string >                         attributes;                  ///< List of attributes types for this edge.
   vector< string >                         attribute_values;            ///< List of attribute values for this edge.
   string                                   json;                        ///< Public plaintext JSON edge attribute information.
   string                                   json_private;                ///< Private encrypted ciphertext JSON edge attribute information.
   public_key_type                          edge_public_key;             ///< Key used for encrypting and decrypting private edge JSON data.
   account_name_type                        interface;                   ///< Name of the application that facilitated the creation of the edge.
   time_point                               created;                     ///< Time the edge was created.
   time_point                               last_updated;                ///< Time that the edge was last updated by its creator.
};



struct graph_node_property_api_obj
{
   graph_node_property_api_obj( const chain::graph_node_property_object& o ):
      id( o.id ),
      account( o.account ),
      node_type( o.node_type ),
      graph_privacy( connection_tier_values[ int( o.graph_privacy ) ] ),
      edge_permission( connection_tier_values[ int( o.edge_permission ) ] ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      interface( o.interface ),
      created( o.created ),
      last_updated( o.last_updated )
      {
         for( auto t : o.attributes )
         {
            attributes.push_back( to_string( t ) );
         }
      }

   graph_node_property_api_obj(){}

   graph_node_property_id_type        id;
   account_name_type                  account;                     ///< Name of the account that created the node type.
   graph_node_name_type               node_type;                   ///< Name of the type of node being specified.
   string                             graph_privacy;               ///< Encryption level of the node attribute data. 
   string                             edge_permission;             ///< The Level of connection required to create an edge to or from this node type. 
   string                             details;                     ///< Describes the additional details of the node.
   string                             url;                         ///< Reference URL link for more details.
   string                             json;                        ///< Public plaintext JSON metadata information.
   vector< string >                   attributes;                  ///< List of attributes that each node is required to have.
   account_name_type                  interface;                   ///< Name of the application that facilitated the creation of the node type.
   time_point                         created;                     ///< Time the node type was created.
   time_point                         last_updated;                ///< Time that the node type was last updated by its creator.
};



struct graph_edge_property_api_obj
{
   graph_edge_property_api_obj( const chain::graph_edge_property_object& o ):
      id( o.id ),
      account( o.account ),
      edge_type( o.edge_type ),
      graph_privacy( connection_tier_values[ int( o.graph_privacy ) ] ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      interface( o.interface ),
      created( o.created ),
      last_updated( o.last_updated )
      {
         for( auto t : o.from_node_types )
         {
            from_node_types.push_back( t );
         }
         for( auto t : o.to_node_types )
         {
            to_node_types.push_back( t );
         }
         for( auto t : o.attributes )
         {
            attributes.push_back( to_string( t ) );
         }
      }

   graph_edge_property_api_obj(){}

   graph_edge_property_id_type               id;
   account_name_type                         account;                     ///< Name of the account that created the edge type.
   graph_edge_name_type                      edge_type;                   ///< Name of the type of edge being specified.
   string                                    graph_privacy;               ///< Encryption level of the edge attribute data.
   vector< graph_node_name_type >            from_node_types;             ///< Types of node that the edge can connect from. Empty for all types. 
   vector< graph_node_name_type >            to_node_types;               ///< Types of node that the edge can connect to. Empty for all types.
   string                                    details;                     ///< Describes the additional details of the node.
   string                                    url;                         ///< Reference URL link for more details.
   string                                    json;                        ///< Public plaintext JSON metadata information.
   vector< string >                          attributes;                  ///< List of attributes that each edge is required to have.
   account_name_type                         interface;                   ///< Name of the application that facilitated the creation of the edge type.
   time_point                                created;                     ///< Time the edge type was created.
   time_point                                last_updated;                ///< Time that the edge type was last updated by its creator.
};



//==================================//
// ===== Transfer API Objects ===== //
//==================================//



struct transfer_request_api_obj
{
   transfer_request_api_obj( const chain::transfer_request_object& o ):
      id( o.id ),
      to( o.to ),
      from( o.from ),
      amount( o.amount ),
      request_id( to_string( o.request_id ) ),
      memo( to_string( o.memo ) ),
      expiration( o.expiration ){}

   transfer_request_api_obj(){}

   transfer_request_id_type               id;
   account_name_type                      to;             ///< Account requesting the transfer.
   account_name_type                      from;           ///< Account that is being requested to accept the transfer.
   asset                                  amount;         ///< The amount of asset to transfer.
   string                                 request_id;     ///< uuidv4 of the request transaction.
   string                                 memo;           ///< The memo is plain-text, encryption on the memo is advised. 
   time_point                             expiration;     ///< time that the request expires. 
};



struct transfer_recurring_api_obj
{
   transfer_recurring_api_obj( const chain::transfer_recurring_object& o ):
      id( o.id ),
      from( o.from ),
      to( o.to ),
      amount( o.amount ),
      transfer_id( to_string( o.transfer_id ) ),
      memo( to_string( o.memo ) ),
      begin( o.begin ),
      end( o.end ),
      interval( o.interval ),
      next_transfer( o.next_transfer ){}

   transfer_recurring_api_obj(){}

   transfer_recurring_id_type        id;
   account_name_type                 from;              ///< Sending account to transfer asset from.
   account_name_type                 to;                ///< Recieving account to transfer asset to.
   asset                             amount;            ///< The amount of asset to transfer for each payment interval.
   string                            transfer_id;       ///< uuidv4 of the request transaction.
   string                            memo;              ///< The memo is plain-text, encryption on the memo is advised.
   time_point                        begin;             ///< Starting time of the first payment.
   time_point                        end;               ///< Ending time of the recurring payment. 
   fc::microseconds                  interval;          ///< Microseconds between each transfer event.
   time_point                        next_transfer;     ///< Time of the next transfer.   
};



struct transfer_recurring_request_api_obj
{
   transfer_recurring_request_api_obj( const chain::transfer_recurring_request_object& o ):
      id( o.id ),
      from( o.from ),
      to( o.to ),
      amount( o.amount ),
      request_id( to_string( o.request_id ) ),
      memo( to_string( o.memo ) ),
      begin( o.begin ),
      end( o.end ),
      interval( o.interval ),
      expiration( o.expiration ){}

   transfer_recurring_request_api_obj(){}

   transfer_recurring_request_id_type     id;
   account_name_type                      from;              ///< Sending account to transfer asset from.
   account_name_type                      to;                ///< Recieving account to transfer asset to.
   asset                                  amount;            ///< The amount of asset to transfer for each payment interval.
   string                                 request_id;        ///< uuidv4 of the request transaction.
   string                                 memo;              ///< The memo is plain-text, encryption on the memo is advised.
   time_point                             begin;             ///< Starting time of the first payment.
   time_point                             end;               ///< Ending time of the recurring payment. 
   fc::microseconds                       interval;          ///< Microseconds between each transfer event.
   time_point                             expiration;        ///< time that the request expires.
};


//=================================//
// ===== Balance API Objects ===== //
//=================================//



struct savings_withdraw_api_obj
{
   savings_withdraw_api_obj( const chain::savings_withdraw_object& o ) :
      id( o.id ),
      from( o.from ),
      to( o.to ),
      memo( to_string( o.memo ) ),
      request_id( to_string( o.request_id ) ),
      amount( o.amount ),
      complete( o.complete ){}

   savings_withdraw_api_obj() {}

   savings_withdraw_id_type   id;
   account_name_type          from;
   account_name_type          to;
   string                     memo;
   string                     request_id;
   asset                      amount;
   time_point                 complete;
};



struct confidential_balance_api_obj
{
   confidential_balance_api_obj( const chain::confidential_balance_object& o ):
      id( o.id ),
      owner( o.owner ),
      prev( o.prev ),
      op_in_trx( o.op_in_trx ),
      index( o.index ),
      commitment( o.commitment ),
      symbol( o.symbol ),
      created( o.created ){}

   confidential_balance_api_obj(){}

   confidential_balance_id_type           id;
   authority                              owner;               ///< Owner Authoirty of the confidential balance.
   transaction_id_type                    prev;                ///< Transaction ID of the transaction that created the balance.
   uint16_t                               op_in_trx;           ///< Number of the operation in the creating transaction.
   uint16_t                               index;               ///< Number of the balance created from the origin transaction.
   commitment_type                        commitment;          ///< Commitment of the Balance.
   asset_symbol_type                      symbol;              ///< Asset symbol of the balance.
   time_point                             created;             ///< Time that the balance was created.
};



//=====================================//
// ===== Marketplace API Objects ===== //
//=====================================//


struct product_sale_api_obj
{
   product_sale_api_obj( const chain::product_sale_object& o ) :
      id( o.id ),
      account( o.account ),
      product_id( to_string( o.product_id ) ),
      name( to_string( o.name ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      created( o.created ),
      last_updated( o.last_updated ),
      active( o.active )
      {
         for( auto s : o.product_variants )
         {
            product_variants.push_back( to_string( s ) );
         }
         for( auto s : o.product_details )
         {
            product_details.push_back( to_string( s ) );
         }
         for( auto s : o.product_images )
         {
            product_images.push_back( to_string( s ) );
         }
         for( auto s : o.product_prices )
         {
            product_prices.push_back( s );
         }
         for( auto s : o.wholesale_discount )
         {
            wholesale_discount[ s.first ] = s.second;
         }
         for( auto s : o.stock_available )
         {
            stock_available.push_back( s );
         }
         for( auto s : o.delivery_variants )
         {
            delivery_variants.push_back( to_string( s ) );
         }
         for( auto s : o.delivery_details )
         {
            delivery_details.push_back( to_string( s ) );
         }
         for( auto s : o.delivery_prices )
         {
            delivery_prices.push_back( s );
         }
      }

   product_sale_api_obj(){}

   product_sale_id_type                  id;
   account_name_type                     account;                ///< The Seller of the product.
   string                                product_id;             ///< The name of the product. Unique for each account.
   string                                name;                   ///< The name of the product.
   string                                url;                    ///< Reference URL of the product or seller.
   string                                json;                   ///< JSON metadata attributes of the product.
   vector< string >                      product_variants;       ///< The collection of product variants. Each map must have a key for each variant.
   vector< string >                      product_details;        ///< The Description details of each variant of the product.
   vector< string >                      product_images;         ///< IPFS references to images of each product variant.
   vector< asset >                       product_prices;         ///< The price (or min auction price) for each variant of the product.
   map< uint32_t, uint16_t >             wholesale_discount;     ///< Discount percentages that are applied when quantity is above a given size.
   vector< uint32_t >                    stock_available;        ///< The available stock of each variant of the product.
   vector< string >                      delivery_variants;      ///< The types of product delivery available to purchasers.
   vector< string >                      delivery_details;       ///< The Description details of each variant of the delivery.
   vector< asset >                       delivery_prices;        ///< The price for each variant of delivery.
   time_point                            created;                ///< Time that the order was created.
   time_point                            last_updated;           ///< Time that the order was last updated, approved, or disputed.
   bool                                  active;                 ///< True when the product is active and able to be sold, false when discontinued.
};


struct product_purchase_api_obj
{
   product_purchase_api_obj( const chain::product_purchase_object& o ) :
      id( o.id ),
      buyer( o.buyer ),
      order_id( to_string( o.order_id ) ),
      seller( o.seller ),
      product_id( to_string( o.product_id ) ),
      memo( to_string( o.memo ) ),
      json( to_string( o.json ) ),
      purchase_public_key( o.purchase_public_key ),
      shipping_address( to_string( o.shipping_address ) ),
      delivery_variant( to_string( o.delivery_variant ) ),
      delivery_details( to_string( o.delivery_details ) ),
      order_value( o.order_value ),
      created( o.created ),
      last_updated( o.last_updated )
      {
         for( auto v : o.order_variants )
         {
            order_variants.push_back( to_string( v ) );
         }
         for( auto v : o.order_size )
         {
            order_size.push_back( v );
         }
      }

   product_purchase_api_obj(){}

   product_purchase_id_type          id;
   account_name_type                 buyer;                  ///< The Buyer of the product.
   string                            order_id;               ///< uuidv4 referring to the purchase order.
   account_name_type                 seller;                 ///< The Seller of the product.
   string                            product_id;             ///< uuidv4 refrring to the product.
   vector< string >                  order_variants;         ///< Variants of product ordered in the purchase.
   vector< uint32_t >                order_size;             ///< The number of each product variant ordered. 
   string                            memo;                   ///< The memo for the transaction, encryption on the memo is advised.
   string                            json;                   ///< Additional JSON object attribute details.
   public_key_type                   purchase_public_key;    ///< the Public key used to encrypt the memo and shipping address. 
   string                            shipping_address;       ///< The shipping address requested, encrypted with the secure key of the seller.
   string                            delivery_variant;       ///< The type of product delivery selected.
   string                            delivery_details;       ///< The Description details of the delivery.
   asset                             order_value;            ///< The total value of the order.
   time_point                        created;                ///< Time that the order was created.
   time_point                        last_updated;           ///< Time that the order was last updated, approved, or disputed.
};


struct product_auction_sale_api_obj
{
   product_auction_sale_api_obj( const chain::product_auction_sale_object& o ) :
      id( o.id ),
      account( o.account ),
      auction_id( to_string( o.auction_id ) ),
      auction_type( product_auction_values[ int( o.auction_type ) ] ),
      name( to_string( o.name ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      product_details( to_string( o.product_details ) ),
      reserve_bid( o.reserve_bid ),
      maximum_bid( o.maximum_bid ),
      final_bid_time( o.final_bid_time ),
      completion_time( o.completion_time ),
      winning_bid( o.winning_bid ),
      created( o.created ),
      last_updated( o.last_updated )
      {
         for( auto s : o.product_images )
         {
            product_images.push_back( to_string( s ) );
         }
         for( auto s : o.delivery_variants )
         {
            delivery_variants.push_back( to_string( s ) );
         }
         for( auto s : o.delivery_details )
         {
            delivery_details.push_back( to_string( s ) );
         }
         for( auto s : o.delivery_prices )
         {
            delivery_prices.push_back( s );
         }
      }

   product_auction_sale_api_obj(){}

   product_auction_sale_id_type          id;
   account_name_type                     account;                ///< The Seller of the product.
   string                                auction_id;             ///< The uuidv4 identifying the auction.
   string                                auction_type;           ///< The type of sale to be used for the product.
   string                                name;                   ///< The name of the product. Unique for each account.
   string                                url;                    ///< Reference URL of the product or seller.
   string                                json;                   ///< JSON metadata attributes of the product.
   string                                product_details;        ///< The Description details of each variant of the product.
   vector< string >                      product_images;         ///< IPFS references to images of each product variant.
   asset                                 reserve_bid;            ///< The min auction bid, or minimum price of a reverse auction at final bid time.
   asset                                 maximum_bid;            ///< The max auction bid. Auction will immediately conclude if this price is bidded. Starting price of reverse auction.
   vector< string >                      delivery_variants;      ///< The types of product delivery available to purchasers.
   vector< string >                      delivery_details;       ///< The Description details of each variant of the delivery.
   vector< asset >                       delivery_prices;        ///< The price for each variant of delivery.
   time_point                            final_bid_time;         ///< No more bids will be accepted after this time. Concealed bids must be revealed before completion time.
   time_point                            completion_time;        ///< Time that the auction will select the winning bidder, or end if no bids.
   product_auction_bid_id_type           winning_bid;            ///< ID of the winning bid after completion time.
   time_point                            created;                ///< Time that the order was created.
   time_point                            last_updated;           ///< Time that the order was last updated.
};


struct product_auction_bid_api_obj
{
   product_auction_bid_api_obj( const chain::product_auction_bid_object& o ) :
      id( o.id ),
      buyer( o.buyer ),
      bid_id( to_string( o.bid_id ) ),
      seller( o.seller ),
      auction_id( to_string( o.auction_id ) ),
      bid_asset( o.bid_asset ),
      bid_price_commitment( o.bid_price_commitment ),
      blinding_factor( o.blinding_factor ),
      public_bid_amount( o.public_bid_amount ),
      memo( to_string( o.memo ) ),
      json( to_string( o.json ) ),
      bid_public_key( o.bid_public_key ),
      shipping_address( to_string( o.shipping_address ) ),
      delivery_variant( to_string( o.delivery_variant ) ),
      delivery_details( to_string( o.delivery_details ) ),
      delivery_value( o.delivery_value ),
      created( o.created ),
      last_updated( o.last_updated ),
      completion_time( o.completion_time ),
      winning_bid( o.winning_bid ){}

   product_auction_bid_api_obj(){}

   product_auction_bid_id_type       id;
   account_name_type                 buyer;                  ///< The Buyer of the product.
   string                            bid_id;                 ///< uuidv4 referring to the auction bid.
   account_name_type                 seller;                 ///< The Seller of the product.
   string                            auction_id;             ///< The uuidv4 identifying the auction.
   asset_symbol_type                 bid_asset;              ///< The Symbol of the asset being bidded.
   commitment_type                   bid_price_commitment;   ///< Concealed value of the bid price amount.
   blind_factor_type                 blinding_factor;        ///< Factor to blind the bid price.
   share_type                        public_bid_amount;      ///< Set to 0 initially for concealed bid, revealed to match commitment. Revealed in initial bid if open.
   string                            memo;                   ///< The memo for the transaction, encryption on the memo is advised.
   string                            json;                   ///< Additional JSON object attribute details.
   public_key_type                   bid_public_key;         ///< the Public key used to encrypt the memo and shipping address. 
   string                            shipping_address;       ///< The shipping address requested, encrypted with the secure key of the seller.
   string                            delivery_variant;       ///< The type of product delivery selected.
   string                            delivery_details;       ///< The Description details of the delivery.
   asset                             delivery_value;         ///< The cost of the delivery if the bid is successful.
   time_point                        created;                ///< Time that the order was created.
   time_point                        last_updated;           ///< Time that the order was last updated, approved, or disputed.
   time_point                        completion_time;        ///< Time that the auction will select the winning bidder, or end if no bids.
   bool                              winning_bid;            ///< True when the bid wins its auction, false otherwise.
};


struct escrow_api_obj
{
   escrow_api_obj( const chain::escrow_object& o ) :
      id( o.id ),
      from( o.from ),
      to( o.to ),
      from_mediator( o.from_mediator ),
      to_mediator( o.to_mediator ),
      payment( o.payment ),
      balance( o.balance ),
      escrow_id( to_string( o.escrow_id ) ),
      memo( to_string( o.memo ) ),
      json( to_string( o.json ) ),
      acceptance_time( o.acceptance_time ),
      escrow_expiration( o.escrow_expiration ),
      dispute_release_time( o.dispute_release_time ),
      created( o.created ),
      last_updated( o.last_updated ),
      disputed( o.disputed )
      {
         for( auto m : o.mediators )
         {
            mediators.push_back( m );
         }
         for( auto r : o.release_percentages )
         {
            release_percentages[ r.first ] = r.second;
         }
         for( auto a : o.approvals )
         {
            approvals[ a.first ] = a.second;
         }
      }

   escrow_api_obj(){}

   escrow_id_type                            id;
   account_name_type                         from;                   ///< Account sending funds.
   account_name_type                         to;                     ///< Account receiving funds.
   account_name_type                         from_mediator;          ///< Representative of the sending account.
   account_name_type                         to_mediator;            ///< Representative of the receiving account.
   asset                                     payment;                ///< Total payment to be transferred.
   asset                                     balance;                ///< Current funds deposited in the escrow.
   string                                    escrow_id;              ///< uuidv4 referring to the escrow payment.
   string                                    memo;                   ///< Details of the transaction for reference. 
   string                                    json;                   ///< Additional JSON object attribute details.
   time_point                                acceptance_time;        ///< time that the transfer must be approved by.
   time_point                                escrow_expiration;      ///< Time that the escrow is able to be claimed by either TO or FROM.
   time_point                                dispute_release_time;   ///< Time that the balance is distributed to median release percentage.
   vector< account_name_type >               mediators;              ///< Set of accounts able to mediate the dispute.
   map< account_name_type, uint16_t >        release_percentages;    ///< Declared release percentages of all accounts.
   map< account_name_type, bool >            approvals;              ///< Map of account approvals, paid into balance.
   time_point                                created;                ///< Time that the order was created.
   time_point                                last_updated;           ///< Time that the order was last updated, approved, or disputed.
   bool                                      disputed;               ///< True when escrow is in dispute.
};



//=================================//
// ===== Trading API Objects ===== //
//=================================//



struct limit_order_api_obj
{
   limit_order_api_obj( const chain::limit_order_object& o ):
      id( o.id ),
      seller( o.seller ),
      order_id( to_string( o.order_id ) ),
      for_sale( o.for_sale.value ),
      sell_price( o.sell_price ),
      interface( o.interface ),
      created( o.created ),
      last_updated( o.last_updated ),
      expiration( o.expiration ),
      real_price( o.real_price() ){}

   limit_order_api_obj(){}

   limit_order_id_type    id;
   account_name_type      seller;            ///< Selling account name of the trading order.
   string                 order_id;          ///< UUIDv4 of the order for each account.
   int64_t                for_sale;          ///< asset symbol is sell_price.base.symbol
   price                  sell_price;        ///< Base price is the asset being sold.
   account_name_type      interface;         ///< The interface account that created the order
   time_point             created;           ///< Time that the order was created.
   time_point             last_updated;      ///< Time that the order was last modified.
   time_point             expiration;        ///< Expiration time of the order.
   double                 real_price;        ///< Real decimal price of the limit.
};



struct margin_order_api_obj
{
   margin_order_api_obj( const chain::margin_order_object& o ):
      id( o.id ),
      owner( o.owner ),
      order_id( to_string( o.order_id ) ),
      sell_price( o.sell_price ),
      collateral( o.collateral ),
      debt( o.debt ),
      debt_balance( o.debt_balance ),
      interest( o.interest ),
      position( o.position ),
      position_balance( o.position_balance ),
      collateralization( o.collateralization.value ),
      interface( o.interface ),
      created( o.created ),
      expiration( o.expiration ),
      unrealized_value( o.unrealized_value ),
      last_interest_rate( o.last_interest_rate.value ),
      liquidating( o.liquidating ),
      stop_loss_price( o.stop_loss_price ),
      take_profit_price( o.take_profit_price ),
      limit_stop_loss_price( o.limit_stop_loss_price ),
      limit_take_profit_price( o.limit_take_profit_price ),
      real_price( o.real_price() ){}
      
   margin_order_api_obj(){}

   margin_order_id_type       id;
   account_name_type          owner;                       ///< Margin order owners account name
   string                     order_id;                    ///< UUIDv4 Unique Identifier of the order for each account.
   price                      sell_price;                  ///< limit exchange price of the borrowed asset being sold for the position asset.
   asset                      collateral;                  ///< Collateral asset used to back the loan value; Returned to credit collateral object when position is closed. 
   asset                      debt;                        ///< Amount of asset borrowed to purchase the position asset. Repaid when the margin order is closed. 
   asset                      debt_balance;                ///< Debt asset that is held by the order when selling debt, or liquidating position.
   asset                      interest;                    ///< Amount of interest accrued on the borrowed asset into the debt value.
   asset                      position;                    ///< Minimum amount of asset to receive as margin position.
   asset                      position_balance;            ///< Amount of asset currently held within the order that has filled.                     
   int64_t                    collateralization;           ///< Percentage ratio of ( Collateral + position_balance + debt_balance - debt ) / debt. Position is liquidated when ratio falls below liquidation requirement 
   account_name_type          interface;                   ///< The interface account that created the order.
   time_point                 created;                     ///< Time that the order was created.
   time_point                 last_updated;                ///< Time that interest was last compounded on the margin order, and collateralization was last updated. 
   time_point                 last_interest_time;          ///< Time that interest was last compounded on the margin order. 
   time_point                 expiration;                  ///< Expiration time of the order.
   asset                      unrealized_value;            ///< Current profit or loss that the position is holding.
   int64_t                    last_interest_rate;          ///< The interest rate that was last applied to the order.
   bool                       liquidating;                 ///< Set to true to place the margin order back into the orderbook and liquidate the position at sell price.
   price                      stop_loss_price;             ///< Price at which the position will be force liquidated if it falls into a net loss.
   price                      take_profit_price;           ///< Price at which the position will be force liquidated if it rises into a net profit.
   price                      limit_stop_loss_price;       ///< Price at which the position will be limit liquidated if it falls into a net loss.
   price                      limit_take_profit_price;     ///< Price at which the position will be limit liquidated if it rises into a net profit.
   double                     real_price;                  ///< Real decimal price of the order.
};



struct auction_order_api_obj
{
   auction_order_api_obj( const chain::auction_order_object& o ):
      id( o.id ),
      owner( o.owner ),
      order_id( to_string( o.order_id ) ),
      amount_to_sell( o.amount_to_sell ),
      limit_close_price( o.limit_close_price ),
      interface( o.interface ),
      expiration( o.expiration ),
      created( o.created ),
      last_updated( o.last_updated ),
      real_price( o.real_price() ){}

   auction_order_api_obj(){}

   auction_order_id_type     id;
   account_name_type         owner;                    ///< Owner of the Auction order.
   string                    order_id;                 ///< uuidv4 of the order for reference.
   asset                     amount_to_sell;           ///< Amount of asset to sell at auction clearing price. Asset is base of price.
   price                     limit_close_price;        ///< The asset pair price to sell the amount at the auction clearing price.
   account_name_type         interface;                ///< Name of the interface that created the transaction.
   time_point                expiration;               ///< Time that the order expires.
   time_point                created;                  ///< Time that the order was created.
   time_point                last_updated;             ///< Time that the order was last modified.
   double                    real_price;               ///< Real decimal price of the order.
};



struct call_order_api_obj
{
   call_order_api_obj( const chain::call_order_object& o ):
      id( o.id ),
      borrower( o.borrower ),
      collateral( o.collateral ),
      debt( o.debt ),
      call_price( o.call_price ),
      target_collateral_ratio( *o.target_collateral_ratio ),
      interface( o.interface ),
      real_price( o.real_price() ){}

   call_order_api_obj(){}

   call_order_id_type      id;
   account_name_type       borrower;
   asset                   collateral;                  ///< call_price.base.symbol, access via get_collateral.
   asset                   debt;                        ///< call_price.quote.symbol, access via get_debt
   price                   call_price;                  ///< Collateral / Debt
   uint16_t                target_collateral_ratio;     ///< maximum CR to maintain when selling collateral on margin call
   account_name_type       interface;                   ///< The interface account that created the order
   double                  real_price;                  ///< Real decimal price of the order.
};



struct option_order_api_obj
{
   option_order_api_obj( const chain::option_order_object& o ):
      id( o.id ),
      owner( o.owner ),
      order_id( to_string( o.order_id ) ),
      option_position( o.option_position ),
      underlying_amount( o.underlying_amount ),
      exercise_amount( o.exercise_amount ),
      strike_price( o.strike_price ),
      interface( o.interface ),
      created( o.created ),
      last_updated( o.last_updated ),
      real_price( o.real_price() ){}

   option_order_api_obj(){}

   option_order_id_type      id;
   account_name_type         owner;                    ///< Owner of the Option order.
   string                    order_id;                 ///< uuidv4 of the order for reference.
   asset                     option_position;          ///< Amount of option assets generated by the position. Debt owed by the order. 
   asset                     underlying_amount;        ///< Assets to issue options contract assets against.
   asset                     exercise_amount;          ///< Assets that will be received if the options are all excercised.
   option_strike             strike_price;             ///< The asset pair strike price at which the options can be exercised at any time before expiration.
   account_name_type         interface;                ///< Name of the interface that created the transaction.
   time_point                created;                  ///< Time that the order was created.
   time_point                last_updated;             ///< Time that the order was last modified.
   double                    real_price;               ///< Real decimal price of the order.
};



//===============================//
// ===== Asset API Objects ===== //
//===============================//



struct asset_api_obj
{
   asset_api_obj( const chain::asset_object& a ) :
      id( a.id ),
      symbol( a.symbol ),
      asset_type( asset_property_values[ int( a.asset_type ) ] ),
      issuer( a.issuer ),
      display_symbol( to_string( a.display_symbol ) ),
      details( to_string( a.details ) ),
      json( to_string( a.json ) ),
      url( to_string( a.url ) ),
      max_supply( a.max_supply.value ),
      stake_intervals( a.stake_intervals ),
      unstake_intervals( a.unstake_intervals ),
      market_fee_percent( a.market_fee_percent ),
      market_fee_share_percent( a.market_fee_share_percent ),
      issuer_permissions( a.issuer_permissions ),
      flags( a.flags ),
      created( a.created ),
      last_updated( a.last_updated )
      {
         for( auto auth : a.whitelist_authorities )
         {
            whitelist_authorities.push_back( auth );
         }
         for( auto auth : a.blacklist_authorities )
         {
            blacklist_authorities.push_back( auth );
         }
         for( auto market : a.whitelist_markets )
         {
            whitelist_markets.push_back( market );
         }
         for( auto market : a.blacklist_markets )
         {
            blacklist_markets.push_back( market );
         }
      }

   asset_api_obj(){}

   asset_id_type                   id; 
   asset_symbol_type               symbol;                                ///< Consensus enforced unique Ticker symbol string for this asset. 
   string                          asset_type;                            ///< The type of the asset.
   account_name_type               issuer;                                ///< name of the account which issued this asset.
   asset_symbol_type               display_symbol;                        ///< Non-consensus display name for interface reference.
   string                          details;                               ///< Data that describes the purpose of this asset.
   string                          json;                                  ///< Additional JSON metadata of this asset.
   string                          url;                                   ///< Reference URL for the asset. 
   int64_t                         max_supply;                            ///< The maximum supply of this asset which may exist at any given time. 
   uint8_t                         stake_intervals;                       ///< Weeks required to stake the asset.
   uint8_t                         unstake_intervals;                     ///< Weeks require to unstake the asset.
   uint16_t                        market_fee_percent;                    ///< Percentage of the total traded will be paid to the issuer of the asset.
   uint16_t                        market_fee_share_percent;              ///< Percentage of the market fee that will be shared with the account's referrers.
   int64_t                         max_market_fee;                        ///< Market fee charged on a trade is capped to this value.
   uint32_t                        issuer_permissions;                    ///< The flags which the issuer has permission to update.
   uint32_t                        flags;                                 ///< The currently active flags on this permission.
   vector<account_name_type>       whitelist_authorities;                 ///< Accounts able to transfer this asset if the flag is set and whitelist is non-empty.
   vector<account_name_type>       blacklist_authorities;                 ///< Accounts which cannot transfer or receive this asset.
   vector<asset_symbol_type>       whitelist_markets;                     ///< The assets that this asset may be traded against in the market
   vector<asset_symbol_type>       blacklist_markets;                     ///< The assets that this asset may not be traded against in the market, must not overlap whitelist.
   time_point                      created;                               ///< Time that the asset was created. 
   time_point                      last_updated;                          ///< Time that the asset details were last updated.
};



struct asset_dynamic_data_api_obj
{
   asset_dynamic_data_api_obj( const chain::asset_dynamic_data_object& a ) :
      id( a.id ),
      symbol( a.symbol ),
      issuer( a.issuer ),
      total_supply( a.total_supply.value ),
      liquid_supply( a.liquid_supply.value ),
      staked_supply( a.staked_supply.value ),
      reward_supply( a.reward_supply.value ),
      savings_supply( a.savings_supply.value ),
      vesting_supply( a.vesting_supply.value ),
      delegated_supply( a.delegated_supply.value ),
      receiving_supply( a.receiving_supply.value ),
      pending_supply( a.pending_supply.value ),
      confidential_supply( a.confidential_supply.value ){}

   asset_dynamic_data_api_obj(){}

   asset_dynamic_data_id_type      id;
   asset_symbol_type               symbol;                    ///< Consensus enforced unique Ticker symbol string for this asset.
   account_name_type               issuer;                    ///< Name of the account which issued this asset.
   int64_t                         total_supply;              ///< The total outstanding supply of the asset.
   int64_t                         liquid_supply;             ///< The current liquid supply of the asset.
   int64_t                         staked_supply;             ///< The current staked supply of the asset.
   int64_t                         reward_supply;             ///< The current reward supply of the asset.
   int64_t                         savings_supply;            ///< The current savings supply of the asset.
   int64_t                         vesting_supply;            ///< The current vesting supply of the asset.
   int64_t                         delegated_supply;          ///< The current delegated supply of the asset.
   int64_t                         receiving_supply;          ///< The current receiving supply supply of the asset, should equal delegated.
   int64_t                         pending_supply;            ///< The current supply contained in pending network objects and funds.
   int64_t                         confidential_supply;       ///< The current confidential asset supply.
};



struct currency_data_api_obj
{
   currency_data_api_obj( const chain::asset_currency_data_object& a ) :
      id( a.id ),
      symbol( a.symbol ),
      block_reward( a.block_reward ),
      block_reward_reduction_percent( a.block_reward_reduction_percent ),
      block_reward_reduction_days( a.block_reward_reduction_days ),
      content_reward_percent( a.content_reward_percent ),
      equity_asset( a.equity_asset ),
      equity_reward_percent( a.equity_reward_percent ),
      producer_reward_percent( a.producer_reward_percent ),
      supernode_reward_percent( a.supernode_reward_percent ),
      power_reward_percent( a.power_reward_percent ),
      community_fund_reward_percent( a.community_fund_reward_percent ),
      development_reward_percent( a.development_reward_percent ),
      marketing_reward_percent( a.marketing_reward_percent ),
      advocacy_reward_percent( a.advocacy_reward_percent ),
      activity_reward_percent( a.activity_reward_percent ),
      producer_block_reward_percent( a.producer_block_reward_percent ),
      validation_reward_percent( a.validation_reward_percent ),
      txn_stake_reward_percent( a.txn_stake_reward_percent ),
      work_reward_percent( a.work_reward_percent ),
      producer_activity_reward_percent( a.producer_activity_reward_percent ){}

   currency_data_api_obj(){}

   asset_currency_data_id_type     id;
   asset_symbol_type               symbol;                             ///< The symbol of the currency asset.
   asset                           block_reward;                       ///< The value of the initial reward paid into the reward fund every block.
   uint16_t                        block_reward_reduction_percent;     ///< The percentage by which the block reward is reduced each period. 0 for no reduction.
   uint16_t                        block_reward_reduction_days;        ///< Number of days between reduction events. 0 for no reduction.
   uint16_t                        content_reward_percent;             ///< Percentage of reward paid to content creators.
   asset_symbol_type               equity_asset;                       ///< Asset that will receive equity rewards.
   uint16_t                        equity_reward_percent;              ///< Percentage of reward paid to staked equity asset holders.
   uint16_t                        producer_reward_percent;            ///< Percentage of reward paid to block producers.
   uint16_t                        supernode_reward_percent;           ///< Percentage of reward paid to supernode operators.
   uint16_t                        power_reward_percent;               ///< Percentage of reward paid to staked currency asset holders.
   uint16_t                        community_fund_reward_percent;      ///< Percentage of reward paid to community fund proposals.
   uint16_t                        development_reward_percent;         ///< Percentage of reward paid to elected developers.
   uint16_t                        marketing_reward_percent;           ///< Percentage of reward paid to elected marketers.
   uint16_t                        advocacy_reward_percent;            ///< Percentage of reward paid to elected advocates.
   uint16_t                        activity_reward_percent;            ///< Percentage of reward paid to active accounts each day.
   uint16_t                        producer_block_reward_percent;      ///< Percentage of producer reward paid to the producer of each individual block.
   uint16_t                        validation_reward_percent;          ///< Percentage of producer reward paid to validators of blocks.
   uint16_t                        txn_stake_reward_percent;           ///< Percentage of producer reward paid to producers according to transaction stake weight of blocks.
   uint16_t                        work_reward_percent;                ///< Percentage of producer reward paid to proof of work mining producers for each proof.
   uint16_t                        producer_activity_reward_percent;   ///< Percentage of producer reward paid to the highest voted producer in activity rewards.
};


struct stablecoin_data_api_obj
{
   stablecoin_data_api_obj( const chain::asset_stablecoin_data_object& b ):
      id( b.id ),
      symbol( b.symbol ),
      issuer( b.issuer ),
      backing_asset( b.backing_asset ),
      current_feed( b.current_feed ),
      current_feed_publication_time( b.current_feed_publication_time ),
      force_settled_volume( b.force_settled_volume.value ),
      settlement_price( b.settlement_price ),
      settlement_fund( b.settlement_fund ),
      feed_lifetime( b.feed_lifetime ),
      minimum_feeds( b.minimum_feeds ),
      asset_settlement_delay( b.asset_settlement_delay ),
      asset_settlement_offset_percent( b.asset_settlement_offset_percent ),
      maximum_asset_settlement_volume( b.maximum_asset_settlement_volume )
      {
         for( auto feed: b.feeds )
         {
            feeds[ feed.first ] = feed.second;
         }
      }
   
   stablecoin_data_api_obj(){}

   asset_stablecoin_data_id_type                           id;
   asset_symbol_type                                       symbol;                                  ///< The symbol of the stablecoin that this object belongs to
   account_name_type                                       issuer;                                  ///< The account name of the issuer 
   asset_symbol_type                                       backing_asset;                           ///< The collateral backing asset of the stablecoin
   map<account_name_type, pair<time_point,price_feed>>     feeds;                                   ///< Feeds published for this asset. 
   price_feed                                              current_feed;                            ///< Currently active price feed, median of values from the currently active feeds.
   time_point                                              current_feed_publication_time;           ///< Publication time of the oldest feed which was factored into current_feed.
   price                                                   current_maintenance_collateralization;   ///< Call orders with collateralization (aka collateral/debt) not greater than this value are in margin call territory.
   int64_t                                                 force_settled_volume;                    ///< This is the volume of this asset which has been force-settled this 24h interval
   price                                                   settlement_price;                        ///< Price at which force settlements of a black swanned asset will occur
   asset                                                   settlement_fund;                         ///< Amount of collateral which is available for force settlement
   fc::microseconds                                        feed_lifetime;                           ///< Time before a price feed expires
   uint8_t                                                 minimum_feeds;                           ///< Minimum number of unexpired feeds required to extract a median feed from
   fc::microseconds                                        asset_settlement_delay;                  ///< This is the delay between the time a long requests settlement and the chain evaluates the settlement
   uint16_t                                                asset_settlement_offset_percent;         ///< The percentage to adjust the feed price in the short's favor in the event of a forced settlement
   uint16_t                                                maximum_asset_settlement_volume;         ///< the percentage of current supply which may be force settled within each 24h interval.
};


struct asset_settlement_api_obj
{
   asset_settlement_api_obj( const chain::asset_settlement_object& b ):
      id( b.id ),
      owner( b.owner ),
      balance( b.balance ),
      settlement_date( b.settlement_date ),
      interface( b.interface ),
      created( b.created ),
      last_updated( b.last_updated ){}
   
   asset_settlement_api_obj(){}

   asset_settlement_id_type          id;
   account_name_type                 owner;             ///< Name of the account that is force settling the asset
   asset                             balance;           ///< Amount of debt asset being settled
   time_point                        settlement_date;   ///< Date of asset settlement for collateral
   account_name_type                 interface;         ///< The interface account that created the order
   time_point                        created;           ///< Time that the settlement was created.
   time_point                        last_updated;      ///< Time that the settlement was last modified.
};


struct asset_collateral_bid_api_obj
{
   asset_collateral_bid_api_obj( const chain::asset_collateral_bid_object& b ):
      id( b.id ),
      bidder( b.bidder ),
      collateral( b.collateral ),
      debt( b.debt ),
      created( b.created ),
      last_updated( b.last_updated ){}
   
   asset_collateral_bid_api_obj(){}

   asset_collateral_bid_id_type          id;
   account_name_type                     bidder;           ///< Bidding Account name.
   asset                                 collateral;       ///< Collateral bidded to obtain debt from a global settlement.
   asset                                 debt;             ///< Debt requested for bid.
   time_point                            created;          ///< Time that the bid was created.
   time_point                            last_updated;     ///< Time that the bid was last adjusted.
};


struct equity_data_api_obj
{
   equity_data_api_obj( const chain::asset_equity_data_object& e ):
      id( e.id ),
      business_account( e.business_account ),
      symbol( e.symbol ),
      last_dividend( e.last_dividend ),
      dividend_share_percent( e.dividend_share_percent ),
      liquid_dividend_percent( e.liquid_dividend_percent ),
      staked_dividend_percent( e.staked_dividend_percent ),
      savings_dividend_percent( e.savings_dividend_percent ),
      liquid_voting_rights( e.liquid_voting_rights ),
      staked_voting_rights( e.staked_voting_rights ),
      savings_voting_rights( e.savings_voting_rights ),
      min_active_time( e.min_active_time ),
      min_balance( e.min_balance.value ),
      min_producers( e.min_producers ),
      boost_balance( e.boost_balance.value ),
      boost_activity( e.boost_activity.value ),
      boost_producers( e.boost_producers.value ),
      boost_top( e.boost_top )
      {
         for( auto a: e.dividend_pool )
         {
            dividend_pool[ a.first ] = a.second;
         }
      }

   equity_data_api_obj(){}

   asset_equity_data_id_type            id;
   account_name_type                    business_account;            ///< The business account name of the issuer.
   asset_symbol_type                    symbol;                      ///< The symbol of the equity asset of the business.
   map< asset_symbol_type, asset >      dividend_pool;               ///< Assets pooled for distribution at the next interval.
   time_point                           last_dividend;               ///< Time that the asset last distributed a dividend.
   uint16_t                             dividend_share_percent;      ///< Percentage of incoming assets added to the dividends pool.
   uint16_t                             liquid_dividend_percent;     ///< Percentage of equity dividends distributed to liquid balances.
   uint16_t                             staked_dividend_percent;     ///< Percentage of equity dividends distributed to staked balances.
   uint16_t                             savings_dividend_percent;    ///< Percentage of equity dividends distributed to savings balances.
   uint16_t                             liquid_voting_rights;        ///< Amount of votes per asset conveyed to liquid holders of the asset.
   uint16_t                             staked_voting_rights;        ///< Amount of votes per asset conveyed to staked holders of the asset.
   uint16_t                             savings_voting_rights;       ///< Amount of votes per asset conveyed to savings holders of the asset
   fc::microseconds                     min_active_time;             ///< Time that account must have a recent activity reward within to earn dividend.
   int64_t                              min_balance;                 ///< Minimum amount of equity required to earn dividends.
   uint16_t                             min_producers;               ///< Minimum amount of producer votes required to earn dividends.
   int64_t                              boost_balance;               ///< Amount of equity balance to earn double dividends.
   int64_t                              boost_activity;              ///< Amount of recent activity rewards required to earn double dividends.
   int64_t                              boost_producers;             ///< Amount of producer votes required to earn double dividends.
   uint16_t                             boost_top;                   ///< Percent bonus earned by Top membership accounts.
};


struct bond_data_api_obj
{
   bond_data_api_obj( const chain::asset_bond_data_object& c ):
      id( c.id ),
      business_account( c.business_account ),
      symbol( c.symbol ),
      value( c.value ),
      collateralization( c.collateralization ),
      coupon_rate_percent( c.coupon_rate_percent ),
      maturity_date( c.maturity_date ),
      collateral_pool( c.collateral_pool ){}

   bond_data_api_obj(){}

   asset_bond_data_id_type          id;
   account_name_type                business_account;          ///< The account name of the issuer. Locks collateral to issue new bond units.
   asset_symbol_type                symbol;                    ///< The symbol of the bond asset.
   asset                            value;                     ///< Face value amount of each unit of the bond. Interest is paid as a percentage of value.
   uint16_t                         collateralization;         ///< Percentage of value that is locked in collateral to back the bonds. Should be at least 10%.
   uint16_t                         coupon_rate_percent;       ///< Percentage rate of the value that is paid each month in interest to the holders.
   date_type                        maturity_date;             ///< Date at which the bond will mature. Principle value will be automatically paid from business_account.
   asset                            collateral_pool;           ///< Amount of collateral backing the bond assets. Distributed in case of default. 
};


struct credit_data_api_obj
{
   credit_data_api_obj( const chain::asset_credit_data_object& c ):
      id( c.id ),
      buyback_asset( c.buyback_asset ),
      buyback_pool( c.buyback_pool ),
      buyback_price( c.buyback_price ),
      last_buyback( c.last_buyback ),
      buyback_share_percent( c.buyback_share_percent ),
      liquid_fixed_interest_rate( c.liquid_fixed_interest_rate ),
      liquid_variable_interest_rate( c.liquid_variable_interest_rate ),
      staked_fixed_interest_rate( c.staked_fixed_interest_rate ),
      staked_variable_interest_rate( c.staked_variable_interest_rate ),
      savings_fixed_interest_rate( c.savings_fixed_interest_rate ),
      savings_variable_interest_rate( c.savings_variable_interest_rate ),
      var_interest_range( c.var_interest_range ){}

   credit_data_api_obj(){}

   asset_credit_data_id_type  id;
   account_name_type          business_account;                          ///< The business account name of the issuer.
   asset_symbol_type          symbol;                                    ///< The symbol of the credit asset of the business.
   asset_symbol_type          buyback_asset;                             ///< Symbol used to buyback credit assets
   asset                      buyback_pool;                              ///< Amount of assets pooled to buyback the asset at next interval
   price                      buyback_price;                             ///< Price at which the credit asset is bought back
   time_point                 last_buyback;                              ///< Time that the asset was last updated
   uint16_t                   buyback_share_percent;                     ///< Percentage of incoming assets added to the buyback pool
   uint16_t                   liquid_fixed_interest_rate;                ///< Fixed component of Interest rate of the asset for liquid balances.
   uint16_t                   liquid_variable_interest_rate;             ///< Variable component of Interest rate of the asset for liquid balances.
   uint16_t                   staked_fixed_interest_rate;                ///< Fixed component of Interest rate of the asset for staked balances.
   uint16_t                   staked_variable_interest_rate;             ///< Variable component of Interest rate of the asset for staked balances.
   uint16_t                   savings_fixed_interest_rate;               ///< Fixed component of Interest rate of the asset for savings balances.
   uint16_t                   savings_variable_interest_rate;            ///< Variable component of Interest rate of the asset for savings balances.
   uint16_t                   var_interest_range;                        ///< The percentage range from the buyback price over which to apply the variable interest rate.
};


struct stimulus_data_api_obj
{
   stimulus_data_api_obj( const chain::asset_stimulus_data_object& o ):
      id( o.id ),
      business_account( o.business_account ),
      symbol( o.symbol ),
      redemption_asset( o.redemption_asset ),
      redemption_pool( o.redemption_pool ),
      redemption_price( o.redemption_price ),
      distribution_amount( o.distribution_amount ),
      next_distribution_date( o.next_distribution_date )
      {
         for( auto acc : o.distribution_list )
         {
            distribution_list.push_back( acc );
         }
         for( auto acc : o.redemption_list )
         {
            redemption_list.push_back( acc );
         }
      }

   stimulus_data_api_obj(){}

   asset_stimulus_data_id_type          id;
   account_name_type                    business_account;                   ///< The business account name of the issuer.
   asset_symbol_type                    symbol;                             ///< The symbol of the stimulus asset.
   asset_symbol_type                    redemption_asset;                   ///< Symbol of the asset that can be redeemed in exchange the stimulus asset.
   asset                                redemption_pool;                    ///< Amount of assets pooled to redeem in exchange for the stimulus asset.
   price                                redemption_price;                   ///< Price at which the stimulus asset is redeemed. Redemption asset is base.
   vector< account_name_type >          distribution_list;                  ///< List of accounts that receive an equal balance of the stimulus asset.
   vector< account_name_type >          redemption_list;                    ///< List of accounts that can receive and redeem the stimulus asset.
   asset                                distribution_amount;                ///< Amount of stimulus asset distributed each interval.
   date_type                            next_distribution_date;             ///< Date that the next stimulus asset distribution will proceed.
};


struct unique_data_api_obj
{
   unique_data_api_obj( const chain::asset_unique_data_object& c ):
      id( c.id ),
      symbol( c.symbol ),
      controlling_owner( c.controlling_owner ),
      ownership_asset( c.ownership_asset ),
      access_price( c.access_price )
      {
         for( auto acc : c.control_list )
         {
            control_list.push_back( acc );
         }
         for( auto acc : c.access_list )
         {
            access_list.push_back( acc );
         }
      }
      
   unique_data_api_obj(){}

   asset_unique_data_id_type            id;
   asset_symbol_type                    symbol;                      ///< The symbol of the unique asset.
   account_name_type                    controlling_owner;           ///< Account that currently owns the asset. Controls the control list.
   asset_symbol_type                    ownership_asset;             ///< Asset that represents controlling ownership of the unique asset. Same as symbol for no liquid ownership asset.
   vector< account_name_type >          control_list;                ///< List of accounts that have control over access to the unique asset.
   vector< account_name_type >          access_list;                 ///< List of accounts that have access to the unique asset.
   asset                                access_price;                ///< Price per day for all accounts in the access list.
};



struct liquidity_pool_api_obj
{
   liquidity_pool_api_obj( const chain::asset_liquidity_pool_object& p ):
      id( p.id ),
      issuer( p.issuer ),
      symbol_a( p.symbol_a ),
      symbol_b( p.symbol_b ),
      symbol_liquid( p.symbol_liquid ),
      balance_a( p.balance_a ),
      balance_b( p.balance_b ),
      balance_liquid( p.balance_liquid ),
      hour_median_price( p.hour_median_price ),
      day_median_price( p.day_median_price )
      {
         for( auto feed : p.price_history )
         {
            price_history.push_back( feed );
         }
      }

   liquidity_pool_api_obj(){}

   asset_liquidity_pool_id_type           id; 
   account_name_type                      issuer;                        ///< Name of the account which created the liquidity pool.
   asset_symbol_type                      symbol_a;                      ///< Ticker symbol string of the asset with the lower ID. Must be core asset if one asset is core.
   asset_symbol_type                      symbol_b;                      ///< Ticker symbol string of the asset with the higher ID.
   asset_symbol_type                      symbol_liquid;                 ///< Ticker symbol of the pool's liquidity pool asset. 
   asset                                  balance_a;                     ///< Balance of Asset A. Must be core asset if one asset is core.
   asset                                  balance_b;                     ///< Balance of Asset B.
   asset                                  balance_liquid;                ///< Outstanding supply of the liquidity asset for the asset pair.
   price                                  hour_median_price;             ///< The median price over the past hour, at 10 minute intervals. Used for collateral calculations. 
   price                                  day_median_price;              ///< The median price over the last day, at 10 minute intervals.
   vector< price >                        price_history;                 ///< Tracks the last 24 hours of median price, one per 10 minutes.
};



struct credit_pool_api_obj
{
   credit_pool_api_obj( const chain::asset_credit_pool_object& p ):
      id( p.id ),
      issuer( p.issuer ),
      base_symbol( p.base_symbol ),
      credit_symbol( p.credit_symbol ),
      base_balance( p.base_balance ),
      borrowed_balance( p.borrowed_balance ),
      credit_balance( p.credit_balance ),
      last_interest_rate( p.last_interest_rate.value ),
      last_price( p.last_price ){}

   credit_pool_api_obj(){}

   asset_credit_pool_id_type         id; 
   account_name_type                 issuer;                 ///< Name of the account which created the credit pool.
   asset_symbol_type                 base_symbol;            ///< Ticker symbol string of the base asset being lent and borrowed.
   asset_symbol_type                 credit_symbol;          ///< Ticker symbol string of the credit asset for use as collateral to borrow the base asset.
   asset                             base_balance;           ///< Balance of the base asset that is available for loans and redemptions. 
   asset                             borrowed_balance;       ///< Total amount of base asset currently lent to borrowers, accumulates compounding interest payments. 
   asset                             credit_balance;         ///< Balance of the credit asset redeemable for an increasing amount of base asset.
   int64_t                           last_interest_rate;     ///< The most recently calculated interest rate when last compounded. 
   price                             last_price;             ///< The last price that assets were lent or withdrawn at. 
};



struct option_pool_api_obj
{
   option_pool_api_obj( const chain::asset_option_pool_object& p ):
      id( p.id ),
      issuer( p.issuer ),
      base_symbol( p.base_symbol ),
      quote_symbol( p.quote_symbol )
      {
         for( auto sym : p.call_symbols )
         {
            call_symbols.push_back( sym );
         }
         for( auto sym : p.call_strikes )
         {
            call_strikes.push_back( sym );
         }
         for( auto sym : p.put_symbols )
         {
            put_symbols.push_back( sym );
         }
         for( auto sym : p.put_strikes )
         {
            put_strikes.push_back( sym );
         }
      }

   option_pool_api_obj(){}

   asset_option_pool_id_type              id; 
   account_name_type                      issuer;            ///< Name of the account which created the option pool.
   asset_symbol_type                      base_symbol;       ///< Symbol of the base asset of the trading pair.
   asset_symbol_type                      quote_symbol;      ///< Symbol of the quote asset of the trading pair.
   vector< asset_symbol_type >            call_symbols;      ///< Symbols of the call options at currently active strikes.
   vector< option_strike >                call_strikes;      ///< Available strike price and expirations of call options to buy the quote asset.
   vector< asset_symbol_type >            put_symbols;       ///< Symbols of the put options at currently active strikes.
   vector< option_strike >                put_strikes;       ///< Available strike price and expirations of put options to sell the quote asset.
};



struct prediction_pool_api_obj
{
   prediction_pool_api_obj( const chain::asset_prediction_pool_object& p ):
      id( p.id ),
      issuer( p.issuer ),
      prediction_symbol( p.prediction_symbol ),
      collateral_symbol( p.collateral_symbol ),
      collateral_pool( p.collateral_pool ),
      prediction_bond_pool( p.prediction_bond_pool ),
      json( to_string( p.json ) ),
      url( to_string( p.url ) ),
      details( to_string( p.details ) ),
      outcome_time( p.outcome_time ),
      resolution_time( p.resolution_time )
      {
         for( auto sym : p.outcome_assets )
         {
            outcome_assets.push_back( sym );
         }
         for( auto sym : p.outcome_details )
         {
            outcome_details.push_back( to_string( sym ) );
         }
      }

   prediction_pool_api_obj(){}

   asset_prediction_pool_id_type                id; 
   account_name_type                            issuer;                   ///< Name of the account which created the prediction market pool.
   asset_symbol_type                            prediction_symbol;        ///< Ticker symbol of the prediction pool primary asset.
   asset_symbol_type                            collateral_symbol;        ///< Ticker symbol of the collateral asset backing the prediction market.
   asset                                        collateral_pool;          ///< Funds accumulated by outcome asset positions for distribution to winning outcome.
   asset                                        prediction_bond_pool;     ///< Security deposit placed by the issuer on the market.
   vector< asset_symbol_type >                  outcome_assets;           ///< Outcome asset symbols for the market.
   vector< string >                             outcome_details;          ///< Description of each outcome and the resolution conditions for each asset. 
   string                                       json;                     ///< JSON Metadata of the prediction market.
   string                                       url;                      ///< Reference URL of the market.
   string                                       details;                  ///< Description of the market, how it will be resolved using known public data sources.
   time_point                                   outcome_time;             ///< Time at which the prediction market pool becomes open to resolutions.
   time_point                                   resolution_time;          ///< Time at which the prediction market pool will be resolved.
};



struct prediction_pool_resolution_api_obj
{
   prediction_pool_resolution_api_obj( const chain::asset_prediction_pool_resolution_object& p ):
      id( p.id ),
      account( p.account ),
      prediction_symbol( p.prediction_symbol ),
      resolution_outcome( p.resolution_outcome ),
      amount( p.amount ){}

   prediction_pool_resolution_api_obj(){}

   asset_prediction_pool_resolution_id_type     id; 
   account_name_type                            account;                       ///< Name of the account which created the prediction market pool.
   asset_symbol_type                            prediction_symbol;             ///< Ticker symbol of the prediction pool primary asset.
   asset_symbol_type                            resolution_outcome;            ///< Outcome asset symbol for the resolution.
   asset                                        amount;                        ///< Amount of Prediction market base asset spent for vote.
};



struct distribution_api_obj
{
   distribution_api_obj( const chain::asset_distribution_object& p ):
      id( p.id ),
      issuer( p.issuer ),
      distribution_asset( p.distribution_asset ),
      fund_asset( p.fund_asset ),
      details( to_string( p.details ) ),
      url( to_string( p.url ) ),
      json( to_string( p.json ) ),
      distribution_rounds( p.distribution_rounds ),
      distribution_interval_days( p.distribution_interval_days ),
      max_intervals_missed( p.max_intervals_missed ),
      intervals_paid( p.intervals_paid ),
      intervals_missed( p.intervals_missed ),
      min_input_fund_units( p.min_input_fund_units.value ),
      max_input_fund_units( p.max_input_fund_units.value ),
      min_unit_ratio( p.min_unit_ratio.value ),
      max_unit_ratio( p.max_unit_ratio.value ),
      min_input_balance_units( p.min_input_balance_units.value ),
      max_input_balance_units( p.max_input_balance_units.value ),
      total_distributed( p.total_distributed ),
      total_funded( p.total_funded ),
      begin_time( p.begin_time ),
      next_round_time( p.next_round_time ),
      created( p.created ),
      last_updated( p.last_updated )
      {
         for( auto sym : p.input_fund_unit )
         {
            input_fund_unit.push_back( sym );
         }
         for( auto sym : p.output_distribution_unit )
         {
            output_distribution_unit.push_back( sym );
         }
      }

   distribution_api_obj(){}

   asset_distribution_id_type                 id; 
   account_name_type                          issuer;                                ///< Name of the account which created the distribution market pool.
   asset_symbol_type                          distribution_asset;                    ///< Asset that is generated by the distribution.
   asset_symbol_type                          fund_asset;                            ///< Ticker symbol of the asset being accepted for distribution assets.
   string                                     details;                               ///< Description of the distribution process.
   string                                     url;                                   ///< Reference URL of the distribution process.
   string                                     json;                                  ///< JSON metadata of the distribution process.
   uint32_t                                   distribution_rounds;                   ///< Number of distribution rounds, total distribution amount is divided between all rounds.
   uint32_t                                   distribution_interval_days;            ///< Duration of each distribution round, in days.
   uint32_t                                   max_intervals_missed;                  ///< Number of Rounds that can be missed before the distribution is closed early.
   uint32_t                                   intervals_paid;                        ///< Number of Rounds that have been paid out to contributors.
   uint32_t                                   intervals_missed;                      ///< Number of Rounds that have been missed due to not reaching min input fund units.
   vector< asset_unit >                       input_fund_unit;                       ///< The integer unit ratio for distribution of incoming funds.
   vector< asset_unit >                       output_distribution_unit;              ///< The integer unit ratio for distribution of released funds.
   int64_t                                    min_input_fund_units;                  ///< Soft Cap: Minimum fund units required for each round of the distribution process.
   int64_t                                    max_input_fund_units;                  ///< Hard Cap: Maximum fund units to be accepted before closing each distribution round.
   int64_t                                    min_unit_ratio;                        ///< The lowest value of unit ratio between input and output units.
   int64_t                                    max_unit_ratio;                        ///< The highest possible initial value of unit ratio between input and output units.
   int64_t                                    min_input_balance_units;               ///< Minimum fund units that each sender can contribute in an individual balance.
   int64_t                                    max_input_balance_units;               ///< Maximum fund units that each sender can contribute in an individual balance.
   asset                                      total_distributed;                     ///< Amount of distirbution asset generated and distributed to all fund balances.
   asset                                      total_funded;                          ///< Amount of fund asset funded by all incoming fund balances.
   time_point                                 begin_time;                            ///< Time to begin the first distribution.
   time_point                                 next_round_time;                       ///< Time of the next distribution round.
   time_point                                 created;                               ///< Time that the distribution was created.
   time_point                                 last_updated;                          ///< Time that the distribution was last updated.
};



struct distribution_balance_api_obj
{
   distribution_balance_api_obj( const chain::asset_distribution_balance_object& p ):
      id( p.id ),
      sender( p.sender ),
      distribution_asset( p.distribution_asset ),
      amount( p.amount ),
      created( p.created ),
      last_updated( p.last_updated ){}

   distribution_balance_api_obj(){}

   asset_distribution_balance_id_type     id; 
   account_name_type                      sender;                  ///< Name of the account which sent the funds.
   asset_symbol_type                      distribution_asset;      ///< Asset that is generated by the distribution.
   asset                                  amount;                  ///< Amount of funding asset sent to the distribution, redeemed at next interval. Refunded if the soft cap is not reached.
   time_point                             created;                 ///< Time that the distribution balance was created.
   time_point                             last_updated;            ///< Time that the distribution balance was last updated.
};



//================================//
// ===== Credit API Objects ===== //
//================================//



struct credit_collateral_api_obj
{
   credit_collateral_api_obj( const chain::credit_collateral_object& o ):
      id( o.id ),
      owner( o.owner ),
      symbol( o.symbol ),
      collateral( o.collateral ){}

   credit_collateral_api_obj(){}

   credit_collateral_id_type                    id;
   account_name_type                            owner;         ///< Collateral owners account name.
   asset_symbol_type                            symbol;        ///< Asset symbol being collateralized. 
   asset                                        collateral;    ///< Asset balance that is being locked in for loan backing for loan or margin orders.  
};



struct credit_loan_api_obj
{
   credit_loan_api_obj( const chain::credit_loan_object& o ):
      id( o.id ),
      owner( o.owner ),
      loan_id( to_string( o.loan_id ) ),
      debt( o.debt ),
      interest( o.interest ),
      collateral( o.collateral ),
      loan_price( o.loan_price ),
      liquidation_price( o.liquidation_price ),
      symbol_a( o.symbol_a ),
      symbol_b( o.symbol_b ),
      last_interest_rate( o.last_interest_rate.value ),
      created( o.created ),
      last_updated( o.last_updated ){}
   
   credit_loan_api_obj(){}

   credit_loan_id_type        id;
   account_name_type          owner;                   ///< Collateral owner's account name
   string                     loan_id;                 ///< UUIDV4 for the loan to uniquely identify it for reference. 
   asset                      debt;                    ///< Amount of an asset borrowed. Limit of 75% of collateral value. Increases with interest charged.
   asset                      interest;                ///< Total Amount of interest accrued on the loan. 
   asset                      collateral;              ///< Amount of an asset to use as collateral for the loan. 
   price                      loan_price;              ///< Collateral / Debt. Must be higher than liquidation price to remain solvent. 
   price                      liquidation_price;       ///< Collateral / max_debt value. Rises when collateral/debt market price falls.
   asset_symbol_type          symbol_a;                ///< The symbol of asset A in the debt / collateral exchange pair.
   asset_symbol_type          symbol_b;                ///< The symbol of asset B in the debt / collateral exchange pair.
   int64_t                    last_interest_rate;      ///< Updates the interest rate of the loan hourly. 
   time_point                 created;                 ///< Time that the loan was taken out.
   time_point                 last_updated;            ///< Time that the loan was last updated, and interest was accrued.
};



//========================================//
// ===== Block Producer API Objects ===== //
//========================================//



struct producer_api_obj
{
   producer_api_obj( const chain::producer_object& p ) :
      id( p.id ),
      owner( p.owner ),
      active( p.active ),
      schedule( p.schedule ),
      last_confirmed_block_num( p.last_confirmed_block_num ),
      details( to_string( p.details ) ),
      url( to_string( p.url ) ),
      json( to_string( p.json ) ),
      latitude( p.latitude ),
      longitude( p.longitude ),
      signing_key( p.signing_key ),
      last_commit_height( p.last_commit_height ),
      last_commit_id( p.last_commit_id ),
      total_blocks( p.total_blocks ),
      voting_power( p.voting_power.value ),
      vote_count( p.vote_count ),
      mining_power( p.mining_power.value ),
      mining_count( p.mining_count ),
      last_mining_update( p.last_mining_update ),
      last_pow_time( p.last_pow_time ),
      recent_txn_stake_weight( p.recent_txn_stake_weight ),
      last_txn_stake_weight_update( p.last_txn_stake_weight_update ),
      accumulated_activity_stake( p.accumulated_activity_stake ),
      total_missed( p.total_missed ),
      last_aslot( p.last_aslot ),
      props( p.props ),
      voting_virtual_last_update( p.voting_virtual_last_update ),
      voting_virtual_position( p.voting_virtual_position ),
      voting_virtual_scheduled_time( p.voting_virtual_scheduled_time ),
      mining_virtual_last_update( p.mining_virtual_last_update ),
      mining_virtual_position( p.mining_virtual_position ),
      mining_virtual_scheduled_time( p.mining_virtual_scheduled_time ),
      running_version( p.running_version ),
      hardfork_version_vote( p.hardfork_version_vote ),
      hardfork_time_vote( p.hardfork_time_vote ),
      created( p.created ),
      last_updated( p.last_updated ){}

   producer_api_obj(){}

   producer_id_type             id;
   account_name_type            owner;                            ///< The name of the account that has authority over this producer.
   bool                         active;                           ///< True if the producer is actively seeking to produce blocks, set false to deactivate the producer and remove from production.
   producer_object::producer_schedule_type        schedule;       ///< How the producer was scheduled the last time it was scheduled.
   uint64_t                     last_confirmed_block_num;         ///< Number of the last block that was successfully produced by this producer. 
   string                       details;                          ///< Producer's details, explaining who they are, machine specs, capabilties.
   string                       url;                              ///< The producer's URL explaining their details.
   string                       json;                             ///< The producer's json metadata.
   double                       latitude;                         ///< Latitude Co-ordinates of the producer's approximate geo-location.
   double                       longitude;                        ///< Longitude Co-ordinates of the producer's approximate geo-location.
   public_key_type              signing_key;                      ///< The key used to sign blocks on behalf of this producer.
   uint32_t                     last_commit_height;               ///< Block height that has been most recently committed by the producer
   block_id_type                last_commit_id;                   ///< Block ID of the height that was most recently committed by the producer. 
   uint32_t                     total_blocks;                     ///< Accumulated number of blocks produced.
   int64_t                      voting_power;                     ///< The total weighted voting power that supports the producer. 
   uint32_t                     vote_count;                       ///< The number of accounts that have voted for the producer.
   int64_t                      mining_power;                     ///< The amount of proof of work difficulty accumulated by the miner over the prior 7 days.
   uint32_t                     mining_count;                     ///< Accumulated number of proofs of work published.
   time_point                   last_mining_update;               ///< Time that the account last updated its mining power.
   time_point                   last_pow_time;                    ///< Time that the miner last created a proof of work.
   uint128_t                    recent_txn_stake_weight;          ///< Rolling average Amount of transaction stake weight contained that the producer has included in blocks over the prior 7 days.
   time_point                   last_txn_stake_weight_update;     ///< Time that the recent bandwith and txn stake were last updated.
   uint128_t                    accumulated_activity_stake;       ///< Recent amount of activity reward stake for the prime producer. 
   uint32_t                     total_missed;                     ///< Number of blocks missed recently.
   uint64_t                     last_aslot;                       ///< Last absolute slot that the producer was assigned to produce a block.
   chain_properties             props;                            ///< The chain properties object that the producer currently proposes for global network variables
   uint128_t                    voting_virtual_last_update;
   uint128_t                    voting_virtual_position;
   uint128_t                    voting_virtual_scheduled_time;
   uint128_t                    mining_virtual_last_update;
   uint128_t                    mining_virtual_position;
   uint128_t                    mining_virtual_scheduled_time;
   version                      running_version;                  ///< The blockchain version the producer is running.
   hardfork_version             hardfork_version_vote;
   time_point                   hardfork_time_vote;
   time_point                   created;                          ///< The time the producer was created.
   time_point                   last_updated;                     ///< The time the producer was last updated.
};


struct block_validation_api_obj
{
   block_validation_api_obj( const chain::block_validation_object& o ) :
      id( o.id ),
      producer( o.producer ),
      block_id( o.block_id ),
      block_height( o.block_height ),
      verify_txn( o.verify_txn ),
      committed( o.committed ),
      commit_time( o.commit_time ),
      commitment_stake( o.commitment_stake ),
      commit_txn( o.commit_txn ),
      created( o.created )
      {
         for( auto id : o.verifications )
         {
            verifications.push_back( id );
         }
         for( auto acc : o.verifiers )
         {
            verifiers.push_back( acc );
         }
      }

   block_validation_api_obj(){}

   block_validation_id_type              id;
   account_name_type                     producer;             ///< Name of the block producer creating the validation.
   block_id_type                         block_id;             ///< Block ID commited to for the height.
   uint64_t                              block_height;         ///< Height of the block being validated.
   transaction_id_type                   verify_txn;           ///< Transaction ID in which this validation object was verified.
   vector< transaction_id_type >         verifications;        ///< Validation transactions from other producers.
   vector< account_name_type >           verifiers;            ///< Accounts that have also verfied this block id at this height.
   bool                                  committed;            ///< True if the validation has been committed with stake.
   time_point                            commit_time;          ///< Time that the validation was committed.
   asset                                 commitment_stake;     ///< Amount of COIN staked on the validity of the block.
   transaction_id_type                   commit_txn;           ///< Transaction ID in which this validation object was commited.
   time_point                            created;              ///< Time that the validation was created.
};


struct commit_violation_api_obj
{
   commit_violation_api_obj( const chain::commit_violation_object& o ) :
      id( o.id ),
      reporter( o.reporter ),
      producer( o.producer ),
      block_height( o.block_height ),
      first_trx( o.first_trx ),
      second_trx( o.second_trx ),
      created( o.created ),
      forfeited_stake( o.forfeited_stake ){}

   commit_violation_api_obj(){}

   commit_violation_id_type         id;
   account_name_type                reporter;                ///< Name of the account creating the violation.
   account_name_type                producer;                ///< Name of the account that violated a commit transaction
   uint64_t                         block_height;            ///< Height of the block that the violation occured on.
   signed_transaction               first_trx;               ///< First Transaction that conflicts.
   signed_transaction               second_trx;              ///< Second Transaction that conflicts.
   time_point                       created;                 ///< Time that the violation was created.
   asset                            forfeited_stake;         ///< Stake value that was forfeited.
};


struct tag_api_obj
{
   tag_api_obj( const tags::tag_stats_object& o ):
      id( o.id ),
      tag( o.tag ),
      total_payout( o.total_payout ),
      post_count( o.post_count ),
      children( o.children ),
      net_votes( o.net_votes ),
      view_count( o.view_count ),
      share_count( o.share_count ),
      net_reward( o.net_reward.value ),
      vote_power( o.vote_power.value ),
      view_power( o.view_power.value ),
      share_power( o.share_power.value ),
      comment_power( o.comment_power.value ){}

   tag_api_obj(){}

   tags::tag_stats_id_type    id;
   tag_name_type              tag;             ///< Name of the tag being measured.
   asset                      total_payout;    ///< USD value of all earned content rewards for all posts using the tag.
   uint32_t                   post_count;      ///< Number of posts using the tag.
   uint32_t                   children;        ///< The amount of comments on root posts for all posts using the tag.
   int32_t                    net_votes;       ///< The amount of upvotes, minus downvotes for all posts using the tag.
   int32_t                    view_count;      ///< The amount of views for all posts using the tag.
   int32_t                    share_count;     ///< The amount of shares for all posts using the tag.
   int64_t                    net_reward;      ///< Net reward is the sum of all vote, view, share and comment power, with the reward curve formula applied. 
   int64_t                    vote_power;      ///< Sum of weighted voting power for all posts using the tag.
   int64_t                    view_power;      ///< Sum of weighted view power for all posts using the tag.
   int64_t                    share_power;     ///< Sum of weighted share power for all posts using the tag.
   int64_t                    comment_power;   ///< Sum of weighted comment power for all posts using the tag.
};


struct signed_block_api_obj : public signed_block
{
   signed_block_api_obj( const signed_block& block ) : signed_block( block )
   {
      block_id = id();
      signing_key = signee();
      transaction_ids.reserve( transactions.size() );
      for( const signed_transaction& tx : transactions )
      {
         transaction_ids.push_back( tx.id() );
      } 
   }

   signed_block_api_obj(){}

   block_id_type                     block_id;
   public_key_type                   signing_key;
   vector< transaction_id_type >     transaction_ids;
};



} } // node::app



//================================//
// ===== Global API Objects ===== //
//================================//


FC_REFLECT_DERIVED( node::app::dynamic_global_property_api_obj, (node::chain::dynamic_global_property_object),
         (current_reserve_ratio)
         (average_block_size)
         (max_virtual_bandwidth)
         );

FC_REFLECT( node::app::median_chain_property_api_obj,
         (id)
         (account_creation_fee)
         (asset_coin_liquidity)
         (asset_usd_liquidity)
         (maximum_block_size)
         (pow_target_time)
         (pow_decay_time)
         (txn_stake_decay_time)
         (escrow_bond_percent)
         (credit_interest_rate)
         (credit_open_ratio)
         (credit_liquidation_ratio)
         (credit_min_interest)
         (credit_variable_interest)
         (market_max_credit_ratio)
         (margin_open_ratio)
         (margin_liquidation_ratio)
         (maximum_asset_feed_publishers)
         (membership_base_price)
         (membership_mid_price)
         (membership_top_price)
         (author_reward_percent)
         (vote_reward_percent)
         (view_reward_percent)
         (share_reward_percent)
         (comment_reward_percent)
         (storage_reward_percent)
         (moderator_reward_percent)
         (content_reward_decay_rate)
         (content_reward_interval)
         (vote_reserve_rate)
         (view_reserve_rate)
         (share_reserve_rate)
         (comment_reserve_rate)
         (vote_recharge_time)
         (view_recharge_time)
         (share_recharge_time)
         (comment_recharge_time)
         (curation_auction_decay_time)
         (vote_curation_decay)
         (view_curation_decay)
         (share_curation_decay)
         (comment_curation_decay)
         (supernode_decay_time)
         (enterprise_vote_percent_required)
         (maximum_asset_whitelist_authorities)
         (max_stake_intervals)
         (max_unstake_intervals)
         (max_exec_budget)
         );

FC_REFLECT( node::app::reward_fund_api_obj,
         (id)
         (content_reward_balance)
         (validation_reward_balance) 
         (txn_stake_reward_balance) 
         (work_reward_balance)
         (producer_activity_reward_balance) 
         (supernode_reward_balance)
         (power_reward_balance)
         (community_fund_balance)
         (development_reward_balance)
         (marketing_reward_balance)
         (advocacy_reward_balance)
         (activity_reward_balance)
         (premium_partners_fund_balance)
         (total_pending_reward_balance)
         (recent_content_claims)
         (recent_activity_claims)
         (content_constant)
         (content_reward_decay_rate)
         (author_reward_curve)
         (curation_reward_curve)
         (last_updated)
         );


//=================================//
// ===== Account API Objects ===== //
//=================================//



FC_REFLECT( node::app::account_api_obj,
         (id)
         (name)
         (details)
         (json)
         (json_private)
         (url)
         (first_name)
         (last_name)
         (gender)
         (date_of_birth)
         (email)
         (phone)
         (nationality)
         (pinned_permlink)
         (membership)
         (secure_public_key)
         (connection_public_key)
         (friend_public_key)
         (companion_public_key)
         (proxy)
         (proxied)
         (registrar)
         (referrer)
         (recovery_account)
         (reset_account)
         (membership_interface)
         (reset_account_delay_days)
         (referrer_rewards_percentage)
         (comment_count)
         (follower_count)
         (following_count)
         (post_vote_count)
         (post_count)
         (voting_power)
         (viewing_power)
         (sharing_power)
         (commenting_power)
         (savings_withdraw_requests)
         (withdraw_routes)
         (posting_rewards)
         (curation_rewards)
         (moderation_rewards)
         (total_rewards)
         (author_reputation)
         (loan_default_balance)
         (recent_activity_claims)
         (producer_vote_count)
         (officer_vote_count)
         (executive_board_vote_count)
         (governance_subscriptions)
         (recurring_membership)
         (created)
         (membership_expiration)
         (last_account_update)
         (last_vote_time)
         (last_view_time)
         (last_share_time)
         (last_post)
         (last_root_post)
         (last_transfer_time)
         (last_activity_reward)
         (last_account_recovery)
         (last_community_created)
         (last_asset_created)
         (last_owner_update)
         (average_bandwidth)
         (lifetime_bandwidth)
         (last_bandwidth_update)
         (mined)
         (revenue_share)
         (can_vote)
         (active)
         );

FC_REFLECT( node::app::account_concise_api_obj,
         (id)
         (name)
         (details)
         (image)
         (membership)
         (follower_count)
         (following_count)
         (total_rewards)
         (author_reputation)
         (created)
         (active)
         );

FC_REFLECT( node::app::account_verification_api_obj,
         (id)
         (verifier_account)
         (verified_account)
         (shared_image)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::account_business_api_obj,
         (id)
         (account)
         (business_type)
         (business_public_key)
         (executive_board)
         (executive_votes)
         (executives)
         (officer_votes)
         (officers)
         (members)
         (officer_vote_threshold)
         (equity_assets)
         (credit_assets)
         (equity_revenue_shares)
         (credit_revenue_shares)
         );

FC_REFLECT( node::app::account_permission_api_obj,
         (id)
         (account)
         (whitelisted_accounts)
         (blacklisted_accounts)
         (whitelisted_assets)
         (blacklisted_assets)
         );

FC_REFLECT( node::app::account_request_api_obj,
         (id)
         (account)
         (business_account)
         (message)
         (expiration)
         );

FC_REFLECT( node::app::account_invite_api_obj,
         (id)
         (account)
         (business_account)
         (member)
         (message)
         (expiration)
         );

FC_REFLECT( node::app::account_vesting_balance_api_obj,
         (id)
         (owner)
         (symbol)
         (vesting_balance)
         (vesting_time)
         );

FC_REFLECT( node::app::account_balance_api_obj,
         (id)
         (owner)
         (symbol)
         (liquid_balance)
         (staked_balance)
         (reward_balance)
         (savings_balance)
         (delegated_balance)
         (receiving_balance)
         (total_balance)
         (stake_rate)
         (next_stake_time)
         (to_stake)
         (total_staked)
         (unstake_rate)
         (next_unstake_time)
         (to_unstake)
         (total_unstaked)
         (last_interest_time)
         );

FC_REFLECT( node::app::account_following_api_obj,
         (id)
         (account)
         (followers)
         (following)
         (mutual_followers)
         (connections)
         (friends)
         (companions)
         (followed_communities)
         (followed_tags)
         (filtered)
         (filtered_communities)
         (filtered_tags)
         (last_updated)
         );

FC_REFLECT( node::app::tag_following_api_obj,
         (id)
         (tag)
         (followers)
         (last_updated)
         );

FC_REFLECT( node::app::connection_api_obj,
         (id)
         (account_a)
         (encrypted_key_a)
         (account_b)
         (encrypted_key_b)
         (connection_type)
         (connection_id)
         (message_count)
         (consecutive_days)
         (last_message_time_a)
         (last_message_time_b)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::connection_request_api_obj,
         (id)
         (account)
         (requested_account)
         (connection_type)
         (message)
         (expiration)
         );

FC_REFLECT( node::app::owner_authority_history_api_obj,
         (id)
         (account)
         (previous_owner_authority)
         (last_valid_time)
         );

FC_REFLECT( node::app::account_recovery_request_api_obj,
         (id)
         (account_to_recover)
         (new_owner_authority)
         (expiration)
         );


//=================================//
// ===== Network API Objects ===== //
//=================================//


FC_REFLECT( node::app::network_officer_api_obj,
         (id)
         (account)
         (active)
         (officer_approved)
         (officer_type)
         (details)
         (url)
         (json)
         (created)
         (vote_count)
         (voting_power)
         (producer_vote_count)
         (producer_voting_power)
         );

FC_REFLECT( node::app::network_officer_vote_api_obj,
         (id)
         (account)
         (network_officer)
         (officer_type)
         (vote_rank)
         );


FC_REFLECT( node::app::executive_board_api_obj,
         (id)
         (account)
         (active)
         (board_approved)
         (budget)
         (details)
         (url)
         (json)
         (created)
         (vote_count)
         (voting_power)
         (producer_vote_count)
         (producer_voting_power)
         );

FC_REFLECT( node::app::executive_board_vote_api_obj,
         (id)
         (account)
         (executive_board)
         (vote_rank)
         );

FC_REFLECT( node::app::governance_account_api_obj,
         (id)
         (account)
         (active)
         (account_approved)
         (details)
         (url)
         (json)
         (created)
         (subscriber_count)
         (subscriber_power)
         (producer_subscriber_count)
         (producer_subscriber_power)
         );

FC_REFLECT( node::app::governance_subscription_api_obj,
         (id)
         (account)
         (governance_account)
         (vote_rank)
         );

FC_REFLECT( node::app::supernode_api_obj,
         (id)
         (account)
         (active)
         (details)
         (url)
         (node_api_endpoint)
         (notification_api_endpoint)
         (auth_api_endpoint)
         (ipfs_endpoint)
         (bittorrent_endpoint)
         (json)
         (created)
         (storage_rewards)
         (daily_active_users)
         (monthly_active_users)
         (recent_view_weight)
         (last_updated)
         (last_activation_time)
         );

FC_REFLECT( node::app::interface_api_obj,
         (id)
         (account)
         (active)
         (details)
         (url)
         (json)
         (created)
         (daily_active_users)
         (monthly_active_users)
         (last_updated)
         );

FC_REFLECT( node::app::mediator_api_obj,
         (id)
         (account)
         (active)
         (details)
         (url)
         (json)
         (mediator_bond)
         (mediation_virtual_position)
         (last_escrow_from)
         (last_escrow_id)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::community_enterprise_api_obj,
         (id)
         (creator)
         (enterprise_id)
         (active)
         (proposal_type)
         (beneficiaries)
         (milestone_shares)
         (milestone_details)
         (milestone_history)
         (approved_milestones)
         (claimed_milestones)
         (investment)
         (details)
         (url)
         (json)
         (begin)
         (end)
         (expiration)
         (daily_budget)
         (duration)
         (pending_budget)
         (total_distributed)
         (days_paid)
         (total_approvals)
         (total_voting_power)
         (total_producer_approvals)
         (total_producer_voting_power)
         (current_approvals)
         (current_voting_power)
         (current_producer_approvals)
         (current_producer_voting_power)
         (created)
         );

FC_REFLECT( node::app::enterprise_approval_api_obj,
         (id)
         (account)
         (creator)
         (enterprise_id)
         (vote_rank)
         (milestone)
         );


//=================================//
// ===== Comment API Objects ===== //
//=================================//


FC_REFLECT( node::app::comment_api_obj,
         (id)
         (author)
         (permlink)
         (title)
         (post_type)
         (public_key)
         (encrypted)
         (reach)
         (reply_connection)
         (community)
         (tags)
         (body)
         (ipfs)
         (magnet)
         (url)
         (interface)
         (rating)
         (language)
         (root_comment)
         (parent_author)
         (parent_permlink)
         (json)
         (category)
         (latitude)
         (longitude)
         (comment_price)
         (reply_price)
         (premium_price)
         (payments_received)
         (beneficiaries)
         (last_updated)
         (created)
         (active)
         (last_payout)
         (depth)
         (children)
         (net_votes)
         (view_count)
         (share_count)
         (net_reward)
         (vote_power)
         (view_power)
         (share_power)
         (comment_power)
         (cashout_time)
         (cashouts_received)
         (total_vote_weight)
         (total_view_weight)
         (total_share_weight)
         (total_comment_weight)
         (total_payout_value)
         (curator_payout_value)
         (beneficiary_payout_value)
         (content_rewards)
         (percent_liquid)
         (reward)
         (weight)
         (max_weight)
         (max_accepted_payout)
         (author_reward_percent)
         (vote_reward_percent)
         (view_reward_percent)
         (share_reward_percent)
         (comment_reward_percent)
         (storage_reward_percent)
         (moderator_reward_percent)
         (allow_replies)
         (allow_votes)
         (allow_views)
         (allow_shares)
         (allow_curation_rewards)
         (root)
         (deleted)
         );

FC_REFLECT( node::app::moderation_tag_api_obj,
         (id)
         (moderator)
         (comment)
         (community)
         (tags)
         (rating)
         (details)
         (interface)
         (filter)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::message_api_obj,
         (id)
         (sender)
         (recipient)
         (sender_public_key)
         (recipient_public_key)
         (message)
         (json)
         (uuid)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::list_api_obj,
         (id)
         (creator)
         (list_id)
         (name)
         (accounts)
         (comments)
         (communities)
         (assets)
         (products)
         (auctions)
         (nodes)
         (edges)
         (node_types)
         (edge_types)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::poll_api_obj,
         (id)
         (creator)
         (poll_id)
         (details)
         (poll_options)
         (completion_time)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::poll_vote_api_obj,
         (id)
         (voter)
         (creator)
         (poll_id)
         (poll_option)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::blog_api_obj,
         (id)
         (account)
         (community)
         (tag)
         (comment)
         (shared_by)
         (blog_type)
         (first_shared_by)
         (shares)
         (blog_time)
         );

FC_REFLECT( node::app::feed_api_obj,
         (id)
         (account)
         (comment)
         (feed_type)
         (shared_by)
         (communities)
         (tags)
         (first_shared_by)
         (shares)
         (feed_time)
         );


//===================================//
// ===== Community API Objects ===== //
//===================================//


FC_REFLECT( node::app::community_api_obj,
         (id)
         (name)
         (founder)
         (community_privacy)
         (community_public_key)
         (json)
         (json_private)
         (pinned_author)
         (pinned_permlink)
         (subscriber_count)
         (post_count)
         (comment_count)
         (vote_count)
         (view_count)
         (share_count)
         (reward_currency)
         (created)
         (last_community_update)
         (last_post)
         (last_root_post)
         (active)
         );

FC_REFLECT( node::app::community_request_api_obj,
         (id)
         (account)
         (community)
         (message)
         (expiration)
         );

FC_REFLECT( node::app::community_invite_api_obj,
         (id)
         (account)
         (community)
         (member)
         (message)
         (expiration)
         );

FC_REFLECT( node::app::community_event_api_obj,
         (id)
         (account)
         (community)
         (event_name)
         (location)
         (latitude)
         (longitude)
         (details)
         (url)
         (json)
         (invited)
         (interested)
         (attending)
         (not_attending)
         (event_start_time)
         (event_end_time)
         (last_updated)
         (created)
         );

//=====================================//
// ===== Advertising API Objects ===== //
//=====================================//



FC_REFLECT( node::app::ad_creative_api_obj,
         (id)
         (account)
         (creative_id)
         (format_type) 
         (author)
         (objective)
         (creative)
         (json)
         (created)
         (last_updated)
         (active)
         );

FC_REFLECT( node::app::ad_campaign_api_obj,
         (id)
         (account)
         (campaign_id)  
         (budget)
         (total_bids)
         (begin)
         (end)
         (json)
         (agents)
         (interface)
         (created)
         (last_updated)
         (active)
         );

FC_REFLECT( node::app::ad_inventory_api_obj,
         (id)
         (provider)
         (inventory_id)  
         (metric)
         (audience_id)
         (min_price)
         (inventory)
         (remaining)
         (json)
         (created)
         (last_updated)
         (expiration)
         (active)
         );

FC_REFLECT( node::app::ad_audience_api_obj,
         (id)
         (account)
         (audience_id)
         (json)
         (audience)
         (created)
         (last_updated)
         (active)
         );

FC_REFLECT( node::app::ad_bid_api_obj,
         (id)
         (bidder)
         (bid_id)
         (audience_id)
         (account)
         (campaign_id)
         (author)
         (creative_id)
         (provider)
         (inventory_id)
         (bid_price)
         (format)
         (metric)
         (objective)
         (requested)
         (remaining)
         (delivered)
         (json)
         (created)
         (last_updated)
         (expiration)
         );


//====================================//
// ===== Graph Data API Objects ===== //
//====================================//


FC_REFLECT( node::app::graph_node_api_obj,
         (id)
         (account)
         (node_types)
         (node_id)
         (name)
         (details)
         (attributes)
         (attribute_values)
         (json)
         (json_private)
         (node_public_key)
         (interface)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::graph_edge_api_obj,
         (id)
         (account)
         (edge_types)
         (edge_id)
         (from_node)
         (to_node)
         (name)
         (details)
         (attributes)
         (attribute_values)
         (json)
         (json_private)
         (edge_public_key)
         (interface)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::graph_node_property_api_obj,
         (id)
         (account)
         (node_type)
         (graph_privacy)
         (edge_permission)
         (details)
         (url)
         (json)
         (attributes)
         (interface)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::graph_edge_property_api_obj,
         (id)
         (account)
         (edge_type)
         (graph_privacy)
         (from_node_types)
         (to_node_types)
         (details)
         (url)
         (json)
         (attributes)
         (interface)
         (created)
         (last_updated)
         );


//==================================//
// ===== Transfer API Objects ===== //
//==================================//


FC_REFLECT( node::app::transfer_request_api_obj,
         (id)
         (to)
         (from)
         (amount)
         (request_id)
         (memo)
         (expiration)
         );

FC_REFLECT( node::app::transfer_recurring_api_obj,
         (id)
         (from)
         (to)
         (amount)
         (transfer_id)
         (memo)
         (begin)
         (end)
         (interval)
         (next_transfer)
         );

FC_REFLECT( node::app::transfer_recurring_request_api_obj,
         (id)
         (from)
         (to)
         (amount)
         (request_id)
         (memo)
         (begin)
         (end)
         (interval)
         (expiration)
         );


//=================================//
// ===== Balance API Objects ===== //
//=================================//


FC_REFLECT( node::app::savings_withdraw_api_obj,
         (id)
         (from)
         (to)
         (memo)
         (request_id)
         (amount)
         (complete)
         );

FC_REFLECT( node::app::confidential_balance_api_obj,
         (id)
         (owner)
         (prev)
         (op_in_trx)
         (index)
         (commitment)
         (symbol)
         (created)
         );


//=====================================//
// ===== Marketplace API Objects ===== //
//=====================================//


FC_REFLECT( node::app::product_sale_api_obj,
         (id)
         (account)
         (product_id)
         (name)
         (url)
         (json)
         (product_variants)
         (product_details)
         (product_images)
         (product_prices)
         (stock_available)
         (delivery_variants)
         (delivery_details)
         (delivery_prices)
         (created)
         (last_updated)
         (active)
         );

FC_REFLECT( node::app::product_purchase_api_obj,
         (id)
         (buyer)
         (order_id)
         (seller)
         (product_id)
         (order_variants)
         (order_size)
         (memo)
         (json)
         (purchase_public_key)
         (shipping_address)
         (delivery_variant)
         (delivery_details)
         (order_value)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::product_auction_sale_api_obj,
         (id)
         (account)
         (auction_id)
         (auction_type)
         (name)
         (url)
         (json)
         (product_details)
         (product_images)
         (reserve_bid)
         (maximum_bid)
         (delivery_variants)
         (delivery_details)
         (delivery_prices)
         (final_bid_time)
         (completion_time)
         (winning_bid)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::product_auction_bid_api_obj,
         (id)
         (buyer)
         (bid_id)
         (seller)
         (auction_id)
         (bid_asset)
         (bid_price_commitment)
         (blinding_factor)
         (public_bid_amount)
         (memo)
         (json)
         (bid_public_key)
         (shipping_address)
         (delivery_variant)
         (delivery_details)
         (delivery_value)
         (created)
         (last_updated)
         (completion_time)
         (winning_bid)
         );

FC_REFLECT( node::app::escrow_api_obj,
         (id)
         (from)
         (to)
         (from_mediator)
         (to_mediator)
         (payment)
         (balance)
         (escrow_id)
         (memo)
         (json)
         (acceptance_time)
         (escrow_expiration)
         (dispute_release_time)
         (mediators)
         (release_percentages)
         (approvals)
         (created)
         (last_updated)
         (disputed)
         );


//=================================//
// ===== Trading API Objects ===== //
//=================================//    


FC_REFLECT( node::app::limit_order_api_obj,
         (id)
         (seller)
         (order_id)
         (for_sale)
         (sell_price)
         (interface)
         (created)
         (expiration)
         (real_price)
         );

FC_REFLECT( node::app::margin_order_api_obj,
         (id)
         (owner)
         (order_id)
         (sell_price)
         (collateral)
         (debt)
         (debt_balance)
         (interest)
         (position)
         (position_balance)
         (collateralization)
         (interface)
         (created)
         (last_updated)
         (last_interest_time)
         (expiration)
         (unrealized_value)
         (last_interest_rate)
         (liquidating)
         (stop_loss_price)
         (take_profit_price)
         (limit_stop_loss_price)
         (limit_take_profit_price)
         (real_price)
         );

FC_REFLECT( node::app::auction_order_api_obj,
         (id)
         (owner)
         (order_id)
         (amount_to_sell)
         (limit_close_price)
         (interface)
         (expiration)
         (created)
         (last_updated)
         (real_price)
         );

FC_REFLECT( node::app::call_order_api_obj,
         (id)
         (borrower)
         (collateral)
         (debt)
         (call_price)
         (target_collateral_ratio)
         (interface)
         (real_price)
         );

FC_REFLECT( node::app::option_order_api_obj,
         (id)
         (owner)
         (order_id)
         (option_position)
         (underlying_amount)
         (exercise_amount)
         (strike_price)
         (interface)
         (created)
         (last_updated)
         (real_price)
         );


//===============================//
// ===== Asset API Objects ===== //
//===============================//


FC_REFLECT( node::app::asset_api_obj,
         (id)
         (symbol)
         (asset_type)
         (issuer)
         (display_symbol)
         (details)
         (json)
         (url)
         (max_supply)
         (stake_intervals)
         (unstake_intervals)
         (market_fee_percent)
         (market_fee_share_percent)
         (max_market_fee)
         (issuer_permissions)
         (flags)
         (whitelist_authorities)
         (blacklist_authorities)
         (whitelist_markets)
         (blacklist_markets)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::currency_data_api_obj,
         (id)
         (symbol)
         (block_reward)
         (block_reward_reduction_percent)
         (block_reward_reduction_days)
         (content_reward_percent)
         (equity_asset)
         (equity_reward_percent)
         (producer_reward_percent)
         (supernode_reward_percent)
         (power_reward_percent)
         (community_fund_reward_percent)
         (development_reward_percent)
         (marketing_reward_percent)
         (advocacy_reward_percent)
         (activity_reward_percent)
         (producer_block_reward_percent)
         (validation_reward_percent)
         (txn_stake_reward_percent)
         (work_reward_percent)
         (producer_activity_reward_percent)
         );

FC_REFLECT( node::app::stablecoin_data_api_obj,
         (id)
         (symbol)
         (issuer)
         (backing_asset)
         (feeds)
         (current_feed)
         (current_feed_publication_time)
         (current_maintenance_collateralization)
         (force_settled_volume)
         (settlement_price)
         (settlement_fund)
         (feed_lifetime)
         (minimum_feeds)
         (asset_settlement_delay)
         (asset_settlement_offset_percent)
         (maximum_asset_settlement_volume)
         );

FC_REFLECT( node::app::asset_settlement_api_obj, 
         (id)
         (owner)
         (balance)
         (settlement_date)
         (interface)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::asset_collateral_bid_api_obj, 
         (id)
         (bidder)
         (collateral)
         (debt)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::equity_data_api_obj,
         (id)
         (business_account)
         (symbol)
         (dividend_pool)
         (last_dividend)
         (dividend_share_percent)
         (liquid_dividend_percent)
         (staked_dividend_percent)
         (savings_dividend_percent)
         (liquid_voting_rights)
         (staked_voting_rights)
         (savings_voting_rights)
         (min_active_time)
         (min_balance)
         (min_producers)
         (boost_balance)
         (boost_activity)
         (boost_producers)
         (boost_top)
         );

FC_REFLECT( node::app::bond_data_api_obj,
         (id)
         (business_account)
         (symbol)
         (value)
         (collateralization)
         (coupon_rate_percent)
         (maturity_date)
         (collateral_pool)
         );

FC_REFLECT( node::app::credit_data_api_obj,
         (id)
         (business_account)
         (symbol)
         (buyback_asset)
         (buyback_pool)
         (buyback_price)
         (last_buyback)
         (buyback_share_percent)
         (liquid_fixed_interest_rate)
         (liquid_variable_interest_rate)
         (staked_fixed_interest_rate)
         (staked_variable_interest_rate)
         (savings_fixed_interest_rate)
         (savings_variable_interest_rate)
         (var_interest_range)
         );

FC_REFLECT( node::app::stimulus_data_api_obj,
         (id)
         (business_account)
         (symbol)
         (redemption_asset)
         (redemption_pool)
         (redemption_price)
         (distribution_list)
         (redemption_list)
         (distribution_amount)
         (next_distribution_date)
         );

FC_REFLECT( node::app::unique_data_api_obj,
         (id)
         (symbol)
         (controlling_owner)
         (ownership_asset)
         (control_list)
         (access_list)
         (access_price)
         );

FC_REFLECT( node::app::liquidity_pool_api_obj,
         (id)
         (issuer)
         (symbol_a)
         (symbol_b)
         (symbol_liquid)
         (balance_a)
         (balance_b)
         (balance_liquid)
         (hour_median_price)
         (day_median_price)
         (price_history)
         );

FC_REFLECT( node::app::credit_pool_api_obj,
         (id)
         (issuer)
         (base_symbol)
         (credit_symbol)
         (base_balance)
         (borrowed_balance)
         (credit_balance)
         (last_interest_rate)
         (last_price)
         );

FC_REFLECT( node::app::option_pool_api_obj,
         (id)
         (issuer)
         (base_symbol)
         (quote_symbol)
         (call_symbols)
         (call_strikes)
         (put_symbols)
         (put_strikes)
         );

FC_REFLECT( node::app::prediction_pool_api_obj,
         (id)
         (issuer)
         (prediction_symbol)
         (collateral_symbol)
         (collateral_pool)
         (prediction_bond_pool)
         (outcome_assets)
         (outcome_details)
         (json)
         (url)
         (details)
         (outcome_time)
         (resolution_time)
         );

FC_REFLECT( node::app::prediction_pool_resolution_api_obj,
         (id)
         (account)
         (prediction_symbol)
         (resolution_outcome)
         (amount)
         );

FC_REFLECT( node::app::distribution_api_obj,
         (id)
         (issuer)
         (distribution_asset)
         (fund_asset)
         (details)
         (url)
         (json)
         (distribution_rounds)
         (distribution_interval_days)
         (max_intervals_missed)
         (intervals_paid)
         (intervals_missed)
         (input_fund_unit)
         (output_distribution_unit)
         (min_input_fund_units)
         (max_input_fund_units)
         (min_unit_ratio)
         (max_unit_ratio)
         (min_input_balance_units)
         (max_input_balance_units)
         (total_distributed)
         (total_funded)
         (begin_time)
         (next_round_time)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::distribution_balance_api_obj,
         (id)
         (sender)
         (distribution_asset)
         (amount)
         (created)
         (last_updated)
         );


//================================//
// ===== Credit API Objects ===== //
//================================//


FC_REFLECT( node::app::credit_collateral_api_obj,
         (id)
         (owner)
         (symbol)
         (collateral)  
         );

FC_REFLECT( node::app::credit_loan_api_obj,
         (id)
         (owner)
         (loan_id)
         (debt)
         (interest)
         (collateral)
         (loan_price)
         (liquidation_price)
         (symbol_a)
         (symbol_b)
         (last_interest_rate)
         (created)
         (last_updated)
         );


//========================================//
// ===== Block Producer API Objects ===== //
//========================================//


FC_REFLECT( node::app::producer_api_obj,
         (id)
         (owner)
         (active)
         (schedule)
         (last_confirmed_block_num)
         (details)
         (url)
         (json)
         (latitude)
         (longitude)
         (signing_key)
         (last_commit_height)
         (last_commit_id)
         (total_blocks)
         (voting_power)
         (vote_count)
         (mining_power)
         (mining_count)
         (last_mining_update)
         (last_pow_time)
         (recent_txn_stake_weight)
         (last_txn_stake_weight_update)
         (accumulated_activity_stake)
         (total_missed)
         (last_aslot)
         (props)
         (voting_virtual_last_update)
         (voting_virtual_position)
         (voting_virtual_scheduled_time)
         (mining_virtual_last_update)
         (mining_virtual_position)
         (mining_virtual_scheduled_time)
         (running_version)
         (hardfork_version_vote)
         (hardfork_time_vote)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::block_validation_api_obj,
         (id)
         (producer)
         (block_id)
         (block_height)
         (verify_txn)
         (verifications)
         (verifiers)
         (committed)
         (commit_time)
         (commitment_stake)
         (commit_txn)
         (created)
         );

FC_REFLECT( node::app::commit_violation_api_obj,
         (id)
         (reporter)
         (producer)
         (block_height)
         (first_trx)
         (second_trx)
         (created)
         (forfeited_stake)
         );

FC_REFLECT( node::app::tag_api_obj,
         (id)
         (tag)
         (total_payout)
         (post_count)
         (children)
         (net_votes)
         (view_count)
         (share_count)
         (net_reward)
         (vote_power)
         (view_power)
         (share_power)
         (comment_power)
         );

FC_REFLECT_DERIVED( node::app::signed_block_api_obj, (node::protocol::signed_block),
         (block_id)
         (signing_key)
         (transaction_ids)
         );