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

#include "zplos_include.h"
#include "hal_client.h"



struct bsp_driver;

typedef int (*bsp_sdk_func)(struct bsp_driver *, zpl_void *);

typedef struct bsp_driver
{
    zpl_void *master;
    zpl_uint32 taskid;
    zpl_void *hal_client;
    bsp_sdk_func bsp_sdk_init;
    bsp_sdk_func bsp_sdk_start;
    bsp_sdk_func bsp_sdk_stop;
    bsp_sdk_func bsp_sdk_exit;

    int (*bsp_sdk_unicast)(struct bsp_driver *, zpl_phyport_t, int, zpl_void *, int);
    int (*bsp_sdk_vlan_flood)(struct bsp_driver *, vlan_t, int, zpl_void *, int);

    zpl_void *sdk_driver;
    
} bsp_driver_t;



#ifndef ZPL_SDK_MODULE
typedef struct sdk_driver {

	zpl_phyport_t 	cpu_port;
	zpl_uint32 		num_vlans;
	zpl_phyport_t 	num_ports;
    zpl_phyport_t	ports_table[8];
	void 			*sdk_device;
}sdk_driver_t;

extern sdk_driver_t *__msdkdriver;
extern int sdk_driver_init(struct bsp_driver *, zpl_void *);
extern int sdk_driver_start(struct bsp_driver *, zpl_void *);
extern int sdk_driver_stop(struct bsp_driver *, zpl_void *);
extern int sdk_driver_exit(struct bsp_driver *, zpl_void *);
#elif defined(ZPL_SDK_MODULE) && defined(ZPL_SDK_KERNEL)

#define HAL_CFG_REQUEST_CMD (30) 
#define HAL_DATA_REQUEST_CMD (29)
#define HAL_KLOG_REQUEST_CMD (28)

#define HAL_CFG_NETLINK_PROTO (30)
#define HAL_DATA_NETLINK_PROTO (29)
#define HAL_KLOG_NETLINK_PROTO (28)

typedef struct sdk_driver {

	zpl_phyport_t 	cpu_port;
	zpl_uint32 		num_vlans;
	zpl_phyport_t 	num_ports;
    zpl_phyport_t	ports_table[8];

    int debug;
    zpl_socket_t    cfg_sock;
    int             cfg_seq;
    zpl_socket_t    data_sock;
    int             data_seq;

    zpl_socket_t    klog_sock;
}sdk_driver_t;

extern sdk_driver_t *__msdkdriver;
extern int sdk_driver_init(struct bsp_driver *, zpl_void *);
extern int sdk_driver_start(struct bsp_driver *, zpl_void *);
extern int sdk_driver_stop(struct bsp_driver *, zpl_void *);
extern int sdk_driver_exit(struct bsp_driver *, zpl_void *);

#endif

extern bsp_driver_t bsp_driver;

extern int bsp_driver_msg_handle(struct hal_client *client, zpl_uint32 cmd, void *driver);



extern int bsp_driver_init(bsp_driver_t *);
extern int bsp_driver_task_init(bsp_driver_t *);
extern int bsp_driver_exit(bsp_driver_t *);
extern int bsp_driver_task_exit(bsp_driver_t *);

extern int bsp_module_func(bsp_driver_t *, bsp_sdk_func init_func, bsp_sdk_func start_func, bsp_sdk_func stop_func, bsp_sdk_func exit_func);

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
