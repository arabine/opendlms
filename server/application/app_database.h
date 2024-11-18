#ifndef CSM_DATABASE_H
#define CSM_DATABASE_H

#include <stdint.h>
#include "csm_services.h"


// Database access from Cosem
csm_db_code csm_db_access_func(csm_db_context_t *ctx, csm_array *in, csm_array *out, csm_request *request);

// Initialize Cosem database (objects list)
void csm_db_set_database(const struct db_element *db, uint32_t size);

#endif // CSM_DATABASE_H
