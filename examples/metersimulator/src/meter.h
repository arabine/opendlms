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

#ifdef __cplusplus
extern "C" {
#endif


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

void meter_initialize();

int8_t meter_connect();
void meter_disconnect(int8_t channel_id);
int meter_tcp_data_handler(int8_t channel_id, uint8_t *buffer, uint32_t payload_size, uint32_t buffer_size);

// ASCII string of hexadecimal values TCP Wrapper + cosem APDU
void meter_send_ascii_tcp_message(int8_t channel_id, const char *message, uint32_t size);

#ifdef __cplusplus
}
#endif

