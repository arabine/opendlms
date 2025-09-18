#pragma once

#include "app_database.h"
#include "db_cosem_clock.h"
#include "db_cosem_associations.h"
#include "db_cosem_image_transfer.h"

const db_attr_descr clock_attributes[] = {
{ DB_ACCESS_GETSET, 2 },
{ DB_ACCESS_GETSET, 3 },
{ DB_ACCESS_GET,    4 },
{ DB_ACCESS_GETSET, 5 },
{ DB_ACCESS_GETSET, 6 },
{ DB_ACCESS_GETSET, 7 },
{ DB_ACCESS_GETSET, 8 },
{ DB_ACCESS_GETSET, 9 },
};

const db_attr_descr current_association_attributes[] = {
{ DB_ACCESS_GET, 2 },
{ DB_ACCESS_GET, 3 },
{ DB_ACCESS_GET, 4 },
{ DB_ACCESS_GET, 5 },
{ DB_ACCESS_GET, 6 },
{ DB_ACCESS_SET, 7 },
{ DB_ACCESS_GETSET, 8 },
{ DB_ACCESS_GETSET, 9 },
{ DB_ACCESS_GET, 10 },
{ DB_ACCESS_GET, 11 },
};

const db_attr_descr current_association_methods[] = {
{ DB_ACCESS_EXECUTE, 1 },
};

const db_attr_descr logical_device_name[] = {
{ DB_ACCESS_GET, 2 }
};

const db_object_descr clock_objects[] = {
    {&clock_attributes[0], NULL, 8U , { 0U, 0U, 1U, 0U, 0U, 255U } , 0U , 8U, 0U },
};

const db_object_descr associations_objects[] = {
    {&current_association_attributes[0], &current_association_methods[0], 15U , { 0U, 0U, 40U, 0U, 0U, 255U } , 2U , 10U, 1U },
    {&logical_device_name[0], NULL, 1U , { 0U, 0U, 42U, 0U, 0U, 255U } , 0U , 1U, 0U },
};


const struct db_element gDataBaseList[] = {
    { &clock_objects[0], db_cosem_clock_func, sizeof(clock_objects) / sizeof(clock_objects[0]) },
    { &associations_objects[0], db_cosem_associations_func, sizeof(associations_objects) / sizeof(associations_objects[0]) },
};


#define COSEM_DATABASE_SIZE (sizeof(gDataBaseList)/sizeof(gDataBaseList[0]))

