#include "db_cosem_associations.h"
#include "csm_axdr_codec.h"
#include "db_definitions.h"

typedef struct 
{
    uint32_t element_idx;
    uint32_t nb_elements;
    uint32_t object_idx;

} asso_db_context_t;

static  asso_db_context_t g_asso_db_contexes[DB_NUMBER_OF_ASSOCIATIONS] = {0};

csm_db_code db_cosem_associations_func(csm_server_context_t *ctx, csm_array *in, csm_array *out)
{
    csm_db_code code = CSM_ERR_OBJECT_ERROR;

    if ((ctx->asso.channel_id < 0) || (ctx->asso.channel_id >= DB_NUMBER_OF_ASSOCIATIONS))
    {
        CSM_ERR("[DB] Channel ID invalid");
        return CSM_ERR_TEMPORARY_FAILURE;
    }
    
    asso_db_context_t *asso_ctx = &g_asso_db_contexes[ctx->asso.channel_id];

    if (ctx->request.db_request.logical_name.obis.C == 42)
    {
        // Logical device name
        code = CSM_OK;
        code = csm_axdr_wr_octetstring(out, "D8S0000000000001", 2U, AXDR_TAG_OCTETSTRING);
    }
    else
    {
        if (ctx->request.db_request.service == SVC_GET)
        {
            // object_list
            if (ctx->request.db_request.logical_name.id == 2)
            {
                // Always reply by block
                code = CSM_OK_BLOCK;

                if (ctx->asso.state == CSM_RESPONSE_STATE_SENDING)
                {
                    return code;
                }

                int valid = TRUE;

                // -------- ONE LOOP == ONE OBJECT --------
                if (ctx->asso.state == CSM_RESPONSE_STATE_START)
                {
                    // Initialize counters
                    ctx->asso.current_loop = 0;
                    asso_ctx->element_idx = 0;
                    asso_ctx->object_idx = 0;
                    asso_ctx->nb_elements = ctx->db->size;

                    // Compute the numer of objects
                    ctx->asso.nb_loops = 0;
                    for (uint32_t i = 0; i < ctx->db->size; i++)
                    {
                        const struct db_element *e = &ctx->db->el[i];
                        ctx->asso.nb_loops += e->nb_objects;
                    }
                    valid = valid && csm_array_write_u8(out, AXDR_TAG_ARRAY);
                    valid = valid && csm_ber_write_len(out, ctx->asso.nb_loops);

                    // Continue with the first object
                }
                
                // Get one object
                const struct db_element *e = &ctx->db->el[asso_ctx->element_idx];
                const db_object_descr *obj = &e->objects[asso_ctx->object_idx];

                valid = valid && csm_array_write_u8(out, AXDR_TAG_STRUCTURE);
                valid = valid && csm_ber_write_len(out, 4);
                
                valid = valid && csm_axdr_wr_u16(out, obj->class_id);
                valid = valid && csm_axdr_wr_u8(out, obj->version);
                valid = valid && csm_axdr_wr_octetstring(out, &obj->obis_code.A, 6U, AXDR_TAG_OCTETSTRING);
                
                valid = valid && csm_array_write_u8(out, AXDR_TAG_STRUCTURE);
                valid = valid && csm_ber_write_len(out, 2);

                // Attributes access rights
                valid = valid && csm_array_write_u8(out, AXDR_TAG_ARRAY);
                valid = valid && csm_ber_write_len(out, obj->nb_attr + 1);

                // Auto encode logical name (always attribute 1)
                valid = valid && csm_array_write_u8(out, AXDR_TAG_STRUCTURE);
                valid = valid && csm_ber_write_len(out, 3);
                valid = valid && csm_axdr_wr_i8(out, 1);
                valid = valid && csm_axdr_wr_enum(out, DB_ACCESS_GET);
                valid = valid && csm_array_write_u8(out, AXDR_TAG_NULL);

                // Encode the other attributes (id > 1)
                for (int a  = 0; a < obj->nb_attr; a++)
                {
                    const db_attr_descr *attr = &obj->attr_list[a];
                    valid = valid && csm_array_write_u8(out, AXDR_TAG_STRUCTURE);
                    valid = valid && csm_ber_write_len(out, 3);
                    valid = valid && csm_axdr_wr_i8(out, attr->number);
                    valid = valid && csm_axdr_wr_enum(out, attr->access_rights);
                    valid = valid && csm_array_write_u8(out, AXDR_TAG_NULL);
                }

                // Encode the methods
                valid = valid && csm_array_write_u8(out, AXDR_TAG_ARRAY);
                valid = valid && csm_ber_write_len(out, obj->nb_meth);
                for (int a  = 0; a < obj->nb_meth; a++)
                {
                    const db_attr_descr *attr = &obj->meth_list[a];
                    valid = valid && csm_array_write_u8(out, AXDR_TAG_STRUCTURE);
                    valid = valid && csm_ber_write_len(out, 2);
                    valid = valid && csm_axdr_wr_i8(out, attr->number);
                    valid = valid && csm_axdr_wr_enum(out, attr->access_rights);
                }

                asso_ctx->object_idx++;
                if (asso_ctx->object_idx >= e->nb_objects)
                {
                    // On passe à l'élément suivant
                    asso_ctx->element_idx++;
                    asso_ctx->object_idx = 0;                
                }

                ctx->asso.current_loop++;

            }
        }
        else if (ctx->request.db_request.service == SVC_SET)
        {
            // Not implemented
        }
        else
        {
            // Action
            uint32_t size = 0;
            if (csm_axdr_rd_octetstring(in, &size))
            {
                CSM_LOG("[DB] Reply to HLS authentication");
                int ret = csm_asso_hls_pass3(&ctx->asso, &ctx->request, in);
                ret = ret && csm_asso_hls_pass4(&ctx->asso, &ctx->request, out);

                if (ret)
                {
                    code = CSM_OK;
                }
            }
        }
    }

    return code;
}

