/*
 * hal_mac.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zpl_type.h"
#include "if.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_mac.h"


int hal_mac_age(zpl_uint32 age)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, age);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_REQ, HAL_MAC_CMD_AGE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mac_clrall(void)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_REQ, HAL_MAC_CMD_CLEARALL);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_mac_add(ifindex_t ifindex, vlan_t vlan, mac_t *mac, zpl_uint32 pri)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putl(&ipcmsg, pri);
	hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_REQ, HAL_MAC_CMD_ADD);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mac_del(ifindex_t ifindex, vlan_t vlan, mac_t *mac, zpl_uint32 pri)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putl(&ipcmsg, pri);
	hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_REQ, HAL_MAC_CMD_DEL);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mac_clr(ifindex_t ifindex, vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));

	hal_ipcmsg_port_set(&ipcmsg, ifindex);

	hal_ipcmsg_putw(&ipcmsg, vlan);
	//hal_ipcmsg_putl(&ipcmsg, pri);
	//hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_REQ, HAL_MAC_CMD_CLEAR);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mac_read(ifindex_t ifindex, vlan_t vlan, int (*callback)(zpl_uint8 *, zpl_uint32, void *), void  *pVoid)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	struct hal_ipcmsg_callback ipcmsg_callback;
	HAL_ENTER_FUNC();
	ipcmsg_callback.ipcmsg_callback = callback;
	ipcmsg_callback.pVoid = pVoid;
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	//hal_ipcmsg_putl(&ipcmsg, pri);
	//hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_GET, HAL_MAC_CMD_READ);
	
	return hal_ipcmsg_getmsg_callback(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg), NULL, &ipcmsg_callback);
}


int hal_mac_dump(ifindex_t ifindex, vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putw(&ipcmsg, vlan);

	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_GET, HAL_MAC_CMD_DUMP);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}