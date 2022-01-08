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

enum hal_trunk_cmd 
{
    HAL_TRUNK_CMD_NONE,
	HAL_TRUNK_CMD_ENABLE,
	HAL_TRUNK_CMD_ADDIF,
	HAL_TRUNK_CMD_DELIF,
	HAL_TRUNK_CMD_MODE,
    HAL_TRUNK_CMD_MAX,
};

typedef struct hal_trunk_param_s
{
	zpl_uint32 trunkid;
	zpl_uint32 mode;
	zpl_bool enable;
}hal_trunk_param_t;

int hal_trunk_enable(zpl_bool enable);
int hal_trunk_mode(zpl_uint32 trunkid, zpl_uint32 mode);
int hal_trunk_interface_enable(ifindex_t ifindex, zpl_uint32 trunkid);
int hal_trunk_interface_disable(ifindex_t ifindex, zpl_uint32 trunkid);

int hal_trunkid(zpl_uint32 trunkid);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_TRUNK_H__ */
