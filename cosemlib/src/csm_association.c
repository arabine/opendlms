/**
 * Implementation of the Cosem ACSE services
 *
 * Copyright (c) 2016, Anthony Rabine
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the MIT license.
 * See LICENSE.txt for more details.
 *
 */

#include "csm_array.h"
#include "csm_association.h"
#include "string.h"
#include "csm_axdr_codec.h"
#include "csm_security.h"

// Since this is part of a Cosem stack, simplify the decoding to lower code & RAM ;
// Instead of performing a real decoding, just compare the memory as it is always the same
static const uint8_t cOidHeader[] = {0x60U, 0x85U, 0x74U, 0x05U, 0x08U};

// Object identifier names
#define APP_CONTEXT_NAME            1U
#define SECURITY_MECHANISM_NAME     2U


typedef enum
{
    CSM_ACSE_ERR = 0U, ///< Encoding/decoding is NOT good, stop here if it is required
    CSM_ACSE_OK = 1U    ///< Encoding/decoding is good, we can continue

} csm_acse_code;

typedef csm_acse_code (*codec_func)(csm_asso_state *state, csm_ber *ber, csm_array *array);

enum acse_context
{
    ACSE_SKIP,  //!< Never decode/encode
    ACSE_ALWAYS,   //!< Always decode/encode
    ACSE_OPT,   //!< Optional, skiped if not exists
    ACSE_SEC,   //!< When use ciphered authentication
};

typedef struct
{
    uint8_t tag;
    uint8_t context;    //!< Requirement on the context
    codec_func extract_func;
} csm_asso_dec;

typedef struct
{
    uint8_t tag;
    uint8_t context;    //!< Requirement on the context
    codec_func insert_func;
} csm_asso_enc;

// -------------------------------   DECODERS   ------------------------------------------

static csm_acse_code acse_proto_version_decoder(csm_asso_state *asso, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;

    CSM_LOG("[ACSE] Found Protocol version tag");

    // we support only version1 of the protocol
    if (ber->length.length == 2U)
    {
        uint8_t version, unused_bits;
        if (csm_array_read_u8(array, &unused_bits))
        {
            if (csm_array_read_u8(array, &version))
            {
                // if ((unused_bits == 7U) && (version == 0x80U))
                if ((version&0x80) == 0x80U)
                {
                    ret = CSM_ACSE_OK;
                }
                else
                {
                    asso->handshake.failure_type = CSM_ASSO_FAIL_BAD_PROTOCOL_VERSION;
                }
            }
        }
    }

    return ret;
}


static csm_acse_code acse_app_context_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) state;
    (void) array;

    CSM_LOG("[ACSE] Found APPLICATION CONTEXT tag");

    // the length of the object identifier must be 7 bytes + 2 bytes for the BER header = 9 bytes
    if (ber->length.length == 9U)
    {
        ret = CSM_ACSE_OK;
    }
    else
    {
        CSM_ERR("[ACSE] Bad object identifier size");
    }

    return ret;
}

static csm_acse_code acse_oid_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) ber;

    CSM_LOG("[ACSE] Found OBJECT IDENTIFIER tag");

    ber_object_identifier oid;
    oid.header = cOidHeader;
    oid.size = 5U;

    if (csm_ber_decode_object_identifier(&oid, array) == TRUE)
    {
        // in the case of LN referencing, with no ciphering: 2, 16, 756, 5, 8, 1, 1;
        // in the case of SN referencing, with no ciphering: 2, 16, 756, 5, 8, 1, 2;

        if ((oid.name == APP_CONTEXT_NAME))
        {
            switch (oid.id)
            {
            case LN_REF:
                state->ref = LN_REF;
                ret = CSM_ACSE_OK;
                CSM_LOG("[ACSE] LogicalName referencing");
                break;
            case SN_REF:
                state->ref = SN_REF;
                ret = CSM_ACSE_OK;
                CSM_LOG("[ACSE] ShortName referencing");
                break;
            case LN_REF_WITH_CYPHERING:
                state->ref = LN_REF_WITH_CYPHERING;
                ret = CSM_ACSE_OK;
                CSM_LOG("[ACSE] LogicalName referencing with ciphering");
                break;

            case SN_REF_WITH_CYPHERING:
                state->ref = SN_REF_WITH_CYPHERING;
                ret = CSM_ACSE_OK;
                CSM_LOG("[ACSE] ShortName referencing with ciphering");
                break;

            default:
                CSM_LOG("[ACSE] Referencing not supported");
                break;
            }
        }
        // in the case of low-level-security: 2, 16, 756, 5, 8, 2, 1;
        // in the case of high-level-security (5): 2, 16, 756, 5, 8, 2, 5;
        else if ((oid.name == SECURITY_MECHANISM_NAME))
        {
            switch (oid.id)
            {
                case CSM_AUTH_LOW_LEVEL:
                    state->auth_level = CSM_AUTH_LOW_LEVEL;
                    ret = CSM_ACSE_OK;
                    CSM_LOG("[ACSE] Low level (LLS) authentication");
                    break;
                case CSM_AUTH_HIGH_LEVEL:
                    state->auth_level = CSM_AUTH_HIGH_LEVEL;
                    ret = CSM_ACSE_OK;
                    CSM_LOG("[ACSE] High level 2 (Manufacturer) authentication");
                    break;
                case CSM_AUTH_HIGH_LEVEL_MD5:
                    state->auth_level = CSM_AUTH_HIGH_LEVEL_MD5;
                    ret = CSM_ACSE_OK;
                    CSM_LOG("[ACSE] High level 3 (MD5) authentication");
                    break;
                case CSM_AUTH_HIGH_LEVEL_SHA1:
                    state->auth_level = CSM_AUTH_HIGH_LEVEL_SHA1;
                    ret = CSM_ACSE_OK;
                    CSM_LOG("[ACSE] High level 4 (SHA1) authentication");
                    break;
                case CSM_AUTH_HIGH_LEVEL_GMAC:
                    state->auth_level = CSM_AUTH_HIGH_LEVEL_GMAC;
                    ret = CSM_ACSE_OK;
                    CSM_LOG("[ACSE] High level 5 (GMAC) authentication");
                    break;
                case CSM_AUTH_HIGH_LEVEL_SHA256:
                    state->auth_level = CSM_AUTH_HIGH_LEVEL_SHA256;
                    ret = CSM_ACSE_OK;
                    CSM_LOG("[ACSE] High level 6 (SHA256) authentication");
                    break;
                default:
                    CSM_LOG("[ACSE] Authentication level not supported");
                    break;
            }
        }
    }
    else
    {
        CSM_ERR("[ACSE] Bad Object Identifier contents or size");
    }

    return ret;
}


static csm_acse_code acse_req_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) state;

    CSM_LOG("[ACSE] Found sender requirements tag");

    if (ber->length.length == 2U)
    {
        // encoding of the authentication functional unit (0)
        // NOTE The number of bits coded may vary from client to client, but
        // within the COSEM environment, only bit 0 set to 1 (indicating the
        // requirement of the authentication functional unit) is to be respected.
        uint8_t byte;
        if (csm_array_read_u8(array, &byte))
        {
            // encoding of the number of unused bits in the last byte of the BIT STRING
            if (byte == 0x07U)
            {
                if (csm_array_read_u8(array, &byte))
                {
                    if (byte == 0x80U)
                    {
                        ret = CSM_ACSE_OK;
                    }
                }
            }
        }
    }
    else
    {
        CSM_ERR("[ACSE] Sender requirements bad size");
    }

    return ret;
}

static csm_acse_code acse_auth_value_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array, uint8_t tag)
{
    csm_acse_code ret = CSM_ACSE_ERR;

    CSM_LOG("[ACSE] Found authentication value tag");
    if (csm_ber_decode(ber, array))
    {
        // Can be a challenge or a LLS
        if ((ber->length.length >= (CSM_DEF_LLS_MIN_SIZE)) &&
            (ber->length.length <= (CSM_DEF_CHALLENGE_SIZE)))
        {
            if (ber->tag.tag == (TAG_CONTEXT_SPECIFIC))
            {

                if (tag == CSM_ASSO_CALLING_AUTH_VALUE)
                {
                    // It is a GraphicString, the size is dynamic
                    if (csm_array_read_buff(array, &state->handshake.ctos.value[0], ber->length.length))
                    {
                        state->handshake.ctos.size = ber->length.length;
                        ret = CSM_ACSE_OK;
                    }
                }
                else
                {
                    // It is a GraphicString, the size is dynamic
                    if (csm_array_read_buff(array, &state->handshake.stoc.value[0], ber->length.length))
                    {
                        state->handshake.stoc.size = ber->length.length;
                        ret = CSM_ACSE_OK;
                    }
                }
            }
        }

        if (ret == CSM_ACSE_ERR)
        {
            CSM_ERR("[ACSE] Bad authentication value size");
        }
    }
    else
    {
        CSM_ERR("[ACSE] Bad authentication value format");
    }

    return ret;
}

static csm_acse_code acse_client_auth_value_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    return acse_auth_value_decoder(state, ber, array, CSM_ASSO_CALLING_AUTH_VALUE);
}

static csm_acse_code acse_server_auth_value_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    return acse_auth_value_decoder(state, ber, array, CSM_ASSO_RESP_AUTH_VALUE);
}

int acse_decode_conformance_block(csm_asso_state *state, csm_ber *ber, csm_array *array, uint8_t tag)
{
    // conformance, [APPLICATION 31] IMPLICIT BIT STRING
    // encoding of the [APPLICATION 31] tag (ASN.1 explicit tag)
    int valid = csm_ber_decode(ber, array);
    if ((ber->tag.tag == 0x5FU) && (ber->tag.ext == 0x1FU))
    {
        if (ber->length.length == 4U)
        {
            uint8_t byte;
            valid = valid && csm_array_read_u8(array, &byte);
            valid = valid && (byte == 0U ? TRUE : FALSE); // unused bits in the bitstring

            state->handshake.proposed_conformance = ((uint32_t)byte) << 16U;
            valid = valid && csm_array_read_u8(array, &byte);
            state->handshake.proposed_conformance += ((uint32_t)byte) << 8U;
            valid = valid && csm_array_read_u8(array, &byte);
            state->handshake.proposed_conformance += ((uint32_t)byte);

            uint16_t pdu_size;
            valid = valid && csm_array_read_u16(array, &pdu_size);
            if (tag == AXDR_INITIATE_REQUEST)
            {
                state->handshake.client_max_receive_pdu_size = pdu_size;
            }
            else
            {
                state->handshake.server_max_receive_pdu_size = pdu_size;
            }
        }
    }
    else
    {
        valid = 0U;
    }

    return valid;
}

static csm_acse_code acse_user_info_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array, uint8_t tag)
{
    csm_acse_code ret = CSM_ACSE_ERR;

    CSM_LOG("[ACSE] Found user info tag");
    if (csm_ber_decode(ber, array))
    {
        if (ber->tag.id == CSM_BER_TYPE_OCTET_STRING)
        {
            // Now decode the A-XDR encoded packet
            uint8_t byte;
            int valid = TRUE;
            if(csm_array_read_u8(array, &byte))
            {
                if (byte == AXDR_INITIATE_REQUEST)
                {
                    CSM_LOG("[ACSE] Found xDLMS InitiateRequest encoded APDU");
                    if(csm_array_read_u8(array, &byte))
                    {
                        if (byte != AXDR_TAG_NULL)
                        {
                            // FIXME: copy the dedicated key
                        }

                        valid = csm_axdr_rd_null(array); //  response-allowed
                    }
                    else
                    {
                        valid = FALSE;
                    }
                }
                else if (byte == AXDR_INITIATE_RESPONSE)
                {
                    CSM_LOG("[ACSE] Found xDLMS InitiateResponse encoded APDU");
                }
                else
                {
                    CSM_ERR("[ACSE] Bad InitiateRequest tag");
                    valid = FALSE;
                }

                if (valid)
                {
                    // proposed_quality_of_service
                    uint8_t byte = 0xFFU;
                    valid = csm_array_read_u8(array, &byte);
                    if (valid)
                    {
                        if (byte == 1U)
                        {
                            valid = csm_array_read_u8(array, &byte);
                            CSM_LOG("QoS used, value=0x%X", byte);
                        }
                        else
                        {
                            CSM_LOG("QoS not used.");
                        }
                    }

                    // proposed-dlms-version-number: always 6
                    valid = valid && csm_array_read_u8(array, &byte);
                    valid = valid && (byte == 6U ? TRUE : FALSE);

                    valid = valid && acse_decode_conformance_block(state, ber, array, tag);

                    if (byte == AXDR_INITIATE_RESPONSE)
                    {
                        // VAA-name
                        uint16_t vaaname;
                        if (csm_array_read_u16(array, &vaaname))
                        {
                            valid = valid && (vaaname == 0x0007U);
                        }
                    }
                }

                if (valid)
                {
                    ret = CSM_ACSE_OK;
                }
            }
        }
    }

    return ret;
}


static csm_acse_code acse_initiate_request_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    return acse_user_info_decoder(state, ber, array, AXDR_INITIATE_REQUEST);
}

static csm_acse_code acse_initiate_response_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    return acse_user_info_decoder(state, ber, array, AXDR_INITIATE_RESPONSE);
}

static csm_acse_code acse_system_title_decoder(csm_ber *ber, csm_array *array, uint8_t *buffer)
{
    csm_acse_code ret = CSM_ACSE_ERR;

    CSM_LOG("[ACSE] Found AP-Title tag");
    if (csm_ber_decode(ber, array))
    {
        // Can be a challenge or a LLS
        if (ber->length.length == CSM_DEF_APP_TITLE_SIZE)
        {
            if (ber->tag.id == CSM_BER_TYPE_OCTET_STRING)
            {
                // Store the AP-Title in the association context
                if (csm_array_read_buff(array, buffer, CSM_DEF_APP_TITLE_SIZE))
                {
                    ret = CSM_ACSE_OK;
                }
            }
        }

        if (ret == CSM_ACSE_ERR)
        {
            CSM_ERR("[ACSE] Bad AP-Title size");
        }
    }
    else
    {
        CSM_ERR("[ACSE] Bad AP-Title format");
    }

    return ret;
}


static csm_acse_code acse_client_system_title_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    return acse_system_title_decoder(ber, array, state->client_app_title);
}

static csm_acse_code acse_server_system_title_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    return acse_system_title_decoder(ber, array, state->server_app_title);
}

static csm_acse_code acse_result_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) ber;

    CSM_LOG("[ACSE] Decoding result tag ...");

    uint8_t result = 0U; // accepted
    if (csm_ber_read_u8(array, &result))
    {
        ret = CSM_ACSE_OK;

        if (result == 0U)
        {
            // accepted, connection OK
            state->handshake.accepted = TRUE;
        }
        else
        {
            state->handshake.accepted = FALSE;
        }
    }

    return ret;
}

static csm_acse_code acse_result_src_diag_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;

    CSM_LOG("[ACSE] Decoding result source diagnostic tag ...");

    int valid = csm_ber_decode(ber, array);
    if (valid && (ber->length.length == 3U) && (ber->tag.tag == CSM_ASSO_SERVICE_USER))
    {
        uint8_t result;
        if (csm_ber_read_u8(array, &result))
        {
            state->handshake.result = (enum csm_asso_result)result;
            ret = CSM_ACSE_OK;
        }
    }
    return ret;
}


static csm_acse_code acse_requirements_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) state;
    (void) ber;

    CSM_LOG("[ACSE] Decoding ACSE requirements tag ...");

    if (ber->length.length == 2U)
    {
        uint8_t bits;
        uint8_t val;
        int valid = csm_array_read_u8(array, &bits); // unused bits in the bit-string
        valid = valid && csm_array_read_u8(array, &val);

        if (valid && (bits == 7U) && (val == 0x80U))
        {
            ret = CSM_ACSE_OK;
        }
    }

    return ret;
}

static csm_acse_code acse_skip_decoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    (void) state;
    if (!ber->tag.isPrimitive)
    {
        // This BER contains data that is not managed
        // advance the pointer to the next BER header
        (void) csm_array_reader_advance(array, ber->length.length);
    }
    CSM_LOG("[ACSE] Skipped tag: %d", ber->tag.tag);
    return CSM_ACSE_OK;
}



static const csm_asso_dec aarq_decoder_chain[] =
{
    {CSM_ASSO_PROTO_VER,                ACSE_OPT, acse_proto_version_decoder},
    {CSM_ASSO_APP_CONTEXT_NAME,         ACSE_ALWAYS, acse_app_context_decoder},
    {CSM_BER_TYPE_OBJECT_IDENTIFIER,    ACSE_ALWAYS, acse_oid_decoder},
    // {CSM_ASSO_CALLED_AP_TITLE,          ACSE_SKIP, acse_skip_decoder},
    // {CSM_ASSO_CALLED_AE_QUALIFIER,      ACSE_SKIP, acse_skip_decoder},
    // {CSM_ASSO_CALLED_AP_INVOC_ID,       ACSE_SKIP, acse_skip_decoder},
    // {CSM_BER_TYPE_INTEGER,              ACSE_SKIP, acse_skip_decoder},
    // {CSM_ASSO_CALLED_AE_INVOC_ID,       ACSE_SKIP, acse_skip_decoder},
    // {CSM_BER_TYPE_INTEGER,              ACSE_SKIP, acse_skip_decoder},
    {CSM_ASSO_CALLING_AP_TITLE,         ACSE_OPT, acse_client_system_title_decoder},
    // {CSM_ASSO_CALLING_AE_QUALIFIER,     ACSE_SKIP, acse_skip_decoder},
    // {CSM_ASSO_CALLING_AP_INVOC_ID,      ACSE_SKIP, acse_skip_decoder},
    // {CSM_BER_TYPE_INTEGER,              ACSE_SKIP, acse_skip_decoder},
    // {CSM_ASSO_CALLING_AE_INVOC_ID,      ACSE_SKIP, acse_skip_decoder},
    // {CSM_BER_TYPE_INTEGER,              ACSE_SKIP, acse_skip_decoder},
    {CSM_ASSO_SENDER_ACSE_REQU,         ACSE_OPT, acse_req_decoder},
    {CSM_ASSO_REQ_MECHANISM_NAME,       ACSE_OPT, acse_oid_decoder},
    {CSM_ASSO_CALLING_AUTH_VALUE,       ACSE_OPT, acse_client_auth_value_decoder},
    // {CSM_ASSO_IMPLEMENTATION_INFO,      ACSE_OPT, acse_skip_decoder},
    {CSM_ASSO_USER_INFORMATION,         ACSE_OPT, acse_initiate_request_decoder}
};


static const csm_asso_dec aare_decoder_chain[] =
{
    {CSM_ASSO_PROTO_VER,                ACSE_OPT,       acse_proto_version_decoder},
    {CSM_ASSO_APP_CONTEXT_NAME,         ACSE_ALWAYS,    acse_app_context_decoder},
    {CSM_BER_TYPE_OBJECT_IDENTIFIER,    ACSE_ALWAYS,    acse_oid_decoder},
    {CSM_ASSO_RESULT_FIELD,             ACSE_ALWAYS,    acse_result_decoder},
    {CSM_ASSO_SOURCE_DIAGNOSTIC,          ACSE_ALWAYS,    acse_result_src_diag_decoder},

    // Additional fields specific when ciphered authentication is required
    {CSM_ASSO_RESP_AP_TITLE,            ACSE_SEC,       acse_server_system_title_decoder},
    {CSM_ASSO_RESPONDER_ACSE_REQ,       ACSE_SEC,       acse_requirements_decoder},
    {CSM_ASSO_RESP_MECHANISM_NAME,      ACSE_SEC,       acse_oid_decoder},
    {CSM_ASSO_RESP_AUTH_VALUE,          ACSE_SEC,       acse_server_auth_value_decoder},

    // Final field
    {CSM_ASSO_USER_INFORMATION,         ACSE_ALWAYS,    acse_initiate_response_decoder},
};



#define CSM_ACSE_AARQ_DECODER_CHAIN_SIZE   (sizeof(aarq_decoder_chain)/sizeof(aarq_decoder_chain[0]))
#define CSM_ACSE_AARE_DECODER_CHAIN_SIZE   (sizeof(aare_decoder_chain)/sizeof(aare_decoder_chain[0]))


// -------------------------------   ENCODERS ------------------------------------------

static csm_acse_code acse_app_context_encoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) state;
    (void) ber;

    CSM_LOG("[ACSE] Encoding APPLICATION CONTEXT tag ...");

    // the length of the object identifier must be 7 bytes + 2 bytes for the BER header = 9 bytes
    if (csm_ber_write_len(array, 9U))
    {
        ret = CSM_ACSE_OK;
    }
    return ret;
}

static int acse_oid_encoder(csm_array *array, uint8_t name, uint8_t id)
{
    // the length of the object identifier must be 7 bytes
    int valid = csm_ber_write_len(array, 7U);
    valid = valid && csm_array_write_buff(array, &cOidHeader[0], 5U);
    valid = valid && csm_array_write_u8(array, name);
    valid = valid && csm_array_write_u8(array, id);
    return valid;
}


static csm_acse_code acse_oid_context_encoder(csm_asso_state *asso, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) ber;

    CSM_LOG("[ACSE] Encoding Object Identifier tag ...");

    if (acse_oid_encoder(array, APP_CONTEXT_NAME, (uint8_t)asso->ref))
    {
        ret = CSM_ACSE_OK;
    }
    return ret;
}

/*
Association-result ::=                 INTEGER
{
    accepted                           (0),
    rejected-permanent                 (1),
    rejected-transient                 (2)
}
*/
static csm_acse_code acse_result_encoder(csm_asso_state *asso, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) ber;

    CSM_LOG("[ACSE] Encoding result tag ...");

    uint8_t result = 0U; // accepted
    if ((asso->state_cf == CF_IDLE) || (asso->handshake.failure_type != CSM_ASSO_FAIL_NO_ANY))
    {
        result = 1U; // rejected-permanent
    }

    if (csm_ber_write_len(array, 3U)) // 3 bytes = integer tag, integer length and result boolean
    {
        if (csm_ber_write_u8(array, result))
        {
            ret = CSM_ACSE_OK;
        }
    }
    return ret;
}

static csm_acse_code acse_result_src_diag_encoder(csm_asso_state *asso, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) ber;

    CSM_LOG("[ACSE] Encoding result source diagnostic tag ...");


    uint8_t acse_service_type = CSM_ASSO_SERVICE_USER;
    uint8_t source_diagnostic = CSM_ASSO_ERR_NULL;
    switch (asso->handshake.failure_type)
    {
        case CSM_ASSO_FAIL_BAD_PROTOCOL_VERSION:
            acse_service_type = CSM_ASSO_SERVICE_PROVIDER;
            source_diagnostic = CSM_ASSO_SERVICE_PROVIDER_NO_COMMON_VERSION;
            break;
        default:
            acse_service_type = CSM_ASSO_SERVICE_USER;
            source_diagnostic = CSM_ASSO_ERR_NULL;
    }

    if (csm_ber_write_len(array, 5U))
    {
        if (csm_array_write_u8(array, acse_service_type))
        {
            if (csm_ber_write_len(array, 3U)) // 3 bytes = integer tag, integer length and result boolean
            {
                if (csm_ber_write_u8(array, source_diagnostic))
                {
                    ret = CSM_ACSE_OK;
                }
            }
        }
    }
    return ret;
}

static csm_acse_code acse_resp_system_title_encoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) state;
    (void) ber;

    CSM_LOG("[ACSE] Encoding server AP-Title ...");

    int valid = csm_ber_write_len(array, CSM_DEF_APP_TITLE_SIZE + 2U);
    valid = valid && csm_array_write_u8(array, CSM_BER_TYPE_OCTET_STRING);
    valid = valid && csm_array_write_u8(array, CSM_DEF_APP_TITLE_SIZE);
    valid = valid && csm_array_write_buff(array, csm_sys_get_system_title(), CSM_DEF_APP_TITLE_SIZE);

    if (valid)
    {
        ret = CSM_ACSE_OK;
    }
    return ret;
}

static csm_acse_code acse_requirements_encoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) ber;
    (void) state;

    CSM_LOG("[ACSE] Encoding ACSE requirements tag ...");

    int valid = csm_ber_write_len(array, 2U);
    valid = valid && csm_array_write_u8(array, 7U); // unused bits in the bit-string
    valid = valid && csm_array_write_u8(array, 0x80U);

    if (valid)
    {
        ret = CSM_ACSE_OK;
    }

    return ret;
}

static csm_acse_code acse_oid_mechanism_encoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) ber;

    CSM_LOG("[ACSE] Encoding Object Identifier tag ...");

    if (acse_oid_encoder(array, (uint8_t)SECURITY_MECHANISM_NAME, (uint8_t)state->auth_level))
    {
        ret = CSM_ACSE_OK;
    }
    return ret;
}



static csm_acse_code acse_auth_value_encoder(csm_array *array, uint8_t *value, uint8_t size)
{
    csm_acse_code ret = CSM_ACSE_ERR;

    int valid = csm_ber_write_len(array, size + 2U);
    valid = valid && csm_array_write_u8(array, TAG_CONTEXT_SPECIFIC); // GraphicsString

    // Serialize the server authentication value to the output buffer and in our scratch buffer
    valid = valid && csm_array_write_u8(array, size);
    valid = valid && csm_array_write_buff(array, value, size);

    if (valid)
    {
        ret = CSM_ACSE_OK;
    }
    return ret;
}

#ifdef GB_TEST_VECTORS
char stoc[] = "P6wRJ21F";
#endif

static csm_acse_code acse_responder_auth_value_encoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) ber;

    CSM_LOG("[ACSE] Encoding Responder authentication value ...");

    // Generate the same challenge size than the client
    // FIXME: randomize the size for the StoC challenge?
    uint8_t size = state->handshake.ctos.size;
    state->handshake.stoc.size = size;

    // Serialize the server authentication value to the output buffer and in our scratch buffer
    for (uint8_t i = 0U; i < size; i++)
    {
#ifdef GB_TEST_VECTORS
        uint8_t byte = stoc[i];
#else
        uint8_t byte = csm_hal_get_random_u8(0, 255);
#endif
        state->handshake.stoc.value[i] = byte;
    }

    if (acse_auth_value_encoder(array, &state->handshake.stoc.value[0], size))
    {
        ret = CSM_ACSE_OK;
    }

    return ret;
}

static csm_acse_code acse_aarq_auth_value_encoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) ber;

    CSM_LOG("[ACSE] Encoding Requester authentication value ...");

    if (state->auth_level == CSM_AUTH_LOW_LEVEL)
    {
        state->handshake.ctos.size = 8U;
        csm_hal_get_lls_password(0U, &state->handshake.ctos.value[0], state->handshake.ctos.size);
    }
    else
    {
        // High level security, generate a challenge
        state->handshake.ctos.size = csm_hal_get_random_u8(8U, 64U);

        for (uint8_t i = 0U; i < state->handshake.ctos.size; i++)
        {
            uint8_t byte = csm_hal_get_random_u8(0, 255);
            state->handshake.ctos.value[i] = byte;
        }
    }

    if (acse_auth_value_encoder(array, &state->handshake.ctos.value[0], state->handshake.ctos.size))
    {
        ret = CSM_ACSE_OK;
    }
    return ret;
}

static int asce_conformance_block_encoder(csm_array *array, uint32_t value)
{
    uint8_t byte = (value >> 16U) & 0xFFU;
    int valid = csm_array_write_u8(array, byte);
    byte = (value >> 8U) & 0xFFU;
    valid = valid && csm_array_write_u8(array, byte);
    byte = value & 0xFFU;
    valid = valid && csm_array_write_u8(array, byte);

    return valid;
}


static csm_acse_code acse_user_info_encoder(csm_asso_state *state, csm_ber *ber, csm_array *array, uint8_t initiate_tag)
{
    csm_acse_code ret = CSM_ACSE_ERR;
    (void) ber;

    CSM_LOG("[ACSE] Encoding user info tag ...");

    uint32_t saved_index = array->wr_index; // save the location of the UserInfo structure size

    int valid = csm_array_write_u8(array, 0U); // size of the structure, will be updated at the end
    valid = valid && csm_array_write_u8(array, CSM_BER_TYPE_OCTET_STRING);
    valid = valid && csm_array_write_u8(array, 0U); // size of the octet-string, will be updated at the end

    // Now encode the A-XDR encoded packet
    valid = valid && csm_array_write_u8(array, initiate_tag);

    if (initiate_tag == AXDR_INITIATE_RESPONSE)
    {
        valid = valid && csm_array_write_u8(array, 0U); // null, no QoS
        valid = valid && csm_array_write_u8(array, 6U);// negotiated-dlms-version-number
    }
    else
    {
        valid = valid && csm_array_write_u8(array, 0U); // null, no Dedicated key (FIXME: add dedicated key support)
        valid = valid && csm_array_write_u8(array, 0U); // response-allowed (false)
        valid = valid && csm_array_write_u8(array, 0U); // proposed-quality-of-service (false)
        valid = valid && csm_array_write_u8(array, 6U);// proposed-dlms-version-number
    }

    // Conformance block   FIXME: to be clean, rely on a real BER encoder for the long TAG
    valid = valid && csm_array_write_u8(array, 0x5FU);
    valid = valid && csm_array_write_u8(array, 0x1FU);
    valid = valid && csm_array_write_u8(array, 4U); // Size of the conformance block data
    valid = valid && csm_array_write_u8(array, 0U); // unused bits in the bit-string

    if (initiate_tag == AXDR_INITIATE_RESPONSE)
    {
        // Serialize the conformance block (3 bytes)
        valid = valid && asce_conformance_block_encoder(array, state->config->conformance);

        // server-max-receive-pdu-size
        uint16_t server_pdu_size = csm_array_data_size(array);
        uint8_t byte = (server_pdu_size >> 8U) & 0xFFU;
        valid = valid && csm_array_write_u8(array, byte);
        byte = server_pdu_size & 0xFFU;
        valid = valid && csm_array_write_u8(array, byte);

        if ((state->ref == LN_REF) || (state->ref == LN_REF_WITH_CYPHERING))
        {
            valid = valid && csm_array_write_u8(array, 0U);
            valid = valid && csm_array_write_u8(array, 7U);
        }
        else
        {
            valid = valid && csm_array_write_u8(array, 0xFAU);
            valid = valid && csm_array_write_u8(array, 0U);
        }
    }
    else
    {
        // proposed conformance block
        valid = valid && asce_conformance_block_encoder(array, 0xFFFFFFFFU);
        // client-max-receive-pdu-size
        valid = valid && csm_array_write_u16(array, 0xFFFFU);
    }

    // Update the size of the initiate response elements
    uint32_t size = array->wr_index - saved_index;

    valid = valid && csm_array_set(array, saved_index, size - 1U); // minus the size byte field
    valid = valid && csm_array_set(array, saved_index + 2U, size - 3U); // minus whole header (BER size + Octet-String tag + length)

    if (valid)
    {
        ret = CSM_ACSE_OK;
    }

    return ret;
}

static csm_acse_code acse_initiate_response_encoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    return acse_user_info_encoder(state, ber, array, AXDR_INITIATE_RESPONSE);
}

static csm_acse_code acse_initiate_request_encoder(csm_asso_state *state, csm_ber *ber, csm_array *array)
{
    return acse_user_info_encoder(state, ber, array, AXDR_INITIATE_REQUEST);
}


// FIXME: export context field in the configuration file
static const csm_asso_enc aare_encoder_chain[] =
{
    {CSM_ASSO_APP_CONTEXT_NAME,         ACSE_ALWAYS,    acse_app_context_encoder},
    {CSM_BER_TYPE_OBJECT_IDENTIFIER,    ACSE_ALWAYS,    acse_oid_context_encoder},
    {CSM_ASSO_RESULT_FIELD,             ACSE_ALWAYS,    acse_result_encoder},
    {CSM_ASSO_SOURCE_DIAGNOSTIC,        ACSE_ALWAYS,    acse_result_src_diag_encoder},

    // Additional fields specific when ciphered authentication is required
    {CSM_ASSO_RESP_AP_TITLE,            ACSE_SEC,       acse_resp_system_title_encoder},
    {CSM_ASSO_RESPONDER_ACSE_REQ,       ACSE_SEC,       acse_requirements_encoder},
    {CSM_ASSO_RESP_MECHANISM_NAME,      ACSE_SEC,       acse_oid_mechanism_encoder},
    {CSM_ASSO_RESP_AUTH_VALUE,          ACSE_SEC,       acse_responder_auth_value_encoder},

    // Final field
    {CSM_ASSO_USER_INFORMATION,         ACSE_ALWAYS,    acse_initiate_response_encoder},

};

#define CSM_ACSE_AARE_ENCODER_CHAIN_SIZE   (sizeof(aare_encoder_chain)/sizeof(aare_encoder_chain[0]))


static const csm_asso_enc aarq_encoder_chain[] =
{
    {CSM_ASSO_APP_CONTEXT_NAME,         ACSE_ALWAYS,    acse_app_context_encoder},
    {CSM_BER_TYPE_OBJECT_IDENTIFIER,    ACSE_ALWAYS,    acse_oid_context_encoder},
    {CSM_ASSO_SENDER_ACSE_REQU,         ACSE_SEC,       acse_requirements_encoder},
    {CSM_ASSO_REQ_MECHANISM_NAME,       ACSE_SEC,       acse_oid_mechanism_encoder},
    {CSM_ASSO_CALLING_AUTH_VALUE,       ACSE_SEC,       acse_aarq_auth_value_encoder},

    // Final field
    {CSM_ASSO_USER_INFORMATION,         ACSE_ALWAYS,    acse_initiate_request_encoder},

};

#define CSM_ACSE_AARQ_ENCODER_CHAIN_SIZE   (sizeof(aarq_encoder_chain)/sizeof(aarq_encoder_chain[0]))

// --------------------------  ASSOCIATION MAIN FUNCTIONS -------------------------------------------

void csm_asso_init(csm_asso_state *state)
{
    state->state_cf = CF_INACTIVE;
    state->auth_level = CSM_AUTH_LOWEST_LEVEL;
    state->ref = NO_REF;
    state->handshake.result = CSM_ASSO_ERR_NULL;
    state->current_block = 0;
    state->state = CSM_RESPONSE_STATE_START;
    state->nb_loops = 0;
}


// Check is association is granted
int csm_asso_is_granted(csm_asso_state *state)
{
    int ret = FALSE;
    if (state->state_cf == CF_IDLE)
    {
        // Test the password if required
        if (state->auth_level == CSM_AUTH_LOWEST_LEVEL)
        {
            CSM_LOG("Granted (No security)");
            state->state_cf = CF_ASSOCIATED;
            state->handshake.result = CSM_ASSO_ERR_NULL;
            ret = TRUE;
        }
        else if (state->auth_level == CSM_AUTH_LOW_LEVEL)
        {
            // Use unused StoC buffer to store our temporary password
            csm_hal_get_lls_password(state->config->llc.dsap, &state->handshake.stoc.value[0], 8U);
            if (memcmp(&state->handshake.stoc.value[0], &state->handshake.ctos.value[0], state->handshake.ctos.size) == 0)
            {
                state->state_cf = CF_ASSOCIATED;
                state->handshake.result = CSM_ASSO_ERR_NULL;
                ret = TRUE;
            }
            else
            {
                state->handshake.result = CSM_ASSO_ERR_AUTH_FAILURE;
            }
        }
        else if (state->auth_level == CSM_AUTH_HIGH_LEVEL_GMAC)
        {
            state->state_cf = CF_ASSOCIATION_PENDING;
            state->handshake.result = CSM_ASSO_AUTH_REQUIRED;
            ret = TRUE;
        }
        else
        {
            // Failure, other cases are not managed
            CSM_ERR("[ACSE] Access refused, bad authentication level");
            state->handshake.result = CSM_ASSO_AUTH_NOT_RECOGNIZED;
        }
    }

    return ret;
}

int csm_asso_decoder(csm_asso_state *state, csm_array *array, uint8_t tag)
{
    csm_ber ber;

    // Decode first bytes
    int ret = csm_ber_decode(&ber, array);
    if (ber.length.length != csm_array_unread(array))
    {
        CSM_ERR("[ACSE] Bad size");
        ret = CSM_ACSE_ERR;
    }
    else if (ber.tag.tag != tag)
    {
        CSM_ERR("[ACSE] Bad ACSE tag");
        ret = CSM_ACSE_ERR;
    }
    else
    {
        state->handshake.failure_type = CSM_ASSO_FAIL_NO_ANY;
        const csm_asso_dec *codec = &aare_decoder_chain[0];
        uint32_t size = CSM_ACSE_AARE_DECODER_CHAIN_SIZE;

        if (tag == CSM_ASSO_AARQ)
        {
            CSM_LOG("[ACSE] AARQ");
            codec = &aarq_decoder_chain[0];
            size = CSM_ACSE_AARQ_DECODER_CHAIN_SIZE;
        }
        else
        {
            CSM_ERR("[ACSE] AARE");
        }

        int eat_bytes = TRUE;
        for (uint8_t decoder_index = 0U; (decoder_index < size) && ret; decoder_index++)
        {
            if (eat_bytes)
            {
                ret = csm_ber_decode(&ber, array);
            }

            if (ret)
            {
                if (ber.tag.tag == codec[decoder_index].tag)
                {
                    eat_bytes = TRUE;
                    if ((codec[decoder_index].extract_func != NULL))
                    {
                        csm_acse_code acse_ret = codec[decoder_index].extract_func(state, &ber, array);
                        if (acse_ret != CSM_ACSE_OK)
                        {
                            CSM_ERR("[ACSE] Decoding field error!");
                        }
                        // If it is a ACSE error (not a BER deconding)
                        // we do not threat it as a fatal error, error code and cause must be sent in AARE reply
                    }
                    else
                    {
                        CSM_ERR("[ACSE] No extract function for this tag");
                    }
                }
                else
                {
                    if (codec[decoder_index].context == ACSE_OPT)
                    {
                        CSM_LOG("Optional field not found");
                        eat_bytes = FALSE;
                    }
                    else if ((state->auth_level < CSM_AUTH_LOWEST_LEVEL) && (codec[decoder_index].context == ACSE_SEC))
                    {
                        continue;
                    }
                    else
                    {
                        (void) csm_array_reader_advance(array, ber.length.length);
                        CSM_LOG("[ACSE] Skipped tag: %d", ber.tag.tag);
                        //CSM_ERR("Decoded tag: %d (0x%X), expected tag: %d (0x%X)", ber.tag.tag, ber.tag.tag, codec[decoder_index].tag, codec[decoder_index].tag);
                    }
                }
            }
            else
            {
                CSM_ERR("BER decoding error!");
            }

        }
    }

    return ret;
}

int csm_asso_encoder(csm_asso_state *asso, uint8_t tag)
{
    csm_array *out = &asso->tx;
    int ret = FALSE;
    csm_ber ber;

    if (csm_array_write_u8(out, tag))
    {
        // Write dummy size, it will be updated later
        // Since the AARE is never bigger than 127, the length encoding can one-byte size
        if (csm_array_write_u8(out, 0U))
        {
            const csm_asso_enc *codec = &aare_encoder_chain[0];
            uint32_t size = CSM_ACSE_AARE_ENCODER_CHAIN_SIZE;

            if (tag == CSM_ASSO_AARQ)
            {
                codec = &aarq_encoder_chain[0];
                size = CSM_ACSE_AARQ_ENCODER_CHAIN_SIZE;
            }
            uint32_t i = 0U;
            for (i = 0U; i < size; i++)
            {
                // Don't encode optional data
                if ((codec[i].insert_func != NULL) && (codec[i].context != ACSE_SKIP))
                {
                    // Don't encode some fields when no security is required
                    if ((asso->auth_level <= CSM_AUTH_LOW_LEVEL) && (codec[i].context == ACSE_SEC))
                    {
                        continue;
                    }
                    else
                    {
                        // Insert codec tag identifier
                        if (csm_array_write_u8(out, codec[i].tag))
                        {
                            if (!codec[i].insert_func(asso, &ber, out))
                            {
                                // Exit on error
                                break;
                            }
                        }
                    }
                }
            }

            if (i >= size)
            {
                ret = TRUE;
                // Update the size
                csm_array_set(out, 1U, out->wr_index - 2U); // skip the BER header (tag+length = 2 bytes)
            }
            else
            {
                CSM_ERR("[ACSE] Encoding chain error");
            }
        }
    }
    return ret;
}

int csm_asso_server_execute(csm_asso_state *asso)
{
    int bytes_to_reply = 0;

    if (asso->state_cf  == CF_IDLE)
    {
        int ret = csm_asso_decoder(asso, &asso->rx, CSM_ASSO_AARQ);

        // Either we have a decoder error, or protocol field contents problems
        if ((ret == CSM_ACSE_OK) && (asso->handshake.failure_type == CSM_ASSO_FAIL_NO_ANY))
        {
            // We test the association state only if the whole decioding is OK
            if (csm_asso_is_granted(asso))
            {
                CSM_LOG("[ACSE] Access granted!");
            }
            else
            {
                // FIXME: print textual reason
                CSM_ERR("[ACSE] Connection rejected, reason: %d", asso->handshake.result);
            }
        }
        else
        {
            CSM_ERR("[ACSE] AARQ Decoding error");
        }

        // Send AARE, success or failure
        if (csm_asso_encoder(asso, CSM_ASSO_AARE))
        {
            bytes_to_reply = csm_array_written(&asso->tx);
            CSM_LOG("[ACSE] AARE length: %d", bytes_to_reply);
            //   csm_array_dump(packet);
        }
    }
    else if (asso->state_cf  == CF_ASSOCIATED)
    {
        uint8_t byte;
        // Associated, so maybe it is an RLRQ disconnection packet
        if (csm_array_get(&asso->rx, 0U, &byte))
        {
            if (byte == CSM_ASSO_RLRQ)
            {
                CSM_LOG("[ACSE] RLRQ Received, send RLRE");
                asso->state_cf = CF_IDLE;
                // FIXME: for now, send minimal fixed raw RLRE reply
                static uint8_t rlre[] = { CSM_ASSO_RLRE, 3U, 0x80U, 0x01U, 0x00U };

                csm_array_write_buff(&asso->tx, rlre, 5U);
                bytes_to_reply = 5U;
            }
            else
            {
                CSM_ERR("[ACSE] Bad tag received: %X", byte);
            }
        }
    }

    return bytes_to_reply;
}


int csm_asso_hls_pass3(csm_asso_state *asso, csm_request *request, csm_array *array)
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
            // Reserve memory & prepare packet
            array->offset = (offset + array->rd_index) - (CSM_DEF_SEC_HDR_SIZE + asso->handshake.stoc.size);
            array->rd_index = 0U;
            array->wr_index = 0U;

            // Build a new fake packet with: SC || IC || Information || Tag
            // Tag is left untouched, other data are appended just before
            csm_array_write_u8(array, sc.sh_byte);
            csm_array_write_u32(array, ic);
            csm_array_write_buff(array, &asso->handshake.stoc.value[0], asso->handshake.stoc.size);
            csm_array_writer_advance(array, 12U); // Add the tag (already in the buffer)

            csm_sec_result res = csm_sec_auth_decrypt(array, request, &asso->client_app_title[0]);

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

int csm_asso_hls_pass4(csm_asso_state *asso, csm_request *request, csm_array *array)
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
        array->offset = offset - (asso->handshake.ctos.size - CSM_DEF_SEC_HDR_SIZE - 2U); // 2U is the OctetString encoding
        // Write information data to authenticate
        csm_array_write_buff(array, &asso->handshake.ctos.value[0], asso->handshake.ctos.size);

        csm_sec_result res = csm_sec_auth_encrypt(array, request, csm_sys_get_system_title(), sc, ic);

        array->offset = offset; // restore offset
        array->wr_index = 0;

        int valid = csm_array_write_u8(array, AXDR_TAG_OCTETSTRING);
        valid = valid && csm_ber_write_len(array, 17U);
        valid = valid && csm_array_write_u8(array, sc.sh_byte);
        valid = valid && csm_array_write_u32(array, ic);
        valid = valid && csm_array_writer_advance(array, 12U);

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



