/*
 * bsp_global.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_GLOBAL_H__
#define __BSP_GLOBAL_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef struct sdk_global_s
{
	int (*sdk_jumbo_size_cb) (void * , zpl_uint32 );
	int (*sdk_switch_manege_cb) (void *, hal_global_header_t*, zpl_bool);
	int (*sdk_switch_forward_cb) (void *, hal_global_header_t*, zpl_bool);
	int (*sdk_multicast_flood_cb) (void *, hal_global_header_t*, zpl_bool);
	int (*sdk_unicast_flood_cb) (void *, hal_global_header_t*, zpl_bool);
	int (*sdk_multicast_learning_cb) (void *, hal_global_header_t*, zpl_bool);
	int (*sdk_bpdu_enable_cb) (void *, hal_global_header_t*, zpl_bool);//全局使能接收BPDU报文
	int (*sdk_aging_time_cb) (void *, hal_global_header_t*, zpl_uint32);
}sdk_global_t;

extern sdk_global_t sdk_global;
extern int bsp_global_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_MIRROR_H__ */
