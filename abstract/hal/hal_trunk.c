/*
 * hal_trunk.c
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

#include "hal_trunk.h"
#include "hal_driver.h"


int hal_trunkid(zpl_uint32 trunkid)
{
	return (trunkid);
}

int hal_trunk_enable(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putl(&ipcmsg, trunkid);
	//hal_ipcmsg_putc(&ipcmsg, mode);
	command = IPCCMD_SET(HAL_MODULE_TRUNK, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_TRUNK_CMD_ENABLE);
	return hal_ipcmsg_send_message(-1,//IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_trunk_mode(zpl_uint32 trunkid, zpl_uint32 mode)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, trunkid);
	hal_ipcmsg_putc(&ipcmsg, mode);
	command = IPCCMD_SET(HAL_MODULE_TRUNK, HAL_MODULE_CMD_SET, HAL_TRUNK_CMD_MODE);
	return hal_ipcmsg_send_message(-1,//IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_trunk_interface_enable(ifindex_t ifindex, zpl_uint32 trunkid)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, trunkid);
	command = IPCCMD_SET(HAL_MODULE_TRUNK, HAL_MODULE_CMD_ADD, HAL_TRUNK_CMD_ADDIF);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_trunk_interface_disable(ifindex_t ifindex, zpl_uint32 trunkid)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, trunkid);
	command = IPCCMD_SET(HAL_MODULE_TRUNK, HAL_MODULE_CMD_DEL, HAL_TRUNK_CMD_DELIF);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

