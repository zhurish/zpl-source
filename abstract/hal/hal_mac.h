/*
 * hal_mac.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_MAC_H__
#define __HAL_MAC_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "nsm_vlan.h"
#include "nsm_mac.h"


enum hal_mac_cmd 
{
    HAL_MAC_CMD_NONE,
	HAL_MAC_CMD_AGE,
	HAL_MAC_CMD_ADD,
	HAL_MAC_CMD_DEL,
	HAL_MAC_CMD_CLEAR,
	HAL_MAC_CMD_READ,
    HAL_MAC_CMD_MAX,
};

typedef struct hal_mac_param_s
{
	vlan_t vlan;
	zpl_uint32 value;
	mac_t mac[NSM_MAC_MAX];
}hal_mac_param_t;



int hal_mac_age(zpl_uint32 age);
int hal_mac_add(ifindex_t ifindex, vlan_t vlan, mac_t *mac, zpl_uint32 pri);
int hal_mac_del(ifindex_t ifindex, vlan_t vlan, mac_t *mac, zpl_uint32 pri);
int hal_mac_clr(ifindex_t ifindex, vlan_t vlan);
int hal_mac_read(ifindex_t ifindex, vlan_t vlan);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_MAC_H__ */
