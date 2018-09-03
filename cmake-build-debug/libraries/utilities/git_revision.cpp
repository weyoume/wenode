#include <stdint.h>
#include <graphene/utilities/git_revision.hpp>

#define GRAPHENE_GIT_REVISION_SHA "2e34a9cee465b295c655f0070105fcce288a729c"
#define GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP 1535894712
#define GRAPHENE_GIT_REVISION_DESCRIPTION "v0.19.2-259-g2e34a9ce"

namespace graphene { namespace utilities {

const char* const git_revision_sha = GRAPHENE_GIT_REVISION_SHA;
const uint32_t git_revision_unix_timestamp = GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP;
const char* const git_revision_description = GRAPHENE_GIT_REVISION_DESCRIPTION;

} } // end namespace graphene::utilities
