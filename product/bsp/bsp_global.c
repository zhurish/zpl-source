/*
 * bsp_global.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */



#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_global.h"

sdk_global_t sdk_global;

static int bsp_global_jumbo_size(void *driver, void *bsp, zpl_uint32 *size)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_global.sdk_jumbo_size_cb)
		ret = sdk_global.sdk_jumbo_size_cb(driver, *size);
	BSP_LEAVE_FUNC();
	return ret;
}


/*
 * Global
 */
static int bsp_switch_mode(void *driver, hal_global_header_t *global, zpl_bool *manage)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_global.sdk_switch_manege_cb)
		ret = sdk_global.sdk_switch_manege_cb(driver, global, *manage);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_switch_forward(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_global.sdk_switch_forward_cb)
		ret = sdk_global.sdk_switch_forward_cb(driver, global, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_multicast_flood(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_global.sdk_multicast_flood_cb)
		ret = sdk_global.sdk_multicast_flood_cb(driver, global, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_unicast_flood(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_global.sdk_unicast_flood_cb)
		ret = sdk_global.sdk_unicast_flood_cb(driver, global, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_multicast_learning(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_global.sdk_multicast_learning_cb)
		ret = sdk_global.sdk_multicast_learning_cb(driver, global, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_global_bpdu_enable(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_global.sdk_bpdu_enable_cb)
		ret = sdk_global.sdk_bpdu_enable_cb(driver, global, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_global_aging_time(void *driver, hal_global_header_t *global, zpl_uint32 *value)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_global.sdk_aging_time_cb)
		ret = sdk_global.sdk_aging_time_cb(driver, global, *value);
	BSP_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_GLOBAL_CMD_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_GLOBAL_CMD_JUMBO_SIZE, bsp_global_jumbo_size),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_MANEGE, bsp_switch_mode),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_FORWARD, bsp_switch_forward),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_MULTICAST_FLOOD, bsp_multicast_flood),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_UNICAST_FLOOD, bsp_unicast_flood),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_MULTICAST_LEARNING, bsp_multicast_learning),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_BPDU, bsp_global_bpdu_enable),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_AGINT, bsp_global_aging_time),
};



int bsp_global_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	zpl_uint32 value = 0;
	BSP_ENTER_FUNC();
	ret = bsp_driver_module_check(subcmd_table, sizeof(subcmd_table)/sizeof(subcmd_table[0]), subcmd);
	if(ret == 0)
	{
		BSP_LEAVE_FUNC();
		return NO_SDK;
	}
	hal_ipcmsg_getl(&client->ipcmsg, &value);

	if(!(subcmd_table[subcmd].cmd_handle))
	{
		BSP_LEAVE_FUNC();
		return NO_SDK;
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (subcmd_table[subcmd].cmd_handle)(driver, NULL, &value);
	break;
	default:
		break;
	}
	BSP_LEAVE_FUNC();
	return ret;
}
