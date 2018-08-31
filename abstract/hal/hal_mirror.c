/*
 * hal_mirror.c
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

#include "hal_mirror.h"

sdk_mirror_t sdk_mirror;



int hal_mirror_enable(ifindex_t ifindex, BOOL enable)
{
	if(sdk_mirror.sdk_mirror_enable_cb)
		return sdk_mirror.sdk_mirror_enable_cb(ifindex, enable);
	return ERROR;
}

int hal_mirror_source_enable(ifindex_t ifindex, int mode, BOOL enable)
{
	if(sdk_mirror.sdk_mirror_source_enable_cb)
		return sdk_mirror.sdk_mirror_source_enable_cb(enable, ifindex, mode);
	return ERROR;
}


int hal_mirror_source_filter_enable(BOOL enable, BOOL dst, mac_t *mac, int mode)
{
	if(sdk_mirror.sdk_mirror_source_filter_enable_cb)
		return sdk_mirror.sdk_mirror_source_filter_enable_cb(enable, dst, mac, mode);
	return ERROR;
}
