#include "csm_server.h"
#include "csm_axdr_codec.h"

static const uint32_t gResponseNormalHeaderSize = 6U; // Offset where data can be returned for an Action
static const uint32_t gResponseWithDataBlockHeaderSize = 8U;


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
            request->db_request.next = FALSE;
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
            request->db_request.next = TRUE;
            valid = valid && csm_array_read_u32(array, &request->db_request.block_number); // save the invoke ID to reuse the same
        }
    }

    return valid;
}

static csm_db_code svc_get_request_decoder(csm_server_context_t *ctx)
{
    csm_db_code code = CSM_ERR_BAD_ENCODING;

    CSM_LOG("[SVC] Decoding GET.request");

    ctx->request.db_request.service = SVC_GET;
    csm_array *in = &ctx->asso.rx;
    csm_array *out = &ctx->asso.tx;

    // Prepare the intermediate buffer
    csm_array_reset(&ctx->asso.scratch);

    if (svc_decode_request(&ctx->request, in))
    {
        if (ctx->db != NULL)
        {
            CSM_LOG("[SVC] Encoding GET.response");

            // if attribute 1 (logical name) is asked, anwer with the object descriptor
            // else, ask the application to provide the data

            if (ctx->request.db_request.logical_name.id == 1)
            {
                // Encode the object descriptor
                int valid = csm_array_write_u8(out, AXDR_GET_RESPONSE);
                valid = valid && csm_array_write_u8(out, SVC_GET_RESPONSE_NORMAL); // default, can be changed by the application
                valid = valid && csm_array_write_u8(out, ctx->request.sender_invoke_id);
                valid = valid && csm_array_write_u8(out, 0U); // data result
                valid = valid && csm_axdr_wr_octetstring(out, &ctx->request.db_request.logical_name.obis.A, 6U, AXDR_TAG_OCTETSTRING);
                if (valid)
                {
                    code = CSM_OK;
                }
                return code;
            }

            // We pass the scratch buffer as an output array so that we can manage the block transfer easier   
            code = ctx->db_access_func(ctx, in, &ctx->asso.scratch);

            if (code == CSM_OK)
            {
                int valid = csm_array_write_u8(out, AXDR_GET_RESPONSE);
                valid = valid && csm_array_write_u8(out, SVC_GET_RESPONSE_NORMAL); // default, can be changed by the application
                valid = valid && csm_array_write_u8(out, ctx->request.sender_invoke_id);
                valid = valid && csm_array_write_u8(out, 0U); // data result
                valid = valid && csm_array_write_array(out, &ctx->asso.scratch); // append the data
            }
            else if (code == CSM_OK_BLOCK)
            {
                // First loop
                if (ctx->asso.state == CSM_RESPONSE_STATE_START)
                {
                    ctx->asso.current_block = 0U;
                    ctx->asso.state = CSM_RESPONSE_STATE_SENDING;
                }

                // Compute transmit buffer capacity

                // Detect last block:
                //  - when all loops are done
                //  - When there is no more data to read
                uint8_t last_block = FALSE;
                // Compute the maximum size to send to the client
                uint16_t client_max_pdu_size = csm_array_data_size(out);
                if (client_max_pdu_size > ctx->asso.handshake.client_max_receive_pdu_size)
                {
                    client_max_pdu_size = ctx->asso.handshake.client_max_receive_pdu_size;
                }
                client_max_pdu_size -= gResponseWithDataBlockHeaderSize; // remove the header size

                const uint8_t *data = csm_array_rd_current(&ctx->asso.scratch);
                uint32_t size_to_send = csm_array_unread(&ctx->asso.scratch);

                // We determine if the application loop buffer can be send in one block or multiple
                // We can send the data in one block if the sratch buffer is smaller than the client max pdu size
                if (size_to_send <= client_max_pdu_size)
                {
                    // Prepare next time, ask for next loop
                    ctx->asso.state = CSM_RESPONSE_STATE_NEXT_LOOP;
                    // Detect last block boolean state
                    if (ctx->asso.current_loop == ctx->asso.nb_loops)
                    {
                        last_block = TRUE;
                    }
                }
                else
                {
                    // Read a maximum of client_max_pdu_size bytes
                    size_to_send = client_max_pdu_size;
                }
                csm_array_reader_advance(&ctx->asso.scratch, size_to_send); // manually advance the read pointer

                // Header
                int valid = csm_array_write_u8(out, AXDR_GET_RESPONSE);
                valid = valid && csm_array_write_u8(out, SVC_GET_RESPONSE_WITH_DATABLOCK);
                valid = valid && csm_array_write_u8(out, ctx->request.sender_invoke_id);

                /*
                DataBlock-G ::= SEQUENCE -- G == DataBlock for the GET-response
                {
                    last-block                  BOOLEAN,
                    block-number                Unsigned32,
                    result CHOICE
                    {
                        raw-data                    [0] IMPLICIT OCTET STRING,
                        data-access-result          [1] IMPLICIT Data-Access-Result
                    }
                }
                */

                valid = valid && csm_array_write_u8(out, last_block);

                ctx->asso.current_block++;
                valid = valid && csm_array_write_u32(out, ctx->asso.current_block);

                /*
                result CHOICE
                {
                    raw-data   [0] IMPLICIT OCTET STRING,
                    data-access-result  [1] IMPLICIT Data-Access-Result
                }
                */

                
                valid = valid && csm_axdr_wr_octetstring(out, data, size_to_send, 0); // tag = 0 because IMPLICIT   

                
                if (last_block)
                {
                    // auto reset all states
                    ctx->asso.current_block = 0;
                    ctx->asso.state = CSM_RESPONSE_STATE_START;
                }
            }
        }
        else
        {
            CSM_ERR("[SVC] Database pointer not set");
            code = CSM_ERR_OBJECT_ERROR;
        }
    }

    if (code > CSM_OK_BLOCK)
    {
        csm_array_reset(out);
        if (svc_exception_response_encoder(out))
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


static csm_db_code svc_set_or_action_decoder(csm_server_context_t *ctx)
{
    csm_db_code code = CSM_ERR_BAD_ENCODING;
    csm_array *in = &ctx->asso.rx;
    csm_array *out = &ctx->asso.tx;

    if (svc_decode_request(&ctx->request, in))
    {
        if (ctx->db_access_func != NULL)
        {
            CSM_LOG("[SVC] Encoding SET/ACTION.response");

            // The output data will point to a different area into our working buffer
            // This will help us to encode the data
            uint32_t reply_size = 0U;
            out->offset += gResponseNormalHeaderSize; // begin to encode the reply just after the response header
            out->rd_index = 0U;
            out->wr_index = 0U;

            code = ctx->db_access_func(ctx, in, out);

            reply_size = out->wr_index;

            // Encode the response
            out->offset -= gResponseNormalHeaderSize;
            out->wr_index = 0U;

            uint8_t service_resp = (ctx->request.db_request.service == SVC_SET) ? AXDR_SET_RESPONSE : AXDR_ACTION_RESPONSE;
            int valid = csm_array_write_u8(out, service_resp);
            valid = valid && csm_array_write_u8(out, 1U); // FIXME: use proper service tag according to service type
            valid = valid && csm_array_write_u8(out, ctx->request.sender_invoke_id);
            valid = svc_data_access_result_encoder(out, code);

            if (ctx->request.db_request.service == SVC_ACTION)
            {
                // Encode additional data if any
                if (reply_size > 0U)
                {
                    valid = valid && csm_array_write_u8(out, 1U); // presence flag for optional return-parameters
                    valid = valid && csm_array_write_u8(out, 0U); // Data
                    valid = valid && csm_array_writer_advance(out, reply_size); // Virtually add the data (already encoded in the buffer)
                }
                else
                {
                    valid = valid && csm_array_write_u8(out, 0U); // presence flag for optional return-parameters
                }
            }

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
        csm_array_reset(out);
        if (svc_exception_response_encoder(out))
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

static csm_db_code svc_set_request_decoder(csm_server_context_t *ctx)
{
    ctx->request.db_request.service = SVC_SET;
    CSM_LOG("[SVC] Decoding SET.request");
    return svc_set_or_action_decoder(ctx);
}


static csm_db_code svc_action_request_decoder(csm_server_context_t *ctx)
{
    ctx->request.db_request.service = SVC_ACTION;
    CSM_LOG("[SVC] Decoding ACTION.request");
    return svc_set_or_action_decoder(ctx);
}


typedef csm_db_code (*svc_func)(csm_server_context_t *ctx);


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


int csm_server_services_execute(csm_server_context_t *ctx)
{
    int number_of_bytes = 0;
    // FIXME: test the array size: minimum/maximum data size allowed
    if (ctx->db != NULL)
    {
        uint8_t tag;
        csm_array *in = &ctx->asso.rx;
        if (csm_array_read_u8(in, &tag))
        {
            for (uint32_t i = 0U; i < NUMBER_OF_SERVICES; i++)
            {
                const csm_service_handler *srv = &services[i];
                if ((srv->tag == tag) && (srv->decoder != NULL))
                {
                    CSM_LOG("[SVC] Found service");
                    csm_db_code code = srv->decoder(ctx);
                    if ((code == CSM_OK) ||
                        (code == CSM_OK_BLOCK))
                    {
                        number_of_bytes = csm_array_written(&ctx->asso.tx);
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

int csm_server_hls_execute(csm_server_context_t *ctx)
{
    // FIXME: restrict only to the current association object and reply_to_hls_authentication method
    CSM_LOG("[SVC] Received HLS Pass 3 -- FIXME accept only current association object");

    return csm_server_services_execute(ctx);
}

 
int csm_server_execute(csm_server_context_t *ctx, const csm_asso_config *configs, uint32_t number_of_associations, csm_db_t *db, uint32_t number_of_logical_devices)
{
    int ret = FALSE;

    // Find the association
    int association_found = FALSE;
    for (uint32_t i = 0U; i < number_of_associations; i++)
    {
        if ((ctx->request.llc.ssap == configs[i].llc.ssap) &&
            (ctx->request.llc.dsap == configs[i].llc.dsap))
        {
            // Found the association
            ctx->asso.config = &configs[i];
            association_found = TRUE;
            break;
        }
    }

    // Find the logical device
    int logical_device_found = FALSE;
    for (uint32_t i = 0U; i < number_of_logical_devices; i++)
    {
        if (ctx->asso.config->llc.dsap == db[i].logical_device)
        {
            ctx->db = &db[i];
            logical_device_found = TRUE;
            break;
        }
    }

    if (!association_found)
    {
        CSM_ERR("[SVC] Association not found");
        return ret;
    }

    if (!logical_device_found)
    {
        CSM_ERR("[SVC] Logical device not found");
        return ret;
    }

    uint8_t tag;
    if (csm_array_get(&ctx->asso.rx, 0U, &tag))
    {
        switch (tag)
        {
        case CSM_ASSO_AARE:
        case CSM_ASSO_AARQ:
        case CSM_ASSO_RLRE:
        case CSM_ASSO_RLRQ:
            ret = csm_asso_server_execute(&ctx->asso);
            break;
        default:
            if (ctx->asso.state_cf == CF_ASSOCIATED)
            {
                ret = csm_server_services_execute(ctx);
            }
            else if (ctx->asso.state_cf == CF_ASSOCIATION_PENDING)
            {
                // In case of HLS, we have to access to one attribute
                ret = csm_server_hls_execute(ctx);
            }
            else
            {
                CSM_ERR("[CHAN] Association is not open");
            }
            break;
        }
    }

    return ret;
}


