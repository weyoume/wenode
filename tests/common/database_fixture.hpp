#pragma once

#include <node/app/application.hpp>
#include <node/chain/database.hpp>
#include <node/protocol/node_operations.hpp>
#include <graphene/utilities/key_conversion.hpp>
#include <fc/io/json.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/aes.hpp>
#include <node/plugins/debug_node/debug_node_plugin.hpp>

#include <iostream>

#define INITIAL_TEST_SUPPLY (10000000000ll)
using namespace graphene::db;

extern fc::time_point TESTING_GENESIS_TIMESTAMP;

#define PUSH_TX \
   node::chain::test::_push_transaction

#define PUSH_BLOCK \
   node::chain::test::_push_block

// See below
#define REQUIRE_OP_VALIDATION_SUCCESS( op, field, value ) \
{ \
   const auto temp = op.field; \
   op.field = value; \
   op.validate(); \
   op.field = temp; \
}
#define REQUIRE_OP_EVALUATION_SUCCESS( op, field, value ) \
{ \
   const auto temp = op.field; \
   op.field = value; \
   trx.operations.back() = op; \
   op.field = temp; \
   db.push_transaction( trx, ~0 ); \
}

/*#define REQUIRE_THROW( expr, exc_type )          \
{                                                         \
   std::string req_throw_info = fc::json::to_string(      \
      fc::mutable_variant_object()                        \
      ("source_file", __FILE__)                           \
      ("source_lineno", __LINE__)                         \
      ("expr", #expr)                                     \
      ("exc_type", #exc_type)                             \
      );                                                  \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "REQUIRE_THROW begin "        \
         << req_throw_info << std::endl;                  \
   BOOST_REQUIRE_THROW( expr, exc_type );                 \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "REQUIRE_THROW end "          \
         << req_throw_info << std::endl;                  \
}*/

#define REQUIRE_THROW( expr, exc_type )          \
   BOOST_REQUIRE_THROW( expr, exc_type );

#define CHECK_THROW( expr, exc_type )            \
{                                                         \
   std::string req_throw_info = fc::json::to_string(      \
      fc::mutable_variant_object()                        \
      ("source_file", __FILE__)                           \
      ("source_lineno", __LINE__)                         \
      ("expr", #expr)                                     \
      ("exc_type", #exc_type)                             \
      );                                                  \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "CHECK_THROW begin "          \
         << req_throw_info << std::endl;                  \
   BOOST_CHECK_THROW( expr, exc_type );                   \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "CHECK_THROW end "            \
         << req_throw_info << std::endl;                  \
}

#define REQUIRE_OP_VALIDATION_FAILURE_2( op, field, value, exc_type ) \
{ \
   const auto temp = op.field; \
   op.field = value; \
   REQUIRE_THROW( op.validate(), exc_type ); \
   op.field = temp; \
}
#define REQUIRE_OP_VALIDATION_FAILURE( op, field, value ) \
   REQUIRE_OP_VALIDATION_FAILURE_2( op, field, value, fc::exception )

#define REQUIRE_THROW_WITH_VALUE_2(op, field, value, exc_type) \
{ \
   auto bak = op.field; \
   op.field = value; \
   trx.operations.back() = op; \
   op.field = bak; \
   REQUIRE_THROW(db.push_transaction(trx, ~0), exc_type); \
}

#define REQUIRE_THROW_WITH_VALUE( op, field, value ) \
   REQUIRE_THROW_WITH_VALUE_2( op, field, value, fc::exception )

///This simply resets v back to its default-constructed value. Requires v to have a working assingment operator and
/// default constructor.
#define RESET(v) v = decltype(v)()
///This allows me to build consecutive test cases. It's pretty ugly, but it works well enough for unit tests.
/// i.e. This allows a test on update_account to begin with the database at the end state of create_account.
#define INVOKE(test) ((struct test*)this)->test_method(); trx.clear()

#define PREP_ACTOR(name) \
   private_key_type name ## _private_owner_key = protocol::generate_private_key(std::string( BOOST_PP_STRINGIZE(name) ) + "owner" + INIT_ACCOUNT_PASSWORD );             \
   private_key_type name ## _private_active_key = protocol::generate_private_key(std::string( BOOST_PP_STRINGIZE(name) ) + "active" + INIT_ACCOUNT_PASSWORD );           \
   private_key_type name ## _private_posting_key = protocol::generate_private_key(std::string( BOOST_PP_STRINGIZE(name) ) + "posting" + INIT_ACCOUNT_PASSWORD );         \
   private_key_type name ## _private_secure_key = protocol::generate_private_key(std::string( BOOST_PP_STRINGIZE(name) ) + "secure" + INIT_ACCOUNT_PASSWORD );           \
   private_key_type name ## _private_connection_key = protocol::generate_private_key(std::string( BOOST_PP_STRINGIZE(name) ) + "connection" + INIT_ACCOUNT_PASSWORD );   \
   private_key_type name ## _private_friend_key = protocol::generate_private_key(std::string( BOOST_PP_STRINGIZE(name) ) + "friend" + INIT_ACCOUNT_PASSWORD );           \
   private_key_type name ## _private_companion_key = protocol::generate_private_key(std::string( BOOST_PP_STRINGIZE(name) ) + "companion" + INIT_ACCOUNT_PASSWORD );     \
   public_key_type name ## _public_owner_key = name ## _private_owner_key.get_public_key();                                                                              \
   public_key_type name ## _public_active_key = name ## _private_active_key.get_public_key();                                                                            \
   public_key_type name ## _public_posting_key = name ## _private_posting_key.get_public_key();                                                                          \
   public_key_type name ## _public_secure_key = name ## _private_secure_key.get_public_key();                                                                            \
   public_key_type name ## _public_connection_key = name ## _private_connection_key.get_public_key();                                                                    \
   public_key_type name ## _public_friend_key = name ## _private_friend_key.get_public_key();                                                                            \
   public_key_type name ## _public_companion_key = name ## _private_companion_key.get_public_key();                                                          

#define ACTOR(name) \
   PREP_ACTOR(name) \
   const auto& name = account_create( BOOST_PP_STRINGIZE(name), name ## _private_secure_key, name ## _public_owner_key, \
   name ## _public_active_key, name ## _public_posting_key, name ## _public_secure_key, name ## _public_connection_key, \
   name ## _public_friend_key, name ## _public_companion_key );                                                         \
   account_id_type name ## _id = name.id; (void)name ## _id;                                                            \
   test_accounts_set.insert( BOOST_PP_STRINGIZE(name) );

#define GET_ACTOR(name) \
   const account_object& name = get_account(BOOST_PP_STRINGIZE(name)); \
   account_id_type name ## _id = name.id; \
   (void)name ##_id

#define ACTORS_IMPL(r, data, elem) ACTOR(elem)
#define ACTORS(names) BOOST_PP_SEQ_FOR_EACH(ACTORS_IMPL, ~, names) \
   validate_database();

#define ASSET( s ) \
   asset::from_string( s )

namespace node { namespace chain {

   using namespace node::protocol;

   using node::protocol::validate_account_name;
   using node::protocol::validate_community_name;
   using node::protocol::validate_tag_name;
   using node::protocol::validate_permlink;
   using node::protocol::validate_url;
   using node::protocol::validate_uuidv4;
   using node::protocol::validate_public_key;
   using node::protocol::edit_distance;
   using node::protocol::generate_private_key;
   using node::protocol::round_sig_figs;
   using node::protocol::find_msb;
   using node::protocol::approx_sqrt;
   using node::protocol::get_public_key;
   using node::protocol::get_private_key;

   /**
    * The reason we use an app is to exercise the indexes of built-in plugins.
    */
   struct database_fixture {

      node::app::application       app;

      chain::database              &db;

      signed_transaction           trx;

      flat_set<account_name_type>  test_accounts_set;

      fc::ecc::private_key         private_key = fc::ecc::private_key::generate();


      fc::ecc::public_key          init_account_public_owner_key = protocol::get_public_key( INIT_ACCOUNT, "owner", INIT_ACCOUNT_PASSWORD );

      fc::ecc::public_key          init_account_public_active_key = protocol::get_public_key( INIT_ACCOUNT, "active", INIT_ACCOUNT_PASSWORD );

      fc::ecc::public_key          init_account_public_posting_key = protocol::get_public_key( INIT_ACCOUNT, "posting", INIT_ACCOUNT_PASSWORD );

      fc::ecc::public_key          init_account_public_producer_key = protocol::get_public_key( INIT_ACCOUNT, "producer", INIT_ACCOUNT_PASSWORD );


      fc::ecc::private_key         init_account_private_owner_key = protocol::get_private_key( INIT_ACCOUNT, "owner", INIT_ACCOUNT_PASSWORD );

      fc::ecc::private_key         init_account_private_active_key = protocol::get_private_key( INIT_ACCOUNT, "active", INIT_ACCOUNT_PASSWORD );

      fc::ecc::private_key         init_account_private_posting_key = protocol::get_private_key( INIT_ACCOUNT, "posting", INIT_ACCOUNT_PASSWORD );

      fc::ecc::private_key         init_account_private_producer_key = protocol::get_private_key( INIT_ACCOUNT, "producer", INIT_ACCOUNT_PASSWORD );


      string                       init_account_private_owner_wif = graphene::utilities::key_to_wif( init_account_private_owner_key );

      string                       init_account_private_active_wif = graphene::utilities::key_to_wif( init_account_private_active_key );

      string                       init_account_private_posting_wif = graphene::utilities::key_to_wif( init_account_private_posting_key );

      string                       init_account_private_producer_wif = graphene::utilities::key_to_wif( init_account_private_producer_key );

      uint32_t                     default_skip = 0 | database::skip_undo_history_check | database::skip_authority_check;

      std::shared_ptr< node::plugin::debug_node::debug_node_plugin > db_plugin;

      optional<fc::temp_directory> data_dir;
      bool skip_key_index_test = false;
      uint32_t anon_acct_count;

      database_fixture(): app(), db( *app.chain_database() ) {}
      ~database_fixture() {}

      string generate_anon_acct_name();

      void open_database();

      void generate_block();

      void generate_block( uint32_t skip );
      
      void generate_block( uint32_t skip, int miss_blocks );

      /**
       * @brief Generates block_count blocks
       * @param block_count number of blocks to generate
       */
      void generate_blocks( uint32_t block_count );

      /**
       * @brief Generates blocks with a skip 
       * @param block_count Number of blocks to generate
       * @param miss_blocks Number of blocks to skip
       */
      void generate_blocks( uint32_t block_count, int miss_blocks );

      /**
       * @brief Generates blocks until the head block time matches or exceeds timestamp
       * @param timestamp target time to generate blocks until
       */
      void generate_blocks( fc::time_point timestamp, bool miss_intermediate_blocks = true );

      void generate_until_block( uint64_t head_block_num, bool miss_intermediate_blocks = true );

      const account_object& account_create(
         const string& name,
         const private_key_type& private_secure_key,
         const public_key_type& public_owner_key,
         const public_key_type& public_active_key,
         const public_key_type& public_posting_key,
         const public_key_type& public_secure_key,
         const public_key_type& public_connection_key,
         const public_key_type& public_friend_key,
         const public_key_type& public_companion_key
      );

      const account_object& account_create(
         const string& name,
         const private_key_type& private_secure_key,
         const public_key_type& key
      );

      const community_object& community_create(
         const string& name,
         const string& founder,
         const private_key_type& founder_key,
         const public_key_type& community_key,
         const string& community_privacy,
         const string& details,
         const string& url,
         const string& json
      );

      const asset_object& asset_create(
         const string& symbol,
         const string& issuer,
         const private_key_type& issuer_key,
         const string& asset_type,
         const string& details,
         const string& url,
         const string& json,
         const share_type& liquidity
      );

      const producer_object& producer_create(
         const string& owner,
         const private_key_type& owner_key
      );

      const producer_vote_object& producer_vote(
         const string& owner,
         const private_key_type& owner_key
      );

      const comment_object& comment_create(
         const string& author, 
         const private_key_type& author_key,
         const string& permlink
      );

      void generate_liquid( const string& account_name, const asset& amount );

      void generate_stake( const string& from, const asset& amount );

      void generate_reward( const string& from, const asset& amount );

      void generate_savings( const string& from, const asset& amount );

      void fund_liquid( const string& account_name, const asset& amount );

      void fund_stake( const string& from, const asset& amount );

      void fund_savings( const string& from, const asset& amount );
      
      void proxy( const string& account, const string& proxy );

      asset get_liquid_balance( const string& account_name, const string& symbol )const;

      asset get_staked_balance( const string& account_name, const string& symbol )const;

      asset get_savings_balance( const string& account_name, const string& symbol )const;

      asset get_reward_balance( const string& account_name, const string& symbol )const;

      time_point now()const;

      void sign( signed_transaction& trx, const fc::ecc::private_key& key );

      vector< operation > get_last_operations( uint32_t ops );

      void validate_database( void );

      string get_encrypted_message( string from_private_key, string from_public_key, string to_public_key, string message ) const;

      string get_encrypted_message( private_key_type from_private_key, public_key_type from_public_key, public_key_type to_public_key, string message ) const;

      struct encrypted_message_data
      {
         static optional< encrypted_message_data > from_string( string str ) 
         {
            try
            {
               if( str.size() > sizeof( encrypted_message_data ) && str[0] == '#')
               {
                  auto data = fc::from_base58( str.substr(1) );
                  auto m  = fc::raw::unpack< encrypted_message_data >( data );
                  FC_ASSERT( string( m ) == str );
                  return m;
               }
            }
            catch ( ... ) {}

            return optional< encrypted_message_data >();
         }

         public_key_type          from;          ///< Public key of sending account.

         public_key_type          to;            ///< Public key of the receiving account.

         uint64_t                 nonce = 0;     ///< Iterated value derived from the time of encryption.

         uint32_t                 check = 0;     ///< Hash checksum of the plaintext.

         vector< char >           encrypted;     ///< Raw encrypted data of the message.

         operator string()const                  ///< Returns the base58 Hash-prefixed compressed object.
         {
            auto data = fc::raw::pack( *this );
            auto base58 = fc::to_base58( data );
            return '#'+base58;
         }
      };
   };

   struct clean_database_fixture : public database_fixture
   {
      clean_database_fixture();
      ~clean_database_fixture();

      void resize_shared_mem( uint64_t size );
   };

   struct live_database_fixture : public database_fixture
   {
      live_database_fixture();
      ~live_database_fixture();

      fc::path _chain_dir;
   };

   namespace test
   {
      bool _push_block( database& db, const signed_block& b, uint32_t skip_flags = 0 );
      void _push_transaction( database& db, const signed_transaction& tx, uint32_t skip_flags = 0 );
   }

} }   // node::chain

FC_REFLECT( node::chain::database_fixture::encrypted_message_data, 
         (from)
         (to)
         (nonce)
         (check)
         (encrypted)
         );