/*
 * hal_mirror.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_MIRROR_H__
#define __HAL_MIRROR_H__

#include "nsm_mac.h"

typedef enum hal_mirror_mode_e {
    HAL_MIRROR_SOURCE_PORT 	= 1,
	HAL_MIRROR_SOURCE_MAC 	= 2,
	HAL_MIRROR_SOURCE_DIV	= 3,
} hal_mirror_mode_t;

typedef enum hal_mirror_type_e {
    HAL_MIRROR_INGRESS 	= 1,
	HAL_MIRROR_EGRESS 	= 2,
	HAL_MIRROR_BOTH	= 3,
} hal_mirror_type_t;

typedef enum hal_mirror_filter_e {
    HAL_MIRROR_FILTER_ALL 	= 0,
    HAL_MIRROR_FILTER_DA 	= 1,
	HAL_MIRROR_FILTER_SA 	= 2,
	HAL_MIRROR_FILTER_BOTH	= 3,
} hal_mirror_filter_t;


typedef struct sdk_mirror_s
{
	int (*sdk_mirror_enable_cb) (void *, ifindex_t, BOOL);
	int (*sdk_mirror_source_enable_cb) (void *, BOOL enable, ifindex_t ifindex,
			hal_mirror_mode_t mode, hal_mirror_type_t type);
	int (*sdk_mirror_source_filter_enable_cb) (void *, BOOL enable,
			hal_mirror_filter_t filter, hal_mirror_type_t type, mac_t *mac, mac_t *mac1);
	void *sdk_driver;
}sdk_mirror_t;


int hal_mirror_enable(ifindex_t ifindex, BOOL enable);
int hal_mirror_source_enable(ifindex_t ifindex, BOOL enable, hal_mirror_mode_t mode, hal_mirror_type_t type);
int hal_mirror_source_filter_enable(BOOL enable, hal_mirror_filter_t filter, hal_mirror_type_t type, mac_t *mac, mac_t *mac1);

#endif /* __HAL_MIRROR_H__ */
