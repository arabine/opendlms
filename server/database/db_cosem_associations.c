#include "db_cosem_associations.h"
#include "csm_channel.h"
#include "csm_axdr_codec.h"

csm_db_code db_cosem_associations_func(csm_db_context_t *ctx, csm_array *in, csm_array *out, csm_request *request)
{
    csm_db_code code = CSM_ERR_OBJECT_ERROR;

    if (request->db_request.service == SVC_GET)
    {
        if (request->db_request.logical_name.id == 2)
        {
            // object_list
            code = CSM_OK;

            int valid = csm_array_write_u8(out, AXDR_TAG_ARRAY);
            valid = valid && csm_ber_write_len(out, ctx->size);

            for (uint32_t i = 0; i < ctx->size; i++)
            {
                const struct db_element *e = &ctx->db[i];

                for (uint32_t j = 0; j < e->nb_objects; j++)
                {
                    valid = valid && csm_array_write_u8(out, AXDR_TAG_STRUCTURE);
                    valid = valid && csm_ber_write_len(out, 4);

                    const db_object_descr *obj = &e->objects[j];
                    valid = valid && csm_array_write_u16(out, obj->class_id);
                    valid = valid && csm_array_write_u8(out, obj->version);
                    valid = valid && csm_array_write_buff(out, &request->db_request.logical_name.obis.A, 6U);
                    
                    valid = valid && csm_array_write_u8(out, AXDR_TAG_STRUCTURE);
                    valid = valid && csm_ber_write_len(out, 2);

                    // Attributes access rights
                    valid = valid && csm_array_write_u8(out, AXDR_TAG_ARRAY);
                    valid = valid && csm_ber_write_len(out, obj->nb_attr);
                    for (int a  = 0; a < obj->nb_attr; a++)
                    {
                        const db_attr_descr *attr = &obj->attr_list[a];
                        valid = valid && csm_array_write_u8(out, AXDR_TAG_STRUCTURE);
                        valid = valid && csm_ber_write_len(out, 3);
                        valid = valid && csm_array_write_u8(out, attr->number);
                        valid = valid && csm_array_write_u8(out, attr->access_rights);
                        valid = valid && csm_array_write_u8(out, AXDR_TAG_NULL);
                    }

                    valid = valid && csm_array_write_u8(out, AXDR_TAG_ARRAY);
                    valid = valid && csm_ber_write_len(out, obj->nb_meth);
                    for (int a  = 0; a < obj->nb_meth; a++)
                    {
                        const db_attr_descr *attr = &obj->meth_list[a];
                        valid = valid && csm_array_write_u8(out, AXDR_TAG_STRUCTURE);
                        valid = valid && csm_ber_write_len(out, 2);
                        valid = valid && csm_array_write_u8(out, attr->number);
                        valid = valid && csm_array_write_u8(out, attr->access_rights);
                    }
                }
            }
        }
    }
    else if (request->db_request.service == SVC_SET)
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
            int ret = csm_channel_hls_pass3(in, request);
            ret = ret && csm_channel_hls_pass4(out, request);

            if (ret)
            {
                code = CSM_OK;
            }
        }
    }

    return code;
}

