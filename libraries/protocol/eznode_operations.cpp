#include <eznode/protocol/eznode_operations.hpp>
#include <fc/io/json.hpp>

#include <locale>

namespace eznode { namespace protocol {

   bool inline is_asset_type( asset asset, asset_symbol_type symbol )
   {
      return asset.symbol == symbol;
   }

   void accountCreate_operation::validate() const
   {
      validate_account_name( new_account_name );
      FC_ASSERT( is_asset_type( fee, SYMBOL_ECO ), "Account creation fee must be ECO" );
      owner.validate();
      active.validate();

      if ( json_metadata.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8(json_metadata), "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid(json_metadata), "JSON Metadata not valid JSON" );
      }
      FC_ASSERT( fee >= asset( 0, SYMBOL_ECO ), "Account creation fee cannot be negative" );
   }

   void accountCreateWithDelegation_operation::validate() const
   {
      validate_account_name( new_account_name );
      validate_account_name( creator );
      FC_ASSERT( is_asset_type( fee, SYMBOL_ECO ), "Account creation fee must be ECO" );
      FC_ASSERT( is_asset_type( delegation, SYMBOL_ESCOR ), "Delegation must be ESCOR" );

      owner.validate();
      active.validate();
      posting.validate();

      if( json_metadata.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8(json_metadata), "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid(json_metadata), "JSON Metadata not valid JSON" );
      }

      FC_ASSERT( fee >= asset( 0, SYMBOL_ECO ), "Account creation fee cannot be negative" );
      FC_ASSERT( delegation >= asset( 0, SYMBOL_ESCOR ), "Delegation cannot be negative" );
   }

   void accountUpdate_operation::validate() const
   {
      validate_account_name( account );
      /*if( owner )
         owner->validate();
      if( active )
         active->validate();
      if( posting )
         posting->validate();*/

      if ( json_metadata.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8(json_metadata), "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid(json_metadata), "JSON Metadata not valid JSON" );
      }
   }

   void comment_operation::validate() const
   {
      FC_ASSERT( title.size() < 256, "Title larger than size limit" );
      FC_ASSERT( fc::is_utf8( title ), "Title not formatted in UTF8" );
      FC_ASSERT( body.size() > 0, "Body is empty" );
      FC_ASSERT( fc::is_utf8( body ), "Body not formatted in UTF8" );


      if( parent_author.size() )
         validate_account_name( parent_author );
      validate_account_name( author );
      validate_permlink( parent_permlink );
      validate_permlink( permlink );

      if( json_metadata.size() > 0 )
      {
         FC_ASSERT( fc::json::is_valid(json_metadata), "JSON Metadata not valid JSON" );
      }
   }

   struct comment_options_extension_validate_visitor
   {
      comment_options_extension_validate_visitor() {}

      typedef void result_type;

      void operator()( const comment_payout_beneficiaries& cpb ) const
      {
         cpb.validate();
      }
   };

   void comment_payout_beneficiaries::validate()const
   {
      uint32_t sum = 0;

      FC_ASSERT( beneficiaries.size(), "Must specify at least one beneficiary" );
      FC_ASSERT( beneficiaries.size() < 128, "Cannot specify more than 127 beneficiaries." ); // Require size serializtion fits in one byte.

      validate_account_name( beneficiaries[0].account );
      FC_ASSERT( beneficiaries[0].weight <= PERCENT_100, "Cannot allocate more than 100% of rewards to one account" );
      sum += beneficiaries[0].weight;
      FC_ASSERT( sum <= PERCENT_100, "Cannot allocate more than 100% of rewards to a comment" ); // Have to check incrementally to avoid overflow

      for( size_t i = 1; i < beneficiaries.size(); i++ )
      {
         validate_account_name( beneficiaries[i].account );
         FC_ASSERT( beneficiaries[i].weight <= PERCENT_100, "Cannot allocate more than 100% of rewards to one account" );
         sum += beneficiaries[i].weight;
         FC_ASSERT( sum <= PERCENT_100, "Cannot allocate more than 100% of rewards to a comment" ); // Have to check incrementally to avoid overflow
         FC_ASSERT( beneficiaries[i - 1] < beneficiaries[i], "Benficiaries must be specified in sorted order (account ascending)" );
      }
   }

   void comment_options_operation::validate()const
   {
      validate_account_name( author );
      FC_ASSERT( percent_EUSD <= PERCENT_100, "Percent cannot exceed 100%" );
      FC_ASSERT( max_accepted_payout.symbol == SYMBOL_EUSD, "Max accepted payout must be in EUSD" );
      FC_ASSERT( max_accepted_payout.amount.value >= 0, "Cannot accept less than 0 payout" );
      validate_permlink( permlink );
      for( auto& e : extensions )
         e.visit( comment_options_extension_validate_visitor() );
   }

   void deleteComment_operation::validate()const
   {
      validate_permlink( permlink );
      validate_account_name( author );
   }

   void challenge_authority_operation::validate()const
    {
      validate_account_name( challenger );
      validate_account_name( challenged );
      FC_ASSERT( challenged != challenger, "cannot challenge yourself" );
   }

   void prove_authority_operation::validate()const
   {
      validate_account_name( challenged );
   }

   void vote_operation::validate() const
   {
      validate_account_name( voter );
      validate_account_name( author );\
      FC_ASSERT( abs(weight) <= PERCENT_100, "Weight is not a ECO percentage" );
      validate_permlink( permlink );
   }

   void transfer_operation::validate() const
   { try {
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( amount.symbol != SYMBOL_ESCOR, "transferring of Ezira Power (STMP) is not allowed." );
      FC_ASSERT( amount.amount > 0, "Cannot transfer a negative amount (aka: stealing)" );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE, "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ), "Memo is not UTF8" );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   void transferECOtoESCORfund_operation::validate() const
   {
      validate_account_name( from );
      FC_ASSERT( is_asset_type( amount, SYMBOL_ECO ), "Amount must be ECO" );
      if ( to != account_name_type() ) validate_account_name( to );
      FC_ASSERT( amount > asset( 0, SYMBOL_ECO ), "Must transfer a nonzero amount" );
   }

   void withdraw_ESCOR_operation::validate() const
   {
      validate_account_name( account );
      FC_ASSERT( is_asset_type( eScore, SYMBOL_ESCOR), "Amount must be ESCOR"  );
   }

   void setWithdrawESCORasECOroute_operation::validate() const
   {
      validate_account_name( from_account );
      validate_account_name( to_account );
      FC_ASSERT( 0 <= percent && percent <= PERCENT_100, "Percent must be valid ECO percent" );
   }

   void witness_update_operation::validate() const
   {
      validate_account_name( owner );
      FC_ASSERT( url.size() > 0, "URL size must be greater than 0" );
      FC_ASSERT( fc::is_utf8( url ), "URL is not valid UTF8" );
      FC_ASSERT( fee >= asset( 0, SYMBOL_ECO ), "Fee cannot be negative" );
      props.validate();
   }

   void accountWitnessVote_operation::validate() const
   {
      validate_account_name( account );
      validate_account_name( witness );
   }

   void account_witness_proxy_operation::validate() const
   {
      validate_account_name( account );
      if( proxy.size() )
         validate_account_name( proxy );
      FC_ASSERT( proxy != account, "Cannot proxy to self" );
   }

   void custom_operation::validate() const {
      /// required auth accounts are the ones whose bandwidth is consumed
      FC_ASSERT( required_auths.size() > 0, "at least on account must be specified" );
   }
   void customJson_operation::validate() const {
      /// required auth accounts are the ones whose bandwidth is consumed
      FC_ASSERT( (required_auths.size() + required_posting_auths.size()) > 0, "at least on account must be specified" );
      FC_ASSERT( id.size() <= 32, "id is too long" );
      FC_ASSERT( fc::is_utf8(json), "JSON Metadata not formatted in UTF8" );
      FC_ASSERT( fc::json::is_valid(json), "JSON Metadata not valid JSON" );
   }
   void custom_binary_operation::validate() const {
      /// required auth accounts are the ones whose bandwidth is consumed
      FC_ASSERT( (required_owner_auths.size() + required_active_auths.size() + required_posting_auths.size()) > 0, "at least on account must be specified" );
      FC_ASSERT( id.size() <= 32, "id is too long" );
      for( const auto& a : required_auths ) a.validate();
   }


   fc::sha256 pow_operation::work_input()const
   {
      auto hash = fc::sha256::hash( block_id );
      hash._hash[0] = nonce;
      return fc::sha256::hash( hash );
   }

   void pow_operation::validate()const
   {
      props.validate();
      validate_account_name( worker_account );
      FC_ASSERT( work_input() == work.input, "Determninistic input does not match recorded input" );
      work.validate();
   }

   struct pow2_operation_validate_visitor
   {
      typedef void result_type;

      template< typename PowType >
      void operator()( const PowType& pow )const
      {
         pow.validate();
      }
   };

   void pow2_operation::validate()const
   {
      props.validate();
      work.visit( pow2_operation_validate_visitor() );
   }

   struct pow2_operation_get_required_active_visitor
   {
      typedef void result_type;

      pow2_operation_get_required_active_visitor( flat_set< account_name_type >& required_active )
         : _required_active( required_active ) {}

      template< typename PowType >
      void operator()( const PowType& work )const
      {
         _required_active.insert( work.input.worker_account );
      }

      flat_set<account_name_type>& _required_active;
   };

   void pow2_operation::get_required_active_authorities( flat_set<account_name_type>& a )const
   {
      if( !new_owner_key )
      {
         pow2_operation_get_required_active_visitor vtor( a );
         work.visit( vtor );
      }
   }

   void pow::create( const fc::ecc::private_key& w, const digest_type& i )
   {
      input  = i;
      signature = w.sign_compact(input,false);

      auto sig_hash            = fc::sha256::hash( signature );
      public_key_type recover  = fc::ecc::public_key( signature, sig_hash, false );

      work = fc::sha256::hash(recover);
   }
   void pow2::create( const block_id_type& prev, const account_name_type& account_name, uint64_t n )
   {
      input.worker_account = account_name;
      input.prev_block     = prev;
      input.nonce          = n;

      auto prv_key = fc::sha256::hash( input );
      auto input = fc::sha256::hash( prv_key );
      auto signature = fc::ecc::private_key::regenerate( prv_key ).sign_compact(input);

      auto sig_hash            = fc::sha256::hash( signature );
      public_key_type recover  = fc::ecc::public_key( signature, sig_hash );

      fc::sha256 work = fc::sha256::hash(std::make_pair(input,recover));
      pow_summary = work.approx_log_32();
   }

   void equihash_pow::create( const block_id_type& recent_block, const account_name_type& account_name, uint32_t nonce )
   {
      input.worker_account = account_name;
      input.prev_block = recent_block;
      input.nonce = nonce;

      auto seed = fc::sha256::hash( input );
      proof = fc::equihash::proof::hash( EQUIHASH_N, EQUIHASH_K, seed );
      pow_summary = fc::sha256::hash( proof.inputs ).approx_log_32();
   }

   void pow::validate()const
   {
      FC_ASSERT( work != fc::sha256() );
      FC_ASSERT( public_key_type(fc::ecc::public_key( signature, input, false )) == worker );
      auto sig_hash = fc::sha256::hash( signature );
      public_key_type recover  = fc::ecc::public_key( signature, sig_hash, false );
      FC_ASSERT( work == fc::sha256::hash(recover) );
   }

   void pow2::validate()const
   {
      validate_account_name( input.worker_account );
      pow2 tmp; tmp.create( input.prev_block, input.worker_account, input.nonce );
      FC_ASSERT( pow_summary == tmp.pow_summary, "reported work does not match calculated work" );
   }

   void equihash_pow::validate() const
   {
      validate_account_name( input.worker_account );
      auto seed = fc::sha256::hash( input );
      FC_ASSERT( proof.n == EQUIHASH_N, "proof of work 'n' value is incorrect" );
      FC_ASSERT( proof.k == EQUIHASH_K, "proof of work 'k' value is incorrect" );
      FC_ASSERT( proof.seed == seed, "proof of work seed does not match expected seed" );
      FC_ASSERT( proof.is_valid(), "proof of work is not a solution", ("block_id", input.prev_block)("worker_account", input.worker_account)("nonce", input.nonce) );
      FC_ASSERT( pow_summary == fc::sha256::hash( proof.inputs ).approx_log_32() );
   }

   void feed_publish_operation::validate()const
   {
      validate_account_name( publisher );
      FC_ASSERT( ( is_asset_type( exchange_rate.base, SYMBOL_ECO ) && is_asset_type( exchange_rate.quote, SYMBOL_EUSD ) )
         || ( is_asset_type( exchange_rate.base, SYMBOL_EUSD ) && is_asset_type( exchange_rate.quote, SYMBOL_ECO ) ),
         "Price feed must be a ECO/EUSD price" );
      exchange_rate.validate();
   }

   void limit_order_create_operation::validate()const
   {
      validate_account_name( owner );
      FC_ASSERT( ( is_asset_type( amount_to_sell, SYMBOL_ECO ) && is_asset_type( min_to_receive, SYMBOL_EUSD ) )
         || ( is_asset_type( amount_to_sell, SYMBOL_EUSD ) && is_asset_type( min_to_receive, SYMBOL_ECO ) ),
         "Limit order must be for the ECO:EUSD market" );
      (amount_to_sell / min_to_receive).validate();
   }
   void limit_order_create2_operation::validate()const
   {
      validate_account_name( owner );
      FC_ASSERT( amount_to_sell.symbol == exchange_rate.base.symbol, "Sell asset must be the base of the price" );
      exchange_rate.validate();

      FC_ASSERT( ( is_asset_type( amount_to_sell, SYMBOL_ECO ) && is_asset_type( exchange_rate.quote, SYMBOL_EUSD ) ) ||
                 ( is_asset_type( amount_to_sell, SYMBOL_EUSD ) && is_asset_type( exchange_rate.quote, SYMBOL_ECO ) ),
                 "Limit order must be for the ECO:EUSD market" );

      FC_ASSERT( (amount_to_sell * exchange_rate).amount > 0, "Amount to sell cannot round to 0 when traded" );
   }

   void limit_order_cancel_operation::validate()const
   {
      validate_account_name( owner );
   }

   void convert_operation::validate()const
   {
      validate_account_name( owner );
      /// only allow conversion from EUSD to ECO, allowing the opposite can enable traders to abuse
      /// market fluxuations through converting large quantities without moving the price.
      FC_ASSERT( is_asset_type( amount, SYMBOL_EUSD ), "Can only convert EUSD to ECO" );
      FC_ASSERT( amount.amount > 0, "Must convert some EUSD" );
   }

   void report_over_production_operation::validate()const
   {
      validate_account_name( reporter );
      validate_account_name( first_block.witness );
      FC_ASSERT( first_block.witness   == second_block.witness );
      FC_ASSERT( first_block.timestamp == second_block.timestamp );
      FC_ASSERT( first_block.signee()  == second_block.signee() );
      FC_ASSERT( first_block.id() != second_block.id() );
   }

   void escrow_transfer_operation::validate()const
   {
      validate_account_name( from );
      validate_account_name( to );
      validate_account_name( agent );
      FC_ASSERT( fee.amount >= 0, "fee cannot be negative" );
      FC_ASSERT( EUSD_amount.amount >= 0, "EUSD amount cannot be negative" );
      FC_ASSERT( ECOamount.amount >= 0, "ECO amount cannot be negative" );
      FC_ASSERT( EUSD_amount.amount > 0 || ECOamount.amount > 0, "escrow must transfer a non-zero amount" );
      FC_ASSERT( from != agent && to != agent, "agent must be a third party" );
      FC_ASSERT( (fee.symbol == SYMBOL_ECO) || (fee.symbol == SYMBOL_EUSD), "fee must be ECO or EUSD" );
      FC_ASSERT( EUSD_amount.symbol == SYMBOL_EUSD, "EUSD amount must contain EUSD" );
      FC_ASSERT( ECOamount.symbol == SYMBOL_ECO, "ECO amount must contain ECO" );
      FC_ASSERT( ratification_deadline < escrow_expiration, "ratification deadline must be before escrow expiration" );
      if ( json_meta.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8(json_meta), "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid(json_meta), "JSON Metadata not valid JSON" );
      }
   }

   void escrow_approve_operation::validate()const
   {
      validate_account_name( from );
      validate_account_name( to );
      validate_account_name( agent );
      validate_account_name( who );
      FC_ASSERT( who == to || who == agent, "to or agent must approve escrow" );
   }

   void escrow_dispute_operation::validate()const
   {
      validate_account_name( from );
      validate_account_name( to );
      validate_account_name( agent );
      validate_account_name( who );
      FC_ASSERT( who == from || who == to, "who must be from or to" );
   }

   void escrow_release_operation::validate()const
   {
      validate_account_name( from );
      validate_account_name( to );
      validate_account_name( agent );
      validate_account_name( who );
      validate_account_name( receiver );
      FC_ASSERT( who == from || who == to || who == agent, "who must be from or to or agent" );
      FC_ASSERT( receiver == from || receiver == to, "receiver must be from or to" );
      FC_ASSERT( EUSD_amount.amount >= 0, "EUSD amount cannot be negative" );
      FC_ASSERT( ECOamount.amount >= 0, "ECO amount cannot be negative" );
      FC_ASSERT( EUSD_amount.amount > 0 || ECOamount.amount > 0, "escrow must release a non-zero amount" );
      FC_ASSERT( EUSD_amount.symbol == SYMBOL_EUSD, "EUSD amount must contain EUSD" );
      FC_ASSERT( ECOamount.symbol == SYMBOL_ECO, "ECO amount must contain ECO" );
   }

   void request_account_recovery_operation::validate()const
   {
      validate_account_name( recovery_account );
      validate_account_name( account_to_recover );
      new_owner_authority.validate();
   }

   void recover_account_operation::validate()const
   {
      validate_account_name( account_to_recover );
      FC_ASSERT( !( new_owner_authority == recent_owner_authority ), "Cannot set new owner authority to the recent owner authority" );
      FC_ASSERT( !new_owner_authority.is_impossible(), "new owner authority cannot be impossible" );
      FC_ASSERT( !recent_owner_authority.is_impossible(), "recent owner authority cannot be impossible" );
      FC_ASSERT( new_owner_authority.weight_threshold, "new owner authority cannot be trivial" );
      new_owner_authority.validate();
      recent_owner_authority.validate();
   }

   void change_recovery_account_operation::validate()const
   {
      validate_account_name( account_to_recover );
      validate_account_name( new_recovery_account );
   }

   void transferToSavings_operation::validate()const {
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( amount.amount > 0 );
      FC_ASSERT( amount.symbol == SYMBOL_ECO || amount.symbol == SYMBOL_EUSD );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE, "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ), "Memo is not UTF8" );
   }
   void transferFromSavings_operation::validate()const {
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( amount.amount > 0 );
      FC_ASSERT( amount.symbol == SYMBOL_ECO || amount.symbol == SYMBOL_EUSD );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE, "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ), "Memo is not UTF8" );
   }
   void cancelTransferFromSavings_operation::validate()const {
      validate_account_name( from );
   }

   void decline_voting_rights_operation::validate()const
   {
      validate_account_name( account );
   }

   void reset_account_operation::validate()const
   {
      validate_account_name( reset_account );
      validate_account_name( account_to_reset );
      FC_ASSERT( !new_owner_authority.is_impossible(), "new owner authority cannot be impossible" );
      FC_ASSERT( new_owner_authority.weight_threshold, "new owner authority cannot be trivial" );
      new_owner_authority.validate();
   }

   void set_reset_account_operation::validate()const
   {
      validate_account_name( account );
      if( current_reset_account.size() )
         validate_account_name( current_reset_account );
      validate_account_name( reset_account );
      FC_ASSERT( current_reset_account != reset_account, "new reset account cannot be current reset account" );
   }

   void claimRewardBalance_operation::validate()const
   {
      validate_account_name( account );
      FC_ASSERT( is_asset_type( ECOreward, SYMBOL_ECO ), "Reward_ECO must be ECO" );
      FC_ASSERT( is_asset_type( EUSDreward, SYMBOL_EUSD ), "EUSDreward must be EUSD" );
      FC_ASSERT( is_asset_type( ESCORreward, SYMBOL_ESCOR ), "Reward_ESCOR must be ESCOR" );
      FC_ASSERT( ECOreward.amount >= 0, "Cannot claim a negative amount" );
      FC_ASSERT( EUSDreward.amount >= 0, "Cannot claim a negative amount" );
      FC_ASSERT( ESCORreward.amount >= 0, "Cannot claim a negative amount" );
      FC_ASSERT( ECOreward.amount > 0 || EUSDreward.amount > 0 || ESCORreward.amount > 0, "Must claim something." );
   }

   void delegateESCOR_operation::validate()const
   {
      validate_account_name( delegator );
      validate_account_name( delegatee );
      FC_ASSERT( delegator != delegatee, "You cannot delegate ESCOR to yourself" );
      FC_ASSERT( is_asset_type( eScore, SYMBOL_ESCOR ), "Delegation must be ESCOR" );
      FC_ASSERT( eScore >= asset( 0, SYMBOL_ESCOR ), "Delegation cannot be negative" );
   }

} } // eznode::protocol
