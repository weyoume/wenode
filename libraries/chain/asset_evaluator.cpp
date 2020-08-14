
#include <node/chain/node_evaluator.hpp>
#include <node/chain/database.hpp>
#include <node/chain/custom_operation_interpreter.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/block_summary_object.hpp>
#include <cmath>

#include <node/chain/util/reward.hpp>

#ifndef IS_LOW_MEM
//#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#endif

#include <fc/uint128.hpp>
#include <fc/utf8.hpp>

#include <limits>

namespace node { namespace chain {


//==========================//
// === Asset Evaluators === //
//==========================//


// The following Asset Evaluators are inspired by the framework of the Bitshares core codebase with much appreciation.
// https://bitshares.org
// https://github.com/bitshares/bitshares-core
// https://readthedocs.org/projects/howbitsharesworks/downloads/pdf/master/

/**
 * ASSET TYPES
 * 
 * CURRENCY_ASSET,         ///< Cryptocurrency that is issued by the network, starts from zero supply, issuing account is the null account, cannot be issued by any accounts. 
 * STANDARD_ASSET,         ///< Regular asset, can be transferred and staked, saved, and delegated.
 * STABLECOIN_ASSET,       ///< Asset backed by collateral that tracks the value of an external asset. Redeemable at any time with settlement.
 * EQUITY_ASSET,           ///< Asset issued by a business account that distributes a dividend from incoming revenue, and has voting power over a business accounts transactions.
 * BOND_ASSET,             ///< Asset backed by collateral that pays a coupon rate and is redeemed after expiration.
 * CREDIT_ASSET,           ///< Asset issued by a business account that is backed by repayments up to a face value, and interest payments.
 * STIMULUS_ASSET,         ///< Asset issued by a business account with expiring balances that is distributed to a set of accounts on regular intervals.
 * LIQUIDITY_POOL_ASSET,   ///< Asset that is backed by the deposits of an asset pair's liquidity pool and earns trading fees. 
 * CREDIT_POOL_ASSET,      ///< Asset that is backed by deposits of the base asset, used for borrowing funds from the pool, used as collateral to borrow base asset.
 * OPTION_ASSET,           ///< Asset that enables the execution of a trade at a specific strike price until an expiration date. 
 * PREDICTION_ASSET,       ///< Asset backed by an underlying collateral claim, on the condition that a prediction market resolve in a particular outcome.
 * UNIQUE_ASSET            ///< Asset with a supply of one, contains metadata relating to the ownership of a unique non-fungible asset.
 */

/**
 * Creates a new asset object.
 * 
 * Also creates its dynamic data object, 
 * and stablecoin object if it is a stablecoin.
 * Creates the liquitity pool between the new asset and the core asset.
 * Creates the credit pool for the new asset.
 */
void asset_create_evaluator::do_apply( const asset_create_operation& o )
{ try {
   const account_name_type& signed_for = o.issuer;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   time_point now = _db.head_block_time();

   const account_object& issuer = _db.get_account( o.issuer );

   FC_ASSERT( ( issuer.last_asset_created + MIN_ASSET_CREATE_INTERVAL ) <= now, 
      "Can only create one asset per day. Please try again tomorrow." );

   account_name_type issuer_account_name = o.issuer;
   auto& asset_indx =_db.get_index< asset_index >().indices().get< by_symbol >();
   auto asset_symbol_itr = asset_indx.find( o.symbol );

   FC_ASSERT( asset_symbol_itr == asset_indx.end(),
      "Asset with this symbol already exists, please choose a new symbol." );
   
   asset liquid_coin = _db.get_liquid_balance( o.issuer, SYMBOL_COIN );
   asset liquid_usd = _db.get_liquid_balance( o.issuer, SYMBOL_USD );

   size_t asset_length = o.symbol.size();
   asset coin_liq = median_props.asset_coin_liquidity;
   asset usd_liq = median_props.asset_usd_liquidity;

   if( is_premium_account_name( o.symbol ) )    // Double fee per character less than 8 characters.
   {
      coin_liq.amount = share_type( coin_liq.amount.value << uint16_t( 8 - asset_length ) );
      usd_liq.amount = share_type( usd_liq.amount.value << uint16_t( 8 - asset_length ) );
   }

   FC_ASSERT( o.options.whitelist_authorities.size() <= median_props.maximum_asset_whitelist_authorities,
      "Too many Whitelist authorities." );
   FC_ASSERT( o.options.blacklist_authorities.size() <= median_props.maximum_asset_whitelist_authorities,
      "Too many Blacklist authorities." );
   FC_ASSERT( o.options.stake_intervals <= median_props.max_stake_intervals && o.options.stake_intervals >= 0, 
      "Asset stake intervals outside of acceptable limits." );
   FC_ASSERT( o.options.unstake_intervals <= median_props.max_unstake_intervals && o.options.unstake_intervals >= 0, 
      "Asset unstake intervals outside of acceptable limits." );

   for( auto account : o.options.whitelist_authorities )
   {
      _db.get_account( account );    // Check that all authorities are valid accounts
   }
     
   for( auto account : o.options.blacklist_authorities )
   {
      _db.get_account( account );
   }
     
   string asset_string = o.symbol;
   auto dotpos = asset_string.find( '.' );

   if( dotpos != std::string::npos )
   {
      auto prefix = asset_string.substr( 0, dotpos );
      auto asset_symbol_itr = asset_indx.find( prefix );

      FC_ASSERT( asset_symbol_itr != asset_indx.end(),
         "Asset: ${s} may only be created by issuer of ${p}, but ${p} has not been registered.",
         ("s",o.symbol)("p",prefix) );

      FC_ASSERT( asset_symbol_itr->issuer == o.issuer,
         "Asset: ${s} may only be created by issuer of ${p}, ${i}.",
         ("s",o.symbol)("p",prefix)("i", o.issuer) );
   }

   asset_property_type asset_property = asset_property_type::STANDARD_ASSET;

   for( size_t i = 0; i < asset_property_values.size(); i++ )
   {
      if( o.asset_type == asset_property_values[ i ] )
      {
         asset_property = asset_property_type( i );
         break;
      }
   }

   switch( asset_property )  // Asset specific requirements
   {
      case asset_property_type::STANDARD_ASSET:
      {
         // No specific checks
      }
      break;
      case asset_property_type::CURRENCY_ASSET:
      {
         issuer_account_name = NULL_ACCOUNT;

         FC_ASSERT( o.options.block_reward.symbol == o.symbol,
            "Currency asset must have a block reward denominated in the asset symbol: ${s}",
            ("s",o.symbol));

         FC_ASSERT( o.coin_liquidity >= coin_liq, 
            "Asset has insufficient initial COIN liquidity." );
         FC_ASSERT( o.usd_liquidity >= usd_liq, 
            "Asset has insufficient initial USD liquidity." );
         FC_ASSERT( liquid_coin >= o.coin_liquidity, 
            "Issuer has insufficient coin balance to provide specified initial liquidity." );
         FC_ASSERT( liquid_usd >= o.usd_liquidity, 
            "Issuer has insufficient USD balance to provide specified initial liquidity." );

         const reward_fund_object& reward_fund = _db.create< reward_fund_object >( [&]( reward_fund_object& rfo )
         {
            rfo.symbol = o.symbol;
            rfo.content_reward_balance = asset( 0, o.symbol );
            rfo.validation_reward_balance = asset( 0, o.symbol );
            rfo.txn_stake_reward_balance = asset( 0, o.symbol );
            rfo.work_reward_balance = asset( 0, o.symbol );
            rfo.producer_activity_reward_balance = asset( 0, o.symbol );
            rfo.supernode_reward_balance = asset( 0, o.symbol );
            rfo.power_reward_balance = asset( 0, o.symbol );
            rfo.community_fund_balance = asset( 0, o.symbol );
            rfo.development_reward_balance = asset( 0, o.symbol );
            rfo.marketing_reward_balance = asset( 0, o.symbol );
            rfo.advocacy_reward_balance = asset( 0, o.symbol );
            rfo.activity_reward_balance = asset( 0, o.symbol );
            rfo.premium_partners_fund_balance = asset( 0, o.symbol );
            rfo.recent_content_claims = 0;
            rfo.last_updated = now;
         });

         ilog( "Reward Fund: ${r}",("r",reward_fund));

         const asset_currency_data_object& currency = _db.create< asset_currency_data_object >( [&]( asset_currency_data_object& a )
         {
            a.symbol = o.symbol;
            a.block_reward = o.options.block_reward;
            a.block_reward_reduction_percent = o.options.block_reward_reduction_percent;
            a.block_reward_reduction_days = o.options.block_reward_reduction_days;;
            a.content_reward_percent = o.options.content_reward_percent;
            a.equity_asset = o.options.equity_asset;
            a.equity_reward_percent = o.options.equity_reward_percent;
            a.producer_reward_percent = o.options.producer_reward_percent;
            a.supernode_reward_percent = o.options.supernode_reward_percent;
            a.power_reward_percent = o.options.power_reward_percent;
            a.community_fund_reward_percent = o.options.community_fund_reward_percent;
            a.development_reward_percent = o.options.development_reward_percent;
            a.marketing_reward_percent = o.options.marketing_reward_percent;
            a.advocacy_reward_percent = o.options.advocacy_reward_percent;
            a.activity_reward_percent = o.options.activity_reward_percent;
            a.producer_block_reward_percent = o.options.producer_block_reward_percent;
            a.validation_reward_percent = o.options.validation_reward_percent;
            a.txn_stake_reward_percent = o.options.txn_stake_reward_percent;
            a.work_reward_percent = o.options.work_reward_percent;
            a.producer_activity_reward_percent = o.options.producer_activity_reward_percent;
         });

         ilog( "Currency Data: ${c}",("c",currency));
      }
      break;
      case asset_property_type::STABLECOIN_ASSET:
      {
         

         const asset_object& backing_asset = _db.get_asset( o.options.backing_asset );
         if( backing_asset.is_market_issued() )
         {
            const asset_stablecoin_data_object& backing_stablecoin_data = _db.get_stablecoin_data( backing_asset.symbol );
            const asset_object& backing_backing_asset = _db.get_asset( backing_stablecoin_data.backing_asset );

            FC_ASSERT( !backing_backing_asset.is_market_issued(),
               "May not create a stablecoin backed by a stablecoin backed by a stablecoin." );
            FC_ASSERT( backing_backing_asset.symbol == SYMBOL_COIN,
               "Backing asset should be the core asset.");
         } 
         else 
         {
            FC_ASSERT( backing_asset.symbol == SYMBOL_COIN,
               "Backing asset should be the core asset.");
         }
            
         FC_ASSERT( o.options.feed_lifetime > MIN_FEED_LIFETIME,
            "Feed lifetime must be greater than network minimum." );
         FC_ASSERT( o.options.asset_settlement_delay > MIN_SETTLEMENT_DELAY,
            "Force Settlement delay must be greater than network minimum." );

         const asset_stablecoin_data_object& stablecoin = _db.create< asset_stablecoin_data_object >( [&]( asset_stablecoin_data_object& a )
         {
            a.symbol = o.symbol;
            a.backing_asset = o.options.backing_asset;
            a.current_feed_publication_time = now;
            a.feed_lifetime = o.options.feed_lifetime;
            a.minimum_feeds = o.options.minimum_feeds;
            a.asset_settlement_delay = o.options.asset_settlement_delay;
            a.asset_settlement_offset_percent = o.options.asset_settlement_offset_percent;
            a.maximum_asset_settlement_volume = o.options.maximum_asset_settlement_volume;

            price_feed feed;
            feed.settlement_price = price( asset( 1, o.symbol ), asset( 1, o.options.backing_asset ) );

            a.feeds[ o.issuer ] = make_pair( now, feed );
            a.update_median_feeds( now );
         });

         asset init_stablecoin_supply = asset( o.coin_liquidity.amount + o.usd_liquidity.amount + o.credit_liquidity.amount, o.symbol );
         asset init_collateral = init_stablecoin_supply * stablecoin.current_maintenance_collateralization;
         asset liquid_collateral = _db.get_liquid_balance( o.issuer, init_collateral.symbol );

         FC_ASSERT( liquid_collateral >= init_collateral, 
            "Issuer has insufficient initial collateral asset to back stablecoin initial balances. Required: ${r} Actual: ${a}",
            ("r",init_collateral.to_string())("a",liquid_collateral.to_string()) );

         _db.adjust_liquid_balance( o.issuer, -init_collateral );
         _db.adjust_pending_supply( init_collateral );

         const call_order_object& call = _db.create< call_order_object >( [&]( call_order_object& coo )
         {
            coo.borrower = o.issuer;
            coo.collateral = init_collateral;
            coo.debt = init_stablecoin_supply;
            coo.created = now;
            coo.last_updated = now;
         });

         FC_ASSERT( o.coin_liquidity >= coin_liq, 
            "Asset has insufficient initial COIN liquidity." );
         FC_ASSERT( o.usd_liquidity >= usd_liq, 
            "Asset has insufficient initial USD liquidity." );
         FC_ASSERT( liquid_coin >= o.coin_liquidity, 
            "Issuer has insufficient COIN balance to provide specified initial liquidity." );
         FC_ASSERT( liquid_usd >= o.usd_liquidity, 
            "Issuer has insufficient USD balance to provide specified initial liquidity." );

         ilog( "Stablecoin Data: \n ${s} \n Created Call: \n ${c} \n",
            ("s",stablecoin)("c",call) );
      }
      break;
      case asset_property_type::EQUITY_ASSET:
      {
         FC_ASSERT( o.coin_liquidity >= coin_liq, 
            "Asset has insufficient initial COIN liquidity." );
         FC_ASSERT( o.usd_liquidity >= usd_liq, 
            "Asset has insufficient initial USD liquidity." );
         FC_ASSERT( liquid_coin >= o.coin_liquidity, 
            "Issuer has insufficient coin balance to provide specified initial liquidity." );
         FC_ASSERT( liquid_usd >= o.usd_liquidity, 
            "Issuer has insufficient USD balance to provide specified initial liquidity." );

         const account_business_object& abo = _db.get_account_business( o.issuer );
         FC_ASSERT( abo.account == o.issuer, 
            "Account: ${s} must be a business account to create an equity asset.",
            ("s", o.issuer) );
         uint16_t revenue_share_sum = o.options.dividend_share_percent;
         for( auto share : abo.equity_revenue_shares )
         {
            revenue_share_sum += share.second;
         }
         for( auto share : abo.credit_revenue_shares )
         {
            revenue_share_sum += share.second;
         }
         FC_ASSERT( revenue_share_sum <= 50 * PERCENT_1,
            "Cannot share more than 50 percent of account revenue." );

         const asset_equity_data_object& equity = _db.create< asset_equity_data_object >( [&]( asset_equity_data_object& a )
         {
            a.symbol = o.symbol;
            a.business_account = o.issuer;
            a.last_dividend = time_point::min();
            a.dividend_share_percent = o.options.dividend_share_percent;
            a.liquid_dividend_percent = o.options.liquid_dividend_percent;
            a.staked_dividend_percent = o.options.staked_dividend_percent;
            a.savings_dividend_percent = o.options.savings_dividend_percent;
            a.liquid_voting_rights = o.options.liquid_voting_rights;
            a.staked_voting_rights = o.options.staked_voting_rights;
            a.savings_voting_rights = o.options.savings_voting_rights;
            a.min_active_time = o.options.min_active_time;
            a.min_balance = o.options.min_balance;
            a.min_producers = o.options.min_producers;
            a.boost_balance = o.options.boost_balance;
            a.boost_activity = o.options.boost_activity;
            a.boost_producers = o.options.boost_producers;
            a.boost_top = o.options.boost_top;
         });

         ilog( "Equity Data: ${e}",("e",equity));

         _db.modify( issuer, [&]( account_object& a )
         {
            a.revenue_share = true;
         });

         _db.modify( abo, [&]( account_business_object& a )
         {
            a.equity_revenue_shares[ o.symbol ] = o.options.dividend_share_percent;
            a.equity_assets.insert( o.symbol );
         });
      }
      break;
      case asset_property_type::BOND_ASSET:
      {
         const account_business_object& abo = _db.get_account_business( o.issuer );
         FC_ASSERT( abo.account == o.issuer, 
            "Account: ${s} must be a business account to create a bond asset.",
            ("s", o.issuer) );
         FC_ASSERT( o.options.maturity_date > date_type( now + fc::days(30) ),
            "Bond Maturity date must be at least 30 days in the future. Maturity date: ${d} Current date: ${t}",
            ("d",o.options.maturity_date)("t",date_type(now)));
         
         const asset_object& value_asset = _db.get_asset( o.options.value.symbol );
         FC_ASSERT( value_asset.asset_type == asset_property_type::CURRENCY_ASSET || 
            value_asset.asset_type == asset_property_type::STABLECOIN_ASSET, 
            "Value asset must be either a currency or stablecoin type asset." );

         const asset_bond_data_object& bond = _db.create< asset_bond_data_object >( [&]( asset_bond_data_object& a )
         {
            a.business_account = o.issuer;
            a.symbol = o.symbol;
            a.value = o.options.value;
            a.collateralization = o.options.collateralization;
            a.coupon_rate_percent = o.options.coupon_rate_percent;
            a.maturity_date = o.options.maturity_date;
            a.collateral_pool = asset( 0, a.value.symbol );
         });

         ilog( "Bond Data: ${b}",("b",bond));
      }
      break;
      case asset_property_type::CREDIT_ASSET:
      {
         FC_ASSERT( o.coin_liquidity >= coin_liq, 
            "Asset has insufficient initial COIN liquidity." );
         FC_ASSERT( o.usd_liquidity >= usd_liq, 
            "Asset has insufficient initial USD liquidity." );
         FC_ASSERT( liquid_coin >= o.coin_liquidity, 
            "Issuer has insufficient coin balance to provide specified initial liquidity." );
         FC_ASSERT( liquid_usd >= o.usd_liquidity, 
            "Issuer has insufficient USD balance to provide specified initial liquidity." );

         const account_business_object& abo = _db.get_account_business( o.issuer );
         FC_ASSERT( abo.account == o.issuer, 
            "Account: ${s} must be a business account to create a credit asset.",("s", o.issuer) );
         FC_ASSERT( !o.options.buyback_price.is_null(),
            "Buyback price cannot be null." );
         FC_ASSERT( o.options.buyback_price.base.symbol == o.options.buyback_asset,
            "Buyback price must have buyback asset as base." );
         FC_ASSERT( o.options.buyback_price.quote.symbol == o.symbol,
            "Buyback price must have credit asset as quote." );
         
         const asset_object& buyback_asset = _db.get_asset( o.options.buyback_asset );
         FC_ASSERT( buyback_asset.asset_type == asset_property_type::CURRENCY_ASSET || 
            buyback_asset.asset_type == asset_property_type::STABLECOIN_ASSET, 
            "Buyback asset must be either a currency or stablecoin type asset." );

         uint16_t revenue_share_sum = o.options.buyback_share_percent;

         for( auto share : abo.equity_revenue_shares )
         {
            revenue_share_sum += share.second;
         }
         for( auto share : abo.credit_revenue_shares )
         {
            revenue_share_sum += share.second;
         }
         FC_ASSERT( revenue_share_sum <= 50 * PERCENT_1,
            "Cannot share more than 50 percent of account revenue." );

         const asset_credit_data_object& credit = _db.create< asset_credit_data_object >( [&]( asset_credit_data_object& a )
         {
            a.business_account = o.issuer;
            a.symbol = o.symbol;
            a.buyback_asset = o.options.buyback_asset;
            a.buyback_pool = asset( 0, a.buyback_asset );
            a.buyback_price = o.options.buyback_price;
            a.last_buyback = time_point::min();
            a.buyback_share_percent = o.options.buyback_share_percent;
            a.liquid_fixed_interest_rate = o.options.liquid_fixed_interest_rate;
            a.liquid_variable_interest_rate = o.options.liquid_variable_interest_rate;
            a.staked_fixed_interest_rate = o.options.staked_fixed_interest_rate;
            a.staked_variable_interest_rate = o.options.staked_variable_interest_rate;
            a.savings_fixed_interest_rate = o.options.savings_fixed_interest_rate;
            a.savings_variable_interest_rate = o.options.savings_variable_interest_rate;
            a.var_interest_range = o.options.var_interest_range;
         });

         ilog( "Credit Data: ${c}",("c",credit));

         _db.modify( issuer, [&]( account_object& a )
         {
            a.revenue_share = true;
         });

         _db.modify( abo, [&]( account_business_object& a )
         {
            a.credit_revenue_shares[ o.symbol ] = o.options.buyback_share_percent;
            a.credit_assets.insert( o.symbol );
         });
      }
      break;
      case asset_property_type::STIMULUS_ASSET:
      {
         const account_business_object& abo = _db.get_account_business( o.issuer );
         FC_ASSERT( abo.account == o.issuer, 
            "Account: ${s} must be a business account to create a stimulus asset.",("s", o.issuer) );
         FC_ASSERT( !o.options.redemption_price.is_null(),
            "Redemption price cannot be null." );
         FC_ASSERT( o.options.redemption_price.base.symbol == o.options.redemption_asset,
            "Redemption price must have Redemption asset as base." );
         FC_ASSERT( o.options.redemption_price.quote.symbol == o.symbol,
            "Redemption price must have Stimulus asset as quote." );

         flat_set< account_name_type > distribution_list;
         flat_set< account_name_type > redemption_list;

         for( account_name_type a : o.options.distribution_list )
         {
            distribution_list.insert( a );
         }
         for( account_name_type a : o.options.redemption_list )
         {
            redemption_list.insert( a );
         }

         share_type recipients = o.options.distribution_list.size();
         FC_ASSERT( o.options.max_supply >= o.options.distribution_amount.amount * recipients,
            "Stimulus asset monthly distribution must not exceed asset max supply." );
         
         const asset_object& redemption_asset = _db.get_asset( o.options.redemption_asset );
         FC_ASSERT( redemption_asset.asset_type == asset_property_type::CURRENCY_ASSET || 
            redemption_asset.asset_type == asset_property_type::STABLECOIN_ASSET, 
            "Redemption asset must be either a currency or stablecoin type asset." );

         date_type today = date_type( now );
         date_type next_distribution_date;

         if( today.month > 11 )
         {
            next_distribution_date = date_type( 1, 1, today.year + 1 );
         }
         else
         {
            next_distribution_date = date_type( 1, today.month + 1, today.year );
         }

         const asset_stimulus_data_object& stimulus = _db.create< asset_stimulus_data_object >( [&]( asset_stimulus_data_object& asdo )
         {
            asdo.business_account = o.issuer;
            asdo.symbol = o.symbol;
            asdo.redemption_asset = o.options.redemption_asset;
            asdo.redemption_pool = asset( 0, asdo.redemption_asset );
            asdo.redemption_price = o.options.redemption_price;
            asdo.redemption_list = redemption_list;
            asdo.distribution_list = distribution_list;
            asdo.distribution_amount = o.options.distribution_amount;
            asdo.next_distribution_date = next_distribution_date;
         });

         ilog( "Stimulus Data: ${s}",("s",stimulus));
      }
      break;
      case asset_property_type::UNIQUE_ASSET:
      {
         FC_ASSERT( o.options.max_supply == BLOCKCHAIN_PRECISION,
            "Unique assets must have a maximum supply of exactly one unit." );

         if( o.options.ownership_asset != o.symbol )
         {
            const asset_object ownership_asset = _db.get_asset( o.options.ownership_asset );
            FC_ASSERT( ownership_asset.is_credit_enabled(),
               "Ownership asset must be a Credit enabled asset type" );
         }
         
         const asset_unique_data_object& unique = _db.create< asset_unique_data_object >( [&]( asset_unique_data_object& audo )
         {
            audo.symbol = o.symbol;
            audo.controlling_owner = o.issuer;
            audo.ownership_asset = o.options.ownership_asset;
            for( account_name_type a : o.options.control_list )
            {
               audo.control_list.insert( a );
            }
            for( account_name_type a : o.options.access_list )
            {
               audo.access_list.insert( a );
            }
            audo.access_price = o.options.access_price;
         });

         ilog( "Unique Data: ${u}",("u",unique));
      }
      break;
      case asset_property_type::LIQUIDITY_POOL_ASSET:
      {
         FC_ASSERT( false,
            "Cannot directly create a new liquidity pool asset. Please create a liquidity pool between two existing assets." );
      }
      break;
      case asset_property_type::CREDIT_POOL_ASSET:
      {
         FC_ASSERT( false, 
            "Cannot directly create a new credit pool asset. Credit pool assets are created in addition to every asset." );
      }
      break;
      case asset_property_type::OPTION_ASSET:
      {
         FC_ASSERT( false,
            "Cannot directly create a new option asset. Option assets are issued from an Options market." );
      }
      break;
      case asset_property_type::PREDICTION_ASSET:
      {
         FC_ASSERT( false, 
            "Cannot directly create a new prediction asset. Prediction assets are issued from a Prediction market." );
      }
      break;
      default:
      {
         FC_ASSERT( false,
            "Invalid Asset Type." );
      }
      break;
   }
   
   // Create the new asset object.

   const asset_object new_asset = _db.create< asset_object >( [&]( asset_object& a )
   {
      a.symbol = o.symbol;
      a.asset_type = asset_property;
      a.issuer = issuer_account_name;
      from_string( a.display_symbol, o.options.display_symbol );
      from_string( a.details, o.options.details );
      from_string( a.json, o.options.json );
      from_string( a.url, o.options.url );
      a.max_supply = o.options.max_supply;
      a.stake_intervals = o.options.stake_intervals;
      a.unstake_intervals = o.options.unstake_intervals;
      a.market_fee_percent = o.options.market_fee_percent;
      a.market_fee_share_percent = o.options.market_fee_share_percent;
      a.issuer_permissions = o.options.issuer_permissions;
      a.flags = o.options.flags;

      for( account_name_type auth : o.options.whitelist_authorities )
      {
         a.whitelist_authorities.insert( auth );
      }
      for( account_name_type auth : o.options.blacklist_authorities )
      {
         a.blacklist_authorities.insert( auth );
      }
      for( asset_symbol_type mar : o.options.whitelist_markets )
      {
         a.whitelist_markets.insert( mar );
      }
      for( asset_symbol_type mar : o.options.blacklist_markets )
      {
         a.blacklist_markets.insert( mar );
      }
      
      a.created = now;
      a.last_updated = now;
   });

   _db.create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = o.symbol;
   });

   _db.modify( issuer, [&]( account_object& a )
   {
      a.last_asset_created = now;
   });

   if( new_asset.is_credit_enabled() )
   {
      asset_symbol_type core_liq_symbol = string( LIQUIDITY_ASSET_PREFIX )+ string( SYMBOL_COIN )+"."+ string( o.symbol );
      
      _db.create< asset_object >( [&]( asset_object& a )
      {
         a.issuer = issuer_account_name;
         a.symbol = core_liq_symbol;
         a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;    // Create the core liquidity pool for the new asset.
         from_string( a.display_symbol, core_liq_symbol );
         from_string( a.details, o.options.details );
         from_string( a.json, o.options.json );
         from_string( a.url, o.options.url );
         a.max_supply = o.options.max_supply;
         a.stake_intervals = o.options.stake_intervals;
         a.unstake_intervals = o.options.unstake_intervals;
         a.market_fee_percent = o.options.market_fee_percent;
         a.market_fee_share_percent = o.options.market_fee_share_percent;
         a.issuer_permissions = o.options.issuer_permissions;
         a.flags = o.options.flags;

         for( account_name_type auth : o.options.whitelist_authorities )
         {
            a.whitelist_authorities.insert( auth );
         }
         for( account_name_type auth : o.options.blacklist_authorities )
         {
            a.blacklist_authorities.insert( auth );
         }
         for( asset_symbol_type mar : o.options.whitelist_markets )
         {
            a.whitelist_markets.insert( mar );
         }
         for( asset_symbol_type mar : o.options.blacklist_markets )
         {
            a.blacklist_markets.insert( mar );
         }

         a.created = now;
         a.last_updated = now;
      });

      _db.create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
      {
         a.symbol = core_liq_symbol;
      });

      asset init_new_asset = asset( o.coin_liquidity.amount, o.symbol );              // Creates initial new asset supply equivalent to core liquidity. 
      asset init_liquid_asset = asset( o.coin_liquidity.amount, core_liq_symbol );    // Creates equivalent supply of the liquidity pool asset for liquidity injection.
         
      _db.create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
      {
         a.symbol_a = SYMBOL_COIN;
         a.symbol_b = o.symbol;
         a.symbol_liquid = core_liq_symbol;
         a.balance_a = o.coin_liquidity;
         a.balance_b = init_new_asset;
         a.hour_median_price = price( a.balance_a, a.balance_b );
         a.day_median_price = price( a.balance_a, a.balance_b );
         a.price_history.push_back( price( a.balance_a, a.balance_b ) );
         a.balance_liquid = init_liquid_asset;
      });

      _db.adjust_liquid_balance( o.issuer, -o.coin_liquidity );
      _db.adjust_liquid_balance( o.issuer, init_liquid_asset );
      _db.adjust_pending_supply( init_new_asset );

      asset_symbol_type usd_liq_symbol = string( LIQUIDITY_ASSET_PREFIX )+ string( SYMBOL_USD )+ "." + string( o.symbol );
      
      _db.create< asset_object >( [&]( asset_object& a )
      {
         a.issuer = issuer_account_name;
         a.symbol = usd_liq_symbol;
         a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;    // Create the USD liquidity pool for the new asset.
         from_string( a.display_symbol, usd_liq_symbol );
         from_string( a.details, o.options.details );
         from_string( a.json, o.options.json );
         from_string( a.url, o.options.url );
         a.max_supply = o.options.max_supply;
         a.stake_intervals = o.options.stake_intervals;
         a.unstake_intervals = o.options.unstake_intervals;
         a.market_fee_percent = o.options.market_fee_percent;
         a.market_fee_share_percent = o.options.market_fee_share_percent;
         a.issuer_permissions = o.options.issuer_permissions;
         a.flags = o.options.flags;

         for( account_name_type auth : o.options.whitelist_authorities )
         {
            a.whitelist_authorities.insert( auth );
         }
         for( account_name_type auth : o.options.blacklist_authorities )
         {
            a.blacklist_authorities.insert( auth );
         }
         for( asset_symbol_type mar : o.options.whitelist_markets )
         {
            a.whitelist_markets.insert( mar );
         }
         for( asset_symbol_type mar : o.options.blacklist_markets )
         {
            a.blacklist_markets.insert( mar );
         }

         a.created = now;
         a.last_updated = now;
      });

      _db.create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
      {
         a.symbol = usd_liq_symbol;
      });

      init_new_asset = asset( o.usd_liquidity.amount, o.symbol );           // Creates initial new asset supply equivalent to core liquidity. 
      init_liquid_asset = asset( o.usd_liquidity.amount, usd_liq_symbol);   // Creates equivalent supply of the liquidity pool asset for liquidity injection.
         
      _db.create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
      {
         a.symbol_a = SYMBOL_USD;
         a.symbol_b = o.symbol;
         a.symbol_liquid = usd_liq_symbol;
         a.balance_a = o.usd_liquidity;
         a.balance_b = init_new_asset;
         a.hour_median_price = price( a.balance_a, a.balance_b );
         a.day_median_price = price( a.balance_a, a.balance_b );
         a.price_history.push_back( price( a.balance_a, a.balance_b ) );
         a.balance_liquid = init_liquid_asset;
      });

      _db.adjust_liquid_balance( o.issuer, -o.usd_liquidity );
      _db.adjust_liquid_balance( o.issuer, init_liquid_asset );
      _db.adjust_pending_supply( init_new_asset );

      asset_symbol_type credit_asset_symbol = string( CREDIT_ASSET_PREFIX ) + string( o.symbol );
      
      _db.create< asset_object >( [&]( asset_object& a )
      {
         a.issuer = issuer_account_name;
         a.symbol = credit_asset_symbol;
         a.asset_type = asset_property_type::CREDIT_POOL_ASSET; // Create the asset credit pool for the new asset.
         from_string( a.display_symbol, credit_asset_symbol );
         from_string( a.details, o.options.details );
         from_string( a.json, o.options.json );
         from_string( a.url, o.options.url );
         a.max_supply = o.options.max_supply;
         a.stake_intervals = o.options.stake_intervals;
         a.unstake_intervals = o.options.unstake_intervals;
         a.market_fee_percent = o.options.market_fee_percent;
         a.market_fee_share_percent = o.options.market_fee_share_percent;
         a.issuer_permissions = o.options.issuer_permissions;
         a.flags = o.options.flags;

         for( account_name_type auth : o.options.whitelist_authorities )
         {
            a.whitelist_authorities.insert( auth );
         }
         for( account_name_type auth : o.options.blacklist_authorities )
         {
            a.blacklist_authorities.insert( auth );
         }
         for( asset_symbol_type mar : o.options.whitelist_markets )
         {
            a.whitelist_markets.insert( mar );
         }
         for( asset_symbol_type mar : o.options.blacklist_markets )
         {
            a.blacklist_markets.insert( mar );
         }

         a.created = now;
         a.last_updated = now;
      });

      _db.create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
      {
         a.symbol = credit_asset_symbol;
      });

      asset init_lent_asset = asset( o.credit_liquidity.amount, o.symbol );                       // Creates and lends equivalent new assets to the credit pool.
      asset init_credit_asset = asset( o.credit_liquidity.amount * 100, credit_asset_symbol );    // Creates equivalent credit pool assets and passes to issuer. 
      price init_credit_price = price( init_lent_asset, init_credit_asset );                      // Starts the initial credit asset exchange rate at 100:1.

      _db.create< asset_credit_pool_object >( [&]( asset_credit_pool_object& a )
      {
         a.base_symbol = o.symbol;   
         a.credit_symbol = credit_asset_symbol; 
         a.base_balance = init_lent_asset;
         a.borrowed_balance = asset( 0, o.symbol );
         a.credit_balance = init_credit_asset;
         a.last_price = init_credit_price;     // Initializes credit pool price with a ratio of 100:1
      });
      
      _db.adjust_pending_supply( init_lent_asset );
      _db.adjust_liquid_balance( o.issuer, init_credit_asset );
   }

   ilog( "Created Asset: ${a}",("a",new_asset) );
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


/**
 * Loop through assets, looking for ones that are backed by the asset being changed. 
 * When found, perform checks to verify validity.
 *
 * @param op the stablecoin update operation being performed
 * @param new_backing_asset
 */
void check_children_of_stablecoin( database& db, const asset_update_operation& o, const asset_object& new_backing_asset )
{
   if( new_backing_asset.symbol != SYMBOL_COIN )    //  If new backing asset is CORE, no child stablecoins to review. 
   {
      const auto& idx = db.get_index< asset_stablecoin_data_index >().indices().get< by_backing_asset >();
      auto backed_range = idx.equal_range( o.asset_to_update );
      std::for_each( backed_range.first, backed_range.second, // loop through all assets that have this asset as a backing asset
         [&new_backing_asset, &o, &db]( const asset_stablecoin_data_object& stablecoin_data )
         {
            const auto& child = db.get_stablecoin_data( stablecoin_data.symbol );

            FC_ASSERT( child.symbol != o.new_options.backing_asset,
               "The BitAsset would be invalidated by changing this backing asset ('A' backed by 'B' backed by 'A')." );
         });
   }
}


/**
 * @brief Apply requested changes to stablecoin options
 *
 * This applies the requested changes to the stablecoin object.
 * It also cleans up the releated feeds
 *
 * @param op the requested operation
 * @param db the database
 * @param bdo the actual database object
 * @param asset_to_update the asset_object related to this stablecoin_data_object
 * @returns true if the feed price is changed
 */
bool update_stablecoin_object_options( const asset_update_operation& o, database& db, 
   asset_stablecoin_data_object& bdo, const asset_object& asset_to_update )
{
   time_point now = db.head_block_time();

   // If the minimum number of feeds to calculate a median has changed, recalculate the median
   bool should_update_feeds = false;

   if( o.new_options.minimum_feeds != bdo.minimum_feeds )
   {
      should_update_feeds = true;
   }
      
   if( o.new_options.feed_lifetime != bdo.feed_lifetime )
   {
      should_update_feeds = true;    // call update_median_feeds if the feed_lifetime changed
   }

   bool backing_asset_changed = false;    // feeds must be reset if the backing asset is changed
   bool is_producer_fed = false;
   if( o.new_options.backing_asset != bdo.backing_asset )
   {
      backing_asset_changed = true;
      should_update_feeds = true;
      if( asset_to_update.is_producer_fed() )
      {
         is_producer_fed = true;
      }   
   }

   bdo.backing_asset = o.new_options.backing_asset;
   bdo.feed_lifetime = o.new_options.feed_lifetime;
   bdo.minimum_feeds = o.new_options.minimum_feeds;
   bdo.asset_settlement_delay = o.new_options.asset_settlement_delay;
   bdo.asset_settlement_offset_percent = o.new_options.asset_settlement_offset_percent;
   bdo.maximum_asset_settlement_volume = o.new_options.maximum_asset_settlement_volume;

   if( backing_asset_changed )        // Reset price feeds if modifying backing asset
   {
      if( is_producer_fed )
      {
         bdo.feeds.clear();
      }
      else
      {
         for( auto& current_feed : bdo.feeds )
         {
            current_feed.second.second.settlement_price = price();  // Zero all outstanding price feeds
         }
      }
   }

   if( should_update_feeds ) // Call check_call_orders if the price feed changes
   {
      const auto old_feed = bdo.current_feed;
      bdo.update_median_feeds( now );
      return ( !( old_feed == bdo.current_feed ) ); 
   }

   return false;
}


void asset_update_evaluator::do_apply( const asset_update_operation& o )
{ try {
   const account_name_type& signed_for = o.issuer;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   time_point now = _db.head_block_time();

   const asset_object& asset_obj = _db.get_asset( o.asset_to_update );

   FC_ASSERT( !asset_obj.immutable_properties(),
      "Asset has Immutable Properties and cannot be altered after creation." );
   FC_ASSERT( ( asset_obj.last_updated + MIN_ASSET_UPDATE_INTERVAL ) <= now, 
      "Can only update asset once per 10 minutes." );
   const asset_dynamic_data_object& dyn_data = _db.get_dynamic_data( o.asset_to_update );

   if( ( dyn_data.get_total_supply().amount != 0 ) )      // new issuer_permissions must be subset of old issuer permissions
   {
      FC_ASSERT(!( o.new_options.issuer_permissions & ~asset_obj.issuer_permissions ), 
         "Cannot reinstate previously revoked issuer permissions on an asset.");
   }

   // Changed flags must be subset of old issuer permissions

   FC_ASSERT(!(( o.new_options.flags ^ asset_obj.flags ) & ~asset_obj.issuer_permissions ),
      "Flag change is forbidden by issuer permissions" );
   FC_ASSERT( o.new_options.whitelist_authorities.size() <= median_props.maximum_asset_whitelist_authorities,
      "Too many Whitelist authorities." );

   for( auto account : o.new_options.whitelist_authorities )
   {
      _db.get_account( account );
   }
   FC_ASSERT( o.new_options.blacklist_authorities.size() <= median_props.maximum_asset_whitelist_authorities,
      "Too many Blacklist authorities." );
   for( auto account : o.new_options.blacklist_authorities )
   {
      _db.get_account( account );
   }

   // If we are now disabling force settlements, cancel all open force settlement orders
   if( ( o.new_options.flags & int( asset_issuer_permission_flags::disable_force_settle ) ) && asset_obj.enable_force_settle() )
   {
      const auto& idx = _db.get_index< asset_settlement_index >().indices().get< by_expiration >();
      // Re-initialize itr every loop as objects are being removed each time.
      for( auto itr = idx.lower_bound( o.asset_to_update );
         itr != idx.end() && itr->settlement_asset_symbol() == o.asset_to_update;
         itr = idx.lower_bound( o.asset_to_update ) )
      {
         _db.cancel_settle_order( *itr );
      }
   }

   switch( asset_obj.asset_type )  // Asset specific requirements
   {
      case asset_property_type::CURRENCY_ASSET:
      {
         FC_ASSERT( false, "Cannot Edit Currency asset." );
      }
      break;
      case asset_property_type::STANDARD_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Issuer can update asset. (${o.issuer} != ${a.issuer})",
            ("o.issuer", o.issuer)("asset.issuer", asset_obj.issuer) );
      }
      break;
      case asset_property_type::STABLECOIN_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Issuer can update asset. (${o.issuer} != ${a.issuer})",
            ("o.issuer", o.issuer)("asset.issuer", asset_obj.issuer) );
         FC_ASSERT( asset_obj.is_market_issued(), 
            "Asset must be market issued to update stablecoin." );
         
         const asset_stablecoin_data_object& current_stablecoin_data = _db.get_stablecoin_data( asset_obj.symbol );

         FC_ASSERT( !current_stablecoin_data.has_settlement(), 
            "Cannot update a stablecoin after a global settlement has executed." );

         const asset_object& backing_asset = _db.get_asset( o.new_options.backing_asset );

         if( o.new_options.backing_asset != current_stablecoin_data.backing_asset )   // Are we changing the backing asset?
         {
            FC_ASSERT( dyn_data.get_total_supply().amount == 0,
               "Cannot update a stablecoin's backing asset if there is already a current supply. Please globally settle first." );
            FC_ASSERT( o.new_options.backing_asset != asset_obj.symbol,
               "Cannot update a stablecoin to be backed by itself. Please select a different backing asset." );
            const asset_object& new_backing_asset = _db.get_asset( o.new_options.backing_asset ); // Check if the new backing asset exists

            if ( new_backing_asset.symbol != SYMBOL_COIN ) // not backed by CORE
            {
               check_children_of_stablecoin( _db, o, new_backing_asset );   // Checks for recursive backing assets
            }
            
            // Check if the new backing asset is itself backed by something. It must be CORE or a UIA.
            if ( new_backing_asset.is_market_issued() )
            {
               const asset_stablecoin_data_object& backing_stablecoin_data = _db.get_stablecoin_data( new_backing_asset.symbol );
               const asset_object& backing_backing_asset = _db.get_asset( backing_stablecoin_data.backing_asset );
               FC_ASSERT( ( backing_backing_asset.symbol == SYMBOL_COIN || !backing_backing_asset.is_market_issued() ),
                  "A BitAsset cannot be backed by a BitAsset that itself is backed by a BitAsset.");
            }
         }
         
         bool to_check_call_orders = false;

         _db.modify( current_stablecoin_data, [&]( asset_stablecoin_data_object& bdo )
         {
            to_check_call_orders = update_stablecoin_object_options( o, _db, bdo, asset_obj );
         });

         if( to_check_call_orders )     // Process margin calls, allow black swan, not for a new limit order
         {
            _db.check_call_orders( asset_obj, true, false );
         }
         
         if( backing_asset.is_market_issued() )
         {
            const asset_stablecoin_data_object& backing_stablecoin_data = _db.get_stablecoin_data( backing_asset.symbol );
            const asset_object& backing_backing_asset = _db.get_asset( backing_stablecoin_data.backing_asset );

            FC_ASSERT( !backing_backing_asset.is_market_issued(),
               "May not create a stablecoin backed by a stablecoin backed by a stablecoin." );
            FC_ASSERT( backing_backing_asset.symbol == SYMBOL_COIN,
               "Backing asset should be the core asset." );
         } 
         else 
         {
            FC_ASSERT( backing_asset.symbol == SYMBOL_COIN,
               "Backing asset should be the core asset.");
         }
      }
      break;
      case asset_property_type::EQUITY_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Issuer can update asset. (${o.issuer} != ${a.issuer})",
            ("o.issuer", o.issuer)("asset.issuer", asset_obj.issuer) );
         const account_business_object& bus_acc = _db.get_account_business( o.issuer );
         const asset_equity_data_object& equity_obj = _db.get_equity_data( o.asset_to_update );
         uint16_t revenue_share_sum = o.new_options.dividend_share_percent;

         for( auto share : bus_acc.equity_revenue_shares )
         {
            revenue_share_sum += share.second;
         }
         for( auto share : bus_acc.credit_revenue_shares )
         {
            revenue_share_sum += share.second;
         }

         FC_ASSERT( revenue_share_sum <= 50 * PERCENT_1,
            "Cannot share more than 50 percent of account revenue." );

         _db.modify( equity_obj, [&]( asset_equity_data_object& a )
         {
            a.dividend_share_percent = o.new_options.dividend_share_percent;
            a.liquid_dividend_percent = o.new_options.liquid_dividend_percent;
            a.staked_dividend_percent = o.new_options.staked_dividend_percent;
            a.savings_dividend_percent = o.new_options.savings_dividend_percent;
            a.liquid_voting_rights = o.new_options.liquid_voting_rights;
            a.staked_voting_rights = o.new_options.staked_voting_rights;
            a.savings_voting_rights = o.new_options.savings_voting_rights;
            a.min_active_time = o.new_options.min_active_time;
            a.min_balance = o.new_options.min_balance;
            a.min_producers = o.new_options.min_producers;
            a.boost_balance = o.new_options.boost_balance;
            a.boost_activity = o.new_options.boost_activity;
            a.boost_producers = o.new_options.boost_producers;
            a.boost_top = o.new_options.boost_top;
         });

         _db.modify( bus_acc, [&]( account_business_object& a )
         {
            a.equity_revenue_shares[ o.asset_to_update ] = o.new_options.dividend_share_percent;
         });
      }
      break;
      case asset_property_type::BOND_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Issuer can update asset. (${o.issuer} != ${a.issuer})",
            ("o.issuer", o.issuer)("asset.issuer", asset_obj.issuer) );

         const account_business_object& abo = _db.get_account_business( o.issuer );
         FC_ASSERT( abo.account == o.issuer, 
            "Account: ${s} must be a business account to create a credit asset.",("s", o.issuer) );
         FC_ASSERT( o.new_options.maturity_date > date_type( now + fc::days(30) ),
            "Maturity must be at least 30 days in the future." );
         
         const asset_object& value_asset = _db.get_asset( o.new_options.value.symbol );
         FC_ASSERT( value_asset.asset_type == asset_property_type::CURRENCY_ASSET || 
            value_asset.asset_type == asset_property_type::STABLECOIN_ASSET, 
            "Value asset must be either a currency or stablecoin type asset." );

         const asset_dynamic_data_object& bond_dyn_data = _db.get_dynamic_data( o.asset_to_update );
         const asset_bond_data_object& bond_obj = _db.get_bond_data( o.asset_to_update );

         if( bond_dyn_data.get_total_supply().amount == 0 )     // Only edit bond economics if no supply has been issued.
         {
            _db.modify( bond_obj, [&]( asset_bond_data_object& a )
            {
               a.value = o.new_options.value;
               a.collateralization = o.new_options.collateralization;
               a.coupon_rate_percent = o.new_options.coupon_rate_percent;
               a.maturity_date = o.new_options.maturity_date;
            });
         }
      }
      break;
      case asset_property_type::CREDIT_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Issuer can update asset. (${o.issuer} != ${a.issuer})",
            ("o.issuer", o.issuer)("asset.issuer", asset_obj.issuer) );
         const account_business_object& bus_acc = _db.get_account_business( o.issuer );
         const asset_credit_data_object& credit_obj = _db.get_credit_data( o.asset_to_update );
         uint16_t revenue_share_sum = o.new_options.buyback_share_percent;

         for( auto share : bus_acc.equity_revenue_shares )
         {
            revenue_share_sum += share.second;
         }
         for( auto share : bus_acc.credit_revenue_shares )
         {
            revenue_share_sum += share.second;
         }
         FC_ASSERT( revenue_share_sum <= 50 * PERCENT_1,
            "Cannot share more than 50 percent of account revenue." );
         FC_ASSERT( o.new_options.buyback_asset == credit_obj.buyback_asset,
            "Credit buyback asset cannot be altered." );
         FC_ASSERT( !o.new_options.buyback_price.is_null(),
            "Buyback price cannot be null." );
         FC_ASSERT( o.new_options.buyback_price.base.symbol == credit_obj.buyback_asset,
            "Credit buyback price base asset cannot be altered." );
         FC_ASSERT( o.new_options.buyback_price.quote.symbol == credit_obj.symbol,
            "Credit buyback price quote asset cannot be altered." );
         
         _db.modify( credit_obj, [&]( asset_credit_data_object& a )
         {
            a.buyback_price = o.new_options.buyback_price;
            a.buyback_share_percent = o.new_options.buyback_share_percent;
            a.liquid_fixed_interest_rate = o.new_options.liquid_fixed_interest_rate;
            a.liquid_variable_interest_rate = o.new_options.liquid_variable_interest_rate;
            a.staked_fixed_interest_rate = o.new_options.staked_fixed_interest_rate;
            a.staked_variable_interest_rate = o.new_options.staked_variable_interest_rate;
            a.savings_fixed_interest_rate = o.new_options.savings_fixed_interest_rate;
            a.savings_variable_interest_rate = o.new_options.savings_variable_interest_rate;
            a.var_interest_range = o.new_options.var_interest_range;
         });

         _db.modify( bus_acc, [&]( account_business_object& a )
         {
            a.credit_revenue_shares[ o.asset_to_update ] = o.new_options.buyback_share_percent;
         });
      }
      break;
      case asset_property_type::STIMULUS_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Issuer can update asset. (${o.issuer} != ${a.issuer})",
            ("o.issuer", o.issuer)("asset.issuer", asset_obj.issuer) );
         const asset_stimulus_data_object& stimulus_obj = _db.get_stimulus_data( o.asset_to_update );
         FC_ASSERT( !o.new_options.redemption_price.is_null(),
            "Redemption price cannot be null." );
         FC_ASSERT( o.new_options.redemption_price.base.symbol == o.new_options.redemption_asset,
            "Redemption price must have Redemption asset as base." );
         FC_ASSERT( o.new_options.redemption_price.quote.symbol == o.asset_to_update,
            "Redemption price must have Stimulus asset as quote." );

         flat_set< account_name_type > distribution_list;
         flat_set< account_name_type > redemption_list;

         for( account_name_type a : o.new_options.distribution_list )
         {
            distribution_list.insert( a );
         }
         for( account_name_type a : o.new_options.redemption_list )
         {
            redemption_list.insert( a );
         }

         share_type recipients = o.new_options.distribution_list.size();
         asset new_distribution = asset( o.new_options.distribution_amount.amount * recipients, stimulus_obj.symbol );
         FC_ASSERT( o.new_options.max_supply >= new_distribution.amount,
            "Stimulus asset monthly distribution must not exceed asset max supply." );

         if( stimulus_obj.redemption_price > o.new_options.redemption_price )
         {
            FC_ASSERT( stimulus_obj.redemption_pool >= new_distribution * o.new_options.redemption_price,
               "Cannot raise redemption price without sufficient redemption pool balance." );
         }
         
         _db.modify( stimulus_obj, [&]( asset_stimulus_data_object& asdo )
         {
            asdo.redemption_price = o.new_options.redemption_price;
            asdo.distribution_list = distribution_list;
            asdo.redemption_list = redemption_list;
            asdo.distribution_amount = o.new_options.distribution_amount;
         });
      }
      break;
      case asset_property_type::LIQUIDITY_POOL_ASSET:
      {
         FC_ASSERT( false, "Cannot Edit Liquidity Pool asset." );
      }
      break;
      case asset_property_type::CREDIT_POOL_ASSET:
      {
         FC_ASSERT( false, "Cannot Edit Credit pool asset." );
      }
      break;
      case asset_property_type::OPTION_ASSET:
      {
         FC_ASSERT( false, "Cannot Edit Option asset." );
      }
      break;
      case asset_property_type::UNIQUE_ASSET:
      {
         const asset_unique_data_object& unique_obj = _db.get_unique_data( o.asset_to_update );
         flat_set< account_name_type > control_list = unique_obj.control_list;
         flat_set< account_name_type > access_list = unique_obj.access_list;

         FC_ASSERT( o.new_options.max_supply == BLOCKCHAIN_PRECISION,
            "Unique assets must have a maximum supply of exactly one unit." );
         FC_ASSERT( unique_obj.is_control( o.issuer ),
            "Account must be in the control list to update unique asset. (${o.issuer} != ${a.issuer})",
            ("o.issuer", o.issuer)("asset.issuer", unique_obj.controlling_owner ));

         if( unique_obj.controlling_owner == o.issuer )
         {
            control_list.clear();
            for( account_name_type a : o.new_options.control_list )
            {
               control_list.insert( a );
            }
         }

         access_list.clear();
         for( account_name_type a : o.new_options.access_list )
         {
            access_list.insert( a );
         }
         
         _db.modify( unique_obj, [&]( asset_unique_data_object& a )
         {
            a.control_list = control_list;
            a.access_list = access_list;
            a.access_price = o.new_options.access_price;
         });
      }
      break;
      default:
      {
         FC_ASSERT( false, "Invalid Asset type." );
      }
      break;
   }

   _db.modify( asset_obj, [&]( asset_object& a )
   {
      from_string( a.display_symbol, o.new_options.display_symbol );
      from_string( a.details, o.new_options.details );
      from_string( a.json, o.new_options.json );
      from_string( a.url, o.new_options.url );
      
      a.stake_intervals = o.new_options.stake_intervals;
      a.unstake_intervals = o.new_options.unstake_intervals;
      a.market_fee_percent = o.new_options.market_fee_percent;
      a.market_fee_share_percent = o.new_options.market_fee_share_percent;
      a.issuer_permissions = o.new_options.issuer_permissions;
      a.flags = o.new_options.flags;
      
      a.whitelist_authorities.clear();
      a.blacklist_authorities.clear();
      a.whitelist_markets.clear();
      a.blacklist_markets.clear();

      for( account_name_type auth : o.new_options.whitelist_authorities )
      {
         a.whitelist_authorities.insert( auth );
      }
      for( account_name_type auth : o.new_options.blacklist_authorities )
      {
         a.blacklist_authorities.insert( auth );
      }
      for( asset_symbol_type mar : o.new_options.whitelist_markets )
      {
         a.whitelist_markets.insert( mar );
      }
      for( asset_symbol_type mar : o.new_options.blacklist_markets )
      {
         a.blacklist_markets.insert( mar );
      }
      a.last_updated = now;
   });

   ilog( "Account: ${ac} asset Update: ${a}",
      ("ac",o.issuer)("a",asset_obj));

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void asset_issue_evaluator::do_apply( const asset_issue_operation& o )
{ try {
   const account_name_type& signed_for = o.issuer;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& asset_obj = _db.get_asset( o.asset_to_issue.symbol );
   const account_object& to_account = _db.get_account( o.issue_to_account );
   FC_ASSERT( to_account.active,
      "Account: ${s} must be active to broadcast transaction.",("s", o.issue_to_account) );
   const account_permission_object& to_account_permissions = _db.get_account_permissions( o.issue_to_account );
   const asset_dynamic_data_object& asset_dyn_data = _db.get_dynamic_data( o.asset_to_issue.symbol );

   FC_ASSERT( o.issuer == asset_obj.issuer,
      "Only the asset issuer can issue new units of an asset." );
   FC_ASSERT( !asset_obj.is_market_issued(),
      "Cannot manually issue ${s} because it is a market-issued asset.",
      ("s", o.asset_to_issue.symbol) );
   FC_ASSERT( to_account_permissions.is_authorized_transfer( o.issue_to_account, asset_obj ),
      "The recipient account is not authorized to receive the asset being issued.");
   FC_ASSERT( ( asset_dyn_data.get_total_supply().amount + o.asset_to_issue.amount ) <= asset_obj.max_supply,
      "Issuing this amount would exceed the asset's maximum supply. Please raise the maximum supply, or reduce issuance amount." );

   if( asset_obj.asset_type == asset_property_type::BOND_ASSET )
   {
      const asset_bond_data_object& bond_obj = _db.get_bond_data( o.asset_to_issue.symbol );
      price collateral_price = price( asset( BLOCKCHAIN_PRECISION, o.asset_to_issue.symbol ), bond_obj.value );
      asset bond_collateral = ( ( o.asset_to_issue * collateral_price ) * bond_obj.collateralization ) / PERCENT_100;
      asset liquid = _db.get_liquid_balance( o.issuer, bond_obj.value.symbol );
      FC_ASSERT( liquid >= bond_collateral,
         "Insufficient Collateral liquid balance to issue requested quantity of bond assets." );

      _db.adjust_liquid_balance( o.issuer, -bond_collateral );

      _db.modify( bond_obj, [&]( asset_bond_data_object& abdo )
      {
         abdo.collateral_pool += bond_collateral;
      });
   }
   else if( asset_obj.asset_type == asset_property_type::UNIQUE_ASSET )
   {
      FC_ASSERT( o.asset_to_issue.amount == BLOCKCHAIN_PRECISION,
         "Unique asset must be issued as a single unit asset." );
      const asset_unique_data_object& unique = _db.get_unique_data( asset_obj.symbol );

      _db.modify( unique, [&]( asset_unique_data_object& audo )
      {
         audo.controlling_owner = o.issue_to_account;
      });
   }

   _db.adjust_liquid_balance( o.issue_to_account, o.asset_to_issue );

   ilog( "Account: ${a} Issued new asset: ${am} to account: ${i} Current Liquid Supply: ${ls} Current Total Supply: ${ts}",
      ("a",o.issuer)("am",o.asset_to_issue.to_string() )("i",o.issue_to_account)("ls",asset_dyn_data.get_liquid_supply().to_string())("ts",asset_dyn_data.get_total_supply().to_string()));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void asset_reserve_evaluator::do_apply( const asset_reserve_operation& o )
{ try {
   const account_name_type& signed_for = o.payer;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& asset_obj = _db.get_asset( o.amount_to_reserve.symbol );
   const account_permission_object& from_account_permissions = _db.get_account_permissions( o.payer );
   const asset_dynamic_data_object& asset_dyn_data = _db.get_dynamic_data( asset_obj.symbol );
   asset liquid = _db.get_liquid_balance( o.payer, o.amount_to_reserve.symbol );

   FC_ASSERT( liquid >= o.amount_to_reserve,
      "Cannot reserve more of an asset than account's liquid balance. Required: ${r} Actual: ${a}",
      ("r",o.amount_to_reserve.to_string())("a",liquid.to_string()) );
   FC_ASSERT( !asset_obj.is_market_issued(),
      "Cannot reserve asset: ${s} because it is a market-issued asset",
      ("s",o.amount_to_reserve.symbol) );
   FC_ASSERT( from_account_permissions.is_authorized_transfer( o.payer, asset_obj ),
      "The recipient account is not authorized to reserve the asset.");
   FC_ASSERT( ( asset_dyn_data.get_total_supply().amount - o.amount_to_reserve.amount ) >= 0,
      "Cannot reserve more of an asset than its current total supply." );

   if( asset_obj.asset_type == asset_property_type::BOND_ASSET )
   {
      // Redeem bond for underlying collateral split
      const asset_bond_data_object& bond_obj = _db.get_bond_data( o.amount_to_reserve.symbol );
      price collateral_price = bond_obj.collateral_pool / asset_dyn_data.get_total_supply();
      asset bond_collateral = o.amount_to_reserve * collateral_price;
      
      _db.adjust_liquid_balance( o.payer, bond_collateral );

      _db.modify( bond_obj, [&]( asset_bond_data_object& abdo )
      {
         abdo.collateral_pool -= bond_collateral;
      });
   }
   else if( asset_obj.asset_type == asset_property_type::UNIQUE_ASSET )
   {
      FC_ASSERT( false,
         "Cannot reserve Unique Asset." );
   }

   _db.adjust_liquid_balance( o.payer, -o.amount_to_reserve );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void asset_update_issuer_evaluator::do_apply( const asset_update_issuer_operation& o )
{ try {
   const account_name_type& signed_for = o.issuer;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_chief( o.signatory ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& asset_obj = _db.get_asset( o.asset_to_update );
   const account_object& new_issuer_account = _db.get_account( o.new_issuer );

   FC_ASSERT( new_issuer_account.active, 
      "Account: ${s} must be active to become new issuer.",
      ("s",o.new_issuer));

   const account_permission_object& new_issuer_permissions = _db.get_account_permissions( o.new_issuer );
   const account_permission_object& issuer_permissions = _db.get_account_permissions( o.issuer );

   FC_ASSERT( !asset_obj.immutable_properties(),
      "Asset Has Immutable Properties and cannot be altered after creation." );
   FC_ASSERT( new_issuer_permissions.is_authorized_transfer( o.issuer, asset_obj ),
      "Asset Issuer Transfer is not authorized, due to recipient account's asset permisssions." );
   FC_ASSERT( issuer_permissions.is_authorized_transfer( o.new_issuer, asset_obj ),
      "Asset Issuer Transfer is not authorized, due to sender account's asset permisssions." );


   switch( asset_obj.asset_type )  // Asset specific requirements
   {
      case asset_property_type::CURRENCY_ASSET:
      {
         FC_ASSERT( false, "Cannot Edit Currency asset." );
      }
      break;
      case asset_property_type::STANDARD_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Current Asset Issuer: ${i} can update asset to new issuer.",
            ("i", asset_obj.issuer));
      }
      break;
      case asset_property_type::STABLECOIN_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Current Asset Issuer: ${i} can update asset to new issuer.",
            ("i", asset_obj.issuer));
      }
      break;
      case asset_property_type::EQUITY_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Current Asset Issuer: ${i} can update asset to new issuer.",
            ("i", asset_obj.issuer));

         const account_business_object& new_issuer_bus_acc = _db.get_account_business( o.new_issuer );

         FC_ASSERT( new_issuer_bus_acc.active,
            "Issuer: ${i} must have active business account.",
            ("i", o.new_issuer));

         const asset_equity_data_object& equity_obj = _db.get_equity_data( o.asset_to_update );

         _db.modify( equity_obj, [&]( asset_equity_data_object& a )
         {
            a.business_account = o.new_issuer;
         });
      }
      break;
      case asset_property_type::BOND_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Current Asset Issuer: ${i} can update asset to new issuer.",
            ("i", asset_obj.issuer));
            
         const account_business_object& new_issuer_bus_acc = _db.get_account_business( o.new_issuer );

         FC_ASSERT( new_issuer_bus_acc.active,
            "Issuer: ${i} must have active business account.",
            ("i", o.new_issuer));

         const asset_bond_data_object& bond_obj = _db.get_bond_data( o.asset_to_update );
         const asset_dynamic_data_object& bond_dyn_data = _db.get_dynamic_data( o.asset_to_update );

         FC_ASSERT( bond_dyn_data.get_total_supply().amount == 0, 
            "Cannot update Bond issuer while outstanding Supply exists. All bonds must mature before transfer." );
         
         _db.modify( bond_obj, [&]( asset_bond_data_object& a )
         {
            a.business_account = o.new_issuer;
         });
      }
      break;
      case asset_property_type::CREDIT_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Current Asset Issuer: ${i} can update asset to new issuer.",
            ("i", asset_obj.issuer));

         const account_business_object& new_issuer_bus_acc = _db.get_account_business( o.new_issuer );

         FC_ASSERT( new_issuer_bus_acc.active,
            "Issuer: ${i} must have active business account.",
            ("i", o.new_issuer));

         const asset_credit_data_object& credit_obj = _db.get_credit_data( o.asset_to_update );
         const asset_dynamic_data_object& credit_dyn_data = _db.get_dynamic_data( o.asset_to_update );

         FC_ASSERT( credit_dyn_data.get_total_supply().amount == 0, 
            "Cannot update Credit issuer while outstanding Supply exists. All Credits must be repurchased before transfer." );

         _db.modify( credit_obj, [&]( asset_credit_data_object& a )
         {
            a.business_account = o.new_issuer;
         });
      }
      break;
      case asset_property_type::STIMULUS_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Current Asset Issuer: ${i} can update asset to new issuer.",
            ("i", asset_obj.issuer));

         const account_business_object& new_issuer_bus_acc = _db.get_account_business( o.new_issuer );

         FC_ASSERT( new_issuer_bus_acc.active,
            "Issuer: ${i} must have active business account.",
            ("i", o.new_issuer));

         const asset_stimulus_data_object& stimulus_obj = _db.get_stimulus_data( o.asset_to_update );
         
         _db.modify( stimulus_obj, [&]( asset_stimulus_data_object& a )
         {
            a.business_account = o.new_issuer;
         });
      }
      break;
      case asset_property_type::LIQUIDITY_POOL_ASSET:
      {
         FC_ASSERT( false, "Cannot Edit Liquidity Pool asset." );
      }
      break;
      case asset_property_type::CREDIT_POOL_ASSET:
      {
         FC_ASSERT( false, "Cannot Edit Credit pool asset." );
      }
      break;
      case asset_property_type::OPTION_ASSET:
      {
         FC_ASSERT( false, "Cannot Edit Option asset." );
      }
      break;
      case asset_property_type::UNIQUE_ASSET:
      {
         FC_ASSERT( o.issuer == asset_obj.issuer,
            "Only Current Asset Issuer: ${i} can update asset to new issuer.",
            ("i", asset_obj.issuer));
      }
      break;
      default:
      {
         FC_ASSERT( false, "Invalid Asset type." );
      }
      break;
   }

   _db.modify( asset_obj, [&]( asset_object& a ) 
   {
      a.issuer = o.new_issuer;
   });

   ilog( "Updated Asset Issuer: ${i}",
      ("i",o.new_issuer));

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void asset_distribution_evaluator::do_apply( const asset_distribution_operation& o )
{ try {
   const account_name_type& signed_for = o.issuer;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& distribution_asset = _db.get_asset( o.distribution_asset );
   const asset_object& fund_asset = _db.get_asset( o.fund_asset );
   time_point now = _db.head_block_time();

   FC_ASSERT( o.issuer == distribution_asset.issuer,
      "Asset Distribution can only be created by the asset issuer." );
   FC_ASSERT( fund_asset.symbol == o.fund_asset,
      "Fund Asset is not valid." );
   
   auto& distribution_idx =_db.get_index< asset_distribution_index >().indices().get< by_symbol >();
   auto distribution_itr = distribution_idx.find( o.distribution_asset );

   if( distribution_itr == distribution_idx.end() )
   {
      FC_ASSERT( o.begin_time >= ( now + fc::days(30) ),
         "Asset Distribution begin time must be at least 30 days in the future." );

      const asset_distribution_object& distribution = _db.create< asset_distribution_object >( [&]( asset_distribution_object& ado )
      {
         ado.distribution_asset = o.distribution_asset;
         ado.fund_asset = o.fund_asset;
         from_string( ado.details, o.details );
         from_string( ado.url, o.url );
         from_string( ado.json, o.json );
         ado.distribution_rounds = o.distribution_rounds;
         ado.distribution_interval_days = o.distribution_interval_days;
         ado.max_intervals_missed = o.max_intervals_missed;
         ado.intervals_paid = 0;
         ado.intervals_missed = 0;
         
         for( asset_unit a : o.input_fund_unit )
         {
            ado.input_fund_unit.insert( a );
         }
         for( asset_unit a : o.output_distribution_unit )
         {
            ado.output_distribution_unit.insert( a );
         }

         ado.min_input_fund_units = o.min_input_fund_units;
         ado.max_input_fund_units = o.max_input_fund_units;
         ado.min_unit_ratio = o.min_unit_ratio;
         ado.max_unit_ratio = o.max_unit_ratio;
         ado.min_input_balance_units = o.min_input_balance_units;
         ado.max_input_balance_units = o.max_input_balance_units;
         ado.total_distributed = asset( 0, o.distribution_asset );
         ado.total_funded = asset( 0, o.fund_asset );
         ado.begin_time = o.begin_time;
         ado.next_round_time = ado.begin_time + fc::days( ado.distribution_interval_days );
         ado.created = now;
         ado.last_updated = now;
      });

      ilog( "Account: ${a} created New Asset Distribution: \n ${d} \n",
         ("a",o.issuer)("d",distribution));
   }
   else
   {
      const asset_distribution_object& distribution = *distribution_itr;

      FC_ASSERT( distribution.begin_time >= now,
         "Asset Distribution properties cannot be changed after it has begun." );
      FC_ASSERT( o.begin_time >= distribution.begin_time,
         "Asset Distribution begin time cannot be brought forward." );

      _db.modify( distribution, [&]( asset_distribution_object& ado )
      {
         from_string( ado.details, o.details );
         from_string( ado.url, o.url );
         from_string( ado.json, o.json );
         ado.distribution_rounds = o.distribution_rounds;
         ado.distribution_interval_days = o.distribution_interval_days;
         ado.max_intervals_missed = o.max_intervals_missed;

         for( asset_unit a : o.input_fund_unit )
         {
            ado.input_fund_unit.insert( a );
         }
         for( asset_unit a : o.output_distribution_unit )
         {
            ado.output_distribution_unit.insert( a );
         }

         ado.min_input_fund_units = o.min_input_fund_units;
         ado.max_input_fund_units = o.max_input_fund_units;
         ado.min_unit_ratio = o.min_unit_ratio;
         ado.max_unit_ratio = o.max_unit_ratio;
         ado.min_input_balance_units = o.min_input_balance_units;
         ado.max_input_balance_units = o.max_input_balance_units;
         ado.begin_time = o.begin_time;
         ado.next_round_time = ado.begin_time + fc::days( ado.distribution_interval_days );
         ado.last_updated = now;
      });

      ilog( "Account: ${a} edited Asset Distribution: \n ${d} \n",
         ("a",o.issuer)("d",distribution));
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void asset_distribution_fund_evaluator::do_apply( const asset_distribution_fund_operation& o )
{ try {
   const account_name_type& signed_for = o.sender;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_distribution_object& distribution = _db.get_asset_distribution( o.distribution_asset );
   time_point now = _db.head_block_time();
   share_type input_units = o.amount.amount / distribution.input_unit_amount();

   FC_ASSERT( now >= distribution.begin_time,
      "Cannot create Distribution balance before begin time: ${t}.",
      ("t", distribution.begin_time) );
   FC_ASSERT( o.amount.symbol == distribution.fund_asset,
      "Asset Distribution accepts funding asset ${s}.",
      ("s", distribution.fund_asset) );
   FC_ASSERT( o.amount.amount % distribution.input_unit_amount() == 0,
      "Asset Distribution funding amount must be a multiple of the input asset unit: ${a} % ${u} == 0.",
      ("a", o.amount.amount )("u", distribution.input_unit_amount()) );
   FC_ASSERT( input_units >= distribution.min_input_balance_units,
      "Asset Distribution funding amount must be at least the minimum number of input asset units: ${a} >= ${u}.",
      ("a", input_units )("u", distribution.min_input_balance_units) );
   FC_ASSERT( input_units <= distribution.max_input_balance_units,
      "Asset Distribution funding amount must be at most the maximum number of input asset units: ${a} <= ${u}.",
      ("a", input_units )("u", distribution.max_input_balance_units) );

   asset liquid = _db.get_liquid_balance( o.sender, o.amount.symbol );
   
   auto& distribution_balance_idx =_db.get_index< asset_distribution_balance_index >().indices().get< by_account_distribution >();
   auto distribution_balance_itr = distribution_balance_idx.find( boost::make_tuple( o.sender, o.distribution_asset ) );

   if( distribution_balance_itr == distribution_balance_idx.end() )
   {
      FC_ASSERT( liquid >= o.amount,
         "Sender has insufficient liquid balance to create distribution balance." );

      _db.adjust_liquid_balance( o.sender, -o.amount );
      _db.adjust_pending_supply( o.amount );

      const asset_distribution_balance_object& balance = _db.create< asset_distribution_balance_object >( [&]( asset_distribution_balance_object& a )
      {
         a.sender = o.sender;
         a.distribution_asset = o.distribution_asset;
         a.amount = o.amount;
         a.created = now;
         a.last_updated = now;
      });

      ilog( "Account: ${a} created New Asset Distribution Balance: \n ${b} \n",
         ("a",o.sender)("b",balance));
   }
   else
   {
      const asset_distribution_balance_object& balance = *distribution_balance_itr;

      asset delta = o.amount - balance.amount;

      FC_ASSERT( liquid >= delta,
         "Sender has insufficient liquid balance to create distribution balance." );

      _db.adjust_liquid_balance( o.sender, -delta );
      _db.adjust_pending_supply( delta );

      if( o.amount.amount.value != 0 )
      {
         _db.modify( balance, [&]( asset_distribution_balance_object& a )
         {
            a.amount = o.amount;
            a.last_updated = now;
         });
      }
      else
      {
         ilog( "Removed: ${v}",("v",balance));
         _db.remove( balance );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void asset_option_exercise_evaluator::do_apply( const asset_option_exercise_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& option_asset = _db.get_asset( o.amount.symbol );
   
   FC_ASSERT( option_asset.asset_type == asset_property_type::OPTION_ASSET, 
      "Can only exercise option type assets: ${s} is not an option.",("s", o.amount.symbol ) );

   const account_object& account = _db.get_account( o.account );
   asset liquid = _db.get_liquid_balance( o.account, option_asset.symbol );

   FC_ASSERT( liquid >= o.amount,
      "Account has insufficient liquid balance of option asset to exercise specified amount." );

   _db.exercise_option( o.amount, account );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void asset_stimulus_fund_evaluator::do_apply( const asset_stimulus_fund_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& stimulus_asset = _db.get_asset( o.stimulus_asset );
   const asset_object& redemption_asset = _db.get_asset( o.amount.symbol );

   FC_ASSERT( stimulus_asset.asset_type == asset_property_type::STIMULUS_ASSET, 
      "Can only Fund Stimulus Assets: ${s} is not a stimulus asset.",
      ("s", o.stimulus_asset ) );
   FC_ASSERT( redemption_asset.asset_type == asset_property_type::CURRENCY_ASSET || 
      redemption_asset.asset_type == asset_property_type::STABLECOIN_ASSET, 
      "Redemption asset must be either a currency or stablecoin type asset." );

   const asset_stimulus_data_object& stimulus = _db.get_stimulus_data( o.stimulus_asset );

   asset liquid = _db.get_liquid_balance( o.account, redemption_asset.symbol );

   FC_ASSERT( liquid >= o.amount,
      "Account has insufficient liquid balance of option asset to exercise specified amount." );

   _db.adjust_liquid_balance( o.account, -o.amount );

   _db.modify( stimulus, [&]( asset_stimulus_data_object& asdo )
   {
      asdo.adjust_pool( o.amount );
   });
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void asset_update_feed_producers_evaluator::do_apply( const asset_update_feed_producers_operation& o )
{ try {
   const account_name_type& signed_for = o.issuer;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   time_point now = _db.head_block_time();
   const asset_object& asset_obj = _db.get_asset( o.asset_to_update );

   FC_ASSERT( asset_obj.asset_type == asset_property_type::STABLECOIN_ASSET,
      "Cannot Update Feed Producers for asset: ${s} Asset is not a Stablecoin.",
      ("s",o.asset_to_update) );

   const asset_stablecoin_data_object& stablecoin_to_update = _db.get_stablecoin_data( o.asset_to_update );
   
   FC_ASSERT( o.new_feed_producers.size() <= median_props.maximum_asset_feed_publishers,
      "Cannot specify more feed producers than maximum allowed." );
   FC_ASSERT( asset_obj.asset_type == asset_property_type::STABLECOIN_ASSET,
      "Cannot update feed producers on a non-BitAsset." );
   FC_ASSERT(!( asset_obj.is_producer_fed() ),
      "Cannot set feed producers on a producer-fed asset." );
   FC_ASSERT( asset_obj.issuer == o.issuer,
      "Only asset issuer can update feed producers of an asset" );
   
   for( account_name_type name : o.new_feed_producers )   // Make sure all producers exist. Check these after asset because account lookup is more expensive.
   {
      _db.get_account( name );
   }
   
   _db.modify( stablecoin_to_update, [&]( asset_stablecoin_data_object& abdo )
   {
      for( auto feed_itr = abdo.feeds.begin(); feed_itr != abdo.feeds.end(); )    // Remove any old publishers who are no longer publishers.
      {
         if( !o.new_feed_producers.count( feed_itr->first ) )
         {
            feed_itr = abdo.feeds.erase( feed_itr );     // Resets iterator to the new feeds feed_itr with the name's key removed.
         }
         else
         {
            ++feed_itr;
         }
      }
      for( const account_name_type name : o.new_feed_producers )    // Now, add map keys for any new publishers.
      {
         abdo.feeds[ name ];
      }
      abdo.update_median_feeds( now );
   });
   
   _db.check_call_orders( asset_obj, true, false );     // Process margin calls, allow black swan, not for a new limit order
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void asset_publish_feed_evaluator::do_apply( const asset_publish_feed_operation& o )
{ try {
   const account_name_type& signed_for = o.publisher;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
   const asset_object& base = _db.get_asset( o.symbol );     // Verify that this feed is for a market-issued asset and that asset is backed by the base.
   time_point now = _db.head_block_time();

   FC_ASSERT( base.asset_type == asset_property_type::STABLECOIN_ASSET,
      "Cannot Publish Price feed for asset: ${s} Asset is not a Stablecoin.",
      ("s",o.symbol) );

   const asset_stablecoin_data_object& stablecoin = _db.get_stablecoin_data( base.symbol );

   // The settlement price must be quoted in terms of the backing asset.

   FC_ASSERT( o.feed.settlement_price.quote.symbol == stablecoin.backing_asset,
      "Quote asset type in settlement price should be same as backing asset of this asset" );
   
   if( base.is_producer_fed() )     // Verify that the publisher is authoritative to publish a feed.
   {
      const account_authority_object& auth = _db.get_account_authority( PRODUCER_ACCOUNT );
      FC_ASSERT( auth.active_auth.account_auths.count( o.publisher ),
         "Asset: ${a} is a producer fed asset, and Account: ${ac} is not an active producer. Current active producer authorities are: \n ${p} \n",
         ("a",o.symbol)("ac",o.publisher)("p",auth.active_auth.account_auths) );
   }
   else
   {
      FC_ASSERT( stablecoin.feeds.count( o.publisher ),
         "Asset: ${a} is a stablecoin and Account: ${ac} is not an active publisher. Current active publisher feeds are: \n ${p} \n",
         ("a",o.symbol)("ac",o.publisher)("p",stablecoin.feeds) );
   }

   price_feed old_feed =  stablecoin.current_feed;    // Store medians for this asset.
   
   _db.modify( stablecoin, [&o,now]( asset_stablecoin_data_object& asdo )
   {
      asdo.feeds[ o.publisher ] = make_pair( now, o.feed );
      asdo.update_median_feeds( now );
   });

   if( !( old_feed == stablecoin.current_feed ) )
   {
      // Check whether need to revive the asset and proceed if need
      if( stablecoin.has_settlement() && !stablecoin.current_feed.settlement_price.is_null() ) // has globally settled and has a valid feed
      {
         bool should_revive = false;
         const asset_dynamic_data_object& mia_dyn = _db.get_dynamic_data( base.symbol );
         if( mia_dyn.get_total_supply().amount == 0 ) // If current supply is zero, revive the asset
         {
            should_revive = true;
         }
         else // if current supply is not zero, when collateral ratio of settlement fund is greater than MCR, revive the asset.
         {
            // calculate collateralization and compare to maintenance_collateralization
            if( price( stablecoin.settlement_fund, mia_dyn.get_total_supply() ) > stablecoin.current_maintenance_collateralization )
            {
               should_revive = true;
            }
         }
         if( should_revive )
         {
            _db.revive_stablecoin( base );
         } 
      }
      _db.check_call_orders( base, true, false );    // Process margin calls, allow black swan, not for a new limit order
   }

   ilog("Account: ${a} published price feed: ${f}",
      ("a",o.publisher)("f",o.feed.settlement_price.to_string()));
} FC_CAPTURE_AND_RETHROW(( o )) }


void asset_settle_evaluator::do_apply( const asset_settle_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& asset_to_settle = _db.get_asset( o.amount.symbol );
   const asset_dynamic_data_object& mia_dyn = _db.get_dynamic_data( o.amount.symbol );
   time_point now = _db.head_block_time();

   FC_ASSERT( asset_to_settle.asset_type == asset_property_type::STABLECOIN_ASSET,
      "Cannot Settle asset: ${s} Asset is not a Stablecoin.",
      ("s",o.amount.symbol) );

   const asset_stablecoin_data_object& stablecoin = _db.get_stablecoin_data( o.amount.symbol );

   FC_ASSERT( asset_to_settle.enable_force_settle() || stablecoin.has_settlement(),
      "Asset must be able to be force settled, or have a global settlement price to settle." );

   asset liquid = _db.get_liquid_balance( o.account, asset_to_settle.symbol );
   
   FC_ASSERT( liquid >= o.amount,
      "Account does not have enough of the asset to settle the requested amount. Required: ${r} Actual: ${a}",
      ("r",o.amount.to_string())("a",liquid.to_string()) );

   if( stablecoin.has_settlement() )   // Asset has been globally settled. 
   {
      asset settled_amount = o.amount * stablecoin.settlement_price;     // Round down, in favor of global settlement fund.

      if( o.amount == mia_dyn.get_total_supply() )     // Settling the entire asset remaining supply. 
      {
         settled_amount.amount = stablecoin.settlement_fund;   // Set the settled amount to the exact remaining supply. 
      }
      else
      {
         FC_ASSERT( settled_amount.amount <= stablecoin.settlement_fund,
            "Settled amount should be less than or equal to settlement fund." );         // should be strictly < except for PM with zero outcome
      }
         
      FC_ASSERT( settled_amount.amount != 0, 
         "Rounding issue: Settlement amount should not be zero." );
      
      asset pays = o.amount;
      if( o.amount != mia_dyn.get_total_supply() && settled_amount.amount != 0 )
      {
         pays = settled_amount.multiply_and_round_up( stablecoin.settlement_price );
      }

      _db.adjust_liquid_balance( o.account, -pays );

      if( settled_amount.amount > 0 )     // Transfer from global settlement fund to account. 
      {
         _db.modify( stablecoin, [&]( asset_stablecoin_data_object& abdo )  
         {
            abdo.settlement_fund -= settled_amount.amount;
         });

         _db.adjust_liquid_balance( o.account, settled_amount );
         _db.adjust_pending_supply( -settled_amount );
      }
   }
   else    // Not globally settled, creating force settlement. 
   {
      const auto& settle_idx = _db.get_index< asset_settlement_index >().indices().get< by_account_asset >();
      auto settle_itr = settle_idx.find( boost::make_tuple( o.account, o.amount.symbol ) );

      time_point settlement_time = now + stablecoin.asset_settlement_delay;

      if( settle_itr == settle_idx.end() )
      {
         FC_ASSERT( o.amount.amount > 0,
            "Amount to settle must be greater than zero when creating new settlement order." );

         _db.adjust_liquid_balance( o.account, -o.amount );
         _db.adjust_pending_supply( o.amount );

         const asset_settlement_object& settle = _db.create< asset_settlement_object >([&]( asset_settlement_object& fso ) 
         {
            fso.owner = o.account;
            fso.balance = o.amount;
            fso.settlement_date = settlement_time;
            fso.interface = o.interface;
            fso.created = now;
            fso.last_updated = now;
         });

         ilog( "Account: ${a} Created new asset settlement order: ${o}",
            ("a",o.account)("o",settle.to_string()) );
      }
      else
      {
         const asset_settlement_object& settle = *settle_itr;
         asset delta = o.amount - settle.balance;

         _db.adjust_liquid_balance( o.account, -delta );
         _db.adjust_pending_supply( delta );

         if( o.amount.amount == 0 )
         {
            ilog( "Removed: ${v}",("v",settle));
            _db.remove( settle );
         }
         else
         {
            _db.modify( settle, [&]( asset_settlement_object& fso )
            {
               fso.balance = o.amount;
               fso.settlement_date = settlement_time;
               fso.last_updated = now;
            });

            ilog( "Account: ${a} edited asset settlement order: ${o}",
               ("a",o.account)("o",settle.to_string()) );
         }
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void asset_global_settle_evaluator::do_apply( const asset_global_settle_operation& o )
{ try {
   const account_name_type& signed_for = o.issuer;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& asset_to_settle = _db.get_asset( o.asset_to_settle );

   FC_ASSERT( asset_to_settle.asset_type == asset_property_type::STABLECOIN_ASSET,
      "Cannot Globally settle asset: ${s} Asset is not a Stablecoin.",
      ("s",o.asset_to_settle) );
   FC_ASSERT( asset_to_settle.can_global_settle(),
      "The global_settle permission of this asset is disabled." );
   FC_ASSERT( asset_to_settle.issuer == o.issuer,
      "Only asset issuer can globally settle an asset." );

   const asset_dynamic_data_object& mia_dyn = _db.get_dynamic_data( o.asset_to_settle );
   const asset_stablecoin_data_object& stablecoin = _db.get_stablecoin_data( o.asset_to_settle );

   FC_ASSERT( mia_dyn.get_total_supply().amount > 0,
      "Cannot globally settle an asset with zero supply." );
   FC_ASSERT( !stablecoin.has_settlement(),
      "This asset has already been globally settled, cannot global settle again." );

   const auto& call_idx = _db.get_index< call_order_index >().indices().get< by_collateral >();

   FC_ASSERT( !call_idx.empty(), 
      "Critical error: No debt positions found on entire network. Stablecoin supply should not exist." );

   auto call_itr = call_idx.lower_bound( price::min( stablecoin.backing_asset, o.asset_to_settle ) );

   FC_ASSERT( call_itr != call_idx.end() && call_itr->debt_type() == o.asset_to_settle,
      "Critical error: No debt positions found for the asset being settled. Stablecoin supply should not exist." );

   const call_order_object& least_collateralized_short = *call_itr;

   FC_ASSERT( least_collateralized_short.debt * o.settle_price <= least_collateralized_short.collateral,
      "Cannot force settle at supplied price: Least collateralized short lacks sufficient collateral to settle. Please lower the price");

   _db.globally_settle_asset( asset_to_settle, o.settle_price );
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void asset_collateral_bid_evaluator::do_apply( const asset_collateral_bid_operation& o )
{ try {
   const account_name_type& signed_for = o.bidder;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& debt_asset = _db.get_asset( o.debt.symbol );
   const asset_stablecoin_data_object& stablecoin_data = _db.get_stablecoin_data( debt_asset.symbol );
   const asset& liquid = _db.get_liquid_balance( o.bidder, stablecoin_data.backing_asset );

   time_point now = _db.head_block_time();

   FC_ASSERT( debt_asset.is_market_issued(),
      "Unable to cover ${sym} as it is not a collateralized asset.", ("sym", debt_asset.symbol) );
   FC_ASSERT( stablecoin_data.has_settlement(),
      "Asset being bidded on must have a settlement price.");
   FC_ASSERT( o.collateral.symbol == stablecoin_data.backing_asset,
      "Additional collateral must be the same asset as backing asset.");

   if( o.collateral.amount > 0 )
   {
      FC_ASSERT( liquid >= o.collateral,
         "Cannot bid ${c} collateral when payer only has ${b}", 
         ("c", o.collateral.amount)("b", liquid ) );
   }

   const auto& bid_idx = _db.get_index< asset_collateral_bid_index >().indices().get< by_account >();
   const auto& bid_itr = bid_idx.find( boost::make_tuple( o.bidder, o.debt.symbol ) );

   if( bid_itr == bid_idx.end() )    // No bid exists
   {
      FC_ASSERT( o.debt.amount > 0,
         "No collateral bid found." );
      
      _db.adjust_liquid_balance( o.bidder, -o.collateral );

      _db.create< asset_collateral_bid_object >([&]( asset_collateral_bid_object& b )
      {
         b.bidder = o.bidder;
         b.collateral = o.collateral;
         b.debt = o.debt;
         b.last_updated = now;
         b.created = now;
      });
   }
   else
   {
      const asset_collateral_bid_object& bid = *bid_itr;

      asset delta_collateral = o.collateral - bid.collateral;

      _db.adjust_liquid_balance( o.bidder, -delta_collateral );

      if( o.debt.amount > 0 )     // Editing bid
      {
         _db.modify( bid, [&]( asset_collateral_bid_object& b )
         {
            b.collateral = o.collateral;
            b.debt = o.debt;
            b.last_updated = now;
         });
      }
      else     // Removing bid
      {
         _db.cancel_bid( bid );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain