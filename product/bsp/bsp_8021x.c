/*
 * bsp_8021x.c
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */


#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"

#include "bsp_8021x.h"



sdk_8021x_t sdk_8021x_cb;


static int bsp_8021x_port_enable(void *driver, hal_port_header_t *bspport, hal_8021x_param_t *bsp8021x)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_8021x_cb.sdk_8021x_enable_cb)
		ret = sdk_8021x_cb.sdk_8021x_port_enable_cb(driver, bspport, bsp8021x->u.value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_8021x_port_state(void *driver, hal_port_header_t *bspport, hal_8021x_param_t *bsp8021x)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_8021x_cb.sdk_8021x_port_state_cb)
		ret = sdk_8021x_cb.sdk_8021x_port_state_cb(driver, bspport, bsp8021x->u.value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_8021x_port_mode(void *driver, hal_port_header_t *bspport, hal_8021x_param_t *bsp8021x)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_8021x_cb.sdk_8021x_port_mode_cb)
		ret = sdk_8021x_cb.sdk_8021x_port_mode_cb(driver, bspport, bsp8021x->u.value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_8021x_auth_bypass(void *driver, hal_port_header_t *bspport, hal_8021x_param_t *bsp8021x)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_8021x_cb.sdk_8021x_auth_bypass_cb)
		ret = sdk_8021x_cb.sdk_8021x_auth_bypass_cb(driver, bspport, bsp8021x->u.value);
	SDK_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_8021X_PORT, bsp_8021x_port_enable),
	HAL_CALLBACK_ENTRY(HAL_8021X_PORT_MAC, NULL),
	HAL_CALLBACK_ENTRY(HAL_8021X_PORT_STATE, bsp_8021x_port_state),
	HAL_CALLBACK_ENTRY(HAL_8021X_PORT_MODE, bsp_8021x_port_mode),
	HAL_CALLBACK_ENTRY(HAL_8021X_PORT_BYPASS, bsp_8021x_auth_bypass),
};

int bsp_8021x_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_8021x_param_t	bsp8021x;
	hal_port_header_t	bspport;
	SDK_ENTER_FUNC();
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
	hal_ipcmsg_getc(&client->ipcmsg, &bsp8021x.u.value);
	if(!(subcmd_table[subcmd].cmd_handle))
	{
		SDK_LEAVE_FUNC();
		return NO_SDK;
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &bsp8021x);
	break;
	default:
		break;
	}
	
	SDK_LEAVE_FUNC();
	return ret;
}



