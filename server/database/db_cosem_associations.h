#ifndef DB_COSEM_ASSOCIATIONS_H
#define DB_COSEM_ASSOCIATIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_database.h"
#include "csm_definitions.h"

csm_db_code db_cosem_associations_func(csm_server_context_t *ctx, csm_channel *channel, csm_array *in, csm_array *out);

#ifdef __cplusplus
}
#endif

#endif // DB_COSEM_ASSOCIATIONS_H
