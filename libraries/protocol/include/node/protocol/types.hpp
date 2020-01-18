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

namespace node {

   using                                    fc::uint128_t;
   typedef boost::multiprecision::int128_t  int128_t;
   typedef boost::multiprecision::uint256_t u256;
   typedef boost::multiprecision::uint512_t u512;

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
   struct void_t{};

   namespace protocol {

      typedef fc::ecc::private_key        private_key_type;
      typedef fc::sha256                  chain_id_type;
      typedef fixed_string_16             account_name_type;
      typedef fixed_string_16             board_name_type;
      typedef fixed_string_32             tag_name_type;
      typedef fixed_string_16             asset_symbol_type;
      typedef fc::ripemd160               block_id_type;
      typedef fc::ripemd160               checksum_type;
      typedef fc::ripemd160               transaction_id_type;
      typedef fc::sha256                  digest_type;
      typedef fc::ecc::compact_signature  signature_type;
      typedef safe<int64_t>               share_type;
      typedef uint16_t                    weight_type;
      typedef boost::rational<int32_t>    ratio_type;


      // Types of accounts available, each offering different functionality.
      enum account_identity_type
      {
         PERSONA,   // Standard account type, pseudonymous, creates posts and transactions
         PROFILE,   // Full identity account type, requires a valid governance address signature and naming convention [firstname.lastname.00000000]
         VERIFIED,  // Profile account that has been upgraded to verifed status 
         BUSINESS,  // Corporate account type, issues a cryptoequity asset to owners for approving txns, owned by other accounts
         ANONYMOUS  // No identity account type, creates new stealth accounts by delegation for every txn
      };

      vector< string > account_identity_values =
      {
         "persona",
         "profile",
         "verified",
         "business",
         "anonymous"
      };

      // Types of boards, groups and events that can be used for holding posts, topic moderation, and community management.
      enum board_structure_type
      {
         BOARD,   // General purpose board, all types of content can be posted by all types of accounts. Sorted with low equalization by default, emphasis on content before author.
         GROUP,   // Curated content board, able to limit account types, content types, post rate. Sorted with high equalization by default, emphasis on author before content. 
         EVENT,   // Events contain a location and event time. All members can select either attending or not attending.
         STORE    // Stores contain product posts, All users can browse and purchase, members can create product posts.
      };

      vector< string > board_structure_values =
      {
         "board",
         "group",
         "event",
         "store"
      };

      // Privacy levels of boards, used for determining encryption levels, and access controls for board administration.
      enum board_privacy_type
      {
         OPEN_BOARD,      // Default board type, all users can read, interact, and post.
         PUBLIC_BOARD,    // All users can read, interact, and request to join. Members can post and invite.
         PRIVATE_BOARD,   // Members can read and interact, and create posts. Moderators can invite and accept.
         EXCLUSIVE_BOARD  // All board details are encrypted, cannot request to join. Admins can invite and accept.
      };

      vector< string > board_privacy_values = 
      {
         "open",
         "public",
         "private",
         "exclusive"
      };

      // Business account types, used for determining access controls for signatories of business account transactions and equity assets.
      enum business_structure_type
      {
         OPEN_BUSINESS,     // All equity holders can become members and vote for officers and executives, all officers can sign transactions.
         PUBLIC_BUSINESS,   // Executives select officers, officers can invite and accept members. All Equity holders can request membership. Members vote for executives.
         PRIVATE_BUSINESS   // CEO selects Executives, Executives can sign transactions, and invite members. Only members may hold equity assets.
      };

      vector< string > business_structure_values = 
      {
         "open",
         "public",
         "private"
      };

      // Types of membership available, each offers additional features, and improves the user experience for more advanced users.
      enum membership_tier_type 
      {
         NONE,                 // Account does not have membership, free user
         STANDARD_MEMBERSHIP,  // Regular membership, removes advertising and offers voting benefits.
         MID_MEMBERSHIP,       // Mid level membership, featured page eligibility.
         TOP_MEMBERSHIP        // Enterprise membership, enables governance account and interface account and pro application suite.
      };

      vector< string > membership_tier_values =
      {
         "none",
         "standard",
         "mid",
         "top"
      };

      // Types of network officers, each receive reward distributions from the network upon voter approval.
      enum network_officer_role_type 
      {
         DEVELOPMENT,  // Creates and maintains core network software and applications.
         MARKETING,    // Markets the network to the public, creates awareness and adoption campaigns.
         ADVOCACY      // Advocates the network to business partners, investors, and supports businesses using the protocol.  
      };

      vector< string > network_officer_role_values =
      {
         "development",
         "marketing",
         "advocacy"
      };

      // Types of network officers, each receive reward distributions from the network upon voter approval.
      enum executive_role_type
      {
         CHIEF_EXECUTIVE_OFFICER,    // Overall leader of Executive team.
         CHIEF_OPERATING_OFFICER,    // Manages communications and coordination of team.
         CHIEF_FINANCIAL_OFFICER,    // Oversees Credit issuance and salaries.
         CHIEF_DEVELOPMENT_OFFICER,  // Oversees protocol development and upgrades.
         CHIEF_TECHNOLOGY_OFFICER,   // Manages front end interface stability and network infrastructure.
         CHIEF_SECURITY_OFFICER,     // Manages security of servers, funds, wallets, keys, and cold storage.
         CHIEF_GOVERNANCE_OFFICER,   // Manages Governance account moderation teams.
         CHIEF_MARKETING_OFFICER,    // Manages public facing communication and campaigns.
         CHIEF_DESIGN_OFFICER,       // Oversees graphical design and User interface design.
         CHIEF_ADVOCACY_OFFICER      // Coordinates advocacy efforts and teams.
      };

      vector< string > executive_role_values =
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

      // Types of network officers, each receive reward distributions from the network upon voter approval.
      enum proposal_distribution_type 
      {
         FUNDING,      // Funds are distributed to beneficiarires based on milestone based projects completed.
         COMPETITION,  // Funds are distributed to the winners of a competitons with predefined terms and objectives. 
         INVESTMENT    // Funds are used to purchase a business account"s equity or credit asset, to be held by the community account.
      };

      vector< string > proposal_distribution_values =
      {
         "funding",
         "competition",
         "investment"
      };

      // Types of Asset, each is used for specific functions and operation types, and has different properties.
      enum asset_property_type 
      {
         CURRENCY_ASSET,         // Cryptocurrency that is issued by the network, starts from zero supply, issuing account is the null account, cannot be issued by any accounts. 
         STANDARD_ASSET,         // Regular asset, can be transferred and staked, saved, and delegated.
         EQUITY_ASSET,           // Asset issued by a business account that distributes a dividend from incoming revenue, and has voting power over a business accounts transactions. 
         CREDIT_ASSET,           // Asset issued by a business account that is backed by repayments up to a face value, and interest payments.
         BITASSET_ASSET,         // Asset based by collateral and track the value of an external asset.
         LIQUIDITY_POOL_ASSET,   // Liquidity pool asset that is backed by the deposits of an asset pair"s liquidity pool and earns trading fees. 
         CREDIT_POOL_ASSET,      // Credit pool asset that is backed by deposits of the base asset, used for borrowing funds from the pool, used as collateral to borrow base asset.
         OPTION_ASSET,           // Asset that enables the execution of a trade at a specific price until an expiration date. 
         PREDICTION_ASSET,       // Asset backed by an underlying collateral claim, on the condition that a prediction market resolve in a particular outcome.
         GATEWAY_ASSET,          // Asset backed by deposits with an exchange counterparty of another asset or currency. 
         UNIQUE_ASSET            // Asset with a supply of one, contains metadata relating to the ownership of a unique non-fungible asset. 
      };

      vector< string > asset_property_values =
      {
         "currency",
         "standard",
         "equity",
         "credit",
         "bitasset",
         "liquidity_pool",
         "credit_pool",
         "option",
         "prediction",
         "gateway",
         "unique"
      };

      // Type of advertising format, determines how creative is formatted in interfaces.
      enum ad_format_type
      {
         STANDARD_FORMAT,  // A regular post, objective is permlink
         PREMIUM_FORMAT,   // A premium post, objective is permlink
         PRODUCT_FORMAT,   // A product post, objective is permlink
         LINK_FORMAT,      // A link to an external webpage, objective is URL
         ACCOUNT_FORMAT,   // An account, objective is account name
         BOARD_FORMAT,     // A board, objective is board name
         GROUP_FORMAT,     // A group, objective is group board name
         EVENT_FORMAT,     // An event, objective is board name
         STORE_FORMAT,     // A store, objective is store name
         ASSET_FORMAT      // An asset, objective is asset symbol
      };

      vector< string > ad_format_values =
      {
         "standard",
         "premium",
         "product",
         "link",
         "account",
         "board",
         "group",
         "event",
         "store",
         "asset"
      };

      // Type of Post, each loaded and displayed accordingly on interfaces.
      enum post_format_type 
      {
         TEXT_POST,        // A post containing a maxmium of 300 characters of text.
         IMAGE_POST,       // A post containing an IPFS media file of an image, and up to 1000 characters of description text
         VIDEO_POST,       // A post containing a title, an IPFS media file or bittorrent magent link of a video, and up to 1000 characters of description text
         LINK_POST,        // A post containing a URL link, link title, and up to 1000 characters of description text
         ARTICLE_POST,     // A post containing a title, a header image, and an unlimited amount of body text with embedded images
         AUDIO_POST,       // A post containing a title, an IPFS link to an audio file, and up to 1000 characters of description text
         FILE_POST,        // A post containing a title, either an IPFS link to a file, or a magnet link to a bittorrent swarm for a file, and up to 1000 characters of description text
         POLL_POST,        // A post containing a title, at least 2 voting options, and up to 1000 characters of description text
         LIVESTREAM_POST,  // A post containing a title, a link to a livestreaming video, and up to 1000 characters of description text
         PRODUCT_POST,     // A post containing a product title, at least 1 IPFS image of the product, and purchase details to create an escrow order
         LIST_POST         // A post containing a list of at least 2 other posts, a title, and up to 1000 characters of description text
      };

      vector< string > post_format_values =
      {
         "text",
         "image",
         "video",
         "link",
         "article",
         "audio",
         "file",
         "poll",
         "livestream",
         "product",
         "list"
      };

      // Types of expense metrics for advertising transactions.
      enum ad_metric_type 
      {
         VIEW_METRIC,      // View transaction ids required for delivery.
         VOTE_METRIC,      // vote transaction ids required for delivery.
         SHARE_METRIC,     // share transaction ids required for delivery.
         FOLLOW_METRIC,    // follow or board join transaction ids required for delivery.
         PURCHASE_METRIC,  // product marketplace purchase orders required for delivery. 
         PREMIUM_METRIC    // premium content purchases transaction ids required for delivery.
      };

      vector< string > ad_metric_values =
      {
         "view",
         "vote",
         "share",
         "follow",
         "purchase",
         "premium"
      };

      // Types of connections between accounts, each offers a higher level of security for private information exchanges. 
      enum connection_tier_type
      {
         PUBLIC,       // No privacy setting, post is public.
         CONNECTION,   // Standard connection level, enables private messaging and viewing private posts.
         FRIEND,       // Elevated connection level, activates notifications when posts are made by friends.
         COMPANION     // Highest connection level, maximum privacy for close friends or partners. 
      };

      vector< string > connection_tier_values =
      {
         "public",
         "connection",
         "friend",
         "companion"
      };

      // Types of feeds for subscribing to the posts of different sets of users. 
      enum feed_reach_type 
      { 
         NO_FEED,           // Posts that should not be distributed to any feeds. 
         FOLLOW_FEED,       // Feed from accounts that are followed.
         MUTUAL_FEED,       // Feed from accounts that are mutually followed. 
         CONNECTION_FEED,   // Feed from accounts that are connected. 
         FRIEND_FEED,       // Feed from accounts that are friends. 
         COMPANION_FEED,    // Feed from accounts that are companions.
         BOARD_FEED,        // Feed from subscribed boards. 
         GROUP_FEED,        // Feed from subscribed groups.
         EVENT_FEED,        // Feed from subscribed events.
         STORE_FEED,        // Feed from subscribed stores.
         TAG_FEED           // Feed from followed tags. 
      };

      vector< string > feed_reach_values =
      { 
         "none",
         "follow",
         "mutual",
         "connection",
         "friend",
         "companion",
         "board",
         "group",
         "event",
         "store",
         "tag"
      };

      // Types of blogs for tracking what has been created or shared with an account, board, or tag. 
      enum blog_reach_type 
      { 
         ACCOUNT_BLOG,  // Blog within an account, includes authored posts and shared posts
         BOARD_BLOG,    // Blog within a board, includes all posts within the board, plus shared with the board.
         TAG_BLOG       // Blog within a tag, includes all posts that use the tag, plus shared with the tag.
      };

      vector< string > blog_reach_values =
      { 
         "account",
         "board",
         "tag"
      };

      // Types of post ratings, indicating the maturity level of the content. 
      enum post_rating_type
      {
         FAMILY,    // Posts that are SFW and do not contain images of people. Displays in family mode and higher. [All ages]
         GENERAL,   // Posts that are SFW and do contain images of people. Displays in safe mode and higher. [Ages 12+]
         MATURE,    // Posts that are NSFW and but are not explicit. Displays in standard mode and higher. [Ages 16+]
         EXPLICIT   // Posts that are NSFW and are explicit. Displays in autonomous mode only. [Ages 18+]
      };

      vector< string > post_rating_values =
      {
         "family",
         "general",
         "mature",
         "explicit"
      };

      enum sort_time_type
      {
         ACTIVE_TIME,
         RAPID_TIME,
         STANDARD_TIME,
         TOP_TIME,
         ELITE_TIME
      };

      vector< string > sort_time_values = 
      {
         "active",
         "rapid",
         "standard",
         "top",
         "elite"
      };

      enum sort_option_type
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

      vector< string > sort_option_values =
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

      enum post_time_type
      {
         ALL_TIME,
         LAST_HOUR,
         LAST_DAY,
         LAST_WEEK,
         LAST_MONTH,
         LAST_YEAR
      };

      vector< string > post_time_values =
      {
         "all",
         "hour",
         "day",
         "week",
         "month",
         "year"
      };

      enum asset_issuer_permission_flags 
      {
         balance_whitelist           = 1,       // Accounts must be whitelisted in order to send, receive or hold the asset.
         trade_whitelist             = 2,       // Accounts must be whitelisted to trade the asset.
         maker_restricted            = 4,       // Only issuer may create new trading orders onto the orderbook, others must fill them.
         issuer_accept_requests      = 8,       // Issuer may approve transfer requests, enabling them to retrieve funds from any account.
         transfer_restricted         = 16,      // Transfers may only be to or from the issuer.
         disable_requests            = 32,      // Payment requests are disabled.
         disable_recurring           = 64,      // Recurring payments are disabled.
         disable_credit              = 128,     // The asset cannot be lent into a credit pool, Disabling margin and credit loan orders.
         disable_liquid              = 256,     // The asset cannot be used to create a liquidity pool.
         disable_options             = 512,     // The asset cannot be used to issue options assets.
         disable_escrow              = 1024,    // Disable escrow transfers and marketplace trades using the asset.
         disable_force_settle        = 2048,    // Disable force settling of bitassets, only global settle may return collateral.
         disable_confidential        = 4096,    // Asset cannot be used with confidential transactions.
         disable_auction             = 8192,    // Disable creation of auction orders for the asset.
         witness_fed_asset           = 16384,   // Allow the asset to be price food to be published by top elected witnesses.
         global_settle               = 32768,   // Allow the bitasset issuer to force a global settlement: Set in permissions, but not flags.
         governance_oversight        = 65536,   // Asset update, issuer transfer and issuance require the account's governance address to approve
         immutable_properties        = 131072,  // Disable any future asset options updates or changes to flags
      };

      const static uint32_t ASSET_ISSUER_PERMISSION_MASK =
            balance_whitelist
            | trade_whitelist
            | maker_restricted
            | issuer_accept_requests
            | transfer_restricted
            | disable_requests
            | disable_recurring
            | disable_credit
            | disable_liquid
            | disable_options
            | disable_escrow
            | disable_force_settle
            | disable_confidential
            | disable_auction
            | witness_fed_asset
            | global_settle
            | governance_oversight
            | immutable_properties;

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

         public_key_type   secure_key;                 // The public key used to encrypt the encrypted key.
         public_key_type   public_key;                 // The public key of the private encrypted key.
         string            encrypted_private_key;      // The encrypted private key of the public key.
         
         friend bool operator == ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 );
         friend bool operator < ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 );
         friend bool operator > ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 );
         friend bool operator != ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 );
         friend bool operator <= ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 );
         friend bool operator >= ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 );
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

FC_REFLECT_ENUM( node::protocol::account_identity_type,
         (PERSONA)
         (PROFILE)
         (VERIFIED)
         (BUSINESS)
         (ANONYMOUS)
         );

FC_REFLECT_ENUM( node::protocol::board_structure_type,
         (BOARD)
         (GROUP)
         (EVENT)
         (STORE)
         );

FC_REFLECT_ENUM( node::protocol::board_privacy_type,
         (OPEN_BOARD)
         (PUBLIC_BOARD)
         (PRIVATE_BOARD)
         (EXCLUSIVE_BOARD)
         );

FC_REFLECT_ENUM( node::protocol::business_structure_type,
         (OPEN_BUSINESS)
         (PUBLIC_BUSINESS)
         (PRIVATE_BUSINESS)
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

FC_REFLECT_ENUM( node::protocol::proposal_distribution_type,
         (FUNDING)
         (COMPETITION)
         (INVESTMENT)
         );

FC_REFLECT_ENUM( node::protocol::asset_property_type,
         (CURRENCY_ASSET)
         (STANDARD_ASSET)
         (EQUITY_ASSET)
         (CREDIT_ASSET)
         (BITASSET_ASSET)
         (LIQUIDITY_POOL_ASSET)
         (CREDIT_POOL_ASSET)
         (OPTION_ASSET)
         (PREDICTION_ASSET)
         (GATEWAY_ASSET)
         (UNIQUE_ASSET)
         );

FC_REFLECT_ENUM( node::protocol::post_format_type,
         (TEXT_POST)
         (IMAGE_POST)
         (VIDEO_POST)
         (LINK_POST)
         (ARTICLE_POST)
         (AUDIO_POST)
         (FILE_POST)
         (POLL_POST)
         (LIVESTREAM_POST)
         (PRODUCT_POST)
         (LIST_POST)
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
         );

FC_REFLECT_ENUM( node::protocol::feed_reach_type,
         (NO_FEED)
         (FOLLOW_FEED)
         (MUTUAL_FEED)
         (CONNECTION_FEED)
         (FRIEND_FEED)
         (COMPANION_FEED)
         (BOARD_FEED)
         (GROUP_FEED)
         (EVENT_FEED)
         (STORE_FEED)
         (TAG_FEED)
         );

FC_REFLECT_ENUM( node::protocol::blog_reach_type,
         (ACCOUNT_BLOG)
         (BOARD_BLOG)
         (TAG_BLOG)
         );

FC_REFLECT_ENUM( node::protocol::post_rating_type,
         (FAMILY)
         (GENERAL)
         (MATURE)
         (EXPLICIT)
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

FC_REFLECT_TYPENAME( node::protocol::share_type );

FC_REFLECT( node::void_t, );