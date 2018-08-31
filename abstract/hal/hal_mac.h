/*
 * hal_mac.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_MAC_H__
#define __HAL_MAC_H__

#include "nsm_vlan.h"
#include "nsm_mac.h"

typedef struct sdk_mac_s
{
	int (*sdk_mac_age_cb) (int);
	int (*sdk_mac_add_cb) (ifindex_t, vlan_t, mac_t *, int);
	int (*sdk_mac_del_cb) (ifindex_t, vlan_t, mac_t *, int);
	int (*sdk_mac_clr_cb) (ifindex_t, vlan_t);
	int (*sdk_mac_read_cb) (ifindex_t, vlan_t);
}sdk_mac_t;


int hal_mac_age(int age);
int hal_mac_add(ifindex_t ifindex, vlan_t vlan, mac_t *mac, int pri);
int hal_mac_del(ifindex_t ifindex, vlan_t vlan, mac_t *mac, int pri);
int hal_mac_clr(ifindex_t ifindex, vlan_t vlan);
int hal_mac_read(ifindex_t ifindex, vlan_t vlan);


#endif /* __HAL_MAC_H__ */
