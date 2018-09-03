#include <stdint.h>
#include <graphene/utilities/git_revision.hpp>

#define GRAPHENE_GIT_REVISION_SHA "ec7dd60ff83a8e45363d93b88521aa57d5263c5b"
#define GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP 1535941449
#define GRAPHENE_GIT_REVISION_DESCRIPTION "v0.19.2-260-gec7dd60f"

namespace graphene { namespace utilities {

const char* const git_revision_sha = GRAPHENE_GIT_REVISION_SHA;
const uint32_t git_revision_unix_timestamp = GRAPHENE_GIT_REVISION_UNIX_TIMESTAMP;
const char* const git_revision_description = GRAPHENE_GIT_REVISION_DESCRIPTION;

} } // end namespace graphene::utilities
