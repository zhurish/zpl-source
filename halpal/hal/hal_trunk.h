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



typedef struct hal_trunk_param_s
{
	zpl_bool enable;
	zpl_uint32 trunkid;
	zpl_uint32 mode;
}hal_trunk_param_t;

int hal_trunk_enable(zpl_bool enable);
int hal_trunk_mode(zpl_uint32 trunkid, zpl_uint32 mode);
int hal_trunk_create(zpl_uint32 trunkid, zpl_bool enable);
int hal_trunk_add_interface(zpl_uint32 trunkid, ifindex_t ifindex);
int hal_trunk_del_interface(zpl_uint32 trunkid, ifindex_t ifindex);
int hal_trunk_mcast_join(zpl_uint32 trunkid, vlan_t vid, mac_t *mac);
int hal_trunk_mcast_level(zpl_uint32 trunkid, vlan_t vid, mac_t *mac);

int hal_trunkid(zpl_uint32 trunkid);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_TRUNK_H__ */
