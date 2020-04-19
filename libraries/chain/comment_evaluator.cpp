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


//=====================================//
// === Post and Comment Evaluators === //
//=====================================//

/**
 * POST TYPES
 * 
 * TEXT_POST,        ///< A post containing a maxmium of 300 characters of text.
 * IMAGE_POST,       ///< A post containing an IPFS media file of an image, and up to 1000 characters of description text.
 * VIDEO_POST,       ///< A post containing a title, an IPFS media file or bittorrent magent link of a video, and up to 1000 characters of description tex.
 * LINK_POST,        ///< A post containing a URL link, link title, and up to 1000 characters of description text.
 * ARTICLE_POST,     ///< A post containing a title, a header image, and an unlimited amount of body text with embedded images.
 * AUDIO_POST,       ///< A post containing a title, an IPFS link to an audio file, and up to 1000 characters of description text.
 * FILE_POST,        ///< A post containing a title, either an IPFS link to a file, or a magnet link to a bittorrent swarm for a file, and up to 1000 characters of description text.
 * POLL_POST,        ///< A post containing a title, at least 2 voting options, and up to 1000 characters of description text.
 * LIVESTREAM_POST,  ///< A post containing a title, a link to a livestreaming video, and up to 1000 characters of description text.
 * PRODUCT_POST,     ///< A post containing a product title, at least 1 IPFS image of the product, and purchase details to create an escrow order.
 * LIST_POST         ///< A post containing a list of at least 2 other posts, a title, and up to 1000 characters of description text
 */


/**
 * Creates a new comment object, or edits an existing comments.
 * 
 * Comments made within a community must be created 
 * in a community that the user has the 
 * permission to author a post in. 
*/
void comment_evaluator::do_apply( const comment_operation& o )
{ try {
   const account_name_type& signed_for = o.author;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   time_point now = _db.head_block_time();
   const auto& by_permlink_idx = _db.get_index< comment_index >().indices().get< by_permlink >();
   auto itr = by_permlink_idx.find( boost::make_tuple( o.author, o.permlink ) );
   const account_object& auth = _db.get_account( o.author );
   comment_options options = o.options;

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }

   feed_reach_type reach_type = feed_reach_type::FOLLOW_FEED;

   for( size_t i = 0; i < feed_reach_values.size(); i++ )
   {
      if( options.reach == feed_reach_values[ i ] )
      {
         reach_type = feed_reach_type( i );
         break;
      }
   }

   post_format_type format_type = post_format_type::TEXT_POST;

   for( size_t i = 0; i < post_format_values.size(); i++ )
   {
      if( options.post_type == post_format_values[ i ] )
      {
         format_type = post_format_type( i );
         break;
      }
   }

   connection_tier_type reply_connection = connection_tier_type::PUBLIC;

   for( size_t i = 0; i < connection_tier_values.size(); i++ )
   {
      if( options.reply_connection == connection_tier_values[ i ] )
      {
         reply_connection = connection_tier_type( i );
         break;
      }
   }
   
   const community_object* community_ptr = nullptr;

   if( o.community.size() )     // Community validity and permissioning checks
   {
      community_ptr = _db.find_community( o.community );

      FC_ASSERT( community_ptr != nullptr, 
         "Community Name: ${b} not found.", ("b", o.community ));
      const community_object& community = *community_ptr;
      feed_reach_type community_feed_type = feed_reach_type::COMMUNITY_FEED;
      const community_member_object& community_member = _db.get_community_member( community.name );

      if( o.parent_author == ROOT_POST_PARENT )
      {
         FC_ASSERT( community_member.is_authorized_author( o.author ), 
            "User ${u} is not authorized to post in the community ${b}.",("b", community.name)("u", auth.name));
      }
      else
      {
         FC_ASSERT( community_member.is_authorized_interact( o.author ), 
            "User ${u} is not authorized to interact with posts in the community ${b}.",("b", community.name)("u", auth.name));
      }
      
      switch( community.community_privacy )
      {
         case community_privacy_type::OPEN_PUBLIC_COMMUNITY:
         case community_privacy_type::GENERAL_PUBLIC_COMMUNITY:
         case community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY:
         case community_privacy_type::CLOSED_PUBLIC_COMMUNITY:
         {
            FC_ASSERT( public_key_type( o.public_key ) != public_key_type(), 
               "Posts in Open and Public communities should not be encrypted." );
         }
         case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
         case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
         case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
         case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
         {
            FC_ASSERT( public_key_type( o.public_key ) == community.community_public_key, 
               "Posts in Private and Exclusive Communities must be encrypted with the community public key.");
            FC_ASSERT( reach_type == community_feed_type, 
               "Posts in Private and Exclusive Communities should have reach limited to only community level subscribers.");
         }
         break;
         default:
         {
            FC_ASSERT( false, "Invalid Community Privacy type." );
         }
         break;
      }

      FC_ASSERT( o.options.rating <= community.max_rating, 
         "Post rating exceeds maximum rating of community." );
      FC_ASSERT( o.options.reward_currency == community.reward_currency ||
         o.options.reward_currency == SYMBOL_COIN, 
         "Community does not accept specified reward currency: ${c}.", ("c", o.options.reward_currency ));
   }

   switch( reach_type )
   {
      case feed_reach_type::TAG_FEED:
      case feed_reach_type::FOLLOW_FEED:
      case feed_reach_type::MUTUAL_FEED:
      {
         FC_ASSERT( public_key_type( o.public_key ) == public_key_type(), 
            "Follow, Mutual and Tag level posts should not be encrypted." );
      }
      break;
      case feed_reach_type::CONNECTION_FEED:
      {
         FC_ASSERT( public_key_type( o.public_key ) == auth.connection_public_key, 
            "Connection level posts must be encrypted with the account's Connection public key." );
      }
      break;
      case feed_reach_type::FRIEND_FEED:
      {
         FC_ASSERT( public_key_type( o.public_key ) == auth.friend_public_key, 
            "Connection level posts must be encrypted with the account's Friend public key.");
      }
      break;
      case feed_reach_type::COMPANION_FEED:
      {
         FC_ASSERT( public_key_type( o.public_key ) == auth.companion_public_key, 
            "Connection level posts must be encrypted with the account's Companion public key.");
      }
      break;
      case feed_reach_type::COMMUNITY_FEED:
      {
         FC_ASSERT( community_ptr != nullptr, 
            "Community level posts must be made within a valid community.");
         FC_ASSERT( public_key_type( o.public_key ) == community_ptr->community_public_key, 
            "Community level posts must be encrypted with the community public key.");
      }
      break;
      case feed_reach_type::NO_FEED:
      break;
      default:
      {
         FC_ASSERT( false, "Invalid Post Reach Type." );
      }
   }

   switch( format_type )
   {
      case post_format_type::TEXT_POST:
      {
         FC_ASSERT( o.body.size() <= MAX_TEXT_POST_LENGTH,
            "Comment rejected: Body size is too large for text post, maximum of 300 characters." );
         FC_ASSERT( o.title.size() == 0,
            "Comment rejected: Should not include title in text post." );
         if( community_ptr != nullptr )
         {
            FC_ASSERT( community_ptr->enable_text_posts(), 
               "Community Name: ${b} does not enable Text Posts.", ("b", o.community ));
         }
      }
      break;
      case post_format_type::IMAGE_POST:
      {
         FC_ASSERT( o.ipfs.size() >= 1,
            "Comment rejected: Image post must contain at least one IPFS referenced image file." );
         if( community_ptr != nullptr )
         {
            FC_ASSERT( community_ptr->enable_image_posts(), 
               "Community Name: ${b} does not enable Image Posts.", ("b", o.community ));
         }
      }
      break;
      case post_format_type::VIDEO_POST:
      {
         FC_ASSERT( o.magnet.size() >= 1 || o.ipfs.size() >= 1,
            "Comment rejected: Video post must contain at least one IPFS or magnet referenced video file." );
         FC_ASSERT( o.title.size() >=1,
            "Comment rejected: Should include title in video post." );
         if( community_ptr != nullptr )
         {
            FC_ASSERT( community_ptr->enable_video_posts(), 
               "Community Name: ${b} does not enable Video Posts.", ("b", o.community ));
         }
      }
      break;
      case post_format_type::LINK_POST:
      {
         FC_ASSERT( o.url.size() >=1,
            "Comment rejected: Link post must contain at least a valid url link." );
         FC_ASSERT( o.title.size() >=1,
            "Comment rejected: Should include title in link post." );
         if( community_ptr != nullptr )
         {
            FC_ASSERT( community_ptr->enable_link_posts(), 
               "Community Name: ${b} does not enable Link Posts.", ("b", o.community ));
         }
      }
      break;
      case post_format_type::ARTICLE_POST:
      {
         FC_ASSERT( o.body.size() >=1,
            "Comment rejected: Article post must include body." );
         FC_ASSERT( o.title.size() >=1,
            "Comment rejected: Article post must include title." );
         if( community_ptr != nullptr )
         {
            FC_ASSERT( community_ptr->enable_article_posts(), 
               "Community Name: ${b} does not enable Article Posts.", ("b", o.community ));
         }
      }
      break;
      case post_format_type::AUDIO_POST:
      {
         FC_ASSERT( o.ipfs.size() >= 1,
            "Comment rejected: Audio post must contain at least one IPFS referenced audio file." );
         FC_ASSERT( o.title.size() >= 1,
            "Comment rejected: Audio post must contain title." );
         if( community_ptr != nullptr )
         {
            FC_ASSERT( community_ptr->enable_audio_posts(), 
               "Community Name: ${b} does not enable Audio Posts.", ("b", o.community ));
         }
      }
      break;
      case post_format_type::FILE_POST:
      {
         FC_ASSERT( o.magnet.size() >= 1 || o.ipfs.size() >= 1,
            "Comment rejected: File post must contain at least one IPFS or Magnet referenced file." );
         FC_ASSERT( o.title.size() >= 1,
            "Comment rejected: File post must contain title." );
         if( community_ptr != nullptr )
         {
            FC_ASSERT( community_ptr->enable_file_posts(), 
               "Community Name: ${b} does not enable File Posts.", ("b", o.community ));
         }
      }
      break;
      case post_format_type::POLL_POST:
      {
         if( community_ptr != nullptr )
         {
            FC_ASSERT( community_ptr->enable_poll_posts(), 
               "Community Name: ${b} does not enable Poll Posts.", ("b", o.community ));
         }
      }
      break;
      case post_format_type::LIVESTREAM_POST:
      {
         if( community_ptr != nullptr )
         {
            FC_ASSERT( community_ptr->enable_livestream_posts(), 
               "Community Name: ${b} does not enable Livestream Posts.", ("b", o.community ));
         }
      }
      break;
      case post_format_type::PRODUCT_POST:
      {
         if( community_ptr != nullptr )
         {
            FC_ASSERT( community_ptr->enable_product_posts(), 
               "Community Name: ${b} does not enable Product Posts.", ("b", o.community ));
         }
      }
      break;
      case post_format_type::LIST_POST:
      {
         if( community_ptr != nullptr )
         {
            FC_ASSERT( community_ptr->enable_list_posts(), 
               "Community Name: ${b} does not enable List Posts.", ("b", o.community ));
         }
      }
      break;
      default:
      {
         FC_ASSERT( false, "Comment rejected: Invalid post type." );
      }
   }

   for( auto& b : options.beneficiaries )
   {
      const account_object* acc = _db.find< account_object, by_name >( b.account );
      FC_ASSERT( acc != nullptr,
         "Beneficiary \"${a}\" must exist.", ("a", b.account) );
   }
   
   share_type reward = 0;
   uint128_t weight = 0;
   uint128_t max_weight = 0;
   uint16_t new_commenting_power = auth.commenting_power;

   comment_id_type id;

   const comment_object* parent = nullptr;
   if( o.parent_author != ROOT_POST_PARENT )
   {
      parent = &_db.get_comment( o.parent_author, o.parent_permlink );
      FC_ASSERT( parent->depth < MAX_COMMENT_DEPTH, 
         "Comment is nested ${x} posts deep, maximum depth is ${y}.", ("x",parent->depth)("y",MAX_COMMENT_DEPTH) );
   }
      
   if ( itr == by_permlink_idx.end() )             // Post does not yet exist, creating new post
   {
      if( o.parent_author == ROOT_POST_PARENT )       // Post is a new root post
      {
         FC_ASSERT( ( now - auth.last_root_post ) > MIN_ROOT_COMMENT_INTERVAL, 
            "You may only post once every 60 seconds.", ("now",now)("last_root_post", auth.last_root_post) );
      }    
      else         // Post is a new comment
      {
         const comment_object& root = _db.get( parent->root_comment );       // If root post, gets the posts own object.
         const account_object& root_auth = _db.get_account( root.author );

         FC_ASSERT( root.allow_replies, 
            "The parent comment has disabled replies." );
         FC_ASSERT( ( now - auth.last_post ) > MIN_REPLY_INTERVAL, 
            "You may only comment once every 15 seconds.", ("now",now)("auth.last_post",auth.last_post) );

         // If root charges a comment price, pay the comment price to root author.

         if( root.comment_price.amount > 0 && !parent->comment_paid( o.author ) )
         {
            _db.adjust_liquid_balance( o.author, -root.comment_price );
            _db.adjust_liquid_balance( root.author, root.comment_price );

            _db.modify( root, [&]( comment_object& co )
            {
               if( co.payments_received[ o.author ].size() )
               {
                  if( co.payments_received[ o.author ][ root.comment_price.symbol ].amount > 0 )
                  {
                     co.payments_received[ o.author ][ root.comment_price.symbol ] += root.comment_price;
                  }
                  else
                  {
                     co.payments_received[ o.author ][ root.comment_price.symbol ] = root.comment_price;
                  }
               }
               else
               {
                  co.payments_received[ o.author ][ root.comment_price.symbol ] = root.comment_price;
               }
            });
         }

         // If parent comment pays a reply price, then transfer from parent to root author.

         if( parent->reply_price.amount > 0 && o.author == root.author )
         {
            asset liquid = _db.get_liquid_balance( parent->author, parent->reply_price.symbol );

            if( liquid >= parent->reply_price )
            {
               _db.adjust_liquid_balance( parent->author, -parent->reply_price );
               _db.adjust_liquid_balance( root.author, parent->reply_price );
            }
         }

         account_name_type account_a_name;
         account_name_type account_b_name;

         if( auth.id < root_auth.id )        // Connection objects are sorted with lowest ID is account A. 
         {
            account_a_name = auth.name;
            account_b_name = root_auth.name;
         }
         else
         {
            account_b_name = auth.name;
            account_a_name = root_auth.name;
         }

         const auto& connection_idx = _db.get_index< connection_index >().indices().get< by_accounts >();

         switch( reply_connection )
         {
            case connection_tier_type::PUBLIC:
            {
               // Anyone can comment
            }
            break;
            case connection_tier_type::CONNECTION:
            {
               auto con_itr = connection_idx.find( boost::make_tuple( account_a_name, account_b_name, reply_connection ) );
               FC_ASSERT( con_itr != connection_idx.end(), 
                  "Cannot create reply: No Connection between Account: ${a} and Account: ${b}", ("a", account_a_name)("b", account_b_name) );
            }
            break;
            case connection_tier_type::FRIEND:
            {
               auto con_itr = connection_idx.find( boost::make_tuple( account_a_name, account_b_name, reply_connection ) );
               FC_ASSERT( con_itr != connection_idx.end(), 
                  "Cannot create reply: No Friend Connection between Account: ${a} and Account: ${b}", ("a", account_a_name)("b", account_b_name) );
            }
            break;
            case connection_tier_type::COMPANION:
            {
               auto con_itr = connection_idx.find( boost::make_tuple( account_a_name, account_b_name, reply_connection ) );
               FC_ASSERT( con_itr != connection_idx.end(), 
                  "Cannot create reply: No Companion Connection between Account: ${a} and Account: ${b}", ("a", account_a_name)("b", account_b_name) );
            }
            break;
            case connection_tier_type::SECURE:
            {
               FC_ASSERT( false, 
                  "Cannot create reply: Post is set to Close replies.");
            }
            break;
            default:
            {
               FC_ASSERT( false, 
                  "Invalid connection type." );
            }
         }

         const reward_fund_object& reward_fund = _db.get_reward_fund( root.reward_currency );
         auto curve = reward_fund.curation_reward_curve;
         int64_t elapsed_seconds = ( now - auth.last_post ).to_seconds();
         int16_t regenerated_power = (PERCENT_100 * elapsed_seconds) / median_props.comment_recharge_time.to_seconds();
         int16_t current_power = std::min( int64_t( auth.commenting_power + regenerated_power), int64_t(PERCENT_100) );
         FC_ASSERT( current_power > 0, 
            "Account currently does not have any commenting power." );
         share_type voting_power = _db.get_voting_power( auth.name );
         int16_t max_comment_denom = median_props.comment_reserve_rate * ( median_props.comment_recharge_time.count() / fc::days(1).count() );  // Weights the viewing power with the network reserve ratio and recharge time
         FC_ASSERT( max_comment_denom > 0 );
         int16_t used_power = (current_power + max_comment_denom - 1) / max_comment_denom;
         new_commenting_power = current_power - used_power;
         FC_ASSERT( used_power <= current_power, 
            "Account does not have enough power to comment." );
         
         reward = ( voting_power.value * used_power) / PERCENT_100;
         
         uint128_t old_power = std::max( uint128_t( root.comment_power.value ), uint128_t(0) );  // Record comment value before applying comment

         _db.modify( root, [&]( comment_object& c )
         {
            c.net_reward += reward;
            c.comment_power += reward;
         });

         uint128_t new_power = std::max( uint128_t( root.comment_power.value ), uint128_t(0) );   // record new net reward after applying comment
         bool curation_reward_eligible = reward > 0 && root.cashout_time != fc::time_point::maximum() && root.allow_curation_rewards;
            
         if( curation_reward_eligible )
         {
            uint128_t old_weight = util::evaluate_reward_curve( 
               old_power, 
               root.cashouts_received, 
               curve, 
               median_props.content_reward_decay_rate, 
               reward_fund.content_constant
            );
            uint128_t new_weight = util::evaluate_reward_curve(
               new_power,
               root.cashouts_received, 
               curve, 
               median_props.content_reward_decay_rate,
               reward_fund.content_constant
               );       
            uint128_t max_comment_weight = new_weight - old_weight;   // Gets the difference in content reward weight before and after the comment occurs.

            _db.modify( root, [&]( comment_object& c )
            {
               c.total_comment_weight += max_comment_weight;
            });

            uint128_t curation_auction_decay_time = median_props.curation_auction_decay_time.to_seconds();
            uint128_t w = max_comment_weight;
            uint128_t delta_t = std::min( uint128_t( ( now - root.created ).to_seconds()), curation_auction_decay_time );

            w *= delta_t;
            w /= curation_auction_decay_time;                     // Discount weight linearly by time for early comments in the first 10 minutes.

            double curation_decay = median_props.comment_curation_decay;      // Number of comments for half life of curation reward decay.
            double comment_discount_rate = std::max(( double( root.children ) / curation_decay), double(0));
            double comment_discount = std::pow(0.5, comment_discount_rate );     // Raises 0.5 to a fractional power for each 100 comments added
            double comment_discount_percent = comment_discount * double(PERCENT_100);
            FC_ASSERT( comment_discount_percent >= 0,
               "Comment discount percent should not become negative." );
            w *= uint128_t( int(comment_discount_percent) );
            w /= PERCENT_100;      // Discount weight exponentially for each successive comment on the post, decaying by 50% per 100 comments.
            weight = w;
            max_weight = max_comment_weight;
         } 
      }

      _db.modify( auth, [&]( account_object& a )
      {
         if( o.parent_author == ROOT_POST_PARENT )
         {
            a.last_root_post = now;
            a.post_count++;
         }
         else
         {
            a.commenting_power = new_commenting_power;
            a.comment_count++;
         }
         a.last_post = now;
      });

      const comment_object& new_comment = _db.create< comment_object >( [&]( comment_object& com )
      {
         validate_permlink( o.parent_permlink );
         validate_permlink( o.permlink );
         
         com.author = o.author;
         com.rating = options.rating;
         com.community = o.community;
         com.reach = reach_type;
         com.reply_connection = reply_connection;
         com.post_type = format_type;
         com.author_reputation = auth.author_reputation;
         com.comment_price = o.comment_price;
         com.premium_price = o.premium_price;
         com.public_key = public_key_type( o.public_key );
         
         if( o.interface.size() )
         {
            com.interface = o.interface;
         }

         com.ipfs.reserve( o.ipfs.size() );
         for( size_t i = 0; i < o.ipfs.size(); i++ )
         {
            from_string( com.ipfs[ i ], o.ipfs[ i ] );
         }

         com.magnet.reserve( o.magnet.size() );
         for( size_t i = 0; i < o.magnet.size(); i++ )
         {
            from_string( com.magnet[ i ], o.magnet[ i ] );
         }

         for( auto& tag : o.tags )
         {
            com.tags.push_back( tag );
         }
         for( auto& b : options.beneficiaries )
         {
            com.beneficiaries.push_back( b );
         }

         from_string( com.language, o.language );
         from_string( com.permlink, o.permlink );
         from_string( com.body, o.body );
         from_string( com.json, o.json );
         from_string( com.url, o.url );

         com.reward_currency = options.reward_currency;
         com.max_accepted_payout = options.max_accepted_payout;
         com.percent_liquid = options.percent_liquid;
         com.allow_replies = options.allow_replies;
         com.allow_votes = options.allow_votes;
         com.allow_views = options.allow_views;
         com.allow_shares = options.allow_shares;
         com.allow_curation_rewards = options.allow_curation_rewards;

         com.last_updated = now;
         com.created = now;
         com.active = now;
         com.last_payout = fc::time_point::min();

         com.cashout_time = com.created + median_props.content_reward_interval;
         com.author_reward_percent = median_props.author_reward_percent;
         com.vote_reward_percent = median_props.vote_reward_percent;
         com.view_reward_percent = median_props.view_reward_percent;
         com.share_reward_percent = median_props.share_reward_percent;
         com.comment_reward_percent = median_props.comment_reward_percent;
         com.storage_reward_percent = median_props.storage_reward_percent;
         com.moderator_reward_percent = median_props.moderator_reward_percent;

         if ( o.parent_author == ROOT_POST_PARENT )     // New Root post
         {
            com.parent_author = "";
            from_string( com.parent_permlink, o.parent_permlink );
            from_string( com.category, o.parent_permlink );
            com.root_comment = com.id;
         }
         else       // New comment
         {
            com.parent_author = parent->author;
            com.parent_permlink = parent->permlink;
            com.depth = parent->depth + 1;
            com.category = parent->category;
            com.root_comment = parent->root_comment;
            com.reward = reward;
            com.weight = weight;
            com.max_weight = max_weight;
         }
      });

      id = new_comment.id;

      while( parent != nullptr )        // Increments the children counter on all ancestor comments, and bumps active time.
      {
         _db.modify( *parent, [&]( comment_object& p )
         {
            p.children++;
            p.active = now;
         });

         if( parent->parent_author != ROOT_POST_PARENT )
         {
            parent = &_db.get_comment( parent->parent_author, parent->parent_permlink );
         }
         else
         {
            parent = nullptr;
         }
      }

      // Create feed and blog objects for author account's followers and connections, community followers, and tag followers. 
      _db.add_comment_to_feeds( new_comment );

      if( community_ptr != nullptr )
      {
         _db.modify( *community_ptr, [&]( community_object& bo )
         {
            if ( o.parent_author == ROOT_POST_PARENT )
            {
               bo.post_count++;
               bo.last_root_post = now;
            }
            else 
            {
               bo.comment_count++;
            }
            bo.last_post = now;
         });
      }  
   }
   else           // Post found, editing or deleting existing post.
   {
      const comment_object& comment = *itr;

      if( options.beneficiaries.size() )
      {
         FC_ASSERT( comment.beneficiaries.size() == 0,
            "Comment already has beneficiaries specified." );
         FC_ASSERT( comment.net_reward == 0,
            "Comment must not have been voted on before specifying beneficiaries." );
      }
      
      FC_ASSERT( comment.allow_curation_rewards >= options.allow_curation_rewards,
         "Curation rewards cannot be re-enabled." );
      FC_ASSERT( comment.allow_replies >= options.allow_replies,
         "Replies cannot be re-enabled." );
      FC_ASSERT( comment.allow_votes >= options.allow_votes,
         "Voting cannot be re-enabled." );
      FC_ASSERT( comment.allow_views >= options.allow_views,
         "Viewing cannot be re-enabled." );
      FC_ASSERT( comment.allow_shares >= options.allow_shares,
         "Shares cannot be re-enabled." );
      FC_ASSERT( comment.max_accepted_payout >= options.max_accepted_payout,
         "A comment cannot accept a greater payout." );
      FC_ASSERT( comment.percent_liquid >= options.percent_liquid,
         "A comment cannot accept a greater percent USD." );

      if( !o.deleted )     // Editing post
      {
         feed_reach_type old_reach = comment.reach;

         _db.modify( comment, [&]( comment_object& com )
         {
            com.last_updated = now;
            com.active = now;
            com.rating = options.rating;
            com.community = o.community;
            com.reach = reach_type;
            com.reply_connection = reply_connection;
            com.reward_currency = options.reward_currency;
            com.max_accepted_payout = options.max_accepted_payout;
            com.percent_liquid = options.percent_liquid;
            com.allow_replies = options.allow_replies;
            com.allow_votes = options.allow_votes;
            com.allow_views = options.allow_views;
            com.allow_shares = options.allow_shares;
            com.allow_curation_rewards = options.allow_curation_rewards;

            strcmp_equal equal;

            if( !parent )
            {
               FC_ASSERT( com.parent_author == account_name_type(), 
                  "The parent of a comment cannot change." );
               FC_ASSERT( equal( com.parent_permlink, o.parent_permlink ), 
                  "The permlink of a comment cannot change." );
            }
            else
            {
               FC_ASSERT( com.parent_author == o.parent_author, 
                  "The parent of a comment cannot change." );
               FC_ASSERT( equal( com.parent_permlink, o.parent_permlink ), 
                  "The permlink of a comment cannot change." );
            }       
            if( o.json.size() && fc::is_utf8( o.json ) )
            {
               from_string( com.json, o.json );  
            }
            if( o.url.size() && fc::is_utf8( o.url ) )
            {
               from_string( com.url, o.url );  
            }
            com.ipfs.reserve( o.ipfs.size() );
            for( size_t i = 0; i < o.ipfs.size(); i++ )
            {
               from_string( com.ipfs[ i ], o.ipfs[ i ] );
            }
            
            com.magnet.reserve( o.magnet.size() );
            for( size_t i = 0; i < o.magnet.size(); i++ )
            {
               from_string( com.magnet[ i ], o.magnet[ i ] );
            }
            for( auto& tag : o.tags )
            {
               com.tags.push_back( tag );
            }
            if( o.language.size() ) 
            {
               from_string( com.language, o.language );
            }
            if( o.public_key.size() ) 
            {
               com.public_key = public_key_type( o.public_key );
            }
            if( o.body.size() )
            {  
               if( !fc::is_utf8( o.body ) )
               {
                  idump(("invalid utf8")(o.body));
                  from_string( com.body, fc::prune_invalid_utf8( o.body ) );
               } 
               else 
               { 
                  from_string( com.body, o.body ); 
               }
            }
         });

         if( comment.reach != old_reach )    // If reach has changed, recreate feed objects for author account's followers and connections.
         {
            _db.clear_comment_feeds( comment );
            _db.add_comment_to_feeds( comment );
         } 
      } 
      else
      {
         _db.modify( comment, [&]( comment_object& com )
         {
            com.deleted = true;               // deletes comment, nullifying all possible information.
            com.last_updated = fc::time_point::min();
            com.active = fc::time_point::min();
            com.rating = 1;
            com.community = community_name_type();
            com.reach = feed_reach_type::NO_FEED;
            from_string( com.json, "" );
            from_string( com.url, "" );
            com.ipfs.clear();
            com.magnet.clear();
            from_string( com.language, "" );
            com.public_key = public_key_type();
            from_string( com.body, "" );
         });
        
         _db.clear_comment_feeds( comment );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


struct comment_options_extension_visitor
{
   comment_options_extension_visitor( const comment_object& c, database& db ) : _c( c ), _db( db ) {}
   typedef void result_type;
   const comment_object& _c;
   database& _db;

   void operator()( const comment_payout_beneficiaries& cpb ) const
   {
      FC_ASSERT( _c.beneficiaries.size() == 0,
         "Comment already has beneficiaries specified." );
      FC_ASSERT( _c.net_reward == 0,
         "Comment must not have been voted on before specifying beneficiaries." );

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


void message_evaluator::do_apply( const message_operation& o )
{ try {
   const account_name_type& signed_for = o.sender;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& sender = _db.get_account( o.sender );
   const account_object& recipient = _db.get_account( o.recipient );
   time_point now = _db.head_block_time();
   
   account_name_type account_a_name;
   account_name_type account_b_name;

   if( sender.id < recipient.id )        // Connection objects are sorted with lowest ID is account A. 
   {
      account_a_name = sender.name;
      account_b_name = recipient.name;
   }
   else
   {
      account_b_name = sender.name;
      account_a_name = recipient.name;
   }

   const auto& connection_idx = _db.get_index< connection_index >().indices().get< by_accounts >();
   auto connection_itr = connection_idx.find( boost::make_tuple( account_a_name, account_b_name, connection_tier_type::CONNECTION ) );

   FC_ASSERT( connection_itr != connection_idx.end(), 
      "Cannot send message: No Connection between Account: ${a} and Account: ${b}", ("a", account_a_name)("b", account_b_name) );

   const auto& message_idx = _db.get_index< message_index >().indices().get< by_sender_uuid >();
   auto message_itr = message_idx.find( boost::make_tuple( sender.name, o.uuid ) );

   if( message_itr == message_idx.end() )         // Message uuid does not exist, creating new message
   {
      _db.create< message_object >( [&]( message_object& mo )
      {
         mo.sender = sender.name;
         mo.recipient = recipient.name;
         mo.sender_public_key = sender.secure_public_key;
         mo.recipient_public_key = recipient.secure_public_key;
         from_string( mo.message, o.message );
         from_string( mo.uuid, o.uuid );
         mo.created = now;
         mo.last_updated = now;
      });

      _db.modify( *connection_itr, [&]( connection_object& co )
      {
         if( account_a_name == sender.name )
         {
            co.last_message_time_a = now;
         }
         else if( account_b_name == sender.name )
         {
            co.last_message_time_b = now;
         }
         co.message_count++;
      });
   }
   else
   {
      _db.modify( *message_itr, [&]( message_object& mo )
      {
         from_string( mo.message, o.message );
         mo.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void vote_evaluator::do_apply( const vote_operation& o )
{ try {
   const account_name_type& signed_for = o.voter;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const comment_object& comment = _db.get_comment( o.author, o.permlink );
   const account_object& voter = _db.get_account( o.voter );
   time_point now = _db.head_block_time();
   const median_chain_property_object& median_props = _db.get_median_chain_properties();

   FC_ASSERT( voter.can_vote, 
      "Voter has declined their voting rights." );
   FC_ASSERT( comment.allow_votes, 
      "Votes are not allowed on the comment." );

   const community_object* community_ptr = nullptr;
   if( comment.community.size() )
   {
      community_ptr = _db.find_community( comment.community );     
      const community_member_object& community_member = _db.get_community_member( comment.community );
      FC_ASSERT( community_member.is_authorized_interact( voter.name ), 
         "User ${u} is not authorized to interact with posts in the community ${b}.",("b", comment.community)("u", voter.name));
   }

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }

   const reward_fund_object& reward_fund = _db.get_reward_fund( comment.reward_currency );
   auto curve = reward_fund.curation_reward_curve;

   const auto& comment_vote_idx = _db.get_index< comment_vote_index >().indices().get< by_comment_voter >();
   auto itr = comment_vote_idx.find( std::make_tuple( comment.id, voter.name ) );

   int64_t elapsed_seconds = (now - voter.last_vote_time).to_seconds();
   FC_ASSERT( elapsed_seconds >= MIN_VOTE_INTERVAL.to_seconds(), 
      "Can only vote once every ${s} seconds.", ("s", MIN_VOTE_INTERVAL) );
   int16_t regenerated_power = (PERCENT_100 * elapsed_seconds) / median_props.vote_recharge_time.to_seconds();
   int16_t current_power = std::min( int64_t(voter.voting_power + regenerated_power), int64_t(PERCENT_100) );
   
   FC_ASSERT( current_power > 0,
      "Account currently does not have voting power." );
   int16_t abs_weight = abs(o.weight);
   int16_t used_power = (current_power * abs_weight) / PERCENT_100;
   int16_t max_vote_denom = median_props.vote_reserve_rate * ( median_props.vote_recharge_time.count() / fc::days(1).count() );
   
   FC_ASSERT( max_vote_denom > 0 );
   used_power = ( used_power + max_vote_denom - 1 ) / max_vote_denom;
   FC_ASSERT( used_power <= current_power, 
      "Account does not have enough power to vote." );

   share_type voting_power = _db.get_voting_power( o.voter );    // Gets the user's voting power from their Equity and Staked coin balances
   share_type abs_reward = ( voting_power * used_power ) / PERCENT_100;
   FC_ASSERT( abs_reward > 0 || o.weight == 0, 
      "Voting weight is too small, please accumulate more voting power." );
   share_type reward = o.weight < 0 ? -abs_reward : abs_reward; // Determines the sign of abs_reward for upvote and downvote

   const comment_object& root = _db.get( comment.root_comment );

   if( itr == comment_vote_idx.end() )   // New vote is being added to emtpy index
   {
      FC_ASSERT( o.weight != 0, 
         "Vote weight cannot be 0 for the first vote.");
      FC_ASSERT( abs_reward > 0, 
         "Cannot vote with 0 reward." );

      _db.modify( voter, [&]( account_object& a )
      {
         a.voting_power = current_power - used_power;
         a.last_vote_time = now;
         a.post_vote_count++;
      });

      uint128_t old_power = std::max( uint128_t(comment.vote_power.value ), uint128_t(0));

      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_reward += reward;
         c.vote_power += reward;
         if( reward > 0 )
         {
            c.net_votes++;
         }   
         else
         {
            c.net_votes--;
         } 
      });

      uint128_t new_power = std::max( uint128_t(comment.vote_power.value ), uint128_t(0));

      /** this verifies uniqueness of voter
       *
       *  cv.weight / c.total_vote_weight ==> % of reward increase that is accounted for by the vote
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
       */

      _db.create< comment_vote_object >( [&]( comment_vote_object& cv )
      {
         cv.voter = voter.name;
         cv.comment = comment.id;
         cv.reward = reward;
         cv.vote_percent = o.weight;
         cv.last_updated = now;
         if( o.interface.size() )
         {
            cv.interface = o.interface;
         }

         bool curation_reward_eligible = reward > 0 && comment.cashout_time != fc::time_point::maximum() && comment.allow_curation_rewards;
         
         if( curation_reward_eligible )
         {
            uint128_t old_weight = util::evaluate_reward_curve(
               old_power, 
               root.cashouts_received, 
               curve, 
               median_props.content_reward_decay_rate, 
               reward_fund.content_constant
            );
            uint128_t new_weight = util::evaluate_reward_curve(
               new_power,
               root.cashouts_received, 
               curve, 
               median_props.content_reward_decay_rate, 
               reward_fund.content_constant
            );

            uint128_t max_vote_weight = new_weight - old_weight;

            _db.modify( comment, [&]( comment_object& c )
            {
               c.total_vote_weight += max_vote_weight;       // Increase reward weight for curation rewards by maximum
            });

            uint128_t curation_auction_decay_time = median_props.curation_auction_decay_time.to_seconds();
            uint128_t w = max_vote_weight;
            uint128_t delta_t = std::min( uint128_t(( now - comment.created ).to_seconds()), curation_auction_decay_time ); 

            w *= delta_t;
            w /= curation_auction_decay_time;       // Discount weight linearly by time for early votes in the first 10 minutes

            double curation_decay = median_props.vote_curation_decay;
            double vote_discount_rate = std::max(( double(comment.net_votes) / curation_decay), double(0));
            double vote_discount = std::pow(0.5, vote_discount_rate );     // Raises 0.5 to a fractional power for each 100 net_votes added
            double vote_discount_percent = double(vote_discount) * double(PERCENT_100);
            FC_ASSERT( vote_discount_percent >= 0,
               "Vote discount should not become negative");
            w *= uint128_t( int( vote_discount_percent ) );
            w /= PERCENT_100; // Discount weight exponentially for each successive vote on the post, decaying by 50% per 100 votes.
            cv.max_weight = max_vote_weight;
            cv.weight = w;
         }
         else
         {
            cv.weight = 0;
         }
      });

      if( community_ptr != nullptr )
      {
         _db.modify( *community_ptr, [&]( community_object& bo )
         {
            if( reward > 0 )
            {
               bo.vote_count++;
            }
            else 
            {
               bo.vote_count--;
            }
         });
      }

      if( voter.membership == membership_tier_type::NONE )     // Check for the presence of an ad bid on this vote.
      {
         const auto& bid_idx = _db.get_index< ad_bid_index >().indices().get< by_provider_metric_author_objective_price >();
         auto bid_itr = bid_idx.lower_bound( std::make_tuple( o.interface, ad_metric_type::VOTE_METRIC, comment.author, comment.permlink ) );

         while( bid_itr != bid_idx.end() &&
            bid_itr->provider == o.interface &&
            bid_itr->metric == ad_metric_type::VOTE_METRIC &&
            bid_itr->author == comment.author &&
            bid_itr->objective == comment.permlink )    // Retrieves highest paying bids for this vote by this interface.
         {
            const ad_bid_object& bid = *bid_itr;
            const ad_audience_object& audience = _db.get_ad_audience( bid.bidder, bid.audience_id );

            if( !bid.is_delivered( o.voter ) && audience.is_audience( o.voter ) )
            {
               _db.deliver_ad_bid( bid, voter );
               break;
            }

            ++bid_itr;
         }
      }
   }
   else  // Vote is being altered from a previous vote
   {
      FC_ASSERT( itr->num_changes < MAX_VOTE_CHANGES, 
         "Voter has used the maximum number of vote changes on this comment." );
      FC_ASSERT( itr->vote_percent != o.weight, 
         "You have already voted in a similar way." );

      _db.modify( voter, [&]( account_object& a )
      {
         a.voting_power = current_power - used_power;     // Decrement Voting power by amount used
         a.last_vote_time = now;                          // Update last vote time
      });

      uint128_t old_power = std::max( uint128_t( comment.vote_power.value ), uint128_t(0));  // Record net reward before new vote is applied

      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_reward -= itr->reward;
         c.net_reward += reward;
         c.vote_power -= itr->reward;
         c.vote_power += reward;
         c.total_vote_weight -= itr->max_weight;     // deduct votes previous total weight contribution

         if( reward > 0 && itr->reward < 0 ) // Remove downvote and add upvote
            c.net_votes += 2;
         else if( reward > 0 && itr->reward == 0 ) // Add upvote from neutral vote
            c.net_votes += 1;
         else if( reward == 0 && itr->reward < 0 ) // Remove downvote and leave neutral
            c.net_votes += 1;
         else if( reward == 0 && itr->reward > 0 )  // Remove upvote and leave neutral
            c.net_votes -= 1;
         else if( reward < 0 && itr->reward == 0 ) // Add downvote from neutral vote
            c.net_votes -= 1;
         else if( reward < 0 && itr->reward > 0 ) // Remove Upvote and add downvote
            c.net_votes -= 2;
      });

      uint128_t new_power = std::max( uint128_t( comment.vote_power.value ), uint128_t(0));    // Record net reward after new vote is applied

      _db.modify( *itr, [&]( comment_vote_object& cv )
      {
         cv.reward = reward;
         cv.vote_percent = o.weight;
         cv.last_updated = now;
         cv.num_changes += 1;

         bool curation_reward_eligible = reward > 0 && comment.cashout_time != fc::time_point::maximum() && comment.allow_curation_rewards;
         
         if( curation_reward_eligible )
         {
            uint128_t old_weight = util::evaluate_reward_curve(
               old_power, 
               root.cashouts_received, 
               curve, 
               median_props.content_reward_decay_rate, 
               reward_fund.content_constant
            );
            uint128_t new_weight = util::evaluate_reward_curve(
               new_power,
               root.cashouts_received, 
               curve, 
               median_props.content_reward_decay_rate, 
               reward_fund.content_constant
            );

            uint128_t max_vote_weight = new_weight - old_weight;

            _db.modify( comment, [&]( comment_object& c )
            {
               c.total_vote_weight += max_vote_weight;    // Increase reward weight for curation rewards by maximum
            });

            uint128_t curation_auction_decay_time = median_props.curation_auction_decay_time.to_seconds();
            uint128_t w = max_vote_weight;
            uint128_t delta_t = std::min( uint128_t(( now - comment.created ).to_seconds()), curation_auction_decay_time ); 

            w *= delta_t;
            w /= curation_auction_decay_time;     // Discount weight linearly by time for early votes in the first 10 minutes

            double curation_decay = median_props.vote_curation_decay;
            double vote_discount_rate = std::max(( double(comment.net_votes) / curation_decay), double(0));
            double vote_discount = std::pow(0.5, vote_discount_rate );     // Raises 0.5 to a fractional power for each 100 net_votes added
            uint64_t vote_discount_percent = double(vote_discount) * double(PERCENT_100);
            FC_ASSERT( vote_discount_percent >= 0,
               "Vote discount should not become negative" );
            w *= vote_discount_percent;
            w /= PERCENT_100; // Discount weight exponentially for each successive vote on the post, decaying by 50% per 100 votes.
            cv.max_weight = max_vote_weight;
            cv.weight = w;
         }
         else
         {
            cv.weight = 0;
            cv.max_weight = 0;
         }
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void view_evaluator::do_apply( const view_operation& o )
{ try {
   const account_name_type& signed_for = o.viewer;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const comment_object& comment = _db.get_comment( o.author, o.permlink );
   const account_object& viewer = _db.get_account( o.viewer );
   share_type vp = _db.get_voting_power( viewer );         // Gets the user's voting power from their Equity and Staked coin balances to weight the view.
   FC_ASSERT( comment.allow_views,
      "Views are not allowed on the comment." );
   FC_ASSERT( viewer.can_vote,
      "Viewer has declined their voting rights." );
   const community_object* community_ptr = nullptr; 
   if( comment.community.size() )
   {
      community_ptr = _db.find_community( comment.community );      
      const community_member_object& community_member = _db.get_community_member( comment.community );       
      FC_ASSERT( community_member.is_authorized_interact( viewer.name ), 
         "User ${u} is not authorized to interact with posts in the community ${b}.",("b", comment.community)("u", viewer.name));
   }

   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   const reward_fund_object& reward_fund = _db.get_reward_fund( comment.reward_currency );
   auto curve = reward_fund.curation_reward_curve;

   const supernode_object* supernode_ptr = nullptr;
   const interface_object* interface_ptr = nullptr;

   if( o.supernode.size() )
   {
      supernode_ptr = _db.find_supernode( o.supernode );
   }
   if( o.interface.size() )
   {
      interface_ptr = _db.find_interface( o.interface );
   }

   time_point now = _db.head_block_time();

   const auto& comment_view_idx = _db.get_index< comment_view_index >().indices().get< by_comment_viewer >();
   auto itr = comment_view_idx.find( std::make_tuple( comment.id, viewer.name ) );
   int64_t elapsed_seconds = ( now - viewer.last_view_time ).to_seconds();

   FC_ASSERT( elapsed_seconds >= MIN_VIEW_INTERVAL.to_seconds(), 
      "Can only view once every ${s} seconds.", ("s", MIN_VIEW_INTERVAL) );
   int16_t regenerated_power = (PERCENT_100 * elapsed_seconds) / median_props.view_recharge_time.to_seconds();
   int16_t current_power = std::min( int64_t( viewer.viewing_power + regenerated_power ), int64_t(PERCENT_100) );

   FC_ASSERT( current_power > 0, 
      "Account currently does not have any viewing power." );

   int16_t max_view_denom = median_props.view_reserve_rate * ( median_props.view_recharge_time.count() / fc::days(1).count() );    // Weights the viewing power with the network reserve ratio and recharge time
   FC_ASSERT( max_view_denom > 0, 
      "View denominiator must be greater than zero.");
   int16_t used_power = (current_power + max_view_denom - 1) / max_view_denom;
   FC_ASSERT( used_power <= current_power, 
      "Account does not have enough power to view." );
   
   share_type reward = ( vp.value * used_power ) / PERCENT_100;
   const comment_object& root = _db.get( comment.root_comment );       // If root post, gets the posts own object.

   if( itr == comment_view_idx.end() )   // New view is being added 
   {
      FC_ASSERT( reward > 0, 
         "Cannot claim view with 0 reward." );
      FC_ASSERT( o.viewed, 
         "Viewed must be set to true to create new view, View does not exist to remove." );

      const auto& supernode_view_idx = _db.get_index< comment_view_index >().indices().get< by_supernode_viewer >();
      const auto& interface_view_idx = _db.get_index< comment_view_index >().indices().get< by_interface_viewer >();

      if( supernode_ptr != nullptr )
      {
         auto supernode_view_itr = supernode_view_idx.lower_bound( std::make_tuple( o.supernode, viewer.name ) );
         if( supernode_view_itr == supernode_view_idx.end() )   // No view exists
         {
            _db.adjust_view_weight( *supernode_ptr, vp, true );    // Adds voting power to the supernode view weight once per day per user. 
         }
         else if( ( supernode_view_itr->created + fc::days(1) ) < now )     // Latest view is more than 1 day old
         {
            _db.adjust_view_weight( *supernode_ptr, vp, true );    // Adds voting power to the supernode view weight once per day per user. 
         }
      }
      if( interface_ptr != nullptr )
      {
         auto interface_view_itr = interface_view_idx.lower_bound( std::make_tuple( o.interface, viewer.name ) );
         if( interface_view_itr == interface_view_idx.end() )   // No view exists
         {
            _db.adjust_interface_users( *interface_ptr, true );
         }
         else if( ( interface_view_itr->created + fc::days(1) ) < now )    // Latest View is more than 1 day old
         {
            _db.adjust_interface_users( *interface_ptr, true );
         }
      }

      _db.modify( viewer, [&]( account_object& a )
      {
         a.viewing_power = current_power - used_power;
         a.last_view_time = now;
      });

      uint128_t old_power = std::max( uint128_t( comment.view_power.value ), uint128_t(0) );  // Record reward value before applying view transaction

      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_reward += reward;
         c.view_power += reward;
         c.view_count++;
      });

      uint128_t new_power = std::max( uint128_t( comment.view_power.value ), uint128_t(0) );   // record new net reward after viewing

      if( community_ptr != nullptr )
      {
         _db.modify( *community_ptr, [&]( community_object& bo )
         {
            bo.view_count++;
         });
      }

      _db.create< comment_view_object >( [&]( comment_view_object& cv )
      {
         cv.viewer = viewer.name;
         cv.comment = comment.id;
         if( o.interface.size() )
         {
            cv.interface = o.interface;
         }
         if( o.supernode.size() )
         {
            cv.supernode = o.supernode;
         }
         cv.reward = reward;
         cv.created = now;

         bool curation_reward_eligible = reward > 0 && comment.cashout_time != fc::time_point::maximum() && comment.allow_curation_rewards;
            
         if( curation_reward_eligible )
         {
            uint128_t old_weight = util::evaluate_reward_curve(
               old_power, 
               root.cashouts_received, 
               curve, 
               median_props.content_reward_decay_rate, 
               reward_fund.content_constant
            );
            uint128_t new_weight = util::evaluate_reward_curve(
               new_power,
               root.cashouts_received, 
               curve, 
               median_props.content_reward_decay_rate, 
               reward_fund.content_constant
            );
            uint128_t max_view_weight = new_weight - old_weight;  // Gets the difference in content reward weight before and after the view occurs.

            _db.modify( comment, [&]( comment_object& c )
            {
               c.total_view_weight += max_view_weight;
            });

            uint128_t curation_auction_decay_time = median_props.curation_auction_decay_time.to_seconds();
            uint128_t w = max_view_weight;
            uint128_t delta_t = std::min( uint128_t(( now - comment.created).to_seconds()), curation_auction_decay_time );

            w *= delta_t;
            w /= curation_auction_decay_time;                     // Discount weight linearly by time for early views in the first 10 minutes.

            double curation_decay = median_props.view_curation_decay;      // Number of views for half life of curation reward decay.
            double view_discount_rate = std::max(( double(comment.view_count) / curation_decay), double(0));
            double view_discount = std::pow(0.5, view_discount_rate );     // Raises 0.5 to a fractional power for each 1000 views added
            uint64_t view_discount_percent = double(view_discount)*double(PERCENT_100);
            FC_ASSERT( view_discount_percent >= 0,
               "View discount should not become negative");
            w *= view_discount_percent;
            w /= PERCENT_100;      // Discount weight exponentially for each successive view on the post, decaying by 50% per 1000 views.
            cv.weight = w;
            cv.max_weight = max_view_weight;
         }
         else
         {
            cv.weight = 0;
            cv.max_weight = 0;
         }
      });

      if( viewer.membership == membership_tier_type::NONE )     // Check for the presence of an ad bid on this view.
      {
         const auto& bid_idx = _db.get_index< ad_bid_index >().indices().get< by_provider_metric_author_objective_price >();
         auto bid_itr = bid_idx.lower_bound( std::make_tuple( o.interface, ad_metric_type::VIEW_METRIC, comment.author, comment.permlink ) );

         while( bid_itr != bid_idx.end() &&
            bid_itr->provider == o.interface &&
            bid_itr->metric == ad_metric_type::VIEW_METRIC &&
            bid_itr->author == comment.author &&
            bid_itr->objective == comment.permlink )    // Retrieves highest paying bids for this view by this interface.
         {
            const ad_bid_object& bid = *bid_itr;
            const ad_audience_object& audience = _db.get_ad_audience( bid.bidder, bid.audience_id );

            if( !bid.is_delivered( o.viewer ) && audience.is_audience( o.viewer ) )
            {
               _db.deliver_ad_bid( bid, viewer );
               break;
            }

            ++bid_itr;
         }
      }
   }
   else  // View is being removed
   {
      FC_ASSERT( !o.viewed, 
         "Must select viewed = false to remove existing view" );
      
      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_reward -= itr->reward;
         c.view_power -= itr->reward;
         c.total_view_weight -= itr->max_weight;
         c.view_count--;
      });

      if( community_ptr != nullptr )
      {
         _db.modify( *community_ptr, [&]( community_object& bo )
         {
            bo.view_count--;  
         });
      }

      _db.remove( *itr );
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void share_evaluator::do_apply( const share_operation& o )
{ try {
   const account_name_type& signed_for = o.sharer;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const comment_object& comment = _db.get_comment( o.author, o.permlink );
   FC_ASSERT( comment.parent_author.size() == 0, 
      "Only top level posts can be shared." );
   FC_ASSERT( comment.allow_shares, 
      "shares are not allowed on the comment." );
   const account_object& sharer = _db.get_account( o.sharer );
   FC_ASSERT( sharer.can_vote,  
      "sharer has declined their voting rights." );
   const community_object* community_ptr = nullptr;
   if( comment.community.size() )
   {
      community_ptr = _db.find_community( comment.community );      
      const community_member_object& community_member = _db.get_community_member( comment.community );       
      FC_ASSERT( community_member.is_authorized_interact( sharer.name ), 
         "User ${u} is not authorized to interact with posts in the community ${b}.",("b", comment.community)("u", sharer.name));
   }

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }

   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   const reward_fund_object& reward_fund = _db.get_reward_fund( comment.reward_currency );
   auto curve = reward_fund.curation_reward_curve;
   time_point now = _db.head_block_time();
   
   const auto& comment_share_idx = _db.get_index< comment_share_index >().indices().get< by_comment_sharer >();
   auto itr = comment_share_idx.find( std::make_tuple( comment.id, sharer.name ) );

   int64_t elapsed_seconds = ( now - sharer.last_share_time ).to_seconds();
   FC_ASSERT( elapsed_seconds >= MIN_SHARE_INTERVAL.to_seconds(), 
      "Can only share once every ${s} seconds.", ("s", MIN_SHARE_INTERVAL) );
   int16_t regenerated_power = (PERCENT_100 * elapsed_seconds) / median_props.share_recharge_time.to_seconds();
   int16_t current_power = std::min( int64_t(sharer.sharing_power + regenerated_power), int64_t(PERCENT_100) );
   FC_ASSERT( current_power > 0, 
      "Account currently does not have any sharing power." );

   int16_t max_share_denom = median_props.share_reserve_rate * (median_props.share_recharge_time.count() / fc::days(1).count() );    // Weights the sharing power with the network reserve ratio and recharge time
   FC_ASSERT( max_share_denom > 0 );
   int16_t used_power = (current_power + max_share_denom - 1) / max_share_denom;
   FC_ASSERT( used_power <= current_power,   
      "Account does not have enough power to share." );
   share_type vp = _db.get_voting_power( sharer );         // Gets the user's voting power from their Equity and Staked coin balances to weight the share.
   share_type reward = ( vp.value * used_power ) / PERCENT_100;
   const comment_object& root = _db.get( comment.root_comment );       // If root post, gets the posts own object.

   if( itr == comment_share_idx.end() )   // New share is being added to emtpy index
   {
      FC_ASSERT( reward > 0, 
         "Cannot claim share with 0 reward." );
      FC_ASSERT( o.shared , 
         "Shared must be set to true to create new share, share does not exist to remove." );

      _db.modify( sharer, [&]( account_object& a )
      {
         a.sharing_power = current_power - used_power;
         a.last_share_time = now;
      });

      uint128_t old_power = std::max( uint128_t( comment.share_power.value ), uint128_t(0) );  // Record reward value before applying share transaction

      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_reward += reward;
         c.share_power += reward;
         c.share_count++;
      });

      if( community_ptr != nullptr )
      {
         _db.modify( *community_ptr, [&]( community_object& bo )
         {
            bo.share_count++;
         });
      }

      uint128_t new_power = std::max( uint128_t( comment.share_power.value ), uint128_t(0));   // record new net reward after sharing

      // Create comment share object for tracking share.
      _db.create< comment_share_object >( [&]( comment_share_object& cs )    
      {
         cs.sharer = sharer.name;
         cs.comment = comment.id;
         if( o.interface.size() )
         {
            cs.interface = o.interface;
         }
         cs.reward = reward;
         cs.created = now;

         bool curation_reward_eligible = reward > 0 && comment.cashout_time != fc::time_point::maximum() && comment.allow_curation_rewards;
  
         if( curation_reward_eligible )
         {
            uint128_t old_weight = util::evaluate_reward_curve(
               old_power, 
               root.cashouts_received, 
               curve, 
               median_props.content_reward_decay_rate, 
               reward_fund.content_constant
            );
            uint128_t new_weight = util::evaluate_reward_curve(
               new_power,
               root.cashouts_received, 
               curve, 
               median_props.content_reward_decay_rate, 
               reward_fund.content_constant
            );

            uint128_t max_share_weight = new_weight - old_weight;  // Gets the difference in content reward weight before and after the share occurs.

            _db.modify( comment, [&]( comment_object& c )
            {
               c.total_share_weight += max_share_weight;
            });

            uint128_t curation_auction_decay_time = median_props.curation_auction_decay_time.to_seconds();
            uint128_t w = max_share_weight;
            uint128_t delta_t = std::min( uint128_t(( now - comment.created).to_seconds()), curation_auction_decay_time ); 

            w *= delta_t;
            w /= curation_auction_decay_time;   // Discount weight linearly by time for early shares in the first 10 minutes.

            double curation_decay = median_props.share_curation_decay;      // Number of shares for half life of curation reward decay.
            double share_discount_rate = std::max(( double(comment.share_count) / curation_decay), double(0));
            double share_discount = std::pow(0.5, share_discount_rate );     // Raises 0.5 to a fractional power for each 1000 shares added
            uint64_t share_discount_percent = double(share_discount) * double(PERCENT_100);
            FC_ASSERT(share_discount_percent >= 0,
               "Share discount should not become negative");
            w *= share_discount_percent;
            w /= PERCENT_100;      // Discount weight exponentially for each successive share on the post, decaying by 50% per 50 shares.
            cs.weight = w;
            cs.max_weight = max_share_weight;
         }
         else
         {
            cs.weight = 0;
            cs.max_weight = 0;
         }
      });

      feed_reach_type reach_type = feed_reach_type::FOLLOW_FEED;

      for( size_t i = 0; i < feed_reach_values.size(); i++ )
      {
         if( o.reach == feed_reach_values[ i ] )
         {
            reach_type = feed_reach_type( i );
            break;
         }
      }
      
      // Create blog and feed objects for sharer account's followers and connections. 
      _db.share_comment_to_feeds( sharer.name, reach_type, comment );

      if( o.community.valid() )
      {
         const community_member_object& community_member = _db.get_community_member( *o.community );       
         FC_ASSERT( community_member.is_authorized_interact( sharer.name ), 
            "User ${u} is not authorized to interact with posts in the community ${b}.",
            ("b", *o.community)("u", sharer.name));

         _db.share_comment_to_community( sharer.name, *o.community, comment );
      }

      if( o.tag.valid() )
      {
         _db.share_comment_to_tag( sharer.name, *o.tag, comment );
      }

      if( sharer.membership == membership_tier_type::NONE )     // Check for the presence of an ad bid on this share.
      {
         const auto& bid_idx = _db.get_index< ad_bid_index >().indices().get< by_provider_metric_author_objective_price >();
         auto bid_itr = bid_idx.lower_bound( std::make_tuple( o.interface, ad_metric_type::SHARE_METRIC, comment.author, comment.permlink ) );

         while( bid_itr != bid_idx.end() &&
            bid_itr->provider == o.interface &&
            bid_itr->metric == ad_metric_type::SHARE_METRIC &&
            bid_itr->author == comment.author &&
            bid_itr->objective == comment.permlink )    // Retrieves highest paying bids for this share by this interface.
         {
            const ad_bid_object& bid = *bid_itr;
            const ad_audience_object& audience = _db.get_ad_audience( bid.bidder, bid.audience_id );

            if( !bid.is_delivered( o.sharer ) && audience.is_audience( o.sharer ) )
            {
               _db.deliver_ad_bid( bid, sharer );
               break;
            }

            ++bid_itr;
         }
      }
   }
   else  // share is being removed
   {
      FC_ASSERT( !o.shared, 
         "Must select shared = false to remove existing share" );
      
      _db.modify( comment, [&]( comment_object& c )
      {
         c.net_reward -= itr->reward;
         c.share_power -= itr->reward;
         c.total_share_weight -= itr->max_weight;
         c.share_count--;
      });

      if( community_ptr != nullptr )
      {
         _db.modify( *community_ptr, [&]( community_object& bo )
         {
            bo.share_count--;
         });
      }

      // Remove all blog and feed objects that the account has created for the original share operation. 
      _db.remove_shared_comment( sharer.name, comment );

      _db.remove( *itr );
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


/**
 * Moderation tags enable interface providers to coordinate moderation efforts
 * on-chain and provides a method for discretion to be provided
 * to displaying content, based on the governance addresses subscribed to by the 
 * viewing user.
 */
void moderation_tag_evaluator::do_apply( const moderation_tag_operation& o )
{ try {
   const account_name_type& signed_for = o.moderator;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_governance( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
   const account_object& moderator = _db.get_account( o.moderator );
   const account_object& author = _db.get_account( o.author );
   FC_ASSERT( author.active, 
      "Author: ${s} must be active to broadcast transaction.",("s", o.author) );

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }
   
   const comment_object& comment = _db.get_comment( o.author, o.permlink );
   const governance_account_object* gov_ptr = _db.find_governance_account( o.moderator );

   const community_object* community_ptr = nullptr;
   if( comment.community.size() )
   {
      community_ptr = _db.find_community( comment.community );
   }

   if( community_ptr != nullptr || gov_ptr != nullptr )
   {
      const community_member_object* community_member_ptr = _db.find_community_member( comment.community );
      FC_ASSERT( community_member_ptr != nullptr || gov_ptr != nullptr,
         "Account must be a community moderator or governance account to create moderation tag." );

      if( community_member_ptr == nullptr )     // No community, must be governance account.
      {
         FC_ASSERT( gov_ptr != nullptr,
            "Account must be a governance account to create moderation tag." );
         FC_ASSERT( gov_ptr->account == o.moderator,
            "Account must be a governance account to create moderation tag." );
      }
      else if( gov_ptr == nullptr )         // Not governance account, must be moderator.
      {
         FC_ASSERT( community_member_ptr != nullptr,
            "Account must be a community moderator to create moderation tag." );
         FC_ASSERT( community_member_ptr->is_moderator( o.moderator ),
            "Account must be a community moderator to create moderation tag." );
      }
      else
      {
         FC_ASSERT( community_member_ptr->is_moderator( o.moderator ) || gov_ptr->account == o.moderator,
            "Account must be a community moderator or governance account to create moderation tag." );
      } 
   }
   
   const auto& mod_idx = _db.get_index< moderation_tag_index >().indices().get< by_moderator_comment >();
   auto mod_itr = mod_idx.find( boost::make_tuple( o.moderator, comment.id ) );
   time_point now = _db.head_block_time();

   if( mod_itr != mod_idx.end() )     // Creating new moderation tag.
   {
      FC_ASSERT( o.applied,
         "Moderation tag does not exist, Applied should be set to true to create new moderation tag." );

      _db.create< moderation_tag_object >( [&]( moderation_tag_object& mto )
      {
         mto.moderator = moderator.name;
         mto.comment = comment.id;
         mto.community = comment.community;

         for( tag_name_type t : o.tags )
         {
            mto.tags.push_back( t );
         }
         if( o.interface.size() )
         {
            mto.interface = o.interface;
         }

         mto.rating = o.rating;
         from_string( mto.details, o.details );
         mto.filter = o.filter;
         mto.last_updated = now;
         mto.created = now;  
      });
   }
   else    
   {
      if( o.applied )  // Editing existing moderation tag
      {
         _db.modify( *mod_itr, [&]( moderation_tag_object& mto )
         {
            for( tag_name_type t : o.tags )
            {
               mto.tags.push_back( t );
            }
            if( o.interface.size() )
            {
               mto.interface = o.interface;
            }
            mto.rating = o.rating;
            from_string( mto.details, o.details );
            mto.filter = o.filter;
            mto.last_updated = now;
         });
      }
      else    // deleting moderation tag
      {
         _db.remove( *mod_itr );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


} } // node::chain