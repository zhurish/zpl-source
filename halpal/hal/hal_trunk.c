/*
 * hal_trunk.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */
#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"

#include "hal_trunk.h"



int hal_trunkid(zpl_uint32 trunkid)
{
	return (trunkid);
}

int hal_trunk_enable(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	command = IPCCMD_SET(HAL_MODULE_TRUNK, HAL_MODULE_CMD_REQ, HAL_TRUNK_CMD_ENABLE);
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, enable);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, 0);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_trunk_mode(zpl_uint32 trunkid, zpl_uint32 mode)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, 1);
	hal_ipcmsg_putl(&ipcmsg, trunkid);
	hal_ipcmsg_putl(&ipcmsg, mode);
	command = IPCCMD_SET(HAL_MODULE_TRUNK, HAL_MODULE_CMD_REQ, HAL_TRUNK_CMD_MODE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_trunk_add_interface(zpl_uint32 trunkid, ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 1);
	hal_ipcmsg_putl(&ipcmsg, trunkid);
	hal_ipcmsg_putl(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_TRUNK, HAL_MODULE_CMD_REQ, HAL_TRUNK_CMD_ADDIF);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_trunk_del_interface(zpl_uint32 trunkid, ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 1);
	hal_ipcmsg_putl(&ipcmsg, trunkid);
	hal_ipcmsg_putl(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_TRUNK, HAL_MODULE_CMD_REQ, HAL_TRUNK_CMD_DELIF);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_trunk_create(zpl_uint32 trunkid, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, enable);
	hal_ipcmsg_putl(&ipcmsg, trunkid);
	hal_ipcmsg_putl(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_TRUNK, HAL_MODULE_CMD_REQ, HAL_TRUNK_CMD_CREATE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
