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

#ifndef CSM_SERVICES_H
#define CSM_SERVICES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "csm_definitions.h"
#include "csm_association.h"

enum csm_req_type { RQ_NORMAL, RQ_NEXT, RQ_WITH_LIST };

/**
 * @brief Generic Codec error codes
 **/
typedef enum
{
    // These two error codes can be used in priority
    CSM_OK,                 //!< Request OK
    CSM_OK_BLOCK,           //!< Request OK, ask for a block transfer (not enough space to store the data)
    CSM_ERR_OBJECT_ERROR,   //!< Generic error coming from the object

    // Some more specific errors
    CSM_ERR_OBJECT_NOT_FOUND,    //!< Not found in the database, check obis code!
    CSM_ERR_BAD_ENCODING,        //!< Bad encoding of codec
    CSM_ERR_UNAUTHORIZED_ACCESS, //!< Attribute access problem
    CSM_ERR_TEMPORARY_FAILURE,   ///< Temporary failure
    CSM_ERR_DATA_CONTENT_NOT_OK, ///< Data content is not accepted.
} csm_db_code;


#define DB_ACCESS_GET       (uint16_t)1U
#define DB_ACCESS_SET       (uint16_t)2U
#define DB_ACCESS_GETSET    (uint16_t)3U

typedef enum
{
    DB_TYPE_NOT_USED = 0U,
    DB_TYPE_UNSIGNED8,
    DB_TYPE_UNSIGNED16,
    DB_TYPE_UNSIGNED32,
    DB_TYPE_UNSIGNED64,
    DB_TYPE_SIGNED8,
    DB_TYPE_SIGNED16,
    DB_TYPE_SIGNED32,
    DB_TYPE_SIGNED64,
    DB_TYPE_OCTET_STRING,
    DB_TYPE_DATE_TIME,
    DB_TYPE_DATE,
    DB_TYPE_TIME,
    DB_TYPE_ENUM,
    DB_TYPE_STRUCTURE,
    DB_TYPE_BIT_STRING,
    DB_TYPE_BOOLEAN,
    DB_TYPE_ARRAY,
    DB_TYPE_PROFILE_BUFFER,
    DB_TYPE_VISIBLE_STRING,
    DB_TYPE_ATTR_0,
    DB_TYPE_ATTR_1
} db_data_type;


typedef struct
{
     uint16_t  access_rights;   //!< Give the access right coding number
     int8_t    number;          //!<
     uint8_t   type;            //!< Gives the type of the attribute, for deserialization purpose
} db_attr_descr;

/**
 * @class CosemObject base object
 * @brief
**/
typedef struct
{
    const db_attr_descr *attr_list;  ///< The table of attributes
    const db_attr_descr *meth_list;  ///< The table of methods
    uint16_t  class_id;       ///< Class Id
    csm_obis_code   obis_code;          ///< Obis code for Logical Name
    uint8_t   version;       ///< Version
    uint8_t   nb_attr;  ///< Number of attributes
    uint8_t   nb_meth;  ///< Number of methods

} db_object_descr;

/**
 * @brief Handle pointing to a ROM Cosem object
 */
typedef struct
{
    const db_object_descr *object; ///< Pointer to the cosem object
    uint8_t db_index; // database number (index)
    uint8_t obj_index; // object index in the database
} db_obj_handle;

struct db_element;
typedef struct 
{
    const struct db_element *db;
    uint32_t size;

} csm_db_context_t;

typedef csm_db_code (*csm_db_access_handler)(csm_db_context_t *ctx, csm_array *in, csm_array *out, csm_request *request);

struct db_element
{
    const db_object_descr *objects;
    csm_db_access_handler handler;
    uint8_t nb_objects;
};

// ----------------------------------- CLIENT SERVICES -----------------------------------

void csm_client_init(csm_request *request, csm_response *response);
int csm_client_has_more_data(csm_response *response);
int csm_client_decode(csm_response *response, csm_array *array);
int svc_request_encoder(csm_request *request, csm_array *array);
int csm_client_encode_selective_access_by_range(csm_array *array, csm_object_t *restricting_object, csm_array *start, csm_array *end);

// ----------------------------------- SERVER SERVICES -----------------------------------

void csm_services_init(const csm_db_access_handler db_access);

// Return he number of bytes to transfer back, 0 if no response
int csm_server_services_execute(csm_db_context_t *ctx, csm_asso_state *state, csm_request *request, csm_array *array);

// Specific method in case of HLS authentication
int csm_services_hls_execute(csm_db_context_t *ctx, csm_asso_state *state, csm_request *request, csm_array *array);

#ifdef __cplusplus
}
#endif

#endif // CSM_SERVICES_H
