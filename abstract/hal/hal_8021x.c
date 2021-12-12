/*
 * hal_8021x.c
 *
 *  Created on: May 7, 2018
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

#include "hal_8021x.h"
#include "hal_driver.h"



int hal_8021x_interface_enable(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putc(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_8021X, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_8021X_PORT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_8021x_state(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_8021X, HAL_MODULE_CMD_SET, HAL_8021X_PORT_STATE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_8021x_mode(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_8021X, HAL_MODULE_CMD_SET, HAL_8021X_PORT_MODE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_8021x_auth_bypass(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_8021X, HAL_MODULE_CMD_SET, HAL_8021X_PORT_BYPASS);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_8021x_interface_dstmac(ifindex_t ifindex, zpl_uchar *mac)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_8021X, HAL_MODULE_CMD_ADD, HAL_8021X_PORT_MAC);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

/*
int hal_8021x_address(zpl_uint32 index, zpl_uint32 address, zpl_uint32 mask)
{
	zpl_uint32 command = 0;
	struct hal_msg
	{
		hal_port_header_t port;
		hal_8021x_param_t param;
	}msg;
	memset(&msg, 0, sizeof(msg));
	msg.param.value = value;
	hal_ipcmsg_port_set(ifindex, &msg.port);
	command = IPCCMD_SET(HAL_MODULE_8021X, HAL_MODULE_CMD_SET, HAL_8021X_PORT_BYPASS);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_8021x_mult_address(zpl_uint32 index, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_msg
	{
		hal_port_header_t port;
		hal_8021x_param_t param;
	}msg;
	memset(&msg, 0, sizeof(msg));
	msg.param.value = value;
	hal_ipcmsg_port_set(ifindex, &msg.port);
	command = IPCCMD_SET(HAL_MODULE_8021X, HAL_MODULE_CMD_SET, HAL_8021X_PORT_BYPASS);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
*/

