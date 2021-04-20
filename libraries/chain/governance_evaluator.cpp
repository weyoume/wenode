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


//===============================//
// === Governance Evaluators === //
//===============================//


void governance_create_evaluator::do_apply( const governance_create_operation& o )
{ try {
   const account_object& founder = _db.get_account( o.founder );
   FC_ASSERT( founder.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.founder) );
   FC_ASSERT( founder.membership == membership_tier_type::TOP_MEMBERSHIP,
      "Founder Account must be a Top level member to create a new Governance Account" );
   
   time_point now = _db.head_block_time();

   _db.check_namespace( o.new_governance_name );
   _db.check_namespace( o.public_community );
   _db.check_namespace( o.private_community );
   _db.check_namespace( o.equity_asset );
   _db.check_namespace( o.credit_asset );

   FC_ASSERT( ( founder.last_asset_created + MIN_ASSET_CREATE_INTERVAL ) <= now, 
      "Can only create one asset per hour. Please try again later." );

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }
   
   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   size_t name_length = o.new_governance_name.size();
   asset acc_fee = median_props.account_creation_fee;

   if( is_premium_account_name( o.new_governance_name ) )    // Double fee per character less than 8 characters.
   {
      acc_fee.amount = share_type( acc_fee.amount.value << uint16_t( 8 - name_length ) );
   }
   
   FC_ASSERT( o.fee >= asset( acc_fee.amount, SYMBOL_COIN ), 
      "Insufficient Fee: ${f} required, ${p} provided.",
      ("f", acc_fee )("p", o.fee) );
   const auto& founder_balance = _db.get_account_balance( o.founder, SYMBOL_COIN );
   FC_ASSERT( founder_balance.get_liquid_balance() >= o.fee, 
      "Insufficient balance: ${b} of founder ${r} to create account: ${a} with fee: ${f}.", 
      ( "b", founder_balance.liquid_balance )("r", o.founder)("a", o.new_governance_name)("f", o.fee) );

   FC_ASSERT( founder_balance.staked_balance - founder_balance.delegated_balance - founder_balance.to_unstake + founder_balance.total_unstaked >= o.delegation.amount, 
      "Insufficient Stake to delegate to new account.",
      ( "founder_balance.staked_balance", founder_balance.staked_balance )
      ( "founder_balance.delegated_balance", founder_balance.delegated_balance )( "required", o.delegation ) );

   auto target_delegation = asset( acc_fee.amount * CREATE_ACCOUNT_DELEGATION_RATIO, SYMBOL_COIN );
   auto current_delegation = asset( o.fee.amount * CREATE_ACCOUNT_DELEGATION_RATIO, SYMBOL_COIN ) + o.delegation;

   FC_ASSERT( current_delegation >= target_delegation,
      "Insufficient Delegation ${f} required, ${p} provided.",
      ("f", target_delegation )
      ( "p", current_delegation )
      ( "account_creation_fee", acc_fee )
      ( "o.fee", o.fee )
      ( "o.delegation", o.delegation ) );
   
   _db.adjust_liquid_balance( founder.name, -o.fee );
   _db.adjust_delegated_balance( founder.name, o.delegation );

   const account_object& new_account = _db.create< account_object >( [&]( account_object& a )
   {
      a.name = o.new_governance_name;
      a.registrar = founder.name;
      a.referrer = founder.name;
      a.proxy = founder.name;
      a.recovery_account = founder.name;
      a.reset_account = founder.name;
      
      from_string( a.details, o.details );
      from_string( a.url, o.url );
      from_string( a.profile_image, o.profile_image );
      from_string( a.cover_image, o.cover_image );
      
      a.membership = membership_tier_type::NONE;

      if( o.secure_public_key.size() )
      {
         a.secure_public_key = public_key_type( o.secure_public_key );
      }
      if( o.connection_public_key.size() )
      {  
         a.connection_public_key = public_key_type( o.connection_public_key );
      }
      if( o.friend_public_key.size() )
      {
         a.friend_public_key = public_key_type( o.friend_public_key );
      }
      if( o.companion_public_key.size() )
      {
         a.companion_public_key = public_key_type( o.companion_public_key );
      }

      a.created = now;
      a.last_updated = now;
      a.last_vote_time = now;
      a.last_view_time = now;
      a.last_share_time = now;
      a.last_post = now;
      a.last_root_post = now;
      a.last_transfer_time = now;
      a.last_activity_reward = now;
      a.last_account_recovery = now;
      a.last_community_created = now;
      a.last_asset_created = now;
      a.membership_expiration = time_point::maximum();

      a.voting_power = PERCENT_100;
      a.viewing_power = PERCENT_100;
      a.sharing_power = PERCENT_100;
      a.commenting_power = PERCENT_100;

      a.mined = false;
      a.can_vote = true;
      a.revenue_share = true;
      a.active = true;
   });

   if( o.fee.amount > 0 )
   {
      _db.adjust_staked_balance( o.new_governance_name, o.fee );
   }

   if( o.delegation.amount > 0 )
   {
      _db.adjust_receiving_balance( o.new_governance_name, o.delegation );

      _db.create< asset_delegation_object >( [&]( asset_delegation_object& vdo )
      {
         vdo.delegator = o.founder;
         vdo.delegatee = o.new_governance_name;
         vdo.amount = o.delegation;
      });
   }

   const account_authority_object& account_authority = _db.create< account_authority_object >( [&]( account_authority_object& aao )
   {
      aao.account = o.new_governance_name;
      aao.owner_auth.add_authority( o.founder, 1 );
      aao.owner_auth.weight_threshold = 1;
      aao.active_auth.add_authority( o.founder, 1 );
      aao.active_auth.weight_threshold = 1;
      aao.posting_auth.add_authority( o.founder, 1 );
      aao.posting_auth.weight_threshold = 1;
      aao.last_owner_update = now;
   });

   _db.create< account_following_object >( [&]( account_following_object& afo )
   {
      afo.account = o.new_governance_name;
      afo.last_updated = now;
   });

   _db.create< account_permission_object >( [&]( account_permission_object& aao )
   {
      aao.account = o.new_governance_name;
   });

   if( _db.find_producer( o.founder ) != nullptr )
   {
      _db.create< producer_vote_object >( [&]( producer_vote_object& pvo )
      {
         pvo.producer = o.founder;
         pvo.account = o.new_governance_name;
         pvo.vote_rank = 1;
         pvo.last_updated = now;
         pvo.created = now;
      });

      _db.modify( new_account, [&]( account_object& a )
      {
         a.producer_vote_count++;
      });
   }

   if( _db.find_network_officer( o.founder ) != nullptr )
   {
      _db.create< network_officer_vote_object >( [&]( network_officer_vote_object& novo )
      {
         novo.network_officer = o.founder;
         novo.account = o.new_governance_name;
         novo.vote_rank = 1;
         novo.last_updated = now;
         novo.created = now;
      });

      _db.modify( new_account, [&]( account_object& a )
      {
         a.officer_vote_count++;
      });
   }

   ilog( "Founder: ${r} created new account: \n ${a} \n ${auth} \n",
      ("r",o.founder)("a",new_account)("auth",account_authority));

   // Equity Asset Checks
   
   asset liquid_coin = _db.get_liquid_balance( o.founder, SYMBOL_COIN );
   asset liquid_usd = _db.get_liquid_balance( o.founder, SYMBOL_USD );

   size_t asset_length = o.equity_asset.size();
   asset coin_liq = median_props.asset_coin_liquidity;
   asset usd_liq = median_props.asset_usd_liquidity;

   if( is_premium_account_name( o.equity_asset ) )    // Double fee per character less than 8 characters.
   {
      coin_liq.amount = share_type( coin_liq.amount.value << uint16_t( 8 - asset_length ) );
      usd_liq.amount = share_type( usd_liq.amount.value << uint16_t( 8 - asset_length ) );
   }

   FC_ASSERT( o.equity_options.whitelist_authorities.size() <= median_props.maximum_asset_whitelist_authorities,
      "Too many Whitelist authorities." );
   FC_ASSERT( o.equity_options.blacklist_authorities.size() <= median_props.maximum_asset_whitelist_authorities,
      "Too many Blacklist authorities." );
   FC_ASSERT( o.equity_options.stake_intervals <= median_props.max_stake_intervals && o.equity_options.stake_intervals >= 0, 
      "Asset stake intervals outside of acceptable limits." );
   FC_ASSERT( o.equity_options.unstake_intervals <= median_props.max_unstake_intervals && o.equity_options.unstake_intervals >= 0, 
      "Asset unstake intervals outside of acceptable limits." );

   for( auto account : o.equity_options.whitelist_authorities )
   {
      _db.get_account( account );    // Check that all authorities are valid accounts
   }
     
   for( auto account : o.equity_options.blacklist_authorities )
   {
      _db.get_account( account );
   }
     
   string asset_string = o.equity_asset;
   auto dotpos = asset_string.find( '.' );

   if( dotpos != std::string::npos )
   {
      auto prefix = asset_string.substr( 0, dotpos );
      const asset_object* asset_symbol_ptr = _db.find_asset( prefix );

      FC_ASSERT( asset_symbol_ptr != nullptr,
         "Asset: ${s} may only be created by issuer of ${p}, but ${p} has not been registered.",
         ("s",o.equity_asset)("p",prefix) );

      FC_ASSERT( asset_symbol_ptr->issuer == o.founder,
         "Asset: ${s} may only be created by issuer of ${p}, ${i}.",
         ("s",o.equity_asset)("p",prefix)("i", o.founder) );
   }

   // Make Equity Asset

   FC_ASSERT( o.coin_liquidity >= coin_liq,
      "Asset has insufficient initial COIN liquidity." );
   FC_ASSERT( o.usd_liquidity >= usd_liq,
      "Asset has insufficient initial USD liquidity." );
   FC_ASSERT( liquid_coin >= o.coin_liquidity,
      "Issuer has insufficient coin balance to provide specified initial liquidity." );
   FC_ASSERT( liquid_usd >= o.usd_liquidity,
      "Issuer has insufficient USD balance to provide specified initial liquidity." );

   const asset_equity_data_object& equity = _db.create< asset_equity_data_object >( [&]( asset_equity_data_object& a )
   {
      a.issuer = o.new_governance_name;
      a.symbol = o.equity_asset;
      a.last_dividend = time_point::min();
      a.dividend_share_percent = o.equity_options.dividend_share_percent;
      a.min_active_time = o.equity_options.min_active_time;
      a.min_balance = o.equity_options.min_balance;
      a.min_producers = o.equity_options.min_producers;
      a.boost_balance = o.equity_options.boost_balance;
      a.boost_activity = o.equity_options.boost_activity;
      a.boost_producers = o.equity_options.boost_producers;
      a.boost_top = o.equity_options.boost_top;
   });

   const asset_object equity_asset = _db.create< asset_object >( [&]( asset_object& a )
   {
      a.symbol = o.equity_asset;
      a.asset_type = asset_property_type::EQUITY_ASSET;
      a.issuer = o.new_governance_name;
      from_string( a.display_symbol, o.equity_options.display_symbol );
      from_string( a.details, o.equity_options.details );
      from_string( a.json, o.equity_options.json );
      from_string( a.url, o.equity_options.url );
      a.max_supply = o.equity_options.max_supply;
      a.stake_intervals = o.equity_options.stake_intervals;
      a.unstake_intervals = o.equity_options.unstake_intervals;
      a.market_fee_percent = o.equity_options.market_fee_percent;
      a.market_fee_share_percent = o.equity_options.market_fee_share_percent;
      a.issuer_permissions = o.equity_options.issuer_permissions;
      a.flags = o.equity_options.flags;

      for( account_name_type auth : o.equity_options.whitelist_authorities )
      {
         a.whitelist_authorities.insert( auth );
      }
      for( account_name_type auth : o.equity_options.blacklist_authorities )
      {
         a.blacklist_authorities.insert( auth );
      }
      for( asset_symbol_type mar : o.equity_options.whitelist_markets )
      {
         a.whitelist_markets.insert( mar );
      }
      for( asset_symbol_type mar : o.equity_options.blacklist_markets )
      {
         a.blacklist_markets.insert( mar );
      }
      
      a.created = now;
      a.last_updated = now;
   });

   _db.create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = o.equity_asset;
   });

   asset_symbol_type core_liq_symbol = string( LIQUIDITY_ASSET_PREFIX )+ string( SYMBOL_COIN )+"."+ string( o.equity_asset );
   
   _db.create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = o.new_governance_name;
      a.symbol = core_liq_symbol;
      a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;    // Create the core liquidity pool for the new asset.
      from_string( a.display_symbol, core_liq_symbol );
      from_string( a.details, o.equity_options.details );
      from_string( a.json, o.equity_options.json );
      from_string( a.url, o.equity_options.url );
      a.max_supply = o.equity_options.max_supply;
      a.stake_intervals = o.equity_options.stake_intervals;
      a.unstake_intervals = o.equity_options.unstake_intervals;
      a.market_fee_percent = o.equity_options.market_fee_percent;
      a.market_fee_share_percent = o.equity_options.market_fee_share_percent;
      a.issuer_permissions = o.equity_options.issuer_permissions;
      a.flags = o.equity_options.flags;

      for( account_name_type auth : o.equity_options.whitelist_authorities )
      {
         a.whitelist_authorities.insert( auth );
      }
      for( account_name_type auth : o.equity_options.blacklist_authorities )
      {
         a.blacklist_authorities.insert( auth );
      }
      for( asset_symbol_type mar : o.equity_options.whitelist_markets )
      {
         a.whitelist_markets.insert( mar );
      }
      for( asset_symbol_type mar : o.equity_options.blacklist_markets )
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

   asset init_new_asset = asset( o.coin_liquidity.amount, o.equity_asset );        // Creates initial new asset supply equivalent to core liquidity. 
   asset init_liquid_asset = asset( o.coin_liquidity.amount, core_liq_symbol );    // Creates equivalent supply of the liquidity pool asset for liquidity injection.
      
   _db.create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
   {
      a.symbol_a = SYMBOL_COIN;
      a.symbol_b = o.equity_asset;
      a.symbol_liquid = core_liq_symbol;
      a.balance_a = o.coin_liquidity;
      a.balance_b = init_new_asset;
      a.hour_median_price = price( a.balance_a, a.balance_b );
      a.day_median_price = price( a.balance_a, a.balance_b );
      a.price_history.push_back( price( a.balance_a, a.balance_b ) );
      a.balance_liquid = init_liquid_asset;
   });

   _db.adjust_liquid_balance( o.founder, -o.coin_liquidity );
   _db.adjust_liquid_balance( o.founder, init_liquid_asset );
   _db.adjust_pending_supply( init_new_asset );

   asset_symbol_type usd_liq_symbol = string( LIQUIDITY_ASSET_PREFIX )+ string( SYMBOL_USD )+ "." + string( o.equity_asset );
   
   _db.create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = o.new_governance_name;
      a.symbol = usd_liq_symbol;
      a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;    // Create the USD liquidity pool for the new asset.
      from_string( a.display_symbol, usd_liq_symbol );
      from_string( a.details, o.equity_options.details );
      from_string( a.json, o.equity_options.json );
      from_string( a.url, o.equity_options.url );
      a.max_supply = o.equity_options.max_supply;
      a.stake_intervals = o.equity_options.stake_intervals;
      a.unstake_intervals = o.equity_options.unstake_intervals;
      a.market_fee_percent = o.equity_options.market_fee_percent;
      a.market_fee_share_percent = o.equity_options.market_fee_share_percent;
      a.issuer_permissions = o.equity_options.issuer_permissions;
      a.flags = o.equity_options.flags;

      for( account_name_type auth : o.equity_options.whitelist_authorities )
      {
         a.whitelist_authorities.insert( auth );
      }
      for( account_name_type auth : o.equity_options.blacklist_authorities )
      {
         a.blacklist_authorities.insert( auth );
      }
      for( asset_symbol_type mar : o.equity_options.whitelist_markets )
      {
         a.whitelist_markets.insert( mar );
      }
      for( asset_symbol_type mar : o.equity_options.blacklist_markets )
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

   init_new_asset = asset( o.usd_liquidity.amount, o.equity_asset );         // Creates initial new asset supply equivalent to core liquidity. 
   init_liquid_asset = asset( o.usd_liquidity.amount, usd_liq_symbol);       // Creates equivalent supply of the liquidity pool asset for liquidity injection.
      
   _db.create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
   {
      a.symbol_a = SYMBOL_USD;
      a.symbol_b = o.equity_asset;
      a.symbol_liquid = usd_liq_symbol;
      a.balance_a = o.usd_liquidity;
      a.balance_b = init_new_asset;
      a.hour_median_price = price( a.balance_a, a.balance_b );
      a.day_median_price = price( a.balance_a, a.balance_b );
      a.price_history.push_back( price( a.balance_a, a.balance_b ) );
      a.balance_liquid = init_liquid_asset;
   });

   _db.adjust_liquid_balance( o.founder, -o.usd_liquidity );
   _db.adjust_liquid_balance( o.founder, init_liquid_asset );
   _db.adjust_pending_supply( init_new_asset );

   asset_symbol_type credit_asset_symbol = string( CREDIT_ASSET_PREFIX ) + string( o.equity_asset );
   
   _db.create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = o.new_governance_name;
      a.symbol = credit_asset_symbol;
      a.asset_type = asset_property_type::CREDIT_POOL_ASSET; // Create the asset credit pool for the new asset.
      from_string( a.display_symbol, credit_asset_symbol );
      from_string( a.details, o.equity_options.details );
      from_string( a.json, o.equity_options.json );
      from_string( a.url, o.equity_options.url );
      a.max_supply = o.equity_options.max_supply;
      a.stake_intervals = o.equity_options.stake_intervals;
      a.unstake_intervals = o.equity_options.unstake_intervals;
      a.market_fee_percent = o.equity_options.market_fee_percent;
      a.market_fee_share_percent = o.equity_options.market_fee_share_percent;
      a.issuer_permissions = o.equity_options.issuer_permissions;
      a.flags = o.equity_options.flags;

      for( account_name_type auth : o.equity_options.whitelist_authorities )
      {
         a.whitelist_authorities.insert( auth );
      }
      for( account_name_type auth : o.equity_options.blacklist_authorities )
      {
         a.blacklist_authorities.insert( auth );
      }
      for( asset_symbol_type mar : o.equity_options.whitelist_markets )
      {
         a.whitelist_markets.insert( mar );
      }
      for( asset_symbol_type mar : o.equity_options.blacklist_markets )
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

   asset init_lent_asset = asset( o.credit_liquidity.amount, o.equity_asset );                       // Creates and lends equivalent new assets to the credit pool.
   asset init_credit_asset = asset( o.credit_liquidity.amount * 100, credit_asset_symbol );    // Creates equivalent credit pool assets and passes to issuer. 
   price init_credit_price = price( init_lent_asset, init_credit_asset );                      // Starts the initial credit asset exchange rate at 100:1.

   _db.create< asset_credit_pool_object >( [&]( asset_credit_pool_object& a )
   {
      a.base_symbol = o.equity_asset;   
      a.credit_symbol = credit_asset_symbol; 
      a.base_balance = init_lent_asset;
      a.borrowed_balance = asset( 0, o.equity_asset );
      a.credit_balance = init_credit_asset;
      a.last_price = init_credit_price;     // Initializes credit pool price with a ratio of 100:1
   });
   
   _db.adjust_pending_supply( init_lent_asset );
   _db.adjust_liquid_balance( o.founder, init_credit_asset );
   
   ilog( "Created Equity Asset: \n ${a} \n ${e} \n",("a",equity_asset)("e",equity));

   // Credit Asset Checks
   
   liquid_coin = _db.get_liquid_balance( o.founder, SYMBOL_COIN );
   liquid_usd = _db.get_liquid_balance( o.founder, SYMBOL_USD );

   asset_length = o.credit_asset.size();
   
   if( is_premium_account_name( o.credit_asset ) )    // Double fee per character less than 8 characters.
   {
      coin_liq.amount = share_type( coin_liq.amount.value << uint16_t( 8 - asset_length ) );
      usd_liq.amount = share_type( usd_liq.amount.value << uint16_t( 8 - asset_length ) );
   }

   FC_ASSERT( o.credit_options.whitelist_authorities.size() <= median_props.maximum_asset_whitelist_authorities,
      "Too many Whitelist authorities." );
   FC_ASSERT( o.credit_options.blacklist_authorities.size() <= median_props.maximum_asset_whitelist_authorities,
      "Too many Blacklist authorities." );
   FC_ASSERT( o.credit_options.stake_intervals <= median_props.max_stake_intervals && o.credit_options.stake_intervals >= 0, 
      "Asset stake intervals outside of acceptable limits." );
   FC_ASSERT( o.credit_options.unstake_intervals <= median_props.max_unstake_intervals && o.credit_options.unstake_intervals >= 0, 
      "Asset unstake intervals outside of acceptable limits." );

   for( auto account : o.credit_options.whitelist_authorities )
   {
      _db.get_account( account );    // Check that all authorities are valid accounts
   }
     
   for( auto account : o.credit_options.blacklist_authorities )
   {
      _db.get_account( account );
   }
     
   asset_string = o.credit_asset;
   dotpos = asset_string.find( '.' );

   if( dotpos != std::string::npos )
   {
      auto prefix = asset_string.substr( 0, dotpos );
      const asset_object* asset_symbol_ptr = _db.find_asset( prefix );

      FC_ASSERT( asset_symbol_ptr != nullptr,
         "Asset: ${s} may only be created by issuer of ${p}, but ${p} has not been registered.",
         ("s",o.credit_asset)("p",prefix) );

      FC_ASSERT( asset_symbol_ptr->issuer == o.founder,
         "Asset: ${s} may only be created by issuer of ${p}, ${i}.",
         ("s",o.credit_asset)("p",prefix)("i", o.founder) );
   }

   // Make Credit Asset

   FC_ASSERT( o.coin_liquidity >= coin_liq, 
      "Asset has insufficient initial COIN liquidity." );
   FC_ASSERT( o.usd_liquidity >= usd_liq, 
      "Asset has insufficient initial USD liquidity." );
   FC_ASSERT( liquid_coin >= o.coin_liquidity, 
      "Issuer has insufficient coin balance to provide specified initial liquidity." );
   FC_ASSERT( liquid_usd >= o.usd_liquidity, 
      "Issuer has insufficient USD balance to provide specified initial liquidity." );
   FC_ASSERT( !o.credit_options.buyback_price.is_null(),
      "Buyback price cannot be null." );
   FC_ASSERT( o.credit_options.buyback_price.base.symbol == o.credit_options.buyback_asset,
      "Buyback price must have buyback asset as base." );
   FC_ASSERT( o.credit_options.buyback_price.quote.symbol == o.credit_asset,
      "Buyback price must have credit asset as quote." );
   
   const asset_object& buyback_asset = _db.get_asset( o.credit_options.buyback_asset );

   FC_ASSERT( buyback_asset.asset_type == asset_property_type::CURRENCY_ASSET || 
      buyback_asset.asset_type == asset_property_type::STABLECOIN_ASSET, 
      "Buyback asset must be either a currency or stablecoin type asset." );

   const asset_credit_data_object& credit = _db.create< asset_credit_data_object >( [&]( asset_credit_data_object& a )
   {
      a.issuer = o.founder;
      a.symbol = o.credit_asset;
      a.buyback_asset = o.credit_options.buyback_asset;
      a.buyback_pool = asset( 0, a.buyback_asset );
      a.buyback_price = o.credit_options.buyback_price;
      a.last_buyback = time_point::min();
      a.buyback_share_percent = o.credit_options.buyback_share_percent;
      a.liquid_fixed_interest_rate = o.credit_options.liquid_fixed_interest_rate;
      a.liquid_variable_interest_rate = o.credit_options.liquid_variable_interest_rate;
      a.staked_fixed_interest_rate = o.credit_options.staked_fixed_interest_rate;
      a.staked_variable_interest_rate = o.credit_options.staked_variable_interest_rate;
      a.savings_fixed_interest_rate = o.credit_options.savings_fixed_interest_rate;
      a.savings_variable_interest_rate = o.credit_options.savings_variable_interest_rate;
      a.var_interest_range = o.credit_options.var_interest_range;
   });

   const asset_object credit_asset = _db.create< asset_object >( [&]( asset_object& a )
   {
      a.symbol = o.credit_asset;
      a.asset_type = asset_property_type::CREDIT_ASSET;
      a.issuer = o.new_governance_name;
      from_string( a.display_symbol, o.credit_options.display_symbol );
      from_string( a.details, o.credit_options.details );
      from_string( a.json, o.credit_options.json );
      from_string( a.url, o.credit_options.url );
      a.max_supply = o.credit_options.max_supply;
      a.stake_intervals = o.credit_options.stake_intervals;
      a.unstake_intervals = o.credit_options.unstake_intervals;
      a.market_fee_percent = o.credit_options.market_fee_percent;
      a.market_fee_share_percent = o.credit_options.market_fee_share_percent;
      a.issuer_permissions = o.credit_options.issuer_permissions;
      a.flags = o.credit_options.flags;

      for( account_name_type auth : o.credit_options.whitelist_authorities )
      {
         a.whitelist_authorities.insert( auth );
      }
      for( account_name_type auth : o.credit_options.blacklist_authorities )
      {
         a.blacklist_authorities.insert( auth );
      }
      for( asset_symbol_type mar : o.credit_options.whitelist_markets )
      {
         a.whitelist_markets.insert( mar );
      }
      for( asset_symbol_type mar : o.credit_options.blacklist_markets )
      {
         a.blacklist_markets.insert( mar );
      }
      
      a.created = now;
      a.last_updated = now;
   });

   _db.create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = o.credit_asset;
   });

   core_liq_symbol = string( LIQUIDITY_ASSET_PREFIX )+ string( SYMBOL_COIN )+"."+ string( o.credit_asset );
      
   _db.create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = o.new_governance_name;
      a.symbol = core_liq_symbol;
      a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;    // Create the core liquidity pool for the new asset.
      from_string( a.display_symbol, core_liq_symbol );
      from_string( a.details, o.credit_options.details );
      from_string( a.json, o.credit_options.json );
      from_string( a.url, o.credit_options.url );
      a.max_supply = o.credit_options.max_supply;
      a.stake_intervals = o.credit_options.stake_intervals;
      a.unstake_intervals = o.credit_options.unstake_intervals;
      a.market_fee_percent = o.credit_options.market_fee_percent;
      a.market_fee_share_percent = o.credit_options.market_fee_share_percent;
      a.issuer_permissions = o.credit_options.issuer_permissions;
      a.flags = o.credit_options.flags;

      for( account_name_type auth : o.credit_options.whitelist_authorities )
      {
         a.whitelist_authorities.insert( auth );
      }
      for( account_name_type auth : o.credit_options.blacklist_authorities )
      {
         a.blacklist_authorities.insert( auth );
      }
      for( asset_symbol_type mar : o.credit_options.whitelist_markets )
      {
         a.whitelist_markets.insert( mar );
      }
      for( asset_symbol_type mar : o.credit_options.blacklist_markets )
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

   init_new_asset = asset( o.coin_liquidity.amount, o.credit_asset );              // Creates initial new asset supply equivalent to core liquidity. 
   init_liquid_asset = asset( o.coin_liquidity.amount, core_liq_symbol );    // Creates equivalent supply of the liquidity pool asset for liquidity injection.
      
   _db.create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
   {
      a.symbol_a = SYMBOL_COIN;
      a.symbol_b = o.credit_asset;
      a.symbol_liquid = core_liq_symbol;
      a.balance_a = o.coin_liquidity;
      a.balance_b = init_new_asset;
      a.hour_median_price = price( a.balance_a, a.balance_b );
      a.day_median_price = price( a.balance_a, a.balance_b );
      a.price_history.push_back( price( a.balance_a, a.balance_b ) );
      a.balance_liquid = init_liquid_asset;
   });

   _db.adjust_liquid_balance( o.founder, -o.coin_liquidity );
   _db.adjust_liquid_balance( o.founder, init_liquid_asset );
   _db.adjust_pending_supply( init_new_asset );

   usd_liq_symbol = string( LIQUIDITY_ASSET_PREFIX )+ string( SYMBOL_USD )+ "." + string( o.credit_asset );
   
   _db.create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = o.new_governance_name;
      a.symbol = usd_liq_symbol;
      a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;    // Create the USD liquidity pool for the new asset.
      from_string( a.display_symbol, usd_liq_symbol );
      from_string( a.details, o.credit_options.details );
      from_string( a.json, o.credit_options.json );
      from_string( a.url, o.credit_options.url );
      a.max_supply = o.credit_options.max_supply;
      a.stake_intervals = o.credit_options.stake_intervals;
      a.unstake_intervals = o.credit_options.unstake_intervals;
      a.market_fee_percent = o.credit_options.market_fee_percent;
      a.market_fee_share_percent = o.credit_options.market_fee_share_percent;
      a.issuer_permissions = o.credit_options.issuer_permissions;
      a.flags = o.credit_options.flags;

      for( account_name_type auth : o.credit_options.whitelist_authorities )
      {
         a.whitelist_authorities.insert( auth );
      }
      for( account_name_type auth : o.credit_options.blacklist_authorities )
      {
         a.blacklist_authorities.insert( auth );
      }
      for( asset_symbol_type mar : o.credit_options.whitelist_markets )
      {
         a.whitelist_markets.insert( mar );
      }
      for( asset_symbol_type mar : o.credit_options.blacklist_markets )
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

   init_new_asset = asset( o.usd_liquidity.amount, o.credit_asset );           // Creates initial new asset supply equivalent to core liquidity. 
   init_liquid_asset = asset( o.usd_liquidity.amount, usd_liq_symbol);   // Creates equivalent supply of the liquidity pool asset for liquidity injection.
      
   _db.create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& a )
   {
      a.symbol_a = SYMBOL_USD;
      a.symbol_b = o.credit_asset;
      a.symbol_liquid = usd_liq_symbol;
      a.balance_a = o.usd_liquidity;
      a.balance_b = init_new_asset;
      a.hour_median_price = price( a.balance_a, a.balance_b );
      a.day_median_price = price( a.balance_a, a.balance_b );
      a.price_history.push_back( price( a.balance_a, a.balance_b ) );
      a.balance_liquid = init_liquid_asset;
   });

   _db.adjust_liquid_balance( o.founder, -o.usd_liquidity );
   _db.adjust_liquid_balance( o.founder, init_liquid_asset );
   _db.adjust_pending_supply( init_new_asset );

   credit_asset_symbol = string( CREDIT_ASSET_PREFIX ) + string( o.credit_asset );
   
   _db.create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = o.new_governance_name;
      a.symbol = credit_asset_symbol;
      a.asset_type = asset_property_type::CREDIT_POOL_ASSET; // Create the asset credit pool for the new asset.
      from_string( a.display_symbol, credit_asset_symbol );
      from_string( a.details, o.credit_options.details );
      from_string( a.json, o.credit_options.json );
      from_string( a.url, o.credit_options.url );
      a.max_supply = o.credit_options.max_supply;
      a.stake_intervals = o.credit_options.stake_intervals;
      a.unstake_intervals = o.credit_options.unstake_intervals;
      a.market_fee_percent = o.credit_options.market_fee_percent;
      a.market_fee_share_percent = o.credit_options.market_fee_share_percent;
      a.issuer_permissions = o.credit_options.issuer_permissions;
      a.flags = o.credit_options.flags;

      for( account_name_type auth : o.credit_options.whitelist_authorities )
      {
         a.whitelist_authorities.insert( auth );
      }
      for( account_name_type auth : o.credit_options.blacklist_authorities )
      {
         a.blacklist_authorities.insert( auth );
      }
      for( asset_symbol_type mar : o.credit_options.whitelist_markets )
      {
         a.whitelist_markets.insert( mar );
      }
      for( asset_symbol_type mar : o.credit_options.blacklist_markets )
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

   init_lent_asset = asset( o.credit_liquidity.amount, o.credit_asset );                       // Creates and lends equivalent new assets to the credit pool.
   init_credit_asset = asset( o.credit_liquidity.amount * 100, credit_asset_symbol );    // Creates equivalent credit pool assets and passes to issuer. 
   init_credit_price = price( init_lent_asset, init_credit_asset );                      // Starts the initial credit asset exchange rate at 100:1.

   _db.create< asset_credit_pool_object >( [&]( asset_credit_pool_object& a )
   {
      a.base_symbol = o.credit_asset;
      a.credit_symbol = credit_asset_symbol; 
      a.base_balance = init_lent_asset;
      a.borrowed_balance = asset( 0, o.credit_asset );
      a.credit_balance = init_credit_asset;
      a.last_price = init_credit_price;     // Initializes credit pool price with a ratio of 100:1
   });
   
   _db.adjust_pending_supply( init_lent_asset );
   _db.adjust_liquid_balance( o.founder, init_credit_asset );

   ilog( "Created Credit Asset: \n ${a} \n ${c} \n",("a",credit_asset)("c",credit));

   // Make Public Community

   FC_ASSERT( now >= founder.last_community_created + MIN_COMMUNITY_CREATE_INTERVAL,
      "Founder: ${f} can only create one community per day, try again after: ${t}.",
      ("f",o.founder)("t", founder.last_community_created + MIN_COMMUNITY_CREATE_INTERVAL) );

   const asset_object& reward_currency = _db.get_asset( o.reward_currency );
    FC_ASSERT( reward_currency.asset_type == asset_property_type::CURRENCY_ASSET,
      "Reward currency asset must be either a currency type asset." );
   
   community_permission_type author_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type reply_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type vote_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type view_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type share_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type message_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type poll_permission = community_permission_type::ADMIN_PERMISSION;
   community_permission_type event_permission = community_permission_type::ADMIN_PERMISSION;
   community_permission_type directive_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type add_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type request_permission = community_permission_type::ALL_PERMISSION;
   community_permission_type remove_permission = community_permission_type::ADMIN_PERMISSION;

   const community_object& new_public_community = _db.create< community_object >( [&]( community_object& co )
   {
      co.name = o.public_community;
      co.founder = o.founder;
      
      from_string( co.display_name, o.public_display_name );
      from_string( co.details, o.details );
      from_string( co.url, o.url );
      from_string( co.profile_image, o.profile_image );
      from_string( co.cover_image, o.cover_image );

      co.community_member_key = public_key_type( o.public_community_member_key );
      co.community_moderator_key = public_key_type( o.public_community_moderator_key );
      co.community_admin_key = public_key_type( o.public_community_admin_key );
      co.community_secure_key = public_key_type( o.public_community_secure_key );
      co.community_standard_premium_key = public_key_type( o.public_community_standard_premium_key );
      co.community_mid_premium_key = public_key_type( o.public_community_mid_premium_key );
      co.community_top_premium_key = public_key_type( o.public_community_top_premium_key );

      co.interface = o.interface;
      co.max_rating = 9;
      co.flags = 0;
      co.permissions = COMMUNITY_PERMISSION_MASK;
      co.reward_currency = o.reward_currency;
      co.standard_membership_price = o.standard_membership_price;
      co.mid_membership_price = o.mid_membership_price;
      co.top_membership_price = o.top_membership_price;
      co.created = now;
      co.last_updated = now;
      co.last_post = now;
      co.last_root_post = now;
      co.active = true;
   });

   const community_permission_object& new_public_community_permission = _db.create< community_permission_object >( [&]( community_permission_object& cmo )
   {
      cmo.name = o.public_community;
      cmo.founder = o.founder;

      cmo.add_subscriber( o.founder );
      cmo.add_member( o.founder );
      cmo.add_standard_premium_member( o.founder );
      cmo.add_mid_premium_member( o.founder );
      cmo.add_top_premium_member( o.founder );
      cmo.add_moderator( o.founder );
      cmo.add_administrator( o.founder );

      cmo.private_community = false;
      cmo.channel = false;
      cmo.author_permission = author_permission;
      cmo.reply_permission = reply_permission;
      cmo.vote_permission = vote_permission;
      cmo.view_permission = view_permission;
      cmo.share_permission = share_permission;
      cmo.message_permission = message_permission;
      cmo.poll_permission = poll_permission;
      cmo.event_permission = event_permission;
      cmo.directive_permission = directive_permission;
      cmo.add_permission = add_permission;
      cmo.request_permission = request_permission;
      cmo.remove_permission = remove_permission;

      cmo.min_verification_count = 0;
      cmo.max_verification_distance = 0;
      cmo.last_updated = now;
   });

   _db.create< community_member_vote_object >( [&]( community_member_vote_object& cmvo )
   {
      cmvo.member = o.founder;
      cmvo.account = o.founder;
      cmvo.community = o.public_community;
      cmvo.vote_rank = 1;
      cmvo.last_updated = now;
      cmvo.created = now;
   });

   _db.update_community_moderators( o.public_community );

   ilog( "Founder: ${f} created new Community: \n ${c} \n ${m} \n",
      ("f",o.founder)("c",new_public_community)("m",new_public_community_permission));

   const community_object& new_private_community = _db.create< community_object >( [&]( community_object& co )
   {
      co.name = o.private_community;
      co.founder = o.founder;
      
      from_string( co.display_name, o.private_display_name );
      from_string( co.details, o.details );
      from_string( co.url, o.url );
      from_string( co.profile_image, o.profile_image );
      from_string( co.cover_image, o.cover_image );

      co.community_member_key = public_key_type( o.private_community_member_key );
      co.community_moderator_key = public_key_type( o.private_community_moderator_key );
      co.community_admin_key = public_key_type( o.private_community_admin_key );
      co.community_secure_key = public_key_type( o.private_community_secure_key );
      co.community_standard_premium_key = public_key_type( o.private_community_standard_premium_key );
      co.community_mid_premium_key = public_key_type( o.private_community_mid_premium_key );
      co.community_top_premium_key = public_key_type( o.private_community_top_premium_key );

      co.interface = o.interface;
      co.max_rating = 9;
      co.flags = 0;
      co.permissions = COMMUNITY_PERMISSION_MASK;
      co.reward_currency = o.reward_currency;
      co.standard_membership_price = o.standard_membership_price;
      co.mid_membership_price = o.mid_membership_price;
      co.top_membership_price = o.top_membership_price;
      co.created = now;
      co.last_updated = now;
      co.last_post = now;
      co.last_root_post = now;
      co.active = true;
   });

   const community_permission_object& new_private_community_permission = _db.create< community_permission_object >( [&]( community_permission_object& cmo )
   {
      cmo.name = o.private_community;
      cmo.founder = o.founder;

      cmo.add_subscriber( o.founder );
      cmo.add_member( o.founder );
      cmo.add_standard_premium_member( o.founder );
      cmo.add_mid_premium_member( o.founder );
      cmo.add_top_premium_member( o.founder );
      cmo.add_moderator( o.founder );
      cmo.add_administrator( o.founder );

      cmo.private_community = true;
      cmo.channel = false;
      cmo.author_permission = author_permission;
      cmo.reply_permission = reply_permission;
      cmo.vote_permission = vote_permission;
      cmo.view_permission = view_permission;
      cmo.share_permission = share_permission;
      cmo.message_permission = message_permission;
      cmo.poll_permission = poll_permission;
      cmo.event_permission = event_permission;
      cmo.directive_permission = directive_permission;
      cmo.add_permission = add_permission;
      cmo.request_permission = request_permission;
      cmo.remove_permission = remove_permission;

      cmo.min_verification_count = 0;
      cmo.max_verification_distance = 0;
      cmo.last_updated = now;
   });

   _db.create< community_member_vote_object >( [&]( community_member_vote_object& cmvo )
   {
      cmvo.member = o.founder;
      cmvo.account = o.founder;
      cmvo.community = o.private_community;
      cmvo.vote_rank = 1;
      cmvo.last_updated = now;
      cmvo.created = now;
   });

   _db.update_community_moderators( o.private_community );

   ilog( "Founder: ${f} created new Community: \n ${c} \n ${m} \n",
      ("f",o.founder)("c",new_private_community)("m",new_private_community_permission) );

   const governance_object& governance = _db.create< governance_object >( [&]( governance_object& bo )
   {
      bo.account = o.new_governance_name;
      from_string( bo.governance_name, o.new_governance_display_name );
      bo.equity_asset = o.equity_asset;
      bo.equity_revenue_share = o.equity_revenue_share;
      bo.credit_asset = o.credit_asset;
      bo.credit_revenue_share = o.credit_revenue_share;
      bo.public_community = o.public_community;
      bo.private_community = o.private_community;
      bo.last_updated = now;
      bo.created = now;
   });

   const governance_permission_object& governance_permission = _db.create< governance_permission_object >( [&]( governance_permission_object& bpo )
   {
      bpo.account = o.new_governance_name;
      bpo.directors.insert( o.founder );
      bpo.executives.insert( o.founder );
      bpo.chief_executive = o.founder;
      bpo.last_updated = now;
      bpo.created = now;
   });

   _db.create< governance_director_object >( [&]( governance_director_object& bdo )
   {
      bdo.director = o.founder;
      bdo.governance = o.new_governance_name;
      bdo.active = true;
      bdo.appointed = true;
      bdo.last_updated = now;
      bdo.created = now;
   });

   _db.create< governance_director_vote_object >( [&]( governance_director_vote_object& bdvo )
   {
      bdvo.account = o.founder;
      bdvo.governance = o.new_governance_name;
      bdvo.director = o.founder;
      bdvo.last_updated = now;
      bdvo.created = now;
   });

   _db.create< governance_executive_object >( [&]( governance_executive_object& beo )
   {
      beo.executive = o.founder;
      beo.governance = o.new_governance_name;
      beo.active = true;
      beo.appointed = true;
      beo.last_updated = now;
      beo.created = now;
   });

   _db.create< governance_executive_vote_object >( [&]( governance_executive_vote_object& bevo )
   {
      bevo.director = o.founder;
      bevo.governance = o.new_governance_name;
      bevo.executive = o.founder;
      bevo.last_updated = now;
      bevo.created = now;
   });

   _db.modify( founder, [&]( account_object& a )
   {
      a.last_community_created = now;
      a.last_asset_created = now;
   });

   _db.update_governance( governance );

   ilog( "Founder: ${r} created new governance Account: \n ${a} \n ${p} \n",
      ("r",o.founder)("a",governance)("p",governance_permission));

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void governance_update_evaluator::do_apply( const governance_update_operation& o )
{ try {
   const account_object& chief_executive = _db.get_account( o.chief_executive );
   FC_ASSERT( chief_executive.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.chief_executive) );
   time_point now = _db.head_block_time();

   const governance_object& governance = _db.get_governance( o.governance );
   const account_object& account = _db.get_account( o.governance );
   const governance_permission_object& governance_permission = _db.get_governance_permission( o.governance );
   const account_authority_object& account_auth = _db.get< account_authority_object, by_account >( o.governance );
   
   FC_ASSERT( ( now - account_auth.last_owner_update ) >= OWNER_UPDATE_LIMIT,
      "Owner authority can only be updated once an hour." );

   for( auto exec : o.executives )
   {
      const account_object& account_auth = _db.get_account( exec );
      FC_ASSERT( account_auth.active, 
         "Account: ${s} must be active to add account authority.",
         ("s", account_auth) );
   }
   
   _db.modify( governance, [&]( governance_object& bo )
   {
      from_string( bo.governance_name, o.governance_display_name );
      bo.equity_revenue_share = o.equity_revenue_share;
      bo.credit_revenue_share = o.credit_revenue_share;
      bo.last_updated = now;
   });

   _db.modify( governance_permission, [&]( governance_permission_object& bpo )
   {
      bpo.executives.clear();
      for( auto e : o.executives )
      {
         bpo.executives.insert( e );
      }

      bpo.last_updated = now;
   });

   authority governance_authority = authority();

   for( auto e : o.executives )
   {
      const governance_executive_object* executive_ptr = _db.find_governance_executive( o.governance, e );

      if( executive_ptr != nullptr )
      {
         _db.modify( *executive_ptr, [&]( governance_executive_object& beo )
         {
            beo.executive = e;
            beo.governance = o.governance;
            beo.active = true;
            beo.appointed = true;
            beo.last_updated = now;
         });
      }
      else
      {
         _db.create< governance_executive_object >( [&]( governance_executive_object& beo )
         {
            beo.executive = e;
            beo.governance = o.governance;
            beo.active = true;
            beo.appointed = true;
            beo.last_updated = now;
            beo.created = now;
         });
      }

      governance_authority.add_authority( e, 1 );
   }

   _db.modify( account_auth, [&]( account_authority_object& auth )
   {
      auth.owner_auth = governance_authority;
      auth.owner_auth.weight_threshold = governance_authority.num_auths();
      auth.active_auth = governance_authority;
      auth.active_auth.weight_threshold = governance_authority.num_auths() / 2;
      auth.posting_auth = governance_authority;
      auth.posting_auth.weight_threshold = 1;
   });

   _db.update_owner_authority( account, governance_authority );

   _db.update_governance( governance );   // Updates the voting status of the governance account to reflect all voting changes.

   ilog( "Chief Executive: ${r} updated new governance Account: \n ${a} \n ${b} \n",
      ("r",o.chief_executive)("a",governance)("b",governance_permission));

} FC_CAPTURE_AND_RETHROW( ( o )) }


void governance_executive_evaluator::do_apply( const governance_executive_operation& o )
{ try {
   const account_object& executive_account = _db.get_account( o.executive );
   FC_ASSERT( executive_account.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.executive) );
   time_point now = _db.head_block_time();

   const governance_object& governance = _db.get_governance( o.governance );
   const governance_member_object& member = _db.get_governance_member( o.executive );
   FC_ASSERT( member.governance == o.governance,
      "Account must be a member of the Governance account to become an executive." );

   const auto& exec_idx = _db.get_index< governance_executive_index >().indices().get< by_governance_executive >();
   auto exec_itr = exec_idx.find( boost::make_tuple( o.governance, o.executive ) );

   share_type voting_power = _db.get_equity_voting_power( o.executive, governance );

   FC_ASSERT( voting_power >= BLOCKCHAIN_PRECISION,
      "Account: ${a} must hold a minimum balance of voting power in the equity assets of the governance account: ${b} in order to become an executive.",
      ("a", o.executive)("b", o.governance) );

   if( exec_itr == exec_idx.end() )
   {
      _db.create< governance_executive_object >( [&]( governance_executive_object& beo )
      {
         beo.executive = o.executive;
         beo.governance = o.governance;
         beo.active = o.active;
         beo.appointed = false;
         beo.last_updated = now;
         beo.created = now;
      });
   }
   else
   {
      const governance_executive_object& executive = *exec_itr;

      _db.modify( executive, [&]( governance_executive_object& beo )
      {
         beo.active = o.active;
         beo.last_updated = now;
      });
   }

   _db.update_governance( governance );   // Updates the voting status of the governance account to reflect all voting changes.

   ilog( "Executive: ${r} created Executive for Business Account: \n ${a} \n",
      ("r",o.executive)("a",governance));

} FC_CAPTURE_AND_RETHROW( ( o )) }


void governance_executive_vote_evaluator::do_apply( const governance_executive_vote_operation& o )
{ try {
   const account_object& voter = _db.get_account( o.director );
   FC_ASSERT( voter.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.director) );
   const account_object& executive = _db.get_account( o.executive );
   FC_ASSERT( executive.active, 
      "Account: ${s} must be active to be voted for.",
      ("s", o.executive));
   const account_object& governance_account = _db.get_account( o.governance );
   FC_ASSERT( governance_account.active, 
      "Account: ${s} must be active to accept executive votes.",
      ("s",o.governance));
   const governance_member_object& member = _db.get_governance_member( o.director );
   FC_ASSERT( member.governance == o.governance,
      "Account must be a member of the Governance account to vote for an executive." );
   const governance_object& governance = _db.get_governance( o.governance );
   const governance_permission_object& governance_permission = _db.get_governance_permission( o.governance );
   const governance_executive_object& governance_executive = _db.get_governance_executive( o.governance, o.executive );

   share_type voting_power = _db.get_equity_voting_power( o.director, governance );
   time_point now = _db.head_block_time();
   
   const auto& vote_idx = _db.get_index< governance_executive_vote_index >().indices().get< by_director_governance >();
   auto vote_itr = vote_idx.find( boost::make_tuple( voter.name, governance_executive.governance ) );

   if( o.approved )      // Adding or modifying vote
   {
      FC_ASSERT( voter.can_vote,
         "Account has declined its voting rights." );
      FC_ASSERT( governance_permission.is_director( voter.name ),
         "Account: ${a} is not authorized to vote for an Executive in governance: ${b}.",
         ("a", o.director)("b", o.governance));
      FC_ASSERT( voting_power >= BLOCKCHAIN_PRECISION,
         "Account: ${a} must hold a minimum balance of voting power in the equity assets of the governance account: ${b} in order to vote for executives.",
         ("a", o.director)("b", o.governance) );

      if( vote_itr == vote_idx.end() )      // No vote for the director within the governance, create a new vote.
      {
         _db.create< governance_executive_vote_object >( [&]( governance_executive_vote_object& v )
         {
            v.director = voter.name;
            v.executive = governance_executive.executive;
            v.governance = governance_executive.governance;
            v.last_updated = now;
            v.created = now;
         });
      }
      else
      {
         const governance_executive_vote_object& vote = *vote_itr;

         _db.modify( vote, [&]( governance_executive_vote_object& v )
         {
            v.director = voter.name;
            v.executive = governance_executive.executive;
            v.governance = governance_executive.governance;
            v.last_updated = now;
         });
      }
   }
   else  // Removing existing vote
   {
      if( vote_itr != vote_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*vote_itr));
         _db.remove( *vote_itr );
      }
   }

   _db.update_governance( governance );   // Updates the voting status of the governance account to reflect all voting changes.

} FC_CAPTURE_AND_RETHROW( ( o )) }


void governance_director_evaluator::do_apply( const governance_director_operation& o )
{ try {
   const account_object& director_account = _db.get_account( o.director );
   FC_ASSERT( director_account.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.director) );
   time_point now = _db.head_block_time();
   const governance_object& governance = _db.get_governance( o.governance );
   const governance_member_object& member = _db.get_governance_member( o.director );
   FC_ASSERT( member.governance == o.governance,
      "Account must be a member of the Governance account to become a director." );

   const auto& director_idx = _db.get_index< governance_director_index >().indices().get< by_governance_director >();
   auto director_itr = director_idx.find( boost::make_tuple( o.governance, o.director ) );

   share_type voting_power = _db.get_equity_voting_power( o.director, governance );

   FC_ASSERT( voting_power >= BLOCKCHAIN_PRECISION,
      "Account: ${a} must hold a minimum balance of voting power in the equity assets of the governance account: ${b} in order to become an director.",
      ("a", o.director)("b", o.governance) );

   if( director_itr == director_idx.end() )
   {
      _db.create< governance_director_object >( [&]( governance_director_object& bdo )
      {
         bdo.director = o.director;
         bdo.governance = o.governance;
         bdo.active = o.active;
         bdo.appointed = false;
         bdo.last_updated = now;
         bdo.created = now;
      });
   }
   else
   {
      const governance_director_object& director = *director_itr;

      _db.modify( director, [&]( governance_director_object& beo )
      {
         beo.active = o.active;
         beo.last_updated = now;
      });
   }

   _db.update_governance( governance );   // Updates the voting status of the governance account to reflect all voting changes.

} FC_CAPTURE_AND_RETHROW( ( o )) }


void governance_director_vote_evaluator::do_apply( const governance_director_vote_operation& o )
{ try {
   const account_object& voter = _db.get_account( o.account );
   FC_ASSERT( voter.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   const account_object& director = _db.get_account( o.director );
   FC_ASSERT( director.active, 
      "Account: ${s} must be active to be voted for.",
      ("s", o.director) );
   const account_object& governance_account = _db.get_account( o.governance );
   FC_ASSERT( governance_account.active, 
      "Account: ${s} must be active to accept director votes.",
      ("s", o.governance));
   const governance_object& governance = _db.get_governance( o.governance );
   const governance_member_object& member = _db.get_governance_member( o.account );
   FC_ASSERT( member.governance == o.governance,
      "Account must be a member of the Governance account to vote for a director." );
   const governance_director_object& governance_director = _db.get_governance_director( o.governance, o.director );

   share_type voting_power = _db.get_equity_voting_power( o.account, governance );
   time_point now = _db.head_block_time();

   const auto& vote_idx = _db.get_index< governance_director_vote_index >().indices().get< by_account_governance >();
   auto vote_itr = vote_idx.find( boost::make_tuple( voter.name, governance_director.governance ) );
   
   if( o.approved )       // Adding or modifying vote
   {
      FC_ASSERT( voter.can_vote,
         "Account has declined its voting rights." );

      if( vote_itr == vote_idx.end() ) // No vote for director or type rank, create new vote.
      {
         FC_ASSERT( voting_power >= BLOCKCHAIN_PRECISION,
            "Account must hold a balance of voting power in the equity assets of the governance account in order to vote for directors." );

         _db.create< governance_director_vote_object>( [&]( governance_director_vote_object& v )
         {
            v.account = voter.name;
            v.director = governance_director.director;
            v.governance = governance_director.governance;
            v.last_updated = now;
            v.created = now;
         });
      }
      else
      {
         const governance_director_vote_object& vote = *vote_itr;

         _db.modify( vote, [&]( governance_director_vote_object& v )
         {
            v.director = governance_director.director;
            v.last_updated = now;
         });
      }  
   }
   else  // Removing existing vote
   {
      
      if( vote_itr != vote_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*vote_itr));
         _db.remove( *vote_itr );
      }
   }

   _db.update_governance( governance );   // updates the voting status of the governance account to reflect all voting changes.

} FC_CAPTURE_AND_RETHROW( ( o )) }


void governance_member_evaluator::do_apply( const governance_member_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   const account_object& governance_account = _db.get_account( o.governance );
   FC_ASSERT( governance_account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.governance) );
   time_point now = _db.head_block_time();
   const governance_object& governance = _db.get_governance( o.governance );
   
   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }

   const auto& member_idx = _db.get_index< governance_member_index >().indices().get< by_account >();
   auto member_itr = member_idx.find( o.account );

   if( member_itr == member_idx.end() )     // Creating new governance member, first for account
   {
      FC_ASSERT( o.approved, 
         "Cannot remove membership, account is not a member of a Governance account" );
      
      const governance_member_request_object& request = _db.get_governance_member_request( o.account );

      FC_ASSERT( request.governance == governance.account, 
         "Cannot create Governance Membership without a valid Governance Membership Request from the account" );

      _db.create< governance_member_object >( [&]( governance_member_object& gmo )
      {
         gmo.governance = o.governance;
         gmo.account = o.account;
         gmo.interface = o.interface;
         gmo.last_updated = now;
         gmo.created = now;
      });

      _db.remove( request );
   }
   else     // Switching to new governance account
   {
      const governance_member_object& member = *member_itr;
      
      if( o.approved )
      {
         const governance_member_request_object& request = _db.get_governance_member_request( o.account );
         FC_ASSERT( request.governance == o.governance, 
            "Cannot create Governance Membership without a valid Governance Membership Request from the account" );

         _db.modify( member, [&]( governance_member_object& gmo )
         {
            gmo.governance = o.governance;
            gmo.interface = o.interface;
            gmo.last_updated = now;
         });

         _db.remove( request );
      }
      else
      {
         FC_ASSERT( member.governance == o.governance, 
            "Cannot remove Governance Membership for another Governance Account" );

         // Remove all Pending Member votes from the Governance Account

         const auto& resolution_vote_idx = _db.get_index< governance_resolution_vote_index >().indices().get< by_governance_resolution_id >();
         auto resolution_vote_itr = resolution_vote_idx.find( o.governance );

         while( resolution_vote_itr != resolution_vote_idx.end() && 
            resolution_vote_itr->account == o.account )
         {
            const governance_resolution_vote_object& vote = *resolution_vote_itr;
            ++resolution_vote_itr;
            _db.remove( vote );
         }

         // Remove any pending member request

         const auto& request_idx = _db.get_index< governance_member_request_index >().indices().get< by_account >();
         auto request_itr = request_idx.find( o.account );

         if( request_itr != request_idx.end() )
         {
            const governance_member_request_object& request = *request_itr;
            if( request.governance == o.governance )
            {
               _db.remove( request );
            }
         }

         // Remove Director votes

         const auto& account_director_vote_idx = _db.get_index< governance_director_vote_index >().indices().get< by_account_governance >();
         auto account_director_vote_itr = account_director_vote_idx.find( boost::make_tuple( o.account, o.governance ) );

         if( account_director_vote_itr != account_director_vote_idx.end() )
         {
            const governance_director_vote_object& director_vote = *account_director_vote_itr;
            _db.remove( director_vote );
         }

         const auto& director_vote_idx = _db.get_index< governance_director_vote_index >().indices().get< by_governance_director >();
         auto director_vote_itr = director_vote_idx.find( boost::make_tuple( o.governance, o.account ) );

         while( director_vote_itr != director_vote_idx.end() &&
            director_vote_itr->governance == o.governance &&
            director_vote_itr->director == o.account )
         {
            const governance_director_vote_object& director_vote = *director_vote_itr;
            ++director_vote_itr;
            _db.remove( director_vote );
         }

         // Remove Executive votes

         const auto& account_executive_vote_idx = _db.get_index< governance_executive_vote_index >().indices().get< by_director_governance >();
         auto account_executive_vote_itr = account_executive_vote_idx.find( boost::make_tuple( o.account, o.governance ) );

         if( account_executive_vote_itr != account_executive_vote_idx.end() )
         {
            const governance_executive_vote_object& executive_vote = *account_executive_vote_itr;
            _db.remove( executive_vote );
         }

         const auto& executive_vote_idx = _db.get_index< governance_executive_vote_index >().indices().get< by_governance_executive >();
         auto executive_vote_itr = executive_vote_idx.find( boost::make_tuple( o.governance, o.account ) );

         while( executive_vote_itr != executive_vote_idx.end() &&
            executive_vote_itr->governance == o.governance &&
            executive_vote_itr->executive == o.account )
         {
            const governance_executive_vote_object& executive_vote = *executive_vote_itr;
            ++executive_vote_itr;
            _db.remove( executive_vote );
         }

         // Remove Director

         const auto& director_idx = _db.get_index< governance_director_index >().indices().get< by_governance_director >();
         auto director_itr = director_idx.find( boost::make_tuple( o.governance, o.account ) );

         if( director_itr != director_idx.end() )
         {
            const governance_director_object& director = *director_itr;
            _db.remove( director );
         }

         // Remove Executive

         const auto& executive_idx = _db.get_index< governance_executive_index >().indices().get< by_governance_executive >();
         auto executive_itr = executive_idx.find( boost::make_tuple( o.governance, o.account ) );

         if( executive_itr != executive_idx.end() )
         {
            const governance_executive_object& executive = *executive_itr;
            _db.remove( executive );
         }

         _db.remove( member );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void governance_member_request_evaluator::do_apply( const governance_member_request_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   const account_object& governance_account = _db.get_account( o.governance );
   FC_ASSERT( governance_account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.governance) );

   time_point now = _db.head_block_time();
   const governance_object& governance = _db.get_governance( o.governance );
   
   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }

   const auto& member_idx = _db.get_index< governance_member_index >().indices().get< by_account >();
   auto member_itr = member_idx.find( o.account );

   const auto& request_idx = _db.get_index< governance_member_request_index >().indices().get< by_account >();
   auto request_itr = request_idx.find( o.account );

   if( request_itr == request_idx.end() )     // Creating new governance member request
   {
      FC_ASSERT( o.active, 
         "No Governance Membership Request exists from this account to cancel" );

      if( member_itr != member_idx.end() )
      {
         const governance_member_object& member = *member_itr;

         FC_ASSERT( member.governance != governance.account, 
            "Account is already a member of the Governance Account" );
      }
         
      _db.create< governance_member_request_object >( [&]( governance_member_request_object& gmro )
      {
         gmro.governance = governance.account;
         gmro.account = o.account;
         gmro.interface = o.interface;
         from_string( gmro.message, o.message );
         gmro.expiration = now + CONNECTION_REQUEST_DURATION;
         gmro.last_updated = now;
         gmro.created = now;
      });
   }
   else     // Editing or cancelling request
   {
      const governance_member_request_object& request = *request_itr;

      if( o.active )     // Editing governance account request
      {
         _db.modify( request, [&]( governance_member_request_object& gmro )
         {
            gmro.governance = o.governance;
            gmro.interface = o.interface;
            from_string( gmro.message, o.message );
            gmro.expiration = now + CONNECTION_REQUEST_DURATION;
            gmro.last_updated = now;
         });
      }
      else   // Removing
      {
         _db.remove( request );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void governance_resolution_evaluator::do_apply( const governance_resolution_operation& o )
{ try {
   const account_object& governance_account = _db.get_account( o.governance );
   FC_ASSERT( governance_account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.governance) );
   time_point now = _db.head_block_time();

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }

   const governance_object& governance = _db.get_governance( o.governance );

   const auto& resolution_idx = _db.get_index< governance_resolution_index >().indices().get< by_resolution_id >();
   auto resolution_itr = resolution_idx.find( boost::make_tuple( governance.account, o.resolution_id, o.ammendment_id ) );

   const auto& resolution_vote_idx = _db.get_index< governance_resolution_vote_index >().indices().get< by_governance_resolution_id >();
   auto resolution_vote_itr = resolution_vote_idx.find( boost::make_tuple( o.governance, o.resolution_id, o.ammendment_id ) );

   if( resolution_vote_itr == resolution_vote_idx.end() )
   {
      FC_ASSERT( o.active,
         "No Resolution exists with this Resolution ID and ammendment ID to remove." );

      const governance_resolution_object& resolution = _db.create< governance_resolution_object >( [&]( governance_resolution_object& gro )
      {
         gro.governance = o.governance;
         from_string( gro.resolution_id, o.resolution_id );
         from_string( gro.ammendment_id, o.ammendment_id );
         from_string( gro.title, o.title );
         from_string( gro.details, o.details );
         from_string( gro.body, o.body );
         from_string( gro.json, o.json );

         gro.interface = o.interface;
         gro.completion_time = o.completion_time;
         gro.last_updated = now;
         gro.created = now;
      });

      ilog( "Governance: ${r} created new resolution: \n ${a} \n",
         ("r",o.governance)("a",resolution));
   }
   else
   {
      const governance_resolution_object& resolution = *resolution_itr;

      if( o.active )             // Editing existing resolution
      {
         FC_ASSERT( resolution_vote_itr == resolution_vote_idx.end(), 
            "Resolution cannot be edited after it has been voted on, create an ammendment" ); 
         
         _db.modify( resolution, [&]( governance_resolution_object& gro )
         {
            from_string( gro.title, o.title );
            from_string( gro.details, o.details );
            from_string( gro.body, o.body );
            from_string( gro.json, o.json );

            gro.completion_time = o.completion_time;
            gro.last_updated = now;
         });
      }
      else          // Removing resolution ammendment and all votes
      {
         FC_ASSERT( !resolution.approved,
            "Resolution Ammendment has already been approved, and cannot be removed" );
         
         while( resolution_vote_itr != resolution_vote_idx.end() && 
            resolution_vote_itr->governance == resolution.governance && 
            resolution_vote_itr->resolution_id == resolution.resolution_id &&
            resolution_vote_itr->ammendment_id == resolution.ammendment_id )
         {
            const governance_resolution_vote_object& vote = *resolution_vote_itr;
            ++resolution_vote_itr;
            _db.remove( vote );
         }
         _db.remove( resolution );
      }
   }

   
} FC_CAPTURE_AND_RETHROW( ( o )) }


void governance_resolution_vote_evaluator::do_apply( const governance_resolution_vote_operation& o )
{ try {
   const account_object& voter = _db.get_account( o.account );
   FC_ASSERT( voter.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   const account_object& governance_account = _db.get_account( o.governance );
   FC_ASSERT( governance_account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.governance) );
   time_point now = _db.head_block_time();

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }

   const governance_object& governance = _db.get_governance( o.governance );
   const governance_resolution_object& resolution = _db.get_governance_resolution( governance.account, o.resolution_id, o.ammendment_id );

   FC_ASSERT( resolution.completion_time >= now, 
      "Cannot vote after resolution completion time." );

   const auto& resolution_vote_idx = _db.get_index< governance_resolution_vote_index >().indices().get< by_account_resolution_id >();
   auto resolution_vote_itr = resolution_vote_idx.find( boost::make_tuple( o.account, governance.account, o.resolution_id, o.ammendment_id ) );

   if( resolution_vote_itr == resolution_vote_idx.end() )
   {
      FC_ASSERT( o.active,
         "No Resolution Vote exists with this Resolution ID and Ammendment ID to remove." );
      const governance_member_object& member = _db.get_governance_member( o.account );
      FC_ASSERT( member.governance == o.governance,
         "Account must be a member of the Governance account to vote for a Resolution." );

      _db.create< governance_resolution_vote_object >( [&]( governance_resolution_vote_object& grvo )
      {
         grvo.account = o.account;
         grvo.governance = governance.account;
         from_string( grvo.resolution_id, o.resolution_id );
         from_string( grvo.ammendment_id, o.ammendment_id );

         grvo.interface = o.interface;
         grvo.approved = o.approved;
         grvo.last_updated = now;
         grvo.created = now;
      });
   }
   else
   {
      const governance_resolution_vote_object& vote = *resolution_vote_itr;

      if( o.active )             // Editing existing vote
      {
         FC_ASSERT( resolution_vote_itr == resolution_vote_idx.end(),
            "Resolution cannot be edited after it has been voted on, create an ammendment" );
         const governance_member_object& member = _db.get_governance_member( o.account );
         FC_ASSERT( member.governance == governance.account,
            "Account must be a member of the Governance account to update vote for a Resolution." );
         
         _db.modify( vote, [&]( governance_resolution_vote_object& grvo )
         {
            grvo.approved = o.approved;
            grvo.last_updated = now;
         });
      }
      else          // Removing resolution ammendment vote
      {
         _db.remove( vote );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


} } // node::chain