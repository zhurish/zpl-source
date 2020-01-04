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
#include "interface.h"
#include <log.h>
#include "os_list.h"

#include "nsm_client.h"

#include "hal_8021x.h"
#include "hal_driver.h"



int hal_8021x_interface_enable(ifindex_t ifindex, BOOL enable)
{
	if(hal_driver && hal_driver->q8021x_tbl && hal_driver->q8021x_tbl->sdk_8021x_enable_cb)
		return hal_driver->q8021x_tbl->sdk_8021x_enable_cb(hal_driver->driver, ifindex,enable);
	return ERROR;
}

int hal_8021x_state(ifindex_t ifindex, u_int value)
{
	if(hal_driver && hal_driver->q8021x_tbl && hal_driver->q8021x_tbl->sdk_8021x_state_cb)
		return hal_driver->q8021x_tbl->sdk_8021x_state_cb(hal_driver->driver, ifindex, value);
	return ERROR;
}

int hal_8021x_mode(ifindex_t ifindex, u_int value)
{
	if(hal_driver && hal_driver->q8021x_tbl && hal_driver->q8021x_tbl->sdk_8021x_mode_cb)
		return hal_driver->q8021x_tbl->sdk_8021x_mode_cb(hal_driver->driver, ifindex, value);
	return ERROR;
}

int hal_8021x_auth_bypass(ifindex_t ifindex, u_int value)
{
	if(hal_driver && hal_driver->q8021x_tbl && hal_driver->q8021x_tbl->sdk_8021x_auth_bypass_cb)
		return hal_driver->q8021x_tbl->sdk_8021x_auth_bypass_cb(hal_driver->driver, ifindex, value);
	return ERROR;
}

/*
int hal_8021x_interface_dstmac(ifindex_t ifindex, u_char *mac)
{
	if(sdk_8021x.sdk_8021x_dstmac_cb)
		return sdk_8021x.sdk_8021x_dstmac_cb(ifindex, mac);
	return ERROR;
}

int hal_8021x_address(u_int index, u_int address, u_int mask)
{
	if(sdk_8021x.sdk_8021x_address_cb)
		return sdk_8021x.sdk_8021x_address_cb(index, address, mask);
	return ERROR;
}

int hal_8021x_mult_address(u_int index, BOOL enable)
{
	if(sdk_8021x.sdk_8021x_mult_address_cb)
		return sdk_8021x.sdk_8021x_mult_address_cb(index, enable);
	return ERROR;
}

int hal_8021x_mode(u_int index, BOOL enable)
{
	if(sdk_8021x.sdk_8021x_mode_cb)
		return sdk_8021x.sdk_8021x_mode_cb(index, enable);
	return ERROR;
}*/


