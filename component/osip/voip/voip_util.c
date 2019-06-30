/*
 * voip_util.c
 *
 *  Created on: Dec 8, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"
#include "if.h"
#include "interface.h"

/*
#include <netinet/in.h>
#include <arpa/inet.h>
*/

#include "voip_def.h"
#include "voip_util.h"

#include "os_uci.h"


u_int32 voip_get_address(u_int32 ifindex)
{
	struct interface * ifp = if_lookup_by_index (ifindex);
	if(ifp)
	{
		struct prefix address;
		if(nsm_interface_address_get_api(ifp, &address) == OK)
		{
			//zlog_debug(ZLOG_VOIP, "============ %s :%s -> %s = %x", __func__, ifp->name, ifp->k_name, address.u.prefix4.s_addr);
			if(address.family == AF_INET)
				return ntohl(address.u.prefix4.s_addr);
		}
		//zlog_debug(ZLOG_VOIP, "============ %s :%s -> %s = %x", __func__, ifp->name, ifp->k_name, address.u.prefix4.s_addr);
	}
	return 0;
}

int phone_string_to_hex(char * room, u_int8 *phone)
{
	zassert(room != NULL);
	int i = 0, ln = strlen(room);
	char *src = room;
	while(ln)
	{
		phone[i++] = atoascii(*src);
		src++;
		ln--;
	}
	return i;
}

int phone_string_to_compress(char * room, u_int8 *phone)
{
	zassert(room != NULL);
	int i = 0, j = 0;
	u_int8 phonetmp[64];
	memset(phonetmp, 0, sizeof(phonetmp));
	int n = phone_string_to_hex(room, phonetmp);
	if(n & 0x01)
	{
		phone[j++] = (phonetmp[0] & 0x0f);
		n -= 1;
		for(i = 1; i < n; i+=2)
		{
			phone[j++] = ((phonetmp[i] & 0x0f)<<4)|(phonetmp[i+1] & 0x0f);
		}
	}
	else
	{
		for(i = 0; i < n; i+=2)
		{
			phone[j++] = ((phonetmp[i] & 0x0f)<<4)|(phonetmp[i+1] & 0x0f);
		}
	}
	return j;
}

int phone_compress_to_uncompress(u_int8 *phonetmp, int len, u_int8 *phone)
{
	int i = 0, j = 0;
	if(len & 0x01)
	{
		phone[j++] = (phonetmp[0] & 0xf0)>>4;
		if(phone[0] == 0)
			j = 0;
		phone[j++] = (phonetmp[0] & 0x0f);

		len -= 1;
		for(i = 1; i < len; i++)
		{
			phone[j++] = (phonetmp[i] & 0xf0)>>4;
			phone[j++] = (phonetmp[i] & 0x0f);
		}
	}
	else
	{
		for(i = 0; i < len; i++)
		{
			phone[j++] = (phonetmp[i] & 0xf0)>>4;
			if(phone[0] == 0)
				j = 0;
			phone[j++] = (phonetmp[i] & 0x0f);
		}
	}
	return j;
}


/*
 * test
 */
#if 0
#include <dirent.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/netlink.h>

static int switch_event_socket_read(int fd)
{
	int rcvlen;//, ret;
	char buf[1024] = { 0 };
	//int fd = OS_ANSYNC_FD(pVoid);
try_agent:
	errno = 0;
	os_memset(buf, 0, sizeof(buf));
	rcvlen = recv(fd, buf, sizeof(buf), 0);
	if (rcvlen > 0)
	{
		zlog_debug(ZLOG_VOIP, "add usb_event_socket_read:%s", buf);
		//modem_ansync_add(usb_event_socket_read, fd, "usb_event_socket_read");;
		return OK;
	}
	else
	{
		if (errno == EINTR)
			goto try_agent;
		else if(errno == EAGAIN)
		{
			//os_ansync_register_api(modem_ansync_lst, OS_ANSYNC_INPUT, usb_event_socket_read, NULL, fd);
			return OK;
		}
		else if(errno == ENOBUFS)
		{

		}
		else
		{
			close(fd);
			return ERROR;
			//usb_event_socket_handle(1);
		}
	}
	//MODEM_DR_DEBUG("usb_event_socket_read(%d)(%s)\n", fd, strerror(errno));
	return ERROR;
}

int usb_event_socket_handle(int timeout)
{
	int sock = 0;
	struct sockaddr_nl client;
    os_memset(&client, 0, sizeof(client));
    sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
	if(sock)
	{
		int buffersize = 1024 * 4;
		client.nl_family = AF_NETLINK;
		client.nl_pid = getpid();
		client.nl_groups = 1; /* receive broadcast message*/

		if(bind(sock, (struct sockaddr*)&client, sizeof(client)) == 0)
		{
			//os_set_nonblocking(sock);
			setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));
			while(1)
			{
				if(switch_event_socket_read(sock) == ERROR)
					return OK;
			}
			//return modem_ansync_add(switch_event_socket_read, sock, "usb_event_socket_read");
		}
		close(sock);
		sock = 0;
	}
	return ERROR;
}
#endif
