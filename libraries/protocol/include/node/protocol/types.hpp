#pragma once

#include <node/protocol/config.hpp>
#include <node/protocol/fixed_string.hpp>

#include <fc/container/flat_fwd.hpp>
#include <fc/io/varint.hpp>
#include <fc/io/enum_type.hpp>
#include <fc/crypto/sha224.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/optional.hpp>
#include <fc/safe.hpp>
#include <fc/container/flat.hpp>
#include <fc/string.hpp>
#include <fc/io/raw.hpp>
#include <fc/uint128.hpp>
#include <fc/static_variant.hpp>
#include <fc/smart_ref_fwd.hpp>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/rational.hpp>

#include <memory>
#include <vector>
#include <deque>
#include <cstdint>
#include <ctime>

namespace node {

   using                                    fc::uint128_t;
   typedef boost::multiprecision::int128_t  int128_t;
   typedef boost::multiprecision::int256_t  int256_t;
   typedef boost::multiprecision::uint256_t uint256_t;
   typedef boost::multiprecision::uint512_t uint512_t;

   using                               std::map;
   using                               std::vector;
   using                               std::unordered_map;
   using                               std::string;
   using                               std::deque;
   using                               std::shared_ptr;
   using                               std::weak_ptr;
   using                               std::unique_ptr;
   using                               std::set;
   using                               std::pair;
   using                               std::enable_shared_from_this;
   using                               std::time_t;
   using                               std::tie;
   using                               std::make_pair;
   
   using                               fc::smart_ref;
   using                               fc::variant_object;
   using                               fc::variant;
   using                               fc::enum_type;
   using                               fc::optional;
   using                               fc::unsigned_int;
   using                               fc::signed_int;
   using                               fc::time_point_sec;
   using                               fc::time_point;
   using                               fc::safe;
   using                               fc::flat_map;
   using                               fc::flat_set;
   using                               fc::static_variant;
   using                               fc::ecc::range_proof_type;
   using                               fc::ecc::range_proof_info;
   using                               fc::ecc::commitment_type;
   using                               fc::ecc::blind_factor_type;

   struct void_t{};

   namespace protocol {

      typedef fc::ecc::private_key              private_key_type;
      typedef fc::sha256                        chain_id_type;
      typedef fixed_string_32                   account_name_type;
      typedef fixed_string_32                   community_name_type;
      typedef fixed_string_32                   tag_name_type;
      typedef fixed_string_32                   asset_symbol_type;
      typedef fixed_string_32                   graph_node_name_type;
      typedef fixed_string_32                   graph_edge_name_type;
      typedef fc::sha256                        block_id_type;
      typedef fc::ripemd160                     checksum_type;
      typedef fc::sha256                        transaction_id_type;
      typedef fc::sha256                        digest_type;
      typedef fc::ecc::compact_signature        signature_type;
      typedef safe<int64_t>                     share_type;
      typedef safe<int128_t>                    share_128_type;
      typedef uint16_t                          weight_type;
      typedef boost::rational<int32_t>          ratio_type;


      /**
       * Privacy levels of communities, used for determining encryption levels, and access controls for community administration.
       */
      enum class community_privacy_type : int
      {
         OPEN_PUBLIC_COMMUNITY,         ///< All Users can read, interact, post, and request to join. Accounts cannot be blacklisted.
         GENERAL_PUBLIC_COMMUNITY,      ///< All Users can read, interact, post, and request to join.
         EXCLUSIVE_PUBLIC_COMMUNITY,    ///< All users can read, interact, and request to join. Members can post and invite.
         CLOSED_PUBLIC_COMMUNITY,       ///< All users can read, and request to join. Members can interact, post, and invite.
         OPEN_PRIVATE_COMMUNITY,        ///< Members can read and interact, and create posts. Moderators can invite and accept.
         GENERAL_PRIVATE_COMMUNITY,     ///< Members can read and interact, and create posts. Moderators can invite and accept. Cannot share posts.
         EXCLUSIVE_PRIVATE_COMMUNITY,   ///< Members can read and interact, and post. Cannot share posts or request to join. Admins can invite and accept.
         CLOSED_PRIVATE_COMMUNITY       ///< Members can read and interact. Moderators can post. Cannot share posts or request to join. Admins can invite and accept.
      };

      const static vector< string > community_privacy_values = 
      {
         "open_public",
         "general_public",
         "exclusive_public",
         "closed_public",
         "open_private",
         "general_private",
         "exclusive_private",
         "closed_private"
      };

      /**
       * Business account types, used for determining access controls for signatories of business account transactions and equity assets.
       */
      enum class business_structure_type : int
      {
         OPEN_BUSINESS,     ///< All equity holders can become members and vote for officers and executives, all officers can sign transactions.
         PUBLIC_BUSINESS,   ///< Executives select officers, officers can invite and accept members. All Equity holders can request membership. Members vote for executives.
         PRIVATE_BUSINESS   ///< CEO selects Executives, Executives can sign transactions, and invite members. Only members may hold equity assets.
      };

      const static vector< string > business_structure_values = 
      {
         "open",
         "public",
         "private"
      };

      /** 
       * Types of membership available, each offers additional features, and improves the user experience for more advanced users.
       */
      enum class membership_tier_type : int 
      {
         NONE,                 ///< Account does not have membership, free user
         STANDARD_MEMBERSHIP,  ///< Regular membership, removes advertising and offers voting benefits.
         MID_MEMBERSHIP,       ///< Mid level membership, featured page eligibility.
         TOP_MEMBERSHIP        ///< Enterprise membership, enables governance account and interface account and pro application suite.
      };

      const static vector< string > membership_tier_values =
      {
         "none",
         "standard",
         "mid",
         "top"
      };

      /**
       * Types of network officers, each receive reward distributions from the network upon voter approval.
       */
      enum class network_officer_role_type : int 
      {
         DEVELOPMENT,  ///< Creates and maintains core network software and applications.
         MARKETING,    ///< Markets the network to the public, creates awareness and adoption campaigns.
         ADVOCACY      ///< Advocates the network to business partners, investors, and supports businesses using the protocol.  
      };

      const static vector< string > network_officer_role_values =
      {
         "development",
         "marketing",
         "advocacy"
      };

      /** 
       * Types of network officers, each receive reward distributions from the network upon voter approval.
       */
      enum class executive_role_type : int
      {
         CHIEF_EXECUTIVE_OFFICER,    ///< Overall leader of Executive team.
         CHIEF_OPERATING_OFFICER,    ///< Manages communications and coordination of team.
         CHIEF_FINANCIAL_OFFICER,    ///< Oversees Credit issuance and salaries.
         CHIEF_DEVELOPMENT_OFFICER,  ///< Oversees protocol development and upgrades.
         CHIEF_TECHNOLOGY_OFFICER,   ///< Manages front end interface stability and network infrastructure.
         CHIEF_SECURITY_OFFICER,     ///< Manages security of servers, funds, wallets, keys, and cold storage.
         CHIEF_GOVERNANCE_OFFICER,   ///< Manages Governance account moderation teams.
         CHIEF_MARKETING_OFFICER,    ///< Manages public facing communication and campaigns.
         CHIEF_DESIGN_OFFICER,       ///< Oversees graphical design and User interface design.
         CHIEF_ADVOCACY_OFFICER      ///< Coordinates advocacy efforts and teams.
      };

      const static vector< string > executive_role_values =
      {
         "executive",
         "operating",
         "financial",
         "development",
         "technology",
         "security",
         "governance",
         "marketing",
         "design",
         "advocacy"
      };

      /**
       * Types of Product Auction sale, varying the mechanism of determining the bidding prices of the item being sold.
       */
      enum class product_auction_type : int
      {
         OPEN_AUCTION,                    ///< Product is for sale to the highest bidder. Bids are open and visible publicly.
         REVERSE_AUCTION,                 ///< Product is for sale at a linearly decreasing price to the first purchaser.
         CONCEALED_FIRST_PRICE_AUCTION,   ///< Product is for sale at the price of the highest bidder. Bids are secret using commit and reveal process. 
         CONCEALED_SECOND_PRICE_AUCTION   ///< Product is for sale at the price of the second highest bidder. Bids are secret using commit and reveal process. 
      };

      const static vector< string > product_auction_values =
      {
         "open",
         "reverse",
         "concealed_first_price",
         "concealed_second_price"
      };

      /**
       * Types of Asset, each is used for specific functions and operation types, and has different properties.
       */
      enum class asset_property_type : int
      {
         CURRENCY_ASSET,         ///< Cryptocurrency that is issued by the network, starts from zero supply, issuing account is the null account, cannot be issued by any accounts. 
         STANDARD_ASSET,         ///< Regular asset, can be transferred and staked, saved, and delegated.
         STABLECOIN_ASSET,       ///< Asset backed by collateral that tracks the value of an external asset. Redeemable at any time with settlement.
         EQUITY_ASSET,           ///< Asset issued by a business account that distributes a dividend from incoming revenue, and has voting power over a business accounts transactions.
         BOND_ASSET,             ///< Asset issued by a business account, partially backed by collateral, that pays a coupon rate and is redeemed after maturity.
         CREDIT_ASSET,           ///< Asset issued by a business account that is backed by repayments up to a face value, and interest payments.
         STIMULUS_ASSET,         ///< Asset issued by a business account with expiring balances that is distributed to a set of accounts on regular intervals.
         LIQUIDITY_POOL_ASSET,   ///< Asset that is backed by the deposits of an asset pair's liquidity pool and earns trading fees. 
         CREDIT_POOL_ASSET,      ///< Asset that is backed by deposits of the base asset, used for borrowing funds from the pool, used as collateral to borrow base asset.
         OPTION_ASSET,           ///< Asset that enables the execution of a trade at a specific strike price until an expiration date. 
         PREDICTION_ASSET,       ///< Asset backed by an underlying collateral claim, on the condition that a prediction market resolve in a particular outcome.
         GATEWAY_ASSET,          ///< Asset backed by deposits with an exchange counterparty of another asset or currency.
         UNIQUE_ASSET            ///< Asset with a supply of one, contains metadata relating to the ownership of a unique non-fungible asset.
      };

      const static vector< string > asset_property_values =
      {
         "currency",
         "standard",
         "stablecoin",
         "equity",
         "bond",
         "credit",
         "stimulus",
         "liquidity_pool",
         "credit_pool",
         "option",
         "prediction",
         "gateway",
         "unique"
      };

      /**
       * Type of advertising format, determines how creative is formatted in interfaces.
       */
      enum class ad_format_type : int
      {
         STANDARD_FORMAT,      ///< A regular post, objective is permlink.
         PREMIUM_FORMAT,       ///< A premium post, objective is permlink.
         PRODUCT_FORMAT,       ///< A product post, objective is permlink.
         LINK_FORMAT,          ///< A link to an external webpage, objective is URL.
         ACCOUNT_FORMAT,       ///< An account, objective is account name.
         COMMUNITY_FORMAT,     ///< A community, objective is community name.
         ASSET_FORMAT          ///< An asset, objective is asset symbol.
      };

      const static vector< string > ad_format_values =
      {
         "standard",
         "premium",
         "product",
         "link",
         "account",
         "community",
         "asset"
      };

      /**
       * Type of Post, each loaded and displayed accordingly on interfaces.
       */
      enum class post_format_type : int 
      {
         TEXT_POST,        ///< A post containing a maxmium of 300 characters of text.
         IMAGE_POST,       ///< A post containing an IPFS media file of an image, and up to 1000 characters of description text.
         GIF_POST,         ///< A post containing an IPFS media file of a GIF, and up to 1000 characters of description text.
         VIDEO_POST,       ///< A post containing a title, an IPFS media file or bittorrent magent link of a video, and up to 1000 characters of description text.
         LINK_POST,        ///< A post containing a URL link, link title, and up to 1000 characters of description text.
         ARTICLE_POST,     ///< A post containing a title, a header image, and an unlimited amount of body text with embedded images.
         AUDIO_POST,       ///< A post containing a title, an IPFS link to an audio file, and up to 1000 characters of description text.
         FILE_POST,        ///< A post containing a title, either an IPFS link to a file, or a magnet link to a bittorrent swarm for a file, and up to 1000 characters of description text.
         LIVESTREAM_POST,  ///< A post containing a title, a link to a livestreaming video, and up to 1000 characters of description text.
      };

      const static vector< string > post_format_values =
      {
         "text",
         "image",
         "gif",
         "video",
         "link",
         "article",
         "audio",
         "file",
         "livestream"
      };

      /**
       * Types of expense metrics for advertising transactions.
       */
      enum class ad_metric_type : int 
      {
         VIEW_METRIC,      ///< View transaction required for delivery.
         VOTE_METRIC,      ///< vote transaction required for delivery.
         SHARE_METRIC,     ///< share transaction required for delivery.
         FOLLOW_METRIC,    ///< follow or community join transaction required for delivery.
         PURCHASE_METRIC,  ///< product marketplace purchase orders required for delivery. 
         PREMIUM_METRIC    ///< premium content purchases transaction ids required for delivery.
      };

      const static vector< string > ad_metric_values =
      {
         "view",
         "vote",
         "share",
         "follow",
         "purchase",
         "premium"
      };

      /**
       * Types of connections between accounts, each offers a higher level of security for private information exchanges.
       */ 
      enum class connection_tier_type : int
      {
         PUBLIC,       ///< No privacy setting, post is public.
         CONNECTION,   ///< Standard connection level, enables private messaging and viewing private posts.
         FRIEND,       ///< Elevated connection level, activates notifications when posts are made by friends.
         COMPANION,    ///< Highest connection level, maximum privacy for close friends or partners.
         SECURE        ///< Fully Private setting, only the author can read.
      };

      const static vector< string > connection_tier_values =
      {
         "public",
         "connection",
         "friend",
         "companion",
         "secure"
      };

      /**
       * Types of feeds for subscribing to the posts of different sets of users.
       */
      enum class feed_reach_type : int
      { 
         NO_FEED,           ///< Posts that should not be distributed to any feeds.
         FOLLOW_FEED,       ///< Feed from accounts that are followed.
         MUTUAL_FEED,       ///< Feed from accounts that are mutually followed.
         CONNECTION_FEED,   ///< Feed from accounts that are connected.
         FRIEND_FEED,       ///< Feed from accounts that are friends.
         COMPANION_FEED,    ///< Feed from accounts that are companions.
         COMMUNITY_FEED,    ///< Feed from subscribed communities.
         TAG_FEED           ///< Feed from followed tags.
      };

      const static vector< string > feed_reach_values =
      { 
         "none",
         "follow",
         "mutual",
         "connection",
         "friend",
         "companion",
         "community",
         "tag"
      };

      /**
       * Types of blogs for tracking what has been created or shared with an account, community, or tag. 
       */
      enum class blog_reach_type : int
      { 
         ACCOUNT_BLOG,      ///< Blog within an account, includes authored posts and shared posts
         COMMUNITY_BLOG,    ///< Blog within a community, includes all posts within the community, plus shared with the community.
         TAG_BLOG           ///< Blog within a tag, includes all posts that use the tag, plus shared with the tag.
      };

      const static vector< string > blog_reach_values =
      { 
         "account",
         "community",
         "tag"
      };

      /**
       * Time weighting options for sorting discusions by index.
       */
      enum class sort_time_type : int
      {
         ACTIVE_TIME,
         RAPID_TIME,
         STANDARD_TIME,
         TOP_TIME,
         ELITE_TIME
      };

      const static vector< string > sort_time_values = 
      {
         "active",
         "rapid",
         "standard",
         "top",
         "elite"
      };

      /**
       * Variety of options for sorting discussions by index according to Votes, Views, Shares, and Comments.
       */
      enum class sort_option_type : int
      {
         VOTES_SORT,
         VIEWS_SORT,
         SHARES_SORT,
         COMMENTS_SORT,
         QUALITY_SORT,
         POPULAR_SORT,
         VIRAL_SORT,
         DISCUSSION_SORT,
         PROMINENT_SORT,
         CONVERSATION_SORT,
         DISCOURSE_SORT
      };

      const static vector< string > sort_option_values =
      {
         "votes",
         "views",
         "shares",
         "comments",
         "quality",
         "popular",
         "viral",
         "discussion",
         "prominent",
         "conversation",
         "discourse"
      };

      /**
       * Time lengths that can be used for filtering posts in discussion API calls.
       */
      enum class post_time_type : int
      {
         ALL_TIME,
         LAST_HOUR,
         LAST_DAY,
         LAST_WEEK,
         LAST_MONTH,
         LAST_YEAR
      };

      const static vector< string > post_time_values =
      {
         "all",
         "hour",
         "day",
         "week",
         "month",
         "year"
      };

      /**
       * Account Balance types that can be specified in asset units.
       */
      enum class account_balance_type : int
      {
         LIQUID_BALANCE,
         STAKED_BALANCE,
         REWARD_BALANCE,
         SAVINGS_BALANCE,
         VESTING_BALANCE,
         DELEGATED_BALANCE,
         RECEIVING_BALANCE
      };

      const static vector< string > account_balance_values =
      {
         "liquid",
         "staked",
         "reward",
         "savings",
         "vesting",
         "delegated",
         "receiving"
      };

      enum class asset_issuer_permission_flags : int
      {
         balance_whitelist           = 1,       ///< Accounts must be whitelisted in order to send, receive or hold the asset.
         trade_whitelist             = 2,       ///< Accounts must be whitelisted to trade the asset.
         maker_restricted            = 4,       ///< Only issuer may create new trading orders onto the orderbook, others must fill them.
         issuer_accept_requests      = 8,       ///< Issuer may approve transfer requests, enabling them to retrieve funds from any account.
         transfer_restricted         = 16,      ///< Transfers may only be to or from the issuer.
         disable_requests            = 32,      ///< Payment requests are disabled.
         disable_recurring           = 64,      ///< Recurring payments are disabled.
         disable_credit              = 128,     ///< The asset cannot be lent into a credit pool, Disabling margin and credit loan orders.
         disable_liquid              = 256,     ///< The asset cannot be used to create a liquidity pool.
         disable_options             = 512,     ///< The asset cannot be used to issue options assets.
         disable_escrow              = 1024,    ///< Disable escrow transfers and marketplace trades using the asset.
         disable_force_settle        = 2048,    ///< Disable force settling of stablecoins, only global settle may return collateral.
         disable_confidential        = 4096,    ///< Asset cannot be used with confidential transactions.
         disable_auction             = 8192,    ///< Disable creation of auction orders for the asset.
         producer_fed_asset          = 16384,   ///< Allow the asset to be price food to be published by top voting producers.
         global_settle               = 32768,   ///< Allow the stablecoin issuer to force a global settlement: Set in permissions, but not flags.
         governance_oversight        = 65536,   ///< Asset update, issuer transfer and issuance require the account's governance address to approve.
         immutable_properties        = 131072   ///< Disable any future asset options updates or changes to flags.
      };

      const static uint32_t ASSET_ISSUER_PERMISSION_MASK =
         int( asset_issuer_permission_flags::balance_whitelist )
         | int( asset_issuer_permission_flags::trade_whitelist )
         | int( asset_issuer_permission_flags::maker_restricted )
         | int( asset_issuer_permission_flags::issuer_accept_requests )
         | int( asset_issuer_permission_flags::transfer_restricted )
         | int( asset_issuer_permission_flags::disable_requests )
         | int( asset_issuer_permission_flags::disable_recurring )
         | int( asset_issuer_permission_flags::disable_credit )
         | int( asset_issuer_permission_flags::disable_liquid )
         | int( asset_issuer_permission_flags::disable_options )
         | int( asset_issuer_permission_flags::disable_escrow )
         | int( asset_issuer_permission_flags::disable_force_settle )
         | int( asset_issuer_permission_flags::disable_confidential )
         | int( asset_issuer_permission_flags::disable_auction )
         | int( asset_issuer_permission_flags::producer_fed_asset )
         | int( asset_issuer_permission_flags::global_settle )
         | int( asset_issuer_permission_flags::governance_oversight )
         | int( asset_issuer_permission_flags::immutable_properties );

      enum class community_permission_flags : int
      {
         member_whitelist            = 1,        ///< Accounts must be whitelisted by the founder to request membership or be invited.
         require_profile             = 2,        ///< Accounts must have a valid profile data to request membership or be invited. 
         require_verified            = 4,        ///< Accounts must have a valid verification from an existing member to request membership or be invited.
         disable_messages            = 8,        ///< Accounts cannot send community messages.
         disable_text_posts          = 16,       ///< Community does not allow text type posts.
         disable_image_posts         = 32,       ///< Community does not allow image type posts.
         disable_gif_posts           = 64,       ///< Community does not allow image type posts.
         disable_video_posts         = 128,      ///< Community does not allow video type posts.
         disable_link_posts          = 256,      ///< Community does not allow link type posts.
         disable_article_posts       = 512,      ///< Community does not allow article type posts.
         disable_audio_posts         = 1024,     ///< Community does not allow audio type posts.
         disable_file_posts          = 2048,     ///< Community does not allow file type posts.
         disable_livestream_posts    = 4096     ///< Community does not allow livestream type posts.
      };

      const static uint32_t COMMUNITY_PERMISSION_MASK =
         int( community_permission_flags::member_whitelist )
         | int( community_permission_flags::require_profile )
         | int( community_permission_flags::require_verified )
         | int( community_permission_flags::disable_messages )
         | int( community_permission_flags::disable_text_posts )
         | int( community_permission_flags::disable_image_posts )
         | int( community_permission_flags::disable_gif_posts )
         | int( community_permission_flags::disable_video_posts )
         | int( community_permission_flags::disable_link_posts )
         | int( community_permission_flags::disable_article_posts )
         | int( community_permission_flags::disable_audio_posts )
         | int( community_permission_flags::disable_file_posts )
         | int( community_permission_flags::disable_livestream_posts );

      struct public_key_type
      {
         struct binary_key
         {
            binary_key() {}
            uint32_t                 check = 0;
            fc::ecc::public_key_data data;
         };
         fc::ecc::public_key_data key_data;
         public_key_type();
         public_key_type( const fc::ecc::public_key_data& data );
         public_key_type( const fc::ecc::public_key& pubkey );
         explicit public_key_type( const std::string& base58str );
         operator fc::ecc::public_key_data() const;
         operator fc::ecc::public_key() const;
         explicit operator std::string() const;
         friend bool operator == ( const public_key_type& p1, const fc::ecc::public_key& p2 );
         friend bool operator == ( const public_key_type& p1, const public_key_type& p2 );
         friend bool operator < ( const public_key_type& p1, const public_key_type& p2 ) { return p1.key_data < p2.key_data; }
         friend bool operator != ( const public_key_type& p1, const public_key_type& p2 );
      };

      #define INIT_PUBLIC_KEY (node::protocol::public_key_type(INIT_PUBLIC_KEY_STR))

      struct extended_public_key_type
      {
         struct binary_key
         {
            binary_key() {}
            uint32_t                   check = 0;
            fc::ecc::extended_key_data data;
         };

         fc::ecc::extended_key_data key_data;

         extended_public_key_type();
         extended_public_key_type( const fc::ecc::extended_key_data& data );
         extended_public_key_type( const fc::ecc::extended_public_key& extpubkey );
         explicit extended_public_key_type( const std::string& base58str );
         operator fc::ecc::extended_public_key() const;
         explicit operator std::string() const;
         friend bool operator == ( const extended_public_key_type& p1, const fc::ecc::extended_public_key& p2);
         friend bool operator == ( const extended_public_key_type& p1, const extended_public_key_type& p2);
         friend bool operator != ( const extended_public_key_type& p1, const extended_public_key_type& p2);
      };

      struct extended_private_key_type
      {
         struct binary_key
         {
            binary_key() {}
            uint32_t                   check = 0;
            fc::ecc::extended_key_data data;
         };

         fc::ecc::extended_key_data key_data;

         extended_private_key_type();
         extended_private_key_type( const fc::ecc::extended_key_data& data );
         extended_private_key_type( const fc::ecc::extended_private_key& extprivkey );
         explicit extended_private_key_type( const std::string& base58str );
         operator fc::ecc::extended_private_key() const;
         explicit operator std::string() const;
         friend bool operator == ( const extended_private_key_type& p1, const fc::ecc::extended_private_key& p2);
         friend bool operator == ( const extended_private_key_type& p1, const extended_private_key_type& p2);
         friend bool operator != ( const extended_private_key_type& p1, const extended_private_key_type& p2);
      };

      /**
       * An Encrypted Keypair enables the effective sharing 
       * of private information with an account, 
       * by enclosing a communication private key, 
       * corresponding to a specified public key, 
       * encrypted with a known secure public key of that account, 
       * with which it can decrypt the private key.
       */
      struct encrypted_keypair_type
      {
         encrypted_keypair_type();
         encrypted_keypair_type( const public_key_type& secure_key, const public_key_type& public_key, const std::string& encrypted_private_key );

         public_key_type          secure_key;                 ///< The public key used to encrypt the encrypted key.

         public_key_type          public_key;                 ///< The public key of the private encrypted key.

         string                   encrypted_private_key;      ///< The encrypted private key of the public key.
         
         friend bool operator == ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 );
         friend bool operator < ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 );
         friend bool operator > ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 );
         friend bool operator != ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 );
         friend bool operator <= ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 );
         friend bool operator >= ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 );
      };

      /**
       * Structure for holding the value of the current date.
       */
      struct date_type
      {
         date_type();
         date_type( uint16_t day, uint16_t month, uint16_t year );
         date_type( time_point time );

         uint16_t      day = 1;         ///< Day of the month [1-31]

         uint16_t      month = 1;       ///< Month of the year [1-12]

         uint16_t      year = 1970;        ///< Year [1970+]

         bool is_null()const;

         void validate()const;

         string to_string()const;

         void next();

         operator fc::time_point()const;
         friend bool operator == ( const date_type& date1, const date_type& date2 );
         friend bool operator < ( const date_type& date1, const date_type& date2 );
         friend bool operator > ( const date_type& date1, const date_type& date2 );
         friend bool operator != ( const date_type& date1, const date_type& date2 );
         friend bool operator <= ( const date_type& date1, const date_type& date2 );
         friend bool operator >= ( const date_type& date1, const date_type& date2 );
      };


      /**
       * This data is encrypted and stored in the
       * encrypted memo portion of the blind output.
       */
      struct blind_memo
      {
         account_name_type         from;

         share_type                amount;

         string                    message;

         uint32_t                  check = 0;
      };


      

} };  // node::protocol

namespace fc
{
   void to_variant( const node::protocol::public_key_type& var,  fc::variant& vo );
   void from_variant( const fc::variant& var,  node::protocol::public_key_type& vo );
   void to_variant( const node::protocol::extended_public_key_type& var, fc::variant& vo );
   void from_variant( const fc::variant& var, node::protocol::extended_public_key_type& vo );
   void to_variant( const node::protocol::extended_private_key_type& var, fc::variant& vo );
   void from_variant( const fc::variant& var, node::protocol::extended_private_key_type& vo );
};

FC_REFLECT_ENUM( node::protocol::community_privacy_type,
         (OPEN_PUBLIC_COMMUNITY)
         (GENERAL_PUBLIC_COMMUNITY)
         (EXCLUSIVE_PUBLIC_COMMUNITY)
         (CLOSED_PUBLIC_COMMUNITY)
         (OPEN_PRIVATE_COMMUNITY)
         (GENERAL_PRIVATE_COMMUNITY)
         (EXCLUSIVE_PRIVATE_COMMUNITY)
         (CLOSED_PRIVATE_COMMUNITY)
         );

FC_REFLECT_ENUM( node::protocol::business_structure_type,
         (OPEN_BUSINESS)
         (PUBLIC_BUSINESS)
         (PRIVATE_BUSINESS)
         );

FC_REFLECT_ENUM( node::protocol::membership_tier_type,
         (NONE)
         (STANDARD_MEMBERSHIP)
         (MID_MEMBERSHIP)
         (TOP_MEMBERSHIP)
         );

FC_REFLECT_ENUM( node::protocol::network_officer_role_type,
         (DEVELOPMENT)
         (MARKETING)
         (ADVOCACY)
         );

FC_REFLECT_ENUM( node::protocol::executive_role_type,
         (CHIEF_EXECUTIVE_OFFICER)
         (CHIEF_OPERATING_OFFICER)
         (CHIEF_FINANCIAL_OFFICER)
         (CHIEF_DEVELOPMENT_OFFICER)
         (CHIEF_TECHNOLOGY_OFFICER)
         (CHIEF_SECURITY_OFFICER)
         (CHIEF_GOVERNANCE_OFFICER)
         (CHIEF_MARKETING_OFFICER)
         (CHIEF_DESIGN_OFFICER)
         (CHIEF_ADVOCACY_OFFICER)
         );

FC_REFLECT_ENUM( node::protocol::product_auction_type,
         (OPEN_AUCTION)
         (REVERSE_AUCTION)
         (CONCEALED_FIRST_PRICE_AUCTION)
         (CONCEALED_SECOND_PRICE_AUCTION)
         );

FC_REFLECT_ENUM( node::protocol::asset_property_type,
         (CURRENCY_ASSET)
         (STANDARD_ASSET)
         (STABLECOIN_ASSET)
         (EQUITY_ASSET)
         (BOND_ASSET)
         (CREDIT_ASSET)
         (STIMULUS_ASSET)
         (LIQUIDITY_POOL_ASSET)
         (CREDIT_POOL_ASSET)
         (OPTION_ASSET)
         (PREDICTION_ASSET)
         (GATEWAY_ASSET)
         (UNIQUE_ASSET)
         );

FC_REFLECT_ENUM( node::protocol::ad_format_type,
         (STANDARD_FORMAT)
         (PREMIUM_FORMAT)
         (PRODUCT_FORMAT)
         (LINK_FORMAT)
         (ACCOUNT_FORMAT)
         (COMMUNITY_FORMAT)
         (ASSET_FORMAT)
         );

FC_REFLECT_ENUM( node::protocol::post_format_type,
         (TEXT_POST)
         (IMAGE_POST)
         (GIF_POST)
         (VIDEO_POST)
         (LINK_POST)
         (ARTICLE_POST)
         (AUDIO_POST)
         (FILE_POST)
         (LIVESTREAM_POST)
         );

FC_REFLECT_ENUM( node::protocol::ad_metric_type,
         (VIEW_METRIC)
         (VOTE_METRIC)
         (SHARE_METRIC)
         (FOLLOW_METRIC)
         (PURCHASE_METRIC)
         (PREMIUM_METRIC)
         );

FC_REFLECT_ENUM( node::protocol::connection_tier_type,
         (PUBLIC)
         (CONNECTION)
         (FRIEND)
         (COMPANION)
         (SECURE)
         );

FC_REFLECT_ENUM( node::protocol::feed_reach_type,
         (NO_FEED)
         (FOLLOW_FEED)
         (MUTUAL_FEED)
         (CONNECTION_FEED)
         (FRIEND_FEED)
         (COMPANION_FEED)
         (COMMUNITY_FEED)
         (TAG_FEED)
         );

FC_REFLECT_ENUM( node::protocol::blog_reach_type,
         (ACCOUNT_BLOG)
         (COMMUNITY_BLOG)
         (TAG_BLOG)
         );

FC_REFLECT_ENUM( node::protocol::sort_time_type,
         (ACTIVE_TIME)
         (RAPID_TIME)
         (STANDARD_TIME)
         (TOP_TIME)
         (ELITE_TIME)
         );

FC_REFLECT_ENUM( node::protocol::sort_option_type,
         (VOTES_SORT)
         (VIEWS_SORT)
         (SHARES_SORT)
         (COMMENTS_SORT)
         (QUALITY_SORT)
         (POPULAR_SORT)
         (VIRAL_SORT)
         (DISCUSSION_SORT)
         (PROMINENT_SORT)
         (CONVERSATION_SORT)
         (DISCOURSE_SORT)
         );

FC_REFLECT_ENUM( node::protocol::post_time_type,
         (ALL_TIME)
         (LAST_HOUR)
         (LAST_DAY)
         (LAST_WEEK)
         (LAST_MONTH)
         (LAST_YEAR)
         );


FC_REFLECT_ENUM( node::protocol::asset_issuer_permission_flags,
         (balance_whitelist)
         (trade_whitelist)
         (maker_restricted)
         (issuer_accept_requests)
         (transfer_restricted)
         (disable_requests)
         (disable_recurring)
         (disable_credit)
         (disable_liquid)
         (disable_options)
         (disable_escrow)
         (disable_force_settle)
         (disable_confidential)
         (disable_auction)
         (producer_fed_asset)
         (global_settle)
         (governance_oversight)
         (immutable_properties)
         );

FC_REFLECT_ENUM( node::protocol::community_permission_flags,
         (member_whitelist)
         (require_profile)
         (require_verified)
         (disable_messages)
         (disable_text_posts)
         (disable_image_posts)
         (disable_gif_posts)
         (disable_video_posts)
         (disable_link_posts)
         (disable_article_posts)
         (disable_audio_posts)
         (disable_file_posts)
         (disable_livestream_posts)
         );

FC_REFLECT( node::protocol::public_key_type, 
         (key_data)
         );

FC_REFLECT( node::protocol::public_key_type::binary_key, 
         (data)
         (check)
         );

FC_REFLECT( node::protocol::extended_public_key_type,
         (key_data)
         );

FC_REFLECT( node::protocol::extended_public_key_type::binary_key,
         (check)
         (data)
         );

FC_REFLECT( node::protocol::extended_private_key_type,
         (key_data)
         );

FC_REFLECT( node::protocol::extended_private_key_type::binary_key,
         (check)
         (data)
         );

FC_REFLECT( node::protocol::encrypted_keypair_type,
         (secure_key)
         (public_key)
         (encrypted_private_key)
         );

FC_REFLECT( node::protocol::date_type,
         (day)
         (month)
         (year)
         );

FC_REFLECT_TYPENAME( node::protocol::share_type );

FC_REFLECT( node::void_t, );