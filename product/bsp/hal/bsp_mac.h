/*
 * bsp_mac.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_MAC_H__
#define __BSP_MAC_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef struct sdk_mac_s
{
	int (*sdk_mac_age_cb) (void *, zpl_uint32);
	int (*sdk_mac_add_cb) (void *, zpl_phyport_t , zpl_vlan_t , zpl_uint32 , mac_t *, zpl_uint32 );
	int (*sdk_mac_del_cb) (void *, zpl_phyport_t , zpl_vlan_t , zpl_uint32 , mac_t *, zpl_uint32 );
	int (*sdk_mac_clr_cb) (void *, zpl_phyport_t , zpl_vlan_t , zpl_uint32 );
	int (*sdk_mac_dump_cb) (void *, zpl_phyport_t , zpl_vlan_t , zpl_uint32 );
	int (*sdk_mac_read_cb) (void *, zpl_phyport_t , zpl_vlan_t , zpl_uint32, hal_mac_param_t *);
	int (*sdk_mac_clrall_cb) (void *);
}sdk_mac_t;

extern sdk_mac_t sdk_maccb;
extern int bsp_mac_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_MAC_H__ */
