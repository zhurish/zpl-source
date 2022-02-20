/*
 * hal_tunnel.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_TUNNEL_H__
#define __HAL_TUNNEL_H__
#ifdef __cplusplus
extern "C" {
#endif

/* Add a destination L2 address to trigger tunnel processing. */
extern int hal_l2_tunnel_add(mac_t *mac, 
    vlan_t vlan);

/* Clear a destination L2 address used to trigger tunnel processing. */
extern int hal_l2_tunnel_delete(mac_t *mac, 
    vlan_t vlan);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_TUNNEL_H__ */
