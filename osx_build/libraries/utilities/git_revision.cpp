#include <stdint.h>
#include <graphene/utilities/git_revision.hpp>

#define GRAPHENE_GIT_REVISION_SHA "40dd9dbe6ee24eb8581121ff912b5654fbe5d1eb"
#define GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP 1537736279
#define GRAPHENE_GIT_REVISION_DESCRIPTION "v0.19.2-572-g40dd9dbe"

namespace graphene { namespace utilities {

const char* const git_revision_sha = GRAPHENE_GIT_REVISION_SHA;
const uint32_t git_revision_unix_timestamp = GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP;
const char* const git_revision_description = GRAPHENE_GIT_REVISION_DESCRIPTION;

} } // end namespace graphene::utilities
