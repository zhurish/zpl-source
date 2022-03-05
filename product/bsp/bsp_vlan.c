/*
 * bsp_vlan.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_vlan.h"

sdk_vlan_t sdk_vlan;

static int bsp_vlan_enable(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_vlan.sdk_vlan_enable)
		ret = sdk_vlan.sdk_vlan_enable(driver, param->enable);
	BSP_LEAVE_FUNC();	
	return ret;
}

static int bsp_vlan_create(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_vlan.sdk_vlan_create)
		ret = sdk_vlan.sdk_vlan_create(driver, param->enable, param->vlan);
	BSP_LEAVE_FUNC();	
	return ret;
}

static int bsp_vlan_batch_create(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	int i = 0;
	BSP_ENTER_FUNC();
	if(driver && sdk_vlan.sdk_vlan_create)
	{
		for(i = 0; i < param->num; i++)
		{
			ret |= sdk_vlan.sdk_vlan_create(driver, param->enable, param->vlantbl[i]);
			if(ret != OK)
				break;
		}
	}
	BSP_LEAVE_FUNC();	
	return ret;
}

static int bsp_vlan_untag_port(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_vlan.sdk_vlan_untag_port)
		ret = sdk_vlan.sdk_vlan_untag_port(driver, param->enable, port->phyport, param->vlan);
	BSP_LEAVE_FUNC();	
	return ret;
}


static int bsp_vlan_tag_port(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_vlan.sdk_vlan_tag_port)
		ret = sdk_vlan.sdk_vlan_tag_port(driver, param->enable, port->phyport, param->vlan);
	BSP_LEAVE_FUNC();	
	return ret;
}



static int bsp_port_native_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_vlan.sdk_port_native_vlan)
		ret = sdk_vlan.sdk_port_native_vlan(driver, param->enable, port->phyport, param->vlan);
	BSP_LEAVE_FUNC();	
	return ret;
}



static int bsp_port_allowed_tag_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_vlan.sdk_port_allowed_tag_vlan)
		ret = sdk_vlan.sdk_port_allowed_tag_vlan(driver, param->enable, port->phyport, param->vlan);
	BSP_LEAVE_FUNC();	
	return ret;
}


static int bsp_port_allowed_tag_batch_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	int i = 0;
	BSP_ENTER_FUNC();
	if(driver && sdk_vlan.sdk_port_allowed_tag_vlan)
	{
		for(i = 0; i < param->num; i++)
		{
			ret |= sdk_vlan.sdk_port_allowed_tag_vlan(driver, param->enable, port->phyport, param->vlantbl[i]);
			if(ret != OK)
				break;
		}
	}
	BSP_LEAVE_FUNC();	
	return ret;
}


static int bsp_port_set_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_vlan.sdk_port_pvid_vlan)
		ret = sdk_vlan.sdk_port_pvid_vlan(driver, param->enable, port->phyport, param->vlan);
	BSP_LEAVE_FUNC();	
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_VLAN_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_VLAN, bsp_vlan_enable),
	HAL_CALLBACK_ENTRY(HAL_VLAN_CREATE, bsp_vlan_create),
	HAL_CALLBACK_ENTRY(HAL_VLAN_RANGE_CREATE, bsp_vlan_batch_create),

	HAL_CALLBACK_ENTRY(HAL_VLAN_UNTAG, bsp_vlan_untag_port),
	HAL_CALLBACK_ENTRY(HAL_VLAN_TAG, bsp_vlan_tag_port),
	HAL_CALLBACK_ENTRY(HAL_VLAN_NATIVE, bsp_port_native_vlan),
	HAL_CALLBACK_ENTRY(HAL_VLAN_ALLOWE, bsp_port_allowed_tag_vlan),
	HAL_CALLBACK_ENTRY(HAL_VLAN_RANGE_ALLOWE, bsp_port_allowed_tag_batch_vlan),
	HAL_CALLBACK_ENTRY(HAL_VLAN_PORT_BASE, bsp_port_set_vlan),
};

int bsp_vlan_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK, i = 0;
	vlan_t vlantbl[4096];
	hal_vlan_param_t	param;
	hal_port_header_t	bspport;
	BSP_ENTER_FUNC();
	ret = bsp_driver_module_check(subcmd_table, sizeof(subcmd_table)/sizeof(subcmd_table[0]), subcmd);
	if(ret == 0)
	{
		BSP_LEAVE_FUNC();
		return NO_SDK;
	}
	param.vlantbl = vlantbl;
	switch(subcmd)
	{
	case HAL_VLAN:
	case HAL_VLAN_CREATE:
	case HAL_VLAN_DELETE:
		hal_ipcmsg_getl(&client->ipcmsg, &param.enable);
		hal_ipcmsg_getw(&client->ipcmsg, &param.vlan);
		hal_ipcmsg_getw(&client->ipcmsg, &param.vlan_end);
	break;
	case HAL_VLAN_RANGE_CREATE:
    case HAL_VLAN_RANGE_DELETE:
	hal_ipcmsg_getl(&client->ipcmsg, &param.num);
	for(i = 0; i < param.num; i++)
		hal_ipcmsg_getw(&client->ipcmsg, &vlantbl[i]);
	break;	
    //PORT
    case HAL_VLAN_UNTAG:
    case HAL_VLAN_TAG:
    case HAL_VLAN_NATIVE:
    case HAL_VLAN_ALLOWE:
	case HAL_VLAN_PORT_BASE:
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
	hal_ipcmsg_getl(&client->ipcmsg, &param.enable);
	hal_ipcmsg_getw(&client->ipcmsg, &param.vlan);
	hal_ipcmsg_getw(&client->ipcmsg, &param.vlan_end);
	break;
    case HAL_VLAN_RANGE_ALLOWE:
	hal_ipcmsg_getl(&client->ipcmsg, &param.enable);
	hal_ipcmsg_getl(&client->ipcmsg, &param.num);
	for(i = 0; i < param.num; i++)
		hal_ipcmsg_getw(&client->ipcmsg, &vlantbl[i]);
	break;
    
	}

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
