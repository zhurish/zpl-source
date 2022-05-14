/*
 * bsp_cpu.c
 *
 *  Created on: 2019年9月8日
 *      Author: zhurish
 */

#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_cpu.h"
#include "bsp_driver.h"

sdk_cpu_t sdk_cpu_cb;
/*
 * CPU Port
 */
static int bsp_cpu_port_mode(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_cpu_cb.sdk_cpu_mode_cb)
		ret = sdk_cpu_cb.sdk_cpu_mode_cb(driver, global, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_cpu_port_enable(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_cpu_cb.sdk_cpu_enable_cb)
		ret = sdk_cpu_cb.sdk_cpu_enable_cb(driver, global, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_cpu_port_speed(void *driver, hal_global_header_t *global, zpl_uint32 *value)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_cpu_cb.sdk_cpu_speed_cb)
		ret = sdk_cpu_cb.sdk_cpu_speed_cb(driver, global, *value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_cpu_port_duplex(void *driver, hal_global_header_t *global, zpl_uint32 *value)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_cpu_cb.sdk_cpu_duplex_cb)
		ret = sdk_cpu_cb.sdk_cpu_duplex_cb(driver, global, *value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_cpu_port_flow(void *driver, hal_global_header_t *global, zpl_bool *tx)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_cpu_cb.sdk_cpu_flow_cb)
		ret = sdk_cpu_cb.sdk_cpu_flow_cb(driver, global, *tx);
	BSP_LEAVE_FUNC();
	return ret;
}




static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_SWITCH_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_CPU_MODE, bsp_cpu_port_mode),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_CPU, bsp_cpu_port_enable),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_CPU_SPEED, bsp_cpu_port_speed),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_CPU_DUPLEX, bsp_cpu_port_duplex),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_CPU_FLOW, bsp_cpu_port_flow),
};


int bsp_cpu_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	zpl_uint8	value = 0;
	hal_global_header_t	global;
	int i = 0;
	hal_ipcsubcmd_callback_t * callback = NULL;
	BSP_ENTER_FUNC();	
	for(i = 0; i < ZPL_ARRAY_SIZE(subcmd_table); i++)
	{
		if(subcmd_table[i].subcmd == subcmd && subcmd_table[i].cmd_handle)
		{
            callback = &subcmd_table[i];
			break;
		}
	}
	if(callback == NULL)
	{
		zlog_warn(MODULE_HAL, "Can not Find this subcmd:%d ", subcmd);
		BSP_LEAVE_FUNC();
		return OS_NO_CALLBACK;
	}
	hal_ipcmsg_global_get(&client->ipcmsg, &global);
	hal_ipcmsg_getc(&client->ipcmsg, &value);	
	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (callback->cmd_handle)(driver, &global, &value);
	break;
	default:
		break;
	}
	BSP_LEAVE_FUNC();
	return ret;
}