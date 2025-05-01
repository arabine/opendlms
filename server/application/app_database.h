#ifndef APP_DATABASE_H
#define APP_DATABASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "csm_server.h"


// Database access from Cosem
csm_db_code csm_db_access_func(csm_server_context_t *ctx, csm_channel *channel, csm_array *in, csm_array *out);

#ifdef __cplusplus
}
#endif

#endif // APP_DATABASE_H
