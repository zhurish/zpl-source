/* Routing Information Base.
 * Copyright (C) 1997, 98, 99, 2001 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "linklist.h"
#include "prefix.h"

#include "str.h"
#include "command.h"
#include "if.h"
#include "log.h"
#include "sockunion.h"
#include "if.h"

#include "host.h"
#include "vty.h"
#include "zmemory.h"
#include "thread.h"
#include "eloop.h"
#include "command.h"
#include "queue.h"
#include "workqueue.h"
#include "nexthop.h"
#include "routemap.h"
#include "table.h"
#include "router-id.h"
#ifdef ZPL_NSM_MODULE
#include "nsm_rib.h"
#include "nsm_zserv.h"
#include "nsm_redistribute.h"
#include "nsm_rnh.h"
#include "nsm_halpal.h"
#include "nsm_debug.h"
#if defined (ZPL_NSM_RTADV)
#include "nsm_rtadv.h"
#endif
#ifdef ZPL_NSM_FPM
#include "fpm.h"
#include "nsm_fpm.h"
#endif
#endif





struct nsm_rib_t nsm_ribd =
{
	.rtm_table_default = 0,
	.nsm_rib_id = 0,
	.initialise = 0,
};

/* Default rtm_table for all clients */
/* Hold time for RIB process, should be very minimal.
 * it is useful to able to set it otherwise for testing, hence exported
 * as global here for test-rig code.
 */
int rib_process_hold_time = 10;
static enum multicast_mode ipv4_multicast_mode = MCAST_NO_CONFIG;

static int nsm_rib_init(void);
static int nsm_rib_task_init(void);
static int nsm_rib_task_exit (void);
static void rib_queue_init(struct nsm_rib_t *nsmribd);

struct module_list module_list_rib = 
{
	.module = MODULE_RIB,
	.name = "RIB\0",
	.module_init = nsm_rib_init,
	.module_exit = NULL,
	.module_task_init = nsm_rib_task_init,
	.module_task_exit = nsm_rib_task_exit,
	.module_cmd_init = NULL,
	.taskid = 0,
	.flags = 0,
};

/* Each route type's string and default distance value. */
static const struct
{
	int key;
	int distance;
} route_info[ZPL_ROUTE_PROTO_MAX] =
{
  [ZPL_ROUTE_PROTO_SYSTEM]  = {ZPL_ROUTE_PROTO_SYSTEM,    0},
  [ZPL_ROUTE_PROTO_KERNEL]  = {ZPL_ROUTE_PROTO_KERNEL,    0},
  [ZPL_ROUTE_PROTO_CONNECT] = {ZPL_ROUTE_PROTO_CONNECT,   0},
  [ZPL_ROUTE_PROTO_STATIC]  = {ZPL_ROUTE_PROTO_STATIC,    1},
  [ZPL_ROUTE_PROTO_RIP]     = {ZPL_ROUTE_PROTO_RIP,     120},
  [ZPL_ROUTE_PROTO_RIPNG]   = {ZPL_ROUTE_PROTO_RIPNG,   120},
  [ZPL_ROUTE_PROTO_OSPF]    = {ZPL_ROUTE_PROTO_OSPF,    110},
  [ZPL_ROUTE_PROTO_OSPF6]   = {ZPL_ROUTE_PROTO_OSPF6,   110},
  [ZPL_ROUTE_PROTO_ISIS]    = {ZPL_ROUTE_PROTO_ISIS,    115},
  [ZPL_ROUTE_PROTO_BGP]     = {ZPL_ROUTE_PROTO_BGP,      20  /* IBGP is 200. */},
  [ZPL_ROUTE_PROTO_BABEL]   = {ZPL_ROUTE_PROTO_BABEL,    95},
  [ZPL_ROUTE_PROTO_NHRP]    = {ZPL_ROUTE_PROTO_NHRP,     10},
  /* no entry/default: 150 */
};

/* RPF lookup behaviour */

static int nsm_rib_task(void *argv)
{
	module_setup_task(MODULE_RIB, os_task_id_self());
	host_waitting_loadconfig();
	thread_mainloop(nsm_ribd.master);
	return 0;
}

static int nsm_rib_init(void)
{
	if(nsm_ribd.initialise == 0)
	{
		memset(&nsm_ribd, 0, sizeof(nsm_ribd));
		if(nsm_ribd.master == NULL)
			nsm_ribd.master = thread_master_module_create(MODULE_RIB);
		if(nsm_ribd.master)	
		{
			nsm_ribd.initialise = 1;
			rib_queue_init(&nsm_ribd);
		}
	}
	return OK;
}
static int nsm_rib_task_init(void)
{
	if(nsm_ribd.initialise == 1)
	{
		if(nsm_ribd.master == NULL)
			nsm_ribd.master = thread_master_module_create(MODULE_RIB);

		if(nsm_ribd.rib_task_id == 0)
			nsm_ribd.rib_task_id = os_task_create("ribTask", OS_TASK_DEFAULT_PRIORITY,
									0, nsm_rib_task, NULL, OS_TASK_DEFAULT_STACK);
		if (nsm_ribd.rib_task_id)
		{
			module_setup_task(MODULE_RIB, nsm_ribd.rib_task_id);
			return OK;
		}	
	}
	return ERROR;
}

static int nsm_rib_task_exit (void)
{
	if(nsm_ribd.rib_task_id)
		os_task_destroy(nsm_ribd.rib_task_id);
	nsm_ribd.rib_task_id = 0;
	nsm_ribd.initialise = 0;
	return OK;
}

static void __attribute__((format (printf, 4, 5)))
_rnode_zlog(const char *_func, struct route_node *rn, int priority,
		const char *msgfmt, ...)
{
	zpl_char prefix[PREFIX_STRLEN], buf[256];
	zpl_char msgbuf[512];
	va_list ap;
	union prefix46constptr up;
	va_start(ap, msgfmt);
	vsnprintf(msgbuf, sizeof(msgbuf), msgfmt, ap);
	va_end(ap);

	if (rn)
	{
		rib_table_info_t *info = rn->table->info;
		up.p = &rn->p;
		snprintf(buf, sizeof(buf), "%s%s vrf %u",
				prefix2str(up, prefix, sizeof(prefix)),
				info->safi == SAFI_MULTICAST ? " (MRIB)" : "",
				info->zvrf->vrf_id);
	}
	else
	{
		snprintf(buf, sizeof(buf), "{(route_node *) NULL}");
	}

	zlog(MODULE_NSM, priority, "%s: %s: %s", _func, buf, msgbuf);
}

#define rnode_debug(node, ...) \
	_rnode_zlog(__func__, node, ZLOG_LEVEL_DEBUG, __VA_ARGS__)
#define rnode_info(node, ...) \
	_rnode_zlog(__func__, node, ZLOG_LEVEL_INFO, __VA_ARGS__)

/* Add nexthop to the end of a rib node's nexthop list */
void rib_nexthop_add(struct rib *rib, struct nexthop *nexthop)
{
	nexthop_add(&rib->nexthop, nexthop);
	rib->nexthop_num++;
}

/* Delete specified nexthop from the list. */
static void rib_nexthop_delete(struct rib *rib, struct nexthop *nexthop)
{
	if (nexthop->next)
		nexthop->next->prev = nexthop->prev;
	if (nexthop->prev)
		nexthop->prev->next = nexthop->next;
	else
		rib->nexthop = nexthop->next;
	rib->nexthop_num--;
}

struct nexthop *
rib_nexthop_ifindex_add(struct rib *rib, ifindex_t ifindex)
{
	struct nexthop *nexthop;

	nexthop = nexthop_new();
	nexthop->type = NEXTHOP_TYPE_IFINDEX;
	nexthop->ifindex = ifindex;

	rib_nexthop_add(rib, nexthop);

	return nexthop;
}

struct nexthop *
rib_nexthop_ifname_add(struct rib *rib, zpl_char *ifname)
{
	struct nexthop *nexthop;

	nexthop = nexthop_new();
	nexthop->type = NEXTHOP_TYPE_IFNAME;
	nexthop->ifname = XSTRDUP(MTYPE_IF_NAME, ifname);

	rib_nexthop_add(rib, nexthop);

	return nexthop;
}

struct nexthop *
rib_nexthop_ipv4_add(struct rib *rib, struct ipstack_in_addr *ipv4, struct ipstack_in_addr *src)
{
	struct nexthop *nexthop;

	nexthop = nexthop_new();
	nexthop->type = NEXTHOP_TYPE_IPV4;
	nexthop->gate.ipv4 = *ipv4;
	if (src)
		nexthop->src.ipv4 = *src;

	rib_nexthop_add(rib, nexthop);

	return nexthop;
}

struct nexthop *
rib_nexthop_ipv4_ifindex_add(struct rib *rib, struct ipstack_in_addr *ipv4,
		struct ipstack_in_addr *src, ifindex_t ifindex)
{
	struct nexthop *nexthop;

	nexthop = nexthop_new();
	nexthop->type = NEXTHOP_TYPE_IPV4_IFINDEX;
	nexthop->gate.ipv4 = *ipv4;
	if (src)
		nexthop->src.ipv4 = *src;
	nexthop->ifindex = ifindex;

	rib_nexthop_add(rib, nexthop);

	return nexthop;
}

#ifdef ZPL_BUILD_IPV6
struct nexthop *
rib_nexthop_ipv6_add (struct rib *rib, struct ipstack_in6_addr *ipv6)
{
	struct nexthop *nexthop;

	nexthop = nexthop_new ();
	nexthop->type = NEXTHOP_TYPE_IPV6;
	nexthop->gate.ipv6 = *ipv6;

	rib_nexthop_add (rib, nexthop);

	return nexthop;
}

static struct nexthop *
rib_nexthop_ipv6_ifname_add (struct rib *rib, struct ipstack_in6_addr *ipv6,
		zpl_char *ifname)
{
	struct nexthop *nexthop;

	nexthop = nexthop_new ();
	nexthop->type = NEXTHOP_TYPE_IPV6_IFNAME;
	nexthop->gate.ipv6 = *ipv6;
	nexthop->ifname = XSTRDUP (MTYPE_IF_NAME, ifname);

	rib_nexthop_add (rib, nexthop);

	return nexthop;
}

struct nexthop *
rib_nexthop_ipv6_ifindex_add (struct rib *rib, struct ipstack_in6_addr *ipv6,
		ifindex_t ifindex)
{
	struct nexthop *nexthop;

	nexthop = nexthop_new ();
	nexthop->type = NEXTHOP_TYPE_IPV6_IFINDEX;
	nexthop->gate.ipv6 = *ipv6;
	nexthop->ifindex = ifindex;

	rib_nexthop_add (rib, nexthop);

	return nexthop;
}
#endif

struct nexthop *
rib_nexthop_blackhole_add(struct rib *rib)
{
	struct nexthop *nexthop;

	nexthop = nexthop_new();
	nexthop->type = NEXTHOP_TYPE_BLACKHOLE;
	SET_FLAG(rib->flags, NSM_RIB_FLAG_BLACKHOLE);

	rib_nexthop_add(rib, nexthop);

	return nexthop;
}

/* This method checks whether a recursive nexthop has at
 * least one resolved nexthop in the fib.
 */
int nexthop_has_fib_child(struct nexthop *nexthop)
{
	struct nexthop *nh;

	if (!CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
		return 0;

	for (nh = nexthop->resolved; nh; nh = nh->next)
		if (CHECK_FLAG(nh->flags, NEXTHOP_FLAG_FIB))
			return 1;

	return 0;
}

/* If force flag is not set, do not modify falgs at all for uninstall
 the route from FIB. */
static int nexthop_active_ipv4(struct rib *rib, struct nexthop *nexthop,
		int set, struct route_node *top)
{
	struct prefix_ipv4 p;
	struct route_table *table;
	struct route_node *rn;
	struct rib *match;
	int resolved;
	struct nexthop *newhop;
	struct nexthop *resolved_hop;

	if (nexthop->type == NEXTHOP_TYPE_IPV4)
		nexthop->ifindex = 0;

	if (set)
	{
		UNSET_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE);
		nexthops_free(nexthop->resolved);
		nexthop->resolved = NULL;
		rib->nexthop_mtu = 0;
	}

	/* Make lookup prefix. */
	memset(&p, 0, sizeof(struct prefix_ipv4));
	p.family = IPSTACK_AF_INET;
	p.prefixlen = IPV4_MAX_PREFIXLEN;
	p.prefix = nexthop->gate.ipv4;

	/* Lookup table.  */
	table = nsm_vrf_table(AFI_IP, SAFI_UNICAST, rib->vrf_id);
	if (!table)
		return 0;

	rn = route_node_match(table, (struct prefix *) &p);
	while (rn)
	{
		route_unlock_node(rn);

		/* If lookup self prefix return immediately. */
		if (rn == top)
			return 0;

		/* Pick up selected route. */
		RNODE_FOREACH_RIB (rn, match)
		{
			if (CHECK_FLAG(match->status, RIB_ENTRY_REMOVED))
				continue;
			if (CHECK_FLAG(match->status, RIB_ENTRY_SELECTED_FIB))
				break;
		}

		/* If there is no selected route or matched route is EGP, go up
		 tree. */
		if (!match || match->type == ZPL_ROUTE_PROTO_BGP)
		{
			do
			{
				rn = rn->parent;
			} while (rn && rn->info == NULL);
			if (rn)
				route_lock_node(rn);
		}
		else
		{
			/* If the longest prefix match for the nexthop yields
			 * a blackhole, mark it as inactive. */
			if (CHECK_FLAG(match->flags, NSM_RIB_FLAG_BLACKHOLE) ||
					CHECK_FLAG (match->flags, NSM_RIB_FLAG_REJECT))
				return 0;

			if (match->type == ZPL_ROUTE_PROTO_CONNECT)
			{
				/* Directly point connected route. */
				newhop = match->nexthop;
				if (newhop && nexthop->type == NEXTHOP_TYPE_IPV4)
					nexthop->ifindex = newhop->ifindex;

				return 1;
			}
			else if (CHECK_FLAG(rib->flags, NSM_RIB_FLAG_INTERNAL))
			{
				resolved = 0;
				for (newhop = match->nexthop; newhop; newhop = newhop->next)
					if (CHECK_FLAG(newhop->flags, NEXTHOP_FLAG_FIB) &&
							!CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_RECURSIVE))
					{
						if (set)
						{
							SET_FLAG(nexthop->flags, NEXTHOP_FLAG_RECURSIVE);

							resolved_hop = XCALLOC(MTYPE_NEXTHOP, sizeof(struct nexthop));
							SET_FLAG(resolved_hop->flags, NEXTHOP_FLAG_ACTIVE);
							/* If the resolving route specifies a gateway, use it */
							if (newhop->type == NEXTHOP_TYPE_IPV4
									|| newhop->type == NEXTHOP_TYPE_IPV4_IFINDEX
									|| newhop->type == NEXTHOP_TYPE_IPV4_IFNAME)
							{
								resolved_hop->type = newhop->type;
								resolved_hop->gate.ipv4 = newhop->gate.ipv4;
								resolved_hop->ifindex = newhop->ifindex;
							}

							/* If the resolving route is an interface route, it
							 * means the gateway we are looking up is connected
							 * to that interface. Therefore, the resolved route
							 * should have the original gateway as nexthop as it
							 * is directly connected. */
							if (newhop->type == NEXTHOP_TYPE_IFINDEX
									|| newhop->type == NEXTHOP_TYPE_IFNAME)
							{
								resolved_hop->type = NEXTHOP_TYPE_IPV4_IFINDEX;
								resolved_hop->gate.ipv4 = nexthop->gate.ipv4;
								resolved_hop->ifindex = newhop->ifindex;
							}

							nexthop_add(&nexthop->resolved, resolved_hop);
						}
						resolved = 1;
					}
				if (resolved && set)
					rib->nexthop_mtu = match->mtu;
				return resolved;
			}
			else
			{
				return 0;
			}
		}
	}
	return 0;
}

#ifdef ZPL_BUILD_IPV6
/* If force flag is not set, do not modify falgs at all for uninstall
 the route from FIB. */
static int
nexthop_active_ipv6 (struct rib *rib, struct nexthop *nexthop, int set,
		struct route_node *top)
{
	struct prefix_ipv6 p;
	struct route_table *table;
	struct route_node *rn;
	struct rib *match;
	int resolved;
	struct nexthop *newhop;
	struct nexthop *resolved_hop;

	if (nexthop->type == NEXTHOP_TYPE_IPV6)
	nexthop->ifindex = 0;

	if (set)
	{
		UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE);
		nexthops_free(nexthop->resolved);
		nexthop->resolved = NULL;
	}

	/* Make lookup prefix. */
	memset (&p, 0, sizeof (struct prefix_ipv6));
	p.family = IPSTACK_AF_INET6;
	p.prefixlen = IPV6_MAX_PREFIXLEN;
	p.prefix = nexthop->gate.ipv6;

	/* Lookup table.  */
	table = nsm_vrf_table (AFI_IP6, SAFI_UNICAST, rib->vrf_id);
	if (! table)
	return 0;

	rn = route_node_match (table, (struct prefix *) &p);
	while (rn)
	{
		route_unlock_node (rn);

		/* If lookup self prefix return immediately. */
		if (rn == top)
		return 0;

		/* Pick up selected route. */
		RNODE_FOREACH_RIB (rn, match)
		{
			if (CHECK_FLAG (match->status, RIB_ENTRY_REMOVED))
			continue;
			if (CHECK_FLAG (match->status, RIB_ENTRY_SELECTED_FIB))
			break;
		}

		/* If there is no selected route or matched route is EGP, go up
		 tree. */
		if (! match
				|| match->type == ZPL_ROUTE_PROTO_BGP)
		{
			do
			{
				rn = rn->parent;
			}while (rn && rn->info == NULL);
			if (rn)
			route_lock_node (rn);
		}
		else
		{
			/* If the longest prefix match for the nexthop yields
			 * a blackhole, mark it as inactive. */
			if (CHECK_FLAG (match->flags, NSM_RIB_FLAG_BLACKHOLE)
					|| CHECK_FLAG (match->flags, NSM_RIB_FLAG_REJECT))
			return 0;

			if (match->type == ZPL_ROUTE_PROTO_CONNECT)
			{
				/* Directly point connected route. */
				newhop = match->nexthop;

				if (newhop && nexthop->type == NEXTHOP_TYPE_IPV6)
				nexthop->ifindex = newhop->ifindex;

				return 1;
			}
			else if (CHECK_FLAG (rib->flags, NSM_RIB_FLAG_INTERNAL))
			{
				resolved = 0;
				for (newhop = match->nexthop; newhop; newhop = newhop->next)
				if (CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_FIB)
						&& ! CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_RECURSIVE))
				{
					if (set)
					{
						SET_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE);

						resolved_hop = XCALLOC(MTYPE_NEXTHOP, sizeof (struct nexthop));
						SET_FLAG (resolved_hop->flags, NEXTHOP_FLAG_ACTIVE);
						/* See nexthop_active_ipv4 for a description how the
						 * resolved nexthop is constructed. */
						if (newhop->type == NEXTHOP_TYPE_IPV6
								|| newhop->type == NEXTHOP_TYPE_IPV6_IFINDEX
								|| newhop->type == NEXTHOP_TYPE_IPV6_IFNAME)
						{
							resolved_hop->type = newhop->type;
							resolved_hop->gate.ipv6 = newhop->gate.ipv6;

							if (newhop->ifindex)
							{
								resolved_hop->type = NEXTHOP_TYPE_IPV6_IFINDEX;
								resolved_hop->ifindex = newhop->ifindex;
							}
						}

						if (newhop->type == NEXTHOP_TYPE_IFINDEX
								|| newhop->type == NEXTHOP_TYPE_IFNAME)
						{
							resolved_hop->flags |= NEXTHOP_FLAG_ONLINK;
							resolved_hop->type = NEXTHOP_TYPE_IPV6_IFINDEX;
							resolved_hop->gate.ipv6 = nexthop->gate.ipv6;
							resolved_hop->ifindex = newhop->ifindex;
						}

						nexthop_add(&nexthop->resolved, resolved_hop);
					}
					resolved = 1;
				}
				return resolved;
			}
			else
			{
				return 0;
			}
		}
	}
	return 0;
}
#endif

struct rib *
rib_match_ipv4_safi(struct ipstack_in_addr addr, safi_t safi, zpl_bool skip_bgp,
		struct route_node **rn_out, vrf_id_t vrf_id)
{
	struct route_table *table;
	struct route_node *rn;
	struct rib *match;
	struct nexthop *newhop, *tnewhop;
	int recursing;

	/* Lookup table.  */
	table = nsm_vrf_table(AFI_IP, safi, vrf_id);
	if (!table)
		return 0;

	rn = route_node_match_ipv4(table, &addr);

	while (rn)
	{
		route_unlock_node(rn);

		/* Pick up selected route. */
		RNODE_FOREACH_RIB (rn, match)
		{
			if (CHECK_FLAG(match->status, RIB_ENTRY_REMOVED))
				continue;
			if (CHECK_FLAG(match->status, RIB_ENTRY_SELECTED_FIB))
				break;
		}

		/* If there is no selected route or matched route is EGP, go up
		 tree. */
		if (!match || (skip_bgp && (match->type == ZPL_ROUTE_PROTO_BGP)))
		{
			do
			{
				rn = rn->parent;
			} while (rn && rn->info == NULL);
			if (rn)
				route_lock_node(rn);
		}
		else
		{
			if (match->type != ZPL_ROUTE_PROTO_CONNECT)
			{
				int found = 0;
				for (ALL_NEXTHOPS_RO(match->nexthop, newhop, tnewhop, recursing)
						)
					if (CHECK_FLAG(newhop->flags, NEXTHOP_FLAG_FIB))
					{
						found = 1;
						break;
					}
				if (!found)
					return NULL;
			}

			if (rn_out)
				*rn_out = rn;
			return match;
		}
	}
	return NULL;
}

struct rib *
rib_match_ipv4_multicast(struct ipstack_in_addr addr, struct route_node **rn_out,
		vrf_id_t vrf_id)
{
	struct rib *rib = NULL, *mrib = NULL, *urib = NULL;
	struct route_node *m_rn = NULL, *u_rn = NULL;
	int skip_bgp = 0; /* bool */

	switch (ipv4_multicast_mode)
	{
	case MCAST_MRIB_ONLY:
		return rib_match_ipv4_safi(addr, SAFI_MULTICAST, skip_bgp, rn_out,
				vrf_id);
	case MCAST_URIB_ONLY:
		return rib_match_ipv4_safi(addr, SAFI_UNICAST, skip_bgp, rn_out, vrf_id);
	case MCAST_NO_CONFIG:
	case MCAST_MIX_MRIB_FIRST:
		rib = mrib = rib_match_ipv4_safi(addr, SAFI_MULTICAST, skip_bgp, &m_rn,
				vrf_id);
		if (!mrib)
			rib = urib = rib_match_ipv4_safi(addr, SAFI_UNICAST, skip_bgp,
					&u_rn, vrf_id);
		break;
	case MCAST_MIX_DISTANCE:
		mrib = rib_match_ipv4_safi(addr, SAFI_MULTICAST, skip_bgp, &m_rn,
				vrf_id);
		urib = rib_match_ipv4_safi(addr, SAFI_UNICAST, skip_bgp, &u_rn, vrf_id);
		if (mrib && urib)
			rib = urib->distance < mrib->distance ? urib : mrib;
		else if (mrib)
			rib = mrib;
		else if (urib)
			rib = urib;
		break;
	case MCAST_MIX_PFXLEN:
		mrib = rib_match_ipv4_safi(addr, SAFI_MULTICAST, skip_bgp, &m_rn,
				vrf_id);
		urib = rib_match_ipv4_safi(addr, SAFI_UNICAST, skip_bgp, &u_rn, vrf_id);
		if (mrib && urib)
			rib = u_rn->p.prefixlen > m_rn->p.prefixlen ? urib : mrib;
		else if (mrib)
			rib = mrib;
		else if (urib)
			rib = urib;
		break;
	}

	if (rn_out)
		*rn_out = (rib == mrib) ? m_rn : u_rn;

	if (IS_NSM_DEBUG_RIB)
	{
		zpl_char buf[BUFSIZ];
		ipstack_inet_ntop(IPSTACK_AF_INET, &addr, buf, BUFSIZ);

		zlog_debug(MODULE_NSM, "%s: %s vrf %u: found %s, using %s", __func__, buf,
				vrf_id,
				mrib ? (urib ? "MRIB+URIB" : "MRIB") :
				urib ? "URIB" : "nothing",
				rib == urib ? "URIB" : rib == mrib ? "MRIB" : "none");
	}
	return rib;
}

void multicast_mode_ipv4_set(enum multicast_mode mode)
{
	if (IS_NSM_DEBUG_RIB)
		zlog_debug(MODULE_NSM, "%s: multicast lookup mode set (%d)", __func__,
				mode);
	ipv4_multicast_mode = mode;
}

enum multicast_mode multicast_mode_ipv4_get(void)
{
	return ipv4_multicast_mode;
}

struct rib *
rib_lookup_ipv4(struct prefix_ipv4 *p, vrf_id_t vrf_id)
{
	struct route_table *table;
	struct route_node *rn;
	struct rib *match;
	struct nexthop *nexthop, *tnexthop;
	int recursing;

	/* Lookup table.  */
	table = nsm_vrf_table(AFI_IP, SAFI_UNICAST, vrf_id);
	if (!table)
		return 0;

	rn = route_node_lookup(table, (struct prefix *) p);

	/* No route for this prefix. */
	if (!rn)
		return NULL;

	/* Unlock node. */
	route_unlock_node(rn);

	RNODE_FOREACH_RIB (rn, match)
	{
		if (CHECK_FLAG(match->status, RIB_ENTRY_REMOVED))
			continue;
		if (CHECK_FLAG(match->status, RIB_ENTRY_SELECTED_FIB))
			break;
	}

	if (!match || match->type == ZPL_ROUTE_PROTO_BGP)
		return NULL;

	if (match->type == ZPL_ROUTE_PROTO_CONNECT)
		return match;

	for (ALL_NEXTHOPS_RO(match->nexthop, nexthop, tnexthop, recursing))
		if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB))
			return match;

	return NULL;
}

/*
 * This clone function, unlike its original rib_lookup_ipv4(), checks
 * if specified IPv4 route record (prefix/mask -> gate) exists in
 * the whole RIB and has NSM_RIB_FLAG_SELECTED set.
 *
 * Return values:
 * -1: error
 * 0: exact match found
 * 1: a match was found with a different gate
 * 2: connected route found
 * 3: no matches found
 */
int rib_lookup_ipv4_route(struct prefix_ipv4 *p, union sockunion * qgate,
		vrf_id_t vrf_id)
{
	struct route_table *table;
	struct route_node *rn;
	struct rib *match;
	struct nexthop *nexthop, *tnexthop;
	int recursing;
	int nexthops_active;

	/* Lookup table.  */
	table = nsm_vrf_table(AFI_IP, SAFI_UNICAST, vrf_id);
	if (!table)
		return NSM_RIB_LOOKUP_ERROR;

	/* Scan the RIB table for exactly matching RIB entry. */
	rn = route_node_lookup(table, (struct prefix *) p);

	/* No route for this prefix. */
	if (!rn)
		return NSM_RIB_NOTFOUND;

	/* Unlock node. */
	route_unlock_node(rn);

	/* Find out if a "selected" RR for the discovered RIB entry exists ever. */
	RNODE_FOREACH_RIB (rn, match)
	{
		if (CHECK_FLAG(match->status, RIB_ENTRY_REMOVED))
			continue;
		if (CHECK_FLAG(match->status, RIB_ENTRY_SELECTED_FIB))
			break;
	}

	/* None such found :( */
	if (!match)
		return NSM_RIB_NOTFOUND;

	if (match->type == ZPL_ROUTE_PROTO_CONNECT)
		return NSM_RIB_FOUND_CONNECTED;

	/* Ok, we have a cood candidate, let's check it's nexthop list... */
	nexthops_active = 0;
	for (ALL_NEXTHOPS_RO(match->nexthop, nexthop, tnexthop, recursing))
		if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB))
		{
			nexthops_active = 1;
			if (nexthop->gate.ipv4.s_addr == sockunion2ip(qgate))
				return NSM_RIB_FOUND_EXACT;
			if (IS_NSM_DEBUG_RIB)
			{
				zpl_char gate_buf[IPSTACK_INET_ADDRSTRLEN], qgate_buf[IPSTACK_INET_ADDRSTRLEN];
				ipstack_inet_ntop(IPSTACK_AF_INET, &nexthop->gate.ipv4.s_addr, gate_buf,
				IPSTACK_INET_ADDRSTRLEN);
				ipstack_inet_ntop(IPSTACK_AF_INET, &sockunion2ip(qgate), qgate_buf,
				IPSTACK_INET_ADDRSTRLEN);
				zlog_debug(MODULE_NSM, "%s: qgate == %s, %s == %s", __func__,
						qgate_buf, recursing ? "rgate" : "gate", gate_buf);
			}
		}

	if (nexthops_active)
		return NSM_RIB_FOUND_NOGATE;

	return NSM_RIB_NOTFOUND;
}

#ifdef ZPL_BUILD_IPV6
struct rib *
rib_match_ipv6 (struct ipstack_in6_addr *addr, vrf_id_t vrf_id)
{
	struct prefix_ipv6 p;
	struct route_table *table;
	struct route_node *rn;
	struct rib *match;
	struct nexthop *newhop, *tnewhop;
	int recursing;

	/* Lookup table.  */
	table = nsm_vrf_table (AFI_IP6, SAFI_UNICAST, vrf_id);
	if (! table)
	return 0;

	memset (&p, 0, sizeof (struct prefix_ipv6));
	p.family = IPSTACK_AF_INET6;
	p.prefixlen = IPV6_MAX_PREFIXLEN;
	IPV6_ADDR_COPY (&p.prefix, addr);

	rn = route_node_match (table, (struct prefix *) &p);

	while (rn)
	{
		route_unlock_node (rn);

		/* Pick up selected route. */
		RNODE_FOREACH_RIB (rn, match)
		{
			if (CHECK_FLAG (match->status, RIB_ENTRY_REMOVED))
			continue;
			if (CHECK_FLAG (match->status, RIB_ENTRY_SELECTED_FIB))
			break;
		}

		/* If there is no selected route or matched route is EGP, go up
		 tree. */
		if (! match
				|| match->type == ZPL_ROUTE_PROTO_BGP)
		{
			do
			{
				rn = rn->parent;
			}while (rn && rn->info == NULL);
			if (rn)
			route_lock_node (rn);
		}
		else
		{
			if (match->type == ZPL_ROUTE_PROTO_CONNECT)
			/* Directly point connected route. */
			return match;
			else
			{
				for (ALL_NEXTHOPS_RO(match->nexthop, newhop, tnewhop, recursing))
				if (CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_FIB))
				return match;
				return NULL;
			}
		}
	}
	return NULL;
}
#endif

#define RIB_SYSTEM_ROUTE(R) \
        ((R)->type == ZPL_ROUTE_PROTO_KERNEL || (R)->type == ZPL_ROUTE_PROTO_CONNECT)

/* This function verifies reachability of one given nexthop, which can be
 * numbered or unnumbered, IPv4 or IPv6. The result is unconditionally stored
 * in nexthop->flags field. If the 4th parameter, 'set', is non-zero,
 * nexthop->ifindex will be updated appropriately as well.
 * An existing route map can turn (otherwise active) nexthop into inactive, but
 * not vice versa.
 *
 * The return value is the final value of 'ACTIVE' flag.
 */

static unsigned nexthop_active_check(struct route_node *rn, struct rib *rib,
		struct nexthop *nexthop, zpl_bool set)
{
	rib_table_info_t *info = rn->table->info;
	struct interface *ifp;

	route_map_result_t ret = RMAP_MATCH;
    extern char *proto_rm[AFI_MAX][ZPL_ROUTE_PROTO_MAX+1];
    struct route_map *rmap;
	zpl_family_t family;

	family = 0;
	switch (nexthop->type)
	{
	case NEXTHOP_TYPE_IFINDEX:
		ifp = if_lookup_by_index_vrf(nexthop->ifindex, rib->vrf_id);
		if (ifp && if_is_operative(ifp))
			SET_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE);
		else
			UNSET_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE);
		break;
	case NEXTHOP_TYPE_IPV6_IFNAME:
	case NEXTHOP_TYPE_IFNAME:
		if(nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME)
			family = AFI_IP6;
		else
			family = AFI_IP;
		ifp = if_lookup_by_name_vrf(nexthop->ifname, rib->vrf_id);
		if (ifp && if_is_operative(ifp))
		{
			if (set)
				nexthop->ifindex = ifp->ifindex;
			SET_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE);
		}
		else
		{
			if (set)
				nexthop->ifindex = 0;
			UNSET_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE);
		}
		break;
	case NEXTHOP_TYPE_IPV4:
	case NEXTHOP_TYPE_IPV4_IFINDEX:
		family = AFI_IP;
		if (nexthop_active_ipv4(rib, nexthop, set, rn))
			SET_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE);
		else
			UNSET_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE);
		break;
#ifdef ZPL_BUILD_IPV6
	case NEXTHOP_TYPE_IPV6:
		family = AFI_IP6;
		if (nexthop_active_ipv6 (rib, nexthop, set, rn))
			SET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
		else
			UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
		break;
	case NEXTHOP_TYPE_IPV6_IFINDEX:
		family = AFI_IP6;
		if (IPSTACK_IN6_IS_ADDR_LINK_LOCAL (&nexthop->gate.ipv6))
		{
			ifp = if_lookup_by_index_vrf (nexthop->ifindex, rib->vrf_id);
			if (ifp && if_is_operative(ifp))
				SET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
			else
				UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
		}
		else
		{
			if (nexthop_active_ipv6 (rib, nexthop, set, rn))
				SET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
			else
				UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
		}
		break;
#endif
	case NEXTHOP_TYPE_BLACKHOLE:
		SET_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE);
		break;
	default:
		break;
	}
	if (!CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
		return 0;

	/* XXX: What exactly do those checks do? Do we support
	 * e.g. IPv4 routes with IPv6 nexthops or vice versa? */
	if (RIB_SYSTEM_ROUTE(rib) || (family == AFI_IP && rn->p.family != IPSTACK_AF_INET)
			|| (family == AFI_IP6 && rn->p.family != IPSTACK_AF_INET6))
		return CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE);

	/* The original code didn't determine the family correctly
	 * e.g. for NEXTHOP_TYPE_IFINDEX. Retrieve the correct afi
	 * from the rib_table_info in those cases.
	 * Possibly it may be better to use only the rib_table_info
	 * in every case.
	 */
	if (!family)
		family = info->afi;

	 rmap = 0;
	 if (rib->type >= 0 && rib->type < ZPL_ROUTE_PROTO_MAX && proto_rm[family][rib->type])
	 	rmap = route_map_lookup_by_name (proto_rm[family][rib->type]);
	 if (!rmap && proto_rm[family][ZPL_ROUTE_PROTO_MAX])
	 	rmap = route_map_lookup_by_name (proto_rm[family][ZPL_ROUTE_PROTO_MAX]);
	 if (rmap) 
	 {
	 	struct nexthop_vrfid nh_vrf = {nexthop, rib->vrf_id};
	 	ret = route_map_apply(rmap, &rn->p, RMAP_NSM, &nh_vrf);
	 }
#ifdef ZPL_NSM_MODULE
	if (ret == RMAP_DENYMATCH)
		UNSET_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE);
#endif
	return CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE);
}

/* Iterate over all nexthops of the given RIB entry and refresh their
 * ACTIVE flag. rib->nexthop_active_num is updated accordingly. If any
 * nexthop is found to toggle the ACTIVE flag, the whole rib structure
 * is flagged with RIB_ENTRY_CHANGED. The 4th 'set' argument is
 * transparently passed to nexthop_active_check().
 *
 * Return value is the new number of active nexthops.
 */

static int nexthop_active_update(struct route_node *rn, struct rib *rib,
		zpl_bool set)
{
	struct nexthop *nexthop;
	zpl_uint32  prev_active, new_active;
	ifindex_t prev_index;

	rib->nexthop_active_num = 0;

	for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	{
		prev_active = CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE);
		prev_index = nexthop->ifindex;
		if ((new_active = nexthop_active_check(rn, rib, nexthop, set)))
			rib->nexthop_active_num++;
		if (prev_active != new_active || prev_index != nexthop->ifindex)
			SET_FLAG(rib->status, RIB_ENTRY_CHANGED);
	}
	return rib->nexthop_active_num;
}


/*static int rib_table_rib_debug(struct route_node *rn, struct rib *rib)
{
	int recursing;
	struct nexthop *nexthop = NULL, *tnexthop = NULL;
	zpl_char prefix[PREFIX_STRLEN], buf[256];
	rib_table_info_t *info = rn->table->info;
	if (rn)
	{
		memset(buf, 0, sizeof(buf));
		memset(prefix, 0, sizeof(prefix));
		snprintf(buf, sizeof(buf), "%s vrf %u",
				prefix2str(&rn->p, prefix, sizeof(prefix)),
				info->zvrf->vrf_id);
	}
	//RNODE_FOREACH_RIB (rn, rib)
	{
		for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
		{
			memset(prefix, 0, sizeof(prefix));
			ipstack_inet_ntop(rn->p.family, &nexthop->gate, prefix, sizeof(prefix));

			printf(" ====== rib add: (%s) %s %s \r\n", prefix_family_str(&rn->p), buf, prefix);
		}
	}
	return OK;
}*/

static int rib_nexthop_num(struct rib *rib)
{
	zpl_uint8 num = 0;
	struct nexthop *nexthop;
	/* Nexthop */
	for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	{
		if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
		{
			num++;
		}
	}
	return num;
}

static int rib_update_forward_pm_install(safi_t safi, struct prefix *p, struct rib *old, struct rib *new)
{
	if (!old && new)
		return nsm_halpal_route_multipath_add(safi, p, new, rib_nexthop_num(new));
	if (old && !new)
		return nsm_halpal_route_multipath_del(safi, p, old, rib_nexthop_num(old));

	/* Replace, can be done atomically if metric does not change;
	 * netlink uses [prefix, tos, priority] to identify prefix.
	 * Now metric is not sent to kernel, so we can just do atomic replace. */
	return nsm_halpal_route_multipath_add(safi, p, new, rib_nexthop_num(new));
}

static int rib_update_forward_pm(struct route_node *rn, struct rib *old,
		struct rib *new)
{
	int ret = 0;
	struct nexthop *nexthop, *tnexthop;
	rib_table_info_t *info = rn->table->info;
	int recursing;

	if (info->safi != SAFI_UNICAST)
	{
		if (new)
			for (ALL_NEXTHOPS_RO(new->nexthop, nexthop, tnexthop, recursing))
				SET_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB);
		if (old)
			for (ALL_NEXTHOPS_RO(old->nexthop, nexthop, tnexthop, recursing))
				UNSET_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB);
		return 0;
	}

  /*
   * Make sure we update the FPM any time we send new information to
   * the kernel.
   */
#ifdef ZPL_NSM_FPM  
  	zfpm_trigger_update (rn, "updating in kernel");
#endif  
	ret = rib_update_forward_pm_install(info->safi, &rn->p, old, new);

	/* This condition is never met, if we are using rt_socket.c */
	if (ret < 0 && new)
	{
		for (ALL_NEXTHOPS_RO(new->nexthop, nexthop, tnexthop, recursing))
			UNSET_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB);
	}
	else if (old && old != new)
	{
		for (ALL_NEXTHOPS_RO(old->nexthop, nexthop, tnexthop, recursing))
			UNSET_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB);
	}

	return ret;
}

/* Uninstall the route from kernel. */
static void rib_uninstall(struct route_node *rn, struct rib *rib)
{
#ifdef ZPL_NSM_FPM 	
  	rib_table_info_t *info = rn->table->info;
#endif
	if (CHECK_FLAG(rib->status, RIB_ENTRY_SELECTED_FIB))
	{
#ifdef ZPL_NSM_FPM 		
      if (info->safi == SAFI_UNICAST)
        zfpm_trigger_update (rn, "rib_uninstall");
#endif		
		nsm_redistribute_route_delete(&rn->p, rib);

		if (!RIB_SYSTEM_ROUTE(rib))
			rib_update_forward_pm(rn, rib, NULL);
		UNSET_FLAG(rib->flags, NSM_RIB_FLAG_SELECTED);
	}
}

static void rib_unlink(struct route_node *, struct rib *);

/*
 * rib_can_delete_dest
 *
 * Returns zpl_true if the given dest can be deleted from the table.
 */
static int rib_can_delete_dest(rib_dest_t *dest)
{
	if (dest->routes)
	{
		return 0;
	}

	/*
	 * Don't delete the dest if we have to update the FPM about this
	 * prefix.
	 */
	if (CHECK_FLAG(dest->flags, RIB_DEST_UPDATE_FPM) ||
	CHECK_FLAG (dest->flags, RIB_DEST_SENT_TO_FPM))
		return 0;

	return 1;
}

/*
 * rib_gc_dest
 *
 * Garbage collect the rib dest corresponding to the given route node
 * if appropriate.
 *
 * Returns zpl_true if the dest was deleted, zpl_false otherwise.
 */
int rib_gc_dest(struct route_node *rn)
{
	rib_dest_t *dest;

	dest = rib_dest_from_rnode(rn);
	if (!dest)
		return 0;

	if (!rib_can_delete_dest(dest))
		return 0;

	if (IS_NSM_DEBUG_RIB)
		rnode_debug(rn, "removing dest from table");

	dest->rnode = NULL;
	XFREE(MTYPE_RIB_DEST, dest);
	rn->info = NULL;

	/*
	 * Release the one reference that we keep on the route node.
	 */
	route_unlock_node(rn);
	return 1;
}

/* Check if 'alternate' RIB entry is better than 'current'. */
static struct rib *
rib_choose_best(struct rib *current, struct rib *alternate)
{
	if (current == NULL)
		return alternate;

	/* filter route selection in following order:
	 * - connected beats other types
	 * - lower distance beats higher
	 * - lower metric beats higher for equal distance
	 * - last, hence oldest, route wins tie break.
	 */

	/* Connected routes. Pick the last connected
	 * route of the set of lowest metric connected routes.
	 */
	if (alternate->type == ZPL_ROUTE_PROTO_CONNECT)
	{
		if (current->type != ZPL_ROUTE_PROTO_CONNECT
				|| alternate->metric <= current->metric)
			return alternate;

		return current;
	}

	if (current->type == ZPL_ROUTE_PROTO_CONNECT)
		return current;

	/* higher distance loses */
	if (alternate->distance < current->distance)
		return alternate;
	if (current->distance < alternate->distance)
		return current;

	/* metric tie-breaks equal distance */
	if (alternate->metric <= current->metric)
		return alternate;

	return current;
}

/* Core function for processing routing information base. */
static void rib_process(struct route_node *rn)
{
	struct rib *rib = NULL;
	struct rib *next = NULL;
	struct rib *old_selected = NULL;
	struct rib *new_selected = NULL;
	struct rib *old_fib = NULL;
	struct rib *new_fib = NULL;
	int installed = 0;
	struct nexthop *nexthop = NULL, *tnexthop = NULL;
	int recursing = 0;
	rib_table_info_t *info = NULL;

	assert(rn);

	info = rn->table->info;

	RNODE_FOREACH_RIB (rn, rib)
	{
		UNSET_FLAG(rib->status, RIB_ENTRY_CHANGED);

		/* Currently installed rib. */
		if (CHECK_FLAG(rib->flags, NSM_RIB_FLAG_SELECTED))
		{
			assert(old_selected == NULL);
			old_selected = rib;
		}
		if (CHECK_FLAG(rib->status, RIB_ENTRY_SELECTED_FIB))
		{
			assert(old_fib == NULL);
			old_fib = rib;
		}

		/* Skip deleted entries from selection */
		if (CHECK_FLAG(rib->status, RIB_ENTRY_REMOVED))
			continue;

		/* Skip unreachable nexthop. */
		if (!nexthop_active_update(rn, rib, 0))
			continue;

		/* Infinit distance. */
		if (rib->distance == DISTANCE_INFINITY)
			continue;

		if (CHECK_FLAG(rib->flags, NSM_RIB_FLAG_FIB_OVERRIDE))
			new_fib = rib_choose_best(new_fib, rib);
		else
			new_selected = rib_choose_best(new_selected, rib);

	} /* RNODE_FOREACH_RIB_SAFE */

	/* If no FIB override route, use the selected route also for FIB */
	if (new_fib == NULL)
		new_fib = new_selected;

	/* After the cycle is finished, the following pointers will be set:
	 * old_selected --- RIB entry currently having SELECTED
	 * new_selected --- RIB entry that is newly SELECTED
	 * old_fib      --- RIB entry currently in kernel FIB
	 * new_fib      --- RIB entry that is newly to be in kernel FIB
	 *
	 * new_selected will get SELECTED flag, and is going to be redistributed
	 * the zclients. new_fib (which can be new_selected) will be installed in kernel.
	 */

	/* Set real nexthops. */
	if (new_fib)
		nexthop_active_update(rn, new_fib, 1);
	if (new_selected && new_selected != new_fib)
		nexthop_active_update(rn, new_selected, 1);

	/* Update kernel if FIB entry has changed */
	if (old_fib != new_fib
			|| (new_fib && CHECK_FLAG(new_fib->status, RIB_ENTRY_CHANGED)))
	{
		if (old_fib && old_fib != new_fib)
		{
			if (!RIB_SYSTEM_ROUTE(old_fib)
					&& (!new_fib || RIB_SYSTEM_ROUTE(new_fib)))
				rib_update_forward_pm(rn, old_fib, NULL);
			UNSET_FLAG(old_fib->status, RIB_ENTRY_SELECTED_FIB);
		}

		if (new_fib)
		{
			/* Install new or replace existing FIB entry */
			SET_FLAG(new_fib->status, RIB_ENTRY_SELECTED_FIB);
			if (!RIB_SYSTEM_ROUTE(new_fib))
				rib_update_forward_pm(rn, old_fib, new_fib);
		}
#ifdef ZPL_NSM_FPM 		
        if (info->safi == SAFI_UNICAST)
          zfpm_trigger_update (rn, "updating existing route");
#endif		  
	}
	else if (old_fib == new_fib && new_fib && !RIB_SYSTEM_ROUTE(new_fib))
	{
		/* Housekeeping code to deal with race conditions in kernel with
		 * linux netlink reporting interface up before IPv4 or IPv6 protocol
		 * is ready to add routes. This makes sure routes are IN the kernel.
		 */
		for (ALL_NEXTHOPS_RO(new_fib->nexthop, nexthop, tnexthop, recursing))
			if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB))
			{
				installed = 1;
				break;
			}
		if (!installed)
			rib_update_forward_pm(rn, NULL, new_fib);
	}

  /* Redistribute SELECTED entry */
  if (old_selected != new_selected
      || (new_selected && CHECK_FLAG (new_selected->status, RIB_ENTRY_CHANGED)))
    {
      if (old_selected)
        {
		
          if (! new_selected)
            nsm_redistribute_route_delete (&rn->p, old_selected);
	
          if (old_selected != new_selected)
            UNSET_FLAG (old_selected->flags, NSM_RIB_FLAG_SELECTED);
        }

      if (new_selected)
        {
          /* Install new or replace existing redistributed entry */
          SET_FLAG (new_selected->flags, NSM_RIB_FLAG_SELECTED);
		  
          nsm_redistribute_route_add (&rn->p, new_selected, old_selected);
		  
        }
     }

	/* Remove all RIB entries queued for removal */
	RNODE_FOREACH_RIB_SAFE (rn, rib, next)
	{
		if (CHECK_FLAG(rib->status, RIB_ENTRY_REMOVED))
		{
			if (IS_NSM_DEBUG_RIB)
				rnode_debug(rn, "rn %p, removing rib %p", (void * )rn,
						(void * )rib);
			rib_unlink(rn, rib);
		}
	}

	if (IS_NSM_DEBUG_RIB_Q)
		rnode_debug(rn, "rn %p dequeued", (void * )rn);

	/*
	 * Check if the dest can be deleted now.
	 */
	rib_gc_dest(rn);
}

/* Take a list of route_node structs and return 1, if there was a record
 * picked from it and processed by rib_process(). Don't process more, 
 * than one RN record; operate only in the specified sub-queue.
 */
static zpl_uint32  process_subq(struct list * subq, zpl_uchar qindex)
{
	struct listnode *lnode = listhead(subq);
	struct route_node *rnode;

	if (!lnode)
		return 0;

	rnode = listgetdata(lnode);

	if(rnode->table->mutex)
		os_mutex_lock(rnode->table->mutex, OS_WAIT_FOREVER);

	rib_process(rnode);

	if (rnode->info)
		UNSET_FLAG(rib_dest_from_rnode(rnode)->flags,
				RIB_ROUTE_QUEUED (qindex));

#if 0
	else
	{
		zlog_debug (MODULE_NSM, "%s: called for route_node (%p, %d) with no ribs",
				__func__, rnode, rnode->lock);
		zlog_backtrace(ZLOG_LEVEL_DEBUG);
	}
#endif

	route_unlock_node(rnode);

	if(rnode->table->mutex)
		os_mutex_unlock(rnode->table->mutex);

	list_delete_node(subq, lnode);
	return 1;
}

/*
 * All meta queues have been processed. Trigger next-hop evaluation.
 */
static void meta_queue_process_complete(struct work_queue *dummy)
{
#ifdef ZPL_NSM_RNH
	nsm_rnh_evaluate_table(0, IPSTACK_AF_INET);

#ifdef ZPL_BUILD_IPV6
	nsm_rnh_evaluate_table(0, IPSTACK_AF_INET6);
#endif /* ZPL_BUILD_IPV6 */
	#endif
}

/* Dispatch the meta queue by picking, processing and unlocking the next RN from
 * a non-empty sub-queue with lowest priority. wq is equal to nsm_ribd->ribq and data
 * is pointed to the meta queue structure.
 */
static wq_item_status meta_queue_process(struct work_queue *dummy, void *data)
{
	struct meta_queue * mq = data;
	unsigned i;

	for (i = 0; i < MQ_SIZE; i++)
		if (process_subq(mq->subq[i], i))
		{
			mq->size--;
			break;
		}
	return mq->size ? WQ_REQUEUE : WQ_SUCCESS;
}

/*
 * Map from rib types to queue type (priority) in meta queue
 */
static const zpl_uchar meta_queue_map[ZPL_ROUTE_PROTO_MAX] = {
  [ZPL_ROUTE_PROTO_SYSTEM]  = 4,
  [ZPL_ROUTE_PROTO_KERNEL]  = 0,
  [ZPL_ROUTE_PROTO_CONNECT] = 0,
  [ZPL_ROUTE_PROTO_STATIC]  = 1,
  [ZPL_ROUTE_PROTO_RIP]     = 2,
  [ZPL_ROUTE_PROTO_RIPNG]   = 2,
  [ZPL_ROUTE_PROTO_OSPF]    = 2,
  [ZPL_ROUTE_PROTO_OSPF6]   = 2,
  [ZPL_ROUTE_PROTO_ISIS]    = 2,
  [ZPL_ROUTE_PROTO_BGP]     = 3,
  [ZPL_ROUTE_PROTO_HSLS]    = 4,
  [ZPL_ROUTE_PROTO_BABEL]   = 2,
  [ZPL_ROUTE_PROTO_NHRP]    = 2,
};

/* Look into the RN and queue it into one or more priority queues,
 * increasing the size for each data push done.
 */
static void rib_meta_queue_add(struct meta_queue *mq, struct route_node *rn)
{
	struct rib *rib;

	RNODE_FOREACH_RIB (rn, rib)
	{
		zpl_uchar qindex = meta_queue_map[rib->type];

		/* Invariant: at this point we always have rn->info set. */
		if (CHECK_FLAG(rib_dest_from_rnode(rn)->flags,
				RIB_ROUTE_QUEUED (qindex)))
		{
			if (IS_NSM_DEBUG_RIB_Q)
				rnode_debug(rn, "rn %p is already queued in sub-queue %u",
						(void * )rn, qindex);
			continue;
		}

		SET_FLAG(rib_dest_from_rnode(rn)->flags, RIB_ROUTE_QUEUED (qindex));
		listnode_add(mq->subq[qindex], rn);
		route_lock_node(rn);
		mq->size++;

		if (IS_NSM_DEBUG_RIB_Q)
			rnode_debug(rn, "queued rn %p into sub-queue %u", (void * )rn,
					qindex);
	}
}

/* Add route_node to work queue and schedule processing */
static void rib_queue_add(struct nsm_rib_t *nsmribd, struct route_node *rn)
{
	assert(nsmribd && rn);

	/* Pointless to queue a route_node with no RIB entries to add or remove */
	if (!rnode_to_ribs(rn))
	{
		zlog_debug(MODULE_NSM, "%s: called for route_node (%p, %d) with no ribs",
				__func__, (void *) rn, rn->lock);
		zlog_backtrace(ZLOG_LEVEL_DEBUG);
		return;
	}

	if (IS_NSM_DEBUG_RIB_Q)
		rnode_info(rn, "work queue added");

	assert(nsmribd);

	if (nsmribd->ribq == NULL)
	{
		zlog_err(MODULE_NSM, "%s: work_queue does not exist!", __func__);
		return;
	}

	/*
	 * The RIB queue should normally be either empty or holding the only
	 * work_queue_item element. In the latter case this element would
	 * hold a pointer to the meta queue structure, which must be used to
	 * actually queue the route nodes to process. So create the MQ
	 * holder, if necessary, then push the work into it in any case.
	 * This semantics was introduced after 0.99.9 release.
	 */
	if (!nsmribd->ribq->items->count)
		work_queue_add(nsmribd->ribq, nsmribd->mq);

	rib_meta_queue_add(nsmribd->mq, rn);

	if (IS_NSM_DEBUG_RIB_Q)
		rnode_debug(rn, "rn %p queued", (void * )rn);

	return;
}

/* Create new meta queue.
 A destructor function doesn't seem to be necessary here.
 */
static struct meta_queue *
meta_queue_new(void)
{
	struct meta_queue *new;
	unsigned i;

	new = XCALLOC(MTYPE_WORK_QUEUE, sizeof(struct meta_queue));
	assert(new);

	for (i = 0; i < MQ_SIZE; i++)
	{
		new->subq[i] = list_new();
		assert(new->subq[i]);
	}

	return new;
}

/* initialise nsmribd rib work queue */
static void rib_queue_init(struct nsm_rib_t *nsmribd)
{
	assert(nsmribd);

	if (!(nsmribd->ribq = work_queue_new(nsmribd->master, "route_node processing")))
	{
		zlog_err(MODULE_NSM, "%s: could not initialise work queue!", __func__);
		return;
	}

	/* fill in the work queue spec */
	nsmribd->ribq->spec.workfunc = &meta_queue_process;
	nsmribd->ribq->spec.errorfunc = NULL;
	nsmribd->ribq->spec.completion_func = &meta_queue_process_complete;
	/* XXX: TODO: These should be runtime configurable via vty */
	nsmribd->ribq->spec.max_retries = 3;
	nsmribd->ribq->spec.hold = rib_process_hold_time;
	if(nsmribd->ribq->master == NULL)
	{
		zlog_err(MODULE_NSM, "%s: nsmribd->ribq->master NULL, master=%p ", __func__, nsmribd->master);
		nsmribd->ribq->master = nsmribd->master;
	}
	if (!(nsmribd->mq = meta_queue_new()))
	{
		zlog_err(MODULE_NSM, "%s: could not initialise meta queue!", __func__);
		return;
	}
	return;
}

/* RIB updates are processed via a queue of pointers to route_nodes.
 *
 * The queue length is bounded by the maximal size of the routing table,
 * as a route_node will not be requeued, if already queued.
 *
 * RIBs are submitted via rib_addnode or rib_delnode which set minimal
 * state, or static_install_route (when an existing RIB is updated)
 * and then submit route_node to queue for best-path selection later.
 * Order of add/delete state changes are preserved for any given RIB.
 *
 * Deleted RIBs are reaped during best-path selection.
 *
 * rib_addnode
 * |-> rib_link or unset RIB_ENTRY_REMOVE        |->Update kernel with
 *       |-------->|                             |  best RIB, if required
 *                 |                             |
 * static_install->|->rib_addqueue...... -> rib_process
 *                 |                             |
 *       |-------->|                             |-> rib_unlink
 * |-> set RIB_ENTRY_REMOVE                           |
 * rib_delnode                                  (RIB freed)
 *
 * The 'info' pointer of a route_node points to a rib_dest_t
 * ('dest'). Queueing state for a route_node is kept on the dest. The
 * dest is created on-demand by rib_link() and is kept around at least
 * as long as there are ribs hanging off it (@see rib_gc_dest()).
 * 
 * Refcounting (aka "locking" throughout the GNU Zebra and Quagga code):
 *
 * - route_nodes: refcounted by:
 *   - dest attached to route_node:
 *     - managed by: rib_link/rib_gc_dest
 *   - route_node processing queue
 *     - managed by: rib_addqueue, rib_process.
 *
 */

/* Add RIB to head of the route node. */
static void rib_link(struct route_node *rn, struct rib *rib)
{
	struct rib *head;
	rib_dest_t *dest;

	assert(rib && rn);

	if (IS_NSM_DEBUG_RIB)
		rnode_debug(rn, "rn %p, rib %p", (void * )rn, (void * )rib);

	dest = rib_dest_from_rnode(rn);
	if (!dest)
	{
		if (IS_NSM_DEBUG_RIB)
			rnode_debug(rn, "adding dest to table");

		dest = XCALLOC(MTYPE_RIB_DEST, sizeof(rib_dest_t));
		route_lock_node(rn); /* rn route table reference */
		rn->info = dest;
		dest->rnode = rn;
	}

	head = dest->routes;
	if (head)
	{
		head->prev = rib;
	}
	rib->next = head;
	dest->routes = rib;
	rib_queue_add(&nsm_ribd, rn);
}

static void rib_addnode(struct route_node *rn, struct rib *rib)
{
	/* RIB node has been un-removed before route-node is processed.
	 * route_node must hence already be on the queue for processing..
	 */
	if (CHECK_FLAG(rib->status, RIB_ENTRY_REMOVED))
	{
		if (IS_NSM_DEBUG_RIB)
			rnode_debug(rn, "rn %p, un-removed rib %p", (void * )rn,
					(void * )rib);

		UNSET_FLAG(rib->status, RIB_ENTRY_REMOVED);
		return;
	}
	rib_link(rn, rib);
}

/*
 * rib_unlink
 *
 * Detach a rib structure from a route_node.
 *
 * Note that a call to rib_unlink() should be followed by a call to
 * rib_gc_dest() at some point. This allows a rib_dest_t that is no
 * longer required to be deleted.
 */
static void rib_unlink(struct route_node *rn, struct rib *rib)
{
	rib_dest_t *dest;

	assert(rn && rib);

	if (IS_NSM_DEBUG_RIB)
		rnode_debug(rn, "rn %p, rib %p", (void * )rn, (void * )rib);

	dest = rib_dest_from_rnode(rn);

	if (rib->next)
		rib->next->prev = rib->prev;

	if (rib->prev)
		rib->prev->next = rib->next;
	else
	{
		dest->routes = rib->next;
	}

	/* free RIB and nexthops */
	nexthops_free(rib->nexthop);
	XFREE(MTYPE_RIB, rib);

}

static void rib_delnode(struct route_node *rn, struct rib *rib)
{
	if (IS_NSM_DEBUG_RIB)
		rnode_debug(rn, "rn %p, rib %p, removing", (void * )rn, (void * )rib);
	SET_FLAG(rib->status, RIB_ENTRY_REMOVED);
	rib_queue_add(&nsm_ribd, rn);
}

int rib_add_ipv4(zpl_uint32 type, zpl_uint32 flags, struct prefix_ipv4 *p,
		struct ipstack_in_addr *gate, struct ipstack_in_addr *src, ifindex_t ifindex,
		vrf_id_t vrf_id, zpl_uint32 table_id, zpl_uint32 metric, zpl_uint32 mtu,
		zpl_uchar distance, safi_t safi)
{
	struct rib *rib;
	struct rib *same = NULL;
	struct route_table *table;
	struct route_node *rn;
	struct nexthop *nexthop;

	/* Lookup table.  */
	table = nsm_vrf_table(AFI_IP, safi, vrf_id);
	if (!table)
		return 0;
	if(table->mutex)
		os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
	/* Make it sure prefixlen is applied to the prefix. */
	apply_mask_ipv4(p);

	/* Set default distance by route type. */
	if (distance == 0)
	{
		if ((unsigned) type >= array_size(route_info))
			distance = 150;
		else
			distance = route_info[type].distance;

		/* iBGP distance is 200. */
		if (type == ZPL_ROUTE_PROTO_BGP && CHECK_FLAG(flags, NSM_RIB_FLAG_IBGP))
			distance = 200;
	}

	/* Lookup route node.*/
	rn = route_node_get(table, (struct prefix *) p);

	/* If same type of route are installed, treat it as a implicit
	 withdraw. */
	RNODE_FOREACH_RIB (rn, rib)
	{
		if (CHECK_FLAG(rib->status, RIB_ENTRY_REMOVED))
			continue;

		if (rib->type != type)
			continue;
		if (rib->type != ZPL_ROUTE_PROTO_CONNECT)
		{
			same = rib;
			break;
		}
		/* Duplicate connected route comes in. */
		else if ((nexthop = rib->nexthop)
				&& nexthop->type == NEXTHOP_TYPE_IFINDEX&&
				nexthop->ifindex == ifindex &&
				!CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
		{
			rib->refcnt++;
			if(table->mutex)
				os_mutex_unlock(table->mutex);
			return 0;
		}
	}

	/* Allocate new rib structure. */
	rib = XCALLOC(MTYPE_RIB, sizeof(struct rib));
	rib->type = type;
	rib->distance = distance;
	rib->flags = flags;
	rib->metric = metric;
	rib->mtu = mtu;
	rib->vrf_id = vrf_id;
	rib->table = table_id;
	rib->nexthop_num = 0;
	rib->uptime = time(NULL);

	/* Nexthop settings. */
	if (gate)
	{
		if (ifindex)
			rib_nexthop_ipv4_ifindex_add(rib, gate, src, ifindex);
		else
			rib_nexthop_ipv4_add(rib, gate, src);
	}
	else
		rib_nexthop_ifindex_add(rib, ifindex);

	/* If this route is kernel route, set FIB flag to the route. */
	if (type == ZPL_ROUTE_PROTO_KERNEL || type == ZPL_ROUTE_PROTO_CONNECT)
		for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
			SET_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB);

	/* Link new rib to node.*/
	if (IS_NSM_DEBUG_RIB)
		zlog_debug(MODULE_NSM, "%s: calling rib_addnode (%p, %p)", __func__,
				(void *) rn, (void *) rib);
	rib_addnode(rn, rib);

	/* Free implicit route.*/
	if (same)
	{
		if (IS_NSM_DEBUG_RIB)
			zlog_debug(MODULE_NSM, "%s: calling rib_delnode (%p, %p)", __func__,
					(void *) rn, (void *) rib);
		rib_delnode(rn, same);
	}

	route_unlock_node(rn);
	if(table->mutex)
		os_mutex_unlock(table->mutex);
	return 0;
}

/* This function dumps the contents of a given RIB entry into
 * standard debug log. Calling function name and IP prefix in
 * question are passed as 1st and 2nd arguments.
 */

void _rib_dump(const char * func, union prefix46constptr pp,
		const struct rib * rib)
{
	const struct prefix *p = pp.p;
	zpl_char straddr[PREFIX_STRLEN];
	struct nexthop *nexthop, *tnexthop;
	int recursing;
	union prefix46constptr up;
	up.p = p;
	zlog_debug(MODULE_NSM, "%s: dumping RIB entry %p for %s vrf %u", func,
			(void *) rib, prefix2str(up, straddr, sizeof(straddr)), rib->vrf_id);
	zlog_debug(MODULE_NSM,
			"%s: refcnt == %lu, uptime == %lu, type == %u, table == %d", func,
			rib->refcnt, (zpl_ulong) rib->uptime, rib->type, rib->table);
	zlog_debug(MODULE_NSM,
			"%s: metric == %u, distance == %u, flags == %u, status == %u", func,
			rib->metric, rib->distance, rib->flags, rib->status);
	zlog_debug(MODULE_NSM,
			"%s: nexthop_num == %u, nexthop_active_num == %u, nexthop_fib_num == %u",
			func, rib->nexthop_num, rib->nexthop_active_num,
			rib->nexthop_fib_num);

	for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
	{
		ipstack_inet_ntop(p->family, &nexthop->gate, straddr, INET6_ADDRSTRLEN);
		zlog_debug(MODULE_NSM, "%s: %s %s with flags %s%s%s", func,
				(recursing ? "  NH" : "NH"), straddr,
				(CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE) ?
						"ACTIVE " : ""),
				(CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB) ? "FIB " : ""),
				(CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE) ?
						"RECURSIVE" : ""));
	}
	zlog_debug(MODULE_NSM, "%s: dump complete", func);
}

/* This is an exported helper to rtm_read() to dump the strange
 * RIB entry found by rib_lookup_ipv4_route()
 */

void rib_lookup_and_dump(struct prefix_ipv4 * p)
{
	struct route_table *table;
	struct route_node *rn;
	struct rib *rib;
	zpl_char prefix_buf[IPSTACK_INET_ADDRSTRLEN];
	union prefix46constptr up;
	/* Lookup table.  */
	table = nsm_vrf_table(AFI_IP, SAFI_UNICAST, VRF_DEFAULT);
	if (!table)
	{
		zlog_err(MODULE_NSM, "%s: nsm_vrf_table() returned NULL", __func__);
		return;
	}
	if(table->mutex)
		os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
	/* Scan the RIB table for exactly matching RIB entry. */
	rn = route_node_lookup(table, (struct prefix *) p);

	/* No route for this prefix. */
	if (!rn)
	{
		up.p = p;
		zlog_debug(MODULE_NSM, "%s: lookup failed for %s", __func__,
				prefix2str(up, prefix_buf, sizeof(prefix_buf)));
		if(table->mutex)
			os_mutex_unlock(table->mutex);
		return;
	}

	/* Unlock node. */
	route_unlock_node(rn);

	/* let's go */
	RNODE_FOREACH_RIB (rn, rib)
	{
		zlog_debug(MODULE_NSM, "%s: rn %p, rib %p: %s, %s", __func__, (void *) rn,
				(void *) rib,
				(CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED) ?
						"removed" : "NOT removed"),
				(CHECK_FLAG (rib->flags, NSM_RIB_FLAG_SELECTED) ?
						"selected" : "NOT selected"));
		rib_dump(up, rib);
	}
	if(table->mutex)
		os_mutex_unlock(table->mutex);
}

int rib_add_ipv4_multipath(struct prefix_ipv4 *p, struct rib *rib, safi_t safi)
{
	struct route_table *table;
	struct route_node *rn;
	struct rib *same;
	struct nexthop *nexthop;
	int ret = 0;
	union prefix46constptr up;
	up.p4 = p;
	/* Lookup table.  */
	table = nsm_vrf_table(AFI_IP, safi, rib->vrf_id);
	if (!table)
		return 0;
	if(table->mutex)
		os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
	/* Make it sure prefixlen is applied to the prefix. */
	apply_mask_ipv4(p);

	/* Set default distance by route type. */
	if (rib->distance == 0)
	{
		rib->distance = route_info[rib->type].distance;

		/* iBGP distance is 200. */
		if (rib->type == ZPL_ROUTE_PROTO_BGP
				&& CHECK_FLAG(rib->flags, NSM_RIB_FLAG_IBGP))
			rib->distance = 200;
	}

	/* Lookup route node.*/
	rn = route_node_get(table, (struct prefix *) p);

	/* If same type of route are installed, treat it as a implicit
	 withdraw. */
	RNODE_FOREACH_RIB (rn, same)
	{
		if (CHECK_FLAG(same->status, RIB_ENTRY_REMOVED))
			continue;

		if (same->type == rib->type&& same->table == rib->table
		&& same->type != ZPL_ROUTE_PROTO_CONNECT)
			break;
	}

	/* If this route is kernel route, set FIB flag to the route. */
	if (rib->type == ZPL_ROUTE_PROTO_KERNEL || rib->type == ZPL_ROUTE_PROTO_CONNECT)
		for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
			SET_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB);

	/* Link new rib to node.*/
	rib_addnode(rn, rib);
	ret = 1;
	if (IS_NSM_DEBUG_RIB)
	{
		zlog_debug(MODULE_NSM, "%s: called rib_addnode (%p, %p) on new RIB entry",
				__func__, (void *) rn, (void *) rib);
		rib_dump(up, rib);
	}

	/* Free implicit route.*/
	if (same)
	{
		if (IS_NSM_DEBUG_RIB)
		{
			zlog_debug(MODULE_NSM,
					"%s: calling rib_delnode (%p, %p) on existing RIB entry",
					__func__, (void *) rn, (void *) same);
			rib_dump(up, same);
		}
		rib_delnode(rn, same);
		ret = -1;
	}

	route_unlock_node(rn);
	if(table->mutex)
		os_mutex_unlock(table->mutex);
	return ret;
}

/* XXX factor with rib_delete_ipv6 */
int rib_delete_ipv4(zpl_uint32 type, zpl_uint32 flags, struct prefix_ipv4 *p,
		struct ipstack_in_addr *gate, ifindex_t ifindex, vrf_id_t vrf_id, safi_t safi)
{
	struct route_table *table;
	struct route_node *rn;
	struct rib *rib;
	struct rib *fib = NULL;
	struct rib *same = NULL;
	struct nexthop *nexthop, *tnexthop;
	int recursing;
	zpl_char buf1[PREFIX_STRLEN];
	zpl_char buf2[IPSTACK_INET_ADDRSTRLEN];
	union prefix46constptr up;
	up.p4 = p;
	/* Lookup table.  */
	table = nsm_vrf_table(AFI_IP, safi, vrf_id);
	if (!table)
		return 0;
	if(table->mutex)
		os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
	/* Apply mask. */
	apply_mask_ipv4(p);

	if (IS_NSM_DEBUG_KERNEL)
	{
		if (gate)
			zlog_debug(MODULE_NSM,
					"rib_delete_ipv4(): route delete %s vrf %u via %s ifindex %d",
					prefix2str(up, buf1, sizeof(buf1)), vrf_id, ipstack_inet_ntoa(*gate),
					ifindex);
		else
			zlog_debug(MODULE_NSM,
					"rib_delete_ipv4(): route delete %s vrf %u ifindex %d",
					prefix2str(up, buf1, sizeof(buf1)), vrf_id, ifindex);
	}

	/* Lookup route node. */
	rn = route_node_lookup(table, (struct prefix *) p);
	if (!rn)
	{
		if (IS_NSM_DEBUG_KERNEL)
		{
			if (gate)
				zlog_debug(MODULE_NSM,
						"route %s vrf %u via %s ifindex %d doesn't exist in rib",
						prefix2str(up, buf1, sizeof(buf1)), vrf_id,
						ipstack_inet_ntop(IPSTACK_AF_INET, gate, buf2, IPSTACK_INET_ADDRSTRLEN),
						ifindex);
			else
				zlog_debug(MODULE_NSM,
						"route %s vrf %u ifindex %d doesn't exist in rib",
						prefix2str(up, buf1, sizeof(buf1)), vrf_id, ifindex);
		}
		if(table->mutex)
			os_mutex_unlock(table->mutex);
		return NSM_RIB_ERR_RTNOEXIST;
	}

	/* Lookup same type route. */
	RNODE_FOREACH_RIB (rn, rib)
	{
		if (CHECK_FLAG(rib->status, RIB_ENTRY_REMOVED))
			continue;

		if (CHECK_FLAG(rib->status, RIB_ENTRY_SELECTED_FIB))
			fib = rib;

		if (rib->type != type)
			continue;
		if (rib->type == ZPL_ROUTE_PROTO_CONNECT && (nexthop = rib->nexthop)
				&& nexthop->type == NEXTHOP_TYPE_IFINDEX)
		{
			if (nexthop->ifindex != ifindex)
				continue;
			if (rib->refcnt)
			{
				rib->refcnt--;
				route_unlock_node(rn);
				route_unlock_node(rn);
				if(table->mutex)
					os_mutex_unlock(table->mutex);
				return 0;
			}
			same = rib;
			break;
		}
		/* Make sure that the route found has the same gateway. */
		else
		{
			if (gate == NULL)
			{
				same = rib;
				break;
			}
			for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
				if (IPV4_ADDR_SAME(&nexthop->gate.ipv4, gate))
				{
					same = rib;
					break;
				}
			if (same)
				break;
		}
	}
	/* If same type of route can't be found and this message is from
	 kernel. */
	if (!same)
	{
		if (fib && type == ZPL_ROUTE_PROTO_KERNEL
				&& CHECK_FLAG(flags, NSM_RIB_FLAG_SELFROUTE))
		{
			if (IS_NSM_DEBUG_KERNEL)
			{
				zlog_debug(MODULE_NSM,
						"Zebra route %s/%d was deleted by others from kernel",
						ipstack_inet_ntop(IPSTACK_AF_INET, &p->prefix, buf1, IPSTACK_INET_ADDRSTRLEN),
						p->prefixlen);
			}
			/* This means someone else, other than Zebra, has deleted
			 * a Zebra router from the kernel. We will add it back */
			rib_update_forward_pm(rn, NULL, fib);
		}
		else
		{
			if (IS_NSM_DEBUG_KERNEL)
			{
				if (gate)
					zlog_debug(MODULE_NSM,
							"route %s vrf %u via %s ifindex %d type %d "
									"doesn't exist in rib",
							prefix2str(up, buf1, sizeof(buf1)), vrf_id,
							ipstack_inet_ntop(IPSTACK_AF_INET, gate, buf2, IPSTACK_INET_ADDRSTRLEN),
							ifindex, type);
				else
					zlog_debug(MODULE_NSM,
							"route %s vrf %u ifindex %d type %d doesn't exist in rib",
							prefix2str(up, buf1, sizeof(buf1)), vrf_id, ifindex,
							type);
			}
			route_unlock_node(rn);
			if(table->mutex)
				os_mutex_unlock(table->mutex);
			return NSM_RIB_ERR_RTNOEXIST;
		}
	}

	if (same)
		rib_delnode(rn, same);

	route_unlock_node(rn);
	if(table->mutex)
		os_mutex_unlock(table->mutex);
	return 0;
}

/* Install static route into rib. */
static void static_install_route(afi_t afi, safi_t safi, struct prefix *p,
		struct static_route *si)
{
	struct rib *rib;
	struct route_node *rn;
	struct route_table *table;

	/* Lookup table.  */
	table = nsm_vrf_table(afi, safi, si->vrf_id);
	if (!table)
		return;
	if(table->mutex)
		os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
	/* Lookup existing route */
	rn = route_node_get(table, p);
	RNODE_FOREACH_RIB (rn, rib)
	{
		if (CHECK_FLAG(rib->status, RIB_ENTRY_REMOVED))
			continue;

		if (rib->type == ZPL_ROUTE_PROTO_STATIC && rib->distance == si->distance)
			break;
	}

	if (rib)
	{
		/* if tag value changed , update old value in RIB */
		if (rib->tag != si->tag)
			rib->tag = si->tag;

		/* Same distance static route is there.  Update it with new
		 nexthop. */
		route_unlock_node(rn);
		switch (si->type)
		{
		case STATIC_IPV4_GATEWAY:
			rib_nexthop_ipv4_add(rib, &si->addr.ipv4, NULL);
			break;
		case STATIC_IPV4_IFNAME:
			rib_nexthop_ifname_add(rib, si->ifname);
			break;
		case STATIC_IPV4_BLACKHOLE:
			rib_nexthop_blackhole_add(rib);
			break;
#ifdef ZPL_BUILD_IPV6
			case STATIC_IPV6_GATEWAY:
			rib_nexthop_ipv6_add (rib, &si->addr.ipv6);
			break;
			case STATIC_IPV6_IFNAME:
			rib_nexthop_ifname_add (rib, si->ifname);
			break;
			case STATIC_IPV6_GATEWAY_IFNAME:
			rib_nexthop_ipv6_ifname_add (rib, &si->addr.ipv6, si->ifname);
			break;
#endif
		}
		rib_queue_add(&nsm_ribd, rn);
	}
	else
	{
		/* This is new static route. */
		rib = XCALLOC(MTYPE_RIB, sizeof(struct rib));

		rib->type = ZPL_ROUTE_PROTO_STATIC;
		rib->distance = si->distance;
		rib->metric = 0;
		rib->vrf_id = si->vrf_id;
		rib->table = nsm_ribd.rtm_table_default;
		rib->nexthop_num = 0;
		rib->tag = si->tag;

		switch (si->type)
		{
		case STATIC_IPV4_GATEWAY:
			rib_nexthop_ipv4_add(rib, &si->addr.ipv4, NULL);
			break;
		case STATIC_IPV4_IFNAME:
			rib_nexthop_ifname_add(rib, si->ifname);
			break;
		case STATIC_IPV4_BLACKHOLE:
			rib_nexthop_blackhole_add(rib);
			break;
#ifdef ZPL_BUILD_IPV6
			case STATIC_IPV6_GATEWAY:
			rib_nexthop_ipv6_add (rib, &si->addr.ipv6);
			break;
			case STATIC_IPV6_IFNAME:
			rib_nexthop_ifname_add (rib, si->ifname);
			break;
			case STATIC_IPV6_GATEWAY_IFNAME:
			rib_nexthop_ipv6_ifname_add (rib, &si->addr.ipv6, si->ifname);
			break;
#endif
		}

		/* Save the flags of this static routes (reject, blackhole) */
		rib->flags = si->flags;

		/* Link this rib to the tree. */
		rib_addnode(rn, rib);
	}
	if(table->mutex)
		os_mutex_unlock(table->mutex);
}

static int static_nexthop_same(struct nexthop *nexthop, struct static_route *si)
{
	if (nexthop->type == NEXTHOP_TYPE_IPV4&& si->type == STATIC_IPV4_GATEWAY
	&& IPV4_ADDR_SAME (&nexthop->gate.ipv4, &si->addr.ipv4))
		return 1;
	if (nexthop->type == NEXTHOP_TYPE_IFNAME && si->type == STATIC_IPV4_IFNAME
			&& strcmp(nexthop->ifname, si->ifname) == 0)
		return 1;
	if (nexthop->type
			== NEXTHOP_TYPE_BLACKHOLE&& si->type == STATIC_IPV4_BLACKHOLE)
		return 1;
#ifdef ZPL_BUILD_IPV6
	if (nexthop->type == NEXTHOP_TYPE_IPV6
			&& si->type == STATIC_IPV6_GATEWAY
			&& IPV6_ADDR_SAME (&nexthop->gate.ipv6, &si->addr.ipv6))
	return 1;
	if (nexthop->type == NEXTHOP_TYPE_IFNAME
			&& si->type == STATIC_IPV6_IFNAME
			&& strcmp (nexthop->ifname, si->ifname) == 0)
	return 1;
	if (nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME
			&& si->type == STATIC_IPV6_GATEWAY_IFNAME
			&& IPV6_ADDR_SAME (&nexthop->gate.ipv6, &si->addr.ipv6)
			&& strcmp (nexthop->ifname, si->ifname) == 0)
	return 1;
#endif
	return 0;
}

/* Uninstall static route from RIB. */
static void static_uninstall_route(afi_t afi, safi_t safi, struct prefix *p,
		struct static_route *si)
{
	struct route_node *rn;
	struct rib *rib;
	struct nexthop *nexthop;
	struct route_table *table;

	/* Lookup table.  */
	table = nsm_vrf_table(afi, safi, si->vrf_id);
	if (!table)
		return;
	if(table->mutex)
		os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
	/* Lookup existing route with type and distance. */
	rn = route_node_lookup(table, p);
	if (!rn)
	{
		if(table->mutex)
			os_mutex_unlock(table->mutex);
		return;
	}
	RNODE_FOREACH_RIB (rn, rib)
	{
		if (CHECK_FLAG(rib->status, RIB_ENTRY_REMOVED))
			continue;

		if (rib->type == ZPL_ROUTE_PROTO_STATIC && rib->distance == si->distance
				&& rib->tag == si->tag)
			break;
	}

	if (!rib)
	{
		route_unlock_node(rn);
		if(table->mutex)
			os_mutex_unlock(table->mutex);
		return;
	}

	/* Lookup nexthop. */
	for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
		if (static_nexthop_same(nexthop, si))
			break;

	/* Can't find nexthop. */
	if (!nexthop)
	{
		route_unlock_node(rn);
		if(table->mutex)
			os_mutex_unlock(table->mutex);
		return;
	}

	/* Check nexthop. */
	if (rib->nexthop_num == 1)
		rib_delnode(rn, rib);
	else
	{
		if (CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_FIB))
			rib_uninstall(rn, rib);
		rib_nexthop_delete(rib, nexthop);
		nexthop_free(nexthop);
		rib_queue_add(&nsm_ribd, rn);
	}
	/* Unlock node. */
	route_unlock_node(rn);
	if(table->mutex)
		os_mutex_unlock(table->mutex);
}

int
static_add_ipv4_safi (safi_t safi, struct prefix *p, struct ipstack_in_addr *gate,
		      const char *ifname, zpl_uchar flags, route_tag_t tag,
		      zpl_uchar distance, vrf_id_t vrf_id)
{
	zpl_uchar type = 0;
	struct route_node *rn;
	struct static_route *si;
	struct static_route *pp;
	struct static_route *cp;
	struct static_route *update = NULL;
	struct nsm_ipvrf *zvrf = ip_vrf_info_lookup(vrf_id);
	if (!zvrf)
		return -1;
	struct route_table *stable = zvrf->stable[AFI_IP][safi];

	if (!stable)
		return -1;
	if(stable->mutex)
		os_mutex_lock(stable->mutex, OS_WAIT_FOREVER);
	/* Lookup static route prefix. */
	rn = route_node_get(stable, p);

	/* Make flags. */
	if (gate)
		type = STATIC_IPV4_GATEWAY;
	else if (ifname)
		type = STATIC_IPV4_IFNAME;
	else
		type = STATIC_IPV4_BLACKHOLE;

	/* Do nothing if there is a same static route.  */
	for (si = rn->info; si; si = si->next)
	{
		if (type == si->type && (!gate || IPV4_ADDR_SAME(gate, &si->addr.ipv4))
				&& (!ifname || strcmp(ifname, si->ifname) == 0))
		{
			if (distance == si->distance && tag == si->tag)
			{
				route_unlock_node(rn);
				if(stable->mutex)
					os_mutex_unlock(stable->mutex);
				return 0;
			}
			else
				update = si;
		}
	}

	/* Distance or tag changed. */
	if (update)
		static_delete_ipv4_safi(safi, p, gate, ifname, update->tag,
				update->distance, vrf_id);

	/* Make new static route structure. */
	si = XCALLOC(MTYPE_STATIC_ROUTE, sizeof(struct static_route));

	si->type = type;
	si->distance = distance;
	si->tag = tag;
	si->flags = flags;
	si->vrf_id = vrf_id;

	if (gate)
		si->addr.ipv4 = *gate;
	if (ifname)
		si->ifname = XSTRDUP(MTYPE_IF_NAME, ifname);

	/* Add new static route information to the tree with sort by
	 distance value and gateway address. */
	for (pp = NULL, cp = rn->info; cp; pp = cp, cp = cp->next)
	{
		if (si->distance < cp->distance)
			break;
		if (si->distance > cp->distance)
			continue;
		if (si->type == STATIC_IPV4_GATEWAY && cp->type == STATIC_IPV4_GATEWAY)
		{
			if (ntohl(si->addr.ipv4.s_addr) < ntohl(cp->addr.ipv4.s_addr))
				break;
			if (ntohl(si->addr.ipv4.s_addr) > ntohl(cp->addr.ipv4.s_addr))
				continue;
		}
	}

	/* Make linked list. */
	if (pp)
		pp->next = si;
	else
		rn->info = si;
	if (cp)
		cp->prev = si;
	si->prev = pp;
	si->next = cp;
	if(stable->mutex)
		os_mutex_unlock(stable->mutex);
	/* Install into rib. */
	static_install_route(AFI_IP, safi, p, si);

	return 1;
}

int static_delete_ipv4_safi(safi_t safi, struct prefix *p, struct ipstack_in_addr *gate,
		const char *ifname, route_tag_t tag, zpl_uchar distance, vrf_id_t vrf_id)
{
	zpl_uchar type = 0;
	struct route_node *rn;
	struct static_route *si;
	struct route_table *stable;

	/* Lookup table.  */
	stable = nsm_vrf_static_table(AFI_IP, safi, vrf_id);
	if (!stable)
		return -1;
	if(stable->mutex)
		os_mutex_lock(stable->mutex, OS_WAIT_FOREVER);
	/* Lookup static route prefix. */
	rn = route_node_lookup(stable, p);
	if (!rn)
	{
		if(stable->mutex)
			os_mutex_unlock(stable->mutex);
		return 0;
	}
	/* Make flags. */
	if (gate)
		type = STATIC_IPV4_GATEWAY;
	else if (ifname)
		type = STATIC_IPV4_IFNAME;
	else
		type = STATIC_IPV4_BLACKHOLE;

	/* Find same static route is the tree */
	for (si = rn->info; si; si = si->next)
		if (type == si->type && (!gate || IPV4_ADDR_SAME(gate, &si->addr.ipv4))
				&& (!ifname || strcmp(ifname, si->ifname) == 0)
				&& (!tag || (tag == si->tag)))
			break;

	/* Can't find static route. */
	if (!si)
	{
		route_unlock_node(rn);
		if(stable->mutex)
			os_mutex_unlock(stable->mutex);
		return 0;
	}

	/* Install into rib. */
	if(stable->mutex)
		os_mutex_unlock(stable->mutex);
	static_uninstall_route(AFI_IP, safi, p, si);
	if(stable->mutex)
		os_mutex_lock(stable->mutex, OS_WAIT_FOREVER);
	/* Unlink static route from linked list. */
	if (si->prev)
		si->prev->next = si->next;
	else
		rn->info = si->next;
	if (si->next)
		si->next->prev = si->prev;
	route_unlock_node(rn);

	/* Free static route configuration. */
	if (ifname)
		XFREE(MTYPE_IF_NAME, si->ifname);
	XFREE(MTYPE_STATIC_ROUTE, si);

	route_unlock_node(rn);
	if(stable->mutex)
		os_mutex_unlock(stable->mutex);
	return 1;
}
#ifdef ZPL_BUILD_IPV6
int
rib_add_ipv6 (zpl_uint32 type, zpl_uint32 flags, struct prefix_ipv6 *p,
	      struct ipstack_in6_addr *gate, ifindex_t ifindex,
	      vrf_id_t vrf_id, zpl_uint32 table_id,
	      zpl_uint32 metric, zpl_uint32 mtu, zpl_uchar distance, safi_t safi)
{
	struct rib *rib;
	struct rib *same = NULL;
	struct route_table *table;
	struct route_node *rn;
	struct nexthop *nexthop;
	union prefix46constptr up;
	up.p6 = p;
	/* Lookup table.  */
	table = nsm_vrf_table (AFI_IP6, safi, vrf_id);
	if (! table)
	return 0;
	if(table->mutex)
		os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
	/* Make sure mask is applied. */
	apply_mask_ipv6 (p);

	/* Set default distance by route type. */
	if (!distance)
	distance = route_info[type].distance;

	if (type == ZPL_ROUTE_PROTO_BGP && CHECK_FLAG (flags, NSM_RIB_FLAG_IBGP))
	distance = 200;

	/* Lookup route node.*/
	rn = route_node_get (table, (struct prefix *) p);

	/* If same type of route are installed, treat it as a implicit
	 withdraw. */
	RNODE_FOREACH_RIB (rn, rib)
	{
		if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
		continue;

		if (rib->type != type)
		continue;
		if (rib->type != ZPL_ROUTE_PROTO_CONNECT)
		{
			same = rib;
			break;
		}
		else if ((nexthop = rib->nexthop) &&
				nexthop->type == NEXTHOP_TYPE_IFINDEX &&
				nexthop->ifindex == ifindex)
		{
			rib->refcnt++;
			if(table->mutex)
				os_mutex_unlock(table->mutex);
			return 0;
		}
	}

	/* Allocate new rib structure. */
	rib = XCALLOC (MTYPE_RIB, sizeof (struct rib));

	rib->type = type;
	rib->distance = distance;
	rib->flags = flags;
	rib->metric = metric;
	rib->mtu = mtu;
	rib->vrf_id = vrf_id;
	rib->table = table_id;
	rib->nexthop_num = 0;
	rib->uptime = time (NULL);

	/* Nexthop settings. */
	if (gate)
	{
		if (ifindex)
		rib_nexthop_ipv6_ifindex_add (rib, gate, ifindex);
		else
		rib_nexthop_ipv6_add (rib, gate);
	}
	else
	rib_nexthop_ifindex_add (rib, ifindex);

	/* If this route is kernel route, set FIB flag to the route. */
	if (type == ZPL_ROUTE_PROTO_KERNEL || type == ZPL_ROUTE_PROTO_CONNECT)
	for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);

	/* Link new rib to node.*/
	rib_addnode (rn, rib);
	if (IS_NSM_DEBUG_RIB)
	{
		zlog_debug (MODULE_NSM, "%s: called rib_addnode (%p, %p) on new RIB entry",
				__func__, (void *)rn, (void *)rib);
		rib_dump (up, rib);
	}

	/* Free implicit route.*/
	if (same)
	{
		if (IS_NSM_DEBUG_RIB)
		{
			zlog_debug (MODULE_NSM, "%s: calling rib_delnode (%p, %p) on existing RIB entry",
					__func__, (void *)rn, (void *)same);
			rib_dump (up, same);
		}
		rib_delnode (rn, same);
	}

	route_unlock_node (rn);
	if(table->mutex)
		os_mutex_unlock(table->mutex);
	return 0;
}

int
rib_add_ipv6_multipath (struct prefix_ipv6 *p, struct rib *rib, safi_t safi)
{
	struct route_table *table;
	struct route_node *rn;
	struct rib *same = NULL;
	struct nexthop *nexthop;
	int ret = 0;
	union prefix46constptr up;
	up.p = p;
	if (!rib)
	return 0; /* why are we getting called with NULL rib */

	/* Lookup table.  */
	table = nsm_vrf_table (AFI_IP6, safi, rib->vrf_id);

	if (! table)
	return 0;
	if(table->mutex)
		os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
	/* Make sure mask is applied. */
	apply_mask_ipv6 (p);

	/* Set default distance by route type. */
	if (rib->distance == 0)
	{
		rib->distance = route_info[rib->type].distance;

		/* iBGP distance is 200. */
		if (rib->type == ZPL_ROUTE_PROTO_BGP
				&& CHECK_FLAG (rib->flags, NSM_RIB_FLAG_IBGP))
		rib->distance = 200;
	}

	/* Lookup route node.*/
	rn = route_node_get (table, (struct prefix *) p);

	/* If same type of route are installed, treat it as a implicit
	 withdraw. */
	RNODE_FOREACH_RIB (rn, same)
	{
		if (CHECK_FLAG (same->status, RIB_ENTRY_REMOVED))
		{
			continue;
		}
		if (same->type != rib->type)
		{
			continue;
		}

		if (same->table != rib->table)
		{
			continue;
		}
		if (same->type != ZPL_ROUTE_PROTO_CONNECT)
		{
			break;
		}
	}

	/* If this route is kernel route, set FIB flag to the route. */
	if (rib->type == ZPL_ROUTE_PROTO_KERNEL || rib->type == ZPL_ROUTE_PROTO_CONNECT)
	{
		for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
		{
			SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
		}
	}

	/* Link new rib to node.*/
	rib_addnode (rn, rib);
	ret = 1;
	/* Free implicit route.*/
	if (same)
	{
		if (IS_NSM_DEBUG_RIB)
		{
			zlog_debug (MODULE_NSM, "%s: calling rib_delnode (%p, %p) on existing RIB entry",
					__func__, rn, same);
			rib_dump (up, same);
		}
		rib_delnode (rn, same);
		ret = -1;
	}

	route_unlock_node (rn);
	if(table->mutex)
		os_mutex_unlock(table->mutex);
	return ret;
}

/* XXX factor with rib_delete_ipv6 */
int
rib_delete_ipv6 (zpl_uint32 type, zpl_uint32 flags, struct prefix_ipv6 *p,
		struct ipstack_in6_addr *gate, ifindex_t ifindex,
		vrf_id_t vrf_id, safi_t safi)
{
	struct route_table *table;
	struct route_node *rn;
	struct rib *rib;
	struct rib *fib = NULL;
	struct rib *same = NULL;
	struct nexthop *nexthop, *tnexthop;
	int recursing;
	zpl_char buf1[PREFIX_STRLEN];
	zpl_char buf2[INET6_ADDRSTRLEN];
	union prefix46constptr up;
	up.p6 = p;
	/* Apply mask. */
	apply_mask_ipv6 (p);

	/* Lookup table.  */
	table = nsm_vrf_table (AFI_IP6, safi, vrf_id);
	if (! table)
	return 0;
	if(table->mutex)
		os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
	/* Lookup route node. */
	rn = route_node_lookup (table, (struct prefix *) p);
	if (! rn)
	{
		if (IS_NSM_DEBUG_KERNEL)
		{
			if (gate)
			zlog_debug (MODULE_NSM, "route %s vrf %u via %s ifindex %d doesn't exist in rib",
					prefix2str (up, buf1, sizeof(buf1)), vrf_id,
					ipstack_inet_ntop (IPSTACK_AF_INET6, gate, buf2, INET6_ADDRSTRLEN),
					ifindex);
			else
			zlog_debug (MODULE_NSM, "route %s vrf %u ifindex %d doesn't exist in rib",
					prefix2str (up, buf1, sizeof(buf1)), vrf_id,
					ifindex);
		}
		if(table->mutex)
			os_mutex_unlock(table->mutex);
		return NSM_RIB_ERR_RTNOEXIST;
	}

	/* Lookup same type route. */
	RNODE_FOREACH_RIB (rn, rib)
	{
		if (CHECK_FLAG(rib->status, RIB_ENTRY_REMOVED))
		continue;

		if (CHECK_FLAG (rib->status, RIB_ENTRY_SELECTED_FIB))
		fib = rib;

		if (rib->type != type)
		continue;
		if (rib->type == ZPL_ROUTE_PROTO_CONNECT && (nexthop = rib->nexthop) &&
				nexthop->type == NEXTHOP_TYPE_IFINDEX)
		{
			if (nexthop->ifindex != ifindex)
			continue;
			if (rib->refcnt)
			{
				rib->refcnt--;
				route_unlock_node (rn);
				route_unlock_node (rn);
				if(table->mutex)
					os_mutex_unlock(table->mutex);
				return 0;
			}
			same = rib;
			break;
		}
		/* Make sure that the route found has the same gateway. */
		else
		{
			if (gate == NULL)
			{
				same = rib;
				break;
			}
			for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
			if (IPV6_ADDR_SAME (&nexthop->gate.ipv6, gate))
			{
				same = rib;
				break;
			}
			if (same)
			break;
		}
	}

	/* If same type of route can't be found and this message is from
	 kernel. */
	if (! same)
	{
		if (fib && type == ZPL_ROUTE_PROTO_KERNEL &&
				CHECK_FLAG(flags, NSM_RIB_FLAG_SELFROUTE))
		{
			if (IS_NSM_DEBUG_KERNEL)
			{
				zlog_debug (MODULE_NSM, "Zebra route %s/%d was deleted by others from kernel",
						ipstack_inet_ntop (IPSTACK_AF_INET, &p->prefix, buf1, IPSTACK_INET_ADDRSTRLEN),
						p->prefixlen);
			}
			/* This means someone else, other than Zebra, has deleted a Zebra
			 * route from the kernel. We will add it back */
			rib_update_forward_pm(rn, NULL, fib);
		}
		else
		{
			if (IS_NSM_DEBUG_KERNEL)
			{
				if (gate)
				zlog_debug (MODULE_NSM, "route %s vrf %u via %s ifindex %d type %d "
						"doesn't exist in rib",
						prefix2str (up, buf1, sizeof(buf1)), vrf_id,
						ipstack_inet_ntop (IPSTACK_AF_INET6, gate, buf2, INET6_ADDRSTRLEN),
						ifindex,
						type);
				else
				zlog_debug (MODULE_NSM, "route %s vrf %u ifindex %d type %d doesn't exist in rib",
						prefix2str (up, buf1, sizeof(buf1)), vrf_id,
						ifindex,
						type);
			}
			route_unlock_node (rn);
			if(table->mutex)
				os_mutex_unlock(table->mutex);
			return NSM_RIB_ERR_RTNOEXIST;
		}
	}

	if (same)
	rib_delnode (rn, same);

	route_unlock_node (rn);
	if(table->mutex)
		os_mutex_unlock(table->mutex);
	return 0;
}

/* Add static route into static route configuration. */
int
static_add_ipv6 (struct prefix *p, zpl_uchar type, struct ipstack_in6_addr *gate,
		const char *ifname, zpl_uchar flags, route_tag_t tag,
		zpl_uchar distance, vrf_id_t vrf_id)
{
	struct route_node *rn;
	struct static_route *si;
	struct static_route *pp;
	struct static_route *cp;
	struct static_route *update = NULL;
	struct nsm_ipvrf *zvrf = ip_vrf_info_lookup (vrf_id);
	if(!zvrf)
	return -1;
	struct route_table *stable = zvrf->stable[AFI_IP6][SAFI_UNICAST];

	if (! stable)
	return -1;

	if (!gate &&
			(type == STATIC_IPV6_GATEWAY || type == STATIC_IPV6_GATEWAY_IFNAME))
	return -1;

	if (!ifname &&
			(type == STATIC_IPV6_GATEWAY_IFNAME || type == STATIC_IPV6_IFNAME))
	return -1;

	if(stable->mutex)
		os_mutex_lock(stable->mutex, OS_WAIT_FOREVER);
	/* Lookup static route prefix. */
	rn = route_node_get (stable, p);

	/* Do nothing if there is a same static route.  */
	for (si = rn->info; si; si = si->next)
	{
		if (type == si->type
				&& tag == si->tag
				&& (! gate || IPV6_ADDR_SAME (gate, &si->addr.ipv6))
				&& (! ifname || strcmp (ifname, si->ifname) == 0))
		{
			if (distance == si->distance)
			{
				route_unlock_node (rn);
				if(stable->mutex)
					os_mutex_unlock(stable->mutex);
				return 0;
			}
			else
			update = si;
		}
	}

	if (update)
	{
		if(stable->mutex)
			os_mutex_unlock(stable->mutex);
		static_delete_ipv6(p, type, gate, ifname, tag, update->distance, vrf_id);
		if(stable->mutex)
			os_mutex_lock(stable->mutex, OS_WAIT_FOREVER);
	}
	/* Make new static route structure. */
	si = XCALLOC (MTYPE_STATIC_ROUTE, sizeof (struct static_route));

	si->type = type;
	si->distance = distance;
	si->tag = tag;
	si->flags = flags;
	si->vrf_id = vrf_id;

	switch (type)
	{
		case STATIC_IPV6_GATEWAY:
		si->addr.ipv6 = *gate;
		break;
		case STATIC_IPV6_IFNAME:
		si->ifname = XSTRDUP (MTYPE_IF_NAME, ifname);
		break;
		case STATIC_IPV6_GATEWAY_IFNAME:
		si->addr.ipv6 = *gate;
		si->ifname = XSTRDUP (MTYPE_IF_NAME, ifname);
		break;
	}

	/* Add new static route information to the tree with sort by
	 distance value and gateway address. */
	for (pp = NULL, cp = rn->info; cp; pp = cp, cp = cp->next)
	{
		if (si->distance < cp->distance)
		break;
		if (si->distance > cp->distance)
		continue;
	}

	/* Make linked list. */
	if (pp)
	pp->next = si;
	else
	rn->info = si;
	if (cp)
	cp->prev = si;
	si->prev = pp;
	si->next = cp;

	/* Install into rib. */
	if(stable->mutex)
		os_mutex_unlock(stable->mutex);
	static_install_route (AFI_IP6, SAFI_UNICAST, p, si);

	return 1;
}

/* Delete static route from static route configuration. */
int
static_delete_ipv6 (struct prefix *p, zpl_uchar type, struct ipstack_in6_addr *gate,
		const char *ifname, route_tag_t tag, zpl_uchar distance,
		vrf_id_t vrf_id)
{
	struct route_node *rn;
	struct static_route *si;
	struct route_table *stable;

	/* Lookup table.  */
	stable = nsm_vrf_static_table (AFI_IP6, SAFI_UNICAST, vrf_id);
	if (! stable)
		return -1;
	if(stable->mutex)
		os_mutex_lock(stable->mutex, OS_WAIT_FOREVER);
	/* Lookup static route prefix. */
	rn = route_node_lookup (stable, p);
	if (! rn)
	{
		if(stable->mutex)
			os_mutex_unlock(stable->mutex);
		return 0;
	}

	/* Find same static route is the tree */
	for (si = rn->info; si; si = si->next)
	if (distance == si->distance
			&& type == si->type
			&& (! gate || IPV6_ADDR_SAME (gate, &si->addr.ipv6))
			&& (! ifname || strcmp (ifname, si->ifname) == 0)
			&& (! tag || (tag == si->tag)))
	break;

	/* Can't find static route. */
	if (! si)
	{
		route_unlock_node (rn);
		if(stable->mutex)
			os_mutex_unlock(stable->mutex);
		return 0;
	}

	/* Install into rib. */
	if(stable->mutex)
		os_mutex_unlock(stable->mutex);
	static_uninstall_route (AFI_IP6, SAFI_UNICAST, p, si);
	if(stable->mutex)
		os_mutex_lock(stable->mutex, OS_WAIT_FOREVER);
	/* Unlink static route from linked list. */
	if (si->prev)
	si->prev->next = si->next;
	else
	rn->info = si->next;
	if (si->next)
	si->next->prev = si->prev;

	/* Free static route configuration. */
	if (ifname)
	XFREE (MTYPE_IF_NAME, si->ifname);
	XFREE (MTYPE_STATIC_ROUTE, si);
	if(stable->mutex)
		os_mutex_unlock(stable->mutex);
	return 1;
}
#endif
/* RIB update function. */
void rib_update(vrf_id_t vrf_id)
{
	struct route_node *rn;
	struct route_table *table;

	table = nsm_vrf_table(AFI_IP, SAFI_UNICAST, vrf_id);
	if (table)
	{
		if(table->mutex)
			os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
		for (rn = route_top(table); rn; rn = route_next(rn))
		{
			if (rnode_to_ribs(rn))
				rib_queue_add(&nsm_ribd, rn);
		}
		if(table->mutex)
			os_mutex_unlock(table->mutex);
	}

	table = nsm_vrf_table(AFI_IP6, SAFI_UNICAST, vrf_id);
	if (table)
	{
		if(table->mutex)
			os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
		for (rn = route_top(table); rn; rn = route_next(rn))
		{
			if (rnode_to_ribs(rn))
				rib_queue_add(&nsm_ribd, rn);
		}
		if(table->mutex)
			os_mutex_unlock(table->mutex);
	}
}

/* Remove all routes which comes from non main table.  */
static void rib_weed_table(struct route_table *table)
{
	struct route_node *rn;
	struct rib *rib;
	struct rib *next;

	if (table)
	{
		if(table->mutex)
			os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
		for (rn = route_top(table); rn; rn = route_next(rn))
		{
			RNODE_FOREACH_RIB_SAFE (rn, rib, next)
			{
				if (CHECK_FLAG(rib->status, RIB_ENTRY_REMOVED))
					continue;

				if (rib->table != nsm_ribd.rtm_table_default
						&& rib->table != IPSTACK_RT_TABLE_MAIN)
					rib_delnode(rn, rib);
			}
		}
		if(table->mutex)
			os_mutex_unlock(table->mutex);
	}
}


/* Delete all routes from non main table. */
static int rib_weed_tables_one(struct ip_vrf *vrf, void *p)
{
	struct nsm_ipvrf *zvrf = vrf->info;
	rib_weed_table(zvrf->table[AFI_IP][SAFI_UNICAST]);
	rib_weed_table(zvrf->table[AFI_IP6][SAFI_UNICAST]);
	return 0;
}
void rib_weed_tables(void)
{
	ip_vrf_foreach(rib_weed_tables_one, NULL);
}

#if 0
/* Delete self installed routes after nsmribd is relaunched.  */
static void
rib_sweep_table (struct route_table *table)
{
	struct route_node *rn;
	struct rib *rib;
	struct rib *next;
	int ret = 0;

	if (table)
	for (rn = route_top (table); rn; rn = route_next (rn))
	RNODE_FOREACH_RIB_SAFE (rn, rib, next)
	{
		if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
		continue;

		if (rib->type == ZPL_ROUTE_PROTO_KERNEL &&
				CHECK_FLAG (rib->flags, NSM_RIB_FLAG_SELFROUTE))
		{
			ret = rib_update_forward_pm (rn, rib, NULL);
			if (! ret)
			rib_delnode (rn, rib);
		}
	}
}
#endif

/* Sweep all RIB tables.  */

static int rib_sweep_tables_one(struct ip_vrf *vrf, void *p)
{
	struct nsm_ipvrf *zvrf = vrf->info;
	rib_weed_table(zvrf->table[AFI_IP][SAFI_UNICAST]);
	rib_weed_table(zvrf->table[AFI_IP6][SAFI_UNICAST]);
	return 0;
}
void rib_sweep_route(void)
{
	ip_vrf_foreach(rib_sweep_tables_one, NULL);
}

/* Remove specific by protocol routes from 'table'. */
static zpl_ulong rib_score_proto_table(zpl_uchar proto,
		struct route_table *table)
{
	struct route_node *rn;
	struct rib *rib;
	struct rib *next;
	zpl_ulong n = 0;

	if (table)
	{
		if(table->mutex)
			os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
		for (rn = route_top(table); rn; rn = route_next(rn))
		{
			RNODE_FOREACH_RIB_SAFE (rn, rib, next)
			{
				if (CHECK_FLAG(rib->status, RIB_ENTRY_REMOVED))
					continue;
				if (rib->type == proto)
				{
					rib_delnode(rn, rib);
					n++;
				}
			}
		}
		if(table->mutex)
			os_mutex_unlock(table->mutex);
	}

	return n;
}



/* Remove specific by protocol routes. */
static int rib_score_proto_one(struct ip_vrf *vrf, void *p)
{
	int *rib_test = (int *)p;
	struct nsm_ipvrf *zvrf = vrf->info;
	rib_test[1] += rib_score_proto_table(rib_test[0], zvrf->table[AFI_IP][SAFI_UNICAST])
					+ rib_score_proto_table(rib_test[0], zvrf->table[AFI_IP6][SAFI_UNICAST]);
	return 0;
}

zpl_ulong rib_score_proto(zpl_uchar proto)
{
	int rib_test[] = {0, 0};
	rib_test[0] = proto;
	rib_test[1] = 0;
	ip_vrf_foreach(rib_score_proto_one, &rib_test);
	return rib_test[1];
}

/* Close RIB and clean up kernel routes. */
void rib_close_table(struct route_table *table)
{
	struct route_node *rn = NULL;
	zassert(table != NULL);
	zassert(table->info != NULL);
#ifdef ZPL_NSM_FPM 	
	rib_table_info_t *info = table->info;
#endif	
	struct rib *rib = NULL;

	if (table)
	{
		if(table->mutex)
			os_mutex_lock(table->mutex, OS_WAIT_FOREVER);
		for (rn = route_top(table); rn; rn = route_next(rn))
		{
			RNODE_FOREACH_RIB (rn, rib)
			{
				if (!CHECK_FLAG(rib->status, RIB_ENTRY_SELECTED_FIB))
					continue;
#ifdef ZPL_NSM_FPM 
          		if (info->safi == SAFI_UNICAST)
            		zfpm_trigger_update (rn, NULL);
#endif			
				if (!RIB_SYSTEM_ROUTE(rib))
					rib_update_forward_pm(rn, rib, NULL);
			}
		}
		if(table->mutex)
			os_mutex_unlock(table->mutex);
	}
}

/* Close all RIB tables.  */
static int rib_close_one(struct ip_vrf *vrf, void *p)
{
	struct nsm_ipvrf *zvrf = vrf->info;
	rib_close_table(zvrf->table[AFI_IP][SAFI_UNICAST]);
	rib_close_table(zvrf->table[AFI_IP6][SAFI_UNICAST]);
	return 0;
}
void rib_close(void)
{
	ip_vrf_foreach(rib_close_one, NULL);
}

/*
 * vrf_id_get_next
 *
 * Get the first vrf id that is greater than the given vrf id if any.
 *
 * Returns zpl_true if a vrf id was found, zpl_false otherwise.
 */
static inline int vrf_id_get_next(vrf_id_t vrf_id, vrf_id_t *next_id_p)
{
	struct ip_vrf * ip_vrf = ip_vrf_lookup ( vrf_id);
	if(ip_vrf)
	{
		if (_ip_vrf_master.vrf_mutex)
      		os_mutex_lock(_ip_vrf_master.vrf_mutex, OS_WAIT_FOREVER);
		struct ip_vrf * ip_vrf_next = (struct ip_vrf *)lstNext((NODE*)ip_vrf);
		if(ip_vrf_next)
		{
			*next_id_p = ip_vrf_next->vrf_id;
			if (_ip_vrf_master.vrf_mutex)
      			os_mutex_unlock(_ip_vrf_master.vrf_mutex);
			return 1;
		}
		if (_ip_vrf_master.vrf_mutex)
      		os_mutex_unlock(_ip_vrf_master.vrf_mutex);
	}
	return 0;
}

/*
 * rib_tables_iter_next
 *
 * Returns the next table in the iteration.
 */
struct route_table *
rib_tables_iter_next(rib_tables_iter_t *iter)
{
	struct route_table *table;

	/*
	 * Array that helps us go over all AFI/SAFI combinations via one
	 * index.
	 */
	static struct
	{
		afi_t afi;
		safi_t safi;
	} afi_safis[] =
	{
	{ AFI_IP, SAFI_UNICAST },
	{ AFI_IP, SAFI_MULTICAST },
	{ AFI_IP6, SAFI_UNICAST },
	{ AFI_IP6, SAFI_MULTICAST }, };

	table = NULL;

	switch (iter->state)
	{

	case RIB_TABLES_ITER_S_INIT:
		iter->vrf_id = VRF_DEFAULT;
		iter->afi_safi_ix = -1;

		/* Fall through */

	case RIB_TABLES_ITER_S_ITERATING:
		iter->afi_safi_ix++;
		while (1)
		{

			while (iter->afi_safi_ix < (int) array_size(afi_safis))
			{
				table = nsm_vrf_table(afi_safis[iter->afi_safi_ix].afi,
						afi_safis[iter->afi_safi_ix].safi, iter->vrf_id);
				if (table)
					break;

				iter->afi_safi_ix++;
			}

			/*
			 * Found another table in this vrf.
			 */
			if (table)
				break;

			/*
			 * Done with all tables in the current vrf, go to the next
			 * one.
			 */
			if (!vrf_id_get_next(iter->vrf_id, &iter->vrf_id))
				break;

			iter->afi_safi_ix = 0;
		}

		break;

	case RIB_TABLES_ITER_S_DONE:
		return NULL;
	}

	if (table)
		iter->state = RIB_TABLES_ITER_S_ITERATING;
	else
		iter->state = RIB_TABLES_ITER_S_DONE;

	return table;
}

/* Lookup VRF by identifier.  */
struct nsm_ipvrf * nsm_vrf_lookup(vrf_id_t vrf_id)
{
	return ip_vrf_info_lookup(vrf_id);
}

/*
 * Create a routing table for the specific AFI/SAFI in the given VRF.
 */
static void nsm_vrf_table_create(struct nsm_ipvrf *zvrf, afi_t afi, safi_t safi)
{
	rib_table_info_t *info;
	struct route_table *table;

	assert(!zvrf->table[afi][safi]);

	table = route_table_init();
	table->mutex = os_mutex_name_init(os_name_format("%s-mutex",zvrf->name));
	zvrf->table[afi][safi] = table;

	info = XCALLOC(MTYPE_RIB_TABLE_INFO, sizeof(rib_table_info_t));
	info->zvrf = zvrf;
	info->afi = afi;
	info->safi = safi;
	table->info = info;
}
static void nsm_vrf_table_destroy(struct nsm_ipvrf *zvrf, afi_t afi, safi_t safi)
{
	struct route_table *table = NULL;

	table = zvrf->table[afi][safi];
	if(table)
	{
		rib_close_table(table);
		if(table->info)
		{
			XFREE(MTYPE_RIB_TABLE_INFO, table->info);
			table->info = NULL;
		}
		if(table->mutex)
		{
			os_mutex_exit(table->mutex);
			table->mutex = NULL;
		}
		route_table_finish(table);
		zvrf->table[afi][safi] = NULL;
	}
}
/* Allocate new nsmribd VRF. */
static struct nsm_ipvrf * nsm_vrf_alloc(const char *name, vrf_id_t vrf_id)
{
	struct nsm_ipvrf *zvrf = NULL;

	zvrf = XCALLOC(MTYPE_VRF, sizeof(struct nsm_ipvrf));
	if(zvrf)
	{
		if(name)
			zvrf->name = strdup(name);
		/* Allocate routing table and static table.  */
		nsm_vrf_table_create(zvrf, AFI_IP, SAFI_UNICAST);
		nsm_vrf_table_create(zvrf, AFI_IP6, SAFI_UNICAST);

		zvrf->stable[AFI_IP][SAFI_UNICAST] = route_table_init();
		zvrf->stable[AFI_IP][SAFI_UNICAST]->mutex = os_mutex_name_init(os_name_format("%s-un-mutex",zvrf->name));
		zvrf->stable[AFI_IP6][SAFI_UNICAST] = route_table_init();
		zvrf->stable[AFI_IP6][SAFI_UNICAST]->mutex = os_mutex_name_init(os_name_format("%s-unv6-mutex",zvrf->name));

		nsm_vrf_table_create(zvrf, AFI_IP, SAFI_MULTICAST);
		nsm_vrf_table_create(zvrf, AFI_IP6, SAFI_MULTICAST);

		zvrf->stable[AFI_IP][SAFI_MULTICAST] = route_table_init();
		zvrf->stable[AFI_IP][SAFI_MULTICAST]->mutex = os_mutex_name_init(os_name_format("%s-mu-mutex",zvrf->name));
		zvrf->stable[AFI_IP6][SAFI_MULTICAST] = route_table_init();
		zvrf->stable[AFI_IP6][SAFI_MULTICAST]->mutex = os_mutex_name_init(os_name_format("%s-umuv6-mutex",zvrf->name));

		zvrf->rnh_table[AFI_IP] = route_table_init();
		zvrf->rnh_table[AFI_IP]->mutex = os_mutex_name_init(os_name_format("%s-rnh-mutex",zvrf->name));
		zvrf->rnh_table[AFI_IP6] = route_table_init();
		zvrf->rnh_table[AFI_IP6]->mutex = os_mutex_name_init(os_name_format("%s-rnhv6-mutex",zvrf->name));

		/* Set VRF ID */
		zvrf->vrf_id = vrf_id;
	}
	return zvrf;
}

static int nsm_vrf_free(struct nsm_ipvrf * zvrf)
{
	if(zvrf)
	{
		if(zvrf->name)
			free(zvrf->name);
		/* Allocate routing table and static table.  */
		nsm_vrf_table_destroy(zvrf, AFI_IP, SAFI_UNICAST);
		nsm_vrf_table_destroy(zvrf, AFI_IP6, SAFI_UNICAST);
		if(zvrf->stable[AFI_IP][SAFI_UNICAST])
		{
			rib_close_table(zvrf->table[AFI_IP][SAFI_UNICAST]);
			if(zvrf->stable[AFI_IP][SAFI_UNICAST]->mutex)
			{
				os_mutex_exit(zvrf->stable[AFI_IP][SAFI_UNICAST]->mutex);
				zvrf->stable[AFI_IP][SAFI_UNICAST]->mutex = NULL;
			}
			route_table_finish(zvrf->stable[AFI_IP][SAFI_UNICAST]);
		}
		if(zvrf->stable[AFI_IP6][SAFI_UNICAST])
		{
			rib_close_table(zvrf->table[AFI_IP6][SAFI_UNICAST]);
			if(zvrf->stable[AFI_IP6][SAFI_UNICAST]->mutex)
			{
				os_mutex_exit(zvrf->stable[AFI_IP6][SAFI_UNICAST]->mutex);
				zvrf->stable[AFI_IP6][SAFI_UNICAST]->mutex = NULL;
			}
			route_table_finish(zvrf->stable[AFI_IP6][SAFI_UNICAST]);
		}

		nsm_vrf_table_destroy(zvrf, AFI_IP, SAFI_MULTICAST);
		nsm_vrf_table_destroy(zvrf, AFI_IP6, SAFI_MULTICAST);

		if(zvrf->stable[AFI_IP][SAFI_MULTICAST])
		{
			rib_close_table(zvrf->table[AFI_IP][SAFI_MULTICAST]);
			if(zvrf->stable[AFI_IP][SAFI_MULTICAST]->mutex)
			{
				os_mutex_exit(zvrf->stable[AFI_IP][SAFI_MULTICAST]->mutex);
				zvrf->stable[AFI_IP][SAFI_MULTICAST]->mutex = NULL;
			}
			route_table_finish(zvrf->stable[AFI_IP][SAFI_MULTICAST]);
		}
		if(zvrf->stable[AFI_IP6][SAFI_MULTICAST])
		{
			rib_close_table(zvrf->table[AFI_IP6][SAFI_MULTICAST]);
			if(zvrf->stable[AFI_IP6][SAFI_MULTICAST]->mutex)
			{
				os_mutex_exit(zvrf->stable[AFI_IP6][SAFI_MULTICAST]->mutex);
				zvrf->stable[AFI_IP6][SAFI_MULTICAST]->mutex = NULL;
			}
			route_table_finish(zvrf->stable[AFI_IP6][SAFI_MULTICAST]);
		}

		if(zvrf->rnh_table[AFI_IP])
		{
			rib_close_table(zvrf->rnh_table[AFI_IP]);
			if(zvrf->rnh_table[AFI_IP]->mutex)
			{
				os_mutex_exit(zvrf->rnh_table[AFI_IP]->mutex);
				zvrf->rnh_table[AFI_IP]->mutex = NULL;
			}
			route_table_finish(zvrf->rnh_table[AFI_IP]);
		}
		if(zvrf->rnh_table[AFI_IP6])
		{
			rib_close_table(zvrf->rnh_table[AFI_IP6]);
			if(zvrf->rnh_table[AFI_IP6]->mutex)
			{
				os_mutex_exit(zvrf->rnh_table[AFI_IP6]->mutex);
				zvrf->rnh_table[AFI_IP6]->mutex = NULL;
			}
			route_table_finish(zvrf->rnh_table[AFI_IP6]);
		}
	}
	return 0;
}
/* Lookup the routing table in an enabled VRF. */
struct route_table * nsm_vrf_table(afi_t afi, safi_t safi, vrf_id_t vrf_id)
{
	struct nsm_ipvrf *zvrf = ip_vrf_info_lookup(vrf_id);

	if (!zvrf)
		return NULL;

	if (afi >= AFI_MAX || safi >= SAFI_MAX)
		return NULL;

	return zvrf->table[afi][safi];
}

/* Lookup the static routing table in a VRF. */
struct route_table * nsm_vrf_static_table(afi_t afi, safi_t safi, vrf_id_t vrf_id)
{
	struct nsm_ipvrf *zvrf = ip_vrf_info_lookup(vrf_id);

	if (!zvrf)
		return NULL;

	if (afi >= AFI_MAX || safi >= SAFI_MAX)
		return NULL;

	return zvrf->stable[afi][safi];
}

/* Callback upon creating a new VRF. */
struct nsm_ipvrf * nsm_vrf_create(const char *name, vrf_id_t vrf_id)
{
    struct nsm_ipvrf *info = nsm_vrf_alloc(name, vrf_id);
	if(info)
	{
    	router_id_init(info);
#if defined(ZPL_NSM_RTADV)
		nsm_rtadv_init(info);
#endif
		return info;
	}
	return NULL;
}

int nsm_vrf_destroy(struct nsm_ipvrf *ipvrf)
{
  	assert(ipvrf);
  	if (ipvrf)
  	{
#if defined (ZPL_NSM_RTADV)
  		nsm_rtadv_terminate (ipvrf);
#endif
		nsm_vrf_free(ipvrf);
		list_delete_all_node (ipvrf->rid_all_sorted_list);
		list_delete_all_node (ipvrf->rid_lo_sorted_list);		
  	}
  	return 0;
}

#if 0
/* Callback upon enabling a VRF. */
int nsm_vrf_enable(struct nsm_ipvrf *zvrf)
{
  	assert(zvrf);

	//nsm_halpal_create_vrf(struct ip_vrf *vrf);
	//nsm_halpal_delete_vrf(struct ip_vrf *vrf);	
#if defined(ZPL_NSM_RTADV)
	nsm_rtadv_init(zvrf);
#endif

  return 0;
}

/* Callback upon disabling a VRF. */
int nsm_vrf_disable(struct nsm_ipvrf *zvrf)
{
  	assert(zvrf);

  	rib_close_table(zvrf->table[AFI_IP][SAFI_UNICAST]);
  	rib_close_table(zvrf->table[AFI_IP6][SAFI_UNICAST]);

  	list_delete_all_node(zvrf->rid_all_sorted_list);
  	list_delete_all_node(zvrf->rid_lo_sorted_list);
	  
  	return 0;
}
#endif


/* Routing information base initialize. */
void rib_init(void)
{
	if(nsm_ribd.initialise == 0)
	{
		memset(&nsm_ribd, 0, sizeof(nsm_ribd));
		if(nsm_ribd.master == NULL)
			nsm_ribd.master = thread_master_module_create(MODULE_RIB);
		if(nsm_ribd.master)	
		{
			nsm_ribd.initialise = 1;
			rib_queue_init(&nsm_ribd);
		}
	}
}