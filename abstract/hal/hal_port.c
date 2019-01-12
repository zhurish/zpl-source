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
#include "interface.h"
#include <log.h>
#include "os_list.h"

#include "nsm_client.h"

#include "hal_port.h"

sdk_port_t sdk_port;



int hal_port_up(ifindex_t ifindex)
{
	if(sdk_port.sdk_port_up_cb)
		return sdk_port.sdk_port_up_cb(ifindex);
	return ERROR;
}

int hal_port_down(ifindex_t ifindex)
{
	if(sdk_port.sdk_port_down_cb)
		return sdk_port.sdk_port_down_cb(ifindex);
	return ERROR;
}

int hal_port_address_set(ifindex_t ifindex, struct prefix *cp, int secondry)
{
	if(sdk_port.sdk_port_set_address_cb)
		return sdk_port.sdk_port_set_address_cb(ifindex, cp, secondry);
	return ERROR;
}

int hal_port_address_unset(ifindex_t ifindex, struct prefix *cp, int secondry)
{
	if(sdk_port.sdk_port_unset_address_cb)
		return sdk_port.sdk_port_unset_address_cb(ifindex, cp, secondry);
	return ERROR;
}

int hal_port_mac_set(ifindex_t ifindex, unsigned char *cp, int secondry)
{
	if(sdk_port.sdk_port_mac_cb)
		return sdk_port.sdk_port_mac_cb(ifindex, cp, secondry);
	return ERROR;
}

int hal_port_mtu_set(ifindex_t ifindex, int value)
{
	if(sdk_port.sdk_port_mtu_cb)
		return sdk_port.sdk_port_mtu_cb(ifindex,  value);
	return ERROR;
}

int hal_port_metric_set(ifindex_t ifindex, int value)
{
	if(sdk_port.sdk_port_metric_cb)
		return sdk_port.sdk_port_metric_cb(ifindex,  value);
	return ERROR;
}

int hal_port_vrf_set(ifindex_t ifindex, int value)
{
	if(sdk_port.sdk_port_vrf_cb)
		return sdk_port.sdk_port_vrf_cb(ifindex,  value);
	return ERROR;
}

int hal_port_multicast_set(ifindex_t ifindex, int value)
{
	if(sdk_port.sdk_port_multicast_cb)
		return sdk_port.sdk_port_multicast_cb(ifindex,  value);
	return ERROR;
}

int hal_port_bandwidth_set(ifindex_t ifindex, int value)
{
	if(sdk_port.sdk_port_bandwidth_cb)
		return sdk_port.sdk_port_bandwidth_cb(ifindex,  value);
	return ERROR;
}

int hal_port_speed_set(ifindex_t ifindex, int value)
{
	if(sdk_port.sdk_port_speed_cb)
		return sdk_port.sdk_port_speed_cb(ifindex,  value);
	return ERROR;
}

int hal_port_duplex_set(ifindex_t ifindex, int value)
{
	if(sdk_port.sdk_port_duplex_cb)
		return sdk_port.sdk_port_duplex_cb(ifindex,  value);
	return ERROR;
}

int hal_port_mode_set(ifindex_t ifindex, int value)
{
	if(sdk_port.sdk_port_mode_cb)
		return sdk_port.sdk_port_mode_cb(ifindex,  value);
	return ERROR;
}

int hal_port_linkdetect_set(ifindex_t ifindex, int value)
{
	if(sdk_port.sdk_port_linkdetect_cb)
		return sdk_port.sdk_port_linkdetect_cb(ifindex,  value);
	return ERROR;
}

int hal_port_stp_set(ifindex_t ifindex, int value)
{
	if(sdk_port.sdk_port_stp_cb)
		return sdk_port.sdk_port_stp_cb(ifindex,  value);
	return ERROR;
}

int hal_port_loop_set(ifindex_t ifindex, int value)
{
	if(sdk_port.sdk_port_loop_cb)
		return sdk_port.sdk_port_loop_cb(ifindex,  value);
	return ERROR;
}

int hal_port_8021x_set(ifindex_t ifindex, int value)
{
	if(sdk_port.sdk_port_8021x_cb)
		return sdk_port.sdk_port_8021x_cb(ifindex,  value);
	return ERROR;
}


