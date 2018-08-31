/*
 * dhcpc_api.h
 *
 *  Created on: Aug 28, 2018
 *      Author: zhurish
 */

#ifndef __DHCPC_API_H__
#define __DHCPC_API_H__

#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>


typedef enum
{
	DHCPC_CLIENT_NONE = 0,

	DHCPC_CLIENT_ADD_IF,
	DHCPC_CLIENT_DEL_IF,

	DHCPC_CLIENT_START_IF,
	DHCPC_CLIENT_STOP_IF,
	DHCPC_CLIENT_RESTART_IF,

	DHCPC_CLIENT_FREE_IF,

	DHCPC_CLIENT_MAX,
}dhcpc_ctrl_en;


typedef struct dhcpc_ctrl
{
	int		action;
	int		ifindex;
	char 	name[IF_NAMESIZE];

}dhcpc_ctrl_head;


extern int _dhcp_syslog(int pri, const char *format, ...);

#if 1
#define dhcp_syslog(pri, fmt,...)	_dhcp_syslog(pri, fmt, ##__VA_ARGS__)
#else
#define dhcp_syslog(pri, fmt,...)
#endif

#endif /* __DHCPC_API_H__ */
