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
	int ret = NO_SDK, i = 0;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_vlan.sdk_vlan_enable)
		ret = sdk_vlan.sdk_vlan_enable(bspdev->sdk_driver, param->enable);
	if(ret == OK)
	{
		if(bspdev->sdk_driver && sdk_vlan.sdk_vlan_create)
			ret = sdk_vlan.sdk_vlan_create(bspdev->sdk_driver, param->enable, 1);
		if(ret == OK)
		{
			for(i = 0; i < PHY_PORT_MAX; i++)
			{
				if(bspdev->phyports[i].phyport >= 0)
				{
					if(bspdev->sdk_driver && sdk_vlan.sdk_port_access_vlan)
						ret = sdk_vlan.sdk_port_access_vlan(bspdev->sdk_driver, param->enable, bspdev->phyports[i].phyport, 1);
					if(ret == OK)
					{
						if(sdk_vlan.sdk_port_allowed_untag_vlan)
							ret = sdk_vlan.sdk_port_allowed_untag_vlan(bspdev->sdk_driver, param->enable, bspdev->phyports[i].phyport, 1);
					}
					else
						break;
				}
			}
		}
	}
	BSP_LEAVE_FUNC();	
	return ret;
}

static int bsp_vlan_create(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_vlan.sdk_vlan_create)
		ret = sdk_vlan.sdk_vlan_create(bspdev->sdk_driver, param->enable, param->vlan);
	BSP_LEAVE_FUNC();	
	return ret;
}

static int bsp_vlan_batch_create(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	int i = 0;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_vlan.sdk_vlan_create)
	{
		for(i = param->vlan; i <= param->vlan_end; i++)
		{
			if(zpl_vlan_bitmap_tst(param->vlanbitmap, i))
			{
				ret |= sdk_vlan.sdk_vlan_create(bspdev->sdk_driver, param->enable, i);
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
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_vlan.sdk_port_access_vlan)
		ret = sdk_vlan.sdk_port_access_vlan(bspdev->sdk_driver, param->enable, port->phyport, param->vlan);
	if(ret == OK)
	{
		if(bspdev->phyports[port->phyport].mode == IF_MODE_ACCESS_L2)
		{
			bspdev->phyports[port->phyport].pvid = param->enable?param->vlan:1;
		}
	}	
	BSP_LEAVE_FUNC();	
	return ret;
}



static int bsp_port_native_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_vlan.sdk_port_native_vlan)
		ret = sdk_vlan.sdk_port_native_vlan(bspdev->sdk_driver, param->enable, port->phyport, param->vlan);
	if(ret == OK)
	{
		if(bspdev->phyports[port->phyport].mode == IF_MODE_TRUNK_L2)
		{
			bspdev->phyports[port->phyport].pvid = param->enable?param->vlan:1;
		}
	}
	BSP_LEAVE_FUNC();	
	return ret;
}



static int bsp_port_allowed_tag_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_vlan.sdk_port_allowed_tag_vlan)
		ret = sdk_vlan.sdk_port_allowed_tag_vlan(bspdev->sdk_driver, param->enable, port->phyport, param->vlan);
	if(ret == OK)
	{
		if(bspdev->phyports[port->phyport].mode == IF_MODE_TRUNK_L2)
		{
			if(param->enable)
				zpl_vlan_bitmap_set(bspdev->phyports[port->phyport].vlanbitmap,param->vlan);
			else
				zpl_vlan_bitmap_clr(bspdev->phyports[port->phyport].vlanbitmap,param->vlan);	
		}
	}
	BSP_LEAVE_FUNC();	
	return ret;
}


static int bsp_port_allowed_tag_batch_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	int i = 0;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_vlan.sdk_port_allowed_tag_vlan)
	{
		for(i = param->vlan; i <= param->vlan_end; i++)
		{
			if(zpl_vlan_bitmap_tst(param->vlanbitmap, i))
			{
				ret |= sdk_vlan.sdk_port_allowed_tag_vlan(bspdev->sdk_driver, param->enable, port->phyport, i);
				if(ret != OK)
					break;
			}
		}
	}
	if(ret == OK)
	{
		if(bspdev->phyports[port->phyport].mode == IF_MODE_TRUNK_L2)
		{
			if(param->enable)
				zpl_vlan_bitmap_or(bspdev->phyports[port->phyport].vlanbitmap, bspdev->phyports[port->phyport].vlanbitmap, param->vlanbitmap);
			else
				zpl_vlan_bitmap_xor(bspdev->phyports[port->phyport].vlanbitmap, bspdev->phyports[port->phyport].vlanbitmap, param->vlanbitmap);
		}
	}
	BSP_LEAVE_FUNC();	
	return ret;
}

static int bsp_port_set_vlan(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = 0, i = 0;
	zpl_phyport_t member = 0;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver)
	{


	for(i = 0; i < PHY_PORT_MAX; i++)
	{
		if(bspdev->phyports[i].phyport >= 0 && 
			bspdev->phyports[i].mode == IF_MODE_ACCESS_L2 && 
			bspdev->phyports[i].pvid == param->vlan && 
			bspdev->phyports[i].phyport != port->phyport)
		{
			member |= 1<<(bspdev->phyports[i].phyport);
			if(sdk_vlan.sdk_vlan_port_bridge_join)
				ret |= sdk_vlan.sdk_vlan_port_bridge_join(bspdev->sdk_driver, param->enable, bspdev->phyports[i].phyport, port->phyport);
		}
	}	
	if(sdk_vlan.sdk_vlan_port_bridge)
		ret |= sdk_vlan.sdk_vlan_port_bridge(bspdev->sdk_driver, param->enable, port->phyport, member);
	}
	BSP_LEAVE_FUNC();	
	return ret;
}



static int bsp_vlan_test(void *driver, hal_port_header_t *port, hal_vlan_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	//extern int sdk_b53_vlan_add_test(int port, int vid,int tag);
	if(bspdev->sdk_driver && sdk_vlan.sdk_vlan_create && port->lgport == 1)
		ret = sdk_vlan.sdk_vlan_create(bspdev->sdk_driver, 1, param->vlan);	


	if(bspdev->sdk_driver && sdk_vlan.sdk_port_allowed_tag_vlan && port->lgport == 2)
		ret = sdk_vlan.sdk_port_allowed_tag_vlan(bspdev->sdk_driver, 1, port->phyport, param->vlan);

	if(bspdev->sdk_driver && sdk_vlan.sdk_port_access_vlan && port->lgport == 3)
		ret = sdk_vlan.sdk_port_access_vlan(bspdev->sdk_driver, 1, port->phyport, param->vlan);


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
