/*
 * bsp_mac.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zpl_type.h"
#include "log.h"
#include "hal_client.h"
#include "hal_driver.h"
#include "hal_mac.h"
#if defined(ZPL_SDK_USER) || defined(ZPL_SDK_NONE)
#include "bsp_driver.h"
#endif
#include "bsp_mac.h"


sdk_mac_t sdk_maccb;


static int bsp_mac_age(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_maccb.sdk_mac_age_cb)
		ret = sdk_maccb.sdk_mac_age_cb(bspdev->sdk_driver, macparam->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mac_add(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_maccb.sdk_mac_add_cb)
		ret = sdk_maccb.sdk_mac_add_cb(bspdev->sdk_driver, bspport->phyport, macparam->vlan, 
			bspport->vrfid, macparam->mac, macparam->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mac_del(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_maccb.sdk_mac_del_cb)
		ret = sdk_maccb.sdk_mac_del_cb(bspdev->sdk_driver, bspport->phyport, macparam->vlan, 
			bspport->vrfid, macparam->mac, macparam->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mac_clr(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_maccb.sdk_mac_clr_cb)
		ret = sdk_maccb.sdk_mac_clr_cb(bspdev->sdk_driver, bspport->phyport, macparam->vlan, 
			bspport->vrfid);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mac_read(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_maccb.sdk_mac_read_cb)
		ret = sdk_maccb.sdk_mac_read_cb(bspdev->sdk_driver, bspport->phyport, macparam->vlan, 
			bspport->vrfid, bspdev);	
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mac_dump(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_maccb.sdk_mac_dump_cb)
		ret = sdk_maccb.sdk_mac_dump_cb(bspdev->sdk_driver, bspport->phyport, macparam->vlan, 
			bspport->vrfid);	
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mac_clrall(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_maccb.sdk_mac_clrall_cb)
		ret = sdk_maccb.sdk_mac_clrall_cb(bspdev->sdk_driver);
	BSP_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_AGE, bsp_mac_age),
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_ADD, bsp_mac_add),
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_DEL, bsp_mac_del),
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_CLEAR, bsp_mac_clr),
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_CLEARALL, bsp_mac_clrall),
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_READ, bsp_mac_read),
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_DUMP, bsp_mac_dump),
};


int bsp_mac_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	BSP_DRIVER(bspdev, driver);
	hal_mac_param_t	mac_param;
	hal_port_header_t	bspport;
	struct hal_ipcmsg_result getvalue;
	int i = 0;
	hal_ipcsubcmd_callback_t * callback = NULL;
	BSP_ENTER_FUNC();
	memset(&mac_param, 0, sizeof(hal_mac_param_t));
	memset(&bspport, 0, sizeof(hal_port_header_t));
	memset(&getvalue, 0, sizeof(struct hal_ipcmsg_result));
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
	if(subcmd == HAL_MAC_CMD_CLEARALL)
	{

	}
	else if(subcmd != HAL_MAC_CMD_AGE)
		hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
	else
		hal_ipcmsg_getl(&client->ipcmsg, &mac_param.value);

	if(subcmd == HAL_MAC_CMD_ADD || subcmd == HAL_MAC_CMD_DEL)
	{
		hal_ipcmsg_getw(&client->ipcmsg, &mac_param.vlan);
		hal_ipcmsg_getl(&client->ipcmsg, &mac_param.value);
		hal_ipcmsg_get(&client->ipcmsg, mac_param.mac, NSM_MAC_MAX);
	}
	else if(subcmd == HAL_MAC_CMD_READ || subcmd == HAL_MAC_CMD_CLEAR || subcmd == HAL_MAC_CMD_DUMP)
	{
		hal_ipcmsg_getw(&client->ipcmsg, &mac_param.vlan);
		if(bspport.type == 0 && subcmd == HAL_MAC_CMD_CLEAR && mac_param.vlan == 0)
		{
			bspport.phyport = -1;
		}
	}

	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (callback->cmd_handle)(driver, &bspport, &mac_param);
	break;
	case HAL_MODULE_CMD_GET:         //设置
	{
		if(subcmd == HAL_MAC_CMD_DUMP)
		{
			hal_client_send_return(client, OK, "cmd:%s", hal_module_cmd_name(cmd));
			ret = (callback->cmd_handle)(driver, &bspport, &mac_param);
			BSP_LEAVE_FUNC();
			return ret;
		}
		bspdev->mac_cache_num = 0;
		memset(bspdev->mac_cache_entry, 0, sizeof(hal_mac_cache_t)*bspdev->mac_cache_max);
		ret = (callback->cmd_handle)(driver, &bspport, &mac_param);
		if(ret == 0)
		{
			getvalue.value = bspdev->mac_cache_num;
			hal_client_send_result_msg(client, ret, &getvalue, HAL_MAC_CMD_READ, bspdev->mac_cache_entry, 
				bspdev->mac_cache_num*sizeof(hal_mac_cache_t));	
		}	
	}
	break;
	default:
		break;
	}
	BSP_LEAVE_FUNC();
	return ret;
}


