/*
 * bsp_vlan.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "bsp_types.h"
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
		for(i = param->vlan; i <= param->vlan_end; i++)
		{
			if(zpl_vlan_bitmap_tst(param->vlanbitmap, i))
			{
				ret |= sdk_vlan.sdk_vlan_create(driver, param->enable, i);
				if(ret != OK)
					break;
			}
		}
	}
	BSP_LEAVE_FUNC();	
	return ret;
}




static int bsp_port_access_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	printk("%s:op=%d phyport=%x vlan=%d", __func__, param->enable, port->phyport, param->vlan);
	if(driver && sdk_vlan.sdk_port_access_vlan)
		ret = sdk_vlan.sdk_port_access_vlan(driver, param->enable, port->phyport, param->vlan);
	BSP_LEAVE_FUNC();	
	return ret;
}



static int bsp_port_native_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	printk("%s:op=%d phyport=%x vlan=%d", __func__, param->enable, port->phyport, param->vlan);
	if(driver && sdk_vlan.sdk_port_native_vlan)
		ret = sdk_vlan.sdk_port_native_vlan(driver, param->enable, port->phyport, param->vlan);
	BSP_LEAVE_FUNC();	
	return ret;
}



static int bsp_port_allowed_tag_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	printk("%s:op=%d phyport=%x vlan=%d", __func__, param->enable, port->phyport, param->vlan);
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
		for(i = param->vlan; i <= param->vlan_end; i++)
		{
			if(zpl_vlan_bitmap_tst(param->vlanbitmap, i))
			{
				ret |= sdk_vlan.sdk_port_allowed_tag_vlan(driver, param->enable, port->phyport, i);
				if(ret != OK)
					break;
			}
		}
	}
	BSP_LEAVE_FUNC();	
	return ret;
}


static int bsp_port_set_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	printk("%s:op=%d phyport=%x vlan=%d", __func__, param->enable, port->phyport, param->vlan);
	if(driver && sdk_vlan.sdk_port_pvid_vlan)
		ret = sdk_vlan.sdk_port_pvid_vlan(driver, param->enable, port->phyport, param->vlan);
	BSP_LEAVE_FUNC();	
	return ret;
}

static int bsp_vlan_test(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	//extern int sdk_b53_vlan_add_test(int port, int vid,int tag);
	if(driver && sdk_vlan.sdk_vlan_create && port->lgport == 1)
		ret = sdk_vlan.sdk_vlan_create(driver, 1, param->vlan);	


	if(driver && sdk_vlan.sdk_port_allowed_tag_vlan && port->lgport == 2)
		ret = sdk_vlan.sdk_port_allowed_tag_vlan(driver, 1, port->phyport, param->vlan);

	if(driver && sdk_vlan.sdk_port_access_vlan && port->lgport == 3)
		ret = sdk_vlan.sdk_port_access_vlan(driver, 1, port->phyport, param->vlan);


	BSP_LEAVE_FUNC();	
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_VLAN_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_VLAN, bsp_vlan_enable),
	HAL_CALLBACK_ENTRY(HAL_VLAN_CREATE, bsp_vlan_create),
	HAL_CALLBACK_ENTRY(HAL_VLAN_DELETE, bsp_vlan_create),
	HAL_CALLBACK_ENTRY(HAL_VLAN_RANGE_CREATE, bsp_vlan_batch_create),
	HAL_CALLBACK_ENTRY(HAL_VLAN_RANGE_DELETE, bsp_vlan_batch_create),
	HAL_CALLBACK_ENTRY(HAL_VLAN_ACCESS, bsp_port_access_vlan),
	HAL_CALLBACK_ENTRY(HAL_VLAN_NATIVE, bsp_port_native_vlan),
	HAL_CALLBACK_ENTRY(HAL_VLAN_ALLOWE, bsp_port_allowed_tag_vlan),
	HAL_CALLBACK_ENTRY(HAL_VLAN_RANGE_ALLOWE, bsp_port_allowed_tag_batch_vlan),
	HAL_CALLBACK_ENTRY(HAL_VLAN_PORT_BASE, bsp_port_set_vlan),
	HAL_CALLBACK_ENTRY(HAL_VLAN_TEST, bsp_vlan_test),
};


int bsp_vlan_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK, i = 0;

	hal_vlan_param_t	param;
	hal_port_header_t	bspport;

	hal_ipcsubcmd_callback_t * callback = NULL;
	BSP_ENTER_FUNC();	
	memset(&param, 0, sizeof(hal_vlan_param_t));
	for(i = 0; i < ZPL_ARRAY_SIZE(subcmd_table); i++)
	{
        //zlog_warn(MODULE_HAL, "=== this subcmd:%d %d", subcmd_table[i].subcmd, subcmd);
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
	//param.vlantbl = vlantbl;
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
	{
		if(HAL_VLAN_RANGE_CREATE == subcmd)
			param.enable = zpl_true;
		else
			param.enable = zpl_false;	

		hal_ipcmsg_getw(&client->ipcmsg, &param.vlan);
		hal_ipcmsg_getw(&client->ipcmsg, &param.vlan_end);
		hal_ipcmsg_get(&client->ipcmsg, param.vlanbitmap.bitmap, sizeof(zpl_vlan_bitmap_t));
		//for(i = 0; i < param.num; i++)
		//	hal_ipcmsg_getw(&client->ipcmsg, &vlantbl[i]);
	}
	break;	
    //PORT
    case HAL_VLAN_ACCESS:
    case HAL_VLAN_NATIVE:
    case HAL_VLAN_ALLOWE:
	case HAL_VLAN_PORT_BASE:
		hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
		hal_ipcmsg_getl(&client->ipcmsg, &param.enable);
		hal_ipcmsg_getw(&client->ipcmsg, &param.vlan);
		hal_ipcmsg_getw(&client->ipcmsg, &param.vlan_end);
	break;
    case HAL_VLAN_RANGE_ALLOWE:
		hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
		hal_ipcmsg_getl(&client->ipcmsg, &param.enable);
		hal_ipcmsg_getw(&client->ipcmsg, &param.vlan);
		hal_ipcmsg_getw(&client->ipcmsg, &param.vlan_end);
		hal_ipcmsg_get(&client->ipcmsg, param.vlanbitmap.bitmap, sizeof(zpl_vlan_bitmap_t));
		//for(i = 0; i < param.num; i++)
		//	hal_ipcmsg_getw(&client->ipcmsg, &vlantbl[i]);
	break;
    case HAL_VLAN_TEST:
	{
		hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
		hal_ipcmsg_getl(&client->ipcmsg, &bspport.lgport);
		hal_ipcmsg_getw(&client->ipcmsg, &param.vlan);
		ret = (callback->cmd_handle)(driver, &bspport, &param);
		return ret;
	}
	break;
	
	default:
	break;
	}

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
