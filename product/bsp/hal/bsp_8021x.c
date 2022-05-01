/*
 * bsp_8021x.c
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_driver.h"
#include "bsp_8021x.h"



sdk_8021x_t sdk_8021x_cb;


static int bsp_8021x_enable(void *driver, hal_port_header_t *bspport, hal_8021x_param_t *bsp8021x)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_8021x_cb.sdk_8021x_enable_cb)
		ret = sdk_8021x_cb.sdk_8021x_enable_cb(driver, bsp8021x->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_8021x_port_state(void *driver, hal_port_header_t *bspport, hal_8021x_param_t *bsp8021x)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_8021x_cb.sdk_8021x_port_state_cb)
		ret = sdk_8021x_cb.sdk_8021x_port_state_cb(driver, bspport->phyport, bsp8021x->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_8021x_port_mode(void *driver, hal_port_header_t *bspport, hal_8021x_param_t *bsp8021x)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_8021x_cb.sdk_8021x_port_mode_cb)
		ret = sdk_8021x_cb.sdk_8021x_port_mode_cb(driver, bspport->phyport, bsp8021x->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_8021x_auth_dmac(void *driver, hal_port_header_t *bspport, hal_8021x_param_t *bsp8021x)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_8021x_cb.sdk_8021x_auth_dmac_cb)
		ret = sdk_8021x_cb.sdk_8021x_auth_dmac_cb(driver, bspport->phyport, bsp8021x->mac);
	BSP_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_8021X_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_8021X, bsp_8021x_enable),
	HAL_CALLBACK_ENTRY(HAL_8021X_PORT_MODE, bsp_8021x_port_mode),
	HAL_CALLBACK_ENTRY(HAL_8021X_PORT_MAC, bsp_8021x_auth_dmac),
	HAL_CALLBACK_ENTRY(HAL_8021X_PORT_STATE, bsp_8021x_port_state),
};

int bsp_8021x_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_8021x_param_t	bsp8021x;
	hal_port_header_t	bspport;
	hal_ipcsubcmd_callback_t * callback = hal_ipcsubcmd_callback_get(subcmd_table, sizeof(subcmd_table)/sizeof(subcmd_table[0]), subcmd);
	BSP_ENTER_FUNC();
	if(!callback)
	{
		BSP_LEAVE_FUNC();
		return OS_NO_CALLBACK;
	}
	switch(subcmd)
	{
	case HAL_8021X:
	hal_ipcmsg_getl(&client->ipcmsg, &bsp8021x.value);
	break;
	case HAL_TRUNK_CMD_CREATE:
	case HAL_TRUNK_CMD_MODE:
	case HAL_TRUNK_CMD_ADDIF:
	case HAL_TRUNK_CMD_DELIF:
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
	hal_ipcmsg_getl(&client->ipcmsg, &bsp8021x.value);
	break;
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (callback->cmd_handle)(driver, &bspport, &bsp8021x);
	break;
	default:
		break;
	}
	
	BSP_LEAVE_FUNC();
	return ret;
}



