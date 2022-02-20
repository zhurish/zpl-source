/*
 * hal_vrrp.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_VRRP_H__
#define __HAL_VRRP_H__

#ifdef __cplusplus
extern "C" {
#endif
/* 
 * Add VRID for the given VSI. Adding a VRID using this API means the
 * physical node has become the master for the virtual router
 */
extern int hal_l3_vrrp_add(
    vlan_t vlan, 
    zpl_uint32 vrid);

/* Delete VRID for a particulat VLAN/VSI */
extern int hal_l3_vrrp_delete(
    vlan_t vlan, 
    zpl_uint32 vrid);

/* Delete all the VRIDs for a particular VLAN/VSI */
extern int hal_l3_vrrp_delete_all(
    vlan_t vlan);

/* 
 * Get all the VRIDs for which the physical node is master for the
 * virtual routers on the given VLAN/VSI
 */
extern int hal_l3_vrrp_get(
    vlan_t vlan, 
    int alloc_size, 
    int *vrid_array, 
    int *count);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_RATE_H__ */
