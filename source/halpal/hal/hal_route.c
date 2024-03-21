/*
 * hal_route.c
 *
 *  Created on: 2019年9月10日
 *      Author: DELL
 */
#include "auto_include.h"
#include "zpl_type.h"
#include "if.h"
#include "nsm_rib.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_route.h"


static int hal_route_multipath_one(zpl_uint16 cmd, zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[2048];
	struct nexthop *nexthop;

	HAL_ENTER_FUNC();
	command = IPCCMD_SET(HAL_MODULE_ROUTE, HAL_MODULE_CMD_REQ, cmd);
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putc(&ipcmsg, safi);
	hal_ipcmsg_putw(&ipcmsg, rib->vrf_id);
	hal_ipcmsg_putc(&ipcmsg, p->family);
	hal_ipcmsg_putl(&ipcmsg, rib->table);
	hal_ipcmsg_putc(&ipcmsg, p->prefixlen);
	if (p->family == IPSTACK_AF_INET)
	{
		nexthop = rib->nexthop;
		hal_ipcmsg_putl(&ipcmsg, p->u.prefix4.s_addr);
		hal_ipcmsg_putl(&ipcmsg, nexthop->src.ipv4.s_addr);
	}
#ifdef ZPL_BUILD_IPV6
	else
	{
		nexthop = rib->nexthop;
		hal_ipcmsg_put(&ipcmsg, (zpl_uchar *)&p->u.prefix, IPV6_MAX_BYTELEN);
		hal_ipcmsg_put(&ipcmsg, (zpl_uchar *)&nexthop->src.ipv6, IPV6_MAX_BYTELEN);
	}
#endif


	hal_ipcmsg_putc(&ipcmsg, num);
	/* Nexthop */
	for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	{
		if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
		{
			/* Interface index. */
			hal_ipcmsg_putl(&ipcmsg, ifindex2ifkernel(nexthop->ifindex));
			hal_ipcmsg_port_set(&ipcmsg, nexthop->ifindex);

			switch (nexthop->type)
			{
			case NEXTHOP_TYPE_IPV4:
			case NEXTHOP_TYPE_IPV4_IFINDEX:
				hal_ipcmsg_putl(&ipcmsg, nexthop->gate.ipv4.s_addr);
				//hal_ipcmsg_putl(&ipcmsg, nexthop->src.ipv4.s_addr);
				break;
#ifdef ZPL_BUILD_IPV6
			case NEXTHOP_TYPE_IPV6:
			case NEXTHOP_TYPE_IPV6_IFINDEX:
			case NEXTHOP_TYPE_IPV6_IFNAME:
				hal_ipcmsg_put(&ipcmsg, (zpl_uchar *)&nexthop->gate.ipv6, 16);
				//hal_ipcmsg_put(&ipcmsg, (zpl_uchar *)&nexthop->src.ipv6, 16);
				break;
#endif
			default:
				if (p->family == IPSTACK_AF_INET)
				{
					struct ipstack_in_addr empty;
					memset(&empty, 0, sizeof(struct ipstack_in_addr));
					hal_ipcmsg_put(&ipcmsg, (zpl_uchar *)&empty, IPV4_MAX_BYTELEN);
					//hal_ipcmsg_put(&ipcmsg, (zpl_uchar *)&empty, IPV4_MAX_BYTELEN);
				}
				else
				{
					struct ipstack_in6_addr empty;
					memset(&empty, 0, sizeof(struct ipstack_in6_addr));
					hal_ipcmsg_put(&ipcmsg, (zpl_uchar *)&empty, IPV6_MAX_BYTELEN);
					//hal_ipcmsg_put(&ipcmsg, (zpl_uchar *)&empty, IPV6_MAX_BYTELEN);
				}
				break;
			}
		}
	}
	/* Put type and nexthop. */
	hal_ipcmsg_putc(&ipcmsg, processid); // processid
	hal_ipcmsg_putc(&ipcmsg, rib->type);
	hal_ipcmsg_putc(&ipcmsg, rib->flags);
	hal_ipcmsg_putc(&ipcmsg, rib->distance);
	hal_ipcmsg_putl(&ipcmsg, rib->metric);
	hal_ipcmsg_putl(&ipcmsg, rib->tag);
	hal_ipcmsg_putl(&ipcmsg, rib->mtu);
	return hal_ipcmsg_send_message(IF_UNIT_ALL,
								   command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_route_multipath_add(zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num)
{
	return hal_route_multipath_one(	HAL_ROUTE_ADD,  processid, safi, p, rib,  num);
}

int hal_route_multipath_del(zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num)
{
	return hal_route_multipath_one(	HAL_ROUTE_DEL,  processid, safi, p, rib,  num);
}