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
      typedef fixed_string_16             asset_symbol_type;
      typedef fc::ripemd160               block_id_type;
      typedef fc::ripemd160               checksum_type;
      typedef fc::ripemd160               transaction_id_type;
      typedef fc::sha256                  digest_type;
      typedef fc::ecc::compact_signature  signature_type;
      typedef safe<int64_t>               share_type;
      typedef uint16_t                    weight_type;
      typedef boost::rational<int32_t>    ratio_type;
      

      enum asset_issuer_permission_flags 
      {
         charge_market_fee    = 0x01, /**< an issuer-specified percentage of all market trades in this asset is paid to the issuer */
         white_list           = 0x02, /**< accounts must be whitelisted in order to hold this asset */
         override_authority   = 0x04, /**< issuer may transfer asset back to himself */
         transfer_restricted  = 0x08, /**< require the issuer to be one party to every transfer */
         disable_force_settle = 0x10, /**< disable force settling */
         global_settle        = 0x20, /**< allow the bitasset issuer to force a global settling -- this may be set in permissions, but not flags */
         disable_confidential = 0x40, /**< allow the asset to be used with confidential transactions */
         witness_fed_asset    = 0x80, /**< allow the asset to be fed by witnesses */
         committee_fed_asset  = 0x100 /**< allow the asset to be fed by the committee */
      };

      // Types of accounts available, each offering different functionality.


      enum account_types 
      {
         PERSONA,   // Standard account type, pseudonymous, creates posts and transactions
         PROFILE,   // Full identity account type, requires a valid governance address signature and naming convention [firstname.lastname.00000000]
         VERIFIED,  // Profile account that has been upgraded to verifed status 
         BUSINESS,  // Corporate account type, issues a cryptoequity asset to owners for approving txns, owned by other accounts
         ANONYMOUS  // No identity account type, creates new stealth accounts by delegation for every txn
      };

      // Types of boards, groups and events that can be used for holding posts, topic moderation, and community management.
      enum board_types
      {
         BOARD,     // General purpose board, all types of content can be posted by all types of accounts. Sorted with low equalization by default, emphasis on content before author.
         GROUP,     // Curated content board, able to limit account types, content types, post rate. Sorted with high equalization by default, emphasis on author before content. 
         EVENT,     // Events contain a location and event time. All members can select either attending or not attending.
         STORE,     // Stores contain product posts, All users can browse and purchase, members can create product posts.
      };

      // Privacy levels of boards, used for determining encryption levels, and access controls for board administration.
      enum board_privacy_types
      {
         OPEN_BOARD,         // Default board type, all users can read, interact, and post.
         PUBLIC_BOARD,       // All users can read, interact, and request to join. Members can post and invite.
         PRIVATE_BOARD,      // Members can read and interact, and create posts. Moderators can invite and accept.
         EXCLUSIVE_BOARD     // All board details are encrypted, cannot request to join. Admins can invite and accept.
      };

      // Business account types, used for determining access controls for signatories of business account transactions and equity assets.
      enum business_types
      {
         OPEN_BUSINESS,         // All equity holders can become members and vote for officers and executives, all officers can sign transactions.
         PUBLIC_BUSINESS,       // Executives select officers, officers can invite and accept members. All Equity holders can request membership. Members vote for executives.
         PRIVATE_BUSINESS,      // CEO selects Executives, Executives can sign transactions, and invite members. Only members may hold equity assets.
      };

      // Types of membership available, each offers additional features, and improves the user experience for more advanced users.
      enum membership_types 
      {
         NONE,                 // Account does not have membership, free user
         STANDARD_MEMBERSHIP,  // Regular membership, removes advertising and offers voting benefits.
         MID_MEMBERSHIP,       // Mid level membership, featured page eligibility.
         TOP_MEMBERSHIP        // Enterprise membership, enables governance account and interface account and pro application suite.
      };

      // Types of network officers, each receive reward distributions from the network upon voter approval.
      enum network_officer_types 
      {
         DEVELOPMENT,      // Creates and maintains core network software and applications.
         MARKETING,        // Markets the network to the public, creates awareness and adoption campaigns.
         ADVOCACY          // Advocates the network to business partners, investors, and supports businesses using the protocol.  
      };

      // Types of network officers, each receive reward distributions from the network upon voter approval.
      enum executive_types 
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

      // Types of network officers, each receive reward distributions from the network upon voter approval.
      enum proposal_types 
      {
         FUNDING,         // Funds are distributed to beneficiarires based on milestone based projects completed.
         COMPETITION,     // Funds are distributed to the winners of a competitons with predefined terms and objectives. 
         INVESTMENT       // Funds are used to purchase a business account's equity or credit asset, to be held by the community account.
      };

      // Types of Asset, each is used for specific functions and operation types, and has different properties.
      enum asset_types 
      {
         CURRENCY_ASSET,         // Cryptocurrency that is issued by the network, starts from zero supply, issuing account is the null account, cannot be issued by any accounts. 
         STANDARD_ASSET,         // Regular asset, can be transferred and staked, saved, and delegated.
         EQUITY_ASSET,           // Asset issued by a business account that distributes a dividend from incoming revenue, and has voting power over a business accounts transactions. 
         CREDIT_ASSET,           // Asset issued by a business account that is backed by repayments up to a face value, and interest payments.
         BITASSET_ASSET,         // Asset based by collateral and track the value of an external asset.
         LIQUIDITY_POOL_ASSET,   // Liquidity pool asset that is backed by the deposits of an asset pair's liquidity pool and earns trading fees. 
         CREDIT_POOL_ASSET,      // Credit pool asset that is backed by deposits of the base asset, used for borrowing funds from the pool, used as collateral to borrow base asset.
         OPTION_ASSET,           // Asset that enables the execution of a trade at a specific price until an expiration date. 
         PREDICTION_ASSET,       // Asset backed by an underlying collateral claim, on the condition that a prediction market resolve in a particular outcome.
         GATEWAY_ASSET,          // Asset backed by deposits with an exchange counterparty of another asset or currency. 
         UNIQUE_ASSET,           // Asset with a supply of one, contains metadata relating to the ownership of a unique non-fungible asset. 
      };

      // Type of advertising format, determines how creative is formatted in interfaces.
      enum format_types 
      {
         STANDARD_FORMAT,  // A regular post, objective is permlink
         PREMIUM_FORMAT,   // A premium post, objective is permlink
         PRODUCT_FORMAT,   // A product post, objective is permlink
         LINK_FORMAT,      // A link to an external webpage
         BOARD_FORMAT,     // A board, objective is board name
         GROUP_FORMAT,     // A group, objective is group board name
         EVENT_FORMAT,     // An event, objective is board name
         ACCOUNT_FORMAT,   // An account, objective is account name
         STORE_FORMAT,     // A store
         ASSET_FORMAT      // An asset, objective is asset symbol
      };

      // Type of Post, each loaded and displayed accordingly on interfaces.
      enum post_types 
      {
         TEXT_POST,        // A post containing a maxmium of 300 characters of text.
         IMAGE_POST,       // A post containing an IPFS media file of an image, and up to 1000 characters of description text
         VIDEO_POST,       // A post containing a title, an IPFS media file or bittorrent magent link of a video, and up to 1000 characters of description text
         PRODUCT_POST,     // A post containing a product title, at least 1 IPFS image of the product, and purchase details to create an escrow order
         LINK_POST,        // A post containing a URL link, link title, and up to 1000 characters of description text
         ARTICLE_POST,     // A post containing a title, a header image, and an unlimited amount of body text with embedded images
         AUDIO_POST,       // A post containing a title, an IPFS link to an audio file, and up to 1000 characters of description text
         FILE_POST,        // A post containing a title, either an IPFS link to a file, or a magnet link to a bittorrent swarm for a file, and up to 1000 characters of description text
         POLL_POST,        // A post containing a title, at least 2 voting options, and up to 1000 characters of description text
         LIVESTREAM_POST,  // A post containing a title, a link to a livestreaming video, and up to 1000 characters of description text
         LIST_POST,        // A post containing a list of at least 2 other posts, a title, and up to 1000 characters of description text
         
      };

      // Types of expense metrics for advertising transactions.
      enum metric_types 
      {
         VIEW_METRIC,      // View transaction ids required for delivery.
         VOTE_METRIC,      // vote transaction ids required for delivery.
         SHARE_METRIC,     // share transaction ids required for delivery.
         FOLLOW_METRIC,    // follow or board join transaction ids required for delivery.
         PURCHASE_METRIC,  // product marketplace purchase orders required for delivery. 
         PREMIUM_METRIC    // premium content purchases transaction ids required for delivery.
      };

      // Types of connections between accounts, each offers a higher level of security for private information exchanges. 
      enum connection_types 
      {
         PUBLIC,     // No privacy setting, post is public.
         CONNECTION, // Standard connection level, enables private messaging and viewing private posts.
         FRIEND,     // Elevated connection level, activates notifications when posts are made by friends.
         COMPANION   // Highest connection level, maximum privacy for close friends or partners. 
      };

      // Types of feeds for subscribing to the posts of different sets of users. 
      enum feed_types 
      { 
         FOLLOW_FEED,            // Feed from accounts that are followed.
         MUTUAL_FEED,            // Feed from accounts that are mutually followed. 
         CONNECTION_FEED,        // Feed from accounts that are connected. 
         FRIEND_FEED,            // Feed from accounts that are friends. 
         COMPANION_FEED,         // Feed from accounts that are companions.
         BOARD_FEED,             // Feed from subscribed boards. 
         GROUP_FEED,             // Feed from subscribed groups.
         EVENT_FEED,             // Feed from subscribed events.
         STORE_FEED,             // Feed from subscribed stores.
      };

      //Types of post ratings, indicating the maturity level of the content. 
      enum rating_types 
      {
         FAMILY,       // Posts that are SFW and do not contain images of people. Displays in family mode and higher. [All ages]
         GENERAL,      // Posts that are SFW and do contain images of people. Displays in safe mode and higher. [Ages 12+]
         MATURE,       // Posts that are NSFW and but are not explicit. Displays in standard mode and higher. [Ages 16+]
         EXPLICIT      // Posts that are NSFW and are explicit. Displays in autonomous mode only. [Ages 18+]
      };

      const static uint32_t ASSET_ISSUER_PERMISSION_MASK =
            charge_market_fee
            | white_list
            | override_authority
            | transfer_restricted
            | disable_force_settle
            | global_settle
            | disable_confidential
            | witness_fed_asset
            | committee_fed_asset;
            
      const static uint32_t UIA_ASSET_ISSUER_PERMISSION_MASK =
            charge_market_fee
            | white_list
            | override_authority
            | transfer_restricted
            | disable_confidential;


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
            friend bool operator == ( const public_key_type& p1, const fc::ecc::public_key& p2);
            friend bool operator == ( const public_key_type& p1, const public_key_type& p2);
            friend bool operator < ( const public_key_type& p1, const public_key_type& p2) { return p1.key_data < p2.key_data; }
            friend bool operator != ( const public_key_type& p1, const public_key_type& p2);
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

FC_REFLECT( node::protocol::public_key_type, (key_data) );
FC_REFLECT( node::protocol::public_key_type::binary_key, (data)(check) );
FC_REFLECT( node::protocol::extended_public_key_type, (key_data) );
FC_REFLECT( node::protocol::extended_public_key_type::binary_key, (check)(data) );
FC_REFLECT( node::protocol::extended_private_key_type, (key_data) );
FC_REFLECT( node::protocol::extended_private_key_type::binary_key, (check)(data) );

FC_REFLECT_TYPENAME( node::protocol::share_type );

FC_REFLECT( node::void_t, );
