/*
 * Copyright (c) 2016 Ezira Network., and contributors.
 */
#pragma once

#define BLOCKCHAIN_VERSION              ( version(0, 19, 5) )
#define BLOCKCHAIN_HARDFORK_VERSION     ( hardfork_version( BLOCKCHAIN_VERSION ) )

#ifdef IS_TEST_NET
#define INIT_PRIVATE_KEY                (fc::ecc::private_key::regenerate(fc::sha256::hash(std::string("init_key"))))
#define INIT_PUBLIC_KEY_STR             (std::string( node::protocol::public_key_type(INIT_PRIVATE_KEY.get_public_key()) ))
#define CHAIN_ID                        (fc::sha256::hash("testnet"))

#define SYMBOL_ECO  									  (uint64_t(3) | (uint64_t('T') << 8) | (uint64_t('E') << 16) | (uint64_t('S') << 24) | (uint64_t('T') << 32) | (uint64_t('S') << 40) ) ///< eCoin/ECO/TESTS with 3 digits of precision
#define SYMBOL_ESCOR  									(uint64_t(6) | (uint64_t('V') << 8) | (uint64_t('E') << 16) | (uint64_t('S') << 24) | (uint64_t('T') << 32) | (uint64_t('S') << 40) ) ///< eScore/ESCOR/VESTS with 6 digits of precision
#define SYMBOL_WYM    							  	(uint64_t(3) | (uint64_t('T') << 8) | (uint64_t('W') << 16) | (uint64_t('Y') << 24) | (uint64_t('M') << 32) | (uint64_t('T') << 40) ) ///< WYM/WEYOUME equity asset with 3 digits of precision
#define SYMBOL_EUSD    									(uint64_t(3) | (uint64_t('T') << 8) | (uint64_t('B') << 16) | (uint64_t('D') << 24) ) ///< Test / eCoin Backed eUSD with 3 digits of precision

#define ADDRESS_PREFIX                  "TWYM"

#define GENESIS_TIME                    (fc::time_point_sec(1451606400))
#define MINING_TIME                     (fc::time_point_sec(1451606400))
#define CASHOUT_WINDOW_SECONDS          (60*60) /// 1 hr
#define CASHOUT_WINDOW_SECONDS_PRE_HF12 (CASHOUT_WINDOW_SECONDS)
#define CASHOUT_WINDOW_SECONDS_PRE_HF17 (CASHOUT_WINDOW_SECONDS)
#define SECOND_CASHOUT_WINDOW           (60*60*24*3) /// 3 days
#define MAX_CASHOUT_WINDOW_SECONDS      (60*60*24) /// 1 day
#define VOTE_CHANGE_LOCKOUT_PERIOD      (60*10) /// 10 minutes
#define UPVOTE_LOCKOUT_HF7              (fc::minutes(1))
#define UPVOTE_LOCKOUT_HF17             (fc::minutes(5))


#define ORIGINAL_MIN_ACCOUNT_CREATION_FEE 0
#define MIN_ACCOUNT_CREATION_FEE          0

#define OWNER_AUTH_RECOVERY_PERIOD                  fc::seconds(60)
#define ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD  fc::seconds(12)
#define OWNER_UPDATE_LIMIT                          fc::seconds(0)
#define OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM 1
#else // IS LIVE EZIRA NETWORK

#define INIT_PUBLIC_KEY_STR             ""
#define CHAIN_ID                        (fc::sha256::hash("tWYM"))

#define SYMBOL_ECO  									  (uint64_t(3) | (uint64_t('E') << 8) | (uint64_t('C') << 16) | (uint64_t('O') << 24) | (uint64_t('T') << 32) ) ///< eCoin with 3 digits of precision
#define SYMBOL_ESCOR  									(uint64_t(6) | (uint64_t('E') << 8) | (uint64_t('S') << 16) | (uint64_t('C') << 24) | (uint64_t('O') << 32) | (uint64_t('R') << 40) ) ///< ESCOR with 6 digits of precision
#define SYMBOL_WYM    									(uint64_t(3) | (uint64_t('E') << 8) | (uint64_t('Z') << 16) | (uint64_t('T') << 24) | (uint64_t('S') << 32) | (uint64_t('T') << 40) ) ///< EZIRA with 3 digits of precision
#define SYMBOL_EUSD    									(uint64_t(3) | (uint64_t('E') << 8) | (uint64_t('Z') << 16) | (uint64_t('D') << 24) | (uint64_t('T') << 32) ) ///< eCoin Backed eUSD with 3 digits of precision

#define ADDRESS_PREFIX                  "TWYM"

#define GENESIS_TIME                    (fc::time_point_sec(0))
#define MINING_TIME                     (fc::time_point_sec(1000))
#define CASHOUT_WINDOW_SECONDS_PRE_HF12 (60*60*24)    /// 1 day
#define CASHOUT_WINDOW_SECONDS_PRE_HF17 (60*60*12)    /// 12 hours
#define CASHOUT_WINDOW_SECONDS          (60*60*24*7)  /// 7 days
#define SECOND_CASHOUT_WINDOW           (60*60*24*30) /// 30 days
#define MAX_CASHOUT_WINDOW_SECONDS      (60*60*24*14) /// 2 weeks
#define VOTE_CHANGE_LOCKOUT_PERIOD      (60*60*2)     /// 2 hours
#define UPVOTE_LOCKOUT_HF7              (fc::minutes(1))
#define UPVOTE_LOCKOUT_HF17             (fc::hours(12))

#define ORIGINAL_MIN_ACCOUNT_CREATION_FEE  0
#define MIN_ACCOUNT_CREATION_FEE           0

#define OWNER_AUTH_RECOVERY_PERIOD                  fc::days(30)
#define ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD  fc::days(1)
#define OWNER_UPDATE_LIMIT                          fc::minutes(60)
#define OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM 3186477

#endif

#define BLOCK_INTERVAL                  3
#define BLOCKS_PER_YEAR                 (365*24*60*60/BLOCK_INTERVAL)
#define BLOCKS_PER_DAY                  (24*60*60/BLOCK_INTERVAL)
#define START_ECO_fund_for_ESCOR_BLOCK             (BLOCKS_PER_DAY * 7)
#define START_MINER_VOTING_BLOCK        (BLOCKS_PER_DAY * 30)

#define INIT_MINER_NAME                 "ezbuilder"
#define NUM_INIT_MINERS                 50
#define NUM_INIT_EXTRAS									0
#define INIT_TIME                       (fc::time_point_sec(0));

#define MAX_WITNESSES                   50

#define MAX_VOTED_WITNESSES_HF0         48
#define MAX_MINER_WITNESSES_HF0         1
#define MAX_RUNNER_WITNESSES_HF0        1

#define MAX_VOTED_WITNESSES_HF17        49
#define MAX_MINER_WITNESSES_HF17        0
#define MAX_RUNNER_WITNESSES_HF17       1

#define HARDFORK_REQUIRED_WITNESSES     0 // 17 of the 21 dpos witnesses (20 elected and 1 virtual time) required for hardfork. This guarantees 75% participation on all subsequent rounds.
#define MAX_TIME_UNTIL_EXPIRATION       (60*60) // seconds,  aka: 1 hour
#define MAX_MEMO_SIZE                   2048
#define MAX_PROXY_RECURSION_DEPTH       4
#define ECO_fund_for_ESCOR_WITHDRAW_INTERVALS_PRE_HF_16 104
#define ECO_fund_for_ESCOR_WITHDRAW_INTERVALS      13
#define ESCOR_WITHDRAW_INTERVAL_SECONDS (60*60*24*7) /// 1 week per interval
#define MAX_WITHDRAW_ROUTES             10
#define SAVINGS_WITHDRAW_TIME        	(fc::days(3))
#define SAVINGS_WITHDRAW_REQUEST_LIMIT  100
#define VOTE_REGENERATION_SECONDS       (5*60*60*24) // 5 day
#define MAX_VOTE_CHANGES                5
#define REVERSE_AUCTION_WINDOW_SECONDS  (60*30) /// 30 minutes
#define MIN_VOTE_INTERVAL_SEC           3
#define VOTE_DUST_THRESHOLD             (50000000)

#define MIN_ROOT_COMMENT_INTERVAL       (fc::seconds(60*5)) // 5 minutes
#define MIN_REPLY_INTERVAL              (fc::seconds(20)) // 20 seconds
#define POST_AVERAGE_WINDOW             (60*60*24u) // 1 day
#define POST_MAX_BANDWIDTH              (4*PERCENT_100) // 2 posts per 1 days, average 1 every 12 hours
#define POST_WEIGHT_CONSTANT            (uint64_t(POST_MAX_BANDWIDTH) * POST_MAX_BANDWIDTH)

#define MAX_accountWitnessVoteS       30

#define PERCENT_100                     10000
#define PERCENT_1                       (PERCENT_100/100)
#define PERCENT_10_OF_PERCENT_1         (PERCENT_100/1000)
#define DEFAULT_EUSD_INTEREST_RATE      (10*PERCENT_1) ///< 10% APR

#define INFLATION_RATE_START_PERCENT    (978) // Fixes block 7,000,000 to 9.5%
#define INFLATION_RATE_STOP_PERCENT     (95) // 0.95%
#define INFLATION_NARROWING_PERIOD      (250000) // Narrow 0.01% every 250k blocks
#define CONTENT_REWARD_PERCENT          (75*PERCENT_1) //75% of inflation, 7.125% inflation
#define ESCOR_fund_PERCENT            	(15*PERCENT_1) //15% of inflation, 1.425% inflation

#define MINER_PAY_PERCENT               (PERCENT_1) // 1%
#define MIN_RATION                      100000
#define MAX_RATION_DECAY_RATE           (1000000)
#define FREE_TRANSACTIONS_WITH_NEW_ACCOUNT 100

#define BANDWIDTH_AVERAGE_WINDOW_SECONDS (60*60*24*7) ///< 1 week
#define BANDWIDTH_PRECISION             (uint64_t(1000000)) ///< 1 million
#define MAX_COMMENT_DEPTH_PRE_HF17      6
#define MAX_COMMENT_DEPTH               0xffff // 64k
#define SOFT_MAX_COMMENT_DEPTH          0xff // 255

#define MAX_RESERVE_RATIO               (20000)

#define CREATE_ACCOUNT_WITH_ECO_MODIFIER 			 30
#define CREATE_ACCOUNT_DELEGATION_RATIO    5
#define CREATE_ACCOUNT_DELEGATION_TIME     fc::days(30)

#define MINING_REWARD                   asset( 10000, SYMBOL_ECO )
#define EQUIHASH_N                      140
#define EQUIHASH_K                      6

#define LIQUIDITY_TIMEOUT_SEC           (fc::seconds(60*60*24*7)) // After one week volume is set to 0
#define MIN_LIQUIDITY_REWARD_PERIOD_SEC (fc::seconds(60)) // 1 minute required on books to receive volume
#define LIQUIDITY_REWARD_PERIOD_SEC     (60*60)
#define LIQUIDITY_REWARD_BLOCKS         (LIQUIDITY_REWARD_PERIOD_SEC/BLOCK_INTERVAL)
#define MIN_LIQUIDITY_REWARD            (asset( 1000*LIQUIDITY_REWARD_BLOCKS, SYMBOL_ECO )) // Minumum reward to be paid out to liquidity providers
#define MIN_CONTENT_REWARD              MINING_REWARD
#define MIN_CURATE_REWARD               MINING_REWARD
#define MIN_PRODUCER_REWARD             MINING_REWARD
#define MIN_POW_REWARD                  MINING_REWARD

#define ACTIVE_CHALLENGE_FEE            asset( 2000, SYMBOL_ECO )
#define OWNER_CHALLENGE_FEE             asset( 30000, SYMBOL_ECO )
#define ACTIVE_CHALLENGE_COOLDOWN       fc::days(1)
#define OWNER_CHALLENGE_COOLDOWN        fc::days(1)

#define POST_REWARD_FUND_NAME           ("post")
#define COMMENT_REWARD_FUND_NAME        ("comment")
#define RECENT_RESCOR_DECAY_RATE_HF17  (fc::days(30))
#define RECENT_RESCOR_DECAY_RATE_HF19  (fc::days(15))
#define CONTENT_CONSTANT_HF0            (uint128_t(uint64_t(2000000000000ll)))
// note, if redefining these constants make sure calculate_claims doesn't overflow

// 5ccc e802 de5f
// int(expm1( log1p( 1 ) / BLOCKS_PER_YEAR ) * 2**APR_PERCENT_SHIFT_PER_BLOCK / 100000 + 0.5)
// we use 100000 here instead of 10000 because we end up creating an additional 9x for eScore held
#define APR_PERCENT_MULTIPLY_PER_BLOCK          ( (uint64_t( 0x5ccc ) << 0x20) \
                                                        | (uint64_t( 0xe802 ) << 0x10) \
                                                        | (uint64_t( 0xde5f )        ) \
                                                        )
// chosen to be the maximal value such that APR_PERCENT_MULTIPLY_PER_BLOCK * 2**64 * 100000 < 2**128
#define APR_PERCENT_SHIFT_PER_BLOCK             87

#define APR_PERCENT_MULTIPLY_PER_ROUND          ( (uint64_t( 0x79cc ) << 0x20 ) \
                                                        | (uint64_t( 0xf5c7 ) << 0x10 ) \
                                                        | (uint64_t( 0x3480 )         ) \
                                                        )

#define APR_PERCENT_SHIFT_PER_ROUND             83

// We have different constants for hourly rewards
// i.e. hex(int(math.expm1( math.log1p( 1 ) / HOURS_PER_YEAR ) * 2**APR_PERCENT_SHIFT_PER_HOUR / 100000 + 0.5))
#define APR_PERCENT_MULTIPLY_PER_HOUR           ( (uint64_t( 0x6cc1 ) << 0x20) \
                                                        | (uint64_t( 0x39a1 ) << 0x10) \
                                                        | (uint64_t( 0x5cbd )        ) \
                                                        )

// chosen to be the maximal value such that APR_PERCENT_MULTIPLY_PER_HOUR * 2**64 * 100000 < 2**128
#define APR_PERCENT_SHIFT_PER_HOUR              77

// These constants add up to GRAPHENE_PERCENT_100.  Each GRAPHENE_PERCENT_1 is equivalent to 1% per year APY
// *including the corresponding 9x eScore held rewards*
#define CURATE_APR_PERCENT              3875
#define CONTENT_APR_PERCENT             3875
#define LIQUIDITY_APR_PERCENT            750
#define PRODUCER_APR_PERCENT             750
#define POW_APR_PERCENT                  750

#define MIN_PAYOUT_EUSD                  (asset(20,SYMBOL_EUSD))

#define EUSD_STOP_PERCENT                (5*PERCENT_1 ) // Stop printing EUSD at 5% Market Cap
#define EUSD_START_PERCENT               (2*PERCENT_1) // Start reducing printing of EUSD at 2% Market Cap

#define MIN_ACCOUNT_NAME_LENGTH          3
#define MAX_ACCOUNT_NAME_LENGTH         16

#define MIN_PERMLINK_LENGTH             0
#define MAX_PERMLINK_LENGTH             256
#define MAX_WITNESS_URL_LENGTH          2048

#define INIT_SUPPLY                     int64_t(10000000000)
#define MAX_ESCOR_SUPPLY                int64_t(1000000000000000ll)
#define MAX_SIG_CHECK_DEPTH             2

#define MIN_TRANSACTION_SIZE_LIMIT      1024
#define SECONDS_PER_YEAR                (uint64_t(60*60*24*365ll))

#define EUSD_INTEREST_COMPOUND_INTERVAL_SEC  (60*60*24*30)
#define MAX_TRANSACTION_SIZE            (1024*64)
#define MIN_BLOCK_SIZE_LIMIT            (MAX_TRANSACTION_SIZE)
#define MAX_BLOCK_SIZE                  (MAX_TRANSACTION_SIZE*BLOCK_INTERVAL*2000)
#define MIN_BLOCK_SIZE                  115
#define BLOCKS_PER_HOUR                 (60*60/BLOCK_INTERVAL)
#define FEED_INTERVAL_BLOCKS            (BLOCKS_PER_HOUR)
#define FEED_HISTORY_WINDOW_PRE_HF_16   (24*7) /// 7 days * 24 hours per day
#define FEED_HISTORY_WINDOW             (12*7) // 3.5 days
#define MAX_FEED_AGE_SECONDS            (60*60*24*7) // 7 days
#define MIN_FEEDS                       (MAX_WITNESSES/3) /// protects the network from conversions before price has been established
#define CONVERSION_DELAY_PRE_HF_16      (fc::days(7))
#define CONVERSION_DELAY                (fc::hours(FEED_HISTORY_WINDOW)) //3.5 day conversion

#define MIN_UNDO_HISTORY                10
#define MAX_UNDO_HISTORY                10000

#define MIN_TRANSACTION_EXPIRATION_LIMIT (BLOCK_INTERVAL * 5) // 5 transactions per block
#define BLOCKCHAIN_PRECISION            uint64_t( 1000 )

#define BLOCKCHAIN_PRECISION_DIGITS     3
#define MAX_INSTANCE_ID                 (uint64_t(-1)>>16)
/** NOTE: making this a power of 2 (say 2^15) would greatly accelerate fee calcs */
#define MAX_AUTHORITY_MEMBERSHIP        10
#define MAX_ASSET_WHITELIST_AUTHORITIES 10
#define MAX_URL_LENGTH                  127

#define IRREVERSIBLE_THRESHOLD          (75 * PERCENT_1)

#define VIRTUAL_SCHEDULE_LAP_LENGTH  ( fc::uint128(uint64_t(-1)) )
#define VIRTUAL_SCHEDULE_LAP_LENGTH2 ( fc::uint128::max_value() )

/**
 *  Reserved Account IDs with special meaning
 */
///@{
/// Represents the current witnesses
#define MINER_ACCOUNT                   "ezbuilders"
/// Represents the canonical account with NO authority (nobody can access funds in null account)
#define NULL_ACCOUNT                    "null"
/// Represents the canonical account with WILDCARD authority (anybody can access funds in temp account)
#define TEMP_ACCOUNT                    "temp"
/// Represents the canonical account for specifying you will vote for directly (as opposed to a proxy)
#define PROXY_TO_SELF_ACCOUNT           ""
/// Represents the canonical root post parent account
#define ROOT_POST_PARENT                (account_name_type())
///@}