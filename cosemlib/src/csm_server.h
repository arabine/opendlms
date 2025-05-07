/**
 * Main server context structure that contains a Cosem implementation states, parameters and database
 *
 * Copyright (c) 2016, Anthony Rabine
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the MIT license.
 * See LICENSE.txt for more details.
 *
 */

#ifndef CSM_SERVER_H
#define CSM_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "csm_association.h"
#include "csm_database.h"


typedef struct csm_server_context_t csm_server_context_t;
typedef csm_db_code (*csm_db_access_handler)(csm_server_context_t *ctx, csm_array *in, csm_array *out);
 
struct csm_server_context_t
{
    csm_db_t *db; // database of cosem objects, possibility to change it dynamically regarding the Logical Device targeted 
    csm_db_access_handler db_access_func; // function to access the database

    csm_request request;   //!< Current request
    csm_asso_state asso;   //!< Current association 
};

struct db_element
{
    const db_object_descr *objects;
    csm_db_access_handler handler;
    uint32_t nb_objects;
};
 

int csm_server_execute( csm_server_context_t *cosem_ctx, 
                        const csm_asso_config *configs, uint32_t number_of_associations,
                        csm_db_t *db, uint32_t number_of_logical_devices
                        );


#ifdef __cplusplus
}
#endif

#endif // CSM_SERVER_H