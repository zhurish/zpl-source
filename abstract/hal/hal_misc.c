/*
 * hal_misc.c
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

#include "hal_misc.h"
#include "hal_driver.h"


//jumbo
int hal_jumbo_size(zpl_uint32 size)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, size);
	command = IPCCMD_SET(HAL_MODULE_SWITCH, HAL_MODULE_CMD_SET, HAL_MISC_JUMBO_SIZE);
	return hal_ipcmsg_send_message(-1, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_jumbo_interface_enable(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putw(&ipcmsg, vlan);
	command = IPCCMD_SET(HAL_MODULE_SWITCH, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_MISC_JUMBO);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}



int hal_snooping_enable (zpl_bool enable, zpl_uint32 mode, zpl_bool ipv6)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, ipv6);
	command = IPCCMD_SET(HAL_MODULE_SWITCH, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_MISC_DHCP_SNOOP);
	return hal_ipcmsg_send_message(-1, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}



//EEE
int hal_eee_enable (ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putw(&ipcmsg, vlan);
	command = IPCCMD_SET(HAL_MODULE_SWITCH, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_MISC_EEE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_eee_set (ifindex_t ifindex, void *eee)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putw(&ipcmsg, vlan);
	command = IPCCMD_SET(HAL_MODULE_SWITCH, HAL_MODULE_CMD_ADD, HAL_MISC_EEE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_eee_unset (ifindex_t ifindex, void *eee)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putw(&ipcmsg, vlan);
	command = IPCCMD_SET(HAL_MODULE_SWITCH, HAL_MODULE_CMD_DEL, HAL_MISC_EEE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
