/*
 * hal_trunk.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_TRUNK_H__
#define __HAL_TRUNK_H__


typedef struct sdk_trunk_s
{
	int (*sdk_trunk_enable_cb) (BOOL);
	int (*sdk_trunk_mode_cb) (int);
	int (*sdk_trunk_add_cb) (ifindex_t, int);
	int (*sdk_trunk_del_cb) (ifindex_t, int);

}sdk_trunk_t;

int hal_trunk_enable(BOOL enable);
int hal_trunk_mode(int mode);
int hal_trunk_interface_enable(ifindex_t ifindex, int trunkid);
int hal_trunk_interface_disable(ifindex_t ifindex, int trunkid);

int hal_trunkid(int trunkid);


#endif /* __HAL_TRUNK_H__ */
