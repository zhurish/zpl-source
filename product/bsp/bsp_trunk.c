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
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_trunk.sdk_trunk_enable_cb)
		ret = sdk_trunk.sdk_trunk_enable_cb(driver, param->enable);
	SDK_LEAVE_FUNC();	
	return ret;
}
static int bsp_trunk_create(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_trunk.sdk_trunk_create_cb)
		ret = sdk_trunk.sdk_trunk_create_cb(driver, param->trunkid, param->enable);
	SDK_LEAVE_FUNC();
	return ret;
}
static int bsp_trunk_mode(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_trunk.sdk_trunk_mode_cb)
		ret = sdk_trunk.sdk_trunk_mode_cb(driver, param->trunkid, param->mode);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_trunk_add_interface(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_trunk.sdk_trunk_addif_cb)
		ret = sdk_trunk.sdk_trunk_addif_cb(driver, param->trunkid,  port->phyport);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_trunk_del_interface(void *driver, hal_port_header_t *port, hal_trunk_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_trunk.sdk_trunk_delif_cb)
		ret = sdk_trunk.sdk_trunk_delif_cb(driver, param->trunkid, port->phyport);
	SDK_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
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
	SDK_ENTER_FUNC();
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);

	hal_ipcmsg_getl(&client->ipcmsg, &param.trunkid);
	hal_ipcmsg_getl(&client->ipcmsg, &param.mode);
	hal_ipcmsg_getl(&client->ipcmsg, &param.enable);

	if(!(subcmd_table[subcmd].cmd_handle))
	{
		SDK_LEAVE_FUNC();
		return NO_SDK;
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &param);
	break;
	default:
		break;
	}
	SDK_LEAVE_FUNC();
	return ret;
}
