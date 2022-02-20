/*
 * bsp_mirror.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */



#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_mirror.h"

sdk_mirror_t sdk_mirror;

static int bsp_mirror_enable(void *driver, hal_port_header_t *bspport, hal_mirror_param_t *mirror)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_mirror.sdk_mirror_enable_cb)
		ret = sdk_mirror.sdk_mirror_enable_cb(driver, bspport->phyport, mirror->value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_mirror_source_enable(void *driver, hal_port_header_t *bspport, hal_mirror_param_t *mirror)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_mirror.sdk_mirror_source_enable_cb)
		ret = sdk_mirror.sdk_mirror_source_enable_cb(driver, bspport->phyport, mirror->value, mirror->mode, mirror->dir);
	SDK_LEAVE_FUNC();
	return ret;
}


static int bsp_mirror_source_filter_enable(void *driver, hal_port_header_t *bspport, hal_mirror_param_t *mirror)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_mirror.sdk_mirror_source_filter_enable_cb)
		ret = sdk_mirror.sdk_mirror_source_filter_enable_cb(driver, mirror->value, mirror->filter, mirror->dir, mirror->srcmac, mirror->dstmac);
	SDK_LEAVE_FUNC();
	return ret;
}


static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_MIRROR_CMD_DST_PORT, bsp_mirror_enable),
	HAL_CALLBACK_ENTRY(HAL_MIRROR_CMD_SRC_PORT, bsp_mirror_source_enable),
	HAL_CALLBACK_ENTRY(HAL_MIRROR_CMD_SRC_MAC, bsp_mirror_source_filter_enable),
	HAL_CALLBACK_ENTRY(HAL_MIRROR_CMD_DST_MAC, bsp_mirror_source_filter_enable),
};



int bsp_mirror_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_mirror_param_t	mirrorparam;
	hal_port_header_t	bspport;
	SDK_ENTER_FUNC();
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
	//hal_ipcmsg_getc(&client->ipcmsg, &mirrorparam);
	if(!(subcmd_table[subcmd].cmd_handle))
	{
		SDK_LEAVE_FUNC();
		return NO_SDK;
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &mirrorparam);
	break;
	default:
		break;
	}
	SDK_LEAVE_FUNC();
	return ret;
}
