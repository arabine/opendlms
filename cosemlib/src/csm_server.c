#include "csm_server.h"
#include "csm_axdr_codec.h"

// FIXME: add parameters to specialize the exception response
int svc_exception_response_encoder(csm_array *array)
{
    int valid = csm_array_write_u8(array, AXDR_EXCEPTION_RESPONSE);
    valid = valid && csm_array_write_u8(array, 1U);
    valid = valid && csm_array_write_u8(array, 1U);
    return valid;
}


enum data_access_result
{
    SRV_RESULT_SUCCESS              = 0U,
    SRV_RESULT_HARDWARE_FAULT       = 1U,
    SRV_RESULT_TEMPORARY_FAILURE    = 2U,
    SRV_RESULT_READ_WRITE_DENIED    = 3U,
    SRV_RESULT_OBJECT_UNDEFINED     = 4U,
    SRV_RESULT_OTHER_REASON         = 250U
};

int svc_data_access_result_encoder(csm_array *array, csm_db_code code)
{
    uint8_t result;
    // Transform the code into a DLMS/Cosem valid response
    if (code == CSM_OK)
    {
        result = SRV_RESULT_SUCCESS;
    }
    else
    {
        result = SRV_RESULT_OTHER_REASON;
    }

    return csm_array_write_u8(array, result);
}


int svc_is_normal_request(uint8_t type)
{
    int istype = FALSE;

    // Same type for SET/GET/ACTION
    if (type == 1U)
    {
        istype = TRUE;
    }

    return istype;
}

int svc_is_next_request(uint8_t type, enum csm_service service)
{
    int istype = FALSE;

    // Same type for SET/GET/ACTION
    if ((type == 2U) && (service != SVC_ACTION))
    {
        istype = TRUE;
    }

    return istype;
}


int svc_decode_request(csm_request *request, csm_array *array)
{
    uint8_t type = 0U;
    int valid = csm_array_read_u8(array, &type);
    valid = valid && csm_array_read_u8(array, &request->sender_invoke_id); // save the invoke ID to reuse the same

    if (valid)
    {
        if (svc_is_normal_request(type))
        {
            valid = valid && csm_array_read_u16(array, &request->db_request.logical_name.class_id);
            valid = valid && csm_array_read_buff(array, &request->db_request.logical_name.obis.A, 6U);
            valid = valid && csm_array_read_u8(array, (uint8_t*)&request->db_request.logical_name.id);

            if (request->db_request.service != SVC_ACTION)
            {
                // GET and SET services can have selective access parameter (option)
                valid = valid && csm_array_read_u8(array, &request->db_request.sel_access.enable);

                if (request->db_request.sel_access.enable)
                {
                    // Retrieve selective access data, user side decoding
                    valid = valid && csm_hal_decode_selective_access(request, array);
                }
            }

            if (request->db_request.service != SVC_GET)
            {
                // SET and ACTION services can have data in the request
                valid = valid && csm_array_read_u8(array, &request->db_request.additional_data.enable);
                // Data is following in the array
            }
        }
        else if (svc_is_next_request(type, request->db_request.service))
        {
            valid = valid && csm_array_read_u32(array, &request->db_request.block_number); // save the invoke ID to reuse the same
        }
    }

    return valid;
}

static csm_db_code svc_get_request_decoder(csm_server_context_t *ctx, csm_channel *channel, csm_array *array)
{
    csm_db_code code = CSM_ERR_BAD_ENCODING;

    CSM_LOG("[SVC] Decoding GET.request");

    channel->request.db_request.service = SVC_GET;

    if (svc_decode_request(&channel->request, array))
    {
        if (ctx->db != NULL)
        {
            // Prepare the response
            array->wr_index = 0U;
            CSM_LOG("[SVC] Encoding GET.response");

            int valid = csm_array_write_u8(array, AXDR_GET_RESPONSE);
            valid = valid && csm_array_write_u8(array, SVC_GET_RESPONSE_NORMAL); // default, can be changed by the application
            valid = valid && csm_array_write_u8(array, channel->request.sender_invoke_id);
            valid = valid && csm_array_write_u8(array, 0U); // data result

            // Actually append the data
            if (valid)
            {
                code = ctx->db_access_func(ctx, channel, array, array);

                if (code == CSM_OK_BLOCK)
                {
                    csm_array_set(array, 1U, SVC_GET_RESPONSE_WITH_DATABLOCK);
                }
                // FIXME: update the code according to the DB result
            }
        }
        else
        {
            CSM_ERR("[SVC] Database pointer not set");
            code = CSM_ERR_OBJECT_ERROR;
        }
    }

    if (code != CSM_OK)
    {
        array->wr_index = 0U;
        if (svc_exception_response_encoder(array))
        {
            code = CSM_OK;
        }
        else
        {
            CSM_ERR("[SVC] Internal problem, cannot encore exception response");
        }
    }

    return code;
}


static const uint32_t gResponseNormalHeaderSize = 6U; // Offset where data can be returned for an Action

static csm_db_code svc_set_or_action_decoder(csm_server_context_t *ctx, csm_channel *channel, csm_array *array)
{
    csm_db_code code = CSM_ERR_BAD_ENCODING;

    if (svc_decode_request(&channel->request, array))
    {
        if (ctx->db_access_func != NULL)
        {
            CSM_LOG("[SVC] Encoding SET/ACTION.response");

            // The output data will point to a different area into our working buffer
            // This will help us to encode the data
            csm_array output = *array;
            uint32_t reply_size = 0U;
            output.offset += gResponseNormalHeaderSize; // begin to encode the reply just after the response header
            output.rd_index = 0U;
            output.wr_index = 0U;

            code = ctx->db_access_func(ctx, channel, array, &output);

            reply_size = output.wr_index;

            // Encode the response
            output.offset -= gResponseNormalHeaderSize;
            output.wr_index = 0U;

            uint8_t service_resp = (channel->request.db_request.service == SVC_SET) ? AXDR_SET_RESPONSE : AXDR_ACTION_RESPONSE;
            int valid = csm_array_write_u8(&output, service_resp);
            valid = valid && csm_array_write_u8(&output, 1U); // FIXME: use proper service tag according to service type
            valid = valid && csm_array_write_u8(&output, channel->request.sender_invoke_id);
            valid = svc_data_access_result_encoder(&output, code);

            if (channel->request.db_request.service == SVC_ACTION)
            {
                // Encode additional data if any
                if (reply_size > 0U)
                {
                    valid = valid && csm_array_write_u8(&output, 1U); // presence flag for optional return-parameters
                    valid = valid && csm_array_write_u8(&output, 0U); // Data
                    valid = valid && csm_array_writer_jump(&output, reply_size); // Virtually add the data (already encoded in the buffer)
                }
                else
                {
                    valid = valid && csm_array_write_u8(&output, 0U); // presence flag for optional return-parameters
                }
            }

            // Update size to send to output channel
            array->wr_index = output.wr_index;

            if (!valid)
            {
                code = CSM_ERR_BAD_ENCODING;
            }
            else
            {
                code = CSM_OK;
            }
        }
        else
        {
            CSM_ERR("[SVC][SET] Database pointer not set");
            code = CSM_ERR_OBJECT_ERROR;
        }
    }

    if (code != CSM_OK)
    {
        array->wr_index = 0U;
        if (svc_exception_response_encoder(array))
        {
            code = CSM_OK;
        }
        else
        {
            CSM_ERR("[SVC][SET] Internal problem, cannot encore exception response");
        }
    }
    else
    {
        CSM_ERR("[SVC][SET] Encoding error");
    }

    return code;
}

static csm_db_code svc_set_request_decoder(csm_server_context_t *ctx, csm_channel *channel, csm_array *array)
{
    channel->request.db_request.service = SVC_SET;
    CSM_LOG("[SVC] Decoding SET.request");
    return svc_set_or_action_decoder(ctx, channel, array);
}


static csm_db_code svc_action_request_decoder(csm_server_context_t *ctx, csm_channel *channel, csm_array *array)
{
    channel->request.db_request.service = SVC_ACTION;
    CSM_LOG("[SVC] Decoding ACTION.request");
    return svc_set_or_action_decoder(ctx, channel, array);
}


typedef csm_db_code (*svc_func)(csm_server_context_t *ctx, csm_channel *channel, csm_array *array);


typedef struct
{
    uint8_t tag;
    svc_func decoder;   //!< Used by the server implementation

} csm_service_handler;

static const csm_service_handler services[] =
{
    { AXDR_GET_REQUEST, svc_get_request_decoder },
    { AXDR_SET_REQUEST, svc_set_request_decoder },
    { AXDR_ACTION_REQUEST, svc_action_request_decoder }
};

#define NUMBER_OF_SERVICES (sizeof(services) / sizeof(services[0]))


int csm_server_services_execute(csm_server_context_t *ctx, csm_channel *channel, csm_array *array)
{
    int number_of_bytes = 0;
    // FIXME: test the array size: minimum/maximum data size allowed
    if (ctx->db != NULL)
    {
        uint8_t tag;
        if (csm_array_read_u8(array, &tag))
        {
            for (uint32_t i = 0U; i < NUMBER_OF_SERVICES; i++)
            {
                const csm_service_handler *srv = &services[i];
                if ((srv->tag == tag) && (srv->decoder != NULL))
                {
                    CSM_LOG("[SVC] Found service");
                    if (srv->decoder(ctx, channel, array) == CSM_OK)
                    {
                        number_of_bytes = array->wr_index;
                    }
                    else
                    {
                        CSM_ERR("[SVC] Encoding error!");
                    }
                    break;
                }
            }
        }
    }
    else
    {
        CSM_ERR("[SVC] DB not initialized!");
    }
    return number_of_bytes;
}

int csm_server_hls_execute(csm_server_context_t *ctx, csm_channel *channel, csm_array *array)
{
    // FIXME: restrict only to the current association object and reply_to_hls_authentication method
    CSM_LOG("[SVC] Received HLS Pass 3 -- FIXME accept only current association object");

    return csm_server_services_execute(ctx, channel, array);
}

 
void csm_server_init(csm_server_context_t *cosem_ctx)
{
    for (uint32_t i = 0U; i < cosem_ctx->asso_list_size; i++)
    {
        csm_asso_init(&cosem_ctx->asso_list[i]);
    }

    for (uint32_t i = 0U; i < cosem_ctx->channel_list_size; i++)
    {
        cosem_ctx->channel_list[i].asso = NULL;
        cosem_ctx->channel_list[i].request.channel_id = INVALID_CHANNEL_ID;
    }
}

int csm_server_execute(csm_server_context_t *cosem_ctx, uint8_t channel, csm_array *packet)
{
    int ret = FALSE;

    if ((cosem_ctx->channel_list == NULL) ||
        (cosem_ctx->asso_list == NULL) ||
        (cosem_ctx->asso_conf_list == NULL))
    {
        CSM_ERR("[CHAN] Stack is not initialized. Call csm_channel_init() first.");
        return ret;
    }

    uint32_t i = 0U;

    // We have to find the association used by this request
    // Find the valid association.
    for (i = 0U; i < cosem_ctx->asso_list_size; i++)
    {
        if ((cosem_ctx->channel_list[channel].request.llc.ssap == cosem_ctx->asso_conf_list[i].llc.ssap) &&
            (cosem_ctx->channel_list[channel].request.llc.dsap == cosem_ctx->asso_conf_list[i].llc.dsap))
        {
            break;
        }
    }

    if (i < cosem_ctx->asso_list_size)
    {
        // Association found, use this one
        // Link the state with the configuration structure
        cosem_ctx->asso_list[i].config = &cosem_ctx->asso_conf_list[i];
        cosem_ctx->channel_list[channel].asso = &cosem_ctx->asso_list[i];

        uint8_t tag;
        if (csm_array_get(packet, 0U, &tag))
        {
            switch (tag)
            {
            case CSM_ASSO_AARE:
            case CSM_ASSO_AARQ:
            case CSM_ASSO_RLRE:
            case CSM_ASSO_RLRQ:
                ret = csm_asso_server_execute(&cosem_ctx->asso_list[i], packet);
                break;
            default:
                if (cosem_ctx->asso_list[i].state_cf == CF_ASSOCIATED)
                {
                    ret = csm_server_services_execute(cosem_ctx, &cosem_ctx->channel_list[channel], packet);
                }
                else if (cosem_ctx->asso_list[i].state_cf == CF_ASSOCIATION_PENDING)
                {
                    // In case of HLS, we have to access to one attribute
                    ret = csm_server_hls_execute(cosem_ctx, &cosem_ctx->channel_list[channel], packet);
                }
                else
                {
                    CSM_ERR("[CHAN] Association is not open");
                }
                break;
            }
        }
    }
    return ret;
}


uint8_t csm_server_connect(csm_server_context_t *cosem_ctx)
{
    uint8_t chan_id = INVALID_CHANNEL_ID;
    // search for a valid free channel
    // In case of CONN_NEW event, channel parameter is 0 (means invalid)
    for (uint32_t i = 0U; i < cosem_ctx->channel_list_size; i++)
    {
        if (cosem_ctx->channel_list[i].request.channel_id == INVALID_CHANNEL_ID)
        {
            chan_id = i + 1U; // generate a channel id
            cosem_ctx->channel_list[i].request.channel_id = chan_id;
            CSM_LOG("[CHAN] Grant connection to channel %d", chan_id);
            break;
        }
    }

    return chan_id;
}

void csm_server_disconnect(csm_server_context_t *cosem_ctx, uint8_t channel)
{
    if (channel < cosem_ctx->channel_list_size)
    {
        cosem_ctx->channel_list[channel].request.channel_id = INVALID_CHANNEL_ID;
        if (cosem_ctx->channel_list[channel].asso != NULL)
        {
            cosem_ctx->channel_list[channel].asso->state_cf = CF_IDLE;
        }
    }
}
