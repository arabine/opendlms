#pragma once

#include <stdint.h>

int csm_llc_wpdu_decode(uint8_t *buffer, uint32_t payload_size, uint16_t *ssap, uint16_t *dsap);
int csm_llc_wpdu_encode(uint8_t *buffer, uint32_t payload_size, uint16_t ssap, uint16_t dsap);

