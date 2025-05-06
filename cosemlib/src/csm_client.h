/**
 * Cosem services coder/decoder
 *
 * Copyright (c) 2016, Anthony Rabine
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the MIT license.
 * See LICENSE.txt for more details.
 *
 */

#ifndef CSM_CLIENT_H
#define CSM_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "csm_definitions.h"
#include "csm_association.h"
#include "csm_database.h"


// ----------------------------------- CLIENT SERVICES -----------------------------------

void csm_client_init(csm_request *request, csm_response *response);
int csm_client_has_more_data(csm_response *response);
int csm_client_decode(csm_response *response, csm_array *array);
int svc_request_encoder(csm_request *request, csm_array *array);
int csm_client_encode_selective_access_by_range(csm_array *array, csm_object_t *restricting_object, csm_array *start, csm_array *end);


#ifdef __cplusplus
}
#endif

#endif // CSM_CLIENT_H
