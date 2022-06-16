/*
 * bsp_trunk.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "bsp_types.h"
#include "hal_client.h"
#include "bsp_trunk.h"



sdk_trunk_t sdk_trunk;

static int bsp_trunk_enable(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_trunk.sdk_trunk_enable_cb)
		ret = sdk_trunk.sdk_trunk_enable_cb(driver, param->enable);
	BSP_LEAVE_FUNC();	
	return ret;
}
static int bsp_trunk_create(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_trunk.sdk_trunk_create_cb)
		ret = sdk_trunk.sdk_trunk_create_cb(driver, param->trunkid, param->enable);
	BSP_LEAVE_FUNC();
	return ret;
}
static int bsp_trunk_mode(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_trunk.sdk_trunk_mode_cb)
		ret = sdk_trunk.sdk_trunk_mode_cb(driver, param->trunkid, param->mode);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_trunk_add_interface(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_trunk.sdk_trunk_addif_cb)
		ret = sdk_trunk.sdk_trunk_addif_cb(driver, param->trunkid,  port->phyport);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_trunk_del_interface(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_trunk.sdk_trunk_delif_cb)
		ret = sdk_trunk.sdk_trunk_delif_cb(driver, param->trunkid, port->phyport);
	BSP_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_TRUNK_CMD_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_TRUNK_CMD_ENABLE, bsp_trunk_enable),
	HAL_CALLBACK_ENTRY(HAL_TRUNK_CMD_CREATE, bsp_trunk_create),
	HAL_CALLBACK_ENTRY(HAL_TRUNK_CMD_ADDIF, bsp_trunk_add_interface),
	HAL_CALLBACK_ENTRY(HAL_TRUNK_CMD_DELIF, bsp_trunk_del_interface),
	HAL_CALLBACK_ENTRY(HAL_TRUNK_CMD_MODE, bsp_trunk_mode),
};


int bsp_trunk_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_trunk_param_t	param;
	hal_port_header_t	bspport;
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
	switch(subcmd)
	{
	case HAL_TRUNK_CMD_ENABLE:
	case HAL_TRUNK_CMD_CREATE:
	case HAL_TRUNK_CMD_MODE:
	break;
	case HAL_TRUNK_CMD_ADDIF:
	case HAL_TRUNK_CMD_DELIF:
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
	break;
	}

	hal_ipcmsg_getl(&client->ipcmsg, &param.enable);
	hal_ipcmsg_getl(&client->ipcmsg, &param.trunkid);
	hal_ipcmsg_getl(&client->ipcmsg, &param.mode);

	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (callback->cmd_handle)(driver, &bspport, &param);
	break;
	default:
		break;
	}
	BSP_LEAVE_FUNC();
	return ret;
}
