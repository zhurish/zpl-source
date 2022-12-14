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

sdk_trunk_t sdk_trunk;

int hal_trunkid(int trunkid)
{
	return (trunkid);
}

int hal_trunk_enable(BOOL enable)
{
	if(sdk_trunk.sdk_trunk_enable_cb)
		return sdk_trunk.sdk_trunk_enable_cb(enable);
	return OK;
}

int hal_trunk_mode(int mode)
{
	if(sdk_trunk.sdk_trunk_mode_cb)
		return sdk_trunk.sdk_trunk_mode_cb(mode);
	return OK;
}

int hal_trunk_interface_enable(ifindex_t ifindex, int trunkid)
{
	if(sdk_trunk.sdk_trunk_add_cb)
		return sdk_trunk.sdk_trunk_add_cb(ifindex, trunkid);
	return OK;
}

int hal_trunk_interface_disable(ifindex_t ifindex, int trunkid)
{
	if(sdk_trunk.sdk_trunk_del_cb)
		return sdk_trunk.sdk_trunk_del_cb(ifindex, trunkid);
	return OK;
}

