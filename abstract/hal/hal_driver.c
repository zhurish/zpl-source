/*
 * hal_driver.c
 *
 *  Created on: 2019年9月8日
 *      Author: zhurish
 */

#include <zebra.h>

#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "nsm_interface.h"
#include <log.h>


#include "hal_driver.h"

#ifdef PL_SDK_MODULE
#include "sdk_driver.h"
#endif


hal_driver_t *hal_driver = NULL;

int hal_module_init()
{

	if(hal_driver)
		return OK;
	hal_driver = malloc(sizeof(hal_driver_t));
	if(hal_driver == NULL)
	{
		zlog_debug(ZLOG_HAL, " Can not malloc hal_driver_t");
		return ERROR;
	}
	memset(hal_driver, 0, sizeof(hal_driver_t));

#ifdef PL_SDK_MODULE
	sdk_module_init(hal_driver);
#endif

	//hal_test_init();

	return OK;
}



/*
 * CPU Port
 */
int hal_cpu_port_mode(BOOL enable)
{
	if(hal_driver && hal_driver->cpu_tbl && hal_driver->cpu_tbl->sdk_cpu_mode_cb)
		return hal_driver->cpu_tbl->sdk_cpu_mode_cb(hal_driver->driver, enable);
	return ERROR;
}

int hal_cpu_port_enable(BOOL enable)
{
	if(hal_driver && hal_driver->cpu_tbl && hal_driver->cpu_tbl->sdk_cpu_enable_cb)
		return hal_driver->cpu_tbl->sdk_cpu_enable_cb(hal_driver->driver, enable);
	return ERROR;
}

int hal_cpu_port_speed(u_int value)
{
	if(hal_driver && hal_driver->cpu_tbl && hal_driver->cpu_tbl->sdk_cpu_speed_cb)
		return hal_driver->cpu_tbl->sdk_cpu_speed_cb(hal_driver->driver, value);
	return ERROR;
}

int hal_cpu_port_duplex(u_int value)
{
	if(hal_driver && hal_driver->cpu_tbl && hal_driver->cpu_tbl->sdk_cpu_duplex_cb)
		return hal_driver->cpu_tbl->sdk_cpu_duplex_cb(hal_driver->driver, value);
	return ERROR;
}

int hal_cpu_port_flow(BOOL rx, BOOL tx)
{
	if(hal_driver && hal_driver->cpu_tbl && hal_driver->cpu_tbl->sdk_cpu_flow_cb)
		return hal_driver->cpu_tbl->sdk_cpu_flow_cb(hal_driver->driver, rx, tx);
	return ERROR;
}


/*
 * Global
 */
int hal_switch_mode(BOOL manage)
{
	if(hal_driver && hal_driver->global_tbl && hal_driver->global_tbl->sdk_switch_manege_cb)
		return hal_driver->global_tbl->sdk_switch_manege_cb(hal_driver->driver, manage);
	return ERROR;
}

int hal_switch_forward(BOOL enable)
{
	if(hal_driver && hal_driver->global_tbl && hal_driver->global_tbl->sdk_switch_forward_cb)
		return hal_driver->global_tbl->sdk_switch_forward_cb(hal_driver->driver, enable);
	return ERROR;
}


int hal_multicast_flood(BOOL enable)
{
	if(hal_driver && hal_driver->global_tbl && hal_driver->global_tbl->sdk_multicast_flood_cb)
		return hal_driver->global_tbl->sdk_multicast_flood_cb(hal_driver->driver, enable);
	return ERROR;
}

int hal_unicast_flood(BOOL enable)
{
	if(hal_driver && hal_driver->global_tbl && hal_driver->global_tbl->sdk_unicast_flood_cb)
		return hal_driver->global_tbl->sdk_unicast_flood_cb(hal_driver->driver, enable);
	return ERROR;
}


int hal_multicast_learning(BOOL enable)
{
	if(hal_driver && hal_driver->global_tbl && hal_driver->global_tbl->sdk_multicast_learning_cb)
		return hal_driver->global_tbl->sdk_multicast_learning_cb(hal_driver->driver, enable);
	return ERROR;
}


int hal_global_bpdu_enable(BOOL enable)
{
	if(hal_driver && hal_driver->global_tbl && hal_driver->global_tbl->sdk_bpdu_enable_cb)
		return hal_driver->global_tbl->sdk_bpdu_enable_cb(hal_driver->driver, enable);
	return ERROR;
}


int hal_global_aging_time(u_int value)
{
	if(hal_driver && hal_driver->global_tbl && hal_driver->global_tbl->sdk_aging_time_cb)
		return hal_driver->global_tbl->sdk_aging_time_cb(hal_driver->driver, value);
	return ERROR;
}



