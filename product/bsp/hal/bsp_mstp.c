/*
 * bsp_mstp.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "bsp_types.h"
#include "hal_client.h"
#include "bsp_mstp.h"



sdk_mstp_t sdk_mstp;

static int bsp_mstp_enable(void *driver, hal_port_header_t *bspport, hal_mstp_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_mstp.sdk_mstp_enable_cb)
		ret = sdk_mstp.sdk_mstp_enable_cb(bspdev->sdk_driver, param->enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mstp_create(void *driver, hal_port_header_t *bspport, hal_mstp_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_mstp.sdk_mstp_create)
		ret = sdk_mstp.sdk_mstp_create(bspdev->sdk_driver, param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mstp_add_vlan(void *driver, hal_port_header_t *bspport, hal_mstp_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_mstp.sdk_mstp_add_vlan)
		ret = sdk_mstp.sdk_mstp_add_vlan(bspdev->sdk_driver, param->value, param->type);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mstp_del_vlan(void *driver, hal_port_header_t *bspport, hal_mstp_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_mstp.sdk_mstp_del_vlan)
		ret = sdk_mstp.sdk_mstp_del_vlan(bspdev->sdk_driver, param->value, param->type);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mstp_state(void *driver, hal_port_header_t *bspport, hal_mstp_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_mstp.sdk_mstp_state)
		ret = sdk_mstp.sdk_mstp_state(bspdev->sdk_driver, param->value, bspport->phyport, param->state);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_stp_state(void *driver, hal_port_header_t *bspport, hal_mstp_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_mstp.sdk_stp_state_cb)
		ret = sdk_mstp.sdk_stp_state_cb(bspdev->sdk_driver, param->value, bspport->phyport, param->state);
	BSP_LEAVE_FUNC();
	return ret;
}


static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_MSTP_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_MSTP_ENABLE, bsp_mstp_enable),
	HAL_CALLBACK_ENTRY(HAL_MSTP_CREATE, bsp_mstp_create),
	HAL_CALLBACK_ENTRY(HAL_MSTP_ADD_VLAN, bsp_mstp_add_vlan),
	HAL_CALLBACK_ENTRY(HAL_MSTP_DEL_VLAN, bsp_mstp_del_vlan),
	HAL_CALLBACK_ENTRY(HAL_MSTP_STATE, bsp_mstp_state),
	HAL_CALLBACK_ENTRY(HAL_STP_STATE, bsp_stp_state),
};


int bsp_mstp_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_mstp_param_t	param;
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
	hal_ipcmsg_getl(&client->ipcmsg, &param.enable);
	hal_ipcmsg_getl(&client->ipcmsg, &param.value);
	hal_ipcmsg_getl(&client->ipcmsg, &param.type);
	hal_ipcmsg_getl(&client->ipcmsg, &param.state);

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