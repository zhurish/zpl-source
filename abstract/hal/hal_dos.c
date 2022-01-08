/*
 * hL_dos.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */


#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"

#include "hal_dos.h"


int hal_dos_enable(zpl_bool enable, dos_type_en type)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_DOS, HAL_MODULE_CMD_SET, type);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_dos_tcp_hdr_size(zpl_uint32 size)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, size);
	command = IPCCMD_SET(HAL_MODULE_DOS, HAL_MODULE_CMD_SET, HAL_DOS_CMD_TCP_HDR_SIZE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_dos_icmp_size(zpl_bool ipv6, zpl_uint32 size)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, size);
	if(ipv6)
		command = IPCCMD_SET(HAL_MODULE_DOS, HAL_MODULE_CMD_SET, HAL_DOS_CMD_ICMPv6_SIZE);
	else
		command = IPCCMD_SET(HAL_MODULE_DOS, HAL_MODULE_CMD_SET, HAL_DOS_CMD_ICMPv4_SIZE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}



