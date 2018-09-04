#include <stdint.h>
#include <graphene/utilities/git_revision.hpp>

#define GRAPHENE_GIT_REVISION_SHA "240b03e73a74330efbd5295d440d627ae3651e51"
#define GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP 1535942822
#define GRAPHENE_GIT_REVISION_DESCRIPTION "v0.19.2-264-g240b03e7"

namespace graphene { namespace utilities {

const char* const git_revision_sha = GRAPHENE_GIT_REVISION_SHA;
const uint32_t git_revision_unix_timestamp = GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP;
const char* const git_revision_description = GRAPHENE_GIT_REVISION_DESCRIPTION;

} } // end namespace graphene::utilities
