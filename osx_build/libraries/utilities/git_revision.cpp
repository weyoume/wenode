#include <stdint.h>
#include <graphene/utilities/git_revision.hpp>

#define GRAPHENE_GIT_REVISION_SHA "ab502e64a0317eb272a467e665596296fb985c55"
#define GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP 1536935849
#define GRAPHENE_GIT_REVISION_DESCRIPTION "v0.19.2-571-gab502e64"

namespace graphene { namespace utilities {

const char* const git_revision_sha = GRAPHENE_GIT_REVISION_SHA;
const uint32_t git_revision_unix_timestamp = GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP;
const char* const git_revision_description = GRAPHENE_GIT_REVISION_DESCRIPTION;

} } // end namespace graphene::utilities
