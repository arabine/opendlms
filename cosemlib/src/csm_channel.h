/**
 * A virtual channel of communication with the logical device
 *
 * Copyright (c) 2016, Anthony Rabine
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the MIT license.
 * See LICENSE.txt for more details.
 *
 */

#ifndef CSM_CHANNEL_H
#define CSM_CHANNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "csm_association.h"
#include "csm_database.h"

#define INVALID_CHANNEL_ID 0U


typedef struct
{
    csm_request request;
    csm_asso_state *asso;   //!< Association used for that channel

    uint32_t current_block;
} csm_channel;



int csm_channel_hls_pass3(csm_array *array, csm_channel *channel);
int csm_channel_hls_pass4(csm_array *array, csm_channel *channel);


#ifdef __cplusplus
}
#endif

#endif // CSM_CHANNEL_H
