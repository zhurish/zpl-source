/*
 * hal_misc.c
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

#include "hal_misc.h"
#include "hal_driver.h"


//jumbo
int hal_jumbo_size(ospl_uint32 size)
{
	if(hal_driver && hal_driver->misc_tbl && hal_driver->misc_tbl->sdk_jumbo_size_cb)
		return hal_driver->misc_tbl->sdk_jumbo_size_cb(hal_driver->driver, size);
	return ERROR;
}

int hal_jumbo_interface_enable(ifindex_t ifindex, ospl_bool enable)
{
	if(hal_driver && hal_driver->misc_tbl && hal_driver->misc_tbl->sdk_jumbo_enable_cb)
		return hal_driver->misc_tbl->sdk_jumbo_enable_cb(hal_driver->driver, ifindex, enable);
	return ERROR;
}



int hal_snooping_enable (ospl_bool enable, ospl_uint32 mode, ospl_bool ipv6)
{
	if(hal_driver && hal_driver->misc_tbl && hal_driver->misc_tbl->sdk_snooping_cb)
		return hal_driver->misc_tbl->sdk_snooping_cb(hal_driver->driver, enable, mode, ipv6);
	return ERROR;
}



//EEE
int hal_eee_enable (ifindex_t ifindex, ospl_bool enable)
{
	if(hal_driver && hal_driver->misc_tbl && hal_driver->misc_tbl->sdk_eee_enable_cb)
		return hal_driver->misc_tbl->sdk_jumbo_enable_cb(hal_driver->driver, ifindex, enable);
	return ERROR;
}

int hal_eee_set (ifindex_t ifindex, void *eee)
{
	if(hal_driver && hal_driver->misc_tbl && hal_driver->misc_tbl->sdk_eee_set_cb)
		return hal_driver->misc_tbl->sdk_eee_set_cb(hal_driver->driver, ifindex, eee);
	return ERROR;
}

int hal_eee_unset (ifindex_t ifindex, void *eee)
{
	if(hal_driver && hal_driver->misc_tbl && hal_driver->misc_tbl->sdk_eee_unset_cb)
		return hal_driver->misc_tbl->sdk_eee_unset_cb(hal_driver->driver, ifindex, eee);
	return ERROR;
}
