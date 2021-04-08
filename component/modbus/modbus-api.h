/*
 * Copyright © 2010-2012 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_API_H
#define MODBUS_API_H

#ifndef _MSC_VER
# include <stdint.h>
# include <sys/time.h>
#else
# include "stdint.h"
# include <time.h>
typedef int ssize_t;
#endif
#include <sys/types.h>
#include <modbus-config.h>

#include "modbus.h"

#include "zebra.h"

MODBUS_BEGIN_DECLS

typedef enum {
    MODBUS_TYPE_NONE,
    MODBUS_TYPE_TCP,
    MODBUS_TYPE_TCP_PI,
    MODBUS_TYPE_RTU
}modbus_type_t;

typedef enum {
    MODBUS_MODE_NONE,
    MODBUS_MODE_MASTER,
    MODBUS_MODE_SLAVE,
    MODBUS_MODE_SERVER,
    MODBUS_MODE_CLIENT
}modbus_mode_t;

typedef struct _modbus_config_t {
    ospl_uint32 id;
#ifdef OS_WIN32
    HANDLE taskhandle;
    DWORD  taskid;
#else
    ospl_uint32 taskid;
#endif
    modbus_mode_t mode;
    modbus_type_t type;
    modbus_t    *ctx;
    ospl_uint32 slave_id;
    ospl_uint16     port;
    ospl_char       *address;
    ospl_uint32     baudrate;
    modbus_mapping_t *tab_registers;		//指向保持寄存器值
    ospl_uint8 *query;
    ospl_bool   running;
    ospl_bool   waiting;
} modbus_config_t;


MODBUS_API int modbus_mode_set_api(modbus_mode_t);
MODBUS_API modbus_mode_t modbus_mode_get_api();

MODBUS_API int modbus_type_set_api(modbus_type_t);
MODBUS_API modbus_type_t modbus_type_get_api();

MODBUS_API int modbus_id_set_api(ospl_uint32);
MODBUS_API ospl_uint32 modbus_id_get_api();

MODBUS_API int modbus_port_set_api(ospl_uint16);
MODBUS_API ospl_uint16 modbus_port_get_api();

MODBUS_API int modbus_device_set_api(ospl_char*);
MODBUS_API ospl_char* modbus_device_get_api();

MODBUS_API int modbus_baudrate_set_api(ospl_uint32);
MODBUS_API ospl_uint32 modbus_baudrate_get_api();

MODBUS_API int modbus_hold_registers_set_api(ospl_double, ospl_double, ospl_double);
MODBUS_API int modbus_input_registers_set_api(ospl_double, ospl_double, ospl_double);

MODBUS_API int modbus_start_api();
MODBUS_API int modbus_stop_api();


MODBUS_API int modbus_module_init();
MODBUS_API int modbus_module_exit();

MODBUS_API int modbus_module_task_init();
MODBUS_API int modbus_module_task_exit();

MODBUS_END_DECLS

#endif  /* MODBUS_PRIVATE_H */
