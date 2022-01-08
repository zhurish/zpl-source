/*
 * bsp_mac.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_mac.h"

sdk_mac_t sdk_maccb;

static int bsp_mac_age(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	if(driver && sdk_maccb.sdk_mac_age_cb)
		return sdk_maccb.sdk_mac_age_cb(driver , bspport, macparam);
	return NO_SDK;
}

static int bsp_mac_add(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	if(driver && sdk_maccb.sdk_mac_add_cb)
		return sdk_maccb.sdk_mac_add_cb(driver , bspport, macparam);
	return NO_SDK;
}

static int bsp_mac_del(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	if(driver && sdk_maccb.sdk_mac_del_cb)
		return sdk_maccb.sdk_mac_del_cb(driver , bspport, macparam);
	return NO_SDK;
}

static int bsp_mac_clr(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	if(driver && sdk_maccb.sdk_mac_clr_cb)
		return sdk_maccb.sdk_mac_clr_cb(driver , bspport, macparam);
	return NO_SDK;
}

static int bsp_mac_read(void *driver, hal_port_header_t *bspport, hal_mac_param_t *macparam)
{
	if(driver && sdk_maccb.sdk_mac_read_cb)
		return sdk_maccb.sdk_mac_read_cb(driver , bspport, macparam);
	return NO_SDK;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
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
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);
	//hal_ipcmsg_getc(&client->ipcmsg, &mac_param);
	if(!(subcmd_table[subcmd].cmd_handle))
	{
		return NO_SDK;
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_SET:         //设置
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &mac_param);
	break;
	case HAL_MODULE_CMD_GET:         //获取
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &mac_param);
	break;
	case HAL_MODULE_CMD_ADD:         //添加
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &mac_param);
	break;
	case HAL_MODULE_CMD_DEL:         //删除
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &mac_param);
	break;
    case HAL_MODULE_CMD_DELALL:      //删除所有
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &bspport, &mac_param);
	break;
	default:
		break;
	}
	return ret;
}


