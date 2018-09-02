#include <stdint.h>
#include <graphene/utilities/git_revision.hpp>

#define GRAPHENE_GIT_REVISION_SHA "9a047ce7cee2045a323b14431cdfb3a14bc6fc87"
#define GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP 1535893392
#define GRAPHENE_GIT_REVISION_DESCRIPTION "v0.19.2-256-g9a047ce7"

namespace graphene { namespace utilities {

const char* const git_revision_sha = GRAPHENE_GIT_REVISION_SHA;
const uint32_t git_revision_unix_timestamp = GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP;
const char* const git_revision_description = GRAPHENE_GIT_REVISION_DESCRIPTION;

} } // end namespace graphene::utilities
