/*
 * hal_qinq.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_QINQ_H__
#define __HAL_QINQ_H__


#include "nsm_vlan.h"


typedef struct sdk_qinq_s
{
	int (*sdk_qinq_enable_cb) (BOOL);
	int (*sdk_qinq_vlan_ptid_cb) (vlan_t);
	int (*sdk_qinq_port_enable_cb) (ifindex_t, BOOL);
}sdk_qinq_t;

int hal_qinq_enable(BOOL enable);
int hal_qinq_vlan_tpid(vlan_t tpid);
int hal_qinq_interface_enable(ifindex_t ifindex, BOOL enable);




#endif /* __HAL_QINQ_H__ */
