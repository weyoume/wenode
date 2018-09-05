#include <steem/protocol/validation.hpp>
#include <steem/protocol/steem_actions.hpp>

namespace steem { namespace protocol {

void example_required_action::validate()const
{
   validate_account_name( account );
}

bool operator==( const example_required_action& lhs, const example_required_action& rhs )
{
   return lhs.account == rhs.account;
}

void example_optional_action::validate()const
{
   validate_account_name( account );
}

bool operator==( const example_optional_action& lhs, const example_optional_action& rhs )
{
   return lhs.account == rhs.account;
}

} } //steem::protocol
