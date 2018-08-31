/*
 * hal_mac.c
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

#include "hal_mac.h"

sdk_mac_t sdk_mac;


int hal_mac_age(int age)
{
	if(sdk_mac.sdk_mac_age_cb)
		return sdk_mac.sdk_mac_age_cb(age);
	return ERROR;
}

int hal_mac_add(ifindex_t ifindex, vlan_t vlan, mac_t *mac, int pri)
{
	if(sdk_mac.sdk_mac_add_cb)
		return sdk_mac.sdk_mac_add_cb(ifindex, vlan, mac, pri);
	return ERROR;
}

int hal_mac_del(ifindex_t ifindex, vlan_t vlan, mac_t *mac, int pri)
{
	if(sdk_mac.sdk_mac_del_cb)
		return sdk_mac.sdk_mac_del_cb(ifindex, vlan, mac, pri);
	return ERROR;
}

int hal_mac_clr(ifindex_t ifindex, vlan_t vlan)
{
	if(sdk_mac.sdk_mac_clr_cb)
		return sdk_mac.sdk_mac_clr_cb(ifindex, vlan);
	return ERROR;
}

int hal_mac_read(ifindex_t ifindex, vlan_t vlan)
{
	if(sdk_mac.sdk_mac_read_cb)
		return sdk_mac.sdk_mac_read_cb(ifindex, vlan);
	return ERROR;
}

