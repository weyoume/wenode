#include <node/app/api_context.hpp>
#include <node/app/application.hpp>
#include <node/app/database_api.hpp>

#include <node/protocol/get_config.hpp>

#include <node/chain/util/reward.hpp>

#include <fc/bloom_filter.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/crypto/hex.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#include <cctype>
#include <cfenv>
#include <iostream>

namespace node { namespace app {


   //=====================================//
   // === Blocks and Transactions API === //
   //=====================================//


optional< block_header > database_api::get_block_header( uint64_t block_num )const
{
   FC_ASSERT( !my->_disable_get_block,
      "get_block_header is disabled on this node." );

   return my->_db.with_read_lock( [&]()
   {
      return my->get_block_header( block_num );
   });
}


optional< block_header > database_api_impl::get_block_header( uint64_t block_num ) const
{
   auto results = _db.fetch_block_by_number( block_num );
   if( results )
   {
      return *results;
   }
      
   return {};
}


optional< signed_block_api_obj > database_api::get_block( uint64_t block_num )const
{
   FC_ASSERT( !my->_disable_get_block,
      "get_block is disabled on this node." );

   return my->_db.with_read_lock( [&]()
   {
      return my->get_block( block_num );
   });
}


optional< signed_block_api_obj > database_api_impl::get_block( uint64_t block_num )const
{
   return _db.fetch_block_by_number( block_num );
}


vector< applied_operation > database_api::get_ops_in_block( uint64_t block_num, bool only_virtual )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_ops_in_block( block_num, only_virtual );
   });
}


vector< applied_operation > database_api_impl::get_ops_in_block(uint64_t block_num, bool only_virtual)const
{
   const auto& operation_idx = _db.get_index< operation_index >().indices().get< by_location >();
   auto operation_itr = operation_idx.lower_bound( block_num );
   vector< applied_operation> results;
   applied_operation temp;

   while( operation_itr != operation_idx.end() && operation_itr->block == block_num )
   {
      temp = *operation_itr;
      if( !only_virtual || is_virtual_operation(temp.op) )
      {
         results.push_back(temp);
      } 
      ++operation_itr;
   }
   return results;
}


std::string database_api::get_transaction_hex( const signed_transaction& trx )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_transaction_hex( trx );
   });
}

std::string database_api_impl::get_transaction_hex( const signed_transaction& trx )const
{
   return fc::to_hex( fc::raw::pack( trx ) );
}

annotated_signed_transaction database_api::get_transaction( transaction_id_type id )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_transaction( id );
   });
}

annotated_signed_transaction database_api_impl::get_transaction( transaction_id_type id )const
{
   #ifndef SKIP_BY_TX_ID
   const auto& operation_idx = _db.get_index< operation_index >().indices().get< by_transaction_id >();
   auto operation_itr = operation_idx.lower_bound( id );

   annotated_signed_transaction results;

   if( operation_itr != operation_idx.end() && operation_itr->trx_id == id )
   {
      auto blk = _db.fetch_block_by_number( operation_itr->block );
      FC_ASSERT( blk.valid() );
      FC_ASSERT( blk->transactions.size() > operation_itr->trx_in_block );
      results = blk->transactions[ operation_itr->trx_in_block ];
      results.block_num = operation_itr->block;
      results.transaction_num = operation_itr->trx_in_block;
      return results;
   }

   #endif

   FC_ASSERT( false, "Unknown Transaction ${t}", ("t",id));
}

set< public_key_type > database_api::get_required_signatures( const signed_transaction& trx, const flat_set< public_key_type >& available_keys )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_required_signatures( trx, available_keys );
   });
}

set< public_key_type > database_api_impl::get_required_signatures( const signed_transaction& trx, const flat_set < public_key_type >& available_keys )const
{
   set< public_key_type > results = trx.get_required_signatures(
      CHAIN_ID,
      available_keys,
      [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).active_auth  ); },
      [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).owner_auth   ); },
      [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).posting_auth ); },
      MAX_SIG_CHECK_DEPTH );
   return results;
}

set< public_key_type > database_api::get_potential_signatures( const signed_transaction& trx )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_potential_signatures( trx );
   });
}

set< public_key_type > database_api_impl::get_potential_signatures( const signed_transaction& trx )const
{
   set< public_key_type > results;
   trx.get_required_signatures(
      CHAIN_ID,
      flat_set< public_key_type >(),
      [&]( account_name_type account_name )
      {
         const auto& auth = _db.get< account_authority_object, by_account >(account_name).active_auth;
         for( const auto& k : auth.get_keys() )
            results.insert(k);
         return authority( auth );
      },
      [&]( account_name_type account_name )
      {
         const auto& auth = _db.get< account_authority_object, by_account >(account_name).owner_auth;
         for( const auto& k : auth.get_keys() )
            results.insert(k);
         return authority( auth );
      },
      [&]( account_name_type account_name )
      {
         const auto& auth = _db.get< account_authority_object, by_account >(account_name).posting_auth;
         for( const auto& k : auth.get_keys() )
            results.insert(k);
         return authority( auth );
      },
      MAX_SIG_CHECK_DEPTH
   );

   return results;
}

bool database_api::verify_authority( const signed_transaction& trx ) const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->verify_authority( trx );
   });
}

bool database_api_impl::verify_authority( const signed_transaction& trx )const
{
   trx.verify_authority( 
      CHAIN_ID,
      [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).active_auth  ); },
      [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).owner_auth   ); },
      [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).posting_auth ); },
      MAX_SIG_CHECK_DEPTH );
   return true;
}

bool database_api::verify_account_authority( const string& name_or_id, const flat_set< public_key_type >& signers )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->verify_account_authority( name_or_id, signers );
   });
}

bool database_api_impl::verify_account_authority( const string& name, const flat_set< public_key_type >& keys )const
{
   FC_ASSERT( name.size() > 0,
      "Verify request must include account name.");
   const account_object* account = _db.find< account_object, by_name >( name );
   FC_ASSERT( account != nullptr, 
      "No such account" );
   signed_transaction trx;
   transfer_operation op;
   op.from = account->name;
   trx.operations.emplace_back( op );

   return verify_authority( trx );    // Confirm authority is able to sign a transfer operation
}


} } // node::app