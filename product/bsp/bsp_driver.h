/*
 * bsp_driver.h
 *
 *  Created on: 2019年9月8日
 *      Author: zhurish
 */

#ifndef __BSP_DRIVER_H__
#define __BSP_DRIVER_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_include.h"
#include "hal_client.h"

typedef int (*bsp_sdk_func)(struct bsp_driver *, zpl_void *);



typedef struct bsp_driver
{
    zpl_void *master;
    zpl_uint32 taskid;
    zpl_void *hal_client;
    int (*bsp_sdk_init)(struct bsp_driver *, zpl_void *);
    int (*bsp_sdk_start)(struct bsp_driver *, zpl_void *);
    int (*bsp_sdk_stop)(struct bsp_driver *, zpl_void *);
    int (*bsp_sdk_exit)(struct bsp_driver *, zpl_void *);

    int (*bsp_sdk_unicast)(struct bsp_driver *, zpl_phyport_t, int, zpl_void *, int);
    int (*bsp_sdk_vlan_flood)(struct bsp_driver *, vlan_t, int, zpl_void *, int);

    zpl_void *sdk_driver;
} bsp_driver_t;

#ifndef ZPL_SDK_MODULE
typedef struct sdk_driver {

	zpl_phyport_t 	cpu_port;
	zpl_uint32 		num_vlans;
	zpl_phyport_t 	num_ports;

	void 			*sdk_device;
}sdk_driver_t;

extern sdk_driver_t *__msdkdriver;
extern int sdk_driver_init(struct bsp_driver *, sdk_driver_t *);
extern int sdk_driver_start(struct bsp_driver *, sdk_driver_t *);
extern int sdk_driver_stop(struct bsp_driver *, sdk_driver_t *);
extern int sdk_driver_exit(struct bsp_driver *, sdk_driver_t *);

#endif

extern bsp_driver_t bsp_driver;

extern int bsp_driver_msg_handle(struct hal_client *client, zpl_uint32 cmd, void *driver);



extern int bsp_driver_init(bsp_driver_t *);
extern int bsp_driver_task_init(bsp_driver_t *);
extern int bsp_driver_exit(bsp_driver_t *);
extern int bsp_driver_task_exit(bsp_driver_t *);

extern int bsp_module_func(bsp_driver_t *, bsp_sdk_func init_func, bsp_sdk_func start_func, bsp_sdk_func stop_func, bsp_sdk_func exit_func);
extern int bsp_driver_module_check(hal_ipccmd_callback_t *cmdtbl, int num, int module);

extern int bsp_module_init(void);
extern int bsp_module_task_init(void);
extern int bsp_module_exit(void);
extern int bsp_module_task_exit(void);
extern int bsp_module_start(void);


extern int bsp_test_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_DRIVER_H__ */
