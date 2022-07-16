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




#pragma pack(1)
typedef struct hal_mac_cache_s
{
	zpl_uint8 port;
	zpl_uint8 mac[ETH_ALEN];
	vlan_t vid;
	zpl_uint8 use:1;
	zpl_uint8 is_valid:1;
	zpl_uint8 is_age:1;
	zpl_uint8 is_static:1;
	zpl_uint8 res:4;
}hal_mac_cache_t;
#pragma pack(0)


typedef struct hal_mac_param_s
{
	vlan_t vlan;
	zpl_uint32 value;
	mac_t mac[NSM_MAC_MAX];
	zpl_uint32 macnum;
	hal_mac_cache_t *mactbl;
}hal_mac_param_t;



int hal_mac_age(zpl_uint32 age);
int hal_mac_clrall(void);
int hal_mac_add(ifindex_t ifindex, vlan_t vlan, mac_t *mac, zpl_uint32 pri);
int hal_mac_del(ifindex_t ifindex, vlan_t vlan, mac_t *mac, zpl_uint32 pri);
int hal_mac_clr(ifindex_t ifindex, vlan_t vlan);
int hal_mac_read(ifindex_t ifindex, vlan_t vlan, int (*callback)(zpl_uint8 *, zpl_uint32, void *), void  *pVoid);
int hal_mac_dump(ifindex_t ifindex, vlan_t vlan);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_MAC_H__ */
