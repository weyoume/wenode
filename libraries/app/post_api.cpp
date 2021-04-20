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


   //==================//
   // === Post API === //
   //==================//


comment_interaction_state database_api::get_comment_interactions( string author, string permlink )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_comment_interactions( author, permlink );
   });
}

comment_interaction_state database_api_impl::get_comment_interactions( string author, string permlink )const
{
   comment_interaction_state results;

   const comment_object& comment = _db.get_comment( author, permlink );

   const auto& vote_idx = _db.get_index< comment_vote_index >().indices().get< by_comment_voter >();
   const auto& view_idx = _db.get_index< comment_view_index >().indices().get< by_comment_viewer >();
   const auto& share_idx = _db.get_index< comment_share_index >().indices().get< by_comment_sharer >();
   const auto& moderation_idx = _db.get_index< comment_moderation_index >().indices().get< by_comment_moderator >();

   comment_id_type cid(comment.id);

   auto vote_itr = vote_idx.lower_bound( cid );

   while( vote_itr != vote_idx.end() && 
      vote_itr->comment == cid )
   {
      vote_state vstate;

      vstate.voter = vote_itr->voter;
      vstate.weight = vote_itr->weight;
      vstate.reward = vote_itr->reward.value;
      vstate.percent = vote_itr->vote_percent;
      vstate.time = vote_itr->last_updated;
      results.votes.push_back( vstate );
      ++vote_itr;
   }

   auto view_itr = view_idx.lower_bound( cid );
   
   while( view_itr != view_idx.end() && 
      view_itr->comment == cid )
   {
      view_state vstate;

      vstate.viewer = view_itr->viewer;
      vstate.weight = view_itr->weight;
      vstate.reward = view_itr->reward.value;
      vstate.time = view_itr->created;
      results.views.push_back( vstate );
      ++view_itr;
   }
   
   auto share_itr = share_idx.lower_bound( cid );

   while( share_itr != share_idx.end() && 
      share_itr->comment == cid )
   {
      share_state sstate;

      sstate.sharer = share_itr->sharer;
      sstate.weight = share_itr->weight;
      sstate.reward = share_itr->reward.value;
      sstate.time = share_itr->created;

      results.shares.push_back( sstate );
      ++share_itr;
   }
  
   auto moderation_itr = moderation_idx.lower_bound( cid );

   while( moderation_itr != moderation_idx.end() && 
      moderation_itr->comment == cid )
   {
      moderation_state mstate;
      mstate.moderator = moderation_itr->moderator;
      for( auto tag : moderation_itr->tags )
      {
         mstate.tags.push_back( tag );
      }
      mstate.rating = moderation_itr->rating;
      mstate.details = to_string( moderation_itr->details );
      mstate.filter = moderation_itr->filter;
      mstate.time = moderation_itr->last_updated;

      results.moderation.push_back( mstate );
      ++moderation_itr;
   }

   return results;
}

vector< account_vote > database_api::get_account_votes( string account, string from_author, string from_permlink, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_votes( account, from_author, from_permlink, limit );
   });
}

vector< account_vote > database_api_impl::get_account_votes( string account, string from_author, string from_permlink, uint32_t limit )const
{
   vector< account_vote > results;
   limit = std::min( limit, uint32_t( 1000 ) );
   results.reserve( limit );

   const auto& com_vote_idx = _db.get_index< comment_vote_index >().indices().get< by_voter_comment >();

   auto com_vote_itr = com_vote_idx.lower_bound( account );
   auto com_vote_end = com_vote_idx.upper_bound( account );

   if( from_author.length() && from_permlink.length() )
   {
      const comment_object& com = _db.get_comment( from_author, from_permlink );
      auto from_itr = com_vote_idx.find( boost::make_tuple( account, com.id ) );
      if( from_itr != com_vote_idx.end() )
      {
         com_vote_itr = com_vote_idx.iterator_to( *from_itr );
      }
   }

   while( com_vote_itr != com_vote_end && results.size() < limit )
   {
      const comment_object& comment = _db.get( com_vote_itr->comment );
      account_vote avote;
      avote.author = comment.author;
      avote.permlink = to_string( comment.permlink );
      avote.weight = com_vote_itr->weight;
      avote.reward = com_vote_itr->reward.value;
      avote.percent = com_vote_itr->vote_percent;
      avote.time = com_vote_itr->last_updated;
      results.push_back( avote );
      ++com_vote_itr;
   }
   return results;
}

vector< account_view > database_api::get_account_views( string account, string from_author, string from_permlink, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_views( account, from_author, from_permlink, limit );
   });
}

vector< account_view > database_api_impl::get_account_views( string account, string from_author, string from_permlink, uint32_t limit )const
{
   vector< account_view > results;
   limit = std::min( limit, uint32_t( 1000 ) );
   results.reserve( limit );

   const auto& com_view_idx = _db.get_index< comment_view_index >().indices().get< by_viewer_comment >();
   auto com_view_itr = com_view_idx.lower_bound( account );
   auto com_view_end = com_view_idx.upper_bound( account );

   if( from_author.length() && from_permlink.length() )
   {
      const comment_object& com = _db.get_comment( from_author, from_permlink );
      auto from_itr = com_view_idx.find( boost::make_tuple( account, com.id ) );
      if( from_itr != com_view_idx.end() )
      {
         com_view_itr = com_view_idx.iterator_to( *from_itr );
      }
   }

   while( com_view_itr != com_view_end && results.size() < limit )
   {
      const comment_object& comment = _db.get( com_view_itr->comment );
      account_view aview;
      aview.author = comment.author;
      aview.permlink = to_string( comment.permlink );
      aview.weight = com_view_itr->weight;
      aview.reward = com_view_itr->reward.value;
      aview.time = com_view_itr->created;
      results.push_back( aview );
      ++com_view_itr;
   }
   return results;
}

vector< account_share > database_api::get_account_shares( string account, string from_author, string from_permlink, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_shares( account, from_author, from_permlink, limit );
   });
}

vector< account_share > database_api_impl::get_account_shares( string account, string from_author, string from_permlink, uint32_t limit )const
{
   vector< account_share > results;
   limit = std::min( limit, uint32_t( 1000 ) );
   results.reserve( limit );

   const auto& com_share_idx = _db.get_index< comment_share_index >().indices().get< by_sharer_comment >();
   auto com_share_itr = com_share_idx.lower_bound( account );
   auto com_share_end = com_share_idx.upper_bound( account );

   if( from_author.length() && from_permlink.length() )
   {
      const comment_object& com = _db.get_comment( from_author, from_permlink );
      auto from_itr = com_share_idx.find( boost::make_tuple( account, com.id ) );
      if( from_itr != com_share_idx.end() )
      {
         com_share_itr = com_share_idx.iterator_to( *from_itr );
      }
   }

   while( com_share_itr != com_share_end && results.size() < limit )
   {
      const comment_object& comment = _db.get( com_share_itr->comment );
      account_share ashare;
      ashare.author = comment.author;
      ashare.permlink = to_string( comment.permlink );
      ashare.weight = com_share_itr->weight;
      ashare.reward = com_share_itr->reward.value;
      ashare.time = com_share_itr->created;
      results.push_back( ashare );
      ++com_share_itr;
   }
   return results;
}

vector< account_moderation > database_api::get_account_moderation( string account, string from_author, string from_permlink, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_moderation( account, from_author, from_permlink, limit );
   });
}

vector< account_moderation > database_api_impl::get_account_moderation( string account, string from_author, string from_permlink, uint32_t limit )const
{
   vector< account_moderation > results;
   limit = std::min( limit, uint32_t( 1000 ) );
   results.reserve( limit );

   const auto& com_mod_idx = _db.get_index< comment_moderation_index >().indices().get< by_moderator_comment >();
   auto com_mod_itr = com_mod_idx.lower_bound( account );
   auto com_mod_end = com_mod_idx.upper_bound( account );

   if( from_author.length() && from_permlink.length() )
   {
      const comment_object& com = _db.get_comment( from_author, from_permlink );
      auto from_itr = com_mod_idx.find( boost::make_tuple( account, com.id ) );
      if( from_itr != com_mod_idx.end() )
      {
         com_mod_itr = com_mod_idx.iterator_to( *from_itr );
      }
   }

   while( com_mod_itr != com_mod_end && results.size() < limit )
   {
      const comment_object& comment = _db.get( com_mod_itr->comment );
      account_moderation amod;
      amod.author = comment.author;
      amod.permlink = to_string( comment.permlink );
      amod.tags.reserve( com_mod_itr->tags.size() );
      for( auto t : com_mod_itr->tags )
      {
         amod.tags.push_back( t );
      }
      amod.rating = com_mod_itr->rating;
      amod.details = to_string( com_mod_itr->details );
      amod.filter = com_mod_itr->filter;
      amod.time = com_mod_itr->last_updated;
      results.push_back( amod );
      ++com_mod_itr;
   }
   return results;
}

vector< account_tag_following_api_obj > database_api::get_account_tag_followings( vector< string > tags )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_tag_followings( tags );
   });
}

vector< account_tag_following_api_obj > database_api_impl::get_account_tag_followings( vector< string > tags )const
{
   vector< account_tag_following_api_obj > results;
   const auto& tag_idx = _db.get_index< account_tag_following_index >().indices().get< by_tag >();
   
   for( auto tag : tags )
   {
      auto tag_itr = tag_idx.find( tag );

      if( tag_itr != tag_idx.end() )
      {
         results.push_back( account_tag_following_api_obj( *tag_itr ) );
      }
   }
   return results;
}

vector< pair< tag_name_type, uint32_t > > database_api::get_tags_used_by_author( string author )const 
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_tags_used_by_author( author );
   });
}

vector< pair< tag_name_type, uint32_t > > database_api_impl::get_tags_used_by_author( string author )const 
{
   if( !_db.has_index<tags::author_tag_stats_index>() )
   {
      return vector< pair< tag_name_type, uint32_t > >();
   }

   const account_object* account_ptr = _db.find_account( author );
   FC_ASSERT( account_ptr != nullptr,
      "Account not found." );

   const auto& author_tag_idx = _db.get_index< tags::author_tag_stats_index >().indices().get< tags::by_author_posts_tag >();
   auto author_tag_itr = author_tag_idx.lower_bound( author );
   vector< pair< tag_name_type, uint32_t > > results;

   while( author_tag_itr != author_tag_idx.end() && 
      author_tag_itr->author == account_ptr->name && 
      results.size() < 1000 )
   {
      results.push_back( std::make_pair( author_tag_itr->tag, author_tag_itr->total_posts ) );
      ++author_tag_itr;
   }
   return results;
}

vector< tag_api_obj > database_api::get_top_tags( string after, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_top_tags( after, limit );
   });
}

vector< tag_api_obj > database_api_impl::get_top_tags( string after, uint32_t limit )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< tag_api_obj >();
   }
   
   vector< tag_api_obj > results;
   limit = std::min( limit, uint32_t( 1000 ) );
   results.reserve( limit );

   const auto& nidx = _db.get_index< tags::tag_stats_index >().indices().get< tags::by_tag >();
   const auto& ridx = _db.get_index< tags::tag_stats_index >().indices().get< tags::by_vote_power >();

   auto itr = ridx.begin();
   if( after != "" && nidx.size() )
   {
      auto nitr = nidx.lower_bound( after );
      if( nitr == nidx.end() )
      {
         itr = ridx.end();
      } 
      else
      {
         itr = ridx.iterator_to( *nitr );
      } 
   }

   while( itr != ridx.end() && results.size() < limit )
   {
      results.push_back( tag_api_obj( *itr ) );
      ++itr;
   }
   return results;
}



} } // node::app