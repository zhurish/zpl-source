/*
 * hal_qinq.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "interface.h"
#include <log.h>
#include "os_list.h"

#include "nsm_client.h"

#include "hal_qinq.h"

sdk_qinq_t sdk_qinq;


int hal_qinq_enable(BOOL enable)
{
	if(sdk_qinq.sdk_qinq_enable_cb)
		return sdk_qinq.sdk_qinq_enable_cb(enable);
	return ERROR;
}

int hal_qinq_vlan_tpid(vlan_t tpid)
{
	if(sdk_qinq.sdk_qinq_vlan_ptid_cb)
		return sdk_qinq.sdk_qinq_vlan_ptid_cb(tpid);
	return ERROR;
}

int hal_qinq_interface_enable(ifindex_t ifindex, BOOL enable)
{
	if(sdk_qinq.sdk_qinq_port_enable_cb)
		return sdk_qinq.sdk_qinq_port_enable_cb(ifindex, enable);
	return ERROR;
}
