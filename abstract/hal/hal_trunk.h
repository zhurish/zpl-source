/*
 * hal_trunk.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_TRUNK_H__
#define __HAL_TRUNK_H__

typedef enum {
    HAL_TRUNK_MODE_MACDASA = 0x0,
    HAL_TRUNK_MODE_MACDA = 0x1,
    HAL_TRUNK_MODE_MACSA = 0x2,
} hal_trunk_mode_t;


typedef struct sdk_trunk_s
{
	int (*sdk_trunk_enable_cb) (void *, BOOL);
	int (*sdk_trunk_mode_cb) (void *, int);
	int (*sdk_trunk_add_cb) (void *, ifindex_t, int);
	int (*sdk_trunk_del_cb) (void *, ifindex_t, int);
	  void *sdk_driver;
}sdk_trunk_t;

int hal_trunk_enable(BOOL enable);
int hal_trunk_mode(int mode);
int hal_trunk_interface_enable(ifindex_t ifindex, int trunkid);
int hal_trunk_interface_disable(ifindex_t ifindex, int trunkid);

int hal_trunkid(int trunkid);


#endif /* __HAL_TRUNK_H__ */
