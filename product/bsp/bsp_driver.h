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
#include "nsm_include.h"




typedef struct bsp_driver {
	zpl_uint32		product;
	zpl_uint32 	id;
	char 	*name;

	void			*driver;

}bsp_driver_t;

extern bsp_driver_t *bsp_driver;

int bsp_client_msg_handle(struct hal_client *client, zpl_uint32 cmd, void *driver);
int bsp_module_init();

int bsp_test_init();

#ifdef __cplusplus
}
#endif

#endif /* __BSP_DRIVER_H__ */
