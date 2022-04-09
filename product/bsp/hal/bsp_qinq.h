/*
 * bsp_qinq.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_QINQ_H__
#define __BSP_QINQ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nsm_vlan.h"


typedef struct sdk_qinq_s
{
	int (*sdk_qinq_enable_cb) (void *, zpl_bool);
	int (*sdk_qinq_vlan_ptid_cb) (void *, vlan_t);
	int (*sdk_qinq_port_enable_cb) (void *, zpl_phyport_t, zpl_bool);

}sdk_qinq_t;

extern sdk_qinq_t sdk_qinq;
extern int bsp_qinq_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif


#endif /* __BSP_QINQ_H__ */
