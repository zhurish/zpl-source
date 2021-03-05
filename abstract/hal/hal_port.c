/*
 * hal_port.c
 *
 *  Created on: Jan 21, 2018
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


#include "nsm_client.h"

#include "hal_port.h"
#include "hal_driver.h"


int hal_port_up(ifindex_t ifindex)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_link_cb)
		return hal_driver->port_tbl->sdk_port_link_cb(hal_driver->driver, ifindex, ospl_true);
	return ERROR;
}

int hal_port_down(ifindex_t ifindex)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_link_cb)
		return hal_driver->port_tbl->sdk_port_link_cb(hal_driver->driver, ifindex, ospl_false);
	return ERROR;
}

/*int hal_port_address_set(ifindex_t ifindex, struct prefix *cp, int secondry)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_set_address_cb)
		return hal_driver->port_tbl->sdk_port_set_address_cb(hal_driver->driver, ifindex, cp, secondry);
	return ERROR;
}

int hal_port_address_unset(ifindex_t ifindex, struct prefix *cp, int secondry)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_unset_address_cb)
		return hal_driver->port_tbl->sdk_port_unset_address_cb(hal_driver->driver, ifindex, cp, secondry);
	return ERROR;
}*/

int hal_port_mac_set(ifindex_t ifindex, ospl_uint8 *cp, ospl_bool secondry)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_mac_cb)
		return hal_driver->port_tbl->sdk_port_mac_cb(hal_driver->driver, ifindex, cp, secondry);
	return ERROR;
}

int hal_port_mtu_set(ifindex_t ifindex, ospl_uint32 value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_mtu_cb)
		return hal_driver->port_tbl->sdk_port_mtu_cb(hal_driver->driver, ifindex,  value);
	return ERROR;
}

/*int hal_port_metric_set(ifindex_t ifindex, ospl_uint32 value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_metric_cb)
		return hal_driver->port_tbl->sdk_port_metric_cb(hal_driver->driver, ifindex,  value);
	return ERROR;
}*/

int hal_port_vrf_set(ifindex_t ifindex, ospl_uint32 value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_vrf_cb)
		return hal_driver->port_tbl->sdk_port_vrf_cb(hal_driver->driver, ifindex,  value);
	return ERROR;
}

/*int hal_port_multicast_set(ifindex_t ifindex, int value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_multicast_cb)
		return hal_driver->port_tbl->sdk_port_multicast_cb(hal_driver->driver, ifindex,  value);
	return ERROR;
}

int hal_port_bandwidth_set(ifindex_t ifindex, int value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_bandwidth_cb)
		return hal_driver->port_tbl->sdk_port_bandwidth_cb(hal_driver->driver, ifindex,  value);
	return ERROR;
}*/

int hal_port_speed_set(ifindex_t ifindex, ospl_uint32 value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_speed_cb)
		return hal_driver->port_tbl->sdk_port_speed_cb(hal_driver->driver, ifindex,  value);
	return ERROR;
}

int hal_port_duplex_set(ifindex_t ifindex, ospl_uint32 value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_duplex_cb)
		return hal_driver->port_tbl->sdk_port_duplex_cb(hal_driver->driver, ifindex,  value);
	return ERROR;
}

int hal_port_mode_set(ifindex_t ifindex, ospl_uint32 value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_mode_cb)
		return hal_driver->port_tbl->sdk_port_mode_cb(hal_driver->driver, ifindex,  value);
	return ERROR;
}

/*
int hal_port_linkdetect_set(ifindex_t ifindex, int value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_linkdetect_cb)
		return hal_driver->port_tbl->sdk_port_linkdetect_cb(hal_driver->driver, ifindex,  value);
	return ERROR;
}
*/


int hal_port_loop_set(ifindex_t ifindex, ospl_uint32 value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_loop_cb)
		return hal_driver->port_tbl->sdk_port_loop_cb(hal_driver->driver, ifindex,  value);
	return ERROR;
}

int hal_port_8021x_set(ifindex_t ifindex, ospl_uint32 value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_8021x_cb)
		return hal_driver->port_tbl->sdk_port_8021x_cb(hal_driver->driver, ifindex,  value);
	return ERROR;
}

int hal_port_jumbo_set(ifindex_t ifindex, ospl_bool enable)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_jumbo_cb)
		return hal_driver->port_tbl->sdk_port_jumbo_cb(hal_driver->driver, ifindex,  enable);
	return hal_jumbo_interface_enable(ifindex, enable);
}

int hal_port_enable_set(ifindex_t ifindex, ospl_bool enable)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_enable_cb)
		return hal_driver->port_tbl->sdk_port_enable_cb(hal_driver->driver, ifindex,  enable);
	return ERROR;
}

ospl_bool hal_port_state_get(ifindex_t ifindex)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_state_get_cb)
		return hal_driver->port_tbl->sdk_port_state_get_cb(hal_driver->driver, ifindex);
	return ospl_false;
}

ospl_uint32 hal_port_speed_get(ifindex_t ifindex)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_speed_get_cb)
		return hal_driver->port_tbl->sdk_port_speed_get_cb(hal_driver->driver, ifindex);
	return ospl_false;
}

ospl_uint32 hal_port_duplex_get(ifindex_t ifindex)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_duplex_get_cb)
		return hal_driver->port_tbl->sdk_port_duplex_get_cb(hal_driver->driver, ifindex);
	return ospl_false;
}

int hal_port_flow_set(ifindex_t ifindex, ospl_uint32 value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_flow_cb)
		return hal_driver->port_tbl->sdk_port_flow_cb(hal_driver->driver, ifindex,  value);
	return ERROR;
}

int hal_port_learning_set(ifindex_t ifindex, ospl_bool enable)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_learning_enable_cb)
		return hal_driver->port_tbl->sdk_port_learning_enable_cb(hal_driver->driver, ifindex,  enable);
	return ERROR;
}

int hal_port_software_learning_set(ifindex_t ifindex, ospl_bool enable)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_swlearning_enable_cb)
		return hal_driver->port_tbl->sdk_port_swlearning_enable_cb(hal_driver->driver, ifindex,  enable);
	return ERROR;
}

int hal_port_protected_set(ifindex_t ifindex, ospl_bool enable)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_protected_enable_cb)
		return hal_driver->port_tbl->sdk_port_protected_enable_cb(hal_driver->driver, ifindex,  enable);
	return ERROR;
}

int hal_port_wan_set(ifindex_t ifindex, ospl_bool enable)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_wan_enable_cb)
		return hal_driver->port_tbl->sdk_port_wan_enable_cb(hal_driver->driver, ifindex,  enable);
	return ERROR;
}
