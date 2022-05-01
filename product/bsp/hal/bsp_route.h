/*
 * bsp_route.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_ROUTE_H__
#define __BSP_ROUTE_H__
#ifdef __cplusplus
extern "C" {
#endif


typedef struct sdk_route_s
{
	int (*sdk_route_add_cb) (void *, hal_route_param_t *param);
	int (*sdk_route_del_cb) (void *, hal_route_param_t *param);
}sdk_route_t;

extern sdk_route_t sdk_route;
extern int bsp_route_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_ROUTE_H__ */
