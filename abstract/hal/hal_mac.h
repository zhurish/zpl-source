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

#pragma pack(1)
typedef struct hal_mac_tbl_s
{
	zpl_phyport_t	phyport;
	vlan_t			vlan;
	mac_t 			mac[NSM_MAC_MAX];
	vrf_id_t		vrfid;
	zpl_uint8 		is_valid:1;
	zpl_uint8 		is_age:1;
	zpl_uint8 		is_static:1;
}hal_mac_tbl_t;
#pragma pack(0)

typedef struct mac_table_info_s
{
	hal_mac_tbl_t *mactbl;
	zpl_uint32 macnum;
}mac_table_info_t;

typedef struct hal_mac_param_s
{
	vlan_t vlan;
	zpl_uint32 value;
	mac_t mac[NSM_MAC_MAX];
	mac_table_info_t table;
}hal_mac_param_t;



int hal_mac_age(zpl_uint32 age);
int hal_mac_add(ifindex_t ifindex, vlan_t vlan, mac_t *mac, zpl_uint32 pri);
int hal_mac_del(ifindex_t ifindex, vlan_t vlan, mac_t *mac, zpl_uint32 pri);
int hal_mac_clr(ifindex_t ifindex, vlan_t vlan);
int hal_mac_read(ifindex_t ifindex, vlan_t vlan, int (*callback)(zpl_uint8 *, zpl_uint32, void *), void  *pVoid);



#ifdef __cplusplus
}
#endif

#endif /* __HAL_MAC_H__ */
