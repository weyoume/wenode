#include <node/protocol/node_operations.hpp>
#include <fc/io/json.hpp>

#include <locale>

namespace node { namespace protocol {

   bool inline is_asset_type( asset a, asset_symbol_type symbol )
   {
      return a.symbol == symbol;
   }

   void account_create_operation::validate() const
   {
      if( account_type == PERSONA )
      {
         validate_persona_account_name( new_account_name );
      }
      else if( account_type == PROFILE )
      {
         validate_profile_account_name( new_account_name );
      }
      else if( account_type == BUSINESS )
      {
         validate_business_account_name( new_account_name );
      }
      
      
      
      FC_ASSERT( is_asset_type( fee, SYMBOL_COIN ), "Account creation fee must be in core asset" );
      FC_ASSERT( is_asset_type( delegation, SYMBOL_COIN ), "Delegation must be in core asset" );

      owner.validate();
      active.validate();

      if ( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8(json), "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid(json), "JSON Metadata not valid JSON" );
      }
      FC_ASSERT( fee >= asset( 0, SYMBOL_COIN ), "Account creation fee cannot be negative" );
      FC_ASSERT( delegation >= asset( 0, SYMBOL_COIN ), "Delegation cannot be negative" );
   }

   void account_update_operation::validate() const
   {
      validate_account_name( account );
      /*if( owner )
         owner->validate();
      if( active )
         active->validate();
      if( posting )
         posting->validate();*/

      if ( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8(json), "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid(json), "JSON Metadata not valid JSON" );
      }
   }

   void comment_operation::validate() const
   {
      string title_text;
     
      FC_ASSERT( body.size() > 0, "Body is empty" );
      FC_ASSERT( fc::is_utf8( body ), "Body not formatted in UTF8" );

      FC_ASSERT( body.size() + json.size(), "Cannot update comment because it does not contain content." );
      FC_ASSERT( body.size() < MAX_BODY_SIZE, "Comment rejected: Body size is too large",("Body.size", body.size() ) );
      FC_ASSERT( media.size() < MAX_BODY_SIZE, "Comment rejected: Media size is too large",("Media.size", media.size() ) );
      FC_ASSERT( json.size() < MAX_BODY_SIZE, "Comment rejected: Body size is too large",("JSON.size", json.size() ) );
      FC_ASSERT( fc::is_utf8( json ), "Comment rejected: JSON is not valid UTF8");

      if( parent_author.size() )
         validate_account_name( parent_author );
      validate_account_name( author );
      validate_permlink( parent_permlink );
      validate_permlink( permlink );

      if( json.size() > 0 )
      {
         FC_ASSERT( fc::json::is_valid(json), "JSON Metadata not valid JSON" );
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
      FC_ASSERT( percent_liquid <= PERCENT_100, "Percent cannot exceed 100%" );
      FC_ASSERT( max_accepted_payout.symbol == SYMBOL_USD, "Max accepted payout must be in USD" );
      FC_ASSERT( max_accepted_payout.amount.value >= 0, "Cannot accept less than 0 payout" );
      validate_permlink( permlink );
      for( auto& e : extensions )
         e.visit( comment_options_extension_validate_visitor() );
   }

   void vote_operation::validate() const
   {
      validate_account_name( voter );
      validate_account_name( author );
      FC_ASSERT( abs(weight) <= PERCENT_100, "Weight is not a valid percentage (0 - 10000)" );
      validate_permlink( permlink );
   }

   void transfer_operation::validate() const
   { try {
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( amount.amount > 0, "INVALID TRANSFER: NEGATIVE AMOUNT - THEFT NOT PERMITTED." );
      FC_ASSERT( is_valid_symbol(amount.symbol), "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE, "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ), "Memo is not UTF8" );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   void stake_asset_operation::validate() const
   {
      validate_account_name( from );
      FC_ASSERT( is_valid_symbol(amount.symbol), "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      if ( to != account_name_type() ) validate_account_name( to );
      FC_ASSERT( amount.amount > 0, "Must transfer a nonzero amount" );
   }

   void unstake_asset_operation::validate() const
   {
      validate_account_name( account );
      FC_ASSERT( is_valid_symbol(amount.symbol), "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( amount.amount >= 0, "Cannot withdraw negative stake. Account: ${account}, Amount:${amount}", ("account", account)("amount", amount) );
   }

   void unstake_asset_route_operation::validate() const
   {
      validate_account_name( from_account );
      validate_account_name( to_account );
      FC_ASSERT( 0 <= percent && percent <= PERCENT_100, "Percent must be valid percent (0 - 10000)" );
   }

   void witness_update_operation::validate() const
   {
      validate_account_name( owner );
      FC_ASSERT( url.size() > 0, "URL size must be greater than 0" );
      FC_ASSERT( fc::is_utf8( url ), "URL is not valid UTF8" );
      FC_ASSERT( fee >= asset( 0, SYMBOL_COIN ), "Fee cannot be negative" );
      props.validate();
   }

   void account_witness_vote_operation::validate() const
   {
      validate_account_name( account );
      validate_account_name( witness );
   }

   void account_update_proxy_operation::validate() const
   {
      validate_account_name( account );
      if( proxy.size() )
         validate_account_name( proxy );
      FC_ASSERT( proxy != account, "Cannot proxy to self" );
   }

   void custom_operation::validate() const 
   {
      /// required auth accounts are the ones whose bandwidth is consumed
      FC_ASSERT( required_auths.size() > 0, "at least on account must be specified" );
   }

   void custom_json_operation::validate() const 
   {
      /// required auth accounts are the ones whose bandwidth is consumed
      FC_ASSERT( (required_auths.size() + required_posting_auths.size()) > 0, "at least on account must be specified" );
      FC_ASSERT( id.size() <= 32, "id is too long" );
      FC_ASSERT( fc::is_utf8(json), "JSON Metadata not formatted in UTF8" );
      FC_ASSERT( fc::json::is_valid(json), "JSON Metadata not valid JSON" );
   }


   struct proof_of_work_operation_validate_visitor
   {
      typedef void result_type;

      template< typename PowType >
      void operator()( const PowType& pow )const
      {
         pow.validate();
      }
   };

   void proof_of_work_operation::validate()const
   {
      props.validate();
      work.visit( proof_of_work_operation_validate_visitor() );
   };

   struct proof_of_work_operation_get_required_active_visitor
   {
      typedef void result_type;

      proof_of_work_operation_get_required_active_visitor( flat_set< account_name_type >& required_active )
         : _required_active( required_active ) {}

      template< typename PowType >
      void operator()( const PowType& work )const
      {
         _required_active.insert( work.input.worker_account );
      }

      flat_set<account_name_type>& _required_active;
   };

   void proof_of_work_operation::get_required_active_authorities( flat_set<account_name_type>& a )const
   {
      if( !new_owner_key )
      {
         proof_of_work_operation_get_required_active_visitor vtor( a );
         work.visit( vtor );
      }
   }

   void proof_of_work::create( const block_id_type& prev, const account_name_type& account_name, uint64_t n )
   {
      input.miner_account = account_name;
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

   void equihash_proof_of_work::create( const block_id_type& recent_block, const account_name_type& account_name, uint32_t nonce )
   {
      input.miner_account = account_name;
      input.prev_block = recent_block;
      input.nonce = nonce;

      auto seed = fc::sha256::hash( input );
      proof = fc::equihash::proof::hash( EQUIHASH_N, EQUIHASH_K, seed );
      pow_summary = fc::sha256::hash( proof.inputs ).approx_log_32();
   }

   void proof_of_work::validate()const
   {
      validate_account_name( input.miner_account );
      proof_of_work tmp; tmp.create( input.prev_block, input.miner_account, input.nonce );
      FC_ASSERT( pow_summary == tmp.pow_summary, "reported work does not match calculated work" );
   }

   void equihash_proof_of_work::validate() const
   {
      validate_account_name( input.miner_account );
      auto seed = fc::sha256::hash( input );
      FC_ASSERT( proof.n == EQUIHASH_N, "proof of work 'n' value is incorrect" );
      FC_ASSERT( proof.k == EQUIHASH_K, "proof of work 'k' value is incorrect" );
      FC_ASSERT( proof.seed == seed, "proof of work seed does not match expected seed" );
      FC_ASSERT( proof.is_valid(), "proof of work is not a solution", ("block_id", input.prev_block)("worker_account", input.miner_account)("nonce", input.nonce) );
      FC_ASSERT( pow_summary == fc::sha256::hash( proof.inputs ).approx_log_32() );
   }

   void limit_order_create_operation::validate()const
   {
      validate_account_name( owner );
      FC_ASSERT( amount_to_sell.symbol == exchange_rate.base.symbol, "Sell asset must be the base of the price" );
      exchange_rate.validate();

      FC_ASSERT( (amount_to_sell * exchange_rate).amount > 0, "Amount to sell cannot round to 0 when traded" );
   }

   void limit_order_cancel_operation::validate()const
   {
      validate_account_name( owner );
   }

   void escrow_transfer_operation::validate()const
   {
      validate_account_name( from );
      validate_account_name( to );
      validate_account_name( agent );
      FC_ASSERT( is_valid_symbol(fee.symbol), "Symbol ${symbol} is not a valid symbol", ("symbol", fee.symbol) );
      FC_ASSERT( is_valid_symbol(amount.symbol), "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( fee.amount >= 0, "Fee cannot be negative" );
      FC_ASSERT( amount.amount >= 0, "Amount cannot be negative" );
      FC_ASSERT( from != agent && to != agent, "Agent account must be a third party" );
      
      FC_ASSERT( ratification_deadline < escrow_expiration, "ratification deadline must be before escrow expiration" );
      if ( json.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8(json), "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid(json), "JSON Metadata not valid JSON" );
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
      FC_ASSERT( is_valid_symbol(amount.symbol), "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( amount.amount > 0, "amount cannot be greater than zero" );
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

   void transfer_to_savings_operation::validate()const 
   {
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( amount.amount > 0 );
      FC_ASSERT( is_valid_symbol(amount.symbol), "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE, "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ), "Memo is not UTF8" );
   }

   void transfer_from_savings_operation::validate()const 
   {
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( amount.amount > 0 );
      FC_ASSERT( is_valid_symbol(amount.symbol), "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
      FC_ASSERT( memo.size() < MAX_MEMO_SIZE, "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ), "Memo is not UTF8" );
   }

   void cancel_transfer_from_savings_operation::validate()const 
   {
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

   void claim_reward_balance_operation::validate()const
   {
      validate_account_name( account );
      FC_ASSERT( reward.amount > 0 , "Must claim something." );
      FC_ASSERT( is_valid_symbol(reward.symbol), "Symbol ${symbol} is not a valid symbol", ("symbol", reward.symbol) );
   }

   void delegate_asset_operation::validate()const
   {
      validate_account_name( delegator );
      validate_account_name( delegatee );
      FC_ASSERT( delegator != delegatee, "You cannot delegate to yourself" );
      FC_ASSERT( amount.amount >= 0, "Delegation cannot be negative" );
      FC_ASSERT( is_valid_symbol(amount.symbol), "Symbol ${symbol} is not a valid symbol", ("symbol", amount.symbol) );
   }

   /**
 *  Valid symbols can contain [A-Z0-9], and '.'
 *  They must start with [A, Z]
 *  They must end with [A, Z] before HF_620 or [A-Z0-9] after it
 *  They can contain a maximum of two '.'
 */
bool is_valid_symbol( const string& symbol )
{
   static const std::locale& loc = std::locale::classic();
   if( symbol.size() < MIN_ASSET_SYMBOL_LENGTH )
      return false;

   if( symbol.substr(0,3) == "BIT" ) 
      return false;

   if( symbol.substr(0,2) == "ME" ) 
      return false;

   if( symbol.substr(0,3) == "WYM" ) 
      return false;

   if( symbol.substr(0,3) == "WYM" ) 
      return false;

   if( symbol.size() > MAX_ASSET_SYMBOL_LENGTH )
      return false;

   if( !isalpha( symbol.front(), loc ) )
      return false;

   if( !isalnum( symbol.back(), loc ) )
      return false;

   uint8_t dot_count = 0;
   for( const auto c : symbol )
   {
      if( (isalpha( c, loc ) && isupper( c, loc )) || isdigit( c, loc ) )
         continue;

      if( c == '.' )
      {
         dot_count++;
         if( dot_count > 2 )
         {
            return false;
         }
         continue;
      }

      return false;
   }

    return true;
}

void asset_create_operation::validate()const
{
   validate_account_name( issuer );
   FC_ASSERT( is_valid_symbol(symbol), "Symbol ${symbol} is not a valid symbol", ("symbol", symbol) );
   common_options.validate();
   if( common_options.issuer_permissions & (disable_force_settle|global_settle) )
      FC_ASSERT( bitasset_opts.valid() );
   if( bitasset_opts )
   {
      bitasset_opts->validate();
   } 
   if( equity_opts )
   {
      bitasset_opts->validate();
   }
   if( credit_opts )
   {
      bitasset_opts->validate();
   }
}

void asset_update_operation::validate()const
{
   validate_account_name( issuer );
   if( new_issuer )
      FC_ASSERT(issuer != *new_issuer);
   new_options.validate();
}

void asset_update_issuer_operation::validate()const
{
   validate_account_name( issuer );
   FC_ASSERT( issuer != new_issuer );
}

void asset_publish_feed_operation::validate()const
{
   validate_account_name( publisher );
   feed.validate();

   if( !feed.core_exchange_rate.is_null() )
   {
      feed.core_exchange_rate.validate();
   }
   if( (!feed.settlement_price.is_null()) && (!feed.core_exchange_rate.is_null()) )
   {
      FC_ASSERT( feed.settlement_price.base.symbol == feed.core_exchange_rate.base.symbol );
   }

   FC_ASSERT( !feed.settlement_price.is_null() );
   FC_ASSERT( !feed.core_exchange_rate.is_null() );
   FC_ASSERT( feed.is_for( symbol ) );
}

void asset_reserve_operation::validate()const
{
   validate_account_name( payer );
   FC_ASSERT( amount_to_reserve.amount.value <= MAX_ASSET_SUPPLY );
   FC_ASSERT( amount_to_reserve.amount.value > 0 );
}

void asset_issue_operation::validate()const
{
   validate_account_name( issuer );
   FC_ASSERT( asset_to_issue.amount.value <= MAX_ASSET_SUPPLY );
   FC_ASSERT( asset_to_issue.amount.value > 0 );
}

void asset_fund_fee_pool_operation::validate() const
{
   validate_account_name( from_account );
   FC_ASSERT( pool_amount.symbol == SYMBOL_COIN );
   FC_ASSERT( pool_amount.amount > 0 );
}

void asset_settle_operation::validate() const
{
   validate_account_name( account );
   FC_ASSERT( amount.amount >= 0 );
}

void asset_update_bitasset_operation::validate() const
{
   validate_account_name( issuer );
   new_options.validate();
}

void asset_update_feed_producers_operation::validate() const
{
   validate_account_name( issuer );
}

void asset_global_settle_operation::validate()const
{
   validate_account_name( issuer );
   FC_ASSERT( asset_to_settle == settle_price.base.symbol );
}

void bitasset_options::validate() const
{
   FC_ASSERT(minimum_feeds > 0);
   FC_ASSERT(force_settlement_offset_percent <= PERCENT_100);
   FC_ASSERT(maximum_force_settlement_volume <= PERCENT_100);
}

void asset_options::validate()const
{
   FC_ASSERT( max_supply > 0 );
   FC_ASSERT( max_supply <= MAX_ASSET_SUPPLY );
   FC_ASSERT( market_fee_percent <= PERCENT_100 );
   FC_ASSERT( max_market_fee >= 0 && max_market_fee <= MAX_ASSET_SUPPLY );
   // There must be no high bits in permissions whose meaning is not known.
   FC_ASSERT( !(issuer_permissions & ~ASSET_ISSUER_PERMISSION_MASK) );
   // The global_settle flag may never be set (this is a permission only)
   FC_ASSERT( !(flags & global_settle) );
   // the witness_fed and committee_fed flags cannot be set simultaneously
   FC_ASSERT( (flags & (witness_fed_asset | committee_fed_asset)) != (witness_fed_asset | committee_fed_asset) );
   core_exchange_rate.validate();
   if(!whitelist_authorities.empty() || !blacklist_authorities.empty())
      FC_ASSERT( flags & white_list );
   for( auto item : whitelist_markets )
   {
      FC_ASSERT( blacklist_markets.find(item) == blacklist_markets.end() );
   }
   for( auto item : blacklist_markets )
   {
      FC_ASSERT( whitelist_markets.find(item) == whitelist_markets.end() );
   }
   if( extensions.value.reward_percent.valid() )
      FC_ASSERT( *extensions.value.reward_percent < PERCENT_100 );
}

void asset_claim_fees_operation::validate()const {
   validate_account_name( issuer );
   FC_ASSERT( amount_to_claim.amount > 0 );
}

void asset_claim_pool_operation::validate()const {
   validate_account_name( issuer );
   FC_ASSERT( amount_to_claim.amount > 0 );
}

} } // node::protocol
