/*
 * hal_mirror.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_MIRROR_H__
#define __HAL_MIRROR_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "nsm_mac.h"
#include "nsm_mirror.h"

enum hal_mirror_cmd 
{
    HAL_MIRROR_CMD_NONE,
	HAL_MIRROR_CMD_DST_PORT,
	HAL_MIRROR_CMD_SRC_PORT,
	HAL_MIRROR_CMD_SRC_MAC,
    HAL_MIRROR_CMD_MAX,
};

typedef struct hal_mirror_param_s
{
	zpl_uint32 value;
	zpl_uint8 dir;
	zpl_uint8 filter;
	mac_t mac[NSM_MAC_MAX];
}hal_mirror_param_t;


int hal_mirror_enable(ifindex_t ifindex, zpl_bool enable);
int hal_mirror_source_enable(ifindex_t ifindex, zpl_bool enable, mirror_dir_en type);

int hal_mirror_source_filter_enable(ifindex_t ifindex, zpl_bool enable,
	mirror_dir_en dir, mirror_filter_t filter, zpl_uchar *mac);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_MIRROR_H__ */
