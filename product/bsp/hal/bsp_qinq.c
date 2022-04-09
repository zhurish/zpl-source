/*
 * bsp_qinq.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"

#include "bsp_qinq.h"
#include "bsp_driver.h"

sdk_qinq_t sdk_qinq;

static int bsp_qinq_enable(void *driver, hal_port_header_t *port, hal_qinq_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver  && sdk_qinq.sdk_qinq_enable_cb)
		ret = sdk_qinq.sdk_qinq_enable_cb(driver, param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_qinq_vlan_tpid(void *driver, hal_port_header_t *port, hal_qinq_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver  && sdk_qinq.sdk_qinq_vlan_ptid_cb)
		ret = sdk_qinq.sdk_qinq_vlan_ptid_cb(driver, param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_qinq_interface_enable(void *driver, hal_port_header_t *port, hal_qinq_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_qinq.sdk_qinq_port_enable_cb)
		ret = sdk_qinq.sdk_qinq_port_enable_cb(driver, port->phyport, param->value);
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
	BSP_ENTER_FUNC();
	ret = bsp_driver_module_check(subcmd_table, sizeof(subcmd_table)/sizeof(subcmd_table[0]), subcmd);
	if(ret == 0)
	{
		BSP_LEAVE_FUNC();
		return NO_SDK;
	}
	if(HAL_QINQ_CMD_IF_ENABLE == subcmd)
		hal_ipcmsg_port_get(&client->ipcmsg, &bspport);

	hal_ipcmsg_getl(&client->ipcmsg, &param.value);

	if(!(subcmd_table[subcmd].cmd_handle))
	{
		BSP_LEAVE_FUNC();
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
	BSP_LEAVE_FUNC();
	return ret;
}
