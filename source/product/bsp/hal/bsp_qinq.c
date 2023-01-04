/*
 * bsp_qinq.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "bsp_types.h"
#include "hal_client.h"

#include "bsp_qinq.h"


sdk_qinq_t sdk_qinq;

static int bsp_qinq_enable(void *driver, hal_port_header_t *port, hal_qinq_param_t *param)
{
	int ret = OK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver  && sdk_qinq.sdk_qinq_enable_cb)
		ret = sdk_qinq.sdk_qinq_enable_cb(bspdev->sdk_driver, param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_qinq_vlan_tpid(void *driver, hal_port_header_t *port, hal_qinq_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver  && sdk_qinq.sdk_qinq_vlan_ptid_cb)
		ret = sdk_qinq.sdk_qinq_vlan_ptid_cb(bspdev->sdk_driver, param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_qinq_interface_enable(void *driver, hal_port_header_t *port, hal_qinq_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_qinq.sdk_qinq_port_enable_cb)
		ret = sdk_qinq.sdk_qinq_port_enable_cb(bspdev->sdk_driver, port->phyport, param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_QINQ_CMD_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_QINQ_CMD_ENABLE, bsp_qinq_enable),
	HAL_CALLBACK_ENTRY(HAL_QINQ_CMD_TPID, bsp_qinq_vlan_tpid),
	HAL_CALLBACK_ENTRY(HAL_QINQ_CMD_IF_ENABLE, bsp_qinq_interface_enable),
};

int bsp_qinq_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_qinq_param_t	param;
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
	if(HAL_QINQ_CMD_IF_ENABLE == subcmd)
		hal_ipcmsg_port_get(&client->ipcmsg, &bspport);

	hal_ipcmsg_getl(&client->ipcmsg, &param.value);

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