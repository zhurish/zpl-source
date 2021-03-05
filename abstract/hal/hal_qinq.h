/*
 * hal_qinq.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_QINQ_H__
#define __HAL_QINQ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nsm_vlan.h"


typedef struct sdk_qinq_s
{
	int (*sdk_qinq_enable_cb) (void *, ospl_bool);
	int (*sdk_qinq_vlan_ptid_cb) (void *, vlan_t);
	int (*sdk_qinq_port_enable_cb) (void *, ifindex_t, ospl_bool);

	void *sdk_driver;
}sdk_qinq_t;

int hal_qinq_enable(ospl_bool enable);
int hal_qinq_vlan_tpid(vlan_t tpid);
int hal_qinq_interface_enable(ifindex_t ifindex, ospl_bool enable);



#ifdef __cplusplus
}
#endif


#endif /* __HAL_QINQ_H__ */
