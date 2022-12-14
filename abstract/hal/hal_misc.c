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
#include "interface.h"
#include <log.h>
#include "os_list.h"

#include "nsm_client.h"

#include "hal_misc.h"

sdk_misc_t sdk_misc;



//jumbo
int hal_jumbo_size(int size)
{
	if(sdk_misc.sdk_jumbo_size_cb)
		return sdk_misc.sdk_jumbo_size_cb(size);
	return ERROR;
}

int hal_jumbo_interface_enable(ifindex_t ifindex, BOOL enable)
{
	if(sdk_misc.sdk_jumbo_enable_cb)
		return sdk_misc.sdk_jumbo_enable_cb(ifindex, enable);
	return ERROR;
}



int hal_snooping_enable (BOOL enable, int mode, BOOL ipv6)
{
	if(sdk_misc.sdk_snooping_cb)
		return sdk_misc.sdk_snooping_cb(enable, mode, ipv6);
	return ERROR;
}



//EEE
int hal_eee_enable (ifindex_t ifindex, BOOL enable)
{
	if(sdk_misc.sdk_eee_enable_cb)
		return sdk_misc.sdk_jumbo_enable_cb(ifindex, enable);
	return ERROR;
}

int hal_eee_set (ifindex_t ifindex, void *eee)
{
	if(sdk_misc.sdk_eee_set_cb)
		return sdk_misc.sdk_eee_set_cb(ifindex, eee);
	return ERROR;
}

int hal_eee_unset (ifindex_t ifindex, void *eee)
{
	if(sdk_misc.sdk_eee_unset_cb)
		return sdk_misc.sdk_eee_unset_cb(ifindex, eee);
	return ERROR;
}
