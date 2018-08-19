#include <eznode/chain/eznode_evaluator.hpp>
#include <eznode/chain/database.hpp>
#include <eznode/chain/custom_operation_interpreter.hpp>
#include <eznode/chain/eznode_objects.hpp>
#include <eznode/chain/witness_objects.hpp>
#include <eznode/chain/block_summary_object.hpp>

#include <eznode/chain/util/reward.hpp>

#ifndef IS_LOW_MEM
#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>

using boost::locale::conv::utf_to_utf;

std::wstring utf8_to_wstring(const std::string& str)
{
    return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
}

std::string wstring_to_utf8(const std::wstring& str)
{
    return utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
}

#endif

#include <fc/uint128.hpp>
#include <fc/utf8.hpp>

#include <limits>

namespace eznode { namespace chain {
   using fc::uint128_t;

inline void validate_permlink_0_1( const string& permlink )
{
   FC_ASSERT( permlink.size() > MIN_PERMLINK_LENGTH && permlink.size() < MAX_PERMLINK_LENGTH, "Permlink is not a valid size." );

   for( auto c : permlink )
   {
      switch( c )
      {
         case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
         case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
         case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z': case '0':
         case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
         case '-':
            break;
         default:
            FC_ASSERT( false, "Invalid permlink character: ${s}", ("s", std::string() + c ) );
      }
   }
}

struct strcmp_equal
{
   bool operator()( const shared_string& a, const string& b )
   {
      return a.size() == b.size() || std::strcmp( a.c_str(), b.c_str() ) == 0;
   }
};

void witness_update_evaluator::do_apply( const witness_update_operation& o )
{
   _db.get_account( o.owner ); // verify owner exists

   if ( _db.has_hardfork( HARDFORK_0_1 ) )
   {
      FC_ASSERT( o.url.size() <= MAX_WITNESS_URL_LENGTH, "URL is too long" );
   }
   else if( o.url.size() > MAX_WITNESS_URL_LENGTH )
   {
      // after HF, above check can be moved to validate() if reindex doesn't show this warning
      wlog( "URL is too long in block ${b}", ("b", _db.head_block_num()+1) );
   }

   if ( _db.has_hardfork( HARDFORK_0_14__410 ) )
   {
      FC_ASSERT( o.props.account_creation_fee.symbol == SYMBOL_ECO );
   }
   else if( o.props.account_creation_fee.symbol != SYMBOL_ECO )
   {
      // after HF, above check can be moved to validate() if reindex doesn't show this warning
      wlog( "Wrong fee symbol in block ${b}", ("b", _db.head_block_num()+1) );
   }

   const auto& by_witness_name_idx = _db.get_index< witness_index >().indices().get< by_name >();
   auto wit_itr = by_witness_name_idx.find( o.owner );
   if( wit_itr != by_witness_name_idx.end() )
   {
      _db.modify( *wit_itr, [&]( witness_object& w ) {
         from_string( w.url, o.url );
         w.signing_key        = o.block_signing_key;
         w.props              = o.props;
      });
   }
   else
   {
      _db.create< witness_object >( [&]( witness_object& w ) {
         w.owner              = o.owner;
         from_string( w.url, o.url );
         w.signing_key        = o.block_signing_key;
         w.created            = _db.head_block_time();
         w.props              = o.props;
      });
   }
}

void accountCreate_evaluator::do_apply( const accountCreate_operation& o )
{
   const auto& creator = _db.get_account( o.creator );

   const auto& props = _db.get_dynamic_global_properties();

   FC_ASSERT( creator.balance >= o.fee, "Insufficient balance to create account.", ( "creator.balance", creator.balance )( "required", o.fee ) );

   if( _db.has_hardfork( HARDFORK_0_19__987) )
   {
      const witness_schedule_object& wso = _db.get_witness_schedule_object();
      FC_ASSERT( o.fee >= asset( wso.median_props.account_creation_fee.amount * CREATE_ACCOUNT_WITH_ECO_MODIFIER, SYMBOL_ECO ), "Insufficient Fee: ${f} required, ${p} provided.",
                 ("f", wso.median_props.account_creation_fee * asset( CREATE_ACCOUNT_WITH_ECO_MODIFIER, SYMBOL_ECO ) )
                 ("p", o.fee) );
   }
   else if( _db.has_hardfork( HARDFORK_0_1 ) )
   {
      const witness_schedule_object& wso = _db.get_witness_schedule_object();
      FC_ASSERT( o.fee >= wso.median_props.account_creation_fee, "Insufficient Fee: ${f} required, ${p} provided.",
                 ("f", wso.median_props.account_creation_fee)
                 ("p", o.fee) );
   }

   if( _db.has_hardfork( HARDFORK_0_15__465 ) )
   {
      for( auto& a : o.owner.account_auths )
      {
         _db.get_account( a.first );
      }

      for( auto& a : o.active.account_auths )
      {
         _db.get_account( a.first );
      }

      for( auto& a : o.posting.account_auths )
      {
         _db.get_account( a.first );
      }
   }

   _db.modify( creator, [&]( account_object& c ){
      c.balance -= o.fee;
   });

   const auto& new_account = _db.create< account_object >( [&]( account_object& acc )
   {
      acc.name = o.newAccountName;
      acc.memoKey = o.memoKey;
      acc.created = props.time;
      acc.last_vote_time = props.time;
      acc.mined = false;

      if( !_db.has_hardfork( HARDFORK_0_11__169 ) )
         acc.recoveryAccount = "ezira";
      else
         acc.recoveryAccount = o.creator;


      #ifndef IS_LOW_MEM
         from_string( acc.json, o.json );
      #endif
   });

   _db.create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = o.newAccountName;
      auth.owner = o.owner;
      auth.active = o.active;
      auth.posting = o.posting;
      auth.last_owner_update = fc::time_point_sec::min();
   });

   if( o.fee.amount > 0 )
      _db.createECOfundForESCOR( new_account, o.fee );
}

void accountCreateWithDelegation_evaluator::do_apply( const accountCreateWithDelegation_operation& o )
{
   FC_ASSERT( _db.has_hardfork( HARDFORK_0_17__818 ), "Account creation with delegation is not enabled until hardfork 17" );

   const auto& creator = _db.get_account( o.creator );
   const auto& props = _db.get_dynamic_global_properties();
   const witness_schedule_object& wso = _db.get_witness_schedule_object();

   FC_ASSERT( creator.balance >= o.fee, "Insufficient balance to create account.",
               ( "creator.balance", creator.balance )
               ( "required", o.fee ) );

   FC_ASSERT( creator.ESCOR - creator.ESCORDelegated - asset( creator.to_withdraw - creator.withdrawn, SYMBOL_ESCOR ) >= o.delegation, "Insufficient eScore to delegate to new account.",
               ( "creator.ESCOR", creator.ESCOR )
               ( "creator.ESCORDelegated", creator.ESCORDelegated )( "required", o.delegation ) );

   auto target_delegation = asset( wso.median_props.account_creation_fee.amount * CREATE_ACCOUNT_WITH_ECO_MODIFIER * CREATE_ACCOUNT_DELEGATION_RATIO, SYMBOL_ECO ) * props.get_ESCOR_price();

   auto current_delegation = asset( o.fee.amount * CREATE_ACCOUNT_DELEGATION_RATIO, SYMBOL_ECO ) * props.get_ESCOR_price() + o.delegation;

   FC_ASSERT( current_delegation >= target_delegation, "Inssufficient Delegation ${f} required, ${p} provided.",
               ("f", target_delegation )
               ( "p", current_delegation )
               ( "account_creation_fee", wso.median_props.account_creation_fee )
               ( "o.fee", o.fee )
               ( "o.delegation", o.delegation ) );

   FC_ASSERT( o.fee >= wso.median_props.account_creation_fee, "Insufficient Fee: ${f} required, ${p} provided.",
               ("f", wso.median_props.account_creation_fee)
               ("p", o.fee) );

   for( auto& a : o.owner.account_auths )
   {
      _db.get_account( a.first );
   }

   for( auto& a : o.active.account_auths )
   {
      _db.get_account( a.first );
   }

   for( auto& a : o.posting.account_auths )
   {
      _db.get_account( a.first );
   }

   _db.modify( creator, [&]( account_object& c )
   {
      c.balance -= o.fee;
      c.ESCORDelegated += o.delegation;
   });

   const auto& new_account = _db.create< account_object >( [&]( account_object& acc )
   {
      acc.name = o.newAccountName;
      acc.memoKey = o.memoKey;
      acc.created = props.time;
      acc.last_vote_time = props.time;
      acc.mined = false;

      acc.recoveryAccount = o.creator;

      acc.ESCORreceived = o.delegation;

      #ifndef IS_LOW_MEM
         from_string( acc.json, o.json );
      #endif
   });

   _db.create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = o.newAccountName;
      auth.owner = o.owner;
      auth.active = o.active;
      auth.posting = o.posting;
      auth.last_owner_update = fc::time_point_sec::min();
   });

   if( o.delegation.amount > 0 || !_db.has_hardfork( HARDFORK_0_19__997 ) )
   {
      _db.create< ECO_fund_for_ESCOR_delegation_object >( [&]( ECO_fund_for_ESCOR_delegation_object& vdo )
      {
         vdo.delegator = o.creator;
         vdo.delegatee = o.newAccountName;
         vdo.ESCOR = o.delegation;
         vdo.min_delegation_time = _db.head_block_time() + CREATE_ACCOUNT_DELEGATION_TIME;
      });
   }

   if( o.fee.amount > 0 )
      _db.createECOfundForESCOR( new_account, o.fee );
}


void accountUpdate_evaluator::do_apply( const accountUpdate_operation& o )
{
   if( _db.has_hardfork( HARDFORK_0_1 ) ) FC_ASSERT( o.account != TEMP_ACCOUNT, "Cannot update temp account." );

   if( ( _db.has_hardfork( HARDFORK_0_15__465 ) ) && o.posting )
      o.posting->validate();

   const auto& account = _db.get_account( o.account );
   const auto& account_auth = _db.get< account_authority_object, by_account >( o.account );

   if( o.owner )
   {
#ifndef IS_TEST_NET
      if( _db.has_hardfork( HARDFORK_0_11 ) )
         FC_ASSERT( _db.head_block_time() - account_auth.last_owner_update > OWNER_UPDATE_LIMIT, "Owner authority can only be updated once an hour." );
#endif

      if( ( _db.has_hardfork( HARDFORK_0_15__465 ) ) )
      {
         for( auto a: o.owner->account_auths )
         {
            _db.get_account( a.first );
         }
      }


      _db.update_owner_authority( account, *o.owner );
   }

   if( o.active && ( _db.has_hardfork( HARDFORK_0_15__465 ) ) )
   {
      for( auto a: o.active->account_auths )
      {
         _db.get_account( a.first );
      }
   }

   if( o.posting && ( _db.has_hardfork( HARDFORK_0_15__465 ) ) )
   {
      for( auto a: o.posting->account_auths )
      {
         _db.get_account( a.first );
      }
   }

   _db.modify( account, [&]( account_object& acc )
   {
      if( o.memoKey != public_key_type() )
            acc.memoKey = o.memoKey;

      if( ( o.active || o.owner ) && acc.active_challenged )
      {
         acc.active_challenged = false;
         acc.last_active_proved = _db.head_block_time();
      }

      acc.last_accountUpdate = _db.head_block_time();

      #ifndef IS_LOW_MEM
        if ( o.json.size() > 0 )
            from_string( acc.json, o.json );
      #endif
   });

   if( o.active || o.posting )
   {
      _db.modify( account_auth, [&]( account_authority_object& auth)
      {
         if( o.active )  auth.active  = *o.active;
         if( o.posting ) auth.posting = *o.posting;
      });
   }

}


/**
 *  Because net_ESCORreward is 0 there is no need to update any pending payout calculations or parent posts.
 */
void deleteComment_evaluator::do_apply( const deleteComment_operation& o )
{
   if( _db.has_hardfork( HARDFORK_0_10 ) )
   {
      const auto& auth = _db.get_account( o.author );
      FC_ASSERT( !(auth.owner_challenged || auth.active_challenged ), "Operation cannot be processed because account is currently challenged." );
   }

   const auto& comment = _db.get_comment( o.author, o.permlink );
   FC_ASSERT( comment.children == 0, "Cannot delete a comment with replies." );

   if( _db.has_hardfork( HARDFORK_0_19__876 ) )
      FC_ASSERT( comment.cashout_time != fc::time_point_sec::maximum() );

   if( _db.has_hardfork( HARDFORK_0_19__977 ) )
      FC_ASSERT( comment.net_ESCORreward <= 0, "Cannot delete a comment with net positive votes." );

   if( comment.net_ESCORreward > 0 ) return;

   const auto& vote_idx = _db.get_index<comment_vote_index>().indices().get<by_comment_voter>();

   auto vote_itr = vote_idx.lower_bound( comment_id_type(comment.id) );
   while( vote_itr != vote_idx.end() && vote_itr->comment == comment.id ) {
      const auto& cur_vote = *vote_itr;
      ++vote_itr;
      _db.remove(cur_vote);
   }

   /// this loop can be skiped for validate-only nodes as it is merely gathering stats for indicies
   if( _db.has_hardfork( HARDFORK_0_6__80 ) && comment.parent_author != ROOT_POST_PARENT )
   {
      auto parent = &_db.get_comment( comment.parent_author, comment.parent_permlink );
      auto now = _db.head_block_time();
      while( parent )
      {
         _db.modify( *parent, [&]( comment_object& p ){
            p.children--;
            p.active = now;
         });
   #ifndef IS_LOW_MEM
         if( parent->parent_author != ROOT_POST_PARENT )
            parent = &_db.get_comment( parent->parent_author, parent->parent_permlink );
         else
   #endif
            parent = nullptr;
      }
   }

   _db.remove( comment );
}

struct comment_options_extension_visitor
{
   comment_options_extension_visitor( const comment_object& c, database& db ) : _c( c ), _db( db ) {}

   typedef void result_type;

   const comment_object& _c;
   database& _db;

   void operator()( const comment_payout_beneficiaries& cpb ) const
   {
      FC_ASSERT( _c.beneficiaries.size() == 0, "Comment already has beneficiaries specified." );
      FC_ASSERT( _c.abs_ESCORreward == 0, "Comment must not have been voted on before specifying beneficiaries." );

      _db.modify( _c, [&]( comment_object& c )
      {
         for( auto& b : cpb.beneficiaries )
         {
            auto acc = _db.find< account_object, by_name >( b.account );
            FC_ASSERT( acc != nullptr, "Beneficiary \"${a}\" must exist.", ("a", b.account) );
            c.beneficiaries.push_back( b );
         }
      });
   }
};

void comment_options_evaluator::do_apply( const comment_options_operation& o )
{
   if( _db.has_hardfork( HARDFORK_0_10 ) )
   {
      const auto& auth = _db.get_account( o.author );
      FC_ASSERT( !(auth.owner_challenged || auth.active_challenged ), "Operation cannot be processed because account is currently challenged." );
   }

   const auto& comment = _db.get_comment( o.author, o.permlink );
   if( !o.allow_curationRewards || !o.allow_votes || o.max_accepted_payout < comment.max_accepted_payout )
      FC_ASSERT( comment.abs_ESCORreward == 0, "One of the included comment options requires the comment to have no ESCORreward allocated to it." );

   FC_ASSERT( comment.allow_curationRewards >= o.allow_curationRewards, "Curation rewards cannot be re-enabled." );
   FC_ASSERT( comment.allow_votes >= o.allow_votes, "Voting cannot be re-enabled." );
   FC_ASSERT( comment.max_accepted_payout >= o.max_accepted_payout, "A comment cannot accept a greater payout." );
   FC_ASSERT( comment.percent_EUSD >= o.percent_EUSD, "A comment cannot accept a greater percent EUSD." );

   _db.modify( comment, [&]( comment_object& c ) {
       c.max_accepted_payout   = o.max_accepted_payout;
       c.percent_EUSD = o.percent_EUSD;
       c.allow_votes           = o.allow_votes;
       c.allow_curationRewards = o.allow_curationRewards;
   });

   for( auto& e : o.extensions )
   {
      e.visit( comment_options_extension_visitor( comment, _db ) );
   }
}

void comment_evaluator::do_apply( const comment_operation& o )
{ try {
   if( _db.has_hardfork( HARDFORK_0_5__55 ) )
      FC_ASSERT( o.title.size() + o.body.size() + o.json.size(), "Cannot update comment because nothing appears to be changing." );

   const auto& by_permlink_idx = _db.get_index< comment_index >().indices().get< by_permlink >();
   auto itr = by_permlink_idx.find( boost::make_tuple( o.author, o.permlink ) );

   const auto& auth = _db.get_account( o.author ); /// prove it exists

   if( _db.has_hardfork( HARDFORK_0_10 ) )
      FC_ASSERT( !(auth.owner_challenged || auth.active_challenged ), "Operation cannot be processed because account is currently challenged." );

   comment_id_type id;

   const comment_object* parent = nullptr;
   if( o.parent_author != ROOT_POST_PARENT )
   {
      parent = &_db.get_comment( o.parent_author, o.parent_permlink );
      if( !_db.has_hardfork( HARDFORK_0_17__767 ) )
         FC_ASSERT( parent->depth < MAX_COMMENT_DEPTH_PRE_HF17, "Comment is nested ${x} posts deep, maximum depth is ${y}.", ("x",parent->depth)("y",MAX_COMMENT_DEPTH_PRE_HF17) );
      else
         FC_ASSERT( parent->depth < MAX_COMMENT_DEPTH, "Comment is nested ${x} posts deep, maximum depth is ${y}.", ("x",parent->depth)("y",MAX_COMMENT_DEPTH) );
   }

   if( ( _db.has_hardfork( HARDFORK_0_17__926 ) ) && o.json.size() )
      FC_ASSERT( fc::is_utf8( o.json ), "JSON Metadata must be UTF-8" );

   auto now = _db.head_block_time();

   if ( itr == by_permlink_idx.end() )
   {
      if( o.parent_author != ROOT_POST_PARENT )
      {
         FC_ASSERT( _db.get( parent->root_comment ).allow_replies, "The parent comment has disabled replies." );
         if( _db.has_hardfork( HARDFORK_0_12__177 ) && !_db.has_hardfork( HARDFORK_0_17__869 ) )
            FC_ASSERT( _db.calculate_discussion_payout_time( *parent ) != fc::time_point_sec::maximum(), "Discussion is frozen." );
      }

      if( _db.has_hardfork( HARDFORK_0_12__176 ) )
      {
         if( o.parent_author == ROOT_POST_PARENT )
             FC_ASSERT( ( now - auth.last_root_post ) > MIN_ROOT_COMMENT_INTERVAL, "You may only post once every 5 minutes.", ("now",now)("last_root_post", auth.last_root_post) );
         else
             FC_ASSERT( (now - auth.last_post) > MIN_REPLY_INTERVAL, "You may only comment once every 20 seconds.", ("now",now)("auth.last_post",auth.last_post) );
      }
      else if( _db.has_hardfork( HARDFORK_0_6__113 ) )
      {
         if( o.parent_author == ROOT_POST_PARENT )
             FC_ASSERT( (now - auth.last_post) > MIN_ROOT_COMMENT_INTERVAL, "You may only post once every 5 minutes.", ("now",now)("auth.last_post",auth.last_post) );
         else
             FC_ASSERT( (now - auth.last_post) > MIN_REPLY_INTERVAL, "You may only comment once every 20 seconds.", ("now",now)("auth.last_post",auth.last_post) );
      }
      else
      {
         FC_ASSERT( (now - auth.last_post) > fc::seconds(60), "You may only post once per minute.", ("now",now)("auth.last_post",auth.last_post) );
      }

      uint16_t reward_weight = PERCENT_100;
      uint64_t post_bandwidth = auth.post_bandwidth;

      if( _db.has_hardfork( HARDFORK_0_12__176 ) && !_db.has_hardfork( HARDFORK_0_17__733 ) && o.parent_author == ROOT_POST_PARENT )
      {
         uint64_t post_delta_time = std::min( _db.head_block_time().sec_since_epoch() - auth.last_root_post.sec_since_epoch(), POST_AVERAGE_WINDOW );
         uint32_t old_weight = uint32_t( ( post_bandwidth * ( POST_AVERAGE_WINDOW - post_delta_time ) ) / POST_AVERAGE_WINDOW );
         post_bandwidth = ( old_weight + PERCENT_100 );
         reward_weight = uint16_t( std::min( ( POST_WEIGHT_CONSTANT * PERCENT_100 ) / ( post_bandwidth * post_bandwidth ), uint64_t( PERCENT_100 ) ) );
      }

      _db.modify( auth, [&]( account_object& a ) {
         if( o.parent_author == ROOT_POST_PARENT )
         {
            a.last_root_post = now;
            a.post_bandwidth = uint32_t( post_bandwidth );
         }
         a.last_post = now;
         a.post_count++;
      });

      const auto& new_comment = _db.create< comment_object >( [&]( comment_object& com )
      {
         if( _db.has_hardfork( HARDFORK_0_1 ) )
         {
            validate_permlink_0_1( o.parent_permlink );
            validate_permlink_0_1( o.permlink );
         }

         com.author = o.author;
         from_string( com.permlink, o.permlink );
         com.last_update = _db.head_block_time();
         com.created = com.last_update;
         com.active = com.last_update;
         com.last_payout = fc::time_point_sec::min();
         com.max_cashout_time = fc::time_point_sec::maximum();
         com.reward_weight = reward_weight;

         if ( o.parent_author == ROOT_POST_PARENT )
         {
            com.parent_author = "";
            from_string( com.parent_permlink, o.parent_permlink );
            from_string( com.category, o.parent_permlink );
            com.root_comment = com.id;
            com.cashout_time = _db.has_hardfork( HARDFORK_0_12__177 ) ?
               _db.head_block_time() + CASHOUT_WINDOW_SECONDS_PRE_HF17 :
               fc::time_point_sec::maximum();
         }
         else
         {
            com.parent_author = parent->author;
            com.parent_permlink = parent->permlink;
            com.depth = parent->depth + 1;
            com.category = parent->category;
            com.root_comment = parent->root_comment;
            com.cashout_time = fc::time_point_sec::maximum();
         }

         if( _db.has_hardfork( HARDFORK_0_17__769 ) )
         {
            com.cashout_time = com.created + CASHOUT_WINDOW_SECONDS;
         }

         #ifndef IS_LOW_MEM
            from_string( com.title, o.title );
            if( o.body.size() < 1024*1024*128 )
            {
               from_string( com.body, o.body );
            }
            if( fc::is_utf8( o.json ) )
               from_string( com.json, o.json );
            else
               wlog( "Comment ${a}/${p} contains invalid UTF-8 metadata", ("a", o.author)("p", o.permlink) );
         #endif
      });

      id = new_comment.id;

/// this loop can be skiped for validate-only nodes as it is merely gathering stats for indicies
      auto now = _db.head_block_time();
      while( parent ) {
         _db.modify( *parent, [&]( comment_object& p ){
            p.children++;
            p.active = now;
         });
#ifndef IS_LOW_MEM
         if( parent->parent_author != ROOT_POST_PARENT )
            parent = &_db.get_comment( parent->parent_author, parent->parent_permlink );
         else
#endif
            parent = nullptr;
      }

   }
   else // start edit case
   {
      const auto& comment = *itr;

      if( !_db.has_hardfork( HARDFORK_0_17__772 ) )
      {
         if( _db.has_hardfork( HARDFORK_0_14__306 ) )
            FC_ASSERT( _db.calculate_discussion_payout_time( comment ) != fc::time_point_sec::maximum(), "The comment is archived." );
         else if( _db.has_hardfork( HARDFORK_0_10 ) )
            FC_ASSERT( comment.last_payout == fc::time_point_sec::min(), "Can only edit during the first 24 hours." );
      }
      _db.modify( comment, [&]( comment_object& com )
      {
         com.last_update   = _db.head_block_time();
         com.active        = com.last_update;
         strcmp_equal equal;

         if( !parent )
         {
            FC_ASSERT( com.parent_author == account_name_type(), "The parent of a comment cannot change." );
            FC_ASSERT( equal( com.parent_permlink, o.parent_permlink ), "The permlink of a comment cannot change." );
         }
         else
         {
            FC_ASSERT( com.parent_author == o.parent_author, "The parent of a comment cannot change." );
            FC_ASSERT( equal( com.parent_permlink, o.parent_permlink ), "The permlink of a comment cannot change." );
         }

         #ifndef IS_LOW_MEM
           if( o.title.size() )         from_string( com.title, o.title );
           if( o.json.size() )
           {
              if( fc::is_utf8( o.json ) )
                 from_string( com.json, o.json );
              else
                 wlog( "Comment ${a}/${p} contains invalid UTF-8 metadata", ("a", o.author)("p", o.permlink) );
           }

           if( o.body.size() ) {
              try {
               diff_match_patch<std::wstring> dmp;
               auto patch = dmp.patch_fromText( utf8_to_wstring(o.body) );
               if( patch.size() ) {
                  auto result = dmp.patch_apply( patch, utf8_to_wstring( to_string( com.body ) ) );
                  auto patched_body = wstring_to_utf8(result.first);
                  if( !fc::is_utf8( patched_body ) ) {
                     idump(("invalid utf8")(patched_body));
                     from_string( com.body, fc::prune_invalid_utf8(patched_body) );
                  } else { from_string( com.body, patched_body ); }
               }
               else { // replace
                  from_string( com.body, o.body );
               }
              } catch ( ... ) {
                  from_string( com.body, o.body );
              }
           }
         #endif

      });

   } // end EDIT case

} FC_CAPTURE_AND_RETHROW( (o) ) }

void escrow_transfer_evaluator::do_apply( const escrow_transfer_operation& o )
{
   try
   {
      const auto& from_account = _db.get_account(o.from);
      _db.get_account(o.to);
      _db.get_account(o.agent);

      FC_ASSERT( o.ratification_deadline > _db.head_block_time(), "The escorw ratification deadline must be after head block time." );
      FC_ASSERT( o.escrow_expiration > _db.head_block_time(), "The escrow expiration must be after head block time." );

      asset ECO_spent = o.ECOamount;
      asset EUSD_spent = o.EUSDamount;
      if( o.fee.symbol == SYMBOL_ECO )
         ECO_spent += o.fee;
      else
         EUSD_spent += o.fee;

      FC_ASSERT( from_account.balance >= ECO_spent, "Account cannot cover ECO costs of escrow. Required: ${r} Available: ${a}", ("r",ECO_spent)("a",from_account.balance) );
      FC_ASSERT( from_account.EUSDbalance >= EUSD_spent, "Account cannot cover EUSD costs of escrow. Required: ${r} Available: ${a}", ("r",EUSD_spent)("a",from_account.EUSDbalance) );

      _db.adjust_balance( from_account, -ECO_spent );
      _db.adjust_balance( from_account, -EUSD_spent );

      _db.create<escrow_object>([&]( escrow_object& esc )
      {
         esc.escrow_id              = o.escrow_id;
         esc.from                   = o.from;
         esc.to                     = o.to;
         esc.agent                  = o.agent;
         esc.ratification_deadline  = o.ratification_deadline;
         esc.escrow_expiration      = o.escrow_expiration;
         esc.EUSDbalance            = o.EUSDamount;
         esc.ECObalance            = o.ECOamount;
         esc.pending_fee            = o.fee;
      });
   }
   FC_CAPTURE_AND_RETHROW( (o) )
}

void escrow_approve_evaluator::do_apply( const escrow_approve_operation& o )
{
   try
   {

      const auto& escrow = _db.get_escrow( o.from, o.escrow_id );

      FC_ASSERT( escrow.to == o.to, "Operation 'to' (${o}) does not match escrow 'to' (${e}).", ("o", o.to)("e", escrow.to) );
      FC_ASSERT( escrow.agent == o.agent, "Operation 'agent' (${a}) does not match escrow 'agent' (${e}).", ("o", o.agent)("e", escrow.agent) );
      FC_ASSERT( escrow.ratification_deadline >= _db.head_block_time(), "The escrow ratification deadline has passed. Escrow can no longer be ratified." );

      bool reject_escrow = !o.approve;

      if( o.who == o.to )
      {
         FC_ASSERT( !escrow.to_approved, "Account 'to' (${t}) has already approved the escrow.", ("t", o.to) );

         if( !reject_escrow )
         {
            _db.modify( escrow, [&]( escrow_object& esc )
            {
               esc.to_approved = true;
            });
         }
      }
      if( o.who == o.agent )
      {
         FC_ASSERT( !escrow.agent_approved, "Account 'agent' (${a}) has already approved the escrow.", ("a", o.agent) );

         if( !reject_escrow )
         {
            _db.modify( escrow, [&]( escrow_object& esc )
            {
               esc.agent_approved = true;
            });
         }
      }

      if( reject_escrow )
      {
         const auto& from_account = _db.get_account( o.from );
         _db.adjust_balance( from_account, escrow.ECObalance );
         _db.adjust_balance( from_account, escrow.EUSDbalance );
         _db.adjust_balance( from_account, escrow.pending_fee );

         _db.remove( escrow );
      }
      else if( escrow.to_approved && escrow.agent_approved )
      {
         const auto& agent_account = _db.get_account( o.agent );
         _db.adjust_balance( agent_account, escrow.pending_fee );

         _db.modify( escrow, [&]( escrow_object& esc )
         {
            esc.pending_fee.amount = 0;
         });
      }
   }
   FC_CAPTURE_AND_RETHROW( (o) )
}

void escrow_dispute_evaluator::do_apply( const escrow_dispute_operation& o )
{
   try
   {
      _db.get_account( o.from ); // Verify from account exists

      const auto& e = _db.get_escrow( o.from, o.escrow_id );
      FC_ASSERT( _db.head_block_time() < e.escrow_expiration, "Disputing the escrow must happen before expiration." );
      FC_ASSERT( e.to_approved && e.agent_approved, "The escrow must be approved by all parties before a dispute can be raised." );
      FC_ASSERT( !e.disputed, "The escrow is already under dispute." );
      FC_ASSERT( e.to == o.to, "Operation 'to' (${o}) does not match escrow 'to' (${e}).", ("o", o.to)("e", e.to) );
      FC_ASSERT( e.agent == o.agent, "Operation 'agent' (${a}) does not match escrow 'agent' (${e}).", ("o", o.agent)("e", e.agent) );

      _db.modify( e, [&]( escrow_object& esc )
      {
         esc.disputed = true;
      });
   }
   FC_CAPTURE_AND_RETHROW( (o) )
}

void escrow_release_evaluator::do_apply( const escrow_release_operation& o )
{
   try
   {
      _db.get_account(o.from); // Verify from account exists
      const auto& receiver_account = _db.get_account(o.receiver);

      const auto& e = _db.get_escrow( o.from, o.escrow_id );
      FC_ASSERT( e.ECObalance >= o.ECOamount, "Release amount exceeds escrow balance. Amount: ${a}, Balance: ${b}", ("a", o.ECOamount)("b", e.ECObalance) );
      FC_ASSERT( e.EUSDbalance >= o.EUSDamount, "Release amount exceeds escrow balance. Amount: ${a}, Balance: ${b}", ("a", o.EUSDamount)("b", e.EUSDbalance) );
      FC_ASSERT( e.to == o.to, "Operation 'to' (${o}) does not match escrow 'to' (${e}).", ("o", o.to)("e", e.to) );
      FC_ASSERT( e.agent == o.agent, "Operation 'agent' (${a}) does not match escrow 'agent' (${e}).", ("o", o.agent)("e", e.agent) );
      FC_ASSERT( o.receiver == e.from || o.receiver == e.to, "Funds must be released to 'from' (${f}) or 'to' (${t})", ("f", e.from)("t", e.to) );
      FC_ASSERT( e.to_approved && e.agent_approved, "Funds cannot be released prior to escrow approval." );

      // If there is a dispute regardless of expiration, the agent can release funds to either party
      if( e.disputed )
      {
         FC_ASSERT( o.who == e.agent, "Only 'agent' (${a}) can release funds in a disputed escrow.", ("a", e.agent) );
      }
      else
      {
         FC_ASSERT( o.who == e.from || o.who == e.to, "Only 'from' (${f}) and 'to' (${t}) can release funds from a non-disputed escrow", ("f", e.from)("t", e.to) );

         if( e.escrow_expiration > _db.head_block_time() )
         {
            // If there is no dispute and escrow has not expired, either party can release funds to the other.
            if( o.who == e.from )
            {
               FC_ASSERT( o.receiver == e.to, "Only 'from' (${f}) can release funds to 'to' (${t}).", ("f", e.from)("t", e.to) );
            }
            else if( o.who == e.to )
            {
               FC_ASSERT( o.receiver == e.from, "Only 'to' (${t}) can release funds to 'from' (${t}).", ("f", e.from)("t", e.to) );
            }
         }
      }
      // If escrow expires and there is no dispute, either party can release funds to either party.

      _db.adjust_balance( receiver_account, o.ECOamount );
      _db.adjust_balance( receiver_account, o.EUSDamount );

      _db.modify( e, [&]( escrow_object& esc )
      {
         esc.ECObalance -= o.ECOamount;
         esc.EUSDbalance -= o.EUSDamount;
      });

      if( e.ECObalance.amount == 0 && e.EUSDbalance.amount == 0 )
      {
         _db.remove( e );
      }
   }
   FC_CAPTURE_AND_RETHROW( (o) )
}

void transfer_evaluator::do_apply( const transfer_operation& o )
{
   const auto& from_account = _db.get_account(o.from);
   const auto& to_account = _db.get_account(o.to);

   if( from_account.active_challenged )
   {
      _db.modify( from_account, [&]( account_object& a )
      {
         a.active_challenged = false;
         a.last_active_proved = _db.head_block_time();
      });
   }

   FC_ASSERT( _db.get_balance( from_account, o.amount.symbol ) >= o.amount, "Account does not have sufficient funds for transfer." );
   _db.adjust_balance( from_account, -o.amount );
   _db.adjust_balance( to_account, o.amount );
}

void transferECOtoESCORfund_evaluator::do_apply( const transferECOtoESCORfund_operation& o )
{
   const auto& from_account = _db.get_account(o.from);
   const auto& to_account = o.to.size() ? _db.get_account(o.to) : from_account;

   FC_ASSERT( _db.get_balance( from_account, SYMBOL_ECO) >= o.amount, "Account does not have sufficient ECO for transfer." );
   _db.adjust_balance( from_account, -o.amount );
   _db.createECOfundForESCOR( to_account, o.amount );
}

void withdrawESCOR_evaluator::do_apply( const withdrawESCOR_operation& o )
{
   const auto& account = _db.get_account( o.account );

   if( o.ESCOR.amount < 0 )
   {
      // TODO: Update this to a HF 20 check
#ifndef IS_TEST_NET
      if( _db.head_block_num() > 23847548 )
      {
#endif
         FC_ASSERT( false, "Cannot withdraw negative ESCOR. account: ${account}, ESCOR:${ESCOR}",
            ("account", o.account)("ESCOR", o.ESCOR) );
#ifndef IS_TEST_NET
      }
#endif

      // else, no-op
      return;
   }

   FC_ASSERT( account.ESCOR >= asset( 0, SYMBOL_ESCOR ), "Account does not have sufficient Ezira Power for withdraw." );
   FC_ASSERT( account.ESCOR - account.ESCORDelegated >= o.ESCOR, "Account does not have sufficient Ezira Power for withdraw." );

   if( !account.mined && _db.has_hardfork( HARDFORK_0_1 ) )
   {
      const auto& props = _db.get_dynamic_global_properties();
      const witness_schedule_object& wso = _db.get_witness_schedule_object();

      asset min_ESCOR = wso.median_props.account_creation_fee * props.get_ESCOR_price();
      min_ESCOR.amount.value *= 10;

      FC_ASSERT( account.ESCOR > min_ESCOR || ( _db.has_hardfork( HARDFORK_0_16__562 ) && o.ESCOR.amount == 0 ),
                 "Account registered by another account requires 10x account creation fee worth of Ezira Power before it can be powered down." );
   }

   if( o.ESCOR.amount == 0 )
   {
      if( _db.has_hardfork( HARDFORK_0_5__57 ) )
         FC_ASSERT( account.ESCORwithdrawRateInECO.amount  != 0, "This operation would not change the eScore ECO fund withdraw rate." );

      _db.modify( account, [&]( account_object& a ) {
         a.ESCORwithdrawRateInECO = asset( 0, SYMBOL_ESCOR );
         a.nextESCORwithdrawalTime = time_point_sec::maximum();
         a.to_withdraw = 0;
         a.withdrawn = 0;
      });
   }
   else
   {
      int ECO_fund_for_ESCOR_withdraw_intervals = ECO_fund_for_ESCOR_WITHDRAW_INTERVALS_PRE_HF_16;
      if( _db.has_hardfork( HARDFORK_0_16__551 ) )
         ECO_fund_for_ESCOR_withdraw_intervals = ECO_fund_for_ESCOR_WITHDRAW_INTERVALS; /// 13 weeks = 1 quarter of a year

      _db.modify( account, [&]( account_object& a )
      {
         auto new_ESCORwithdrawRateInECO = asset( o.ESCOR.amount / ECO_fund_for_ESCOR_withdraw_intervals, SYMBOL_ESCOR );

         if( new_ESCORwithdrawRateInECO.amount == 0 )
            new_ESCORwithdrawRateInECO.amount = 1;

         if( _db.has_hardfork( HARDFORK_0_5__57 ) )
            FC_ASSERT( account.ESCORwithdrawRateInECO  != new_ESCORwithdrawRateInECO, "This operation would not change the eScore ECO fund withdraw rate." );

         a.ESCORwithdrawRateInECO = new_ESCORwithdrawRateInECO;
         a.nextESCORwithdrawalTime = _db.head_block_time() + fc::seconds(ESCOR_WITHDRAW_INTERVAL_SECONDS);
         a.to_withdraw = o.ESCOR.amount;
         a.withdrawn = 0;
      });
   }
}

void setWithdrawESCORasECOroute_evaluator::do_apply( const setWithdrawESCORasECOroute_operation& o )
{
   try
   {
   const auto& from_account = _db.get_account( o.from_account );
   const auto& to_account = _db.get_account( o.to_account );
   const auto& wd_idx = _db.get_index< withdrawESCOR_route_index >().indices().get< by_withdraw_route >();
   auto itr = wd_idx.find( boost::make_tuple( from_account.id, to_account.id ) );

   if( itr == wd_idx.end() )
   {
      FC_ASSERT( o.percent != 0, "Cannot create a 0% destination." );
      FC_ASSERT( from_account.withdraw_routes < MAX_WITHDRAW_ROUTES, "Account already has the maximum number of routes." );

      _db.create< withdrawESCOR_route_object >( [&]( withdrawESCOR_route_object& wvdo )
      {
         wvdo.from_account = from_account.id;
         wvdo.to_account = to_account.id;
         wvdo.percent = o.percent;
         wvdo.autoESCOR = o.autoESCOR;
      });

      _db.modify( from_account, [&]( account_object& a )
      {
         a.withdraw_routes++;
      });
   }
   else if( o.percent == 0 )
   {
      _db.remove( *itr );

      _db.modify( from_account, [&]( account_object& a )
      {
         a.withdraw_routes--;
      });
   }
   else
   {
      _db.modify( *itr, [&]( withdrawESCOR_route_object& wvdo )
      {
         wvdo.from_account = from_account.id;
         wvdo.to_account = to_account.id;
         wvdo.percent = o.percent;
         wvdo.autoESCOR = o.autoESCOR;
      });
   }

   itr = wd_idx.upper_bound( boost::make_tuple( from_account.id, account_id_type() ) );
   uint16_t total_percent = 0;

   while( itr->from_account == from_account.id && itr != wd_idx.end() )
   {
      total_percent += itr->percent;
      ++itr;
   }

   FC_ASSERT( total_percent <= PERCENT_100, "More than 100% of eScore ECO fund withdrawals allocated to destinations." );
   }
   FC_CAPTURE_AND_RETHROW()
}

void account_witness_proxy_evaluator::do_apply( const account_witness_proxy_operation& o )
{
   const auto& account = _db.get_account( o.account );
   FC_ASSERT( account.proxy != o.proxy, "Proxy must change." );

   FC_ASSERT( account.can_vote, "Account has declined the ability to vote and cannot proxy votes." );

   /// remove all current votes
   std::array<share_type, MAX_PROXY_RECURSION_DEPTH+1> delta;
   delta[0] = -account.ESCOR.amount;
   for( int i = 0; i < MAX_PROXY_RECURSION_DEPTH; ++i )
      delta[i+1] = -account.proxied_ESCORfundECObalance_votes[i];
   _db.adjust_proxied_witness_votes( account, delta );

   if( o.proxy.size() ) {
      const auto& new_proxy = _db.get_account( o.proxy );
      flat_set<account_id_type> proxy_chain( { account.id, new_proxy.id } );
      proxy_chain.reserve( MAX_PROXY_RECURSION_DEPTH + 1 );

      /// check for proxy loops and fail to update the proxy if it would create a loop
      auto cprox = &new_proxy;
      while( cprox->proxy.size() != 0 ) {
         const auto next_proxy = _db.get_account( cprox->proxy );
         FC_ASSERT( proxy_chain.insert( next_proxy.id ).second, "This proxy would create a proxy loop." );
         cprox = &next_proxy;
         FC_ASSERT( proxy_chain.size() <= MAX_PROXY_RECURSION_DEPTH, "Proxy chain is too long." );
      }

      /// clear all individual vote records
      _db.clear_witness_votes( account );

      _db.modify( account, [&]( account_object& a ) {
         a.proxy = o.proxy;
      });

      /// add all new votes
      for( int i = 0; i <= MAX_PROXY_RECURSION_DEPTH; ++i )
         delta[i] = -delta[i];
      _db.adjust_proxied_witness_votes( account, delta );
   } else { /// we are clearing the proxy which means we simply update the account
      _db.modify( account, [&]( account_object& a ) {
          a.proxy = o.proxy;
      });
   }
}


void accountWitnessVote_evaluator::do_apply( const accountWitnessVote_operation& o )
{
   const auto& voter = _db.get_account( o.account );
   FC_ASSERT( voter.proxy.size() == 0, "A proxy is currently set, please clear the proxy before voting for a witness." );

   if( o.approve )
      FC_ASSERT( voter.can_vote, "Account has declined its voting rights." );

   const auto& witness = _db.get_witness( o.witness );

   const auto& by_account_witness_idx = _db.get_index< witness_vote_index >().indices().get< by_account_witness >();
   auto itr = by_account_witness_idx.find( boost::make_tuple( voter.id, witness.id ) );

   if( itr == by_account_witness_idx.end() ) {
      FC_ASSERT( o.approve, "Vote doesn't exist, user must indicate a desire to approve witness." );

      if ( _db.has_hardfork( HARDFORK_0_2 ) )
      {
         FC_ASSERT( voter.witnesses_voted_for < MAX_accountWitnessVoteS, "Account has voted for too many witnesses." ); // TODO: Remove after hardfork 2

         _db.create<witness_vote_object>( [&]( witness_vote_object& v ) {
             v.witness = witness.id;
             v.account = voter.id;
         });

         if( _db.has_hardfork( HARDFORK_0_3 ) ) {
            _db.adjust_witness_vote( witness, voter.witness_vote_weight() );
         }
         else {
            _db.adjust_proxied_witness_votes( voter, voter.witness_vote_weight() );
         }

      } else {

         _db.create<witness_vote_object>( [&]( witness_vote_object& v ) {
             v.witness = witness.id;
             v.account = voter.id;
         });
         _db.modify( witness, [&]( witness_object& w ) {
             w.votes += voter.witness_vote_weight();
         });

      }
      _db.modify( voter, [&]( account_object& a ) {
         a.witnesses_voted_for++;
      });

   } else {
      FC_ASSERT( !o.approve, "Vote currently exists, user must indicate a desire to reject witness." );

      if (  _db.has_hardfork( HARDFORK_0_2 ) ) {
         if( _db.has_hardfork( HARDFORK_0_3 ) )
            _db.adjust_witness_vote( witness, -voter.witness_vote_weight() );
         else
            _db.adjust_proxied_witness_votes( voter, -voter.witness_vote_weight() );
      } else  {
         _db.modify( witness, [&]( witness_object& w ) {
             w.votes -= voter.witness_vote_weight();
         });
      }
      _db.modify( voter, [&]( account_object& a ) {
         a.witnesses_voted_for--;
      });
      _db.remove( *itr );
   }
}

void vote_evaluator::do_apply( const vote_operation& o )
{ try {
   const auto& comment = _db.get_comment( o.author, o.permlink );
   const auto& voter   = _db.get_account( o.voter );

   if( _db.has_hardfork( HARDFORK_0_10 ) )
      FC_ASSERT( !(voter.owner_challenged || voter.active_challenged ), "Operation cannot be processed because the account is currently challenged." );

   FC_ASSERT( voter.can_vote, "Voter has declined their voting rights." );

   if( o.weight > 0 ) FC_ASSERT( comment.allow_votes, "Votes are not allowed on the comment." );

   if( _db.has_hardfork( HARDFORK_0_12__177 ) && _db.calculate_discussion_payout_time( comment ) == fc::time_point_sec::maximum() )
   {
#ifndef CLEAR_VOTES
      const auto& comment_vote_idx = _db.get_index< comment_vote_index >().indices().get< by_comment_voter >();
      auto itr = comment_vote_idx.find( std::make_tuple( comment.id, voter.id ) );

      if( itr == comment_vote_idx.end() )
         _db.create< comment_vote_object >( [&]( comment_vote_object& cvo )
         {
            cvo.voter = voter.id;
            cvo.comment = comment.id;
            cvo.vote_percent = o.weight;
            cvo.last_update = _db.head_block_time();
         });
      else
         _db.modify( *itr, [&]( comment_vote_object& cvo )
         {
            cvo.vote_percent = o.weight;
            cvo.last_update = _db.head_block_time();
         });
#endif
      return;
   }

   const auto& comment_vote_idx = _db.get_index< comment_vote_index >().indices().get< by_comment_voter >();
   auto itr = comment_vote_idx.find( std::make_tuple( comment.id, voter.id ) );

   int64_t elapsed_seconds   = (_db.head_block_time() - voter.last_vote_time).to_seconds();

   if( _db.has_hardfork( HARDFORK_0_11 ) )
      FC_ASSERT( elapsed_seconds >= MIN_VOTE_INTERVAL_SEC, "Can only vote once every 3 seconds." );

   int64_t regenerated_power = (PERCENT_100 * elapsed_seconds) / VOTE_REGENERATION_SECONDS;
   int64_t current_power     = std::min( int64_t(voter.voting_power + regenerated_power), int64_t(PERCENT_100) );
   FC_ASSERT( current_power > 0, "Account currently does not have voting power." );

   int64_t  abs_weight    = abs(o.weight);
   int64_t  used_power    = (current_power * abs_weight) / PERCENT_100;

   const dynamic_global_property_object& dgpo = _db.get_dynamic_global_properties();

   // used_power = (current_power * abs_weight / PERCENT_100) * (reserve / max_vote_denom)
   // The second multiplication is rounded up as of HF 259
   int64_t max_vote_denom = dgpo.vote_power_reserve_rate * VOTE_REGENERATION_SECONDS / (60*60*24);
   FC_ASSERT( max_vote_denom > 0 );

   if( !_db.has_hardfork( HARDFORK_0_14__259 ) )
   {
      FC_ASSERT( max_vote_denom == 200 );   // TODO: Remove this assert
      used_power = (used_power / max_vote_denom)+1;
   }
   else
   {
      used_power = (used_power + max_vote_denom - 1) / max_vote_denom;
   }
   FC_ASSERT( used_power <= current_power, "Account does not have enough power to vote." );

   int64_t abs_ESCORreward    = ((uint128_t(voter.effective_ESCOR().amount.value) * used_power) / (PERCENT_100)).to_uint64();
   if( !_db.has_hardfork( HARDFORK_0_14__259 ) && abs_ESCORreward == 0 ) abs_ESCORreward = 1;

   if( _db.has_hardfork( HARDFORK_0_14__259 ) )
   {
      FC_ASSERT( abs_ESCORreward > VOTE_DUST_THRESHOLD || o.weight == 0, "Voting weight is too small, please accumulate more voting power or ESCOR." );
   }
   else if( _db.has_hardfork( HARDFORK_0_13__248 ) )
   {
      FC_ASSERT( abs_ESCORreward > VOTE_DUST_THRESHOLD || abs_ESCORreward == 1, "Voting weight is too small, please accumulate more voting power or ESCOR." );
   }



   // Lazily delete vote
   if( itr != comment_vote_idx.end() && itr->num_changes == -1 )
   {
      if( _db.has_hardfork( HARDFORK_0_12__177 ) )
         FC_ASSERT( false, "Cannot vote again on a comment after payout." );

      _db.remove( *itr );
      itr = comment_vote_idx.end();
   }

   if( itr == comment_vote_idx.end() )
   {
      FC_ASSERT( o.weight != 0, "Vote weight cannot be 0." );
      /// this is the ESCORreward voting for or against the post
      int64_t ESCORreward        = o.weight < 0 ? -abs_ESCORreward : abs_ESCORreward;

      if( ESCORreward > 0 )
      {
         if( _db.has_hardfork( HARDFORK_0_17__900 ) )
            FC_ASSERT( _db.head_block_time() < comment.cashout_time - UPVOTE_LOCKOUT_HF17, "Cannot increase payout within last twelve hours before payout." );
         else if( _db.has_hardfork( HARDFORK_0_7 ) )
            FC_ASSERT( _db.head_block_time() < _db.calculate_discussion_payout_time( comment ) - UPVOTE_LOCKOUT_HF7, "Cannot increase payout within last minute before payout." );
      }

      //used_power /= (50*7); /// a 100% vote means use .28% of voting power which should force users to spread their votes around over 50+ posts day for a week
      //if( used_power == 0 ) used_power = 1;

      _db.modify( voter, [&]( account_object& a ){
         a.voting_power = current_power - used_power;
         a.last_vote_time = _db.head_block_time();
      });

      /// if the current net_ESCORreward is less than 0, the post is getting 0 rewards so it is not factored into total ESCORreward^2
      fc::uint128_t old_ESCORreward = std::max(comment.net_ESCORreward.value, int64_t(0));
      const auto& root = _db.get( comment.root_comment );
      auto old_root_abs_ESCORreward = root.children_abs_ESCORreward.value;

      fc::uint128_t avg_cashout_sec;

      if( !_db.has_hardfork( HARDFORK_0_17__769 ) )
      {
         fc::uint128_t cur_cashout_time_sec = _db.calculate_discussion_payout_time( comment ).sec_since_epoch();
         fc::uint128_t new_cashout_time_sec;

         if( _db.has_hardfork( HARDFORK_0_12__177 ) && !_db.has_hardfork( HARDFORK_0_13__257)  )
            new_cashout_time_sec = _db.head_block_time().sec_since_epoch() + CASHOUT_WINDOW_SECONDS_PRE_HF17;
         else
            new_cashout_time_sec = _db.head_block_time().sec_since_epoch() + CASHOUT_WINDOW_SECONDS_PRE_HF12;

         avg_cashout_sec = ( cur_cashout_time_sec * old_root_abs_ESCORreward + new_cashout_time_sec * abs_ESCORreward ) / ( old_root_abs_ESCORreward + abs_ESCORreward );
      }

      FC_ASSERT( abs_ESCORreward > 0, "Cannot vote with 0 ESCORreward." );

      auto old_vote_ESCORreward = comment.vote_ESCORreward;

      _db.modify( comment, [&]( comment_object& c ){
         c.net_ESCORreward += ESCORreward;
         c.abs_ESCORreward += abs_ESCORreward;
         if( ESCORreward > 0 )
            c.vote_ESCORreward += ESCORreward;
         if( ESCORreward > 0 )
            c.net_votes++;
         else
            c.net_votes--;
         if( !_db.has_hardfork( HARDFORK_0_6__114 ) && c.net_ESCORreward == -c.abs_ESCORreward) FC_ASSERT( c.net_votes < 0, "Comment has negative net votes?" );
      });

      _db.modify( root, [&]( comment_object& c )
      {
         c.children_abs_ESCORreward += abs_ESCORreward;

         if( !_db.has_hardfork( HARDFORK_0_17__769 ) )
         {
            if( _db.has_hardfork( HARDFORK_0_12__177 ) && c.last_payout > fc::time_point_sec::min() )
               c.cashout_time = c.last_payout + SECOND_CASHOUT_WINDOW;
            else
               c.cashout_time = fc::time_point_sec( std::min( uint32_t( avg_cashout_sec.to_uint64() ), c.max_cashout_time.sec_since_epoch() ) );

            if( c.max_cashout_time == fc::time_point_sec::maximum() )
               c.max_cashout_time = _db.head_block_time() + fc::seconds( MAX_CASHOUT_WINDOW_SECONDS );
         }
      });

      fc::uint128_t new_ESCORreward = std::max( comment.net_ESCORreward.value, int64_t(0));

      /// calculate ESCORreward2 value
      new_ESCORreward = util::evaluate_reward_curve( new_ESCORreward );
      old_ESCORreward = util::evaluate_reward_curve( old_ESCORreward );

      uint64_t max_vote_weight = 0;

      /** this verifies uniqueness of voter
       *
       *  cv.weight / c.total_vote_weight ==> % of ESCORreward increase that is accounted for by the vote
       *
       *  W(R) = B * R / ( R + 2S )
       *  W(R) is bounded above by B. B is fixed at 2^64 - 1, so all weights fit in a 64 bit integer.
       *
       *  The equation for an individual vote is:
       *    W(R_N) - W(R_N-1), which is the delta increase of proportional weight
       *
       *  c.total_vote_weight =
       *    W(R_1) - W(R_0) +
       *    W(R_2) - W(R_1) + ...
       *    W(R_N) - W(R_N-1) = W(R_N) - W(R_0)
       *
       *  Since W(R_0) = 0, c.total_vote_weight is also bounded above by B and will always fit in a 64 bit integer.
       *
      **/
      _db.create<comment_vote_object>( [&]( comment_vote_object& cv ){
         cv.voter   = voter.id;
         cv.comment = comment.id;
         cv.ESCORreward = ESCORreward;
         cv.vote_percent = o.weight;
         cv.last_update = _db.head_block_time();

         bool curationReward_eligible = ESCORreward > 0 && (comment.last_payout == fc::time_point_sec()) && comment.allow_curationRewards;

         if( curationReward_eligible && _db.has_hardfork( HARDFORK_0_17__774 ) )
            curationReward_eligible = _db.get_curationRewards_percent( comment ) > 0;

         if( curationReward_eligible )
         {
            if( comment.created < fc::time_point_sec(HARDFORK_0_6_REVERSE_AUCTION_TIME) ) {
               u512 ESCORreward3(ESCORreward);
               u256 total2( comment.abs_ESCORreward.value );

               if( !_db.has_hardfork( HARDFORK_0_1 ) )
               {
                  ESCORreward3 *= 1000000;
                  total2 *= 1000000;
               }

               ESCORreward3 = ESCORreward3 * ESCORreward3 * ESCORreward3;

               total2 *= total2;
               cv.weight = static_cast<uint64_t>( ESCORreward3 / total2 );
            } else {// cv.weight = W(R_1) - W(R_0)
               const uint128_t two_s = 2 * util::get_content_constant_s();
               if( _db.has_hardfork( HARDFORK_0_17__774 ) )
               {
                  const auto& reward_fund = _db.get_reward_fund( comment );
                  auto curve = !_db.has_hardfork( HARDFORK_0_19__1052 ) && comment.created > HF_19_SQRT_PRE_CALC
                                 ? curve_id::square_root : reward_fund.curationReward_curve;
                  uint64_t old_weight = util::evaluate_reward_curve( old_vote_ESCORreward.value, curve, reward_fund.content_constant ).to_uint64();
                  uint64_t new_weight = util::evaluate_reward_curve( comment.vote_ESCORreward.value, curve, reward_fund.content_constant ).to_uint64();
                  cv.weight = new_weight - old_weight;
               }
               else if ( _db.has_hardfork( HARDFORK_0_1 ) )
               {
                  uint64_t old_weight = ( ( std::numeric_limits< uint64_t >::max() * fc::uint128_t( old_vote_ESCORreward.value ) ) / ( two_s + old_vote_ESCORreward.value ) ).to_uint64();
                  uint64_t new_weight = ( ( std::numeric_limits< uint64_t >::max() * fc::uint128_t( comment.vote_ESCORreward.value ) ) / ( two_s + comment.vote_ESCORreward.value ) ).to_uint64();
                  cv.weight = new_weight - old_weight;
               }
               else
               {
                  uint64_t old_weight = ( ( std::numeric_limits< uint64_t >::max() * fc::uint128_t( 1000000 * old_vote_ESCORreward.value ) ) / ( two_s + ( 1000000 * old_vote_ESCORreward.value ) ) ).to_uint64();
                  uint64_t new_weight = ( ( std::numeric_limits< uint64_t >::max() * fc::uint128_t( 1000000 * comment.vote_ESCORreward.value ) ) / ( two_s + ( 1000000 * comment.vote_ESCORreward.value ) ) ).to_uint64();
                  cv.weight = new_weight - old_weight;
               }
            }

            max_vote_weight = cv.weight;

            if( _db.head_block_time() > fc::time_point_sec(HARDFORK_0_6_REVERSE_AUCTION_TIME) )  /// start enforcing this prior to the hardfork
            {
               /// discount weight by time
               uint128_t w(max_vote_weight);
               uint64_t delta_t = std::min( uint64_t((cv.last_update - comment.created).to_seconds()), uint64_t(REVERSE_AUCTION_WINDOW_SECONDS) );

               w *= delta_t;
               w /= REVERSE_AUCTION_WINDOW_SECONDS;
               cv.weight = w.to_uint64();
            }
         }
         else
         {
            cv.weight = 0;
         }
      });

      if( max_vote_weight ) // Optimization
      {
         _db.modify( comment, [&]( comment_object& c )
         {
            c.total_vote_weight += max_vote_weight;
         });
      }
      if( !_db.has_hardfork( HARDFORK_0_17__774) )
         _db.adjust_ESCORreward2( comment, old_ESCORreward, new_ESCORreward );
   }
   else
   {
      FC_ASSERT( itr->num_changes < MAX_VOTE_CHANGES, "Voter has used the maximum number of vote changes on this comment." );

      if( _db.has_hardfork( HARDFORK_0_6__112 ) )
         FC_ASSERT( itr->vote_percent != o.weight, "You have already voted in a similar way." );

      /// this is the ESCORreward voting for or against the post
      int64_t ESCORreward        = o.weight < 0 ? -abs_ESCORreward : abs_ESCORreward;

      if( itr->ESCORreward < ESCORreward )
      {
         if( _db.has_hardfork( HARDFORK_0_17__900 ) )
            FC_ASSERT( _db.head_block_time() < comment.cashout_time - UPVOTE_LOCKOUT_HF17, "Cannot increase payout within last twelve hours before payout." );
         else if( _db.has_hardfork( HARDFORK_0_7 ) )
            FC_ASSERT( _db.head_block_time() < _db.calculate_discussion_payout_time( comment ) - UPVOTE_LOCKOUT_HF7, "Cannot increase payout within last minute before payout." );
      }

      _db.modify( voter, [&]( account_object& a ){
         a.voting_power = current_power - used_power;
         a.last_vote_time = _db.head_block_time();
      });

      /// if the current net_ESCORreward is less than 0, the post is getting 0 rewards so it is not factored into total ESCORreward^2
      fc::uint128_t old_ESCORreward = std::max(comment.net_ESCORreward.value, int64_t(0));
      const auto& root = _db.get( comment.root_comment );
      auto old_root_abs_ESCORreward = root.children_abs_ESCORreward.value;

      fc::uint128_t avg_cashout_sec;

      if( !_db.has_hardfork( HARDFORK_0_17__769 ) )
      {
         fc::uint128_t cur_cashout_time_sec = _db.calculate_discussion_payout_time( comment ).sec_since_epoch();
         fc::uint128_t new_cashout_time_sec;

         if( _db.has_hardfork( HARDFORK_0_12__177 ) && ! _db.has_hardfork( HARDFORK_0_13__257 )  )
            new_cashout_time_sec = _db.head_block_time().sec_since_epoch() + CASHOUT_WINDOW_SECONDS_PRE_HF17;
         else
            new_cashout_time_sec = _db.head_block_time().sec_since_epoch() + CASHOUT_WINDOW_SECONDS_PRE_HF12;

         if( _db.has_hardfork( HARDFORK_0_14__259 ) && abs_ESCORreward == 0 )
            avg_cashout_sec = cur_cashout_time_sec;
         else
            avg_cashout_sec = ( cur_cashout_time_sec * old_root_abs_ESCORreward + new_cashout_time_sec * abs_ESCORreward ) / ( old_root_abs_ESCORreward + abs_ESCORreward );
      }

      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_ESCORreward -= itr->ESCORreward;
         c.net_ESCORreward += ESCORreward;
         c.abs_ESCORreward += abs_ESCORreward;

         /// TODO: figure out how to handle remove a vote (ESCORreward == 0 )
         if( ESCORreward > 0 && itr->ESCORreward < 0 )
            c.net_votes += 2;
         else if( ESCORreward > 0 && itr->ESCORreward == 0 )
            c.net_votes += 1;
         else if( ESCORreward == 0 && itr->ESCORreward < 0 )
            c.net_votes += 1;
         else if( ESCORreward == 0 && itr->ESCORreward > 0 )
            c.net_votes -= 1;
         else if( ESCORreward < 0 && itr->ESCORreward == 0 )
            c.net_votes -= 1;
         else if( ESCORreward < 0 && itr->ESCORreward > 0 )
            c.net_votes -= 2;
      });

      _db.modify( root, [&]( comment_object& c )
      {
         c.children_abs_ESCORreward += abs_ESCORreward;

         if( !_db.has_hardfork( HARDFORK_0_17__769 ) )
         {
            if( _db.has_hardfork( HARDFORK_0_12__177 ) && c.last_payout > fc::time_point_sec::min() )
               c.cashout_time = c.last_payout + SECOND_CASHOUT_WINDOW;
            else
               c.cashout_time = fc::time_point_sec( std::min( uint32_t( avg_cashout_sec.to_uint64() ), c.max_cashout_time.sec_since_epoch() ) );

            if( c.max_cashout_time == fc::time_point_sec::maximum() )
               c.max_cashout_time = _db.head_block_time() + fc::seconds( MAX_CASHOUT_WINDOW_SECONDS );
         }
      });

      fc::uint128_t new_ESCORreward = std::max( comment.net_ESCORreward.value, int64_t(0));

      /// calculate ESCORreward2 value
      new_ESCORreward = util::evaluate_reward_curve( new_ESCORreward );
      old_ESCORreward = util::evaluate_reward_curve( old_ESCORreward );


      _db.modify( comment, [&]( comment_object& c )
      {
         c.total_vote_weight -= itr->weight;
      });

      _db.modify( *itr, [&]( comment_vote_object& cv )
      {
         cv.ESCORreward = ESCORreward;
         cv.vote_percent = o.weight;
         cv.last_update = _db.head_block_time();
         cv.weight = 0;
         cv.num_changes += 1;
      });

      if( !_db.has_hardfork( HARDFORK_0_17__774) )
         _db.adjust_ESCORreward2( comment, old_ESCORreward, new_ESCORreward );
   }

} FC_CAPTURE_AND_RETHROW( (o)) }

void custom_evaluator::do_apply( const custom_operation& o )
{
   database& d = db();
   if( d.is_producing() )
      FC_ASSERT( o.data.size() <= 8192, "custom_operation data must be less than 8k" );
}

void customJson_evaluator::do_apply( const customJson_operation& o )
{
   database& d = db();
   if( d.is_producing() )
      FC_ASSERT( o.json.length() <= 8192, "customJson_operation json must be less than 8k" );

   std::shared_ptr< custom_operation_interpreter > eval = d.get_customJson_evaluator( o.id );
   if( !eval )
      return;

   try
   {
      eval->apply( o );
   }
   catch( const fc::exception& e )
   {
      if( d.is_producing() )
         throw e;
   }
   catch(...)
   {
      elog( "Unexpected exception applying custom json evaluator." );
   }
}


void custom_binary_evaluator::do_apply( const custom_binary_operation& o )
{
   database& d = db();
   if( d.is_producing() )
   {
      FC_ASSERT( false, "custom_binary_operation is deprecated" );
      FC_ASSERT( o.data.size() <= 8192, "custom_binary_operation data must be less than 8k" );
   }
   FC_ASSERT( d.has_hardfork( HARDFORK_0_14__317 ) );

   std::shared_ptr< custom_operation_interpreter > eval = d.get_customJson_evaluator( o.id );
   if( !eval )
      return;

   try
   {
      eval->apply( o );
   }
   catch( const fc::exception& e )
   {
      if( d.is_producing() )
         throw e;
   }
   catch(...)
   {
      elog( "Unexpected exception applying custom json evaluator." );
   }
}


template<typename Operation>
void pow_apply( database& db, Operation o )
{
   const auto& dgp = db.get_dynamic_global_properties();

   if( db.has_hardfork( HARDFORK_0_5__59 ) )
   {
      const auto& witness_by_work = db.get_index<witness_index>().indices().get<by_work>();
      auto work_itr = witness_by_work.find( o.work.work );
      if( work_itr != witness_by_work.end() )
      {
          FC_ASSERT( !"DUPLICATE WORK DISCOVERED", "${w}  ${witness}",("w",o)("wit",*work_itr) );
      }
   }

   const auto& accounts_by_name = db.get_index<account_index>().indices().get<by_name>();

   auto itr = accounts_by_name.find(o.get_worker_account());
   if(itr == accounts_by_name.end())
   {
      db.create< account_object >( [&]( account_object& acc )
      {
         acc.name = o.get_worker_account();
         acc.memoKey = o.work.worker;
         acc.created = dgp.time;
         acc.last_vote_time = dgp.time;

         if( !db.has_hardfork( HARDFORK_0_11__169 ) )
            acc.recoveryAccount = "ezira";
         else
            acc.recoveryAccount = ""; /// highest voted witness at time of recovery
      });

      db.create< account_authority_object >( [&]( account_authority_object& auth )
      {
         auth.account = o.get_worker_account();
         auth.owner = authority( 1, o.work.worker, 1);
         auth.active = auth.owner;
         auth.posting = auth.owner;
      });
   }

   const auto& worker_account = db.get_account( o.get_worker_account() ); // verify it exists
   const auto& worker_auth = db.get< account_authority_object, by_account >( o.get_worker_account() );
   FC_ASSERT( worker_auth.active.num_auths() == 1, "Miners can only have one key authority. ${a}", ("a",worker_auth.active) );
   FC_ASSERT( worker_auth.active.key_auths.size() == 1, "Miners may only have one key authority." );
   FC_ASSERT( worker_auth.active.key_auths.begin()->first == o.work.worker, "Work must be performed by key that signed the work." );
   FC_ASSERT( o.block_id == db.head_block_id(), "pow not for last block" );
   if( db.has_hardfork( HARDFORK_0_13__256 ) )
      FC_ASSERT( worker_account.last_accountUpdate < db.head_block_time(), "Worker account must not have updated their account this block." );

   fc::sha256 target = db.get_pow_target();

   FC_ASSERT( o.work.work < target, "Work lacks sufficient difficulty." );

   db.modify( dgp, [&]( dynamic_global_property_object& p )
   {
      p.total_pow++; // make sure this doesn't break anything...
      p.num_pow_witnesses++;
   });


   const witness_object* cur_witness = db.find_witness( worker_account.name );
   if( cur_witness ) {
      FC_ASSERT( cur_witness->pow_worker == 0, "This account is already scheduled for pow block production." );
      db.modify(*cur_witness, [&]( witness_object& w ){
          w.props             = o.props;
          w.pow_worker        = dgp.total_pow;
          w.last_work         = o.work.work;
      });
   } else {
      db.create<witness_object>( [&]( witness_object& w )
      {
          w.owner             = o.get_worker_account();
          w.props             = o.props;
          w.signing_key       = o.work.worker;
          w.pow_worker        = dgp.total_pow;
          w.last_work         = o.work.work;
      });
   }
   /// POW reward depends upon whether we are before or after MINER_VOTING kicks in
   asset pow_reward = db.get_pow_reward();
   if( db.head_block_num() < START_MINER_VOTING_BLOCK )
      pow_reward.amount *= MAX_WITNESSES;
   db.adjust_supply( pow_reward, true );

   /// pay the witness that includes this POW
   const auto& inc_witness = db.get_account( dgp.current_witness );
   if( db.head_block_num() < START_MINER_VOTING_BLOCK )
      db.adjust_balance( inc_witness, pow_reward );
   else
      db.createECOfundForESCOR( inc_witness, pow_reward );
}

void pow_evaluator::do_apply( const pow_operation& o ) {
   FC_ASSERT( !db().has_hardfork( HARDFORK_0_13__256 ), "pow is deprecated. Use pow2 instead" );
   pow_apply( db(), o );
}


void pow2_evaluator::do_apply( const pow2_operation& o )
{
   database& db = this->db();
   FC_ASSERT( !db.has_hardfork( HARDFORK_0_17__770 ), "mining is now disabled" );

   const auto& dgp = db.get_dynamic_global_properties();
   uint32_t target_pow = db.get_pow_summary_target();
   account_name_type worker_account;

   if( db.has_hardfork( HARDFORK_0_16__551 ) )
   {
      const auto& work = o.work.get< equihash_pow >();
      FC_ASSERT( work.prev_block == db.head_block_id(), "Equihash pow op not for last block" );
      auto recent_block_num = protocol::block_header::num_from_id( work.input.prev_block );
      FC_ASSERT( recent_block_num > dgp.last_irreversible_block_num,
         "Equihash pow done for block older than last irreversible block num" );
      FC_ASSERT( work.pow_summary < target_pow, "Insufficient work difficulty. Work: ${w}, Target: ${t}", ("w",work.pow_summary)("t", target_pow) );
      worker_account = work.input.worker_account;
   }
   else
   {
      const auto& work = o.work.get< pow2 >();
      FC_ASSERT( work.input.prev_block == db.head_block_id(), "Work not for last block" );
      FC_ASSERT( work.pow_summary < target_pow, "Insufficient work difficulty. Work: ${w}, Target: ${t}", ("w",work.pow_summary)("t", target_pow) );
      worker_account = work.input.worker_account;
   }

   FC_ASSERT( o.props.maximum_block_size >= MIN_BLOCK_SIZE_LIMIT * 2, "Voted maximum block size is too small." );

   db.modify( dgp, [&]( dynamic_global_property_object& p )
   {
      p.total_pow++;
      p.num_pow_witnesses++;
   });

   const auto& accounts_by_name = db.get_index<account_index>().indices().get<by_name>();
   auto itr = accounts_by_name.find( worker_account );
   if(itr == accounts_by_name.end())
   {
      FC_ASSERT( o.new_owner_key.valid(), "New owner key is not valid." );
      db.create< account_object >( [&]( account_object& acc )
      {
         acc.name = worker_account;
         acc.memoKey = *o.new_owner_key;
         acc.created = dgp.time;
         acc.last_vote_time = dgp.time;
         acc.recoveryAccount = ""; /// highest voted witness at time of recovery
      });

      db.create< account_authority_object >( [&]( account_authority_object& auth )
      {
         auth.account = worker_account;
         auth.owner = authority( 1, *o.new_owner_key, 1);
         auth.active = auth.owner;
         auth.posting = auth.owner;
      });

      db.create<witness_object>( [&]( witness_object& w )
      {
          w.owner             = worker_account;
          w.props             = o.props;
          w.signing_key       = *o.new_owner_key;
          w.pow_worker        = dgp.total_pow;
      });
   }
   else
   {
      FC_ASSERT( !o.new_owner_key.valid(), "Cannot specify an owner key unless creating account." );
      const witness_object* cur_witness = db.find_witness( worker_account );
      FC_ASSERT( cur_witness, "Witness must be created for existing account before mining.");
      FC_ASSERT( cur_witness->pow_worker == 0, "This account is already scheduled for pow block production." );
      db.modify(*cur_witness, [&]( witness_object& w )
      {
          w.props             = o.props;
          w.pow_worker        = dgp.total_pow;
      });
   }

   if( !db.has_hardfork( HARDFORK_0_16__551) )
   {
      /// pay the witness that includes this POW
      asset inc_reward = db.get_pow_reward();
      db.adjust_supply( inc_reward, true );

      const auto& inc_witness = db.get_account( dgp.current_witness );
      db.createECOfundForESCOR( inc_witness, inc_reward );
   }
}

void feed_publish_evaluator::do_apply( const feed_publish_operation& o )
{
  const auto& witness = _db.get_witness( o.publisher );
  _db.modify( witness, [&]( witness_object& w ){
      w.EUSD_exchange_rate = o.exchange_rate;
      w.last_EUSD_exchange_update = _db.head_block_time();
  });
}

void convert_evaluator::do_apply( const convert_operation& o )
{
  const auto& owner = _db.get_account( o.owner );
  FC_ASSERT( _db.get_balance( owner, o.amount.symbol ) >= o.amount, "Account does not have sufficient balance for conversion." );

  _db.adjust_balance( owner, -o.amount );

  const auto& fhistory = _db.get_feed_history();
  FC_ASSERT( !fhistory.current_median_history.is_null(), "Cannot convert EUSD because there is no price feed." );

  auto ECO_conversion_delay = CONVERSION_DELAY_PRE_HF_16;
  if( _db.has_hardfork( HARDFORK_0_16__551) )
     ECO_conversion_delay = CONVERSION_DELAY;

  _db.create<convert_request_object>( [&]( convert_request_object& obj )
  {
      obj.owner           = o.owner;
      obj.requestid       = o.requestid;
      obj.amount          = o.amount;
      obj.conversion_date = _db.head_block_time() + ECO_conversion_delay;
  });

}

void limit_order_create_evaluator::do_apply( const limit_order_create_operation& o )
{
   FC_ASSERT( o.expiration > _db.head_block_time(), "Limit order has to expire after head block time." );

   const auto& owner = _db.get_account( o.owner );

   FC_ASSERT( _db.get_balance( owner, o.amount_to_sell.symbol ) >= o.amount_to_sell, "Account does not have sufficient funds for limit order." );

   _db.adjust_balance( owner, -o.amount_to_sell );

   const auto& order = _db.create<limit_order_object>( [&]( limit_order_object& obj )
   {
       obj.created    = _db.head_block_time();
       obj.seller     = o.owner;
       obj.orderid    = o.orderid;
       obj.for_sale   = o.amount_to_sell.amount;
       obj.sell_price = o.get_price();
       obj.expiration = o.expiration;
   });

   bool filled = _db.apply_order( order );

   if( o.fill_or_kill ) FC_ASSERT( filled, "Cancelling order because it was not filled." );
}

void limit_order_create2_evaluator::do_apply( const limit_order_create2_operation& o )
{
   FC_ASSERT( o.expiration > _db.head_block_time(), "Limit order has to expire after head block time." );

   const auto& owner = _db.get_account( o.owner );

   FC_ASSERT( _db.get_balance( owner, o.amount_to_sell.symbol ) >= o.amount_to_sell, "Account does not have sufficient funds for limit order." );

   _db.adjust_balance( owner, -o.amount_to_sell );

   const auto& order = _db.create<limit_order_object>( [&]( limit_order_object& obj )
   {
       obj.created    = _db.head_block_time();
       obj.seller     = o.owner;
       obj.orderid    = o.orderid;
       obj.for_sale   = o.amount_to_sell.amount;
       obj.sell_price = o.exchange_rate;
       obj.expiration = o.expiration;
   });

   bool filled = _db.apply_order( order );

   if( o.fill_or_kill ) FC_ASSERT( filled, "Cancelling order because it was not filled." );
}

void limit_order_cancel_evaluator::do_apply( const limit_order_cancel_operation& o )
{
   _db.cancel_order( _db.get_limit_order( o.owner, o.orderid ) );
}

void report_over_production_evaluator::do_apply( const report_over_production_operation& o )
{
   FC_ASSERT( !_db.has_hardfork( HARDFORK_0_4 ), "report_over_production_operation is disabled." );
}

void challenge_authority_evaluator::do_apply( const challenge_authority_operation& o )
{
   if( _db.has_hardfork( HARDFORK_0_14__307 ) ) FC_ASSERT( false, "Challenge authority operation is currently disabled." );
   const auto& challenged = _db.get_account( o.challenged );
   const auto& challenger = _db.get_account( o.challenger );

   if( o.require_owner )
   {
      FC_ASSERT( challenged.reset_account == o.challenger, "Owner authority can only be challenged by its reset account." );
      FC_ASSERT( challenger.balance >= OWNER_CHALLENGE_FEE );
      FC_ASSERT( !challenged.owner_challenged );
      FC_ASSERT( _db.head_block_time() - challenged.last_owner_proved > OWNER_CHALLENGE_COOLDOWN );

      _db.adjust_balance( challenger, - OWNER_CHALLENGE_FEE );
      _db.createECOfundForESCOR( _db.get_account( o.challenged ), OWNER_CHALLENGE_FEE );

      _db.modify( challenged, [&]( account_object& a )
      {
         a.owner_challenged = true;
      });
  }
  else
  {
      FC_ASSERT( challenger.balance >= ACTIVE_CHALLENGE_FEE, "Account does not have sufficient funds to pay challenge fee." );
      FC_ASSERT( !( challenged.owner_challenged || challenged.active_challenged ), "Account is already challenged." );
      FC_ASSERT( _db.head_block_time() - challenged.last_active_proved > ACTIVE_CHALLENGE_COOLDOWN, "Account cannot be challenged because it was recently challenged." );

      _db.adjust_balance( challenger, - ACTIVE_CHALLENGE_FEE );
      _db.createECOfundForESCOR( _db.get_account( o.challenged ), ACTIVE_CHALLENGE_FEE );

      _db.modify( challenged, [&]( account_object& a )
      {
         a.active_challenged = true;
      });
  }
}

void prove_authority_evaluator::do_apply( const prove_authority_operation& o )
{
   const auto& challenged = _db.get_account( o.challenged );
   FC_ASSERT( challenged.owner_challenged || challenged.active_challenged, "Account is not challeneged. No need to prove authority." );

   _db.modify( challenged, [&]( account_object& a )
   {
      a.active_challenged = false;
      a.last_active_proved = _db.head_block_time();
      if( o.require_owner )
      {
         a.owner_challenged = false;
         a.last_owner_proved = _db.head_block_time();
      }
   });
}

void request_account_recovery_evaluator::do_apply( const request_account_recovery_operation& o )
{
   const auto& accountToRecover = _db.get_account( o.accountToRecover );

   if ( accountToRecover.recoveryAccount.length() )   // Make sure recovery matches expected recovery account
      FC_ASSERT( accountToRecover.recoveryAccount == o.recoveryAccount, "Cannot recover an account that does not have you as there recovery partner." );
   else                                                  // Empty string recovery account defaults to top witness
      FC_ASSERT( _db.get_index< witness_index >().indices().get< by_vote_name >().begin()->owner == o.recoveryAccount, "Top witness must recover an account with no recovery partner." );

   const auto& recovery_request_idx = _db.get_index< account_recovery_request_index >().indices().get< by_account >();
   auto request = recovery_request_idx.find( o.accountToRecover );

   if( request == recovery_request_idx.end() ) // New Request
   {
      FC_ASSERT( !o.new_owner_authority.is_impossible(), "Cannot recover using an impossible authority." );
      FC_ASSERT( o.new_owner_authority.weight_threshold, "Cannot recover using an open authority." );

      // Check accounts in the new authority exist
      if( ( _db.has_hardfork( HARDFORK_0_15__465 ) ) )
      {
         for( auto& a : o.new_owner_authority.account_auths )
         {
            _db.get_account( a.first );
         }
      }

      _db.create< account_recovery_request_object >( [&]( account_recovery_request_object& req )
      {
         req.accountToRecover = o.accountToRecover;
         req.new_owner_authority = o.new_owner_authority;
         req.expires = _db.head_block_time() + ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD;
      });
   }
   else if( o.new_owner_authority.weight_threshold == 0 ) // Cancel Request if authority is open
   {
      _db.remove( *request );
   }
   else // Change Request
   {
      FC_ASSERT( !o.new_owner_authority.is_impossible(), "Cannot recover using an impossible authority." );

      // Check accounts in the new authority exist
      if( ( _db.has_hardfork( HARDFORK_0_15__465 ) ) )
      {
         for( auto& a : o.new_owner_authority.account_auths )
         {
            _db.get_account( a.first );
         }
      }

      _db.modify( *request, [&]( account_recovery_request_object& req )
      {
         req.new_owner_authority = o.new_owner_authority;
         req.expires = _db.head_block_time() + ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD;
      });
   }
}

void recover_account_evaluator::do_apply( const recover_account_operation& o )
{
   const auto& account = _db.get_account( o.accountToRecover );

   if( _db.has_hardfork( HARDFORK_0_12 ) )
      FC_ASSERT( _db.head_block_time() - account.last_account_recovery > OWNER_UPDATE_LIMIT, "Owner authority can only be updated once an hour." );

   const auto& recovery_request_idx = _db.get_index< account_recovery_request_index >().indices().get< by_account >();
   auto request = recovery_request_idx.find( o.accountToRecover );

   FC_ASSERT( request != recovery_request_idx.end(), "There are no active recovery requests for this account." );
   FC_ASSERT( request->new_owner_authority == o.new_owner_authority, "New owner authority does not match recovery request." );

   const auto& recent_auth_idx = _db.get_index< owner_authority_history_index >().indices().get< by_account >();
   auto hist = recent_auth_idx.lower_bound( o.accountToRecover );
   bool found = false;

   while( hist != recent_auth_idx.end() && hist->account == o.accountToRecover && !found )
   {
      found = hist->previous_owner_authority == o.recent_owner_authority;
      if( found ) break;
      ++hist;
   }

   FC_ASSERT( found, "Recent authority not found in authority history." );

   _db.remove( *request ); // Remove first, update_owner_authority may invalidate iterator
   _db.update_owner_authority( account, o.new_owner_authority );
   _db.modify( account, [&]( account_object& a )
   {
      a.last_account_recovery = _db.head_block_time();
   });
}

void change_recoveryAccount_evaluator::do_apply( const change_recoveryAccount_operation& o )
{
   _db.get_account( o.new_recoveryAccount ); // Simply validate account exists
   const auto& accountToRecover = _db.get_account( o.accountToRecover );

   const auto& change_recovery_idx = _db.get_index< change_recoveryAccount_request_index >().indices().get< by_account >();
   auto request = change_recovery_idx.find( o.accountToRecover );

   if( request == change_recovery_idx.end() ) // New request
   {
      _db.create< change_recoveryAccount_request_object >( [&]( change_recoveryAccount_request_object& req )
      {
         req.accountToRecover = o.accountToRecover;
         req.recoveryAccount = o.new_recoveryAccount;
         req.effective_on = _db.head_block_time() + OWNER_AUTH_RECOVERY_PERIOD;
      });
   }
   else if( accountToRecover.recoveryAccount != o.new_recoveryAccount ) // Change existing request
   {
      _db.modify( *request, [&]( change_recoveryAccount_request_object& req )
      {
         req.recoveryAccount = o.new_recoveryAccount;
         req.effective_on = _db.head_block_time() + OWNER_AUTH_RECOVERY_PERIOD;
      });
   }
   else // Request exists and changing back to current recovery account
   {
      _db.remove( *request );
   }
}

void transferToSavings_evaluator::do_apply( const transferToSavings_operation& op )
{
   const auto& from = _db.get_account( op.from );
   const auto& to   = _db.get_account(op.to);
   FC_ASSERT( _db.get_balance( from, op.amount.symbol ) >= op.amount, "Account does not have sufficient funds to transfer to savings." );

   _db.adjust_balance( from, -op.amount );
   _db.adjust_ECOsavingsBalance( to, op.amount );
}

void transferFromSavings_evaluator::do_apply( const transferFromSavings_operation& op )
{
   const auto& from = _db.get_account( op.from );
   _db.get_account(op.to); // Verify to account exists

   FC_ASSERT( from.savings_withdraw_requests < SAVINGS_WITHDRAW_REQUEST_LIMIT, "Account has reached limit for pending withdraw requests." );

   FC_ASSERT( _db.get_ECOsavingsBalance( from, op.amount.symbol ) >= op.amount );
   _db.adjust_ECOsavingsBalance( from, -op.amount );
   _db.create<savings_withdraw_object>( [&]( savings_withdraw_object& s ) {
      s.from   = op.from;
      s.to     = op.to;
      s.amount = op.amount;
#ifndef IS_LOW_MEM
      from_string( s.memo, op.memo );
#endif
      s.request_id = op.request_id;
      s.complete = _db.head_block_time() + SAVINGS_WITHDRAW_TIME;
   });

   _db.modify( from, [&]( account_object& a )
   {
      a.savings_withdraw_requests++;
   });
}

void cancelTransferFromSavings_evaluator::do_apply( const cancelTransferFromSavings_operation& op )
{
   const auto& swo = _db.get_savings_withdraw( op.from, op.request_id );
   _db.adjust_ECOsavingsBalance( _db.get_account( swo.from ), swo.amount );
   _db.remove( swo );

   const auto& from = _db.get_account( op.from );
   _db.modify( from, [&]( account_object& a )
   {
      a.savings_withdraw_requests--;
   });
}

void decline_voting_rights_evaluator::do_apply( const decline_voting_rights_operation& o )
{
   FC_ASSERT( _db.has_hardfork( HARDFORK_0_14__324 ) );

   const auto& account = _db.get_account( o.account );
   const auto& request_idx = _db.get_index< decline_voting_rights_request_index >().indices().get< by_account >();
   auto itr = request_idx.find( account.id );

   if( o.decline )
   {
      FC_ASSERT( itr == request_idx.end(), "Cannot create new request because one already exists." );

      _db.create< decline_voting_rights_request_object >( [&]( decline_voting_rights_request_object& req )
      {
         req.account = account.id;
         req.effective_date = _db.head_block_time() + OWNER_AUTH_RECOVERY_PERIOD;
      });
   }
   else
   {
      FC_ASSERT( itr != request_idx.end(), "Cannot cancel the request because it does not exist." );
      _db.remove( *itr );
   }
}

void reset_account_evaluator::do_apply( const reset_account_operation& op )
{
   FC_ASSERT( false, "Reset Account Operation is currently disabled." );
/*
   const auto& acnt = _db.get_account( op.account_to_reset );
   auto band = _db.find< account_bandwidth_object, by_account_bandwidth_type >( boost::make_tuple( op.account_to_reset, bandwidth_type::old_forum ) );
   if( band != nullptr )
      FC_ASSERT( ( _db.head_block_time() - band->last_bandwidth_update ) > fc::days(60), "Account must be inactive for 60 days to be eligible for reset" );
   FC_ASSERT( acnt.reset_account == op.reset_account, "Reset account does not match reset account on account." );

   _db.update_owner_authority( acnt, op.new_owner_authority );
*/
}

void set_reset_account_evaluator::do_apply( const set_reset_account_operation& op )
{
   FC_ASSERT( false, "Set Reset Account Operation is currently disabled." );
/*
   const auto& acnt = _db.get_account( op.account );
   _db.get_account( op.reset_account );

   FC_ASSERT( acnt.reset_account == op.current_reset_account, "Current reset account does not match reset account on account." );
   FC_ASSERT( acnt.reset_account != op.reset_account, "Reset account must change" );

   _db.modify( acnt, [&]( account_object& a )
   {
       a.reset_account = op.reset_account;
   });
*/
}

void claimRewardBalance_evaluator::do_apply( const claimRewardBalance_operation& op )
{
   const auto& acnt = _db.get_account( op.account );

   FC_ASSERT( op.ECOreward <= acnt.ECOrewardBalance, "Cannot claim that much ECO. Claim: ${c} Actual: ${a}",
      ("c", op.ECOreward)("a", acnt.ECOrewardBalance) );
   FC_ASSERT( op.EUSDreward <= acnt.EUSDrewardbalance, "Cannot claim that much EUSD. Claim: ${c} Actual: ${a}",
      ("c", op.EUSDreward)("a", acnt.EUSDrewardbalance) );
   FC_ASSERT( op.ESCORreward <= acnt.ESCORrewardBalance, "Cannot claim that much ESCOR. Claim: ${c} Actual: ${a}",
      ("c", op.ESCORreward)("a", acnt.ESCORrewardBalance) );

   asset ESCORrewardBalance_to_move = asset( 0, SYMBOL_ECO );
   if( op.ESCORreward == acnt.ESCORrewardBalance )
      ESCORrewardBalance_to_move = acnt.ESCORrewardBalance;
   else
      ESCORrewardBalance_to_move = asset( ( ( uint128_t( op.ESCORreward.amount.value ) * uint128_t( acnt.ESCORrewardBalance.amount.value ) )
         / uint128_t( acnt.ESCORrewardBalance.amount.value ) ).to_uint64(), SYMBOL_ECO );

   _db.adjust_reward_balance( acnt, -op.ECOreward );
   _db.adjust_reward_balance( acnt, -op.EUSDreward );
   _db.adjust_balance( acnt, op.ECOreward );
   _db.adjust_balance( acnt, op.EUSDreward );

   _db.modify( acnt, [&]( account_object& a )
   {
      a.ESCOR += op.ESCORreward;
      a.ESCORrewardBalance -= op.ESCORreward;
      a.ESCORrewardBalance -= ESCORrewardBalance_to_move;
   });

   _db.modify( _db.get_dynamic_global_properties(), [&]( dynamic_global_property_object& gpo )
   {
      gpo.totalESCOR += op.ESCORreward;
      gpo.totalECOfundForESCOR += ESCORrewardBalance_to_move;

      gpo.pending_rewarded_ESCOR -= op.ESCORreward;
      gpo.pending_rewarded_ESCORvalueInECO -= ESCORrewardBalance_to_move;
   });

   _db.adjust_proxied_witness_votes( acnt, op.ESCORreward.amount );
}

void delegateESCOR_evaluator::do_apply( const delegateESCOR_operation& op )
{
   const auto& delegator = _db.get_account( op.delegator );
   const auto& delegatee = _db.get_account( op.delegatee );
   auto delegation = _db.find< ECO_fund_for_ESCOR_delegation_object, by_delegation >( boost::make_tuple( op.delegator, op.delegatee ) );

   auto available_ESCOR = delegator.ESCOR - delegator.ESCORDelegated - asset( delegator.to_withdraw - delegator.withdrawn, SYMBOL_ESCOR );

   const auto& wso = _db.get_witness_schedule_object();
   const auto& gpo = _db.get_dynamic_global_properties();
   auto min_delegation = asset( wso.median_props.account_creation_fee.amount * 10, SYMBOL_ECO ) * gpo.get_ESCOR_price();
   auto min_update = wso.median_props.account_creation_fee * gpo.get_ESCOR_price();

   // If delegation doesn't exist, create it
   if( delegation == nullptr )
   {
      FC_ASSERT( available_ESCOR >= op.ESCOR, "Account does not have enough eScore to delegate." );
      FC_ASSERT( op.ESCOR >= min_delegation, "Account must delegate a minimum of ${v}", ("v", min_delegation) );

      _db.create< ECO_fund_for_ESCOR_delegation_object >( [&]( ECO_fund_for_ESCOR_delegation_object& obj )
      {
         obj.delegator = op.delegator;
         obj.delegatee = op.delegatee;
         obj.ESCOR = op.ESCOR;
         obj.min_delegation_time = _db.head_block_time();
      });

      _db.modify( delegator, [&]( account_object& a )
      {
         a.ESCORDelegated += op.ESCOR;
      });

      _db.modify( delegatee, [&]( account_object& a )
      {
         a.ESCORreceived += op.ESCOR;
      });
   }
   // Else if the delegation is increasing
   else if( op.ESCOR >= delegation->ESCOR )
   {
      auto delta = op.ESCOR - delegation->ESCOR;

      FC_ASSERT( delta >= min_update, "Ezira Power increase is not enough of a difference. min_update: ${min}", ("min", min_update) );
      FC_ASSERT( available_ESCOR >= op.ESCOR - delegation->ESCOR, "Account does not have enough eScore to delegate." );

      _db.modify( delegator, [&]( account_object& a )
      {
         a.ESCORDelegated += delta;
      });

      _db.modify( delegatee, [&]( account_object& a )
      {
         a.ESCORreceived += delta;
      });

      _db.modify( *delegation, [&]( ECO_fund_for_ESCOR_delegation_object& obj )
      {
         obj.ESCOR = op.ESCOR;
      });
   }
   // Else the delegation is decreasing
   else /* delegation->ESCOR > op.ESCOR */
   {
      auto delta = delegation->ESCOR - op.ESCOR;

      if( op.ESCOR.amount > 0 )
      {
         FC_ASSERT( delta >= min_update, "Ezira Power decrease is not enough of a difference. min_update: ${min}", ("min", min_update) );
         FC_ASSERT( op.ESCOR >= min_delegation, "Delegation must be removed or leave minimum delegation amount of ${v}", ("v", min_delegation) );
      }
      else
      {
         FC_ASSERT( delegation->ESCOR.amount > 0, "Delegation would set eScore to zero, but it is already zero");
      }

      _db.create< ECO_fund_for_ESCOR_delegation_expiration_object >( [&]( ECO_fund_for_ESCOR_delegation_expiration_object& obj )
      {
         obj.delegator = op.delegator;
         obj.ESCOR = delta;
         obj.expiration = std::max( _db.head_block_time() + CASHOUT_WINDOW_SECONDS, delegation->min_delegation_time );
      });

      _db.modify( delegatee, [&]( account_object& a )
      {
         a.ESCORreceived -= delta;
      });

      if( op.ESCOR.amount > 0 )
      {
         _db.modify( *delegation, [&]( ECO_fund_for_ESCOR_delegation_object& obj )
         {
            obj.ESCOR = op.ESCOR;
         });
      }
      else
      {
         _db.remove( *delegation );
      }
   }
}

} } // eznode::chain
