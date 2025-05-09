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

#include "csm_client.h"
#include "csm_axdr_codec.h"


int svc_is_valid_data_access_result(uint8_t result)
{
    int valid = FALSE;
    switch (result)
    {
        case CSM_ACCESS_RESULT_SUCCESS:
        case CSM_ACCESS_RESULT_HARDWARE_FAULT:
        case CSM_ACCESS_RESULT_TEMPORARY_FAILURE:
        case CSM_ACCESS_RESULT_READ_WRITE_DENIED:
        case CSM_ACCESS_RESULT_OBJECT_UNDEFINED:
        case CSM_ACCESS_RESULT_OBJECT_CLASS_INCONSISTENT:
        case CSM_ACCESS_RESULT_OBJECT_UNAVAILABLE:
        case CSM_ACCESS_RESULT_TYPE_UNMATCHED:
        case CSM_ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED:
        case CSM_ACCESS_RESULT_DATA_BLOCK_UNAVAILABLE:
        case CSM_ACCESS_RESULT_LONG_GET_ABORTED:
        case CSM_ACCESS_RESULT_NO_LONG_GET_IN_PROGRESS:
        case CSM_ACCESS_RESULT_LONG_SET_ABORTED:
        case CSM_ACCESS_RESULT_NO_LONG_SET_IN_PROGRESS:
        case CSM_ACCESS_RESULT_DATA_BLOCK_NUMBER_INVALID:
        case CSM_ACCESS_RESULT_OTHER_REASON:
            valid = TRUE;
            break;
        default:
            break;
    }
    return valid;
}

int svc_is_valid_action_result(uint8_t result)
{
    int valid = FALSE;
    switch (result)
    {
        case CSM_ACTION_RESULT_SUCCESS:
        case CSM_ACTION_RESULT_HARDWARE_FAULT:
        case CSM_ACTION_RESULT_TEMPORARY_FAILURE:
        case CSM_ACTION_RESULT_READ_WRITE_DENIED:
        case CSM_ACTION_RESULT_OBJECT_UNDEFINED:
        case CSM_ACTION_RESULT_OBJECT_CLASS_INCONSISTENT:
        case CSM_ACTION_RESULT_OBJECT_UNAVAILABLE:
        case CSM_ACTION_RESULT_TYPE_UNMATCHED:
        case CSM_ACTION_RESULT_SCOPE_OF_ACCESS_VIOLATED:
        case CSM_ACTION_RESULT_DATA_BLOCK_UNAVAILABLE:
        case CSM_ACTION_RESULT_LONG_ACTION_ABORTED:
        case CSM_ACTION_RESULT_NO_LONG_ACTION_IN_PROGRESS:
        case CSM_ACTION_RESULT_OTHER_REASON:
            valid = TRUE;
            break;
        default:
            break;
    }
    return valid;
}


int svc_is_normal_response(uint8_t type)
{
    int istype = FALSE;

    // Same type for SET/GET/ACTION
    if (type == 1U)
    {
        istype = TRUE;
    }

    return istype;
}

int svc_is_data_block_response(uint8_t type)
{
    int istype = FALSE;

    // Same type for SET/GET/ACTION
    if (type == 2U)
    {
        istype = TRUE;
    }

    return istype;
}


int svc_result_decoder(csm_response *response, csm_array *array)
{
    uint8_t result;
    int valid = csm_array_read_u8(array, &result);

    // for now, set it for everyone (default values)
    response->has_data = FALSE;
    response->access_result = CSM_ACCESS_RESULT_NOT_SET;

    if (response->service == SVC_ACTION)
    {
        // Action is a SET followed by a GET
        // So we have two statuses: the Action-Result (for the SET part) and the Get-Data-Result (for the GET part)

        // C7 ActionResponse
        // 01 Normal
        // C1 Invoke id
        // 00 Result success
        // 01 Get-Data-Result = true
        // 01 Data-Access-Result (0 == Data)
        // 0B  OBJECT_UNAVAILABLE

/*
        // FIXME: support this?

        ACTION test:

        C701C1FA00
        <ActionResponse>
          <ActionResponseNormal>
            <InvokeIdAndPriority Value="C1" />
            <Result Value="OtherReason" />
          </ActionResponseNormal>
        </ActionResponse>

        // In case of data:
         C7 01 C1

           00 Action result Success
           01 Access-Data result
           00 It is Data
               09 20 619446AE4664788D7C2FF7517C53C0CD44F7B4A5EF3BE9CA08B8A262FE985924

    // OTHER :
      C701C1FA00

<ActionResponse>
  <ActionResponseNormal>
    <InvokeIdAndPriority Value="C1" />
    <Result Value="OtherReason" />
    <ReturnParameters>
      <Result Value="OtherReason" />
    </ReturnParameters>
  </ActionResponseNormal>
</ActionResponse>


*/


        valid = valid && svc_is_valid_action_result(result);
        if (valid)
        {
            response->action_result = (csm_action_result)result;
            CSM_LOG("[SVC] Decoded service result");

            // Any Additional information for the GET part?
            valid = valid && csm_array_read_u8(array, &result);
            if (valid)
            {
                if (result == 1U)
                {
                    // Yes, there is a data result
                    valid = valid && svc_is_valid_data_access_result(result);

                    if (valid)
                    {
                        // Data or Data-Access-Result?
                        valid = valid && csm_array_read_u8(array, &result);

                        if (result == 1U)
                        {
                            response->access_result = (csm_data_access_result)result;
                            CSM_LOG("[SVC] Decoded access result");
                        }
                        else if (result == 0U)
                        {
                            // Next bytes are the data
                            response->has_data = TRUE;
                            response->access_result = CSM_ACCESS_RESULT_SUCCESS;
                            CSM_LOG("[SVC] Found data");
                        }
                        else
                        {
                            valid = FALSE;
                        }
                    }
                }
                else if (result != 0U)
                {
                    valid = FALSE;
                    CSM_ERR("[SVC] Bad Get-Data-Result value");
                }
            }
        }

    }
    else
    {
        valid = valid && svc_is_valid_data_access_result(result);
        if (valid)
        {
            response->access_result = (csm_data_access_result)result;
        }
        else
        {
            response->access_result = CSM_ACCESS_RESULT_NOT_SET;
        }
    }
    return valid;
}


//
// Get-Response-WithDataBlock: C4 02 C1 0000000001008201F40601001B0800FF15000000000
static int svc_response_decoder(csm_response *response, csm_array *array)
{
    uint8_t type = 0U;
    int valid = csm_array_read_u8(array, &type);
    valid = valid && csm_array_read_u8(array, &response->invoke_id);

    if (valid)
    {
        if (svc_is_normal_response(type))
        {
            response->type = SVC_RESPONSE_NORMAL;
            CSM_LOG("[SVC] Response-Normal");
            valid = valid && svc_result_decoder(response, array);
        }
        else if (svc_is_data_block_response(type))
        {
            response->type = SVC_RESPONSE_WITH_DATABLOCK;
            CSM_LOG("[SVC] Response-WithDataBlock");
            valid = valid && csm_array_read_u8(array, &response->last_block);
            valid = valid && csm_array_read_u32(array, &response->block_number);

            if (valid)
            {
                CSM_TRACE("[SVC/GET] Block number: %d\r\n", response->block_number);
                response->access_result = CSM_ACCESS_RESULT_SUCCESS;
            }

            // Now the read pointer is on the data
        }
        else
        {
            CSM_LOG("[SVC] Service not supported");
        }
    }

    return valid;
}

static int svc_get_response_decoder(csm_response *response, csm_array *array)
{
    response->service = SVC_GET;
    CSM_LOG("[SVC] Decoding GET.response");
    return svc_response_decoder(response, array);
}

static int svc_set_response_decoder(csm_response *response, csm_array *array)
{
    response->service = SVC_SET;
    CSM_LOG("[SVC] Decoding SET.response");
    return svc_response_decoder(response, array);
}

static int svc_action_response_decoder(csm_response *response, csm_array *array)
{
    response->service = SVC_ACTION;
    CSM_LOG("[SVC] Decoding ACTION.response");
    return svc_response_decoder(response, array);
}


/*
Get request with selective access by range (date)

 7EA04F000200230332BDE2E6E600
C0 01 C1 0007 00 01 62 02 00 FF 02
  01 Use sel access
    01 by range
       02 04 structure of 4

            02 04
                    12 0008 class id
                    09 06 00 00 01 00 00 FF
                    0F 02 attribute index
                    12 0000  data_index

                09 0C 07E1080107000000FFFFC400
                09 0C 07E10B0107000000FFFFC400

                01 00 selected_values (array null)

4CD77E


Selective access with undefined end date

 C0 01 41 0007 00 00 63 01 00 FF 02

   01
     01
        02 04
             02 04
                  12 0008
                  09 06 00 00 01 00 00 FF
                  0F 02
                  12 0000

             09 0C 07E10B1BFF000000FF800000
             09 0C FFFFFFFFFFFFFFFFFF8000FF

             0100


*/

int csm_client_encode_selective_access_by_range(csm_array *array, csm_object_t *restricting_object, csm_array *start, csm_array *end)
{
    int valid = csm_array_write_u8(array, 1U); // by range
    valid = valid && csm_array_write_u8(array, AXDR_TAG_STRUCTURE);
    valid = valid && csm_ber_write_len(array, 4U);

    // 1. restricting_object
    valid = valid && csm_axdr_wr_capture_object(array, restricting_object);

    // 2. start date
    valid = valid && csm_axdr_wr_octetstring(array, csm_array_rd_current(start), csm_array_written(start));
    // 3. end date
    valid = valid && csm_axdr_wr_octetstring(array, csm_array_rd_current(end), csm_array_written(end));

    // 4. selected values
    valid = valid && csm_array_write_u8(array, 0x01U); // selected values
    valid = valid && csm_array_write_u8(array, 0x00U); // array null

    return valid;
}


static int svc_exception_decoder(csm_response *response, csm_array *array)
{
    response->service = SVC_EXCEPTION;
    CSM_LOG("[SVC] Decoding Exception");

    int valid = csm_array_read_u8(array, &response->exception.state_err);
    valid = valid && csm_array_read_u8(array, &response->exception.service_err);
    return valid;
}


typedef int (*srv_resp_decoder_func)(csm_response *response, csm_array *array);

typedef struct
{
    uint8_t tag;
    srv_resp_decoder_func decoder;

} csm_client_service_handler;

static const csm_client_service_handler client_services[] =
{
    { AXDR_GET_RESPONSE, svc_get_response_decoder },
    { AXDR_SET_RESPONSE, svc_set_response_decoder },
    { AXDR_ACTION_RESPONSE, svc_action_response_decoder },
    { AXDR_EXCEPTION_RESPONSE, svc_exception_decoder }
};

#define NUMBER_OF_CLIENT_SERVICES (sizeof(client_services) / sizeof(client_services[0]))

int csm_client_decode(csm_response *response, csm_array *array)
{
    int valid = FALSE;
    uint8_t tag;

    response->service = SVC_UNKOWN;
    if (csm_array_read_u8(array, &tag))
    {
        for (uint32_t i = 0U; i < NUMBER_OF_CLIENT_SERVICES; i++)
        {
            const csm_client_service_handler *svc = &client_services[i];
            if ((svc->tag == tag) && (svc->decoder != NULL))
            {
                valid = svc->decoder(response, array);
                break;
            }
        }
    }

    return valid;
}

void csm_client_init(csm_request *request, csm_response *response)
{
    (void) request;

    response->type = 0U;
    response->block_number = 0U;
    response->invoke_id = 0U;
    response->last_block = 0U;
}

int csm_client_has_more_data(csm_response *response)
{
    int more_data = 0;
    if ((response->type == SVC_RESPONSE_WITH_DATABLOCK) &&
        (!response->last_block))
    {
        more_data = 1;
    }
    return more_data;
}

uint8_t csm_get_request_type(csm_request *request)
{
    uint8_t type = 0U; // Bad type

    // FIXME: set different values according to ASN.1 description and services
    if (request->type == SVC_REQUEST_NORMAL)
    {
        type = 1U;
    }
    else
    {
        // Next
        type = 2U;
    }

    return type;
}


/*
Action request (Here, MD5 pass 3):

C3 01 41

  000F
  00 00 28 00 00 FF

  01 // method 1

  01 // Have Data (true)

  0910 41C845AEEA55D9C9CDE708AF0BA5B4BD

 */

 int svc_request_encoder(csm_request *request, csm_array *array)
 {
     uint8_t tag = (request->db_request.service == SVC_GET) ? AXDR_GET_REQUEST :  (request->db_request.service == SVC_SET) ? AXDR_SET_REQUEST : AXDR_ACTION_REQUEST;
     int valid = csm_array_write_u8(array, tag);
     valid = valid && csm_array_write_u8(array, csm_get_request_type(request));
 
     if (request->type == SVC_REQUEST_NORMAL)
     {
         valid = valid && csm_array_write_u8(array, request->sender_invoke_id);
         valid = valid && csm_array_write_u16(array, request->db_request.logical_name.class_id);
         valid = valid && csm_array_write_buff(array, (const uint8_t *)&request->db_request.logical_name.obis.A, 6U);
         valid = valid && csm_array_write_u8(array, request->db_request.logical_name.id);
 
         // Additional data only for SET and ACTION
 
         if (tag != AXDR_ACTION_REQUEST)
         {
             valid = valid && csm_array_write_u8(array, request->db_request.sel_access.enable); // use selective access or not
 
             if (request->db_request.sel_access.enable)
             {
                 if (request->db_request.sel_access.data.buff != NULL)
                 {
                     valid = valid && csm_array_write_buff(array, request->db_request.sel_access.data.buff, csm_array_written(&request->db_request.sel_access.data));
                 }
                 else
                 {
                     valid = FALSE;
                 }
             }
         }
         else
         {
             // Data is optional for ACTION
             valid = valid && csm_array_write_u8(array, request->db_request.additional_data.enable); // use data or not
         }
 
         // Add data, valid for ACTION or SET only
         if (tag != AXDR_GET_REQUEST)
         {
             uint32_t data_size = csm_array_written(&request->db_request.additional_data.data);
             if ((request->db_request.additional_data.enable) && (data_size > 0U))
             {
                 if (request->db_request.additional_data.data.buff != NULL)
                 {
                     valid = valid && csm_array_write_buff(array, request->db_request.additional_data.data.buff, csm_array_written(&request->db_request.additional_data.data));
                 }
                 else
                 {
                     valid = FALSE;
                 }
             }
             else
             {
                 if (tag == AXDR_SET_REQUEST)
                 {
                     // No data (NULL)
                     valid = valid && csm_array_write_u8(array, AXDR_TAG_NULL);
                 }
             }
         }
 
     }
     else if (request->type == SVC_REQUEST_NEXT)
     {
         valid = valid && csm_array_write_u8(array, request->sender_invoke_id); // save the invoke ID to reuse the same
         valid = valid && csm_array_write_u32(array, request->db_request.block_number); // save the invoke ID to reuse the same
     }
     else
     {
         CSM_LOG("[SVC] Service not supported");
     }
     return valid;
 }
 
