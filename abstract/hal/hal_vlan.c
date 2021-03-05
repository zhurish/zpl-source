/*
 * hal_vlan.c
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
#include "os_list.h"

#include "nsm_vlan.h"

#include "hal_vlan.h"
#include "hal_driver.h"


int hal_vlan_enable(ospl_bool  enable)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_vlan_enable)
		return hal_driver->vlan_tbl->sdk_vlan_enable(hal_driver->driver, enable);
	return ERROR;
}

int hal_vlan_create(vlan_t vlan)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_vlan_create)
		return hal_driver->vlan_tbl->sdk_vlan_create(hal_driver->driver, vlan);
	return ERROR;
}

int hal_vlan_destroy(vlan_t vlan)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_vlan_delete)
		return hal_driver->vlan_tbl->sdk_vlan_delete(hal_driver->driver, vlan);
	return ERROR;
}

int hal_vlan_add_untag_port(ifindex_t ifindex, vlan_t vlan)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_vlan_add_untag_port)
		return hal_driver->vlan_tbl->sdk_vlan_add_untag_port(hal_driver->driver, ifindex, vlan);
	return ERROR;
}

int hal_vlan_del_untag_port(ifindex_t ifindex, vlan_t vlan)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_vlan_del_untag_port)
		return hal_driver->vlan_tbl->sdk_vlan_del_untag_port(hal_driver->driver, ifindex, vlan);
	return ERROR;
}

int hal_vlan_add_tag_port(ifindex_t ifindex, vlan_t vlan)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_vlan_add_tag_port)
		return hal_driver->vlan_tbl->sdk_vlan_add_tag_port(hal_driver->driver, ifindex, vlan);
	return ERROR;
}

int hal_vlan_del_tag_port(ifindex_t ifindex, vlan_t vlan)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_vlan_del_tag_port)
		return hal_driver->vlan_tbl->sdk_vlan_del_tag_port(hal_driver->driver, ifindex, vlan);
	return ERROR;
}

int hal_port_add_native_vlan(ifindex_t ifindex, vlan_t vlan)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_port_set_native_vlan)
		return hal_driver->vlan_tbl->sdk_port_set_native_vlan(hal_driver->driver, ifindex, vlan);
	return ERROR;
}

int hal_port_del_native_vlan(ifindex_t ifindex, vlan_t vlan)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_port_unset_native_vlan)
		return hal_driver->vlan_tbl->sdk_port_unset_native_vlan(hal_driver->driver, ifindex, vlan);
	return ERROR;
}

int hal_port_add_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_port_add_allowed_tag_vlan)
		return hal_driver->vlan_tbl->sdk_port_add_allowed_tag_vlan(hal_driver->driver, ifindex, vlan);
	return ERROR;
}

int hal_port_del_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_port_del_allowed_tag_vlan)
		return hal_driver->vlan_tbl->sdk_port_del_allowed_tag_vlan(hal_driver->driver, ifindex, vlan);
	return ERROR;
}

int hal_port_add_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_port_add_allowed_tag_vlan)
		return hal_driver->vlan_tbl->sdk_port_add_allowed_tag_batch_vlan(hal_driver->driver, ifindex, start, end);
	return ERROR;
}

int hal_port_del_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_port_del_allowed_tag_vlan)
		return hal_driver->vlan_tbl->sdk_port_del_allowed_tag_batch_vlan(hal_driver->driver, ifindex, start, end);
	return ERROR;
}

int hal_port_set_vlan(ifindex_t ifindex, vlan_t vlan)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_port_set_pvid_vlan)
		return hal_driver->vlan_tbl->sdk_port_set_pvid_vlan(hal_driver->driver, ifindex, vlan);
	return ERROR;
}

int hal_port_unset_vlan(ifindex_t ifindex, vlan_t vlan)
{
	if(hal_driver && hal_driver->vlan_tbl && hal_driver->vlan_tbl->sdk_port_unset_pvid_vlan)
		return hal_driver->vlan_tbl->sdk_port_unset_pvid_vlan(hal_driver->driver, ifindex, vlan);
	return ERROR;
}

#if 0
#if 0
/* vlan_mode_set            */  drv_vlan_mode_set,
/* vlan_mode_get            */  drv_vlan_mode_get,
/* port_vlan_pvid_set       */  drv_port_vlan_pvid_set,
/* port_vlan_pvid_get       */  drv_port_vlan_pvid_get,
/* port_vlan_set            */  drv_port_vlan_set,
/* port_vlan_get            */  drv_port_vlan_get,
/* vlan_property_set        */  drv_bcm53115_vlan_prop_set,
/* vlan_property_get        */  drv_bcm53115_vlan_prop_get,
/* vlan_prop_port_enable_set */ drv_bcm53115_vlan_prop_port_enable_set,
/* vlan_prop_port_enable_get */ drv_bcm53115_vlan_prop_port_enable_get,
/* vlan_vt_set              */  drv_bcm53115_vlan_vt_set,
/* vlan_vt_get              */  drv_bcm53115_vlan_vt_get,
/* vlan_vt_add              */  drv_bcm53115_vlan_vt_add,
/* vlan_vt_delete           */  drv_bcm53115_vlan_vt_delete,
/* vlan_vt_delete_all       */  drv_bcm53115_vlan_vt_delete_all,
#endif
int    (*sdk_port_set_native_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_unset_native_vlan)(ifindex_t, vlan_t);

int    (*sdk_port_set_access_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_unset_access_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_add_access_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_del_access_vlan)(ifindex_t, vlan_t);


int    (*sdk_port_set_trunk_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_unset_trunk_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_add_trunk_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_del_trunk_vlan)(ifindex_t, vlan_t);
#endif
