/*
 * hal_trunk.c
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

#include "hal_trunk.h"
#include "hal_driver.h"


int hal_trunkid(int trunkid)
{
	return (trunkid);
}

int hal_trunk_enable(BOOL enable)
{
	if(hal_driver && hal_driver->trunk_tbl && hal_driver->trunk_tbl->sdk_trunk_enable_cb)
		return hal_driver->trunk_tbl->sdk_trunk_enable_cb(hal_driver->driver, enable);
	return OK;
}

int hal_trunk_mode(int mode)
{
	if(hal_driver && hal_driver->trunk_tbl && hal_driver->trunk_tbl->sdk_trunk_mode_cb)
		return hal_driver->trunk_tbl->sdk_trunk_mode_cb(hal_driver->driver, mode);
	return OK;
}

int hal_trunk_interface_enable(ifindex_t ifindex, int trunkid)
{
	if(hal_driver && hal_driver->trunk_tbl && hal_driver->trunk_tbl->sdk_trunk_add_cb)
		return hal_driver->trunk_tbl->sdk_trunk_add_cb(hal_driver->driver, ifindex, trunkid);
	return OK;
}

int hal_trunk_interface_disable(ifindex_t ifindex, int trunkid)
{
	if(hal_driver && hal_driver->trunk_tbl && hal_driver->trunk_tbl->sdk_trunk_del_cb)
		return hal_driver->trunk_tbl->sdk_trunk_del_cb(hal_driver->driver, ifindex, trunkid);
	return OK;
}

