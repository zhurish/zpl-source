/*
 * hal_misc.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"

#include "hal_misc.h"







int hal_snooping_enable (zpl_bool enable, zpl_uint32 mode, zpl_bool ipv6)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	hal_ipcmsg_putc(&ipcmsg, ipv6);
	command = IPCCMD_SET(HAL_MODULE_SWITCH, HAL_MODULE_CMD_REQ, HAL_MISC_DHCP_SNOOP);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}



//EEE
int hal_eee_enable (ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_SWITCH, HAL_MODULE_CMD_REQ, HAL_MISC_EEE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_eee_set (ifindex_t ifindex, void *eee)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putw(&ipcmsg, vlan);
	command = IPCCMD_SET(HAL_MODULE_SWITCH, HAL_MODULE_CMD_REQ, HAL_MISC_EEE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_eee_unset (ifindex_t ifindex, void *eee)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putw(&ipcmsg, vlan);
	command = IPCCMD_SET(HAL_MODULE_SWITCH, HAL_MODULE_CMD_REQ, HAL_MISC_EEE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
