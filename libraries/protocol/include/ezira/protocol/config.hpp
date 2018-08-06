/*
 * Copyright (c) 2016 Ezira Network., and contributors.
 */
#pragma once

#define EZIRA_BLOCKCHAIN_VERSION              ( version(0, 19, 5) )
#define EZIRA_BLOCKCHAIN_HARDFORK_VERSION     ( hardfork_version( EZIRA_BLOCKCHAIN_VERSION ) )

#ifdef IS_TEST_NET
#define EZIRA_INIT_PRIVATE_KEY                (fc::ecc::private_key::regenerate(fc::sha256::hash(std::string("init_key"))))
#define EZIRA_INIT_PUBLIC_KEY_STR             (std::string( ezira::protocol::public_key_type(EZIRA_INIT_PRIVATE_KEY.get_public_key()) ))
#define EZIRA_CHAIN_ID                        (fc::sha256::hash("eztestnet"))

#define VESTS_SYMBOL  (uint64_t(6) | (uint64_t('V') << 8) | (uint64_t('E') << 16) | (uint64_t('S') << 24) | (uint64_t('T') << 32) | (uint64_t('S') << 40)) ///< VESTS with 6 digits of precision
#define EZIRA_SYMBOL  (uint64_t(3) | (uint64_t('T') << 8) | (uint64_t('E') << 16) | (uint64_t('S') << 24) | (uint64_t('T') << 32) | (uint64_t('S') << 40)) ///< EZIRA with 3 digits of precision
#define EZD_SYMBOL    (uint64_t(3) | (uint64_t('T') << 8) | (uint64_t('B') << 16) | (uint64_t('D') << 24) ) ///< Test Backed Dollars with 3 digits of precision
#define EZC_SYMBOL   (uint64_t(3) | (uint64_t('T') << 8) | (uint64_t('S') << 16) | (uint64_t('T') << 24) ) ///< Test Dollars with 3 digits of precision

#define EZIRA_ADDRESS_PREFIX                  "EZT"

#define EZIRA_GENESIS_TIME                    (fc::time_point_sec(1451606400))
#define EZIRA_MINING_TIME                     (fc::time_point_sec(1451606400))
#define EZIRA_CASHOUT_WINDOW_SECONDS          (60*60) /// 1 hr
#define EZIRA_CASHOUT_WINDOW_SECONDS_PRE_HF12 (EZIRA_CASHOUT_WINDOW_SECONDS)
#define EZIRA_CASHOUT_WINDOW_SECONDS_PRE_HF17 (EZIRA_CASHOUT_WINDOW_SECONDS)
#define EZIRA_SECOND_CASHOUT_WINDOW           (60*60*24*3) /// 3 days
#define EZIRA_MAX_CASHOUT_WINDOW_SECONDS      (60*60*24) /// 1 day
#define EZIRA_VOTE_CHANGE_LOCKOUT_PERIOD      (60*10) /// 10 minutes
#define EZIRA_UPVOTE_LOCKOUT_HF7              (fc::minutes(1))
#define EZIRA_UPVOTE_LOCKOUT_HF17             (fc::minutes(5))


#define EZIRA_ORIGINAL_MIN_ACCOUNT_CREATION_FEE 0
#define EZIRA_MIN_ACCOUNT_CREATION_FEE          0

#define EZIRA_OWNER_AUTH_RECOVERY_PERIOD                  fc::seconds(60)
#define EZIRA_ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD  fc::seconds(12)
#define EZIRA_OWNER_UPDATE_LIMIT                          fc::seconds(0)
#define EZIRA_OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM 1
#else // IS LIVE EZIRA NETWORK

#define EZIRA_INIT_PUBLIC_KEY_STR             "EZT8AqecLobkeTBmffuuxdhdYG3fttawrtST77cm83DHpVTcyNNzz"
#define EZIRA_CHAIN_ID                        (fc::sha256::hash("eztestnet"))
#define VESTS_SYMBOL  (uint64_t(6) | (uint64_t('V') << 8) | (uint64_t('E') << 16) | (uint64_t('S') << 24) | (uint64_t('T') << 32) | (uint64_t('S') << 40)) ///< VESTS with 6 digits of precision
#define EZIRA_SYMBOL  (uint64_t(3) | (uint64_t('E') << 8) | (uint64_t('Z') << 16) | (uint64_t('I') << 24) | (uint64_t('R') << 32) | (uint64_t('A') << 40)) ///< EZIRA with 3 digits of precision
#define EZD_SYMBOL    (uint64_t(3) | (uint64_t('E') << 8) | (uint64_t('Z') << 16) | (uint64_t('D') << 24) ) ///< EZIRA Backed Dollars with 3 digits of precision
#define EZC_SYMBOL   (uint64_t(3) | (uint64_t('E') << 8) | (uint64_t('Z') << 16) | (uint64_t('C') << 24) ) ///< EZIRA Dollars with 3 digits of precision
#define EZIRA_ADDRESS_PREFIX                  "EZT"

#define EZIRA_GENESIS_TIME                    (fc::time_point_sec(1532229209))
#define EZIRA_MINING_TIME                     (fc::time_point_sec(1532230307))
#define EZIRA_CASHOUT_WINDOW_SECONDS_PRE_HF12 (60*60*24)    /// 1 day
#define EZIRA_CASHOUT_WINDOW_SECONDS_PRE_HF17 (60*60*12)    /// 12 hours
#define EZIRA_CASHOUT_WINDOW_SECONDS          (60*60*24*7)  /// 7 days
#define EZIRA_SECOND_CASHOUT_WINDOW           (60*60*24*30) /// 30 days
#define EZIRA_MAX_CASHOUT_WINDOW_SECONDS      (60*60*24*14) /// 2 weeks
#define EZIRA_VOTE_CHANGE_LOCKOUT_PERIOD      (60*60*2)     /// 2 hours
#define EZIRA_UPVOTE_LOCKOUT_HF7              (fc::minutes(1))
#define EZIRA_UPVOTE_LOCKOUT_HF17             (fc::hours(12))

#define EZIRA_ORIGINAL_MIN_ACCOUNT_CREATION_FEE  0
#define EZIRA_MIN_ACCOUNT_CREATION_FEE           0

#define EZIRA_OWNER_AUTH_RECOVERY_PERIOD                  fc::days(30)
#define EZIRA_ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD  fc::days(1)
#define EZIRA_OWNER_UPDATE_LIMIT                          fc::minutes(60)
#define EZIRA_OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM 3186477

#endif

#define EZIRA_BLOCK_INTERVAL                  3
#define EZIRA_BLOCKS_PER_YEAR                 (365*24*60*60/EZIRA_BLOCK_INTERVAL)
#define EZIRA_BLOCKS_PER_DAY                  (24*60*60/EZIRA_BLOCK_INTERVAL)
#define EZIRA_START_VESTING_BLOCK             (EZIRA_BLOCKS_PER_DAY * 7)
#define EZIRA_START_MINER_VOTING_BLOCK        (EZIRA_BLOCKS_PER_DAY * 30)

#define EZIRA_INIT_MINER_NAME                 "ezbuilder"
#define EZIRA_NUM_INIT_MINERS                 100
#define EZIRA_NUM_ACTIVE_INIT_MINERS					1
#define EZIRA_INIT_TIME                       (fc::time_point_sec());

#define EZIRA_MAX_WITNESSES                   100

#define EZIRA_MAX_VOTED_WITNESSES_HF0         19
#define EZIRA_MAX_MINER_WITNESSES_HF0         1
#define EZIRA_MAX_RUNNER_WITNESSES_HF0        1

#define EZIRA_MAX_VOTED_WITNESSES_HF17        20
#define EZIRA_MAX_MINER_WITNESSES_HF17        0
#define EZIRA_MAX_RUNNER_WITNESSES_HF17       1

#define EZIRA_HARDFORK_REQUIRED_WITNESSES     17 // 17 of the 21 dpos witnesses (20 elected and 1 virtual time) required for hardfork. This guarantees 75% participation on all subsequent rounds.
#define EZIRA_MAX_TIME_UNTIL_EXPIRATION       (60*60) // seconds,  aka: 1 hour
#define EZIRA_MAX_MEMO_SIZE                   2048
#define EZIRA_MAX_PROXY_RECURSION_DEPTH       4
#define EZIRA_VESTING_WITHDRAW_INTERVALS_PRE_HF_16 104
#define EZIRA_VESTING_WITHDRAW_INTERVALS      13
#define EZIRA_VESTING_WITHDRAW_INTERVAL_SECONDS (60*60*24*7) /// 1 week per interval
#define EZIRA_MAX_WITHDRAW_ROUTES             10
#define EZIRA_SAVINGS_WITHDRAW_TIME        	(fc::days(3))
#define EZIRA_SAVINGS_WITHDRAW_REQUEST_LIMIT  100
#define EZIRA_VOTE_REGENERATION_SECONDS       (5*60*60*24) // 5 day
#define EZIRA_MAX_VOTE_CHANGES                5
#define EZIRA_REVERSE_AUCTION_WINDOW_SECONDS  (60*30) /// 30 minutes
#define EZIRA_MIN_VOTE_INTERVAL_SEC           3
#define EZIRA_VOTE_DUST_THRESHOLD             (50000000)

#define EZIRA_MIN_ROOT_COMMENT_INTERVAL       (fc::seconds(60*5)) // 5 minutes
#define EZIRA_MIN_REPLY_INTERVAL              (fc::seconds(20)) // 20 seconds
#define EZIRA_POST_AVERAGE_WINDOW             (60*60*24u) // 1 day
#define EZIRA_POST_MAX_BANDWIDTH              (4*EZIRA_100_PERCENT) // 2 posts per 1 days, average 1 every 12 hours
#define EZIRA_POST_WEIGHT_CONSTANT            (uint64_t(EZIRA_POST_MAX_BANDWIDTH) * EZIRA_POST_MAX_BANDWIDTH)

#define EZIRA_MAX_ACCOUNT_WITNESS_VOTES       30

#define EZIRA_100_PERCENT                     10000
#define EZIRA_1_PERCENT                       (EZIRA_100_PERCENT/100)
#define EZIRA_1_TENTH_PERCENT                 (EZIRA_100_PERCENT/1000)
#define EZIRA_DEFAULT_EZD_INTEREST_RATE       (10*EZIRA_1_PERCENT) ///< 10% APR

#define EZIRA_INFLATION_RATE_START_PERCENT    (978) // Fixes block 7,000,000 to 9.5%
#define EZIRA_INFLATION_RATE_STOP_PERCENT     (95) // 0.95%
#define EZIRA_INFLATION_NARROWING_PERIOD      (250000) // Narrow 0.01% every 250k blocks
#define EZIRA_CONTENT_REWARD_PERCENT          (75*EZIRA_1_PERCENT) //75% of inflation, 7.125% inflation
#define EZIRA_VESTING_FUND_PERCENT            (15*EZIRA_1_PERCENT) //15% of inflation, 1.425% inflation

#define EZIRA_MINER_PAY_PERCENT               (EZIRA_1_PERCENT) // 1%
#define EZIRA_MIN_RATION                      100000
#define EZIRA_MAX_RATION_DECAY_RATE           (1000000)
#define EZIRA_FREE_TRANSACTIONS_WITH_NEW_ACCOUNT 100

#define EZIRA_BANDWIDTH_AVERAGE_WINDOW_SECONDS (60*60*24*7) ///< 1 week
#define EZIRA_BANDWIDTH_PRECISION             (uint64_t(1000000)) ///< 1 million
#define EZIRA_MAX_COMMENT_DEPTH_PRE_HF17      6
#define EZIRA_MAX_COMMENT_DEPTH               0xffff // 64k
#define EZIRA_SOFT_MAX_COMMENT_DEPTH          0xff // 255

#define EZIRA_MAX_RESERVE_RATIO               (20000)

#define EZIRA_CREATE_ACCOUNT_WITH_EZIRA_MODIFIER 30
#define EZIRA_CREATE_ACCOUNT_DELEGATION_RATIO    5
#define EZIRA_CREATE_ACCOUNT_DELEGATION_TIME     fc::days(30)

#define EZIRA_MINING_REWARD                   asset( 1000, EZIRA_SYMBOL )
#define EZIRA_EQUIHASH_N                      140
#define EZIRA_EQUIHASH_K                      6

#define EZIRA_LIQUIDITY_TIMEOUT_SEC           (fc::seconds(60*60*24*7)) // After one week volume is set to 0
#define EZIRA_MIN_LIQUIDITY_REWARD_PERIOD_SEC (fc::seconds(60)) // 1 minute required on books to receive volume
#define EZIRA_LIQUIDITY_REWARD_PERIOD_SEC     (60*60)
#define EZIRA_LIQUIDITY_REWARD_BLOCKS         (EZIRA_LIQUIDITY_REWARD_PERIOD_SEC/EZIRA_BLOCK_INTERVAL)
#define EZIRA_MIN_LIQUIDITY_REWARD            (asset( 1000*EZIRA_LIQUIDITY_REWARD_BLOCKS, EZIRA_SYMBOL )) // Minumum reward to be paid out to liquidity providers
#define EZIRA_MIN_CONTENT_REWARD              EZIRA_MINING_REWARD
#define EZIRA_MIN_CURATE_REWARD               EZIRA_MINING_REWARD
#define EZIRA_MIN_PRODUCER_REWARD             EZIRA_MINING_REWARD
#define EZIRA_MIN_POW_REWARD                  EZIRA_MINING_REWARD

#define EZIRA_ACTIVE_CHALLENGE_FEE            asset( 2000, EZIRA_SYMBOL )
#define EZIRA_OWNER_CHALLENGE_FEE             asset( 30000, EZIRA_SYMBOL )
#define EZIRA_ACTIVE_CHALLENGE_COOLDOWN       fc::days(1)
#define EZIRA_OWNER_CHALLENGE_COOLDOWN        fc::days(1)

#define EZIRA_POST_REWARD_FUND_NAME           ("post")
#define EZIRA_COMMENT_REWARD_FUND_NAME        ("comment")
#define EZIRA_RECENT_RSHARES_DECAY_RATE_HF17  (fc::days(30))
#define EZIRA_RECENT_RSHARES_DECAY_RATE_HF19  (fc::days(15))
#define EZIRA_CONTENT_CONSTANT_HF0            (uint128_t(uint64_t(2000000000000ll)))
// note, if redefining these constants make sure calculate_claims doesn't overflow

// 5ccc e802 de5f
// int(expm1( log1p( 1 ) / BLOCKS_PER_YEAR ) * 2**EZIRA_APR_PERCENT_SHIFT_PER_BLOCK / 100000 + 0.5)
// we use 100000 here instead of 10000 because we end up creating an additional 9x for vesting
#define EZIRA_APR_PERCENT_MULTIPLY_PER_BLOCK          ( (uint64_t( 0x5ccc ) << 0x20) \
                                                        | (uint64_t( 0xe802 ) << 0x10) \
                                                        | (uint64_t( 0xde5f )        ) \
                                                        )
// chosen to be the maximal value such that EZIRA_APR_PERCENT_MULTIPLY_PER_BLOCK * 2**64 * 100000 < 2**128
#define EZIRA_APR_PERCENT_SHIFT_PER_BLOCK             87

#define EZIRA_APR_PERCENT_MULTIPLY_PER_ROUND          ( (uint64_t( 0x79cc ) << 0x20 ) \
                                                        | (uint64_t( 0xf5c7 ) << 0x10 ) \
                                                        | (uint64_t( 0x3480 )         ) \
                                                        )

#define EZIRA_APR_PERCENT_SHIFT_PER_ROUND             83

// We have different constants for hourly rewards
// i.e. hex(int(math.expm1( math.log1p( 1 ) / HOURS_PER_YEAR ) * 2**EZIRA_APR_PERCENT_SHIFT_PER_HOUR / 100000 + 0.5))
#define EZIRA_APR_PERCENT_MULTIPLY_PER_HOUR           ( (uint64_t( 0x6cc1 ) << 0x20) \
                                                        | (uint64_t( 0x39a1 ) << 0x10) \
                                                        | (uint64_t( 0x5cbd )        ) \
                                                        )

// chosen to be the maximal value such that EZIRA_APR_PERCENT_MULTIPLY_PER_HOUR * 2**64 * 100000 < 2**128
#define EZIRA_APR_PERCENT_SHIFT_PER_HOUR              77

// These constants add up to GRAPHENE_100_PERCENT.  Each GRAPHENE_1_PERCENT is equivalent to 1% per year APY
// *including the corresponding 9x vesting rewards*
#define EZIRA_CURATE_APR_PERCENT              3875
#define EZIRA_CONTENT_APR_PERCENT             3875
#define EZIRA_LIQUIDITY_APR_PERCENT            750
#define EZIRA_PRODUCER_APR_PERCENT             750
#define EZIRA_POW_APR_PERCENT                  750

#define EZIRA_MIN_PAYOUT_EZD                  (asset(20,EZD_SYMBOL))

#define EZIRA_EZD_STOP_PERCENT                (5*EZIRA_1_PERCENT ) // Stop printing EZD at 5% Market Cap
#define EZIRA_EZD_START_PERCENT               (2*EZIRA_1_PERCENT) // Start reducing printing of EZD at 2% Market Cap

#define EZIRA_MIN_ACCOUNT_NAME_LENGTH          3
#define EZIRA_MAX_ACCOUNT_NAME_LENGTH         16

#define EZIRA_MIN_PERMLINK_LENGTH             0
#define EZIRA_MAX_PERMLINK_LENGTH             256
#define EZIRA_MAX_WITNESS_URL_LENGTH          2048

#define EZIRA_INIT_SUPPLY                     int64_t(0)
#define EZIRA_MAX_SHARE_SUPPLY                int64_t(1000000000000000ll)
#define EZIRA_MAX_SIG_CHECK_DEPTH             2

#define EZIRA_MIN_TRANSACTION_SIZE_LIMIT      1024
#define EZIRA_SECONDS_PER_YEAR                (uint64_t(60*60*24*365ll))

#define EZIRA_EZD_INTEREST_COMPOUND_INTERVAL_SEC  (60*60*24*30)
#define EZIRA_MAX_TRANSACTION_SIZE            (1024*64)
#define EZIRA_MIN_BLOCK_SIZE_LIMIT            (EZIRA_MAX_TRANSACTION_SIZE)
#define EZIRA_MAX_BLOCK_SIZE                  (EZIRA_MAX_TRANSACTION_SIZE*EZIRA_BLOCK_INTERVAL*2000)
#define EZIRA_MIN_BLOCK_SIZE                  115
#define EZIRA_BLOCKS_PER_HOUR                 (60*60/EZIRA_BLOCK_INTERVAL)
#define EZIRA_FEED_INTERVAL_BLOCKS            (EZIRA_BLOCKS_PER_HOUR)
#define EZIRA_FEED_HISTORY_WINDOW_PRE_HF_16   (24*7) /// 7 days * 24 hours per day
#define EZIRA_FEED_HISTORY_WINDOW             (12*7) // 3.5 days
#define EZIRA_MAX_FEED_AGE_SECONDS            (60*60*24*7) // 7 days
#define EZIRA_MIN_FEEDS                       (EZIRA_MAX_WITNESSES/3) /// protects the network from conversions before price has been established
#define EZIRA_CONVERSION_DELAY_PRE_HF_16      (fc::days(7))
#define EZIRA_CONVERSION_DELAY                (fc::hours(EZIRA_FEED_HISTORY_WINDOW)) //3.5 day conversion

#define EZIRA_MIN_UNDO_HISTORY                10
#define EZIRA_MAX_UNDO_HISTORY                10000

#define EZIRA_MIN_TRANSACTION_EXPIRATION_LIMIT (EZIRA_BLOCK_INTERVAL * 5) // 5 transactions per block
#define EZIRA_BLOCKCHAIN_PRECISION            uint64_t( 1000 )

#define EZIRA_BLOCKCHAIN_PRECISION_DIGITS     3
#define EZIRA_MAX_INSTANCE_ID                 (uint64_t(-1)>>16)
/** NOTE: making this a power of 2 (say 2^15) would greatly accelerate fee calcs */
#define EZIRA_MAX_AUTHORITY_MEMBERSHIP        10
#define EZIRA_MAX_ASSET_WHITELIST_AUTHORITIES 10
#define EZIRA_MAX_URL_LENGTH                  127

#define EZIRA_IRREVERSIBLE_THRESHOLD          (75 * EZIRA_1_PERCENT)

#define VIRTUAL_SCHEDULE_LAP_LENGTH  ( fc::uint128(uint64_t(-1)) )
#define VIRTUAL_SCHEDULE_LAP_LENGTH2 ( fc::uint128::max_value() )

/**
 *  Reserved Account IDs with special meaning
 */
///@{
/// Represents the current witnesses
#define EZIRA_MINER_ACCOUNT                   "miners"
/// Represents the canonical account with NO authority (nobody can access funds in null account)
#define EZIRA_NULL_ACCOUNT                    "null"
/// Represents the canonical account with WILDCARD authority (anybody can access funds in temp account)
#define EZIRA_TEMP_ACCOUNT                    "temp"
/// Represents the canonical account for specifying you will vote for directly (as opposed to a proxy)
#define EZIRA_PROXY_TO_SELF_ACCOUNT           ""
/// Represents the canonical root post parent account
#define EZIRA_ROOT_POST_PARENT                (account_name_type())
///@}
