/*
 * bsp_qinq.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"

#include "bsp_qinq.h"

sdk_qinq_t sdk_qinq;

static int bsp_qinq_enable(void *driver, hal_port_header_t *port, hal_qinq_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver  && sdk_qinq.sdk_qinq_enable_cb)
		ret = sdk_qinq.sdk_qinq_enable_cb(driver, param->value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_qinq_vlan_tpid(void *driver, hal_port_header_t *port, hal_qinq_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver  && sdk_qinq.sdk_qinq_vlan_ptid_cb)
		ret = sdk_qinq.sdk_qinq_vlan_ptid_cb(driver, param->value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_qinq_interface_enable(void *driver, hal_port_header_t *port, hal_qinq_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_qinq.sdk_qinq_port_enable_cb)
		ret = sdk_qinq.sdk_qinq_port_enable_cb(driver, port->phyport, param->value);
	SDK_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_QINQ_CMD_ENABLE, bsp_qinq_enable),
	HAL_CALLBACK_ENTRY(HAL_QINQ_CMD_TPID, bsp_qinq_vlan_tpid),
	HAL_CALLBACK_ENTRY(HAL_QINQ_CMD_IF_ENABLE, bsp_qinq_interface_enable),
};

int bsp_qinq_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_qinq_param_t	param;
	hal_port_header_t	bspport;
	SDK_ENTER_FUNC();
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);

	hal_ipcmsg_getl(&client->ipcmsg, &param.value);


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
