/*
 * hal_l3if.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __HAL_L3IF_H__
#define __HAL_L3IF_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "if.h"
#include "nexthop.h"
typedef struct hal_l3if_param_s
{
  hal_port_header_t porthdr;
  char    ifname[IF_NAME_MAX];
	zpl_uint8 l2if_type;
  vrf_id_t vrfid;
	mac_t mac[NSM_MAC_MAX];
}hal_l3if_param_t;

typedef struct hal_l3if_addr_param_s
{
  hal_port_header_t porthdr;
  zpl_uint8 family;
  zpl_uint8 prefixlen;
  union g_addr address;  
  zpl_uint8 sec;
}hal_l3if_addr_param_t;

extern int hal_l3if_add(ifindex_t ifindex, char *name, mac_t *mac);
extern int hal_l3if_del(ifindex_t ifindex);

extern int hal_l3if_addr_add(ifindex_t ifindex, struct prefix *address, zpl_bool sec);
extern int hal_l3if_addr_del(ifindex_t ifindex, struct prefix *address, zpl_bool sec);
extern int hal_l3if_dstaddr_add(ifindex_t ifindex, struct prefix *address);
extern int hal_l3if_dstaddr_del(ifindex_t ifindex, struct prefix *address);

extern int hal_l3if_vrf_set(ifindex_t ifindex, vrf_id_t vrfid);
extern int hal_l3if_mac_set(ifindex_t ifindex, mac_t *mac);


#ifdef __cplusplus
}
#endif


#endif /* __HAL_L3IF_H__ */
