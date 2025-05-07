
#include "csm_llc.h"
#include "csm_definitions.h"
#include "os_util.h"

    // The TCP/IP Cosem packet is sent with a header. See GreenBook 8 7.3.3.2 The wrapper protocol data unit (WPDU)

    // Version, 2 bytes
    // Source wPort, 2 bytes
    // Destination wPort: 2 bytes
    // Length: 2 bytes

static const uint16_t COSEM_WRAPPER_VERSION = 0x0001U;

int csm_llc_wpdu_decode(uint8_t *buffer, uint32_t payload_size, uint16_t *ssap, uint16_t *dsap)
{
    int ret = FALSE;
    uint16_t version;
    uint16_t apdu_size;

    // The TCP/IP Cosem packet is sent with a header. See GreenBook 8
    version = GET_BE16(&buffer[0U]);
    *ssap = GET_BE16(&buffer[2]); // client profile (Public, Utility...)
    *dsap = GET_BE16(&buffer[4]); // logical device (1 = management)
    apdu_size = GET_BE16(&buffer[6]);

    // Sanity check of the packet
    if ((payload_size == (apdu_size + COSEM_WRAPPER_SIZE)) &&(version == COSEM_WRAPPER_VERSION))
    {
        ret = apdu_size;
    }

    return ret;
}


int csm_llc_wpdu_encode(uint8_t *buffer, uint32_t payload_size, uint16_t ssap, uint16_t dsap)
{
    PUT_BE16(&buffer[0U], COSEM_WRAPPER_VERSION);
    PUT_BE16(&buffer[2], dsap);
    PUT_BE16(&buffer[4], ssap);
    PUT_BE16(&buffer[6], payload_size);

    return COSEM_WRAPPER_SIZE;
}
