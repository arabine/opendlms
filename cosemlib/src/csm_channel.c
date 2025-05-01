/**
 * A virtual channel of communication with the logical device
 *
 * Copyright (c) 2016, Anthony Rabine
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the MIT license.
 * See LICENSE.txt for more details.
 *
 */

#include "csm_channel.h"
#include "csm_config.h"
#include "csm_security.h"
#include "csm_axdr_codec.h"

int csm_channel_hls_pass3(csm_array *array, csm_channel *channel)
{
    csm_sec_control_byte sc;
    uint32_t ic;
    int ret = FALSE;

    csm_array_dump(array);

    // Save SC and IC
    csm_array_read_u8(array, &sc.sh_byte);
    csm_array_read_u32(array, &ic);

    // Remaining data should be the TAG
    uint32_t unread = csm_array_unread(array);

    if (unread == 12U)
    {
        uint32_t offset = array->offset; // Save the original offset

        if (offset >= CSM_DEF_MAX_HLS_SIZE)
        {
            csm_asso_state *asso = channel->asso;

            // Reserve memory & prepare packet
            array->offset = (offset + array->rd_index) - (CSM_DEF_SEC_HDR_SIZE + asso->handshake.stoc.size);
            array->rd_index = 0U;
            array->wr_index = 0U;

            // Build a new fake packet with: SC || IC || Information || Tag
            // Tag is left untouched, other data are appended just before
            csm_array_write_u8(array, sc.sh_byte);
            csm_array_write_u32(array, ic);
            csm_array_write_buff(array, &asso->handshake.stoc.value[0], asso->handshake.stoc.size);
            csm_array_writer_jump(array, 12U); // Add the tag (already in the buffer)

            csm_sec_result res = csm_sec_auth_decrypt(array, &channel->request, &asso->client_app_title[0]);

            array->offset = offset; // Restore original offset

            if (res == CSM_SEC_OK)
            {
                CSM_LOG("[CHAN] HLS Pass 3 success!");
                ret = TRUE;
            }
            else
            {
                CSM_ERR("[CHAN] Bad tag");
            }
        }
        else
        {
            CSM_ERR("[CHAN] Array too small for HLS");
        }
    }
    else
    {
        CSM_ERR("[CHAN] Bad HLS Pass3 size");
    }

    return ret;
}

int csm_channel_hls_pass4(csm_array *array, csm_channel *channel)
{
    int ret = FALSE;
    // Output buffer state before function call
    //    | offset |
    // rd           ^
    // wr           ^


    // Buffer contents prepared for security processing
    //    | reduced offset | Information (CtoS) | T
    // rd                   ^
    // wr                                        ^ (tag is appended)

    // Output buffer state at the end of the function call
    //    | offset |  OctetString | SC | IC | T |
    // rd           ^
    // wr                                        ^

    csm_sec_control_byte sc;
    sc.sh_byte = 0U;
    sc.sh_bit_field.authentication = 1U; // Turn on only authentication

    uint32_t ic = 0x01234567U; // FIXME: get the IC from the vital data manager
    uint32_t offset = array->offset; // save offset

    if (offset >= CSM_DEF_MAX_HLS_SIZE)
    {
        csm_asso_state *asso = channel->asso;       

        array->offset = offset - (asso->handshake.ctos.size - CSM_DEF_SEC_HDR_SIZE - 2U); // 2U is the OctetString encoding
        // Write information data to authenticate
        csm_array_write_buff(array, &asso->handshake.ctos.value[0], asso->handshake.ctos.size);

        csm_sec_result res = csm_sec_auth_encrypt(array, &channel->request, csm_sys_get_system_title(), sc, ic);

        array->offset = offset; // restore offset
        array->wr_index = 0;

        int valid = csm_array_write_u8(array, AXDR_TAG_OCTETSTRING);
        valid = valid && csm_ber_write_len(array, 17U);
        valid = valid && csm_array_write_u8(array, sc.sh_byte);
        valid = valid && csm_array_write_u32(array, ic);
        valid = valid && csm_array_writer_jump(array, 12U);

        if ((res == CSM_SEC_OK) && valid)
        {
            CSM_LOG("[CHAN] HLS Pass 4 success!");
            ret = TRUE;
        }
        else
        {
            CSM_ERR("[CHAN] HLS Pass 4 failure");
        }
    }
    else
    {
        CSM_ERR("[CHAN] Array too small for HLS pass 4");
    }

    return ret;
}


