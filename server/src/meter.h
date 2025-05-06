/**
 * Simple meter application, communication agnostic
 *
 * Copyright (c) 2016, Anthony Rabine
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the MIT license.
 * See LICENSE.txt for more details.
 *
 */

#pragma once

#include <stdlib.h>
#include <string.h>
#include <time.h> // to initialize the seed


// Cosem stack
#include "csm_array.h"
#include "csm_ber.h"
#include "csm_definitions.h"

// Meter environment
#include "app_database.h"
#include "os_util.h"
#include "bitfield.h"
#include "transports.h"


void meter_cosem_stack_initialize(csm_server_context_t *ctx, csm_db_access_handler db_handler, csm_db_t *db);


void meter_connect(csm_server_context_t *ctx);
void meter_disconnect(csm_server_context_t *ctx, int8_t channel_id);

// ASCII string of hexadecimal values
void meter_send_message(csm_server_context_t *ctx, const char *message, uint32_t size);

int meter_handle_message(csm_server_context_t *ctx, int8_t channel_id, uint32_t payload_size);


