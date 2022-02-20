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
    //zpl_uint32 product;
    //zpl_uint32 id;
    //char *name;
    
    zpl_void *master;
    zpl_uint32 taskid;
    zpl_void *hal_client;
    int (*bsp_sdk_init)(struct bsp_driver *, zpl_void *);
    int (*bsp_sdk_start)(struct bsp_driver *, zpl_void *);
    int (*bsp_sdk_stop)(struct bsp_driver *, zpl_void *);
    int (*bsp_sdk_exit)(struct bsp_driver *, zpl_void *);
    zpl_void *sdk_driver;
} bsp_driver_t;
  
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
