/*
 * hal_l3if.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_global.h"
#include "hal_l3if.h"
#include "hal_misc.h"


int hal_l3if_add(ifindex_t ifindex, char *name, mac_t *mac)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	char    ifname[IF_NAME_MAX];
	HAL_ENTER_FUNC();
	os_memset(ifname, 0, sizeof(ifname));
	os_strcpy(ifname, name);
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_put(&ipcmsg, ifname, IF_NAME_MAX);
	if(mac)
		hal_ipcmsg_put(&ipcmsg, mac, 6);
	else
		hal_ipcmsg_putnull(&ipcmsg, 6);	
	command = IPCCMD_SET(HAL_MODULE_L3IF, HAL_MODULE_CMD_REQ, HAL_L3IF_CREATE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_l3if_del(ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	
	command = IPCCMD_SET(HAL_MODULE_L3IF, HAL_MODULE_CMD_REQ, HAL_L3IF_DELETE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_l3if_addr_add(ifindex_t ifindex, struct prefix *address, zpl_bool sec)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);

	hal_ipcmsg_putc(&ipcmsg, address->family);
	hal_ipcmsg_putc(&ipcmsg, address->prefixlen);
	if (address->family == IPSTACK_AF_INET)
		hal_ipcmsg_putl(&ipcmsg, address->u.prefix4.s_addr);
#ifdef ZPL_BUILD_IPV6		
	else
		hal_ipcmsg_put(&ipcmsg, (zpl_uchar *)&address->u.prefix, IPV6_MAX_BYTELEN);
#endif		
	hal_ipcmsg_putc(&ipcmsg, sec);
	command = IPCCMD_SET(HAL_MODULE_L3IF, HAL_MODULE_CMD_REQ, HAL_L3IF_ADDR_ADD);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_l3if_addr_del(ifindex_t ifindex, struct prefix *address, zpl_bool sec)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, address->family);
	hal_ipcmsg_putc(&ipcmsg, address->prefixlen);
	if (address->family == IPSTACK_AF_INET)
		hal_ipcmsg_putl(&ipcmsg, address->u.prefix4.s_addr);
#ifdef ZPL_BUILD_IPV6		
	else
		hal_ipcmsg_put(&ipcmsg, (zpl_uchar *)&address->u.prefix, IPV6_MAX_BYTELEN);
#endif
	hal_ipcmsg_putc(&ipcmsg, sec);
	command = IPCCMD_SET(HAL_MODULE_L3IF, HAL_MODULE_CMD_REQ, HAL_L3IF_ADDR_DEL);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_l3if_dstaddr_add(ifindex_t ifindex, struct prefix *address)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);

	hal_ipcmsg_putc(&ipcmsg, address->family);
	hal_ipcmsg_putc(&ipcmsg, address->prefixlen);
	if (address->family == IPSTACK_AF_INET)
		hal_ipcmsg_putl(&ipcmsg, address->u.prefix4.s_addr);
#ifdef ZPL_BUILD_IPV6		
	else
		hal_ipcmsg_put(&ipcmsg, (zpl_uchar *)&address->u.prefix, IPV6_MAX_BYTELEN);
#endif		
	command = IPCCMD_SET(HAL_MODULE_L3IF, HAL_MODULE_CMD_REQ, HAL_L3IF_DSTADDR_ADD);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_l3if_dstaddr_del(ifindex_t ifindex, struct prefix *address)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, address->family);
	hal_ipcmsg_putc(&ipcmsg, address->prefixlen);
	if (address->family == IPSTACK_AF_INET)
		hal_ipcmsg_putl(&ipcmsg, address->u.prefix4.s_addr);
#ifdef ZPL_BUILD_IPV6		
	else
		hal_ipcmsg_put(&ipcmsg, (zpl_uchar *)&address->u.prefix, IPV6_MAX_BYTELEN);
#endif
	command = IPCCMD_SET(HAL_MODULE_L3IF, HAL_MODULE_CMD_REQ, HAL_L3IF_DSTADDR_DEL);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_l3if_vrf_set(ifindex_t ifindex, vrf_id_t vrfid)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putw(&ipcmsg, vrfid);
	command = IPCCMD_SET(HAL_MODULE_L3IF, HAL_MODULE_CMD_REQ, HAL_L3IF_VRF);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_l3if_mac_set(ifindex_t ifindex, mac_t *mac)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_put(&ipcmsg, mac, 6);
	command = IPCCMD_SET(HAL_MODULE_L3IF, HAL_MODULE_CMD_REQ, HAL_L3IF_MAC);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
