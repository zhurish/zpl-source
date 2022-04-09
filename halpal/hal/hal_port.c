/*
 * hal_port.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_global.h"
#include "hal_port.h"
#include "hal_misc.h"


int hal_port_up(ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 1);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_LINK);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_down(ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_LINK);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}



int hal_port_speed_set(ifindex_t ifindex, nsm_speed_en value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_SPEED);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_duplex_set(ifindex_t ifindex, nsm_duplex_en value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_DUPLEX);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}







int hal_port_loop_set(ifindex_t ifindex, zpl_bool value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_LOOP);
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
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_port_learning_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_LEARNING);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_flow_set(ifindex_t ifindex, zpl_bool tx, zpl_bool rx)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, (tx<<16|rx));
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_FLOW);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}



zpl_bool hal_port_state_get(ifindex_t ifindex)
{
	int ret = 0;
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	struct hal_ipcmsg_getval getvalue;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_GET, HAL_PORT_LINK);
	ret = hal_ipcmsg_send_andget_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg), &getvalue);
	if(ret == OK)
		return 	getvalue.state;
	return ERROR;	
}

nsm_speed_en hal_port_speed_get(ifindex_t ifindex)
{
	int ret = 0;
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	struct hal_ipcmsg_getval getvalue;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_GET, HAL_PORT_SPEED);
	ret = hal_ipcmsg_send_andget_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg), &getvalue);
	if(ret == OK)
		return 	getvalue.value;
	return ERROR;	
}

nsm_duplex_en hal_port_duplex_get(ifindex_t ifindex)
{
	int ret = 0;
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	struct hal_ipcmsg_getval getvalue;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_GET, HAL_PORT_DUPLEX);
	ret = hal_ipcmsg_send_andget_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg), &getvalue);
	if(ret == OK)
		return 	getvalue.value;
	return ERROR;	
}

int hal_port_mode_set(ifindex_t ifindex, if_mode_t value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, (value));
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_MODE);
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
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_put(&ipcmsg, cp, NSM_MAC_MAX);
	hal_ipcmsg_putc(&ipcmsg, secondry);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_MAC);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_port_pause_set(ifindex_t ifindex, zpl_bool pause_tx, zpl_bool pause_rx)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, (pause_tx<<16|pause_rx));
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_PAUSE);
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


/*
int hal_port_linkdetect_set(ifindex_t ifindex, int value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_linkdetect_cb)
		return hal_driver->port_tbl->sdk_port_linkdetect_cb(hal_driver->driver, ifindex,  value);
	return NO_SDK;
}
*/




int hal_port_protected_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_PROTECTED);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_port_software_learning_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_SWLEARNING);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_port_vrf_set(ifindex_t ifindex, vrf_id_t value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_VRF);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}