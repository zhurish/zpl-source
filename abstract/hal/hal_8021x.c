/*
 * hal_8021x.c
 *
 *  Created on: May 7, 2018
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

#include "hal_8021x.h"
#include "hal_driver.h"



int hal_8021x_interface_enable(ifindex_t ifindex, ospl_bool enable)
{
	if(hal_driver && hal_driver->q8021x_tbl && hal_driver->q8021x_tbl->sdk_8021x_enable_cb)
		return hal_driver->q8021x_tbl->sdk_8021x_enable_cb(hal_driver->driver, ifindex,enable);
	return ERROR;
}

int hal_8021x_state(ifindex_t ifindex, ospl_uint32 value)
{
	if(hal_driver && hal_driver->q8021x_tbl && hal_driver->q8021x_tbl->sdk_8021x_state_cb)
		return hal_driver->q8021x_tbl->sdk_8021x_state_cb(hal_driver->driver, ifindex, value);
	return ERROR;
}

int hal_8021x_mode(ifindex_t ifindex, ospl_uint32 value)
{
	if(hal_driver && hal_driver->q8021x_tbl && hal_driver->q8021x_tbl->sdk_8021x_mode_cb)
		return hal_driver->q8021x_tbl->sdk_8021x_mode_cb(hal_driver->driver, ifindex, value);
	return ERROR;
}

int hal_8021x_auth_bypass(ifindex_t ifindex, ospl_uint32 value)
{
	if(hal_driver && hal_driver->q8021x_tbl && hal_driver->q8021x_tbl->sdk_8021x_auth_bypass_cb)
		return hal_driver->q8021x_tbl->sdk_8021x_auth_bypass_cb(hal_driver->driver, ifindex, value);
	return ERROR;
}

/*
int hal_8021x_interface_dstmac(ifindex_t ifindex, ospl_uchar *mac)
{
	if(sdk_8021x.sdk_8021x_dstmac_cb)
		return sdk_8021x.sdk_8021x_dstmac_cb(ifindex, mac);
	return ERROR;
}

int hal_8021x_address(ospl_uint32 index, ospl_uint32 address, ospl_uint32 mask)
{
	if(sdk_8021x.sdk_8021x_address_cb)
		return sdk_8021x.sdk_8021x_address_cb(index, address, mask);
	return ERROR;
}

int hal_8021x_mult_address(ospl_uint32 index, ospl_bool enable)
{
	if(sdk_8021x.sdk_8021x_mult_address_cb)
		return sdk_8021x.sdk_8021x_mult_address_cb(index, enable);
	return ERROR;
}

int hal_8021x_mode(ospl_uint32 index, ospl_bool enable)
{
	if(sdk_8021x.sdk_8021x_mode_cb)
		return sdk_8021x.sdk_8021x_mode_cb(index, enable);
	return ERROR;
}*/


