/*
 * hal_mirror.c
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

#include "hal_mirror.h"
#include "hal_driver.h"


int hal_mirror_enable(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	//hal_ipcmsg_putl(&ipcmsg, pri);
	//hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_MIRROR, HAL_MODULE_CMD_ENABLE, HAL_MIRROR_CMD_DST_PORT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mirror_source_enable(ifindex_t ifindex, zpl_bool enable, mirror_mode_t mode, mirror_dir_en type)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, mode);
	hal_ipcmsg_putc(&ipcmsg, type);
	command = IPCCMD_SET(HAL_MODULE_MIRROR, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_MIRROR_CMD_SRC_PORT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_mirror_source_filter_enable(zpl_bool enable, mirror_filter_t filter, mirror_dir_en type, mac_t *mac, mac_t *mac1)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, type);
	hal_ipcmsg_putc(&ipcmsg, filter);
	hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	hal_ipcmsg_put(&ipcmsg, mac1, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_MIRROR, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_MIRROR_CMD_SRC_PORT);
	return hal_ipcmsg_send_message(-1, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
