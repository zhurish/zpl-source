/*
 * bsp_cpu.c
 *
 *  Created on: 2019年9月8日
 *      Author: zhurish
 */

#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_cpu.h"

sdk_global_t sdk_global_cb;
sdk_cpu_t sdk_cpu_cb;
/*
 * CPU Port
 */
static int bsp_cpu_port_mode(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	if(driver && sdk_cpu_cb.sdk_cpu_mode_cb)
		return sdk_cpu_cb.sdk_cpu_mode_cb(driver, global, *enable);
	return NO_SDK;
}

static int bsp_cpu_port_enable(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	if(driver && sdk_cpu_cb.sdk_cpu_enable_cb)
		return sdk_cpu_cb.sdk_cpu_enable_cb(driver, global, *enable);
	return NO_SDK;
}

static int bsp_cpu_port_speed(void *driver, hal_global_header_t *global, zpl_uint32 *value)
{
	if(driver && sdk_cpu_cb.sdk_cpu_speed_cb)
		return sdk_cpu_cb.sdk_cpu_speed_cb(driver, global, *value);
	return NO_SDK;
}

static int bsp_cpu_port_duplex(void *driver, hal_global_header_t *global, zpl_uint32 *value)
{
	if(driver && sdk_cpu_cb.sdk_cpu_duplex_cb)
		return sdk_cpu_cb.sdk_cpu_duplex_cb(driver, global, *value);
	return NO_SDK;
}

static int bsp_cpu_port_flow(void *driver, hal_global_header_t *global, zpl_bool *tx)
{
	if(driver && sdk_cpu_cb.sdk_cpu_flow_cb)
		return sdk_cpu_cb.sdk_cpu_flow_cb(driver, global, *tx);
	return NO_SDK;
}


/*
 * Global
 */
static int bsp_switch_mode(void *driver, hal_global_header_t *global, zpl_bool *manage)
{
	if(driver && sdk_global_cb.sdk_switch_manege_cb)
		return sdk_global_cb.sdk_switch_manege_cb(driver, global, *manage);
	return NO_SDK;
}

static int bsp_switch_forward(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	if(driver && sdk_global_cb.sdk_switch_forward_cb)
		return sdk_global_cb.sdk_switch_forward_cb(driver, global, *enable);
	return NO_SDK;
}


static int bsp_multicast_flood(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	if(driver && sdk_global_cb.sdk_multicast_flood_cb)
		return sdk_global_cb.sdk_multicast_flood_cb(driver, global, *enable);
	return NO_SDK;
}

static int bsp_unicast_flood(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	if(driver && sdk_global_cb.sdk_unicast_flood_cb)
		return sdk_global_cb.sdk_unicast_flood_cb(driver, global, *enable);
	return NO_SDK;
}


static int bsp_multicast_learning(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	if(driver && sdk_global_cb.sdk_multicast_learning_cb)
		return sdk_global_cb.sdk_multicast_learning_cb(driver, global, *enable);
	return NO_SDK;
}


static int bsp_global_bpdu_enable(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	if(driver && sdk_global_cb.sdk_bpdu_enable_cb)
		return sdk_global_cb.sdk_bpdu_enable_cb(driver, global, *enable);
	return NO_SDK;
}


static int bsp_global_aging_time(void *driver, hal_global_header_t *global, zpl_uint32 *value)
{
	if(driver && sdk_global_cb.sdk_aging_time_cb)
		return sdk_global_cb.sdk_aging_time_cb(driver, global, *value);
	return NO_SDK;
}


static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_SWITCH_MANEGE, bsp_switch_mode),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_FORWARD, bsp_switch_forward),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_MULTICAST_FLOOD, bsp_multicast_flood),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_UNICAST_FLOOD, bsp_unicast_flood),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_MULTICAST_LEARNING, bsp_multicast_learning),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_BPDU, bsp_global_bpdu_enable),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_AGINT, bsp_global_aging_time),

	HAL_CALLBACK_ENTRY(HAL_SWITCH_CPU_MODE, bsp_cpu_port_mode),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_CPU, bsp_cpu_port_enable),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_CPU_SPEED, bsp_cpu_port_speed),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_CPU_DUPLEX, bsp_cpu_port_duplex),
	HAL_CALLBACK_ENTRY(HAL_SWITCH_CPU_FLOW, bsp_cpu_port_flow),
};


int bsp_global_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	int	value = 0;
	hal_global_header_t	global;
	hal_ipcmsg_global_get(&client->ipcmsg, &global);
	hal_ipcmsg_getc(&client->ipcmsg, &value);
	if(!(subcmd_table[subcmd].cmd_handle))
	{
		return NO_SDK;
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_SET:         //设置
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &global, &value);
	break;
	case HAL_MODULE_CMD_GET:         //获取
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &global, &value);
	break;
	case HAL_MODULE_CMD_ADD:         //添加
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &global, &value);
	break;
	case HAL_MODULE_CMD_DEL:         //删除
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &global, &value);
	break;
    case HAL_MODULE_CMD_DELALL:      //删除所有
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &global, &value);
	break;
	default:
		break;
	}
	return ret;
}

int bsp_cpu_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	int	value = 0;
	hal_global_header_t	global;
	hal_ipcmsg_global_get(&client->ipcmsg, &global);
	hal_ipcmsg_getc(&client->ipcmsg, &value);
	if(!(subcmd_table[subcmd].cmd_handle))
	{
		return NO_SDK;
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_SET:         //设置
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &global, &value);
	break;
	case HAL_MODULE_CMD_GET:         //获取
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &global, &value);
	break;
	case HAL_MODULE_CMD_ADD:         //添加
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &global, &value);
	break;
	case HAL_MODULE_CMD_DEL:         //删除
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &global, &value);
	break;
    case HAL_MODULE_CMD_DELALL:      //删除所有
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &global, &value);
	break;
	default:
		break;
	}
	return ret;
}