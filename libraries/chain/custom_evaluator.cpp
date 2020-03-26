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


//===========================//
// === Custom Evaluators === //
//===========================//


void custom_evaluator::do_apply( const custom_operation& o )
{ try {
   if( _db.is_producing() )
   {
      FC_ASSERT( o.data.size() <= 8192,
         "Data must be less than 8192 characters" );
   }  
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void custom_json_evaluator::do_apply( const custom_json_operation& o )
{ try {
   if( _db.is_producing() )
   {
      FC_ASSERT( o.json.size() <= 8192,
         "JSON must be less than 8192 characters" );
   }
      
   std::shared_ptr< custom_operation_interpreter > eval = _db.get_custom_json_evaluator( o.id );
   if( !eval )
   {
      return;
   }

   try
   {
      eval->apply( o );
   }
   catch( const fc::exception& e )
   {
      if( _db.is_producing() )
      {
         throw e;
      }  
   }
   catch(...)
   {
      elog( "Unexpected exception applying custom json evaluator." );
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain