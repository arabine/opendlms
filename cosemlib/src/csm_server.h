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
#include "csm_channel.h"

typedef struct csm_server_context_t csm_server_context_t;
typedef csm_db_code (*csm_db_access_handler)(csm_server_context_t *ctx, csm_channel *channel, csm_array *in, csm_array *out);
 
struct csm_server_context_t
{
    csm_channel *channel_list;   //!< List of channels
    uint8_t channel_list_size;   //!< Size of the channel list
    csm_asso_state *asso_list;   //!< List of associations
    const csm_asso_config *asso_conf_list;   //!< List of association configurations
    uint8_t asso_list_size;      //!< Size of the association list
    csm_db_t *db; // database of cosem objects
    csm_db_access_handler db_access_func; // function to access the database

};

struct db_element
{
    const db_object_descr *objects;
    csm_db_access_handler handler;
    uint32_t nb_objects;
};
 
void csm_server_init(csm_server_context_t *cosem_ctx);
int csm_server_execute(csm_server_context_t *cosem_ctx, uint8_t channel, csm_array *packet);

// Connect to a channel
uint8_t csm_server_connect(csm_server_context_t *cosem_ctx);
void csm_server_disconnect(csm_server_context_t *cosem_ctx, uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif // CSM_SERVER_H