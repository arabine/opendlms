

#include "meter.h"


#include "app_database.h"
#include "db_cosem_clock.h"
#include "db_cosem_associations.h"
#include "db_cosem_image_transfer.h"
#include "os_util.h"
#include "transports.h"


static const uint16_t COSEM_WRAPPER_VERSION = 0x0001U;

void meter_send_message(csm_server_context_t *ctx, const char *message, uint32_t size)
{
    if (size > BUF_SIZE)
    {
        CSM_ERR("Message too long!");
        return;
    }

    hex2bin(message, (char *)(&ctx->buffer[BUF_WRAPPER_OFFSET]), size);

    // channel to index
    int ret = meter_handle_message(ctx, ctx->currentChannel - 1, &ctx->memory, size / 2);

    if (ret > 0)
    {
        CSM_LOG("Reply: ");
        print_hex((const char *)&ctx->buffer[BUF_APDU_OFFSET], ret);
    }
    else
    {
        CSM_LOG("No reply");
    }
}

com_channel_t meter_connect(csm_server_context_t *ctx)
{
    com_channel_t com;
    int8_t channel_id = csm_server_connect(&ctx->server_context);
    CSM_LOG("Connected!");

    if (channel_id >= 0)
    {
        com.channel_id = channel_id;
        com.mem_rx = ctx->channels[channel_id].rx
    }

}

void meter_disconnect(csm_server_context_t *ctx, int8_t channel_id)
{
    if (channel_id > CSM_CHANNEL_INVALID_ID)
    {
        csm_server_disconnect(&ctx->server_context, channel_id);
    }
    else
    {
        CSM_ERR("[LLC] Channel id invalid");
    }
}



int meter_handle_message(csm_server_context_t *ctx, int8_t channel_id, uint32_t payload_size)
{
    int ret = -1;

            ret = csm_server_execute(&ctx->server_context, &request, &packet);

            if (ret > 0)
            {
                print_hex((const char *)(&b->data[BUF_APDU_OFFSET]), ret);

                // Set Version
                PUT_BE16(&buffer[0], version);

                // Swap SSAP and DSAP
                PUT_BE16(&buffer[2], ctx->channels[channel].request.llc.dsap);
                PUT_BE16(&buffer[4], ctx->channels[channel].request.llc.ssap);

                // Update Cosem Wrapper length
                PUT_BE16(&buffer[6], (uint16_t) ret);

                // Add wrapper size to the data packet size
                ret += COSEM_WRAPPER_SIZE;
            }
        }
        else
        {
            CSM_ERR("[LLC] Bad Packet received");
        }
    }
    else
    {
        CSM_ERR("[LLC] Bad Packet received");
    }

    return ret;

}

void meter_cosem_stack_initialize(csm_server_context_t *ctx, csm_db_access_handler db_handler, csm_db_t *db)
{

    csm_server_init(ctx); 
}



