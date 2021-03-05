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

typedef struct sdk_mac_s
{
	int (*sdk_mac_age_cb) (void *, ospl_uint32);
	int (*sdk_mac_add_cb) (void *, ifindex_t, vlan_t, mac_t *, ospl_uint32);
	int (*sdk_mac_del_cb) (void *, ifindex_t, vlan_t, mac_t *, ospl_uint32);
	int (*sdk_mac_clr_cb) (void *, ifindex_t, vlan_t);
	int (*sdk_mac_read_cb) (void *, ifindex_t, vlan_t);
	void *sdk_driver;
}sdk_mac_t;


int hal_mac_age(ospl_uint32 age);
int hal_mac_add(ifindex_t ifindex, vlan_t vlan, mac_t *mac, ospl_uint32 pri);
int hal_mac_del(ifindex_t ifindex, vlan_t vlan, mac_t *mac, ospl_uint32 pri);
int hal_mac_clr(ifindex_t ifindex, vlan_t vlan);
int hal_mac_read(ifindex_t ifindex, vlan_t vlan);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_MAC_H__ */
