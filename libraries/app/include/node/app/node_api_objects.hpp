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

typedef chain::account_recovery_update_request_object  account_recovery_update_request_api_obj;
typedef chain::block_summary_object                    block_summary_api_obj;
typedef chain::comment_metrics_object                  comment_metrics_api_obj;
typedef chain::unstake_asset_route_object              unstake_asset_route_api_obj;
typedef chain::community_moderator_vote_object         community_moderator_api_obj;
typedef chain::account_decline_voting_request_object   account_decline_voting_request_api_obj;
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
      reward_curve( o.reward_curve ),
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
   comment_reward_curve                  reward_curve;                         ///< The components of the comment reward distribution curve.
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
   reward_fund_api_obj( const chain::asset_reward_fund_object& o ) :
      id( o.id ),
      symbol( o.symbol ),
      total_pending_reward_balance( o.total_pending_reward_balance() ),
      content_reward_balance( o.content_reward_balance ),
      validation_reward_balance( o.validation_reward_balance ),
      txn_stake_reward_balance( o.txn_stake_reward_balance ),
      work_reward_balance( o.work_reward_balance ),
      producer_activity_reward_balance( o.producer_activity_reward_balance ),
      supernode_reward_balance( o.supernode_reward_balance ),
      power_reward_balance( o.power_reward_balance ),
      enterprise_fund_balance( o.enterprise_fund_balance ),
      development_reward_balance( o.development_reward_balance ),
      marketing_reward_balance( o.marketing_reward_balance ),
      advocacy_reward_balance( o.advocacy_reward_balance ),
      activity_reward_balance( o.activity_reward_balance ),
      premium_partners_fund_balance( o.premium_partners_fund_balance ),
      recent_content_claims( o.recent_content_claims ),
      recent_activity_claims( o.recent_activity_claims ),
      last_updated( o.last_updated ){}

   reward_fund_api_obj(){}

   asset_reward_fund_id_type     id;
   asset_symbol_type       symbol;                                                    ///< Currency symbol of the asset that the reward fund issues.
   asset                   total_pending_reward_balance;                              ///< Total of all reward balances. 
   asset                   content_reward_balance;                                    ///< Balance awaiting distribution to content creators.
   asset                   validation_reward_balance;                                 ///< Balance distributed to block validating producers. 
   asset                   txn_stake_reward_balance;                                  ///< Balance distributed to block producers based on the stake weighted transactions in each block.
   asset                   work_reward_balance;                                       ///< Balance distributed to each proof of work block producer. 
   asset                   producer_activity_reward_balance;                          ///< Balance distributed to producers that receive activity reward votes.
   asset                   supernode_reward_balance;                                  ///< Balance distributed to supernodes, based on stake weighted comment views.
   asset                   power_reward_balance;                                      ///< Balance distributed to staked units of the currency.
   asset                   enterprise_fund_balance;                                    ///< Balance distributed to community proposal funds on the currency. 
   asset                   development_reward_balance;                                ///< Balance distributed to elected developers. 
   asset                   marketing_reward_balance;                                  ///< Balance distributed to elected marketers. 
   asset                   advocacy_reward_balance;                                   ///< Balance distributed to elected advocates. 
   asset                   activity_reward_balance;                                   ///< Balance distributed to content creators that are active each day. 
   asset                   premium_partners_fund_balance;                             ///< Receives income from memberships, distributed to premium creators. 
   uint128_t               recent_content_claims;                                     ///< Recently claimed content reward balance shares.
   uint128_t               recent_activity_claims;                                    ///< Recently claimed activity reward balance shares.
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
      profile_image( to_string( a.profile_image ) ),
      cover_image( to_string( a.cover_image ) ),
      json( to_string( a.json ) ),
      json_private( to_string( a.json_private ) ),
      first_name( to_string( a.first_name ) ),
      last_name( to_string( a.last_name ) ),
      gender( to_string( a.json_private ) ),
      date_of_birth( to_string( a.date_of_birth ) ),
      email( to_string( a.email ) ),
      phone( to_string( a.phone ) ),
      nationality( to_string( a.nationality ) ),
      relationship( to_string( a.relationship ) ),
      political_alignment( to_string( a.political_alignment ) ),
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
      reset_delay_days( a.reset_delay_days ),
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
      enterprise_vote_count( a.enterprise_vote_count ),
      recurring_membership( a.recurring_membership ),
      created( a.created ),
      membership_expiration( a.membership_expiration ),
      last_updated( a.last_updated ),
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
         for( auto t : a.interests )
         {
            interests.push_back( t );
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
   string                           profile_image;                         ///< IPFS Reference of the Public profile image of the account.
   string                           cover_image;                           ///< IPFS Reference of the Public cover image of the account.
   string                           json;                                  ///< The JSON string of additional Public profile information.
   string                           json_private;                          ///< The JSON string of additional encrypted profile information. Encrypted with connection key.
   string                           first_name;                            ///< Encrypted First name of the user. Encrypted with connection key.
   string                           last_name;                             ///< Encrypted Last name of the user. Encrypted with connection key.
   string                           gender;                                ///< Encrypted Gender of the user. Encrypted with connection key.
   string                           date_of_birth;                         ///< Encrypted Date of birth of the user. Format: DD-MM-YYYY. Encrypted with connection key.
   string                           email;                                 ///< Encrypted Email address of the user. Encrypted with connection key.
   string                           phone;                                 ///< Encrypted Phone Number of the user. Encrypted with connection key.
   string                           nationality;                           ///< Encrypted Country of user's residence. Encrypted with connection key.
   string                           relationship;                          ///< Encrypted Relationship status of the account. Encrypted with connection key.
   string                           political_alignment;                   ///< Encrypted Political alignment. Encrypted with connection key.
   string                           pinned_permlink;                       ///< Post permlink pinned to the top of the account's profile.
   vector< tag_name_type >          interests;                             ///< Set of tags of the interests of the user.
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
   uint16_t                         reset_delay_days;
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
   uint16_t                         enterprise_vote_count;                 ///< Number of Enterprise proposals that the account votes for. 
   uint16_t                         recurring_membership;                  ///< Amount of months membership should be automatically renewed for on expiration
   time_point                       created;                               ///< Time that the account was created.
   time_point                       membership_expiration;                 ///< Time that the account has its current membership subscription until.
   time_point                       last_updated;                          ///< Time that the account's details were last updated.
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
      profile_image( to_string( a.profile_image ) ),
      cover_image( to_string( a.cover_image ) ),
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
   string                           profile_image;                         ///< Account's public profile image IPFS reference.
   string                           cover_image;                           ///< Account's public cover image IPFS reference.
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
      officer_vote_threshold( a.officer_vote_threshold.value ),
      active( a.active ),
      created( a.created ),
      last_updated( a.last_updated )
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
   bool                                                            active;                     ///< True when the business account is active, false to deactivate business account.
   time_point                                                      created;                    ///< Time that business account object was created.
   time_point                                                      last_updated;               ///< Time that the business account object was last updated.
};


struct account_executive_vote_api_obj
{
   account_executive_vote_api_obj( const chain::account_executive_vote_object& a ):
      id( a.id ),
      account( a.account ),
      business_account( a.business_account ),
      executive_account( a.executive_account ),
      role( executive_role_values[ int( a.role ) ] ),
      vote_rank( a.vote_rank ),
      last_updated( a.last_updated ),
      created( a.created ){}
      
   account_executive_vote_api_obj(){}

   account_executive_vote_id_type        id;
   account_name_type                     account;               ///< Username of the account, voting for the executive.
   account_name_type                     business_account;      ///< Name of the referred business account.
   account_name_type                     executive_account;     ///< Name of the executive account.
   string                                role;                  ///< Role voted in favor of.
   uint16_t                              vote_rank;             ///< The rank of the executive vote.
   time_point                            last_updated;          ///< Time that the vote was last updated.
   time_point                            created;               ///< Time that the vote was created.
};


struct account_officer_vote_api_obj
{
   account_officer_vote_api_obj( const chain::account_officer_vote_object& a ):
      id( a.id ),
      account( a.account ),
      business_account( a.business_account ),
      officer_account( a.officer_account ),
      vote_rank( a.vote_rank ),
      last_updated( a.last_updated ),
      created( a.created ){}
      
   account_officer_vote_api_obj(){}

   account_officer_vote_id_type          id;
   account_name_type                     account;               ///< Username of the account, voting for the officer.
   account_name_type                     business_account;      ///< Name of the referred business account.
   account_name_type                     officer_account;       ///< Name of the officer account.
   uint16_t                              vote_rank;             ///< The rank of the officer vote.
   time_point                            last_updated;          ///< Time that the vote was last updated.
   time_point                            created;               ///< Time that the vote was created.
};


struct account_permission_api_obj
{
   account_permission_api_obj( const chain::account_permission_object& a ):
      id( a.id ),
      account( a.account )
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

         whitelisted_communities.reserve( a.whitelisted_communities.size() );
         for( auto name : a.whitelisted_communities )
         {
            whitelisted_communities.push_back( name );
         }
         blacklisted_communities.reserve( a.blacklisted_communities.size() );
         for( auto name : a.blacklisted_communities )
         {
            blacklisted_communities.push_back( name );
         }
      }

   account_permission_api_obj(){}

   account_permission_id_type               id;
   account_name_type                        account;                       ///< Name of the account with permissions set.
   vector< account_name_type >              whitelisted_accounts;          ///< List of accounts that are able to send transfers to this account.
   vector< account_name_type >              blacklisted_accounts;          ///< List of accounts that are not able to receive transfers from this account.
   vector< asset_symbol_type >              whitelisted_assets;            ///< List of assets that the account has whitelisted to receieve transfers of. 
   vector< asset_symbol_type >              blacklisted_assets;            ///< List of assets that the account has blacklisted against incoming transfers.
   vector< community_name_type >            whitelisted_communities;       ///< List of communities that the account has whitelisted to interact with. 
   vector< community_name_type >            blacklisted_communities;       ///< List of communities that the account has blacklisted against interactions.
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
      total_balance( b.get_total_balance().amount.value ),
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
         for( auto community : a.member_communities )
         {
            member_communities.push_back( community );
         }
         for( auto community : a.moderator_communities )
         {
            moderator_communities.push_back( community );
         }
         for( auto community : a.admin_communities )
         {
            admin_communities.push_back( community );
         }
         for( auto community : a.founder_communities )
         {
            founder_communities.push_back( community );
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
   account_name_type               account;                 ///< Name of the account.
   vector< account_name_type >     followers;               ///< Accounts that follow this account.
   vector< account_name_type >     following;               ///< Accounts that this account follows.
   vector< account_name_type >     mutual_followers;        ///< Accounts that are both following and followers of this account.
   vector< account_name_type >     connections;             ///< Accounts that are connections of this account.
   vector< account_name_type >     friends;                 ///< Accounts that are friends of this account.
   vector< account_name_type >     companions;              ///< Accounts that are companions of this account.
   vector< community_name_type >   followed_communities;    ///< Communities that the account subscribes to.
   vector< community_name_type >   member_communities;      ///< Communities that the account is a member within.
   vector< community_name_type >   moderator_communities;   ///< Communities that the account is a moderator within.
   vector< community_name_type >   admin_communities;       ///< Communities that the account is an admin within.
   vector< community_name_type >   founder_communities;     ///< Communities that the account is a founder of.
   vector< tag_name_type >         followed_tags;           ///< Tags that the account follows.
   vector< account_name_type >     filtered;                ///< Accounts that this account has filtered. Interfaces should not show posts by these users.
   vector< community_name_type >   filtered_communities;    ///< Communities that this account has filtered. Posts will not display if they are in these communities.
   vector< tag_name_type >         filtered_tags;           ///< Tags that this account has filtered. Posts will not display if they have any of these tags. 
   time_point                      last_updated;            ///< Last time that the account changed its following sets.
};



struct account_tag_following_api_obj
{
   account_tag_following_api_obj( const chain::account_tag_following_object& t ):
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

   account_tag_following_api_obj(){}

   account_tag_following_id_type             id;
   tag_name_type                     tag;                  ///< Name of the account.
   vector< account_name_type >       followers;            ///< Accounts that follow this account. 
   time_point                        last_updated;          ///< Last time that the tag changed its following sets.
};


struct account_connection_api_obj
{
   account_connection_api_obj( const chain::account_connection_object& c ):
      id( c.id ),
      account_a( c.account_a ),
      encrypted_key_a( c.encrypted_key_a ),
      message_a( to_string( c.message_a ) ),
      json_a( to_string( c.json_a ) ),
      account_b( c.account_b ),
      encrypted_key_b( c.encrypted_key_b ),
      message_b( to_string( c.message_b ) ),
      json_b( to_string( c.json_b ) ),
      connection_type( connection_tier_values[ int( c.connection_type ) ] ),
      connection_id( to_string( c.connection_id ) ),
      message_count( c.message_count ),
      consecutive_days( c.consecutive_days ),
      last_message_time_a( c.last_message_time_a ),
      last_message_time_b( c.last_message_time_b ),
      last_updated( c.last_updated ),
      created( c.created ){}

   account_connection_api_obj(){}

   account_connection_id_type   id;
   account_name_type            account_a;                ///< Account with the lower ID.
   encrypted_keypair_type       encrypted_key_a;          ///< A's private connection key, encrypted with the public secure key of account B.
   string                       message_a;                ///< A's Connection encrypted accompanying message for reference.
   string                       json_a;                   ///< A's Encrypted JSON metadata.
   account_name_type            account_b;                ///< Account with the greater ID.
   encrypted_keypair_type       encrypted_key_b;          ///< B's private connection key, encrypted with the public secure key of account A.
   string                       message_b;                ///< B's Connection encrypted accompanying message for reference.
   string                       json_b;                   ///< B's Encrypted JSON metadata.
   string                       connection_type;          ///< The connection level shared in this object
   string                       connection_id;            ///< Unique uuidv4 for the connection, for local storage of decryption key.
   uint32_t                     message_count;            ///< Number of total messages sent between connections
   uint32_t                     consecutive_days;         ///< Number of consecutive days that the connected accounts have both sent a message.
   time_point                   last_message_time_a;      ///< Time since the account A last sent a message
   time_point                   last_message_time_b;      ///< Time since the account B last sent a message
   time_point                   last_updated;             ///< Time that the connection was lat updated.
   time_point                   created;                  ///< Time the connection was created. 
};


struct owner_authority_history_api_obj
{
   owner_authority_history_api_obj( const chain::account_authority_history_object& o ) :
      id( o.id ),
      account( o.account ),
      previous_owner_authority( authority( o.previous_owner_authority ) ),
      last_valid_time( o.last_valid_time ){}
   
   owner_authority_history_api_obj(){}

   account_authority_history_id_type      id;
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
      officer_type( network_officer_role_values[ int( o.officer_type ) ] ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      reward_currency( o.reward_currency ),
      vote_count( o.vote_count ),
      voting_power( o.voting_power.value ),
      producer_vote_count( o.producer_vote_count ),
      producer_voting_power( o.producer_voting_power.value ),
      last_updated( o.last_updated ),
      created( o.created ),
      active( o.active ),
      officer_approved( o.officer_approved ){}
      
   network_officer_api_obj(){}

   network_officer_id_type        id;
   account_name_type              account;                  ///< The name of the account that owns the network officer.
   string                         officer_type;             ///< The type of network officer that the account serves as.
   string                         details;                  ///< The officer's details description.
   string                         url;                      ///< The officer's reference URL.
   string                         json;                     ///< JSON metadata of the officer.
   asset_symbol_type              reward_currency;          ///< Symbol of the currency asset that the network officer requests.
   uint32_t                       vote_count;               ///< The number of accounts that support the officer.
   int64_t                        voting_power;             ///< The amount of voting power that votes for the officer.
   uint32_t                       producer_vote_count;      ///< The number of accounts that support the officer.
   int64_t                        producer_voting_power;    ///< The amount of voting power that votes for the officer.
   time_point                     last_updated;             ///< The time the officer was last updated.
   time_point                     created;                  ///< The time the officer was created.
   bool                           active;                   ///< True if the officer is active, set false to deactivate.
   bool                           officer_approved;         ///< True when the network officer has received sufficient voting approval to earn funds.
};


struct network_officer_vote_api_obj
{
   network_officer_vote_api_obj( const chain::network_officer_vote_object& o ):
      id( o.id ),
      account( o.account ),
      network_officer( o.network_officer ),
      officer_type( network_officer_role_values[ int( o.officer_type ) ] ),
      vote_rank( o.vote_rank ),
      last_updated( o.last_updated ),
      created( o.created ){}

   network_officer_vote_api_obj(){}

   network_officer_vote_id_type        id;
   account_name_type                   account;                    ///< The name of the account voting for the officer.
   account_name_type                   network_officer;            ///< The name of the network officer being voted for.
   string                              officer_type;               ///< the type of network officer that is being voted for.
   uint16_t                            vote_rank;                  ///< the ranking of the vote for the officer.
   time_point                          last_updated;               ///< Time that the vote was last updated.
   time_point                          created;                    ///< Time that the vote was created.
};


struct executive_board_api_obj
{
   executive_board_api_obj( const chain::executive_board_object& o ):
      id( o.id ),
      account( o.account ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      budget( o.budget ),
      vote_count( o.vote_count ),
      voting_power( o.voting_power.value ),
      producer_vote_count( o.producer_vote_count ),
      producer_voting_power( o.producer_voting_power.value ),
      last_updated( o.last_updated ),
      created( o.created ),
      active( o.active ),
      board_approved( o.board_approved ){}

   executive_board_api_obj(){}

   executive_board_id_type        id;
   account_name_type              account;                    ///< The name of the governance account that created the executive team.
   string                         details;                    ///< The executive team's details description.
   string                         url;                        ///< The executive team's reference URL.
   string                         json;                       ///< JSON metadata of the executive team.
   asset                          budget;                     ///< Total amount of Credit asset requested for team compensation and funding.
   uint32_t                       vote_count;                 ///< The number of accounts that support the executive team.
   int64_t                        voting_power;               ///< The amount of voting power that votes for the executive team.
   uint32_t                       producer_vote_count;        ///< The number of accounts that support the executive team.
   int64_t                        producer_voting_power;      ///< The amount of voting power that votes for the executive team.
   time_point                     last_updated;               ///< Time that the vote was last updated.
   time_point                     created;                    ///< Time that the vote was created.
   bool                           active;                     ///< True if the executive team is active, set false to deactivate.
   bool                           board_approved;             ///< True when the board has reach sufficient voting support to receive budget.
};


struct executive_board_vote_api_obj
{
   executive_board_vote_api_obj( const chain::executive_board_vote_object& o ):
      id( o.id ),
      account( o.account ),
      executive_board( o.executive_board ),
      vote_rank( o.vote_rank ),
      last_updated( o.last_updated ),
      created( o.created ){}

   executive_board_vote_api_obj(){}

   executive_board_vote_id_type        id;
   account_name_type                   account;               ///< The name of the account that voting for the executive board.
   account_name_type                   executive_board;       ///< The name of the executive board being voted for.
   uint16_t                            vote_rank;             ///< The rank the rank of the vote for the executive board.
   time_point                          last_updated;          ///< Time that the vote was last updated.
   time_point                          created;               ///< Time that the vote was created.
};


struct governance_account_api_obj
{
   governance_account_api_obj( const chain::governance_account_object& o ):
      id( o.id ),
      account( o.account ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      subscriber_count( o.subscriber_count ),
      subscriber_power( o.subscriber_power.value ),
      producer_subscriber_count( o.producer_subscriber_count ),
      producer_subscriber_power( o.producer_subscriber_power.value ),
      last_updated( o.last_updated ),
      created( o.created ),
      active( o.active ),
      account_approved( o.account_approved ){}

   governance_account_api_obj(){}

   governance_account_id_type     id;
   account_name_type              account;                     ///< The name of the governance account that created the governance account.
   string                         details;                     ///< The governance account's details description. 
   string                         url;                         ///< The governance account's reference URL. 
   string                         json;                        ///< Json metadata of the governance account. 
   uint32_t                       subscriber_count;            ///< The number of accounts that support the governance account.
   int64_t                        subscriber_power;            ///< The amount of voting power that votes for the governance account. 
   uint32_t                       producer_subscriber_count;   ///< The number of accounts that support the governance account.
   int64_t                        producer_subscriber_power;   ///< The amount of voting power that votes for the governance account.
   time_point                     last_updated;                ///< The time the governance account was last updated.
   time_point                     created;                     ///< The time the governance account was created.
   bool                           active;                      ///< True if the governance account is active, set false to deactivate.
   bool                           account_approved;            ///< True when the governance account has reach sufficient voting support to receive budget.
};


struct governance_subscription_api_obj
{
   governance_subscription_api_obj( const chain::governance_subscription_object& o ):
      id( o.id ),
      account( o.account ),
      governance_account( o.governance_account ),
      vote_rank( o.vote_rank ),
      last_updated( o.last_updated ),
      created( o.created ){}

   governance_subscription_api_obj(){}

   governance_subscription_id_type     id;
   account_name_type                   account;                    ///< The name of the account that subscribes to the governance account.
   account_name_type                   governance_account;         ///< The name of the governance account being subscribed to.
   uint16_t                            vote_rank;                  ///< The preference rank of subscription for fee splitting.
   time_point                          last_updated;               ///< Time that the subscription was last updated.
   time_point                          created;                    ///< Time that the subscription was created.
};


struct supernode_api_obj
{
   supernode_api_obj( const chain::supernode_object& o ):
      id( o.id ),
      account( o.account ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      node_api_endpoint( to_string( o.node_api_endpoint ) ),
      notification_api_endpoint( to_string( o.notification_api_endpoint ) ),
      auth_api_endpoint( to_string( o.auth_api_endpoint ) ),
      ipfs_endpoint( to_string( o.ipfs_endpoint ) ),
      bittorrent_endpoint( to_string( o.bittorrent_endpoint ) ),
      json( to_string( o.json ) ),
      storage_rewards( o.storage_rewards ),
      daily_active_users( o.daily_active_users / PERCENT_100 ),
      monthly_active_users( o.monthly_active_users / PERCENT_100 ),
      recent_view_weight( o.recent_view_weight.value ),
      last_activation_time( o.last_activation_time ),
      last_updated( o.last_updated ),
      created( o.created ),
      active( o.active ){}
   
   supernode_api_obj(){}

   supernode_id_type       id;
   account_name_type       account;                     ///< The name of the account that owns the supernode.
   string                  details;                     ///< The supernode's details description. 
   string                  url;                         ///< The supernode's reference URL. 
   string                  node_api_endpoint;           ///< The Full Archive node public API endpoint of the supernode.
   string                  notification_api_endpoint;   ///< The Notification API endpoint of the Supernode. 
   string                  auth_api_endpoint;           ///< The Transaction signing authentication API endpoint of the supernode.
   string                  ipfs_endpoint;               ///< The IPFS file storage API endpoint of the supernode.
   string                  bittorrent_endpoint;         ///< The Bittorrent Seed Box endpoint URL of the Supernode. 
   string                  json;                        ///< Json metadata of the supernode, including additional outside of consensus APIs and services. 
   asset                   storage_rewards;             ///< Amount of core asset earned from storage.
   uint64_t                daily_active_users;          ///< The average number of accounts (X percent 100) that have used files from the node in the prior 24h.
   uint64_t                monthly_active_users;        ///< The average number of accounts (X percent 100) that have used files from the node in the prior 30 days.
   int64_t                 recent_view_weight;          ///< The rolling 7 day average of daily accumulated voting power of viewers. 
   time_point              last_activation_time;        ///< The time the Supernode was last reactivated, must be at least 24h ago to claim rewards.
   time_point              last_updated;                ///< The time the file weight and active users was last decayed.
   time_point              created;                     ///< The time the supernode was created.
   bool                    active;                      ///< True if the supernode is active, set false to deactivate.
};


struct interface_api_obj
{
   interface_api_obj( const chain::interface_object& o ):
      id( o.id ),
      account( o.account ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      daily_active_users( o.daily_active_users / PERCENT_100 ),
      monthly_active_users( o.monthly_active_users / PERCENT_100 ),
      last_updated( o.last_updated ),
      created( o.created ),
      active( o.active ){}
   
   interface_api_obj(){}

   interface_id_type       id;
   account_name_type       account;                     ///< The name of the account that owns the interface.
   string                  details;                     ///< The interface's details description. 
   string                  url;                         ///< The interface's reference URL. 
   string                  json;                        ///< Json metadata of the interface, including additional outside of consensus APIs and services. 
   uint64_t                daily_active_users;          ///< The average number of accounts (X percent 100) that have used files from the node in the prior 24h.
   uint64_t                monthly_active_users;        ///< The average number of accounts (X percent 100) that have used files from the node in the prior 30 days.
   time_point              last_updated;                ///< The time the file weight and active users was last decayed.
   time_point              created;                     ///< The time the interface was created.
   bool                    active;                      ///< True if the interface is active, set false to deactivate.
};


struct mediator_api_obj
{
   mediator_api_obj( const chain::mediator_object& o ):
      id( o.id ),
      account( o.account ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      mediator_bond( o.mediator_bond ),
      mediation_virtual_position( o.mediation_virtual_position ),
      last_updated( o.last_updated ),
      created( o.created ),
      active( o.active ){}
   
   mediator_api_obj(){}

   mediator_id_type        id;
   account_name_type       account;                     ///< The name of the account that owns the mediator.
   string                  details;                     ///< The mediator's details description.
   string                  url;                         ///< The mediator's reference URL.
   string                  json;                        ///< Json metadata of the mediator, including additional outside of consensus APIs and services.
   asset                   mediator_bond;               ///< Core Asset staked in mediation bond for selection.
   uint128_t               mediation_virtual_position;  ///< Quantitative ranking of selection for mediation.
   time_point              last_updated;                ///< The time the mediator was last updated.
   time_point              created;                     ///< The time the mediator was created.
   bool                    active;                      ///< True if the mediator is active, set false to deactivate.
};


struct enterprise_api_obj
{
   enterprise_api_obj( const chain::enterprise_object& o ):
      id( o.id ),
      account( o.account ),
      enterprise_id( to_string( o.enterprise_id ) ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      budget( o.budget ),
      distributed( o.distributed ),
      vote_count( o.vote_count ),
      voting_power( o.voting_power.value ),
      producer_vote_count( o.producer_vote_count ),
      producer_voting_power( o.producer_voting_power.value ),
      funder_count( o.funder_count ),
      total_funding( o.total_funding ),
      net_sqrt_voting_power( o.net_sqrt_voting_power ),
      net_sqrt_funding( o.net_sqrt_funding ),
      last_updated( o.last_updated ),
      created( o.created ),
      active( o.active ),
      approved( o.approved ){}

   enterprise_api_obj(){}

   enterprise_id_type                 id;
   account_name_type                  account;                       ///< The name of the governance account that created the enterprise proposal.
   string                             enterprise_id;                 ///< UUIDv4 for referring to the proposal.
   string                             details;                       ///< The proposals's details description.
   string                             url;                           ///< The proposals's reference URL.
   string                             json;                          ///< JSON metadata of the proposal.
   asset                              budget;                        ///< Amount of Currency Asset requested for project funding.
   asset                              distributed;                   ///< Total amount of funds distributed to the proposal's creator.
   uint32_t                           vote_count;                    ///< The number of accounts that support the Enterprise proposal.
   int64_t                            voting_power;                  ///< The oveall amount of voting power that supports the enterprise proposal.
   uint32_t                           producer_vote_count;           ///< The overall number of top 50 producers that support the enterprise proposal.
   int64_t                            producer_voting_power;         ///< The overall amount of producer voting power that supports the enterprise proposal.
   uint32_t                           funder_count;                  ///< The number of accounts that have sent direct funding to the enterprise proposal.
   asset                              total_funding;                 ///< The overall amount of producer voting power that supports the enterprise proposal.
   uint128_t                          net_sqrt_voting_power;         ///< Sum of all of the square roots of the voting power of each vote.
   uint128_t                          net_sqrt_funding;              ///< Sum of all of the square roots of the total funding amount of each vote.
   time_point                         last_updated;                  ///< Time the Enterprise was last updated.
   time_point                         created;                       ///< Time the Enterprise was created.
   bool                               active;                        ///< True if the Enterprise is active, set false to deactivate.
   bool                               approved;                      ///< True when the Enterprise proposal has reached approval status.
};


struct enterprise_vote_api_obj
{
   enterprise_vote_api_obj( const chain::enterprise_vote_object& o ):
      id( o.id ),
      voter( o.voter ),
      account( o.account ),
      enterprise_id( to_string( o.enterprise_id ) ),
      vote_rank( o.vote_rank ),
      last_updated( o.last_updated ),
      created( o.created ){}

   enterprise_vote_api_obj(){}

   enterprise_vote_id_type            id;
   account_name_type                  voter;                   ///< Account voting for the enterprise proposal.
   account_name_type                  account;                 ///< The name of the account that created the community enterprise proposal.
   string                             enterprise_id;           ///< UUIDv4 referring to the proposal being claimed.
   uint16_t                           vote_rank;               ///< The vote rank of the approval for enterprise.
   time_point                         last_updated;            ///< Time the Enterprise vote was last updated.
   time_point                         created;                 ///< Time the Enterprise vote was created.
};


struct enterprise_fund_api_obj
{
   enterprise_fund_api_obj( const chain::enterprise_fund_object& o ):
      id( o.id ),
      funder( o.funder ),
      account( o.account ),
      enterprise_id( to_string( o.enterprise_id ) ),
      amount( o.amount ){}

   enterprise_fund_api_obj(){}

   enterprise_fund_id_type            id;
   account_name_type                  funder;                  ///< Account funding the enterprise proposal.
   account_name_type                  account;                 ///< The name of the account that created the community enterprise proposal.
   string                             enterprise_id;           ///< UUIDv4 referring to the proposal being claimed.
   asset                              amount;                  ///< The vote rank of the approval for enterprise.
   time_point                         last_updated;            ///< Time the Enterprise funding amount was last updated.
   time_point                         created;                 ///< Time the Enterprise funding contribution was created.
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
      parent_author( o.parent_author ),
      parent_permlink( to_string( o.parent_permlink ) ),
      title( to_string( o.title ) ),
      body( to_string( o.body ) ),
      body_private( to_string( o.body_private ) ),
      url( to_string( o.url ) ),
      url_private( to_string( o.url_private ) ),
      ipfs( to_string( o.ipfs ) ),
      ipfs_private( to_string( o.ipfs_private ) ),
      magnet( to_string( o.magnet ) ),
      magnet_private( to_string( o.magnet_private ) ),
      json( to_string( o.json ) ),
      json_private( to_string( o.json_private ) ),
      language( to_string( o.language ) ),
      public_key( o.public_key ),
      community( o.community ),
      interface( o.interface ),
      latitude( o.latitude ),
      longitude( o.longitude ),
      comment_price( o.comment_price ),
      premium_price( o.premium_price ),
      rating( o.rating ),
      root_comment( o.root_comment ),
      post_type( post_format_values[ int( o.post_type ) ]),
      reach( feed_reach_values[ int( o.reach ) ] ),
      reply_connection( connection_tier_values[ int( o.reply_connection ) ] ),
      category( to_string( o.category ) ),
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
      reward_currency( o.reward_currency ),
      reward_curve( o.reward_curve ),
      allow_replies( o.allow_replies ),
      allow_votes( o.allow_votes ),
      allow_views( o.allow_views ),
      allow_shares( o.allow_shares ),
      allow_curation_rewards( o.allow_curation_rewards ),
      root( o.root ),
      encrypted( o.is_encrypted() ),
      deleted( o.deleted )
      {
         for( auto tag : o.tags )
         {
            tags.push_back( tag );
         }
         for( auto name : o.collaborating_authors )
         {
            collaborating_authors.push_back( name );
         }
         for( auto name : o.supernodes )
         {
            supernodes.push_back( name );
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

   comment_id_type                   id;
   account_name_type                 author;                              ///< Name of the account that created the post.
   string                            permlink;                            ///< Unique identifing string for the post.
   account_name_type                 parent_author;                       ///< Account that created the post this post is replying to, empty if root post.
   string                            parent_permlink;                     ///< permlink of the post this post is replying to, empty if root post.
   string                            title;                               ///< String containing title text.
   string                            body;                                ///< Public text for display when the post is opened.
   string                            body_private;                        ///< Encrypted and private text for display when the post is opened and decrypted.
   string                            url;                                 ///< Plaintext URL for the post to link to. For a livestream post, should link to the streaming server embed.
   string                            url_private;                         ///< Encrypted and private Ciphertext URL for the post to link to. For a livestream post, should link to the streaming server embed.
   string                            ipfs;                                ///< Public IPFS file hash: images, videos, files.
   string                            ipfs_private;                        ///< Private and encrypted IPFS file hash: images, videos, files.
   string                            magnet;                              ///< Bittorrent magnet links to torrent file swarms: videos, files.
   string                            magnet_private;                      ///< Bittorrent magnet links to Private and Encrypted torrent file swarms: videos, files.
   string                            json;                                ///< JSON string of additional interface specific data relating to the post.
   string                            json_private;                        ///< Private and Encrypted JSON string of additional interface specific data relating to the post.
   string                            language;                            ///< String containing a two letter language code that the post is broadcast in.
   public_key_type                   public_key;                          ///< The public key used to encrypt the post, holders of the private key may decrypt. 
   community_name_type               community;                           ///< The name of the community to which the post is uploaded to. Null string if no community.
   vector< tag_name_type >           tags;                                ///< Set of string tags for sorting the post by.
   vector< account_name_type >       collaborating_authors;               ///< Accounts that are able to edit the post as shared authors.
   vector< account_name_type >       supernodes;                          ///< Name of the Supernodes that the IPFS file is hosted with. Each should additionally hold the private key.
   account_name_type                 interface;                           ///< Name of the interface account that was used to broadcast the transaction and view the post.
   double                            latitude;                            ///< Latitude co-ordinates of the comment.
   double                            longitude;                           ///< Longitude co-ordinates of the comment.
   asset                             comment_price;                       ///< The price paid to create a comment.
   asset                             premium_price;                       ///< The price paid to unlock the post's premium encryption.
   uint16_t                          rating;                              ///< User nominated rating [1-10] as to the maturity of the content, and display sensitivity.
   comment_id_type                   root_comment;                        ///< The root post that the comment is an ancestor of.
   string                            post_type;                           ///< The type of post that is being created, image, text, article, video etc.
   string                            reach;                               ///< The reach of the post across followers, connections, friends and companions.
   string                            reply_connection;                    ///< Replies to the comment must be connected to the author to at least this level.
   string                            category;                            ///< Permlink of root post that this comment is applied to.
   vector< pair< account_name_type, asset > >  payments_received;         ///< Map of all transfers received that referenced this comment. 
   vector< beneficiary_route_type >  beneficiaries;                       ///< Vector of beneficiary routes that receive a content reward distribution.
   time_point                        last_updated;                        ///< The time the comment was last edited by the author
   time_point                        created;                             ///< Time that the comment was created.
   time_point                        active;                              ///< The last time this post was replied to.
   time_point                        last_payout;                         ///< The last time that the post received a content reward payout
   int64_t                           author_reputation;                   ///< Used to measure author lifetime rewards, relative to other accounts.
   uint32_t                          depth;                               ///< Used to track max nested depth.
   uint32_t                          children;                            ///< The total number of children, grandchildren, posts with this as root comment.
   int32_t                           net_votes;                           ///< The amount of upvotes, minus downvotes on the post.
   uint32_t                          view_count;                          ///< The amount of views on the post.
   uint32_t                          share_count;                         ///< The amount of shares on the post.
   int64_t                           net_reward;                          ///< Net reward is the sum of all vote, view, share and comment power.
   int64_t                           vote_power;                          ///< Sum of weighted voting power from votes.
   int64_t                           view_power;                          ///< Sum of weighted voting power from viewers.
   int64_t                           share_power;                         ///< Sum of weighted voting power from shares.
   int64_t                           comment_power;                       ///< Sum of weighted voting power from comments.
   time_point                        cashout_time;                        ///< Next scheduled time to receive a content reward cashout.
   uint32_t                          cashouts_received;                   ///< Number of times that the comment has received content rewards.
   uint128_t                         total_vote_weight;                   ///< The total weight of votes, used to calculate pro-rata share of curation payouts.
   uint128_t                         total_view_weight;                   ///< The total weight of views, used to calculate pro-rata share of curation payouts.
   uint128_t                         total_share_weight;                  ///< The total weight of shares, used to calculate pro-rata share of curation payouts.
   uint128_t                         total_comment_weight;                ///< The total weight of comments, used to calculate pro-rata share of curation payouts.
   asset                             total_payout_value;                  ///< The total payout this comment has received over time, measured in USD.
   asset                             curator_payout_value;                ///< The total payout this comment paid to curators, measured in USD.
   asset                             beneficiary_payout_value;            ///< The total payout this comment paid to beneficiaries, measured in USD.
   asset                             content_rewards;                     ///< The Total amount of content rewards that the post has earned, measured in reward currency. 
   uint32_t                          percent_liquid;                      ///< The percentage of content rewards that should be paid in liquid reward balance. 0 to stake all rewards.
   int64_t                           reward;                              ///< The amount of reward_curve this comment is responsible for in its root post.
   uint128_t                         weight;                              ///< Used to define the comment curation reward this comment receives.
   uint128_t                         max_weight;                          ///< Used to define relative contribution of this comment to rewards.
   asset                             max_accepted_payout;                 ///< USD value of the maximum payout this post will receive.
   asset_symbol_type                 reward_currency;                     ///< The currency asset that the post can earn content rewards in.
   comment_reward_curve              reward_curve;                        ///< The components of the reward curve determined at the time of creating the post.
   bool                              allow_replies;                       ///< allows a post to receive replies.
   bool                              allow_votes;                         ///< allows a post to receive votes.
   bool                              allow_views;                         ///< allows a post to receive views.
   bool                              allow_shares;                        ///< allows a post to receive shares.
   bool                              allow_curation_rewards;              ///< Allows a post to distribute curation rewards.
   bool                              root;                                ///< True if post is a root post.
   bool                              encrypted;                           ///< True if the post is encrypted. False if it is plaintext.
   bool                              deleted;                             ///< True if author selects to remove from display in all interfaces, removed from API node distribution, cannot be interacted with.
};



struct comment_feed_api_obj
{
   comment_feed_api_obj( const chain::comment_feed_object& o ):
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

   comment_feed_api_obj(){}

   comment_feed_id_type                                               id;
   account_name_type                                                  account;               ///< Account that should see comment in their feed.
   comment_id_type                                                    comment;               ///< ID of comment being shared.
   string                                                             feed_type;             ///< Type of feed, follow, connection, community, tag etc.
   map< account_name_type, time_point >                               shared_by;             ///< Map of the times that accounts that have shared the comment.
   map< community_name_type, map< account_name_type, time_point > >   communities;           ///< Map of all communities that the comment has been shared with.
   map< tag_name_type, map< account_name_type, time_point > >         tags;                  ///< Map of all tags that the comment has been shared with.
   account_name_type                                                  first_shared_by;       ///< First account that shared the comment with account.
   uint32_t                                                           shares;                ///< Number of accounts that have shared the comment with account.
   time_point                                                         feed_time;             ///< Time that the comment was added or last shared with account.
};



struct comment_blog_api_obj
{
   comment_blog_api_obj( const chain::comment_blog_object& o ):
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
      
   comment_blog_api_obj(){}

   comment_blog_id_type                    id;
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



struct comment_vote_api_obj
{
   comment_vote_api_obj( const chain::comment_vote_object& o ) :
      id( o.id ),
      voter( o.voter ),
      comment( o.comment ),
      interface( o.interface ),
      weight( o.weight ),
      max_weight( o.max_weight ),
      reward( o.reward ),
      vote_percent( o.vote_percent ),
      reaction( to_string( o.reaction ) ),
      json( to_string( o.json ) ),
      last_updated( o.last_updated ),
      created( o.created ),
      num_changes( o.num_changes ){}

   comment_vote_api_obj(){}

   comment_vote_id_type           id;
   account_name_type              voter;              ///< Name of the account that voted for the comment.
   comment_id_type                comment;            ///< ID of the comment.
   account_name_type              interface;          ///< Name of the interface account that was used to broadcast the transaction and view the post.
   uint128_t                      weight;             ///< Used to define the curation reward this vote receives. Decays with time and additional votes.
   uint128_t                      max_weight;         ///< Used to define relative contribution of this comment to rewards.
   share_type                     reward;             ///< The amount of reward_curve this vote is responsible for
   int16_t                        vote_percent;       ///< The percent weight of the vote.
   string                         reaction;           ///< An Emoji selected as a reaction to the post while voting.
   string                         json;               ///< JSON Metadata of the vote.
   time_point                     last_updated;       ///< The time of the last update of the vote.
   time_point                     created;            ///< Time the vote was created.
   int8_t                         num_changes;        ///< Number of times the vote has been adjusted.
};



struct comment_view_api_obj
{
   comment_view_api_obj( const chain::comment_view_object& o ) :
      id( o.id ),
      viewer( o.viewer ),
      comment( o.comment ),
      interface( o.interface ),
      supernode( o.supernode ),
      reward( o.reward ),
      weight( o.weight ),
      max_weight( o.max_weight ),
      json( to_string( o.json ) ),
      created( o.created ){}

   comment_view_api_obj(){}

   comment_view_id_type           id;
   account_name_type              viewer;             ///< Name of the viewing account.
   comment_id_type                comment;            ///< ID of the comment.
   account_name_type              interface;          ///< Name of the interface account that was used to broadcast the transaction and view the post. 
   account_name_type              supernode;          ///< Name of the supernode account that served the IPFS file data in the post.
   share_type                     reward;             ///< The amount of voting power this view contributed.
   uint128_t                      weight;             ///< The curation reward weight of the view. Decays with time and additional views.
   uint128_t                      max_weight;         ///< Used to define relative contribution of this view to rewards.
   string                         json;               ///< JSON Metadata of the view.
   time_point                     created;            ///< Time the view was created.
};



struct comment_share_api_obj
{
   comment_share_api_obj( const chain::comment_share_object& o ) :
      id( o.id ),
      sharer( o.sharer ),
      comment( o.comment ),
      interface( o.interface ),
      reward( o.reward ),
      weight( o.weight ),
      max_weight( o.max_weight ),
      json( to_string( o.json ) ),
      reach( feed_reach_values[ int( o.reach ) ] ),
      created( o.created ){}

   comment_share_api_obj(){}

   comment_share_id_type          id;
   account_name_type              sharer;             ///< Name of the sharing account.
   comment_id_type                comment;            ///< ID of the comment.
   account_name_type              interface;          ///< Name of the interface account that was used to broadcast the transaction and share the post. 
   share_type                     reward;             ///< The amount of voting power this share contributed.
   uint128_t                      weight;             ///< The curation reward weight of the share. Decays with time and additional shares.
   uint128_t                      max_weight;         ///< Used to define relative contribution of this share to rewards.
   string                         json;               ///< JSON Metadata of the share.
   string                         reach;              ///< Level of reach for the Share.
   time_point                     created;            ///< Time the share was created.
};



struct comment_moderation_api_obj
{
   comment_moderation_api_obj( const chain::comment_moderation_object& o ) :
      id( o.id ),
      moderator( o.moderator ),
      comment( o.comment ),
      community( o.community ),
      rating( o.rating ),
      details( to_string( o.details ) ),
      json( to_string( o.json ) ),
      interface( o.interface ),
      filter( o.filter ),
      removal_requested( o.removal_requested ),
      last_updated( o.last_updated ),
      created( o.created )
      {
         for( auto tag : o.tags )
         {
            tags.push_back( tag );
         }
         for( auto& route : o.beneficiaries_requested )
         {
            beneficiaries_requested.push_back( route );
         }
      }

   comment_moderation_api_obj(){}

   comment_moderation_id_type        id;
   account_name_type                 moderator;                   ///< Name of the moderator or goverance account that created the comment tag.
   comment_id_type                   comment;                     ///< ID of the comment.
   community_name_type               community;                   ///< The name of the community to which the post is uploaded to.
   vector< tag_name_type >           tags;                        ///< Set of string tags for sorting the post by.
   uint16_t                          rating;                      ///< Moderator updated rating as to the maturity of the content, and display sensitivity. 
   string                            details;                     ///< Explaination as to what rule the post is in contravention of and why it was tagged.
   string                            json;                        ///< JSON Metadata of the moderation tag.
   account_name_type                 interface;                   ///< Name of the interface application that broadcasted the transaction.
   bool                              filter;                      ///< True if the post should be filtered by the community or governance address subscribers.
   bool                              removal_requested;           ///< True if the moderator formally requests that the post be removed by the author.
   vector< beneficiary_route_type >  beneficiaries_requested;     ///< Vector of beneficiary routes that receive a content reward distribution.
   time_point                        last_updated;                ///< Time the comment tag was last edited by the author.
   time_point                        created;                     ///< Time that the comment tag was created.
};



struct message_api_obj
{
   message_api_obj( const chain::message_object& o ):
      id( o.id ),
      sender( o.sender ),
      recipient( o.recipient ),
      community( o.community ),
      sender_public_key( o.sender_public_key ),
      recipient_public_key( o.recipient_public_key ),
      community_public_key( o.community_public_key ),
      parent_message( o.parent_message ),
      message( to_string( o.message ) ),
      ipfs( to_string( o.ipfs ) ),
      json( to_string( o.json ) ),
      uuid( to_string( o.uuid ) ),
      interface( o.interface ),
      last_updated( o.last_updated ),
      created( o.created ),
      expiration( o.expiration ){}

   message_api_obj(){}

   message_id_type         id;
   account_name_type       sender;                   ///< Name of the message sender.
   account_name_type       recipient;                ///< Name of the intended message recipient.
   community_name_type     community;                ///< Name of the intended community for group message.
   public_key_type         sender_public_key;        ///< Public secure key of the sender.
   public_key_type         recipient_public_key;     ///< Public secure key of the recipient.
   public_key_type         community_public_key;     ///< Public key of the recipient community.
   message_id_type         parent_message;           ///< ID of the message that is being replied to. Self for root message.
   string                  message;                  ///< Encrypted private message ciphertext.
   string                  ipfs;                     ///< Encrypted Private IPFS file hash: images, videos, files.
   string                  json;                     ///< Encrypted Message metadata.
   string                  uuid;                     ///< uuidv4 uniquely identifying the message for local storage.
   account_name_type       interface;                ///< Name of the interface account that was used to broadcast the transaction.
   time_point              last_updated;             ///< Time the message was last changed, used to reload encrypted message storage.
   time_point              created;                  ///< Time the message was sent.
   time_point              expiration;               ///< Time that the message expires and is automatically deleted.
};



struct list_api_obj
{
   list_api_obj( const chain::list_object& o ) :
      id( o.id ),
      creator( o.creator ),
      list_id( to_string( o.list_id ) ),
      name( to_string( o.name ) ),
      details( to_string( o.details ) ),
      json( to_string( o.json ) ),
      interface( o.interface ),
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
   string                         name;                ///< Name of the list.
   string                         details;             ///< Public details description of the list.
   string                         json;                ///< JSON metadata of the list.
   account_name_type              interface;           ///< Name of the interface account that was used to broadcast the transaction.
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
      community( o.community ),
      public_key( o.public_key ),
      interface( o.interface ),
      details( to_string( o.details ) ),
      json( to_string( o.json ) ),
      poll_option_0( to_string( o.poll_option_0 ) ),
      poll_option_1( to_string( o.poll_option_1 ) ),
      poll_option_2( to_string( o.poll_option_2 ) ),
      poll_option_3( to_string( o.poll_option_3 ) ),
      poll_option_4( to_string( o.poll_option_4 ) ),
      poll_option_5( to_string( o.poll_option_5 ) ),
      poll_option_6( to_string( o.poll_option_6 ) ),
      poll_option_7( to_string( o.poll_option_7 ) ),
      poll_option_8( to_string( o.poll_option_8 ) ),
      poll_option_9( to_string( o.poll_option_9 ) ),
      completion_time( o.completion_time ),
      last_updated( o.last_updated ),
      created( o.created )
      {}

   poll_api_obj(){}

   poll_id_type                   id;
   account_name_type              creator;                  ///< Name of the account that created the poll.
   string                         poll_id;                  ///< uuidv4 referring to the poll.
   community_name_type            community;                ///< Community that the poll is shown within. Null for no community.
   public_key_type                public_key;               ///< Public key of the recipient community for encrypting details and options.
   account_name_type              interface;                ///< Account of the interface that broadcasted the transaction.
   string                         details;                  ///< Text describing the question being asked.
   string                         json;                     ///< JSON metadata of the poll.
   string                         poll_option_0;            ///< Poll option zero, vote 0 to select.
   string                         poll_option_1;            ///< Poll option one, vote 1 to select.
   string                         poll_option_2;            ///< Poll option two, vote 2 to select.
   string                         poll_option_3;            ///< Poll option three, vote 3 to select.
   string                         poll_option_4;            ///< Poll option four, vote 4 to select.
   string                         poll_option_5;            ///< Poll option five, vote 5 to select.
   string                         poll_option_6;            ///< Poll option six, vote 6 to select.
   string                         poll_option_7;            ///< Poll option seven, vote 7 to select.
   string                         poll_option_8;            ///< Poll option eight, vote 8 to select.
   string                         poll_option_9;            ///< Poll option nine, vote 9 to select.
   time_point                     completion_time;          ///< Time the poll voting completes.
   time_point                     last_updated;             ///< Time the poll was last edited by the creator.
   time_point                     created;                  ///< Time that the poll was created.
};



struct poll_vote_api_obj
{
   poll_vote_api_obj( const chain::poll_vote_object& o ) :
      id( o.id ),
      voter( o.voter ),
      creator( o.creator ),
      poll_id( to_string( o.poll_id ) ),
      public_key( o.public_key ),
      json( to_string( o.json ) ),
      details( to_string( o.json ) ),
      poll_option( o.poll_option ),
      interface( o.interface ),
      last_updated( o.last_updated ),
      created( o.created ){}

   poll_vote_api_obj(){}

   poll_vote_id_type           id;
   account_name_type           voter;               ///< Name of the account that created the vote.
   account_name_type           creator;             ///< Name of the account that created the poll.
   string                      poll_id;             ///< uuidv4 referring to the poll.
   public_key_type             public_key;          ///< Public key used to encrypt the json and details.
   string                      json;                ///< JSON metadata of the poll vote.
   string                      details;             ///< Text describing the reason for the vote.
   uint16_t                    poll_option;         ///< Poll option chosen [0,9]
   account_name_type           interface;           ///< Account of the interface that broadcasted the transaction.
   time_point                  last_updated;        ///< Time the vote was last edited by the voter.
   time_point                  created;             ///< Time that the vote was created.
};



struct premium_purchase_api_obj
{
   premium_purchase_api_obj( const chain::premium_purchase_object& o ) :
      id( o.id ),
      account( o.account ),
      comment( o.comment ),
      premium_price( o.premium_price ),
      interface( o.interface ),
      expiration( o.expiration ),
      last_updated( o.last_updated ),
      created( o.created ),
      released( o.released ){}

   premium_purchase_api_obj(){}

   premium_purchase_id_type           id;
   account_name_type                  account;             ///< Name of the account that created the vote.
   comment_id_type                    comment;             ///< ID of the premium post being purchased.
   asset                              premium_price;       ///< Amount of asset paid to purchase the post.
   account_name_type                  interface;           ///< Interface account used for the transaction.
   time_point                         expiration;          ///< Time that the purchase must be completed before expiring.
   time_point                         last_updated;        ///< Time the vote was last edited by the voter.
   time_point                         created;             ///< Time that the vote was created.
   bool                               released;            ///< True when the purchase has been matched by at least one premium purchase key.
};



struct premium_purchase_key_api_obj
{
   premium_purchase_key_api_obj( const chain::premium_purchase_key_object& o ) :
      id( o.id ),
      provider( o.provider ),
      account( o.account ),
      comment( o.comment ),
      interface( o.interface ),
      encrypted_key( o.encrypted_key ),
      last_updated( o.last_updated ),
      created( o.created ){}

   premium_purchase_key_api_obj(){}

   premium_purchase_key_id_type       id;
   account_name_type                  provider;            ///< Name of the account releasing the premium content. Post author or designated Supernode. 
   account_name_type                  account;             ///< Name of the account purchasing the premium content.
   comment_id_type                    comment;             ///< ID of the Premium Post being purchased.
   account_name_type                  interface;           ///< Interface account used for the transaction.
   encrypted_keypair_type             encrypted_key;       ///< The private key used to encrypt the premium post, encrypted with the public secure key of the purchaser.
   time_point                         last_updated;        ///< Time the vote was last edited by the voter.
   time_point                         created;             ///< Time that the vote was created.
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
      display_name( to_string( b.display_name ) ),
      details( to_string( b.details ) ),
      url( to_string( b.url ) ),
      profile_image( to_string( b.profile_image ) ),
      cover_image( to_string( b.cover_image ) ),
      json( to_string( b.json ) ),
      json_private( to_string( b.json_private ) ),
      pinned_author( b.pinned_author ),
      pinned_permlink( to_string( b.pinned_permlink ) ),
      community_privacy( community_privacy_values[ int( b.community_privacy ) ] ),
      community_member_key( b.community_member_key ),
      community_moderator_key( b.community_moderator_key ),
      community_admin_key( b.community_admin_key ),
      reward_currency( b.reward_currency ),
      membership_price( b.membership_price ),
      max_rating( b.max_rating ),
      flags( b.flags ),
      permissions( b.permissions ),
      subscriber_count( b.subscriber_count ),
      post_count( b.post_count ),
      comment_count( b.comment_count ),
      vote_count( b.vote_count ),
      view_count( b.view_count ),
      share_count( b.share_count ),
      created( b.created ),
      last_updated( b.last_updated ),
      last_post( b.last_post ),
      last_root_post( b.last_root_post ),
      active( b.active )
      {
         for( auto t : b.tags )
         {
            tags.insert( t );
         }
      }

   community_api_obj(){}

   community_id_type                  id;
   community_name_type                name;                               ///< Name of the community, lowercase letters, numbers and hyphens only.
   account_name_type                  founder;                            ///< The account that created the community, able to add and remove administrators.
   string                             display_name;                       ///< The full name of the community (non-consensus), encrypted with the member key if private community.
   string                             details;                            ///< Describes the community and what it is about and the rules of posting, encrypted with the member key if private community.
   string                             url;                                ///< Community URL link for more details, encrypted with the member key if private community.
   string                             profile_image;                      ///< IPFS Reference of the Icon image of the community, encrypted with the member key if private community.
   string                             cover_image;                        ///< IPFS Reference of the Cover image of the community, encrypted with the member key if private community.
   string                             json;                               ///< Public plaintext JSON information about the community, its topic and rules.
   string                             json_private;                       ///< Private ciphertext JSON information about the community, encrypted with the member key if private community.
   account_name_type                  pinned_author;                      ///< Author of Post pinned to the top of the community's page.
   string                             pinned_permlink;                    ///< Permlink of Post pinned to the top of the community's page, encrypted with the member key if private community.
   set< tag_name_type >               tags;                               ///< Set of tags of the topics within the community to enable discovery.
   string                             community_privacy;                  ///< Community privacy level: Open_Public, General_Public, Exclusive_Public, Closed_Public, Open_Private, General_Private, Exclusive_Private, Closed_Private.
   public_key_type                    community_member_key;               ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.
   public_key_type                    community_moderator_key;            ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.
   public_key_type                    community_admin_key;                ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.
   asset_symbol_type                  reward_currency;                    ///< The Currency asset used for content rewards in the community.
   asset                              membership_price;                   ///< Price paid per day by all community members to community founder.
   uint16_t                           max_rating;                         ///< Highest severity rating that posts in the community can have.
   uint32_t                           flags;                              ///< The currently active flags on the community for content settings.
   uint32_t                           permissions;                        ///< The flag permissions that can be activated on the community for content settings.
   uint32_t                           subscriber_count;                   ///< Number of accounts that are subscribed to the community.
   uint32_t                           post_count;                         ///< Number of posts created in the community.
   uint32_t                           comment_count;                      ///< Number of comments on posts in the community.
   uint32_t                           vote_count;                         ///< Accumulated number of votes received by all posts in the community.
   uint32_t                           view_count;                         ///< Accumulated number of views on posts in the community.
   uint32_t                           share_count;                        ///< Accumulated number of shares on posts in the community.
   time_point                         created;                            ///< Time that the community was created.
   time_point                         last_updated;                       ///< Time that the community's details were last updated.
   time_point                         last_post;                          ///< Time that the user most recently created a comment.
   time_point                         last_root_post;                     ///< Time that the community last created a post.
   bool                               active;                             ///< True if the community is active, false to suspend all interaction.
};



struct community_request_api_obj
{
   community_request_api_obj( const chain::community_join_request_object& o ) :
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
   community_invite_api_obj( const chain::community_join_invite_object& o ) :
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



struct community_federation_api_obj
{
   community_federation_api_obj( const chain::community_federation_object& o ) :
      id( o.id ),
      community_a( o.community_a ),
      encrypted_key_a( o.encrypted_key_a ),
      message_a( to_string( o.message_a ) ),
      json_a( to_string( o.json_a ) ),
      community_b( o.community_b ),
      encrypted_key_b( o.encrypted_key_b ),
      message_b( to_string( o.message_b ) ),
      json_b( to_string( o.json_b ) ),
      federation_type( community_federation_values[ int( o.federation_type ) ] ),
      federation_id( to_string( o.federation_id ) ),
      share_accounts_a( o.share_accounts_a ),
      share_accounts_b( o.share_accounts_b ),
      approved_a( o.approved_a ),
      approved_b( o.approved_b ),
      last_updated( o.last_updated ),
      created( o.created ),
      approved( o.approved() ){}

   community_federation_api_obj(){}

   community_federation_id_type     id;                 
   community_name_type              community_a;                   ///< Community with the lower ID.
   encrypted_keypair_type           encrypted_key_a;               ///< A's private community key, encrypted with the equivalent private community key of Community B.
   string                           message_a;                     ///< Encrypted message from A to the community B membership, encrypted with equivalent community public key.
   string                           json_a;                        ///< Encrypted JSON metadata from A to community B, encrypted with equivalent community public key.
   community_name_type              community_b;                   ///< Community with the higher ID.
   encrypted_keypair_type           encrypted_key_b;               ///< B's private community key, encrypted with the equivalent private community key of Community A.
   string                           message_b;                     ///< Encrypted message from B to the community A membership, encrypted with equivalent community public key.
   string                           json_b;                        ///< Encrypted JSON metadata from B to community A, encrypted with equivalent community public key.
   string                           federation_type;               ///< Determines the level of Federation between the Communities.
   string                           federation_id;                 ///< Reference uuidv4 for the federation, for local storage of decryption keys.
   bool                             share_accounts_a;              ///< True when community A accepts incoming memberships from B.
   bool                             share_accounts_b;              ///< True when community B accepts incoming memberships from A.
   bool                             approved_a;                    ///< True when community A approves the federation.
   bool                             approved_b;                    ///< True when account B approves the federation.
   time_point                       last_updated;                  ///< Time the connection keys were last updated.
   time_point                       created;                       ///< Time the connection was created.
   bool                             approved;                      ///< True when both communities approve the federation.
};



struct community_event_api_obj
{
   community_event_api_obj( const chain::community_event_object& o ) :
      id( o.id ),
      community( o.community ),
      event_id( to_string( o.event_id ) ),
      public_key( o.public_key ),
      event_name( to_string( o.event_name ) ),
      location( to_string( o.location ) ),
      latitude( o.latitude ),
      longitude( o.longitude ),
      details( to_string( o.details ) ),
      url( to_string( o.url ) ),
      json( to_string( o.json ) ),
      interface( o.interface ),
      event_price( o.event_price ),
      interested( o.interested ),
      attending( o.attending ),
      not_attending( o.not_attending ),
      event_start_time( o.event_start_time ),
      event_end_time( o.event_end_time ),
      last_updated( o.last_updated ),
      created( o.created ){}

   community_event_api_obj(){}

   community_event_id_type               id;                 
   community_name_type                   community;              ///< Community being invited to join.
   string                                event_id;               ///< UUIDv4 referring to the event within the Community. Unique on community/event_id.
   public_key_type                       public_key;             ///< Public key for encrypting the event details. Null if public event.
   string                                event_name;             ///< The Name of the event.
   string                                location;               ///< Address location of the event.
   double                                latitude;               ///< Latitude co-ordinates of the event.
   double                                longitude;              ///< Longitude co-ordinates of the event.
   string                                details;                ///< Event details describing the purpose of the event.
   string                                url;                    ///< Reference URL for the event.
   string                                json;                   ///< Additional Event JSON data.
   account_name_type                     interface;              ///< Account of the interface that broadcasted the transaction.
   asset                                 event_price;            ///< Amount paid to join the attending list as a ticket holder to the event.
   uint64_t                              interested;             ///< Number of Accounts that are interested in the event.
   uint64_t                              attending;              ///< Members that have confirmed that they will be attending the event.
   uint64_t                              not_attending;          ///< Members that have confirmed that they will not be attending the event.
   time_point                            event_start_time;       ///< Time that the Event will begin.
   time_point                            event_end_time;         ///< Time that the event will end.
   time_point                            last_updated;           ///< Time that the event was last updated.
   time_point                            created;                ///< Time that the event was created.
};



struct community_event_attend_api_obj
{
   community_event_attend_api_obj( const chain::community_event_attend_object& o ) :
      id( o.id ),
      attendee( o.attendee ),
      community( o.community ),
      event_id( to_string( o.event_id ) ),
      public_key( o.public_key ),
      interface( o.interface ),
      message( to_string( o.message ) ),
      json( to_string( o.json ) ),
      interested( o.interested ),
      attending( o.attending ),
      last_updated( o.last_updated ),
      created( o.created ){}

   community_event_attend_api_obj(){}

   community_event_attend_id_type        id;                 
   account_name_type                     attendee;               ///< Account that is attending the event.
   community_name_type                   community;              ///< Community hosting the event, and all members are being invited to attend.
   string                                event_id;               ///< UUIDv4 referring to the event within the Community. Unique on community/event_id.
   public_key_type                       public_key;             ///< Public key for encrypting the event details. Null if public event.
   account_name_type                     interface;              ///< Account of the interface that broadcasted the transaction.
   string                                message;                ///< Encrypted message to the community operating the event.
   string                                json;                   ///< Additional Event Attendance JSON metadata.
   bool                                  interested;             ///< True when the attendee is interested in the event.
   bool                                  attending;              ///< True when the attendee has confirmed that they will be attending the event, and paid the event price.
   time_point                            last_updated;           ///< Time that the attendance was last updated.
   time_point                            created;                ///< Time that the attendance was created.
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
            attributes.push_back( t );
         }
         for( auto t : o.attribute_values )
         {
            attribute_values.push_back( t );
         }
      }

   graph_node_api_obj(){}

   graph_node_id_type                      id;
   account_name_type                       account;                     ///< Name of the account that created the node.
   vector< graph_node_name_type >          node_types;                  ///< Set of Types of node being created, determines the required attributes.
   string                                  node_id;                     ///< uuidv4 identifying the node. Unique for each account.
   string                                  name;                        ///< Name of the node.
   string                                  details;                     ///< Describes the additional details of the node.
   vector< fixed_string_32 >               attributes;                  ///< List of attributes types for this node.
   vector< fixed_string_32 >               attribute_values;            ///< List of attribute values for this node.
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
            attributes.push_back( t );
         }
         for( auto t : o.attribute_values )
         {
            attribute_values.push_back( t );
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
   vector< fixed_string_32 >                attributes;                  ///< List of attributes types for this edge.
   vector< fixed_string_32 >                attribute_values;            ///< List of attribute values for this edge.
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
            attributes.push_back( t );
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
   vector< fixed_string_32 >          attributes;                  ///< List of attributes that each node is required to have.
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
            attributes.push_back( t );
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
   vector< fixed_string_32 >                 attributes;                  ///< List of attributes that each edge is required to have.
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
      expiration( o.expiration ),
      paid( o.paid ){}

   transfer_request_api_obj(){}

   transfer_request_id_type               id;
   account_name_type                      to;             ///< Account requesting the transfer.
   account_name_type                      from;           ///< Account that is being requested to accept the transfer.
   asset                                  amount;         ///< The amount of asset to transfer.
   string                                 request_id;     ///< uuidv4 of the request transaction.
   string                                 memo;           ///< The memo is plain-text, encryption on the memo is advised.
   time_point                             expiration;     ///< time that the request expires.
   bool                                   paid;           ///< True when the request has been paid.
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
      product_details( to_string( o.product_details ) ),
      product_image( to_string( o.product_image ) ),
      delivery_details( to_string( o.delivery_details ) ),
      created( o.created ),
      last_updated( o.last_updated ),
      active( o.active )
      {
         for( auto s : o.product_variants )
         {
            product_variants.push_back( s );
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
            delivery_variants.push_back( s );
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
   vector< fixed_string_32 >             product_variants;       ///< The collection of product variants. Each map must have a key for each variant.
   string                                product_details;        ///< The Description details of each variant of the product.
   string                                product_image;          ///< IPFS reference to primary image of product.
   vector< asset >                       product_prices;         ///< The price (or min auction price) for each variant of the product.
   map< uint32_t, uint16_t >             wholesale_discount;     ///< Discount percentages that are applied when quantity is above a given size.
   vector< uint32_t >                    stock_available;        ///< The available stock of each variant of the product.
   vector< fixed_string_32 >             delivery_variants;      ///< The types of product delivery available to purchasers.
   string                                delivery_details;       ///< The Description details of each variant of the delivery.
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
      delivery_variant( o.delivery_variant ),
      delivery_details( to_string( o.delivery_details ) ),
      order_value( o.order_value ),
      created( o.created ),
      last_updated( o.last_updated )
      {
         for( auto v : o.order_variants )
         {
            order_variants.push_back( v );
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
   vector< fixed_string_32 >         order_variants;         ///< Variants of product ordered in the purchase.
   vector< uint32_t >                order_size;             ///< The number of each product variant ordered. 
   string                            memo;                   ///< The memo for the transaction, encryption on the memo is advised.
   string                            json;                   ///< Additional JSON object attribute details.
   public_key_type                   purchase_public_key;    ///< the Public key used to encrypt the memo and shipping address. 
   string                            shipping_address;       ///< The shipping address requested, encrypted with the secure key of the seller.
   fixed_string_32                   delivery_variant;       ///< The type of product delivery selected.
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
      product_image( to_string( o.product_image ) ),
      reserve_bid( o.reserve_bid ),
      maximum_bid( o.maximum_bid ),
      delivery_details( to_string( o.delivery_details ) ),
      final_bid_time( o.final_bid_time ),
      completion_time( o.completion_time ),
      bid_count( o.bid_count ),
      winning_bidder( o.winning_bidder ),
      winning_bid_id( to_string( o.winning_bid_id ) ),
      completed( o.completed ),
      created( o.created ),
      last_updated( o.last_updated )
      {
         for( auto s : o.delivery_variants )
         {
            delivery_variants.push_back( s );
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
   string                                product_image;          ///< IPFS reference to primary image of product.
   asset                                 reserve_bid;            ///< The min auction bid, or minimum price of a reverse auction at final bid time.
   asset                                 maximum_bid;            ///< The max auction bid. Auction will immediately conclude if this price is bidded. Starting price of reverse auction.
   vector< fixed_string_32 >             delivery_variants;      ///< The types of product delivery available to purchasers.
   string                                delivery_details;       ///< The Description details of each variant of the delivery.
   vector< asset >                       delivery_prices;        ///< The price for each variant of delivery.
   time_point                            final_bid_time;         ///< No more bids will be accepted after this time. Concealed bids must be revealed before completion time.
   time_point                            completion_time;        ///< Time that the auction will select the winning bidder, or end if no bids.
   uint32_t                              bid_count = 0;          ///< Number of bids placed on the auction.
   account_name_type                     winning_bidder;         ///< Name of the account the created the winning bid.
   string                                winning_bid_id;         ///< uuidv4 of the winning bid.
   bool                                  completed;              ///< True when the auction is completed.
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
      public_bid_amount( o.public_bid_amount.value ),
      memo( to_string( o.memo ) ),
      json( to_string( o.json ) ),
      bid_public_key( o.bid_public_key ),
      shipping_address( to_string( o.shipping_address ) ),
      delivery_variant( o.delivery_variant ),
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
   int64_t                           public_bid_amount;      ///< Set to 0 initially for concealed bid, revealed to match commitment. Revealed in initial bid if open.
   string                            memo;                   ///< The memo for the transaction, encryption on the memo is advised.
   string                            json;                   ///< Additional JSON object attribute details.
   public_key_type                   bid_public_key;         ///< the Public key used to encrypt the memo and shipping address. 
   string                            shipping_address;       ///< The shipping address requested, encrypted with the secure key of the seller.
   fixed_string_32                   delivery_variant;       ///< The type of product delivery selected.
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
      collateralization( o.collateralization() ),
      target_collateral_ratio( *o.target_collateral_ratio ),
      interface( o.interface ),
      real_price( o.real_price() ){}

   call_order_api_obj(){}

   call_order_id_type      id;
   account_name_type       borrower;
   asset                   collateral;                  ///< Funds of backing asset held as security to back the loan of debt asset.
   asset                   debt;                        ///< Funds borrowed as issuance of a stablecoin.
   price                   collateralization;                  ///< Collateral / Debt
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
      total_supply( a.get_total_supply().amount.value ),
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
      enterprise_fund_reward_percent( a.enterprise_fund_reward_percent ),
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
   uint16_t                        enterprise_fund_reward_percent;      ///< Percentage of reward paid to community fund proposals.
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
      base_symbol( p.base_symbol ),
      credit_symbol( p.credit_symbol ),
      base_balance( p.base_balance ),
      borrowed_balance( p.borrowed_balance ),
      credit_balance( p.credit_balance ),
      last_interest_rate( p.last_interest_rate.value ),
      last_price( p.last_price ){}

   credit_pool_api_obj(){}

   asset_credit_pool_id_type         id; 
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
      prediction_symbol( p.prediction_symbol ),
      collateral_symbol( p.collateral_symbol ),
      collateral_pool( p.collateral_pool ),
      prediction_bond_pool( p.prediction_bond_pool ),
      outcome_details( to_string( p.outcome_details ) ),
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
      }

   prediction_pool_api_obj(){}

   asset_prediction_pool_id_type                id; 
   asset_symbol_type                            prediction_symbol;        ///< Ticker symbol of the prediction pool primary asset.
   asset_symbol_type                            collateral_symbol;        ///< Ticker symbol of the collateral asset backing the prediction market.
   asset                                        collateral_pool;          ///< Funds accumulated by outcome asset positions for distribution to winning outcome.
   asset                                        prediction_bond_pool;     ///< Security deposit placed by the issuer on the market.
   vector< asset_symbol_type >                  outcome_assets;           ///< Outcome asset symbols for the market.
   string                                       outcome_details;          ///< Description of each outcome and the resolution conditions for each asset. 
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
      loan_price( o.loan_price() ),
      liquidation_price( o.liquidation_price ),
      symbol_a( o.symbol_a ),
      symbol_b( o.symbol_b ),
      symbol_liquid( o.symbol_liquid ),
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
   asset_symbol_type          symbol_liquid;           ///< The symbol of the liquidity pool that underlies the loan object. 
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


struct account_curation_metrics_api_obj
{
   account_curation_metrics_api_obj( const tags::account_curation_metrics_object& o ):
      id( o.id ),
      account( o.account )
      {
         for( auto a : o.author_votes )
         {
            author_votes[ a.first ] = a.second;
         }
         for( auto a : o.community_votes )
         {
            community_votes[ a.first ] = a.second;
         }
         for( auto a : o.tag_votes )
         {
            tag_votes[ a.first ] = a.second;
         }

         for( auto a : o.author_views )
         {
            author_views[ a.first ] = a.second;
         }
         for( auto a : o.community_views )
         {
            community_views[ a.first ] = a.second;
         }
         for( auto a : o.tag_views )
         {
            tag_views[ a.first ] = a.second;
         }

         for( auto a : o.author_shares )
         {
            author_shares[ a.first ] = a.second;
         }
         for( auto a : o.community_shares )
         {
            community_shares[ a.first ] = a.second;
         }
         for( auto a : o.tag_shares )
         {
            tag_shares[ a.first ] = a.second;
         }
      }

   account_curation_metrics_api_obj(){}

   tags::account_curation_metrics_id_type       id;
   account_name_type                            account;
   map< account_name_type, uint32_t >           author_votes;
   map< community_name_type, uint32_t >         community_votes;
   map< tag_name_type, uint32_t >               tag_votes;
   map< account_name_type, uint32_t >           author_views;
   map< community_name_type, uint32_t >         community_views;
   map< tag_name_type, uint32_t >               tag_views;
   map< account_name_type, uint32_t >           author_shares;
   map< community_name_type, uint32_t >         community_shares;
   map< tag_name_type, uint32_t >               tag_shares;
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
         (reward_curve)
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
         (symbol)
         (total_pending_reward_balance)
         (content_reward_balance)
         (validation_reward_balance) 
         (txn_stake_reward_balance) 
         (work_reward_balance)
         (producer_activity_reward_balance) 
         (supernode_reward_balance)
         (power_reward_balance)
         (enterprise_fund_balance)
         (development_reward_balance)
         (marketing_reward_balance)
         (advocacy_reward_balance)
         (activity_reward_balance)
         (premium_partners_fund_balance)
         (recent_content_claims)
         (recent_activity_claims)
         (last_updated)
         );


//=================================//
// ===== Account API Objects ===== //
//=================================//



FC_REFLECT( node::app::account_api_obj,
         (id)
         (name)
         (details)
         (url)
         (profile_image)
         (cover_image)
         (json)
         (json_private)
         (first_name)
         (last_name)
         (gender)
         (date_of_birth)
         (email)
         (phone)
         (nationality)
         (relationship)
         (political_alignment)
         (pinned_permlink)
         (interests)
         (membership)
         (secure_public_key)
         (connection_public_key)
         (friend_public_key)
         (companion_public_key)
         (owner_auth)
         (active_auth)
         (posting_auth)
         (proxy)
         (proxied)
         (registrar)
         (referrer)
         (recovery_account)
         (reset_account)
         (membership_interface)
         (reset_delay_days)
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
         (enterprise_vote_count)
         (recurring_membership)
         (created)
         (membership_expiration)
         (last_updated)
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
         (profile_image)
         (cover_image)
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
         (active)
         (created)
         (last_updated)
         );

FC_REFLECT( node::app::account_executive_vote_api_obj,
         (id)
         (account)
         (business_account)
         (executive_account)
         (role)
         (vote_rank)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::account_officer_vote_api_obj,
         (id)
         (account)
         (business_account)
         (officer_account)
         (vote_rank)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::account_permission_api_obj,
         (id)
         (account)
         (whitelisted_accounts)
         (blacklisted_accounts)
         (whitelisted_assets)
         (blacklisted_assets)
         (whitelisted_communities)
         (blacklisted_communities)
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
         (member_communities)
         (moderator_communities)
         (admin_communities)
         (founder_communities)
         (followed_tags)
         (filtered)
         (filtered_communities)
         (filtered_tags)
         (last_updated)
         );

FC_REFLECT( node::app::account_tag_following_api_obj,
         (id)
         (tag)
         (followers)
         (last_updated)
         );

FC_REFLECT( node::app::account_connection_api_obj,
         (id)
         (account_a)
         (encrypted_key_a)
         (message_a)
         (json_a)
         (account_b)
         (encrypted_key_b)
         (message_b)
         (json_b)
         (connection_type)
         (connection_id)
         (message_count)
         (consecutive_days)
         (last_message_time_a)
         (last_message_time_b)
         (last_updated)
         (created)
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

FC_REFLECT( node::app::network_officer_vote_api_obj,
         (id)
         (account)
         (network_officer)
         (officer_type)
         (vote_rank)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::executive_board_api_obj,
         (id)
         (account)
         (details)
         (url)
         (json)
         (budget)
         (vote_count)
         (voting_power)
         (producer_vote_count)
         (producer_voting_power)
         (last_updated)
         (created)
         (active)
         (board_approved)
         );

FC_REFLECT( node::app::executive_board_vote_api_obj,
         (id)
         (account)
         (executive_board)
         (vote_rank)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::governance_account_api_obj,
         (id)
         (account)
         (details)
         (url)
         (json)
         (subscriber_count)
         (subscriber_power)
         (producer_subscriber_count)
         (producer_subscriber_power)
         (last_updated)
         (created)
         (active)
         (account_approved)
         );

FC_REFLECT( node::app::governance_subscription_api_obj,
         (id)
         (account)
         (governance_account)
         (vote_rank)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::supernode_api_obj,
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

FC_REFLECT( node::app::interface_api_obj,
         (id)
         (account)
         (details)
         (url)
         (json)
         (daily_active_users)
         (monthly_active_users)
         (last_updated)
         (created)
         (active)
         );

FC_REFLECT( node::app::mediator_api_obj,
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

FC_REFLECT( node::app::enterprise_api_obj,
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

FC_REFLECT( node::app::enterprise_vote_api_obj,
         (id)
         (voter)
         (account)
         (enterprise_id)
         (vote_rank)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::enterprise_fund_api_obj,
         (id)
         (funder)
         (account)
         (enterprise_id)
         (amount)
         (last_updated)
         (created)
         );


//=================================//
// ===== Comment API Objects ===== //
//=================================//


FC_REFLECT( node::app::comment_api_obj,
         (id)
         (author)
         (permlink)
         (parent_author)
         (parent_permlink)
         (title)
         (body)
         (body_private)
         (url)
         (url_private)
         (ipfs)
         (ipfs_private)
         (magnet)
         (magnet_private)
         (json)
         (json_private)
         (language)
         (public_key)
         (community)
         (tags)
         (collaborating_authors)
         (supernodes)
         (interface)
         (latitude)
         (longitude)
         (comment_price)
         (premium_price)
         (rating)
         (root_comment)
         (post_type)
         (reach)
         (reply_connection)
         (category)
         (payments_received)
         (beneficiaries)
         (last_updated)
         (created)
         (active)
         (last_payout)
         (author_reputation)
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
         (reward_currency)
         (reward_curve)
         (allow_replies)
         (allow_votes)
         (allow_views)
         (allow_shares)
         (allow_curation_rewards)
         (root)
         (encrypted)
         (deleted)
         );

FC_REFLECT( node::app::comment_feed_api_obj,
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

FC_REFLECT( node::app::comment_blog_api_obj,
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

FC_REFLECT( node::app::comment_vote_api_obj,
         (id)
         (voter)
         (comment)
         (interface)
         (weight)
         (max_weight)
         (reward)
         (vote_percent)
         (reaction)
         (json)
         (last_updated)
         (created)
         (num_changes)
         );

FC_REFLECT( node::app::comment_view_api_obj,
         (id)
         (viewer)
         (comment)
         (interface)
         (supernode)
         (reward)
         (weight)
         (max_weight)
         (json)
         (created)
         );

FC_REFLECT( node::app::comment_share_api_obj,
         (id)
         (sharer)
         (comment)
         (interface)
         (reward)
         (weight)
         (max_weight)
         (json)
         (reach)
         (created)
         );

FC_REFLECT( node::app::comment_moderation_api_obj,
         (id)
         (moderator)
         (comment)
         (community)
         (tags)
         (rating)
         (details)
         (json)
         (interface)
         (filter)
         (removal_requested)
         (beneficiaries_requested)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::message_api_obj,
         (id)
         (sender)
         (recipient)
         (community)
         (sender_public_key)
         (recipient_public_key)
         (community_public_key)
         (parent_message)
         (message)
         (ipfs)
         (json)
         (uuid)
         (interface)
         (last_updated)
         (created)
         (expiration)
         );

FC_REFLECT( node::app::list_api_obj,
         (id)
         (creator)
         (list_id)
         (name)
         (details)
         (json)
         (interface)
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
         (community)
         (public_key)
         (interface)
         (details)
         (json)
         (poll_option_0)
         (poll_option_1)
         (poll_option_2)
         (poll_option_3)
         (poll_option_4)
         (poll_option_5)
         (poll_option_6)
         (poll_option_7)
         (poll_option_8)
         (poll_option_9)
         (completion_time)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::poll_vote_api_obj,
         (id)
         (voter)
         (creator)
         (poll_id)
         (public_key)
         (json)
         (details)
         (poll_option)
         (interface)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::premium_purchase_api_obj,
         (id)
         (account)
         (comment)
         (premium_price)
         (interface)
         (expiration)
         (last_updated)
         (created)
         (released)
         );

FC_REFLECT( node::app::premium_purchase_key_api_obj,
         (id)
         (provider)
         (account)
         (comment)
         (interface)
         (encrypted_key)
         (last_updated)
         (created)
         );


//===================================//
// ===== Community API Objects ===== //
//===================================//


FC_REFLECT( node::app::community_api_obj,
         (id)
         (name)
         (founder)
         (display_name)
         (details)
         (url)
         (profile_image)
         (cover_image)
         (json)
         (json_private)
         (pinned_author)
         (pinned_permlink)
         (tags)
         (community_privacy)
         (community_member_key)
         (community_moderator_key)
         (community_admin_key)
         (reward_currency)
         (membership_price)
         (max_rating)
         (flags)
         (permissions)
         (subscriber_count)
         (post_count)
         (comment_count)
         (vote_count)
         (view_count)
         (share_count)
         (created)
         (last_updated)
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

FC_REFLECT( node::app::community_federation_api_obj,
         (id)
         (community_a)
         (encrypted_key_a)
         (message_a)
         (json_a)
         (community_b)
         (encrypted_key_b)
         (message_b)
         (json_b)
         (federation_type)
         (federation_id)
         (share_accounts_a)
         (share_accounts_b)
         (approved_a)
         (approved_b)
         (last_updated)
         (created)
         (approved)
         );

FC_REFLECT( node::app::community_event_api_obj,
         (id)
         (community)
         (event_id)
         (public_key)
         (event_name)
         (location)
         (latitude)
         (longitude)
         (details)
         (url)
         (json)
         (interface)
         (event_price)
         (interested)
         (attending)
         (not_attending)
         (event_start_time)
         (event_end_time)
         (last_updated)
         (created)
         );

FC_REFLECT( node::app::community_event_attend_api_obj,
         (id)
         (attendee)
         (community)
         (event_id)
         (public_key)
         (interface)
         (message)
         (json)
         (interested)
         (attending)
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
         (paid)
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
         (product_image)
         (product_prices)
         (wholesale_discount)
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
         (product_image)
         (reserve_bid)
         (maximum_bid)
         (delivery_variants)
         (delivery_details)
         (delivery_prices)
         (final_bid_time)
         (completion_time)
         (bid_count)
         (winning_bidder)
         (winning_bid_id)
         (completed)
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
         (last_updated)
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
         (collateralization)
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
         (enterprise_fund_reward_percent)
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
         (base_symbol)
         (quote_symbol)
         (call_symbols)
         (call_strikes)
         (put_symbols)
         (put_strikes)
         );

FC_REFLECT( node::app::prediction_pool_api_obj,
         (id)
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
         (symbol_liquid)
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

FC_REFLECT( node::app::account_curation_metrics_api_obj,
         (id)
         (account)
         (author_votes)
         (community_votes)
         (tag_votes)
         (author_views)
         (community_views)
         (tag_views)
         (author_shares)
         (community_shares)
         (tag_shares)
         );