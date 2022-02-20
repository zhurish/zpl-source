/*
 * hal_l2mc.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_L2MC_H__
#define __HAL_L2MC_H__
#ifdef __cplusplus
extern "C" {
#endif

enum hal_l2mcast_cmd 
{
    HAL_L2MCAST_NONE,
    HAL_L2MCAST_ADD,
	HAL_L2MCAST_DEL,
	HAL_L2MCAST_JOIN,
    HAL_L2MCAST_LEAVE,
};

typedef struct hal_l2mcast_param_s
{
	vlan_t vid;
	mac_t mac[NSM_MAC_MAX];
}hal_l2mcast_param_t;

//添加二层组播
/* Add/Remove an entry in the multicast table. */
extern int hal_l2mcast_addr_add(
    mac_t * mac, 
    vlan_t vid);

/* Add/Remove an entry in the multicast table. */
extern int hal_l2mcast_addr_remove(
    mac_t * mac, 
    vlan_t vid);

//端口加入二层组播	
/* Add a given port to the membership of a given multicast group. */
extern int hal_l2mcast_join(mac_t *mcMacAddr, 
    vlan_t vlanId, 
    ifindex_t ifindex);

/* Remove a given port from the membership of a given multicast group. */
extern int hal_l2mcast_leave(mac_t *mcMacAddr, 
    vlan_t vlanId, 
    ifindex_t ifindex);
	
    
#ifdef __cplusplus
}
#endif

#endif /* __HAL_L2MC_H__ */
