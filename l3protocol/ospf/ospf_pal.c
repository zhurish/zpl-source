/*
 * ospf_pal.c
 *
 *  Created on: May 18, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"

#include "ospf.h"
#include "ospf_pal.h"
#include "../../platform/lib/if_name.h"



BOOL ospf_if_is_trunk(ifindex_t n)
{
	if(IF_TYPE_GET(n) == IF_LAG)
		return TRUE;
	return FALSE;
}

BOOL ospf_if_is_loopback(ifindex_t n)
{
	if(IF_TYPE_GET(n) == IF_LOOPBACK)
		return TRUE;
	return FALSE;
}


u_int	ospf_if_get_mtu(ifindex_t ifindex)
{
	u_int	uiPortMtulen = 0;
	if(nsm_interface_mtu_get_api(if_lookup_by_index(ifindex), &uiPortMtulen) == OK)
		return uiPortMtulen;
	return 1500;
}

u_int if_name_get_by_index(ifindex_t ifindex, char *name)
{
	if(ifindex2ifname(ifindex) == NULL)
		return ERROR;
	sprintf(name, "%s", ifindex2ifname(ifindex));
	return OK;
}

int ospf_ifunit_to_if_name(u_int uiIfUnit, u_int8 *pucName, u_int uiLen)
{
	struct interface *ifp = if_lookup_by_index (uiIfUnit);
	if(!ifp)
		return ERROR;
	sprintf(pucName, "%s", ifp->name);
	return OK;
}

int log_time_print(u_int8 *buf)
{
	struct tm time;
	memset(&time, 0, sizeof(struct tm));
	time.tm_year = 14;
	time.tm_mon = 5;
	time.tm_mday = 14;
	time.tm_hour = 12;
	time.tm_min = 51;
	time.tm_sec = 4;
	return sprintf(buf, "%d/%02d/%02d %02d:%02d:%02d", time.tm_year + 2000,
			(time.tm_mon + 1) % 13, (time.tm_mday) % 32, time.tm_hour % 25,
			time.tm_min % 61, time.tm_sec % 61);
}

void in_len2mask(struct in_addr *mask, int len)
{
	int i;
	u_char *p;

	if ((len < 0) || (len > 32))
		return;
	p = (u_char *) mask;
	bzero((u_char *) mask, sizeof(*mask));
	for (i = 0; i < len / 8; i++)
		p[i] = 0xff;
	if (len % 8)
		p[i] = (0xff00 >> (len % 8)) & 0xff;
}

int in_mask2len(struct in_addr *mask)
{
	u_int8 *p = (u_int8 *) (mask);
	int i;
	int len = 0;
	for (i = 0; i < 4; i++)
	{
		//ospf_logx(ospf_debug," %02x",p[i]);
		if (p[i] == 0xff)
		{
			len += 8;
		}
		else
		{
			break;
		}
	}
	if (i < 4)
	{
		switch (*p)
		{
		case 0x80:
			len += 1;
			break;
		case 0xc0:
			len += 2;
			break;
		case 0xe0:
			len += 3;
			break;
		case 0xf0:
			len += 4;
			break;
		case 0xf8:
			len += 5;
			break;
		case 0xfc:
			len += 6;
			break;
		case 0xfe:
			len += 7;
			break;
		default:
			break;
		}
	}
	return len;
}
