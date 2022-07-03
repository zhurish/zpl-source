/*
 * bsp_mirror.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */



#include "bsp_types.h"

#include "hal_client.h"
#include "bsp_mirror.h"


sdk_mirror_t sdk_mirror;

static int bsp_mirror_enable(void *driver, hal_port_header_t *bspport, hal_mirror_param_t *mirror)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_mirror.sdk_mirror_enable_cb)
		ret = sdk_mirror.sdk_mirror_enable_cb(bspdev->sdk_driver, bspport->phyport, mirror->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mirror_source_enable(void *driver, hal_port_header_t *bspport, hal_mirror_param_t *mirror)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_mirror.sdk_mirror_source_enable_cb)
		ret = sdk_mirror.sdk_mirror_source_enable_cb(bspdev->sdk_driver, bspport->phyport, mirror->value, mirror->dir);
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_mirror_source_filter_enable(void *driver, hal_port_header_t *bspport, hal_mirror_param_t *mirror)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_mirror.sdk_mirror_source_filter_enable_cb)
		ret = sdk_mirror.sdk_mirror_source_filter_enable_cb(bspdev->sdk_driver, bspport->phyport, mirror->value, mirror->filter, mirror->dir, mirror->mac);
	BSP_LEAVE_FUNC();
	return ret;
}


static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_MIRROR_CMD_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_MIRROR_CMD_DST_PORT, bsp_mirror_enable),
	HAL_CALLBACK_ENTRY(HAL_MIRROR_CMD_SRC_PORT, bsp_mirror_source_enable),
	HAL_CALLBACK_ENTRY(HAL_MIRROR_CMD_SRC_MAC, bsp_mirror_source_filter_enable),
};



int bsp_mirror_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_mirror_param_t	mirrorparam;
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
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
	hal_ipcmsg_getl(&client->ipcmsg, &mirrorparam.value);
	if(subcmd == HAL_MIRROR_CMD_SRC_PORT)
		hal_ipcmsg_getc(&client->ipcmsg, &mirrorparam.dir);
	if(subcmd == HAL_MIRROR_CMD_SRC_MAC)
	{
		hal_ipcmsg_getc(&client->ipcmsg, &mirrorparam.dir);
		hal_ipcmsg_getc(&client->ipcmsg, &mirrorparam.filter);
		hal_ipcmsg_get(&client->ipcmsg, &mirrorparam.mac, NSM_MAC_MAX);
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (callback->cmd_handle)(driver, &bspport, &mirrorparam);
	break;
	default:
		break;
	}
	BSP_LEAVE_FUNC();
	return ret;
}
