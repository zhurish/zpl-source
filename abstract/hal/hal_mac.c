/*
 * hal_mac.c
 *
 *  Created on: May 6, 2018
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
#include "os_list.h"

//#include "nsm_client.h"

#include "hal_mac.h"
#include "hal_driver.h"

int hal_mac_age(zpl_uint32 age)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, age);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_SET, HAL_MAC_CMD_AGE);
	return hal_ipcmsg_send_message(-1, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mac_add(ifindex_t ifindex, vlan_t vlan, mac_t *mac, zpl_uint32 pri)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putl(&ipcmsg, pri);
	hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_ADD, HAL_MAC_CMD_ADD);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mac_del(ifindex_t ifindex, vlan_t vlan, mac_t *mac, zpl_uint32 pri)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putl(&ipcmsg, pri);
	hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_DEL, HAL_MAC_CMD_DEL);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mac_clr(ifindex_t ifindex, vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	//hal_ipcmsg_putl(&ipcmsg, pri);
	//hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_DELALL, HAL_MAC_CMD_CLEAR);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mac_read(ifindex_t ifindex, vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	//hal_ipcmsg_putl(&ipcmsg, pri);
	//hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_GET, HAL_MAC_CMD_READ);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

