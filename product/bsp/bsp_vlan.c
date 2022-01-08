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
	if(driver && sdk_vlan.sdk_vlan_enable)
		return sdk_vlan.sdk_vlan_enable(driver, param->enable);
	return NO_SDK;
}

static int bsp_vlan_create(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	if(driver && sdk_vlan.sdk_vlan_create)
		return sdk_vlan.sdk_vlan_create(driver, param->set, param->vlan);
	return NO_SDK;
}

static int bsp_vlan_batch_create(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	if(driver && sdk_vlan.sdk_vlan_batch_create)
		return sdk_vlan.sdk_vlan_batch_create(driver, param->set, param->vlan, param->vlan_end);
	return NO_SDK;
}

static int bsp_vlan_untag_port(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	if(driver && sdk_vlan.sdk_vlan_untag_port)
		return sdk_vlan.sdk_vlan_untag_port(driver, param->set, port->phyport, param->vlan);
	return NO_SDK;
}


static int bsp_vlan_tag_port(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	if(driver && sdk_vlan.sdk_vlan_tag_port)
		return sdk_vlan.sdk_vlan_tag_port(driver, param->set, port->phyport, param->vlan);
	return NO_SDK;
}



static int bsp_port_native_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	if(driver && sdk_vlan.sdk_port_native_vlan)
		return sdk_vlan.sdk_port_native_vlan(driver, param->set, port->phyport, param->vlan);
	return NO_SDK;
}



static int bsp_port_allowed_tag_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	if(driver && sdk_vlan.sdk_port_allowed_tag_vlan)
		return sdk_vlan.sdk_port_allowed_tag_vlan(driver, param->set, port->phyport, param->vlan);
	return NO_SDK;
}


static int bsp_port_allowed_tag_batch_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	if(driver && sdk_vlan.sdk_port_allowed_tag_batch_vlan)
		return sdk_vlan.sdk_port_allowed_tag_batch_vlan(driver, param->set, port->phyport, param->vlan, param->vlan_end);
	return NO_SDK;
}


static int bsp_port_set_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	if(driver && sdk_vlan.sdk_port_pvid_vlan && param->set)
		return sdk_vlan.sdk_port_pvid_vlan(driver, param->set, port->phyport, param->vlan);
	return NO_SDK;
}



static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_VLAN, bsp_vlan_enable),
	HAL_CALLBACK_ENTRY(HAL_VLAN_CREATE, bsp_vlan_create),
	HAL_CALLBACK_ENTRY(HAL_VLAN_RANGE_CREATE, bsp_vlan_batch_create),

	HAL_CALLBACK_ENTRY(HAL_VLAN_UNTAG, bsp_vlan_untag_port),
	HAL_CALLBACK_ENTRY(HAL_VLAN_TAG, bsp_vlan_tag_port),
	HAL_CALLBACK_ENTRY(HAL_VLAN_NATIVE, bsp_port_native_vlan),
	HAL_CALLBACK_ENTRY(HAL_VLAN_ALLOWE, bsp_port_allowed_tag_vlan),
	HAL_CALLBACK_ENTRY(HAL_VLAN_RANGE_ALLOWE, bsp_port_allowed_tag_batch_vlan),
	HAL_CALLBACK_ENTRY(HAL_VLAN_PVID, bsp_port_set_vlan),
};

int bsp_vlan_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_vlan_param_t	param;
	//hal_global_header_t	global;
	hal_port_header_t	bspport;
	//hal_ipcmsg_global_get(&client->ipcmsg, &global);
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);

	hal_ipcmsg_getl(&client->ipcmsg, &param.set);
	hal_ipcmsg_getl(&client->ipcmsg, &param.enable);
	hal_ipcmsg_getw(&client->ipcmsg, &param.vlan);
	hal_ipcmsg_getw(&client->ipcmsg, &param.vlan_end);
	
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
