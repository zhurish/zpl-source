/*
 * bsp_route.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_ROUTE_H__
#define __BSP_ROUTE_H__
#ifdef __cplusplus
extern "C" {
#endif
#ifndef ZPL_SDK_USER
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
#endif
typedef struct sdk_route_s
{
	int (*sdk_route_add_cb) (void *, hal_route_param_t *param);
	int (*sdk_route_del_cb) (void *, hal_route_param_t *param);
}sdk_route_t;

extern sdk_route_t sdk_route;
extern int bsp_route_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_ROUTE_H__ */
