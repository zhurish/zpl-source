/*
 * bsp_mstp.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_MSTP_H__
#define __BSP_MSTP_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sdk_mstp_s
{
	int (*sdk_mstp_enable_cb) (void *, zpl_bool);
	int (*sdk_mstp_create) (void *, zpl_index_t);
	int (*sdk_mstp_add_vlan) (void *, zpl_index_t , vlan_t );
	int (*sdk_mstp_del_vlan) (void *, zpl_index_t , vlan_t );
	int (*sdk_mstp_aging_cb) (void *, zpl_index_t );
	int (*sdk_mstp_state) (void *, zpl_index_t, zpl_phyport_t, zpl_uint32);
	int (*sdk_stp_state_cb) (void *, zpl_index_t, zpl_phyport_t, zpl_uint32);
}sdk_mstp_t;

extern sdk_mstp_t sdk_mstp;
extern int bsp_mstp_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_MSTP_H__ */
