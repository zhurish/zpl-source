/*
 * hal_mirror.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_MIRROR_H__
#define __HAL_MIRROR_H__

#include "nsm_mac.h"

typedef struct sdk_mirror_s
{
	int (*sdk_mirror_enable_cb) (ifindex_t, BOOL);
	int (*sdk_mirror_source_enable_cb) (BOOL enable, ifindex_t ifindex, int mode);
	int (*sdk_mirror_source_filter_enable_cb) (BOOL enable, BOOL dst, mac_t *mac, int mode);
}sdk_mirror_t;


int hal_mirror_enable(ifindex_t ifindex, BOOL enable);
int hal_mirror_source_enable(ifindex_t ifindex, int mode, BOOL enable);
int hal_mirror_source_filter_enable(BOOL enable, BOOL dst, mac_t *mac, int mode);

#endif /* __HAL_MIRROR_H__ */
