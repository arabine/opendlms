
#include <stdlib.h>
#include <string.h>
#include <time.h> // to initialize the seed

// Cosem stack
#include "csm_array.h"
#include "csm_ber.h"

// Meter environment
//#include "unity_fixture.h"
#include "tcp_server.h"
#include "app_database.h"
#include "os_util.h"
#include "bitfield.h"
#include "server_config.h"

#include "db_cosem_object_list.h"
#include "meter.h"
#include "meter_definitions.h"

// Meter specific configuration



// Buffer has the following format:
//   | SC + AK + CHALLENGE | TCP wrapper | APDU
// With
// SC+AK+CHALLENGE: security AAD header, room booked to optimize buffer management with cyphering
// wrapper: Cosem standard TCP wrapper (4 words)
// APDU: Cosem APDU



#define BUF_SIZE (METER_PDU_SIZE + CSM_DEF_MAX_HLS_SIZE + COSEM_WRAPPER_SIZE)


static csm_db_t database = {
    .el = gDataBaseList,
    .size = COSEM_DATABASE_SIZE,
};


typedef struct
{
    uint8_t rx_buffer[BUF_SIZE];
    uint8_t tx_buffer[BUF_SIZE];
    uint8_t scratch_buffer[METER_SCRATCH_BUF_SIZE];    
} asso_buffers_t;

static asso_buffers_t com_buffers[METER_NUMBER_OF_ASSOCIATIONS];

static csm_asso_state assos[METER_NUMBER_OF_ASSOCIATIONS];


static const csm_asso_config default_assos_config[] =
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

#define NUMBER_OF_ASSOS (sizeof(default_assos_config) / sizeof(csm_asso_config))


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
int tcp_data_handler(int8_t channel_id, uint8_t *buffer, uint32_t payload_size)
{   
    int ret = -1;
    uint16_t version;
    uint16_t apdu_size;
    csm_array packet;


    // The TCP/IP Cosem packet is sent with a header. See GreenBook 8 7.3.3.2 The wrapper protocol data unit (WPDU)

    // Version, 2 bytes
    // Source wPort, 2 bytes
    // Destination wPort: 2 bytes
    // Length: 2 bytes
    CSM_LOG("[LLC] TCP Packet received");

    print_hex((const char *)(buffer), payload_size);

    csm_request request;

    if ((payload_size > COSEM_WRAPPER_SIZE)
    {
        version = GET_BE16(&buffer[0U]);
        ctx->channels[channel].request.llc.ssap = GET_BE16(&buffer[2]);
        ctx->channels[channel].request.llc.dsap = GET_BE16(&buffer[4]);
        apdu_size = GET_BE16(&buffer[6]);

        // Sanity check of the packet
        if ((payload_size == (apdu_size + COSEM_WRAPPER_SIZE)) &&(version == COSEM_WRAPPER_VERSION))
        {
            print_hex((const char *)(&b->data[BUF_APDU_OFFSET]), apdu_size);


            ret = meter_handle_message(&meter_ctx, channel, b, payload_size);

        }

    }
    else
    {
        CSM_ERR("[LLC] Bad Packet received");
    }

    return ret;
}




uint8_t tcp_conn_handler(int8_t channel_id, enum conn_event event)
{
    uint8_t ret = FALSE;
    switch(event)
    {
    case CONN_DISCONNECTED:
    {
        if (channel_id > CSM_CHANNEL_INVALID_ID)
        {
            meter_disconnect(&meter_ctx, channel_id);
            ret = TRUE;
            CSM_ERR("[LLC] Channel %d disconnected", channel_id);
        }
        else
        {
            CSM_ERR("[LLC] Channel id invalid");
        }
        break;
    }

    case CONN_NEW:
    {
        meter_connect(&meter_ctx);
        ret = meter_ctx.currentChannel;
        if (!ret)
        {
            CSM_ERR("[LLC] Cannot find free channel slot");
        }

        break;
    }

    default:
        CSM_ERR("[LLC] Received spurious event");
        break;
    }
    return ret; // Returns error or the channel id
}


int main(int argc, const char * argv[])
{
    (void) argc;
    (void) argv;

    // Init random seed
    srand(time(NULL));

    // Initialize the communication buffers for all the associations
    for (uint32_t i = 0U; i < METER_NUMBER_OF_ASSOCIATIONS; i++)
    {
        csm_array_init(&meter_ctx.asso_list[i].rx, com_buffers[i].rx_buffer, sizeof(com_buffers[i].rx_buffer), 0U, BUF_APDU_OFFSET);
        csm_array_init(&meter_ctx.asso_list[i].tx, com_buffers[i].tx_buffer, sizeof(com_buffers[i].tx_buffer), 0U, BUF_APDU_OFFSET);
        csm_array_init(&meter_ctx.asso_list[i].scratch, com_buffers[i].scratch_buffer, sizeof(com_buffers[i].scratch_buffer), 0U, BUF_APDU_OFFSET);
        csm_asso_init(&assos[i]);
    }

    meter_cosem_stack_initialize(&meter_ctx, csm_db_access_func, &database);
    
    printf("Starting DLMS/Cosem meter simulator\r\nCosem library version: %s\r\n\r\n", CSM_DEF_LIB_VERSION);

    int ret = tcp_server_init(tcp_data_handler, tcp_conn_handler, TCP_PORT);

 
    printf("Exiting DLMS/Cosem meter simulator\r\n");

    return ret;
}

