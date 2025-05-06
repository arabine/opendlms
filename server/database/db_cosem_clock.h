#ifndef CSM_CLOCK_OBJECTS_H
#define CSM_CLOCK_OBJECTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_database.h"
#include "csm_definitions.h"

csm_db_code db_cosem_clock_func(csm_server_context_t *ctx, csm_array *in, csm_array *out);

#ifdef __cplusplus
}
#endif

#endif // CSM_CLOCK_OBJECTS_H
