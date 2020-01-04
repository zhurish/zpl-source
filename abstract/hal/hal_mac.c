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
#include "hal_driver.h"

int hal_mac_age(int age)
{
	if(hal_driver && hal_driver->mac_tbl && hal_driver->mac_tbl->sdk_mac_age_cb)
		return hal_driver->mac_tbl->sdk_mac_age_cb(hal_driver->driver, age);
	return ERROR;
}

int hal_mac_add(ifindex_t ifindex, vlan_t vlan, mac_t *mac, int pri)
{
	if(hal_driver && hal_driver->mac_tbl && hal_driver->mac_tbl->sdk_mac_add_cb)
		return hal_driver->mac_tbl->sdk_mac_add_cb(hal_driver->driver, ifindex, vlan, mac, pri);
	return ERROR;
}

int hal_mac_del(ifindex_t ifindex, vlan_t vlan, mac_t *mac, int pri)
{
	if(hal_driver && hal_driver->mac_tbl && hal_driver->mac_tbl->sdk_mac_del_cb)
		return hal_driver->mac_tbl->sdk_mac_del_cb(hal_driver->driver, ifindex, vlan, mac, pri);
	return ERROR;
}

int hal_mac_clr(ifindex_t ifindex, vlan_t vlan)
{
	if(hal_driver && hal_driver->mac_tbl && hal_driver->mac_tbl->sdk_mac_clr_cb)
		return hal_driver->mac_tbl->sdk_mac_clr_cb(hal_driver->driver, ifindex, vlan);
	return ERROR;
}

int hal_mac_read(ifindex_t ifindex, vlan_t vlan)
{
	if(hal_driver && hal_driver->mac_tbl && hal_driver->mac_tbl->sdk_mac_read_cb)
		return hal_driver->mac_tbl->sdk_mac_read_cb(hal_driver->driver, ifindex, vlan);
	return ERROR;
}

