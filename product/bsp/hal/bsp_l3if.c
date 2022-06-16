/*
 * bsp_l3if.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "bsp_types.h"

#include "bsp_l3if.h"



sdk_l3if_t sdk_l3if;


static int bsp_l3if_create(void *driver, hal_port_header_t *port, hal_l3if_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_l3if.sdk_l3if_addif_cb)
		ret = sdk_l3if.sdk_l3if_addif_cb(driver, param->ifname, port->phyport);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3if_delete(void *driver, hal_port_header_t *port, hal_l3if_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_l3if.sdk_l3if_delif_cb)
		ret = sdk_l3if.sdk_l3if_delif_cb(driver, port->phyport);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3if_vrf(void *driver, hal_port_header_t *port, hal_l3if_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_l3if.sdk_l3if_vrf_cb)
		ret = sdk_l3if.sdk_l3if_vrf_cb(driver, port->phyport,  param->vrfid);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3if_mac(void *driver, hal_port_header_t *port, hal_l3if_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_l3if.sdk_l3if_mac_cb)
		ret = sdk_l3if.sdk_l3if_mac_cb(driver, port->phyport, param->mac);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3if_add_addr(void *driver, hal_port_header_t *port, hal_l3if_addr_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_l3if.sdk_l3if_add_addr_cb)
		ret = sdk_l3if.sdk_l3if_add_addr_cb(driver, port->phyport,  param->family, param->prefixlen, param->address, param->sec);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3if_del_addr(void *driver, hal_port_header_t *port, hal_l3if_addr_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_l3if.sdk_l3if_del_addr_cb)
		ret = sdk_l3if.sdk_l3if_del_addr_cb(driver, port->phyport,  param->family, param->prefixlen, param->address, param->sec);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3if_add_dstaddr(void *driver, hal_port_header_t *port, hal_l3if_addr_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_l3if.sdk_l3if_add_dstaddr_cb)
		ret = sdk_l3if.sdk_l3if_add_dstaddr_cb(driver, port->phyport,  param->family, param->prefixlen, param->address, param->sec);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3if_del_dstaddr(void *driver, hal_port_header_t *port, hal_l3if_addr_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_l3if.sdk_l3if_del_dstaddr_cb)
		ret = sdk_l3if.sdk_l3if_del_dstaddr_cb(driver, port->phyport,  param->family, param->prefixlen, param->address, param->sec);
	BSP_LEAVE_FUNC();
	return ret;
}
static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_L3IF_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_L3IF_CREATE, bsp_l3if_create),
	HAL_CALLBACK_ENTRY(HAL_L3IF_DELETE, bsp_l3if_delete),
	HAL_CALLBACK_ENTRY(HAL_L3IF_ADDR_ADD, bsp_l3if_add_addr),
	HAL_CALLBACK_ENTRY(HAL_L3IF_ADDR_DEL, bsp_l3if_del_addr),
	HAL_CALLBACK_ENTRY(HAL_L3IF_DSTADDR_ADD, bsp_l3if_add_dstaddr),
	HAL_CALLBACK_ENTRY(HAL_L3IF_DSTADDR_DEL, bsp_l3if_del_dstaddr),
	HAL_CALLBACK_ENTRY(HAL_L3IF_VRF, bsp_l3if_vrf),
	HAL_CALLBACK_ENTRY(HAL_L3IF_MAC, bsp_l3if_mac),
};


int bsp_l3if_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_l3if_param_t param;
	hal_l3if_addr_param_t addr_param;
	int i = 0;
	hal_ipcsubcmd_callback_t * callback = NULL;
	BSP_ENTER_FUNC();	
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
	switch (subcmd)
	{
	case HAL_L3IF_CREATE:
		hal_ipcmsg_port_get(&client->ipcmsg, &param.port);
		hal_ipcmsg_get(&client->ipcmsg, &param.ifname, 6);
		hal_ipcmsg_get(&client->ipcmsg, &param.mac, 6);
		ret = (callback->cmd_handle)(driver, &param.port, &param);
		break;
	case HAL_L3IF_DELETE:
		hal_ipcmsg_port_get(&client->ipcmsg, &param.port);
		ret = (callback->cmd_handle)(driver, &param.port, &param);
		break;
	case HAL_L3IF_ADDR_ADD:
	case HAL_L3IF_ADDR_DEL:
	case HAL_L3IF_DSTADDR_ADD:
	case HAL_L3IF_DSTADDR_DEL:
		hal_ipcmsg_port_get(&client->ipcmsg, &addr_param.port);
		hal_ipcmsg_getc(&client->ipcmsg, &addr_param.family);
		hal_ipcmsg_getc(&client->ipcmsg, &addr_param.prefixlen);
		if (addr_param.family == IPSTACK_AF_INET)
			hal_ipcmsg_getl(&client->ipcmsg, &addr_param.address.ipv4.s_addr);
#ifdef ZPL_BUILD_IPV6
		else
			hal_ipcmsg_get(&client->ipcmsg, (zpl_uchar *)&addr_param.address.ipv6, sizeof(addr_param.address.ipv6));
#endif
		if(subcmd == HAL_L3IF_ADDR_DEL || subcmd == HAL_L3IF_ADDR_ADD)
			hal_ipcmsg_getc(&client->ipcmsg, &addr_param.sec);
		ret = (callback->cmd_handle)(driver, &addr_param.port, &addr_param);
		break;
	case HAL_L3IF_VRF:
		hal_ipcmsg_port_get(&client->ipcmsg, &param.port);
		hal_ipcmsg_getw(&client->ipcmsg, &param.vrfid);
		ret = (callback->cmd_handle)(driver, &param.port, &param);
		break;
	case HAL_L3IF_MAC:
		hal_ipcmsg_port_get(&client->ipcmsg, &param.port);
		hal_ipcmsg_get(&client->ipcmsg, &param.mac, 6);
		ret = (callback->cmd_handle)(driver, &param.port, &param);
		break;
	}

	BSP_LEAVE_FUNC();
	return ret;
}
