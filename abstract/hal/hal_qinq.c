/*
 * hal_qinq.c
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

#include "hal_qinq.h"
#include "hal_driver.h"

int hal_qinq_enable(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_QINQ, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_QINQ_CMD_ENABLE);
	return hal_ipcmsg_send_message(-1,//IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_qinq_vlan_tpid(vlan_t tpid)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putw(&ipcmsg, tpid);
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_QINQ, HAL_MODULE_CMD_SET, HAL_QINQ_CMD_TPID);
	return hal_ipcmsg_send_message(-1,//IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_qinq_interface_enable(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_QINQ, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_QINQ_CMD_ENABLE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
