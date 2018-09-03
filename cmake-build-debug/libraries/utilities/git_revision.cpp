#include <stdint.h>
#include <graphene/utilities/git_revision.hpp>

#define GRAPHENE_GIT_REVISION_SHA "7bb2ef333cff0eafaafaf8b0c794f66e5d693a63"
#define GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP 1535942659
#define GRAPHENE_GIT_REVISION_DESCRIPTION "v0.19.2-263-g7bb2ef33"

namespace graphene { namespace utilities {

const char* const git_revision_sha = GRAPHENE_GIT_REVISION_SHA;
const uint32_t git_revision_unix_timestamp = GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP;
const char* const git_revision_description = GRAPHENE_GIT_REVISION_DESCRIPTION;

} } // end namespace graphene::utilities
