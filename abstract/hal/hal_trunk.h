/*
 * hal_trunk.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_TRUNK_H__
#define __HAL_TRUNK_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HAL_TRUNK_MODE_MACDASA = 0x0,
    HAL_TRUNK_MODE_MACDA = 0x1,
    HAL_TRUNK_MODE_MACSA = 0x2,
} hal_trunk_mode_t;


typedef struct sdk_trunk_s
{
	int (*sdk_trunk_enable_cb) (void *, ospl_bool);
	int (*sdk_trunk_mode_cb) (void *, ospl_uint32);
	int (*sdk_trunk_add_cb) (void *, ifindex_t, ospl_uint32);
	int (*sdk_trunk_del_cb) (void *, ifindex_t, ospl_uint32);
	  void *sdk_driver;
}sdk_trunk_t;

int hal_trunk_enable(ospl_bool enable);
int hal_trunk_mode(ospl_uint32 mode);
int hal_trunk_interface_enable(ifindex_t ifindex, ospl_uint32 trunkid);
int hal_trunk_interface_disable(ifindex_t ifindex, ospl_uint32 trunkid);

int hal_trunkid(ospl_uint32 trunkid);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_TRUNK_H__ */
