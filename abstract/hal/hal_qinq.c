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
#include "nsm_interface.h"
#include <log.h>
#include "os_list.h"

#include "nsm_client.h"

#include "hal_qinq.h"
#include "hal_driver.h"

int hal_qinq_enable(ospl_bool enable)
{
	if(hal_driver && hal_driver->qinq_tbl && hal_driver->qinq_tbl->sdk_qinq_enable_cb)
		return hal_driver->qinq_tbl->sdk_qinq_enable_cb(hal_driver->driver, enable);
	return ERROR;
}

int hal_qinq_vlan_tpid(vlan_t tpid)
{
	if(hal_driver && hal_driver->qinq_tbl && hal_driver->qinq_tbl->sdk_qinq_vlan_ptid_cb)
		return hal_driver->qinq_tbl->sdk_qinq_vlan_ptid_cb(hal_driver->driver, tpid);
	return ERROR;
}

int hal_qinq_interface_enable(ifindex_t ifindex, ospl_bool enable)
{
	if(hal_driver && hal_driver->qinq_tbl && hal_driver->qinq_tbl->sdk_qinq_port_enable_cb)
		return hal_driver->qinq_tbl->sdk_qinq_port_enable_cb(hal_driver->driver, ifindex, enable);
	return ERROR;
}
