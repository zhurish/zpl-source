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

typedef struct sdk_l3if_s
{
	int (*sdk_l3if_addif_cb) (void *, zpl_phyport_t);
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
