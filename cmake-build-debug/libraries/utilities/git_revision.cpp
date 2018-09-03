#include <stdint.h>
#include <graphene/utilities/git_revision.hpp>

#define GRAPHENE_GIT_REVISION_SHA "b8b2ce13f9c3940694310de3363933618891f962"
#define GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP 1535942230
#define GRAPHENE_GIT_REVISION_DESCRIPTION "v0.19.2-262-gb8b2ce13"

namespace graphene { namespace utilities {

const char* const git_revision_sha = GRAPHENE_GIT_REVISION_SHA;
const uint32_t git_revision_unix_timestamp = GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP;
const char* const git_revision_description = GRAPHENE_GIT_REVISION_DESCRIPTION;

} } // end namespace graphene::utilities
