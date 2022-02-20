/*
 * hal_l3port.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __HAL_L3PORT_H__
#define __HAL_L3PORT_H__

#ifdef __cplusplus
extern "C" {
#endif
/* Create a new L3 interface. */
extern int hal_l3_interface_create(
    mac_t * mac_addr, 
    vlan_t vid, 
    int add_to_arl);

/* Create a new L3 interface with specified interface ID. */
extern int hal_l3_interface_id_create(
    mac_t * mac_addr, 
    vlan_t vid, 
    int intf_idx, 
    int add_to_arl);

/* Create/update a new/existing L3 interface with specified interface ID. */
extern int hal_l3_interface_id_update(
    mac_t * mac_addr, 
    vlan_t vid, 
    int intf_idx, 
    int add_to_arl);

/* Search for L3 interface based on MAC address and VLAN. */
extern int hal_l3_interface_lookup(
    mac_t * mac_addr, 
    vlan_t vid, 
    int *intf_id);

/* Delete a L3 interface. */
extern int hal_l3_interface_destroy(
    int intf_id);

/* Given the L3 interface number, return the MAC and VLAN. */
extern int hal_l3_interface_find(
    int intf_id, 
    vlan_t *vid, 
    mac_t * mac_addr);

/* Find the L3 interface by VLAN ID. */
extern int hal_l3_interface_find_by_vlan(
    vlan_t vid, 
    int *intf_id);

/* Lookup L3 host entry based on IP address. */
extern int hal_l3_ip_find(
    hal_l3_ip_t *info);

/* hal_l3_ip_find_index */
extern int hal_l3_ip_find_index(
    int index, 
    hal_l3_ip_t *info);

/* Add an entry into L3 host table. */
extern int hal_l3_ip_add(
    hal_l3_ip_t *info);

/* Delete an entry from the L3 host table. */
extern int hal_l3_ip_delete(
    hal_ip_t ip_addr);

/* Delete L3 host entries based on IP prefix (network). */
extern int hal_l3_ip_delete_by_prefix(
    hal_ip_t ip_addr, 
    hal_ip_t mask);

/* Delete L3 host entries that match L3 interface number. */
extern int hal_l3_ip_delete_by_interface(
    int intf);

/* Delete all L3 host table entries. */
extern int hal_l3_ip_delete_all(void);

#if 0
/* Add an IP route to the L3 route table. */
extern int hal_l3_route_add(
    hal_l3_route_t *info);

/* Delete an IP route from the DEFIP table. */
extern int hal_l3_route_delete(
    hal_l3_route_t *info);

/* Delete routes based on matching or non-matching L3 interface number. */
extern int hal_l3_route_delete_by_interface(
    hal_l3_route_t *info);

/* Delete all routes. */
extern int hal_l3_route_delete_all(
    hal_l3_route_t *info);

/* Look up a route given the network and netmask. */
extern int hal_l3_route_get(
    hal_l3_route_t *info);

/* Given a network, return all the paths for this route. */
extern int hal_l3_route_multipath_get(
    hal_l3_route_t *the_route, 
    hal_l3_route_t *path_array, 
    int max_path, 
    int *path_count);

/* Age the route table. */
extern int hal_l3_route_age(
    uint32 flags, 
    hal_l3_route_traverse_cb age_out, 
    void *user_data);
#endif

#ifdef __cplusplus
}
#endif


#endif /* __HAL_L3PORT_H__ */
