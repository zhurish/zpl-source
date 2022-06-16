/*
 * bsp_mac.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "bsp_types.h"

#include "bsp_mac.h"


sdk_mac_t sdk_maccb;


static int bsp_mac_age(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_maccb.sdk_mac_age_cb)
		ret = sdk_maccb.sdk_mac_age_cb(driver, macparam->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mac_add(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_maccb.sdk_mac_add_cb)
		ret = sdk_maccb.sdk_mac_add_cb(driver, bspport->phyport, macparam->vlan, 
			bspport->vrfid, macparam->mac, macparam->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mac_del(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_maccb.sdk_mac_del_cb)
		ret = sdk_maccb.sdk_mac_del_cb(driver, bspport->phyport, macparam->vlan, 
			bspport->vrfid, macparam->mac, macparam->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mac_clr(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_maccb.sdk_mac_clr_cb)
		ret = sdk_maccb.sdk_mac_clr_cb(driver, bspport->phyport, macparam->vlan, 
			bspport->vrfid);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mac_read(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_maccb.sdk_mac_read_cb)
		ret = sdk_maccb.sdk_mac_read_cb(driver, bspport->phyport, macparam->vlan, 
			bspport->vrfid, &macparam->table);	
	BSP_LEAVE_FUNC();
	return ret;
}


static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_AGE, bsp_mac_age),
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_ADD, bsp_mac_add),
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_DEL, bsp_mac_del),
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_CLEAR, bsp_mac_clr),
	HAL_CALLBACK_ENTRY(HAL_MAC_CMD_READ, bsp_mac_read),
};


int bsp_mac_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
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
	if(subcmd != HAL_MAC_CMD_AGE)
		hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
	else
		hal_ipcmsg_getl(&client->ipcmsg, &mac_param.value);

	if(subcmd == HAL_MAC_CMD_ADD || subcmd == HAL_MAC_CMD_DEL)
	{
		hal_ipcmsg_getw(&client->ipcmsg, &mac_param.vlan);
		hal_ipcmsg_getl(&client->ipcmsg, &mac_param.value);
		hal_ipcmsg_get(&client->ipcmsg, mac_param.mac, NSM_MAC_MAX);
	}
	else if(subcmd == HAL_MAC_CMD_READ || subcmd == HAL_MAC_CMD_CLEAR)
	{
		hal_ipcmsg_getw(&client->ipcmsg, &mac_param.vlan);
	}

	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (callback->cmd_handle)(driver, &bspport, &mac_param);
	break;
	case HAL_MODULE_CMD_GET:         //设置
	{
		ret = (callback->cmd_handle)(driver, &bspport, &mac_param);
		//if(ret == 0)
		{
			getvalue.value = mac_param.table.macnum;
			hal_client_send_result_msg(client, ret, &getvalue, HAL_MAC_CMD_READ, mac_param.table.mactbl, 
				mac_param.table.macnum*sizeof(hal_mac_tbl_t));
			if(mac_param.table.mactbl)
				XFREE(MTYPE_SDK_DATA, mac_param.table.mactbl);	
		}	
	}
	break;
	default:
		break;
	}
	BSP_LEAVE_FUNC();
	return ret;
}


