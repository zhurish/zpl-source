/*
 * hal_mstp.c
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
#include "nsm_vlan.h"

#include "hal_vlan.h"
#include "hal_mstp.h"
#include "hal_driver.h"

int hal_mstp_enable(BOOL enable)
{
	if(hal_driver && hal_driver->mstp_tbl && hal_driver->mstp_tbl->sdk_mstp_enable_cb)
		return hal_driver->mstp_tbl->sdk_mstp_enable_cb(hal_driver->driver, enable);
	return ERROR;
}

int hal_mstp_age(int age)
{
	if(hal_driver && hal_driver->mstp_tbl && hal_driver->mstp_tbl->sdk_mstp_age_cb)
		return hal_driver->mstp_tbl->sdk_mstp_age_cb(hal_driver->driver, age);
	return ERROR;
}

int hal_mstp_bypass(BOOL enable, int type)
{
	if(hal_driver && hal_driver->mstp_tbl && hal_driver->mstp_tbl->sdk_mstp_bypass_cb)
		return hal_driver->mstp_tbl->sdk_mstp_bypass_cb(hal_driver->driver, enable, type);
	return ERROR;
}

int hal_mstp_state(ifindex_t ifindex, int mstp, hal_port_stp_state_t state)
{
	if(hal_driver && hal_driver->mstp_tbl && hal_driver->mstp_tbl->sdk_mstp_state_cb)
		return hal_driver->mstp_tbl->sdk_mstp_state_cb(hal_driver->driver, ifindex, mstp, state);
	return ERROR;
}

int hal_stp_state(ifindex_t ifindex, hal_port_stp_state_t state)
{
	if(hal_driver && hal_driver->mstp_tbl && hal_driver->mstp_tbl->sdk_stp_state_cb)
		return hal_driver->mstp_tbl->sdk_stp_state_cb(hal_driver->driver, ifindex, state);
	return ERROR;
}
