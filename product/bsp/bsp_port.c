/*
 * bsp_port.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_port.h"

sdk_port_t sdk_port;

static int bsp_port_up(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_link_cb)
		ret = sdk_port.sdk_port_link_cb(driver, port->phyport, param->enable);
	SDK_LEAVE_FUNC();
	return ret;
}



/*static int bsp_port_address_set(ifindex_t port->phyport, struct prefix *cp, static int secondry)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_set_address_cb)
		ret = sdk_port.sdk_port_set_address_cb(driver, port->phyport, cp, secondry);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_port_address_unset(ifindex_t port->phyport, struct prefix *cp, static int secondry)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_unset_address_cb)
		ret = sdk_port.sdk_port_unset_address_cb(driver, port->phyport, cp, secondry);
	SDK_LEAVE_FUNC();
	return ret;
}*/

static int bsp_port_mac_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_mac_cb)
		ret = sdk_port.sdk_port_mac_cb(driver, port->phyport, param->mac, param->enable);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_port_mtu_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_mtu_cb)
		ret = sdk_port.sdk_port_mtu_cb(driver, port->phyport,  param->value);
	SDK_LEAVE_FUNC();
	return ret;
}

/*static int bsp_port_metric_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_metric_cb)
		ret = sdk_port.sdk_port_metric_cb(driver, port->phyport,  value);
	SDK_LEAVE_FUNC();
	return ret;
}*/

static int bsp_port_vrf_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_vrf_cb)
		ret = sdk_port.sdk_port_vrf_cb(driver, port->phyport,  param->value);
	SDK_LEAVE_FUNC();
	return ret;
}

/*static int bsp_port_multicast_set(ifindex_t port->phyport, static int value)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_multicast_cb)
		ret = sdk_port.sdk_port_multicast_cb(driver, port->phyport,  param->value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_port_bandwidth_set(ifindex_t port->phyport, static int value)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_bandwidth_cb)
		ret = sdk_port.sdk_port_bandwidth_cb(driver, port->phyport,  param->value);
	SDK_LEAVE_FUNC();
	return ret;
}*/

static int bsp_port_speed_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_speed_cb)
		ret = sdk_port.sdk_port_speed_cb(driver, port->phyport,  param->value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_port_duplex_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_duplex_cb)
		ret = sdk_port.sdk_port_duplex_cb(driver, port->phyport,  param->value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_port_mode_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_mode_cb)
		ret = sdk_port.sdk_port_mode_cb(driver, port->phyport,  param->value);
	SDK_LEAVE_FUNC();
	return ret;
}

/*
static int bsp_port_linkdetect_set(ifindex_t port->phyport, static int value)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_linkdetect_cb)
		ret = sdk_port.sdk_port_linkdetect_cb(driver, port->phyport,  param->value);
	SDK_LEAVE_FUNC();
	return ret;
}
*/


static int bsp_port_loop_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_loop_cb)
		ret = sdk_port.sdk_port_loop_cb(driver, port->phyport,  param->value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_port_8021x_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_8021x_cb)
		ret = sdk_port.sdk_port_8021x_cb(driver, port->phyport,  param->value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_port_jumbo_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_jumbo_cb)
		ret = sdk_port.sdk_port_jumbo_cb(driver, port->phyport,  param->enable);
	SDK_LEAVE_FUNC();
	return ret;
	//return bsp_jumbo_interface_enable(port->phyport, param->enable);
}

static int bsp_port_enable_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_enable_cb)
		ret = sdk_port.sdk_port_enable_cb(driver, port->phyport,  param->enable);
	SDK_LEAVE_FUNC();
	return ret;
}

/*
static int bsp_port_flow_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_flow_cb)
		ret = sdk_port.sdk_port_flow_cb(driver, port->phyport,  param->value);
	SDK_LEAVE_FUNC();
	return ret;
}
*/

static int bsp_port_learning_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_learning_enable_cb)
		ret = sdk_port.sdk_port_learning_enable_cb(driver, port->phyport,  param->enable);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_port_software_learning_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_swlearning_enable_cb)
		ret = sdk_port.sdk_port_swlearning_enable_cb(driver, port->phyport,  param->enable);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_port_protected_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_protected_enable_cb)
		ret = sdk_port.sdk_port_protected_enable_cb(driver, port->phyport,  param->enable);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_port_wan_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_wan_enable_cb)
		ret = sdk_port.sdk_port_wan_enable_cb(driver, port->phyport,  param->enable);
	SDK_LEAVE_FUNC();
	return ret;
}


static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_PORT_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_PORT, bsp_port_enable_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_LINK, bsp_port_up),
	HAL_CALLBACK_ENTRY(HAL_PORT_SPEED, bsp_port_speed_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_DUPLEX, bsp_port_duplex_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_FLOW, bsp_port_mode_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_JUMBO, bsp_port_jumbo_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_LOOP, bsp_port_loop_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_8021X, bsp_port_8021x_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_LEARNING, bsp_port_learning_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_SWLEARNING, bsp_port_software_learning_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_PROTECTED, bsp_port_protected_set),

	HAL_CALLBACK_ENTRY(HAL_PORT_WAN, bsp_port_wan_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_MAC, bsp_port_mac_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_MTU, bsp_port_mtu_set),

	HAL_CALLBACK_ENTRY(HAL_PORT_METRIC, bsp_port_enable_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_VRF, bsp_port_vrf_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_MODE, bsp_port_mode_set),
	//HAL_CALLBACK_ENTRY(HAL_PORT_IPADDR, bsp_qinq_interface_enable),
};


int bsp_port_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_port_param_t	param;
	hal_port_header_t	bspport;

	SDK_ENTER_FUNC();
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);

	hal_ipcmsg_getl(&client->ipcmsg, &param.enable);
	hal_ipcmsg_getl(&client->ipcmsg, &param.value);
	hal_ipcmsg_get(&client->ipcmsg, &param.mac, NSM_MAC_MAX);

	if(!(subcmd_table[subcmd].cmd_handle))
	{
		SDK_LEAVE_FUNC();
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
	SDK_LEAVE_FUNC();
	return ret;
}