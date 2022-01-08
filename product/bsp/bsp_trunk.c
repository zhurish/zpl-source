/*
 * bsp_trunk.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_trunk.h"



sdk_trunk_t sdk_trunk;

static int bsp_trunk_enable(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	if(driver && sdk_trunk.sdk_trunk_enable_cb)
		return sdk_trunk.sdk_trunk_enable_cb(driver, param->enable);
	return NO_SDK;
}

static int bsp_trunk_mode(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	if(driver && sdk_trunk.sdk_trunk_mode_cb)
		return sdk_trunk.sdk_trunk_mode_cb(driver, param->mode);
	return NO_SDK;
}

static int bsp_trunk_interface_add(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	if(driver && sdk_trunk.sdk_trunk_add_cb)
		return sdk_trunk.sdk_trunk_add_cb(driver,  port->phyport, param->trunkid);
	return NO_SDK;
}

static int bsp_trunk_interface_del(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	if(driver && sdk_trunk.sdk_trunk_del_cb)
		return sdk_trunk.sdk_trunk_del_cb(driver, port->phyport, param->trunkid);
	return NO_SDK;
}



static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_TRUNK_CMD_ENABLE, bsp_trunk_enable),
	//HAL_CALLBACK_ENTRY(HAL_TRUNK_CMD_ENABLE, bsp_trunk_interface_enable),
	HAL_CALLBACK_ENTRY(HAL_TRUNK_CMD_ADDIF, bsp_trunk_interface_add),
	HAL_CALLBACK_ENTRY(HAL_TRUNK_CMD_DELIF, bsp_trunk_interface_del),
	HAL_CALLBACK_ENTRY(HAL_TRUNK_CMD_MODE, bsp_trunk_mode),
};


int bsp_trunk_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_trunk_param_t	param;
	hal_port_header_t	bspport;
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);

	hal_ipcmsg_getl(&client->ipcmsg, &param.trunkid);
	hal_ipcmsg_getl(&client->ipcmsg, &param.mode);
	hal_ipcmsg_getl(&client->ipcmsg, &param.enable);

	if(!(subcmd_table[subcmd].cmd_handle))
	{
		return NO_SDK;
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_SET:         //设置
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &param);
	break;
	case HAL_MODULE_CMD_GET:         //获取
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &param);
	break;
	case HAL_MODULE_CMD_ADD:         //添加
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &param);
	break;
	case HAL_MODULE_CMD_DEL:         //删除
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &param);
	break;
    case HAL_MODULE_CMD_DELALL:      //删除所有
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &param);
	break;
	default:
		break;
	}
	return ret;
}
