/*
 * hal_port.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "zpl_include.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "nsm_interface.h"
#include <log.h>


//#include "nsm_client.h"

#include "hal_port.h"
#include "hal_driver.h"


int hal_port_up(ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_ENABLE, HAL_PORT_LINK);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_down(ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_DISABLE, HAL_PORT_LINK);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

/*int hal_port_address_set(ifindex_t ifindex, struct prefix *cp, int secondry)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_set_address_cb)
		return hal_driver->port_tbl->sdk_port_set_address_cb(hal_driver->driver, ifindex, cp, secondry);
	return NO_SDK;
}

int hal_port_address_unset(ifindex_t ifindex, struct prefix *cp, int secondry)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_unset_address_cb)
		return hal_driver->port_tbl->sdk_port_unset_address_cb(hal_driver->driver, ifindex, cp, secondry);
	return NO_SDK;
}*/

int hal_port_mac_set(ifindex_t ifindex, zpl_uint8 *cp, zpl_bool secondry)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_put(&ipcmsg, cp, NSM_MAC_MAX);
	hal_ipcmsg_putc(&ipcmsg, secondry);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_SET, HAL_PORT_MAC);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_mtu_set(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_SET, HAL_PORT_MTU);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_metric_set(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_SET, HAL_PORT_METRIC);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_vrf_set(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_SET, HAL_PORT_VRF);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

/*int hal_port_multicast_set(ifindex_t ifindex, int value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_multicast_cb)
		return hal_driver->port_tbl->sdk_port_multicast_cb(hal_driver->driver, ifindex,  value);
	return NO_SDK;
}

int hal_port_bandwidth_set(ifindex_t ifindex, int value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_bandwidth_cb)
		return hal_driver->port_tbl->sdk_port_bandwidth_cb(hal_driver->driver, ifindex,  value);
	return NO_SDK;
}*/

int hal_port_speed_set(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_SET, HAL_PORT_SPEED);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_duplex_set(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_SET, HAL_PORT_DUPLEX);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_mode_set(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_SET, HAL_PORT_MODE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

/*
int hal_port_linkdetect_set(ifindex_t ifindex, int value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_linkdetect_cb)
		return hal_driver->port_tbl->sdk_port_linkdetect_cb(hal_driver->driver, ifindex,  value);
	return NO_SDK;
}
*/


int hal_port_loop_set(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_SET, HAL_PORT_LOOP);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_8021x_set(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_SET, HAL_PORT_8021X);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_jumbo_set(ifindex_t ifindex, zpl_bool enable)
{
	return hal_jumbo_interface_enable(ifindex, enable);
}

int hal_port_enable_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_PORT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

zpl_bool hal_port_state_get(ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_GET, HAL_PORT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

zpl_uint32 hal_port_speed_get(ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_GET, HAL_PORT_SPEED);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

zpl_uint32 hal_port_duplex_get(ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_GET, HAL_PORT_DUPLEX);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_flow_set(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_SET, HAL_PORT_FLOW);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_learning_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_PORT_LEARNING);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_software_learning_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_PORT_SWLEARNING);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_protected_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_PORT_PROTECTED);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_wan_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_PORT_WAN);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
