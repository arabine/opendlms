

#include "meter.h"


#include <stdlib.h>
#include <string.h>
#include <time.h> // to initialize the seed

#include "csm_array.h"
#include "csm_ber.h"
#include "csm_llc.h"

#include "app_database.h"

#include "os_util.h"
#include "bitfield.h"
#include "transports.h"
#include "meter_definitions.h"

#include "db_cosem_object_list.h"
#include "db_cosem_clock.h"
#include "db_cosem_associations.h"
#include "db_cosem_image_transfer.h"


// Buffer has the following format:
//   | SC + AK + CHALLENGE | TCP wrapper | APDU
// With
// SC+AK+CHALLENGE: security AAD header, room booked to optimize buffer management with cyphering
// wrapper: Cosem standard TCP wrapper (4 words)
// APDU: Cosem APDU



#define BUF_SIZE (METER_PDU_SIZE + CSM_DEF_MAX_HLS_SIZE + COSEM_WRAPPER_SIZE)

#define METER_NUMBER_OF_LOGICAL_DEVICES 1U

static csm_db_t database[METER_NUMBER_OF_LOGICAL_DEVICES] = {
    {
        .el = gDataBaseList,
        .size = COSEM_DATABASE_SIZE,
        .logical_device = 1U,
    }
};


typedef struct
{
    uint8_t rx_buffer[BUF_SIZE];
    uint8_t tx_buffer[BUF_SIZE];
    uint8_t scratch_buffer[METER_SCRATCH_BUF_SIZE];    
} asso_buffers_t;

static asso_buffers_t com_buffers[METER_NUMBER_OF_ASSOCIATIONS];

static csm_server_context_t contexes[METER_NUMBER_OF_ASSOCIATIONS];

static const csm_asso_config default_assos_config[METER_NUMBER_OF_ASSOCIATIONS] =
{
    // Public association
    { {16U, 1U},
      CSM_CBLOCK_GET | CSM_CBLOCK_BLOCK_TRANSFER_WITH_GET_OR_READ,
      0U, // No auto-connected
    },

    // Client management association
    { {1U, 1U},
        CSM_CBLOCK_GET | CSM_CBLOCK_ACTION | CSM_CBLOCK_SET |CSM_CBLOCK_BLOCK_TRANSFER_WITH_GET_OR_READ | CSM_CBLOCK_SELECTIVE_ACCESS,
        0U, // No auto-connected
    }
};


int8_t meter_connect()
{
    int8_t ret = CSM_CHANNEL_INVALID_ID;

    for (uint32_t i = 0U; i < METER_NUMBER_OF_ASSOCIATIONS; i++)
    {
        if (contexes[i].asso.state_cf == CF_INACTIVE)
        {
            contexes[i].asso.state_cf = CF_IDLE;
            ret = i;
            CSM_LOG("[LLC] Channel %d connected", ret);
            break;
        }
    }

    return ret;
}

void meter_disconnect(int8_t channel_id)
{
    if (channel_id > CSM_CHANNEL_INVALID_ID)
    {
        contexes[channel_id].asso.state_cf = CF_INACTIVE;
    }
    else
    {
        CSM_ERR("[LLC] Channel id invalid");
    }
}


/**
 * @brief tcp_data_handler
 * This link layer manages the data between the transport (TCP/IP) and the Cosem stack
 * The function is called by the TCP/IP server upon reception of a new packet.
 * The passed buffer must be filled by the reply, if any, and return the according number
 * of bytes to transfer back to the sender.
 *
 * The data link layer is application specific - but rather simple - and must be implemented
 * in the application side. Thus, the DLMS/Cosem stack remains agnostic on the transport layer.
 *
 * @param channel_id: index of the association used
 * @param buffer
 * @param size
 * @return > 0 the number of bytes to reply back to the sender
 */
int meter_tcp_data_handler(int8_t channel_id, uint8_t *buffer, uint32_t payload_size, uint32_t buffer_size)
{   
    int ret = -1;
    CSM_LOG("[LLC] TCP Packet received");

    print_hex((const char *)(buffer), payload_size);

    if (channel_id > CSM_CHANNEL_INVALID_ID)
    {
        csm_server_context_t *ctx = &contexes[channel_id];

        ret = csm_llc_wpdu_decode(buffer, payload_size, &ctx->request.llc.ssap, &ctx->request.llc.dsap);
        if (ret > 0)
        {
            CSM_LOG("[LLC] Packet decoded");
            csm_array_reset(&ctx->asso.rx);
            csm_array_reset(&ctx->asso.tx);
            // Append only the Cosem ADPU (jump over the TCP wrapper)
            csm_array_write_buff(&ctx->asso.rx, &buffer[COSEM_WRAPPER_SIZE], ret);

            ret = csm_server_execute(ctx, &default_assos_config[0], METER_NUMBER_OF_ASSOCIATIONS, &database[0], METER_NUMBER_OF_LOGICAL_DEVICES);

            // Some data to reply
            if (ret > 0)
            {
                // Encode the packet
                int llc_size = csm_llc_wpdu_encode(buffer, ret, ctx->request.llc.ssap, ctx->request.llc.dsap);
                // Add the Cosem APDU
                uint32_t apdu_size = csm_array_written(&ctx->asso.tx);
                if (apdu_size > 0)
                {
                    // Compute total packet size
                    ret = apdu_size + llc_size;
                    if (ret <= buffer_size)
                    {
                        // Append the APDU to the buffer, after the LLC
                        memcpy(&buffer[llc_size], csm_array_start(&ctx->asso.tx), ret);
                    }
                    else
                    {
                        CSM_ERR("[LLC] APDU too big for TCP layer");
                        ret = -1;
                    }
                }
                else
                {
                    CSM_ERR("[LLC] No APDU to send");
                    ret = -1;
                }
            }
            else
            {
                CSM_LOG("[LLC] No reply");
            }
        }
        else
        {
            CSM_ERR("[LLC] Packet not decoded");
        }
    }
    else
    {
        CSM_ERR("[LLC] Channel id invalid");
    }

    return ret;
}


void meter_send_ascii_tcp_message(int8_t channel_id, const char *message, uint32_t size)
{
    uint32_t payload_size = size / 2;
    if (payload_size > BUF_SIZE)
    {
        CSM_ERR("Message too long!");
        return;
    }

    // ASCII Hex to binary conversion
    uint8_t buffer[BUF_SIZE];
    hex2bin(message, (char *)(&buffer[0]), size);

    int ret = meter_tcp_data_handler(channel_id, buffer, payload_size, BUF_SIZE);

    if (ret > 0)
    {
        CSM_LOG("Reply: ");
        print_hex((const char *)&buffer[0], ret);
    }
    else
    {
        CSM_LOG("No reply");
    }
}


void meter_initialize()
{
    // Init random seed
    srand(time(NULL));

    // Initialize the communication buffers for all the associations
    for (uint32_t i = 0U; i < METER_NUMBER_OF_ASSOCIATIONS; i++)
    {
        csm_array_init(&contexes[i].asso.rx, com_buffers[i].rx_buffer, sizeof(com_buffers[i].rx_buffer), 0U, BUF_APDU_OFFSET);
        csm_array_init(&contexes[i].asso.tx, com_buffers[i].tx_buffer, sizeof(com_buffers[i].tx_buffer), 0U, BUF_APDU_OFFSET);
        csm_array_init(&contexes[i].asso.scratch, com_buffers[i].scratch_buffer, sizeof(com_buffers[i].scratch_buffer), 0U, BUF_APDU_OFFSET);
        contexes[i].db_access_func = csm_db_access_func;
        contexes[i].asso.channel_id = i;
        csm_asso_init(&contexes[i].asso);
    }
}



