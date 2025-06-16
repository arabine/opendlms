

#include "app_calendar.h"
#include "app_database.h"
#include "csm_server.h"

void csm_print_obis(const csm_object_t *data)
{
    CSM_TRACE("Class: %d, obis: %d.%d.%d.%d.%d.%d, id: %d\r\n", data->class_id, data->obis.A, data->obis.B, data->obis.C, data->obis.D, data->obis.E, data->obis.F, data->id);
}

uint8_t csm_is_obis_equal(const csm_obis_code *first, const csm_obis_code *second)
{
    uint8_t ret = FALSE;
    if ((first->A == second->A) &&
        (first->B == second->B) &&
        (first->C == second->C) &&
        (first->D == second->D) &&
        (first->E == second->E) &&
        (first->F == second->F))
    {
        ret = TRUE;
    }

    return ret;
}


static int csm_db_check_attribute(csm_db_request *db_request, const db_object_descr *object)
{
    int ret = FALSE;
    const db_attr_descr *attr = NULL;
    uint8_t access_rights = 0U;
    uint8_t size = 0U;

    if (db_request->service == SVC_GET)
    {
        access_rights |= DB_ACCESS_GET;
        attr = object->attr_list;
        size = object->nb_attr;
    }
    if (db_request->service == SVC_SET)
    {
        access_rights |= DB_ACCESS_SET;
        attr = object->attr_list;
        size = object->nb_attr;
    }
    if (db_request->service == SVC_ACTION)
    {
        access_rights |= DB_ACCESS_GETSET;
        attr = object->meth_list;
        size = object->nb_meth;
    }

    if (attr != NULL)
    {
        // go direct to the attribute offset, if id is correct
        int8_t offset = db_request->logical_name.id - 2; // Start at zero, and minus the logical name

        if (offset < size)
        {
            // Now check the access right
            if ((attr[offset].access_rights & access_rights) == access_rights)
            {
                ret = TRUE;
            }
            else
            {
                CSM_ERR("[DB] Bad access rights");
            }
        }
        else
        {
            CSM_ERR("[DB] Bad id");
        }
    }

    return ret;
}


static int csm_db_get_object(csm_server_context_t *ctx, db_obj_handle *handle)
{
    uint8_t found = FALSE;

    for (uint8_t i = 0U; (i < ctx->db->size) && (!found); i++)
    {
        const struct db_element *obj_list = &ctx->db->el[i];

        // Loop on all cosem object in list
        for (uint32_t object_index = 0U; (object_index < obj_list->nb_objects) && (!found); object_index++)
        {
            const db_object_descr *curr_obj = &obj_list->objects[object_index];
            // Check the obis code
            if ((curr_obj->class_id == ctx->request.db_request.logical_name.class_id) &&
                     csm_is_obis_equal(&curr_obj->obis_code, &(ctx->request.db_request.logical_name.obis)))
            {
                handle->object       = curr_obj;
                handle->db_index     = i;
                handle->obj_index    = object_index;

                // Verify that this object contains the suitable method/attribute and if we can access to it
                found = csm_db_check_attribute(&ctx->request.db_request, curr_obj);
            }
        }
    }

    return found;
}


csm_db_code csm_db_access_func(csm_server_context_t *ctx, csm_array *in, csm_array *out)
{
    csm_db_code code = CSM_ERR_OBJECT_ERROR;
    // We want to access to an object. First, gets an handle to its parameters
    // If the handle retrieval fails, then this cosem object probably not exists

    db_obj_handle handle;

    if (csm_db_get_object(ctx, &handle))
    {
        // Ok, call the database main function
        const struct db_element *db_element = &ctx->db->el[handle.db_index];
        if (db_element->handler != NULL)
        {
            code = db_element->handler(ctx, in, out);
        }
        else
        {
            CSM_ERR("[DB] Cannot access to DB handler function");
        }
    }
    else
    {
        CSM_ERR("[DB] Cosem object not found: ");
        csm_print_obis(&ctx->request.db_request.logical_name);
        code = CSM_ERR_OBJECT_NOT_FOUND;
    }

    return code;
}

