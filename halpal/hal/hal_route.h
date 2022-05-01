/*
 * hal_route.h
 *
 *  Created on: 2019年9月10日
 *      Author: DELL
 */

#ifndef __HAL_ROUTE_H__
#define __HAL_ROUTE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexthop.h"

enum hal_route_cmd 
{
  HAL_ROUTE_NONE,
	HAL_ROUTE_ADD,
	HAL_ROUTE_DEL,
};


typedef struct hal_nexthop
{
  ifindex_t kifindex;
	hal_port_header_t  port;
  union g_addr gateway;
} hal_nexthop_t;


typedef struct hal_route_param_s
{
  safi_t safi;
  vrf_id_t vrf_id;
  zpl_uint8 family;
  zpl_uint32  table;
  zpl_uint8 prefixlen;
  union g_addr destination;
  union g_addr source;
  zpl_uint8 nexthop_num;
  hal_nexthop_t nexthop[16];
  zpl_uint8 processid;
  zpl_uint8 type;
  zpl_uint8 flags;
  zpl_uint8 distance;
  zpl_uint32 metric;
  zpl_uint32 tag;
  zpl_uint32 mtu;
}hal_route_param_t;


extern int hal_route_multipath_add(zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num);
extern int hal_route_multipath_del(zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_ROUTE_H__ */
