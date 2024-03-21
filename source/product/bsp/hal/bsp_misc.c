/*
 * bsp_misc.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zpl_type.h"
#include "log.h"
#include "hal_client.h"
#include "hal_driver.h"
#include "hal_misc.h"
#if defined(ZPL_SDK_USER) || defined(ZPL_SDK_NONE)
#include "bsp_driver.h"
#endif
#include "bsp_misc.h"


#if 0

//jumbo
int bsp_jumbo_size(zpl_uint32 size)
{
	if(bsp_driver && bsp_driver->misc_tbl && bsp_driver->misc_tbl->sdk_jumbo_size_cb)
		return bsp_driver->misc_tbl->sdk_jumbo_size_cb(bsp_driver->driver, size);
	return NO_SDK;
}

int bsp_jumbo_interface_enable(ifindex_t ifindex, zpl_bool enable)
{
	if(bsp_driver && bsp_driver->misc_tbl && bsp_driver->misc_tbl->sdk_jumbo_enable_cb)
		return bsp_driver->misc_tbl->sdk_jumbo_enable_cb(bsp_driver->driver, ifindex, enable);
	return NO_SDK;
}



int bsp_snooping_enable (zpl_bool enable, zpl_uint32 mode, zpl_bool ipv6)
{
	if(bsp_driver && bsp_driver->misc_tbl && bsp_driver->misc_tbl->sdk_snooping_cb)
		return bsp_driver->misc_tbl->sdk_snooping_cb(bsp_driver->driver, enable, mode, ipv6);
	return NO_SDK;
}



//EEE
int bsp_eee_enable (ifindex_t ifindex, zpl_bool enable)
{
	if(bsp_driver && bsp_driver->misc_tbl && bsp_driver->misc_tbl->sdk_eee_enable_cb)
		return bsp_driver->misc_tbl->sdk_jumbo_enable_cb(bsp_driver->driver, ifindex, enable);
	return NO_SDK;
}

int bsp_eee_set (ifindex_t ifindex, void *eee)
{
	if(bsp_driver && bsp_driver->misc_tbl && bsp_driver->misc_tbl->sdk_eee_set_cb)
		return bsp_driver->misc_tbl->sdk_eee_set_cb(bsp_driver->driver, ifindex, eee);
	return NO_SDK;
}

int bsp_eee_unset (ifindex_t ifindex, void *eee)
{
	if(bsp_driver && bsp_driver->misc_tbl && bsp_driver->misc_tbl->sdk_eee_unset_cb)
		return bsp_driver->misc_tbl->sdk_eee_unset_cb(bsp_driver->driver, ifindex, eee);
	return NO_SDK;
}

#endif