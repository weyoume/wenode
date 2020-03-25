#pragma once
#define BLOCKCHAIN_VERSION              ( version(0, 0, 1) )
#define BLOCKCHAIN_HARDFORK_VERSION     ( hardfork_version( BLOCKCHAIN_VERSION ) )

#ifndef SHOW_PRIVATE_KEYS
	#define SHOW_PRIVATE_KEYS 							1
#endif

#ifndef GEN_PRIVATE_KEY
	#define GEN_PRIVATE_KEY 							1
#endif

#ifdef IS_TEST_NET
	#define INIT_PRIVATE_KEY                (fc::ecc::private_key::regenerate(fc::sha256::hash(std::string("init_key"))))
	#define INIT_PUBLIC_KEY_STR             (std::string( node::protocol::public_key_type(INIT_PRIVATE_KEY.get_public_key()) ))
	#define CHAIN_ID                        (fc::sha256::hash("testnet"))

	#define ADDRESS_PREFIX                  "TWYM"

	#define OWNER_AUTH_RECOVERY_PERIOD                  fc::seconds(60)
	#define ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD  fc::seconds(12)
	#define OWNER_UPDATE_LIMIT                          fc::seconds(0)
	#define OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM 1

#endif // IS LIVE NETWORK
	
	#if GEN_PRIVATE_KEY
		#define INIT_PRIVATE_KEY            (fc::ecc::private_key::regenerate(fc::sha256::hash(std::string("accountNameownerpassword")))) // accountName + permission + password
		#define INIT_PUBLIC_KEY_STR         (std::string( node::protocol::public_key_type(INIT_PRIVATE_KEY.get_public_key()) ))
	#else
		#define INIT_PUBLIC_KEY_STR             "WYM7sbABVQcfuGVS6bXrnt8DRKi58YUWKWM7bVLTj8xvfZBVKfhoU"
	#endif
	

// WeYouMe

#define CHAIN_ID                        (fc::sha256::hash("WeYouMe"))
#define ADDRESS_PREFIX                  "WYM"

#define BLOCKCHAIN_PRECISION            share_type( 100000000 ) // 100 Million integer units per currency unit
#define BLOCKCHAIN_PRECISION_DIGITS     8

#define MECOIN_SYMBOL                   "MEC"
#define WEYOUME_SYMBOL                  "WYM"
#define MEUSD_SYMBOL                    "MUSD"
#define MECREDIT_SYMBOL                 "MCR"
#define LIQUIDITY_ASSET_PREFIX          "LIQUID."
#define CREDIT_ASSET_PREFIX             "CREDIT."
#define OPTION_ASSET_PREFIX             "OPT."

#define SYMBOL_COIN  				    MECOIN_SYMBOL                 // [MEC: MeCoin - Cryptocurrency of the WeYouMe Blockchain]
#define SYMBOL_EQUITY  					WEYOUME_SYMBOL                // [WYM: WeYouMe - CryptoEquity of the WeYouMe Blockchain]
#define SYMBOL_USD    					MEUSD_SYMBOL                  // [MUSD: MeUSD - Stablecoin collateralized by MeCoin with value pegged to $1.00 USD]
#define SYMBOL_CREDIT 					MECREDIT_SYMBOL               // [MCR: MeCredit - Interest bearing CryptoCredit issued by the WeYouMe blockchain]

#define MILLION                         (1000000)                       // 1 Million integer
#define BILLION                         (1000000000)                    // 1 Billion Integer
#define TRILLION                        (1000000000000)                 // 1 Trillion integer
#define PERCENT_100                     (10000)                       // Uses 5 decimal place integer to enable percentages to 2 decimal places of precision
#define PERCENT_1                       (PERCENT_100/100)             // 1% as percentage integer
#define PERCENT_10_OF_PERCENT_1         (PERCENT_100/1000)            // 0.1% as percentage integer

#define GENESIS_TIME                    fc::time_point(fc::microseconds(1590242400000000))    // 24th May 2020 network launch time
#define MINING_TIME                     fc::time_point(fc::microseconds(1590242400000000))    // 24th May 2020 network launch time

#define INIT_COIN_SUPPLY                (0)                                                  // Zero MeCoin preissuance
#define INIT_EQUITY_SUPPLY              share_type( 10 * MILLION * BLOCKCHAIN_PRECISION )        // 10 Million Supply of WYM.
#define MAX_ASSET_SUPPLY                share_type( TRILLION * BLOCKCHAIN_PRECISION )        // 1 Trillion max asset supply.

#define BLOCK_INTERVAL                  fc::microseconds(788400)                                    // 0.7884 Seconds per block, resulting in exactly 40 Million blocks per year.
#define BLOCKS_PER_YEAR                 int64_t(40 * MILLION)                                       // 40 Million blocks per year.
#define BLOCKS_PER_DAY                  int64_t(fc::days(1).count() / BLOCK_INTERVAL.count())       // Approximately 109,589 Blocks per day.
#define BLOCKS_PER_HOUR                 int64_t(fc::hours(1).count() / BLOCK_INTERVAL.count())      // Approximately 4,466 Blocks per hour.
#define BLOCKS_PER_MINUTE               int64_t(fc::minutes(1).count() / BLOCK_INTERVAL.count())    // Approximately 76 Blocks per minute.

#define ANNUAL_COIN_ISSUANCE            asset( 1 * BILLION * BLOCKCHAIN_PRECISION, SYMBOL_COIN)     // 1 Billion MEC issued per year
#define BLOCK_REWARD                    (ANNUAL_COIN_ISSUANCE / BLOCKS_PER_YEAR)                    // 25 MeCoin issued per block
#define MAX_ACCEPTED_PAYOUT             asset( BILLION * BLOCKCHAIN_PRECISION, SYMBOL_USD );        // Maximum amount accepted as a content reward payout

#define IRREVERSIBLE_THRESHOLD                (67 * PERCENT_1)          // Blocks produced become irrervsible after approval by this percentage of active producers. 
#define POW_TARGET_TIME                       fc::minutes(10)           // Aim for approximately one proof of work every 10 minutes to be produced. 
#define POW_DECAY_TIME                        fc::days(7)               // Averaging time of one week for adjusting proof of work difficulty. 
#define POW_UPDATE_BLOCK_INTERVAL             (1 * BLOCKS_PER_HOUR)     // Updates the mining dificulty once per hour.            
#define POA_BLOCK_INTERVAL                    (8 * BLOCKS_PER_HOUR)     // Distributes the proof of activity reward every 8 hours to the highest activity voted producer. 
#define TXN_STAKE_BLOCK_INTERVAL              (1 * BLOCKS_PER_HOUR)     // Transaction stake rewards are distributed each hour.
#define TXN_STAKE_DECAY_TIME                  fc::days(7)               // Transaction stake is averaged over a rolling 7 day window. 
#define NETWORK_OFFICER_BLOCK_INTERVAL        (BLOCKS_PER_DAY)          // Distributes network officer rewards once every day.
#define NETWORK_OFFICER_ACTIVE_SET            (50)
#define EXECUTIVE_BOARD_BLOCK_INTERVAL        (BLOCKS_PER_DAY)          // Distributes network officer rewards once every day.
#define MIN_EXEC_CREDIT_PRICE                 price( asset( 9, SYMBOL_USD), asset( 10, SYMBOL_CREDIT))     // $0.90 minimum credit price to pay executive budgets
#define SUPERNODE_BLOCK_INTERVAL              (BLOCKS_PER_DAY)          // Distributes supernode rewards once every day.
#define SUPERNODE_DECAY_TIME                  fc::days(7)               // Averages supernode file weight over 7 days.

#define GOVERNANCE_VOTE_THRESHOLD_PERCENT             (PERCENT_1)         // Governance Accounts require 1% of network voting power to be approved for profile account creation.
#define GOVERNANCE_VOTE_THRESHOLD_AMOUNT              (25)                // Governance Accounts proposals require 25 individual votes to be approved for profile account creation.
#define GOVERNANCE_VOTE_THRESHOLD_PRODUCERS           (5)                 // Governance Accounts require 5 top voted producer votes to be approved for profile account creation.
#define ENTERPRISE_VOTE_THRESHOLD_PERCENT             (PERCENT_1)         // Enterprise proposals require 1% of network voting power to approve milestones.
#define ENTERPRISE_VOTE_THRESHOLD_AMOUNT              (25)                // Enterprise proposals require 25 individual votes to approve milestones.
#define ENTERPRISE_VOTE_THRESHOLD_PRODUCERS           (5)                 // Enterprise proposals require 5 top voted producer votes to approve milestones.
#define OFFICER_VOTE_THRESHOLD_PERCENT                (PERCENT_1)         // Network Officers require 1% of network voting power to be approved for rewards.
#define OFFICER_VOTE_THRESHOLD_AMOUNT                 (25)                // Network Officers proposals require 25 individual votes to be approved for rewards.
#define OFFICER_VOTE_THRESHOLD_PRODUCERS              (5)                 // Network Officers require 5 top voted producer votes to be approved for rewards.
#define EXECUTIVE_VOTE_THRESHOLD_PERCENT              (10 * PERCENT_1)    // Executive boards require 10% of network voting power to be approved for rewards.
#define EXECUTIVE_VOTE_THRESHOLD_AMOUNT               (100)               // Executive boards proposals require 100 individual votes to be approved for rewards.
#define EXECUTIVE_VOTE_THRESHOLD_PRODUCERS            (20)                // Executive boards require 20 top voted producer votes to be approved for rewards.
#define MAX_ACCOUNT_VOTES                             (1000)  

#define SET_UPDATE_BLOCK_INTERVAL       (BLOCKS_PER_DAY)      // Updates business account executive sets once every day.
#define ENTERPRISE_BLOCK_INTERVAL       (BLOCKS_PER_DAY)      // Distributes community enterprise funding once every day.
#define BITASSET_BLOCK_INTERVAL         (BLOCKS_PER_DAY)      // Updates the settlement volume on bitassets and processes collateral bids
#define VOTE_RECHARGE_TIME              fc::days(7)           // 7 days to regenerate maximum voting power. Accumulation period of unused transactions for allocating rewards.
#define VIEW_RECHARGE_TIME              fc::days(7)           // 7 day to regenerate maximum viewing power. Accumulation period of unused transactions for allocating rewards.
#define SHARE_RECHARGE_TIME             fc::days(7)           // 7 days to regenerate maximum sharing power. Accumulation period of unused transactions for allocating rewards.
#define COMMENT_RECHARGE_TIME           fc::days(7)           // 7 days to regenerate maximum commenting power. Accumulation period of unused transactions for allocating rewards.
#define VOTE_RESERVE_RATE               (20)                  // 20 votes per day to maintain net neutral voting power. 1 share = 2 comments = 4 Votes = 20 views
#define VIEW_RESERVE_RATE               (100)                 // 100 views per day to maintain net neutral viewing power.
#define SHARE_RESERVE_RATE              (5)                   // 5 shares per day to maintain net neutral voting power.
#define COMMENT_RESERVE_RATE            (10)                  // 10 comments per day to maintain net neutral voting power.    
#define CURATION_AUCTION_DECAY_TIME     fc::minutes(10)       // First 10 minutes after a post linearly decays all curation rewards
#define VOTE_CURATION_DECAY             (200)                 // Curation reward decays by 50% per 200 votes.
#define VIEW_CURATION_DECAY             (1000)                // Curation reward decays by 50% per 1000 views.
#define SHARE_CURATION_DECAY            (50)                  // Curation reward decays by 50% per 50 shares.
#define COMMENT_CURATION_DECAY          (100)                 // Curation reward decays by 50% per 100 comments.

#define OWNER_AUTH_RECOVERY_PERIOD                       fc::days(30)
#define ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD       fc::days(1)
#define DECLINE_VOTING_RIGHTS_DURATION                   fc::days(3)
#define OWNER_UPDATE_LIMIT                               fc::minutes(60)
#define OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM      1
#define MIN_COMMUNITY_CREATE_INTERVAL                    fc::days(1)
#define MIN_ASSET_CREATE_INTERVAL                        fc::days(1)
#define MIN_COMMUNITY_UPDATE_INTERVAL                    fc::minutes(10)
#define MIN_ASSET_UPDATE_INTERVAL                        fc::minutes(10)

#define GENESIS_PRODUCER_AMOUNT                   (120)
#define TOTAL_PRODUCERS                           (GENESIS_PRODUCER_AMOUNT)
#define GENESIS_EXTRA_PRODUCERS                   (200-GENESIS_PRODUCER_AMOUNT)
#define GENESIS_ACCOUNT_COIN_STAKE		          (1 * BLOCKCHAIN_PRECISION)
#define GENESIS_ACCOUNT_COIN                      (1 * BLOCKCHAIN_PRECISION)
#define DPOS_VOTING_PRODUCERS                     (50)    // The Top 50 Highest voted producers are selected for block production each round
#define POW_MINING_PRODUCERS                      (50)    // The top 50 Highest producing miners are selected for block production each round
#define DPOS_VOTING_ADDITIONAL_PRODUCERS          (10)    // 10 Additonal producers are randomly selected for block production each round, according to stake voting weight
#define POW_MINING_ADDITIONAL_PRODUCERS           (10)    // 10 Additonal miners are selected for block production each round, according to proof of work weight

#define HARDFORK_REQUIRED_PRODUCERS     ((GENESIS_PRODUCER_AMOUNT/4)*3) // 3 Quarters of producers required for hardfork version upgrade acceptance.
#define MAX_TIME_UNTIL_EXPIRATION       (60*60)            // seconds,  aka: 1 hour
#define MAX_MEMO_SIZE                   2048
#define MAX_PROXY_RECURSION_DEPTH       4
#define COIN_UNSTAKE_INTERVALS          4
#define STAKE_WITHDRAW_INTERVAL         fc::days(7)        // 1 week per stake withdrawal interval
#define MAX_WITHDRAW_ROUTES             10
#define SAVINGS_WITHDRAW_TIME           (fc::days(3))
#define SAVINGS_WITHDRAW_REQUEST_LIMIT  (100)
#define MAX_ASSET_STAKE_INTERVALS       (104)              // Maximum weeks that an asset can stake over.
#define MAX_ASSET_UNSTAKE_INTERVALS     (104)              // Maximum weeks that an asset can unstake over.

#define MAX_VOTE_CHANGES                127
#define MIN_VOTE_INTERVAL               fc::seconds(2)        // 2 seconds
#define MIN_VIEW_INTERVAL               fc::seconds(2)        // 2 seconds
#define MIN_SHARE_INTERVAL              fc::seconds(2)        // 2 seconds
#define MIN_ROOT_COMMENT_INTERVAL       fc::seconds(60)       // 60 seconds
#define MIN_REPLY_INTERVAL              fc::seconds(15)       // 15 seconds

#define FREE_TRANSACTIONS_WITH_NEW_ACCOUNT   100
#define BANDWIDTH_AVERAGE_WINDOW             fc::days(7)                // 1 week averaging window for the calculation of bandwidth reserve ratio
#define BANDWIDTH_PRECISION                  share_type(1000000)         // 1 million decimal places for bandwidth units
#define MAX_BODY_SIZE                        (1024 * 1024 * 128 )  // 128 mb of body text limit
#define MAX_COMMENT_DEPTH                    0xffff // 64k
#define SOFT_MAX_COMMENT_DEPTH               0xff // 255
#define MAX_RESERVE_RATIO                    (20000)
#define MIN_ACCOUNT_CREATION_FEE             asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN )
#define CREATE_ACCOUNT_DELEGATION_RATIO      2
#define CREATE_ACCOUNT_DELEGATION_TIME       fc::days(1)
#define EQUIHASH_N                           140
#define EQUIHASH_K                           6

// Network Issuance Reward allocation percentages

#define CONTENT_REWARD_PERCENT           (25 * PERCENT_1) // Percentage of coin issuance distributed to content creators and curators: 25%
#define EQUITY_REWARD_PERCENT            (20 * PERCENT_1) // Percentage of coin issuance distributed to holders of the network cryptoequity: 20%
#define PRODUCER_REWARD_PERCENT          (20 * PERCENT_1) // Percentage of coin issuance distributed to block producers: 20%
#define SUPERNODE_REWARD_PERCENT         (10 * PERCENT_1) // Percentage of coin issuance distributed to Full archive + IPFS storage providers + Public API : 10%
#define POWER_REWARD_PERCENT             (10 * PERCENT_1) // Percentage of coin issuance distributed to holders of staked coin: 10%
#define COMMUNITY_FUND_REWARD_PERCENT    (5 * PERCENT_1)  // Percentage of coin issuance distributed to the community fund for project proposals: 5%
#define DEVELOPMENT_REWARD_PERCENT       (25 * PERCENT_10_OF_PERCENT_1) // Percentage of coin issuance distributed to elected developers: 2.5%
#define ADVOCACY_REWARD_PERCENT          (25 * PERCENT_10_OF_PERCENT_1) // Percentage of coin issuance distributed to elected advocates: 2.5%
#define MARKETING_REWARD_PERCENT         (25 * PERCENT_10_OF_PERCENT_1) // Percentage of coin issuance distributed to elected marketers: 2.5%
#define ACTIVITY_REWARD_PERCENT          (25 * PERCENT_10_OF_PERCENT_1) // Percentage of coin issuance distributed to daily active accounts: 2.5%

// Content reward allocation percentages:
// 70% of content rewards are distributed to the authors of posts
// 30% of content rewards are distributed to voters, viewers, sharers, commenters, storage supernodes, and community moderators.

#define AUTHOR_REWARD_PERCENT            (70 * PERCENT_1)
#define VOTE_REWARD_PERCENT              (5 * PERCENT_1)
#define VIEW_REWARD_PERCENT              (5 * PERCENT_1)
#define SHARE_REWARD_PERCENT             (5 * PERCENT_1)
#define COMMENT_REWARD_PERCENT           (5 * PERCENT_1)
#define STORAGE_REWARD_PERCENT           (5 * PERCENT_1)
#define MODERATOR_REWARD_PERCENT         (5 * PERCENT_1)
#define CONTENT_REWARD_DECAY_RATE        fc::days(30)          // 30 days of content reward payouts for posts. 
#define CONTENT_REWARD_INTERVAL          fc::days(1)           // 1 day between post content rewards.
#define CONTENT_CONSTANT                 uint128_t(2000000000000)
#define MIN_PAYOUT_USD                   (asset( BLOCKCHAIN_PRECISION / 100 , SYMBOL_USD)) // Minimum payout of $0.01 USD worth of rewards.

// Block Producer reward allocation percentages:
// 40% of producer rewards are shared equally between top selected producers.
// 60% of producer rewards are issued competively based on reward factors that incentivise fast validation for finality, high transaction throughput, Proofs of Work, and Proofs of Activity.

#define PRODUCER_BLOCK_PERCENT           (40 * PERCENT_1) // Issued to the Producer that signs and creates each individual block: 40%
#define PRODUCER_VALIDATOR_PERCENT       (15 * PERCENT_1) // Issued to the first two-thirds plus one producers to broadcast validation commitment transactions for each block generated: 15%
#define PRODUCER_TXN_STAKE_PERCENT       (15 * PERCENT_1) // Issued to the producers proportionally to the amount of transaction stake value included in their blocks in the preceding 7 days: 15%
#define PRODUCER_WORK_PERCENT            (15 * PERCENT_1) // Issued to the first miner that broadcasts a valid Proof of Work with sufficient difficulty after each block: 15%
#define PRODUCER_ACTIVITY_PERCENT        (15 * PERCENT_1) // Issued to the highest activity voted prime producer each 8 hours: 15%

#define GOVERNANCE_SHARE_PERCENT         (10 * PERCENT_1) // Percentage of network revenue distributed to each account's governance addresses: 10%
#define REFERRAL_SHARE_PERCENT           (10 * PERCENT_1) // Percentage of network revenue distributed to each account's referrers: 10%
#define REWARD_LIQUID_PERCENT            (75 * PERCENT_1) // Percentage of content rewards that are liquid when claimed: 75%
#define REWARD_STAKED_PERCENT            (25 * PERCENT_1) // Percentage of content rewards that are staked when claimed: 25%

#define TRADING_FEE_PERCENT              (1 * PERCENT_10_OF_PERCENT_1) // Percentage Fee charged on taker trades, when a user matches an order: 0.1%
#define NETWORK_TRADING_FEE_PERCENT      (50 * PERCENT_1) // Percentage of trading fee that is consumed as network revenue: 50% 
#define MAKER_TRADING_FEE_PERCENT        (25 * PERCENT_1) // Percentage of trading fee that is shared with maker's interface provider: 25%
#define TAKER_TRADING_FEE_PERCENT        (25 * PERCENT_1) // Percentage of trading fee that is shared with taker's interface provider: 25%

#define ADVERTISING_FEE_PERCENT            (50 * PERCENT_1) // Percentage of advertising spend that is charged as a fee when a provider claims ad delivery: 50%
#define NETWORK_ADVERTISING_FEE_PERCENT    (50 * PERCENT_1) // Percentage of advertising fee that is consumed as network revenue: 50%
#define DEMAND_ADVERTISING_FEE_PERCENT     (25 * PERCENT_1) // Percentage of advertising fee that is shared with the advertising purchaser's interface: 25%
#define AUDIENCE_ADVERTISING_FEE_PERCENT   (25 * PERCENT_1) // Percentage of advertising fee that is shared with the audience of the delivery: 25%

#define MEMBERSHIP_FEE_BASE                (asset( ( 5 * BLOCKCHAIN_PRECISION ) / 2, SYMBOL_USD)) // Monthly Price of Standard membership: $2.50 USD
#define MEMBERSHIP_FEE_MID                 (asset( 25 * BLOCKCHAIN_PRECISION, SYMBOL_USD)) // Monthly Price of Mezzanine membership: $25.00 USD
#define MEMBERSHIP_FEE_TOP                 (asset( 250 * BLOCKCHAIN_PRECISION, SYMBOL_USD)) // Monthly Price of Top Level membership: $250.00 USD
#define NETWORK_MEMBERSHIP_FEE_PERCENT     (50 * PERCENT_1) // Percentage of membership spend that is consumed as network revenue: 50%
#define INTERFACE_MEMBERSHIP_FEE_PERCENT   (25 * PERCENT_1) // Percentage of membership spend that is shared with the purchaser's interface: 25%
#define PARTNERS_MEMBERSHIP_FEE_PERCENT    (25 * PERCENT_1) // Percentage of membership spend that is distributed to premium content partners: 25%

#define PREMIUM_FEE_PERCENT                (2 * PERCENT_1) // Percentage fee charged on premium content purchases: 2%
#define NETWORK_PREMIUM_FEE_PERCENT        (50 * PERCENT_1) // Percentage of premium content fee consumed as network revenue: 50%
#define INTERFACE_PREMIUM_FEE_PERCENT      (25 * PERCENT_1) // Percentage of premium content fee shared with the purhaser's interface: 25%
#define NODE_PREMIUM_FEE_PERCENT           (25 * PERCENT_1) // Percentage of premium content fee shared with hosting supernodes: 25%

#define MARKETPLACE_FEE_PERCENT                    (2 * PERCENT_1)   // Percentage fee charged on marketplace sales with mediation: 2%
#define NETWORK_MARKETPLACE_FEE_PERCENT            (50 * PERCENT_1)  // Percentage of marketplace fee consumed as network revenue: 50%
#define BUY_INTERFACE_MARKETPLACE_FEE_PERCENT      (25 * PERCENT_1)  // Percentage of marketplace fee shared with buyer's interface: 25%
#define SELL_INTERFACE_MARKETPLACE_FEE_PERCENT     (25 * PERCENT_1)  // Percentage of marketplace fee shared with seller's interface: 25%
#define DISPUTE_FEE_PERCENT                        (2 * PERCENT_1)   // Percentage of total sale value charged when disputed: 2%
#define NETWORK_DISPUTE_FEE_PERCENT                (10 * PERCENT_1)  // Percentage of marketplace fee distributed to mediation team members: 10%
#define BUYER_MEDIATOR_DISPUTE_FEE_PERCENT         (20 * PERCENT_1)  // Percentage of dispute fee paid to the buyers selected mediator: 20%
#define SELLER_MEDIATOR_DISPUTE_FEE_PERCENT        (20 * PERCENT_1)  // Percentage of dispute fee paid to the sellers selected mediator: 20%
#define ALLOCATED_MEDIATOR_DISPUTE_FEE_PERCENT     (50 * PERCENT_1)  // Percentage of dispute fee paid split between selected mediators: 50%
#define ESCROW_BOND_PERCENT                        (10 * PERCENT_1)  // require bond of 10% of escrow payment value
#define ESCROW_DISPUTE_DURATION                    fc::days(7)       // 7 days of mediation time required before release of funds. 
#define ESCROW_DISPUTE_MEDIATOR_AMOUNT             5                 // 5 Random top mediators added to dispute for resolution.  

#define SUBSCRIPTION_FEE_PERCENT               (2 * PERCENT_1) // Percentage fee charged on subscription content purchases: 2%
#define NETWORK_SUBSCRIPTION_FEE_PERCENT       (50 * PERCENT_1) // Percentage of subscription fee consumed as network revenue: 50%
#define INTERFACE_SUBSCRIPTION_FEE_PERCENT     (25 * PERCENT_1) // Percentage of subscription fee shared with the purchaser's interface: 50%
#define NODE_SUBSCRIPTION_FEE_PERCENT          (25 * PERCENT_1) // Percentage of subscription fee shared with hostign supernodes: 50%

#define MIN_ACTIVITY_PRODUCERS                 (10)              // Accounts need at least 10 producer votes to claim activity reward. 
#define ACTIVITY_BOOST_STANDARD_PERCENT        (125 * PERCENT_1) // Boost activty reward by 25%
#define ACTIVITY_BOOST_MID_PERCENT             (150 * PERCENT_1) // Boost activty reward by 50%
#define ACTIVITY_BOOST_TOP_PERCENT             (200 * PERCENT_1) // Boost activty reward by 100%
#define RECENT_REWARD_DECAY_RATE               fc::days(30)      // Activity Reward decays over 30 days

#define MIN_ACCOUNT_NAME_LENGTH                3
#define MAX_ACCOUNT_NAME_LENGTH                32
#define MIN_ASSET_SYMBOL_LENGTH                3
#define MAX_ASSET_SYMBOL_LENGTH                32
#define MIN_PERMLINK_LENGTH                    0
#define MAX_PERMLINK_LENGTH                    2048
#define MAX_STRING_LENGTH                      2048
#define MAX_TEXT_POST_LENGTH                   300
#define MAX_SIG_CHECK_DEPTH                    2
#define MAX_GOV_ACCOUNTS                       5
#define MAX_EXEC_VOTES                         10
#define MAX_OFFICER_VOTES                      150
#define MAX_EXEC_BUDGET                        asset( 1000000 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT )
#define COLLATERAL_RATIO_DENOM                 (1000)
#define MIN_COLLATERAL_RATIO                   (1001)      // lower than this could result in divide by 0
#define MAX_COLLATERAL_RATIO                   (32000)     // higher than this is unnecessary and may exceed int16 storage
#define MAINTENANCE_COLLATERAL_RATIO           (1750)      // Call when collateral only pays off 175% the debt
#define MAX_SHORT_SQUEEZE_RATIO                (1500)      // Stop calling when collateral only pays off 150% of the debt

#define MARGIN_OPEN_RATIO                      (20 * PERCENT_1)      // Maximum margin loan when a position is opened is 5x the value of collateral equity. 
#define MARGIN_LIQUIDATION_RATIO               (10 * PERCENT_1)      // Liquidate margin positions when their collateralization ratios fall below 10%, position is 4X the value of collateral equity
#define CREDIT_OPEN_RATIO                      (125 * PERCENT_1)     // Credit borrow orders need at least 125% of the value of the loan in collateral. 
#define CREDIT_LIQUIDATION_RATIO               (110 * PERCENT_1)     // Credit borrow orders are liquidated when the value of the collateral falls below 110% of the loan value.
#define CREDIT_INTEREST_RATE                   (5 * PERCENT_1)       // 5% Initial Interest rate for network credit assets
#define CREDIT_MIN_INTEREST                    (PERCENT_1)           // 1% interest PA minimum component on credit pools.
#define CREDIT_VARIABLE_INTEREST               (4* PERCENT_1)        // 4% interest PA variable component on credit pools.
#define CREDIT_INTERVAL_BLOCKS                 (BLOCKS_PER_HOUR)     // Interest payments for credit assets compound hourly.
#define INTEREST_MIN_AMOUNT                    (100)                 // Minimum units of asset required to pay interest in credit pools each block.
#define MARKET_MAX_CREDIT_RATIO                (50 * PERCENT_1)      // Total Margin and borrow positions are limited to 50% of the maximum collateral and debt liquidity. 

#define EQUITY_MIN_PRODUCERS                   (10)                          // Accounts need at least 10 producer votes to claim equity reward. 
#define EQUITY_BOOST_PRODUCERS                 (50)                          // Boost when account has min 50 producer votes
#define EQUITY_ACTIVITY_TIME                   fc::days(30)                  // Dividends available when activity in the last 30 days
#define EQUITY_BOOST_ACTIVITY                  (BLOCKCHAIN_PRECISION * 15)   // Dividends are doubled when account has a rolling average of 15 activity rewards in the last 30 days.
#define EQUITY_BOOST_BALANCE                   (BLOCKCHAIN_PRECISION * 10)   // Dividends are doubled when account has balance greater than 10 units.
#define EQUITY_BOOST_TOP_PERCENT               (10 * PERCENT_1)              // Boost equity reward by 10% for top members
#define POWER_BOOST_TOP_PERCENT                (10 * PERCENT_1)              // Boost power reward by 10% for top members
#define EQUITY_INTERVAL                        fc::days(7)           // Time period of distributing dividends to equity assests
#define EQUITY_INTERVAL_BLOCKS                 uint32_t(EQUITY_INTERVAL.count() / BLOCK_INTERVAL.count() )  // Pays Equity rewards once per week.
#define DIVIDEND_SHARE_PERCENT                 (5 * PERCENT_1)       // Percentage of incoming assets added to the dividends pool
#define LIQUID_DIVIDEND_PERCENT                (10 * PERCENT_1)      // percentage of equity dividends distributed to liquid balances
#define STAKED_DIVIDEND_PERCENT                (80 * PERCENT_1)      // percentage of equity dividends distributed to staked balances
#define SAVINGS_DIVIDEND_PERCENT               (10 * PERCENT_1)      // percentage of equity dividends distirbuted to savings balances 

#define CREDIT_BUYBACK_INTERVAL                fc::days(7)           // Time period of buying back credit assets 
#define CREDIT_BUYBACK_INTERVAL_BLOCKS         uint32_t( CREDIT_BUYBACK_INTERVAL.count() / BLOCK_INTERVAL.count() )  // Time period of buying back credit assets 
#define BUYBACK_SHARE_PERCENT                  (5 * PERCENT_1)       // Percentage of incoming assets added to the buyback pool
#define LIQUID_FIXED_INTEREST_RATE             (1 * PERCENT_1)       // Fixed component of Interest rate of the asset for liquid balances.
#define LIQUID_VARIABLE_INTEREST_RATE          (2 * PERCENT_1)       // Variable component of Interest rate of the asset for liquid balances.
#define STAKED_FIXED_INTEREST_RATE             (1 * PERCENT_1)       // Fixed component of Interest rate of the asset for staked balances.
#define STAKED_VARIABLE_INTEREST_RATE          (10 * PERCENT_1)      // Variable component of Interest rate of the asset for staked balances.
#define SAVINGS_FIXED_INTEREST_RATE            (1 * PERCENT_1)       // Fixed component of Interest rate of the asset for savings balances.
#define SAVINGS_VARIABLE_INTEREST_RATE         (5 * PERCENT_1)       // Variable component of Interest rate of the asset for savings balances.
#define VAR_INTEREST_RANGE                     (50 * PERCENT_1)      // Range of buyback price deviation for max and min variable interest rate.

#define FORCE_SETTLEMENT_DELAY                 fc::days(1)           // 1 day
#define FORCE_SETTLEMENT_OFFSET                0                     // 1%
#define FORCE_SETTLEMENT_MAX_VOLUME            (20 * PERCENT_1)      // 20%
#define PRICE_FEED_LIFETIME                    fc::days(1)           // 1 day
#define MAX_AUTHORITY_MEMBERSHIP               100
#define MAX_ASSET_WHITELIST_AUTHORITIES        1000
#define MAX_ASSET_FEED_PUBLISHERS              100
#define MAX_URL_LENGTH                         127
#define MIN_TRANSACTION_SIZE_LIMIT             1024
#define MAX_TRANSACTION_SIZE                   (1024 * 64)
#define MIN_BLOCK_SIZE_LIMIT                   (MAX_TRANSACTION_SIZE)
#define MAX_BLOCK_SIZE                         (MAX_TRANSACTION_SIZE * 10000) // MAX_TRANSACTION_SIZE*10000 per block
#define MIN_BLOCK_SIZE                         115
#define MIN_FEED_LIFETIME                      fc::seconds(60)
#define MIN_SETTLEMENT_DELAY                   fc::seconds(60)
#define CONNECTION_REQUEST_DURATION            fc::days(7)
#define TRANSFER_REQUEST_DURATION              fc::days(7)
#define RESET_ACCOUNT_DELAY                    fc::days(3)
#define NETWORK_UPDATE_INTERVAL_BLOCKS         uint32_t(fc::minutes(10).count() / BLOCK_INTERVAL.count())
#define MEDIAN_LIQUIDITY_INTERVAL_BLOCKS       uint32_t(fc::minutes(10).count() / BLOCK_INTERVAL.count())
#define MEDIAN_LIQUIDITY_INTERVAL              fc::minutes(10)
#define REP_UPDATE_BLOCK_INTERVAL              (BLOCKS_PER_DAY)
#define METRIC_INTERVAL_BLOCKS                 (BLOCKS_PER_HOUR)      // Updates metrics once per hour
#define METRIC_CALC_TIME                       (fc::days(30))        // Metrics include posts up to 30 days old. 
#define FEED_INTERVAL_BLOCKS                   (BLOCKS_PER_HOUR)     // Updates feeds once per hour
#define FEED_HISTORY_WINDOW                    fc::hours(84)         // 3.5 days
#define MAX_FEED_AGE                           fc::days(30)          // 30 days
#define MIN_FEEDS                              (TOTAL_PRODUCERS / 4) // protects the network from conversions before price has been established
#define MIN_UNDO_HISTORY                       10
#define MAX_UNDO_HISTORY                       10000
#define MAX_INSTANCE_ID                        (uint64_t(-1)>>16)
#define VIRTUAL_SCHEDULE_LAP_LENGTH            fc::uint128::max_value()

/**
 *  Reserved Account IDs with special meaning
 */

#define GENESIS_ACCOUNT_BASE_NAME       account_name_type("producer")
#define PRODUCER_ACCOUNT                account_name_type("producers")              // Represents the current producers
#define NULL_ACCOUNT                    account_name_type("null")                   // Represents the canonical account with NO authority (nobody can access funds in null account)
#define TEMP_ACCOUNT                    account_name_type("temp")                   // Represents the canonical account with WILDCARD authority (anybody can access funds in temp account)
#define PROXY_TO_SELF_ACCOUNT           ""                       // Represents the canonical account for specifying you will vote for directly (as opposed to a proxy)
#define ROOT_POST_PARENT                (account_name_type())    // Represents the canonical root post parent account
#define COMMUNITY_ACCOUNT               account_name_type("community")              // Represents the Community Enterprise Fund
#define EXECUTIVE_ACCOUNT               account_name_type("executive")              // Represents the Executive board
#define DEVELOPMENT_ACCOUNT             account_name_type("development")            // Represents the Development Team
#define MARKETING_ACCOUNT               account_name_type("marketing")              // Represents the Marketing Team
#define ADVOCACY_ACCOUNT                account_name_type("advocacy")               // Represents the Advocacy Team

/**
 * Implementation Specific Config settings for launching entity.
 */

#define INIT_ACCOUNT                    account_name_type("weyoume")                // The initial Executive board account, issuer of equity asset.
#define INIT_ACCOUNT_PASSWORD           "yourpasswordgoeshere"
#define INIT_DETAILS                    "WeYouMe is a social media protocol to enable everyone to share information and value freely."   // Details string of init account.
#define INIT_URL                        "https://www.weyoume.io"
#define INIT_NODE_ENDPOINT              "https://node.weyoume.io"
#define INIT_AUTH_ENDPOINT              "https://auth.weyoume.io"
#define INIT_NOTIFICATION_ENDPOINT      "https://notify.weyoume.io"
#define INIT_IPFS_ENDPOINT              "https://ipfs.weyoume.io"
#define INIT_BITTORRENT_ENDPOINT        "https://bittorrent.weyoume.io"
#define INIT_IMAGE                      "QmfMeLP6uhjEsSitFENcvJwRx6SpNr41ir4XMYGi3hiW1S"
#define INIT_CEO                         account_name_type("harrison.mclean")               // firstname.lastname of launching Chief Executive Officer
#define INIT_COMMUNITY                   community_name_type("general")