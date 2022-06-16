/*
 * bsp_l3if.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_L3IF_H__
#define __BSP_L3IF_H__
#ifdef __cplusplus
extern "C" {
#endif

#ifndef ZPL_SDK_USER
enum hal_l3if_cmd 
{
    HAL_L3IF_NONE,
	  HAL_L3IF_CREATE,
	  HAL_L3IF_DELETE,
	  HAL_L3IF_ADDR_ADD,
	  HAL_L3IF_ADDR_DEL,
	  HAL_L3IF_DSTADDR_ADD,
	  HAL_L3IF_DSTADDR_DEL,
    HAL_L3IF_VRF,
	  HAL_L3IF_MAC,
};



typedef struct hal_l3if_param_s
{
  hal_port_header_t  port;
  char    ifname[64];
	zpl_uint8 l2if_type;
  vrf_id_t vrfid;
	mac_t mac[NSM_MAC_MAX];
}hal_l3if_param_t;

typedef struct hal_l3if_addr_param_s
{
  hal_port_header_t  port;
  zpl_uint8 family;
  zpl_uint8 prefixlen;
  union g_addr address;  
  zpl_uint8 sec;
}hal_l3if_addr_param_t;
#endif

typedef struct sdk_l3if_s
{
	int (*sdk_l3if_addif_cb) (void *, char *, zpl_phyport_t);
	int (*sdk_l3if_delif_cb) (void *, zpl_phyport_t);
	int (*sdk_l3if_mac_cb) (void *, zpl_index_t, mac_t*);
	int (*sdk_l3if_vrf_cb) (void *, zpl_index_t, vrf_id_t);
	int (*sdk_l3if_add_addr_cb) (void *, zpl_phyport_t, zpl_family_t, zpl_uint8, union g_addr, zpl_bool);
	int (*sdk_l3if_del_addr_cb) (void *, zpl_phyport_t, zpl_family_t, zpl_uint8, union g_addr, zpl_bool);
	int (*sdk_l3if_add_dstaddr_cb) (void *, zpl_phyport_t, zpl_family_t, zpl_uint8, union g_addr, zpl_bool);
	int (*sdk_l3if_del_dstaddr_cb) (void *, zpl_phyport_t, zpl_family_t, zpl_uint8, union g_addr, zpl_bool);
}sdk_l3if_t;

extern sdk_l3if_t sdk_l3if;
extern int bsp_l3if_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_L3IF_H__ */
