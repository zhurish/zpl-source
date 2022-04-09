/*
 * hal_mirror.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"

#include "hal_mirror.h"



int hal_mirror_enable(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_MIRROR, HAL_MODULE_CMD_REQ, HAL_MIRROR_CMD_DST_PORT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mirror_source_enable(ifindex_t ifindex, zpl_bool enable, mirror_dir_en type)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	hal_ipcmsg_putc(&ipcmsg, type);
	command = IPCCMD_SET(HAL_MODULE_MIRROR, HAL_MODULE_CMD_REQ, HAL_MIRROR_CMD_SRC_PORT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_mirror_source_filter_enable(ifindex_t ifindex, zpl_bool enable,
	mirror_dir_en dir, mirror_filter_t filter, zpl_uchar *mac)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	hal_ipcmsg_putc(&ipcmsg, dir);
	hal_ipcmsg_putc(&ipcmsg, filter);
	hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_MIRROR, HAL_MODULE_CMD_REQ, HAL_MIRROR_CMD_SRC_PORT);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
