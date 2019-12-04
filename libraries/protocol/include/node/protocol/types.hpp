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
      enum account_types 
      {
         PERSONA    = 'persona',   // Standard account type, pseudonymous, creates posts and transactions
         PROFILE    = 'profile',   // Full identity account type, requires a valid governance address signature and naming convention [firstname.lastname.00000000]
         VERIFIED   = 'verified',  // Profile account that has been upgraded to verifed status 
         BUSINESS   = 'business',  // Corporate account type, issues a cryptoequity asset to owners for approving txns, owned by other accounts
         ANONYMOUS  = 'anonymous'  // No identity account type, creates new stealth accounts by delegation for every txn
      };
s
      // Types of boards, groups and events that can be used for holding posts, topic moderation, and community management.
      enum board_types
      {
         BOARD = 'board',   // General purpose board, all types of content can be posted by all types of accounts. Sorted with low equalization by default, emphasis on content before author.
         GROUP = 'group',   // Curated content board, able to limit account types, content types, post rate. Sorted with high equalization by default, emphasis on author before content. 
         EVENT = 'event',   // Events contain a location and event time. All members can select either attending or not attending.
         STORE = 'store'    // Stores contain product posts, All users can browse and purchase, members can create product posts.
      };

      // Privacy levels of boards, used for determining encryption levels, and access controls for board administration.
      enum board_privacy_types
      {
         OPEN_BOARD       = 'open',      // Default board type, all users can read, interact, and post.
         PUBLIC_BOARD     = 'public',    // All users can read, interact, and request to join. Members can post and invite.
         PRIVATE_BOARD    = 'private',   // Members can read and interact, and create posts. Moderators can invite and accept.
         EXCLUSIVE_BOARD  = 'exclusive'  // All board details are encrypted, cannot request to join. Admins can invite and accept.
      };

      // Business account types, used for determining access controls for signatories of business account transactions and equity assets.
      enum business_types
      {
         OPEN_BUSINESS     = 'open',     // All equity holders can become members and vote for officers and executives, all officers can sign transactions.
         PUBLIC_BUSINESS   = 'public',   // Executives select officers, officers can invite and accept members. All Equity holders can request membership. Members vote for executives.
         PRIVATE_BUSINESS  = 'private'   // CEO selects Executives, Executives can sign transactions, and invite members. Only members may hold equity assets.
      };

      // Types of membership available, each offers additional features, and improves the user experience for more advanced users.
      enum membership_types 
      {
         NONE                 = 'none',      // Account does not have membership, free user
         STANDARD_MEMBERSHIP  = 'standard',  // Regular membership, removes advertising and offers voting benefits.
         MID_MEMBERSHIP       = 'mid',       // Mid level membership, featured page eligibility.
         TOP_MEMBERSHIP       = 'top'        // Enterprise membership, enables governance account and interface account and pro application suite.
      };

      // Types of network officers, each receive reward distributions from the network upon voter approval.
      enum network_officer_types 
      {
         DEVELOPMENT  = 'development',  // Creates and maintains core network software and applications.
         MARKETING    = 'marketing',    // Markets the network to the public, creates awareness and adoption campaigns.
         ADVOCACY     = 'advocacy'      // Advocates the network to business partners, investors, and supports businesses using the protocol.  
      };

      // Types of network officers, each receive reward distributions from the network upon voter approval.
      enum executive_types 
      {
         CHIEF_EXECUTIVE_OFFICER    = 'executive',    // Overall leader of Executive team.
         CHIEF_OPERATING_OFFICER    = 'operating',    // Manages communications and coordination of team.
         CHIEF_FINANCIAL_OFFICER    = 'financial',    // Oversees Credit issuance and salaries.
         CHIEF_DEVELOPMENT_OFFICER  = 'development',  // Oversees protocol development and upgrades.
         CHIEF_TECHNOLOGY_OFFICER   = 'technology',   // Manages front end interface stability and network infrastructure.
         CHIEF_SECURITY_OFFICER     = 'security',     // Manages security of servers, funds, wallets, keys, and cold storage.
         CHIEF_GOVERNANCE_OFFICER   = 'governance',   // Manages Governance account moderation teams.
         CHIEF_MARKETING_OFFICER    = 'marketing',    // Manages public facing communication and campaigns.
         CHIEF_DESIGN_OFFICER       = 'design',       // Oversees graphical design and User interface design.
         CHIEF_ADVOCACY_OFFICER     = 'advocacy'      // Coordinates advocacy efforts and teams.
      };

      // Types of network officers, each receive reward distributions from the network upon voter approval.
      enum proposal_types 
      {
         FUNDING      = 'funding',      // Funds are distributed to beneficiarires based on milestone based projects completed.
         COMPETITION  = 'competition',  // Funds are distributed to the winners of a competitons with predefined terms and objectives. 
         INVESTMENT   = 'investment'    // Funds are used to purchase a business account's equity or credit asset, to be held by the community account.
      };

      // Types of Asset, each is used for specific functions and operation types, and has different properties.
      enum asset_types 
      {
         CURRENCY_ASSET        = 'currency',         // Cryptocurrency that is issued by the network, starts from zero supply, issuing account is the null account, cannot be issued by any accounts. 
         STANDARD_ASSET        = 'standard',         // Regular asset, can be transferred and staked, saved, and delegated.
         EQUITY_ASSET          = 'equity',           // Asset issued by a business account that distributes a dividend from incoming revenue, and has voting power over a business accounts transactions. 
         CREDIT_ASSET          = 'credit',           // Asset issued by a business account that is backed by repayments up to a face value, and interest payments.
         BITASSET_ASSET        = 'bitasset',         // Asset based by collateral and track the value of an external asset.
         LIQUIDITY_POOL_ASSET  = 'liquidity_pool',   // Liquidity pool asset that is backed by the deposits of an asset pair's liquidity pool and earns trading fees. 
         CREDIT_POOL_ASSET     = 'credit_pool',      // Credit pool asset that is backed by deposits of the base asset, used for borrowing funds from the pool, used as collateral to borrow base asset.
         OPTION_ASSET          = 'option',           // Asset that enables the execution of a trade at a specific price until an expiration date. 
         PREDICTION_ASSET      = 'prediction',       // Asset backed by an underlying collateral claim, on the condition that a prediction market resolve in a particular outcome.
         GATEWAY_ASSET         = 'gateway',          // Asset backed by deposits with an exchange counterparty of another asset or currency. 
         UNIQUE_ASSET          = 'unique'            // Asset with a supply of one, contains metadata relating to the ownership of a unique non-fungible asset. 
      };

      // Type of advertising format, determines how creative is formatted in interfaces.
      enum format_types 
      {
         STANDARD_FORMAT  = 'standard',  // A regular post, objective is permlink
         PREMIUM_FORMAT   = 'premium',   // A premium post, objective is permlink
         PRODUCT_FORMAT   = 'product',   // A product post, objective is permlink
         LINK_FORMAT      = 'link',      // A link to an external webpage, objective is URL
         BOARD_FORMAT     = 'board',     // A board, objective is board name
         GROUP_FORMAT     = 'group',     // A group, objective is group board name
         EVENT_FORMAT     = 'event',     // An event, objective is board name
         ACCOUNT_FORMAT   = 'account',   // An account, objective is account name
         STORE_FORMAT     = 'store',     // A store, objective is store name
         ASSET_FORMAT     = 'asset'      // An asset, objective is asset symbol
      };

      // Type of Post, each loaded and displayed accordingly on interfaces.
      enum post_types 
      {
         TEXT_POST        = 'text',        // A post containing a maxmium of 300 characters of text.
         IMAGE_POST       = 'image',       // A post containing an IPFS media file of an image, and up to 1000 characters of description text
         VIDEO_POST       = 'video',       // A post containing a title, an IPFS media file or bittorrent magent link of a video, and up to 1000 characters of description text
         LINK_POST        = 'link',        // A post containing a URL link, link title, and up to 1000 characters of description text
         ARTICLE_POST     = 'article',     // A post containing a title, a header image, and an unlimited amount of body text with embedded images
         AUDIO_POST       = 'audio',       // A post containing a title, an IPFS link to an audio file, and up to 1000 characters of description text
         FILE_POST        = 'file',        // A post containing a title, either an IPFS link to a file, or a magnet link to a bittorrent swarm for a file, and up to 1000 characters of description text
         POLL_POST        = 'poll',        // A post containing a title, at least 2 voting options, and up to 1000 characters of description text
         LIVESTREAM_POST  = 'livestream',  // A post containing a title, a link to a livestreaming video, and up to 1000 characters of description text
         PRODUCT_POST     = 'product',     // A post containing a product title, at least 1 IPFS image of the product, and purchase details to create an escrow order
         LIST_POST        = 'list'         // A post containing a list of at least 2 other posts, a title, and up to 1000 characters of description text

      };

      // Types of expense metrics for advertising transactions.
      enum metric_types 
      {
         VIEW_METRIC      = 'view',      // View transaction ids required for delivery.
         VOTE_METRIC      = 'vote',      // vote transaction ids required for delivery.
         SHARE_METRIC     = 'share',     // share transaction ids required for delivery.
         FOLLOW_METRIC    = 'follow',    // follow or board join transaction ids required for delivery.
         PURCHASE_METRIC  = 'purchase',  // product marketplace purchase orders required for delivery. 
         PREMIUM_METRIC   = 'premium'    // premium content purchases transaction ids required for delivery.
      };

      // Types of connections between accounts, each offers a higher level of security for private information exchanges. 
      enum connection_types 
      {
         PUBLIC      = 'public',       // No privacy setting, post is public.
         CONNECTION  = 'connection',   // Standard connection level, enables private messaging and viewing private posts.
         FRIEND      = 'friend',       // Elevated connection level, activates notifications when posts are made by friends.
         COMPANION   = 'companion'     // Highest connection level, maximum privacy for close friends or partners. 
      };

      // Types of feeds for subscribing to the posts of different sets of users. 
      enum feed_types 
      { 
         NO_FEED          = 'none',         // Posts that should not be distributed to any feeds. 
         FOLLOW_FEED      = 'follow',       // Feed from accounts that are followed.
         MUTUAL_FEED      = 'mutual',       // Feed from accounts that are mutually followed. 
         CONNECTION_FEED  = 'connection',   // Feed from accounts that are connected. 
         FRIEND_FEED      = 'friend',       // Feed from accounts that are friends. 
         COMPANION_FEED   = 'companion',    // Feed from accounts that are companions.
         BOARD_FEED       = 'board',        // Feed from subscribed boards. 
         GROUP_FEED       = 'group',        // Feed from subscribed groups.
         EVENT_FEED       = 'event',        // Feed from subscribed events.
         STORE_FEED       = 'store',        // Feed from subscribed stores.
         TAG_FEED         = 'tag'           // Feed from followed tags. 
      };

      // Types of blogs for tracking what has been created or shared with an account, board, or tag. 
      enum blog_types 
      { 
         ACCOUNT_BLOG  = 'account',  // Blog within an account, includes authored posts and shared posts
         BOARD_BLOG    = 'board',    // Blog within a board, includes all posts within the board, plus shared with the board.
         TAG_BLOG      = 'tag'       // Blog within a tag, includes all posts that use the tag, plus shared with the tag.
      };

      // Types of post ratings, indicating the maturity level of the content. 
      enum rating_types 
      {
         FAMILY    = 'family',    // Posts that are SFW and do not contain images of people. Displays in family mode and higher. [All ages]
         GENERAL   = 'general',   // Posts that are SFW and do contain images of people. Displays in safe mode and higher. [Ages 12+]
         MATURE    = 'mature',    // Posts that are NSFW and but are not explicit. Displays in standard mode and higher. [Ages 16+]
         EXPLICIT  = 'explicit'   // Posts that are NSFW and are explicit. Displays in autonomous mode only. [Ages 18+]
      };

      enum asset_issuer_permission_flags 
      {
         charge_market_fee    = 0x01,    // an issuer-specified percentage of all market trades in this asset is paid to the issuer
         white_list           = 0x02,    // accounts must be whitelisted in order to hold this asset
         override_authority   = 0x04,    // issuer may transfer asset back to himself
         transfer_restricted  = 0x08,    // require the issuer to be one party to every transfer
         disable_force_settle = 0x10,    // disable force settling
         global_settle        = 0x20,    // allow the bitasset issuer to force a global settlement -- this may be set in permissions, but not flags
         disable_confidential = 0x40,    // allow the asset to be used with confidential transactions
         witness_fed_asset    = 0x80,    // allow the asset to be fed by witnesses
         committee_fed_asset  = 0x100    // allow the asset to be fed by the committee
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
         encrypted_keypair_type( const public_key_type& secure_key, const public_key_type& public_key, const std::string& encrypted_private_key );

         public_key_type   secure_key;                 // The public key used to encrypt the encrypted key.
         public_key_type   public_key;                 // The public key of the private encrypted key.
         string            encrypted_private_key;      // The encrypted private key of the public key.
         
         friend bool operator == ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 )
         {
            return std::tie( p1.secure_key, p1.public_key, p1.encrypted_private_key ) == std::tie( p2.secure_key, p2.public_key, p2.encrypted_private_key );
         }
         friend bool operator < ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 )
         {
            return std::tie( p1.secure_key, p1.public_key, p1.encrypted_private_key ) < std::tie( p2.secure_key, p2.public_key, p2.encrypted_private_key );
         }
         friend bool operator > ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 )
         {
            return std::tie( p1.secure_key, p1.public_key, p1.encrypted_private_key ) > std::tie( p2.secure_key, p2.public_key, p2.encrypted_private_key );
         }
         friend bool operator != ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 )
         {
            return std::tie( p1.secure_key, p1.public_key, p1.encrypted_private_key ) != std::tie( p2.secure_key, p2.public_key, p2.encrypted_private_key );
         }
         friend bool operator <= ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 )
         {
            return std::tie( p1.secure_key, p1.public_key, p1.encrypted_private_key ) <= std::tie( p2.secure_key, p2.public_key, p2.encrypted_private_key );
         }
         friend bool operator >= ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 )
         {
            return std::tie( p1.secure_key, p1.public_key, p1.encrypted_private_key ) >= std::tie( p2.secure_key, p2.public_key, p2.encrypted_private_key );
         }
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

FC_REFLECT_ENUM( node::chain::account_types,
         (PERSONA)
         (PROFILE)
         (VERIFIED)
         (BUSINESS)
         (ANONYMOUS)
         );

FC_REFLECT_ENUM( node::chain::board_types,
         (BOARD)
         (GROUP)
         (EVENT)
         (STORE)
         );

FC_REFLECT_ENUM( node::chain::board_privacy_types,
         (OPEN_BOARD)
         (PUBLIC_BOARD)
         (PRIVATE_BOARD)
         (EXCLUSIVE_BOARD)
         );

FC_REFLECT_ENUM( node::chain::business_types,
         (OPEN_BUSINESS)
         (PUBLIC_BUSINESS)
         (PRIVATE_BUSINESS)
         );

FC_REFLECT_ENUM( node::chain::network_officer_types,
         (DEVELOPMENT)
         (MARKETING)
         (ADVOCACY)
         );

FC_REFLECT_ENUM( node::chain::executive_types,
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

FC_REFLECT_ENUM( node::chain::proposal_types,
         (FUNDING)
         (COMPETITION)
         (INVESTMENT)
         );

FC_REFLECT_ENUM( node::chain::asset_types,
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

FC_REFLECT_ENUM( node::chain::post_types,
         (TEXT_POST)
         (IMAGE_POST)
         (VIDEO_POST)
         (PRODUCT_POST)
         (LINK_POST)
         (ARTICLE_POST)
         (AUDIO_POST)
         (FILE_POST)
         (POLL_POST)
         (LIVESTREAM_POST)
         (PRODUCT_POST)
         (LIST_POST)
         );

FC_REFLECT_ENUM( node::chain::metric_types,
         (VIEW_METRIC)
         (VOTE_METRIC)
         (SHARE_METRIC)
         (FOLLOW_METRIC)
         (PURCHASE_METRIC)
         (PREMIUM_METRIC)
         );

FC_REFLECT_ENUM( node::chain::connection_types,
         (PUBLIC)
         (CONNECTION)
         (FRIEND)
         (COMPANION)
         );

FC_REFLECT_ENUM( node::chain::feed_types,
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

FC_REFLECT_ENUM( node::chain::blog_types,
         (ACCOUNT_BLOG)
         (BOARD_BLOG)
         (TAG_BLOG)
         );

FC_REFLECT_ENUM( node::chain::rating_types,
         (FAMILY)
         (GENERAL)
         (MATURE)
         (EXPLICIT)
         );

FC_REFLECT_ENUM( node::chain::feed_types,
         (charge_market_fee)
         (white_list)
         (override_authority)
         (transfer_restricted)
         (disable_force_settle)
         (global_settle)
         (disable_confidential)
         (witness_fed_asset)
         (committee_fed_asset)
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