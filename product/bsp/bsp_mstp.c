/*
 * bsp_mstp.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_mstp.h"

sdk_mstp_t sdk_mstp;

static int bsp_mstp_enable(void *driver, hal_port_header_t *bspport, hal_mstp_param_t *param)
{
	if(driver && sdk_mstp.sdk_mstp_enable_cb)
		return sdk_mstp.sdk_mstp_enable_cb(driver, param->enable);
	return NO_SDK;
}

static int bsp_mstp_age(void *driver, hal_port_header_t *bspport, hal_mstp_param_t *param)
{
	if(driver && sdk_mstp.sdk_mstp_age_cb)
		return sdk_mstp.sdk_mstp_age_cb(driver, param->value);
	return NO_SDK;
}

static int bsp_mstp_bypass(void *driver, hal_port_header_t *bspport, hal_mstp_param_t *param)
{
	if(driver && sdk_mstp.sdk_mstp_bypass_cb)
		return sdk_mstp.sdk_mstp_bypass_cb(driver, param->enable, param->type);
	return NO_SDK;
}

static int bsp_mstp_state(void *driver, hal_port_header_t *bspport, hal_mstp_param_t *param)
{
	if(driver && sdk_mstp.sdk_mstp_state_cb)
		return sdk_mstp.sdk_mstp_state_cb(driver, bspport->phyport, param->value, param->state);
	return NO_SDK;
}

static int bsp_stp_state(void *driver, hal_port_header_t *bspport, hal_mstp_param_t *param)
{
	if(driver && sdk_mstp.sdk_stp_state_cb)
		return sdk_mstp.sdk_stp_state_cb(driver, bspport->phyport, param->state);
	return NO_SDK;
}



static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_MSTP, bsp_mstp_enable),
	HAL_CALLBACK_ENTRY(HAL_MSTP_AGE, bsp_mstp_age),
	HAL_CALLBACK_ENTRY(HAL_MSTP_BYPASS, bsp_mstp_bypass),
	HAL_CALLBACK_ENTRY(HAL_MSTP_STATE, bsp_mstp_state),
	HAL_CALLBACK_ENTRY(HAL_MSTP_VLAN, NULL),
	HAL_CALLBACK_ENTRY(HAL_STP_STATE, bsp_stp_state),
};

int bsp_mstp_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_mstp_param_t	param;
	hal_port_header_t	bspport;
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
	hal_ipcmsg_getl(&client->ipcmsg, &param.enable);
	hal_ipcmsg_getl(&client->ipcmsg, &param.value);
	hal_ipcmsg_getl(&client->ipcmsg, &param.type);
	hal_ipcmsg_getl(&client->ipcmsg, &param.state);

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