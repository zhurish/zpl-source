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
#include "nsm_interface.h"
#include <log.h>
#include "os_list.h"

#include "nsm_client.h"

#include "hal_mirror.h"
#include "hal_driver.h"


int hal_mirror_enable(ifindex_t ifindex, BOOL enable)
{
	if(hal_driver && hal_driver->mirror_tbl && hal_driver->mirror_tbl->sdk_mirror_enable_cb)
		return hal_driver->mirror_tbl->sdk_mirror_enable_cb(hal_driver->driver, ifindex, enable);
	return ERROR;
}

int hal_mirror_source_enable(ifindex_t ifindex, BOOL enable, hal_mirror_mode_t mode, hal_mirror_type_t type)
{
	if(hal_driver && hal_driver->mirror_tbl && hal_driver->mirror_tbl->sdk_mirror_source_enable_cb)
		return hal_driver->mirror_tbl->sdk_mirror_source_enable_cb(hal_driver->driver, enable, ifindex, mode, type);
	return ERROR;
}


int hal_mirror_source_filter_enable(BOOL enable, hal_mirror_filter_t filter, hal_mirror_type_t type, mac_t *mac, mac_t *mac1)
{
	if(hal_driver && hal_driver->mirror_tbl && hal_driver->mirror_tbl->sdk_mirror_source_filter_enable_cb)
		return hal_driver->mirror_tbl->sdk_mirror_source_filter_enable_cb(hal_driver->driver, enable,
				filter, type, mac, mac1);
	return ERROR;
}
