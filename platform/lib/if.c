/* 
 * Interface functions.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 * 
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
//#include "table.h"
#include "vector.h"
#include "nsm_vrf.h"
#include "nsm_connected.h"
#include <net/if_arp.h>

static void *ifMutex = NULL;
static struct list *intfList = NULL;

struct if_master
{
	int (*interface_add_cb)(struct interface *);
	int (*interface_delete_cb)(struct interface *);
	int llc;
	int mode;
};

struct if_master if_master =
	{
		.interface_add_cb = NULL,
		.interface_delete_cb = NULL,
};

int if_data_lock()
{
	if (ifMutex)
		os_mutex_lock(ifMutex, OS_WAIT_FOREVER);
	return OK;
}

int if_data_unlock()
{
	if (ifMutex)
		os_mutex_unlock(ifMutex);
	return OK;
}

struct list *if_list_get()
{
	return intfList;
}

int if_hook_add(int (*add_cb)(struct interface *), int (*del_cb)(struct interface *))
{
	if_master.interface_add_cb = add_cb;
	if_master.interface_delete_cb = del_cb;
	return OK;
}

int if_new_llc_type_mode(int llc, int mode)
{
	if_master.llc = llc;
	if_master.mode = mode;
	return OK;
}

/* Compare interface names, returning an integer greater than, equal to, or
 * less than 0, (following the strcmp convention), according to the
 * relationship between ifp1 and ifp2.  Interface names consist of an
 * alphabetic prefix and a numeric suffix.  The primary sort key is
 * lexicographic by name, and then numeric by number.  No number sorts
 * before all numbers.  Examples: de0 < de1, de100 < fxp0 < xl0, devpty <
 * devpty0, de0 < del0
 */
int if_cmp_func(struct interface *ifp1, struct interface *ifp2)
{
	unsigned int l1, l2;
	long int x1, x2;
	char *p1, *p2;
	int res;

	p1 = ifp1->name;
	p2 = ifp2->name;

	while (*p1 && *p2)
	{
		/* look up to any number */
		l1 = strcspn(p1, "0123456789");
		l2 = strcspn(p2, "0123456789");

		/* name lengths are different -> compare names */
		if (l1 != l2)
		{
			/*
			 * loopback
			 * serial
			 * ethernet
			 * brigde
			 * lag
			 * tunnel
			 * vlan
			 */
			if (ifp1->if_type < ifp2->if_type)
				return -1;
			else if (ifp1->if_type > ifp2->if_type)
				return 1;
			/*			if(strstr(p1,"loop") && !strstr(p2,"loop"))
				return -1;
			if(strstr(p1,"serial") && !strstr(p2,"serial"))
				return -1;
			if(strstr(p2,"loop") && !strstr(p1,"loop"))
				return 1;
			if(strstr(p2,"serial") && !strstr(p1,"serial"))
				return 1;*/

			return (strcmp(p1, p2));
		}

		/* Note that this relies on all numbers being less than all letters, so
		 * that de0 < del0.
		 */
		res = strncmp(p1, p2, l1);

		/* names are different -> compare them */
		if (res)
			return res;

		/* with identical name part, go to numeric part */
		p1 += l1;
		p2 += l1;

		if (!*p1)
			return -1;
		if (!*p2)
			return 1;

		x1 = strtol(p1, &p1, 10);
		x2 = strtol(p2, &p2, 10);

		/* let's compare numbers now */
		if (x1 < x2)
			return -1;
		if (x1 > x2)
			return 1;

		/* numbers were equal, lets do it again..
		 (it happens with name like "eth123.456:789") */
	}
	if (*p1)
		return 1;
	if (*p2)
		return -1;
	return 0;
}

/* Create new interface structure. */

static int if_create_make_ifname(struct interface *ifp, const char *name, int namelen)
{
	if (ifp->if_type == IF_SERIAL ||
		ifp->if_type == IF_ETHERNET ||
		ifp->if_type == IF_GIGABT_ETHERNET ||
		ifp->if_type == IF_TUNNEL ||
		ifp->if_type == IF_WIRELESS ||
#ifdef CUSTOM_INTERFACE
		ifp->if_type == IF_WIFI ||
		ifp->if_type == IF_MODEM ||
#endif
		ifp->if_type == IF_BRIGDE)
	{
		if (os_strstr(name, " ") == NULL)
		{
			if (if_ifname_split(name))
				strcpy(ifp->name, if_ifname_split(name));
		}
		else
			strncpy(ifp->name, name, namelen);
	}
	else if (ifp->if_type == IF_VLAN || ifp->if_type == IF_LAG || ifp->if_type == IF_LOOPBACK)
	{
		strncpy(ifp->name, name, namelen);
	}
	else
		return ERROR;
	return OK;
}

int if_make_llc_type(struct interface *ifp)
{
	switch (ifp->if_type)
	{
	case IF_SERIAL:
		ifp->ll_type = ZEBRA_LLT_SERIAL;
		break;
	case IF_ETHERNET:
		ifp->ll_type = ZEBRA_LLT_ETHER;
		break;
	case IF_GIGABT_ETHERNET:
		ifp->ll_type = ZEBRA_LLT_GIETHER;
		break;
	case IF_WIRELESS:
		ifp->ll_type = ZEBRA_LLT_WIRELESS;
		ifp->if_mode = IF_MODE_L3;
		break;

	case IF_TUNNEL:
		ifp->ll_type = ZEBRA_LLT_TUNNEL;
		ifp->if_mode = IF_MODE_L3;
		break;
	case IF_VLAN:
		ifp->ll_type = ZEBRA_LLT_VLAN;
		ifp->if_mode = IF_MODE_L3;
		break;
	case IF_LAG:
		ifp->ll_type = ZEBRA_LLT_LAG;
		break;
	case IF_LOOPBACK:
		ifp->ll_type = ZEBRA_LLT_LOOPBACK;
		ifp->if_mode = IF_MODE_L3;
		ifp->flags |= IFF_LOOPBACK;
		break;

	case IF_BRIGDE:
		ifp->ll_type = ZEBRA_LLT_BRIGDE;
		ifp->if_mode = IF_MODE_L3;
		break;
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:
		ifp->ll_type = ZEBRA_LLT_WIFI;
		ifp->if_mode = IF_MODE_L3;
		break;
	case IF_MODEM:
		ifp->ll_type = ZEBRA_LLT_MODEM;
		ifp->if_mode = IF_MODE_L3;
		break;
#endif
	default:
		break;
	}
	if (if_master.llc)
	{
		ifp->ll_type = if_master.llc;
		if_master.llc = 0;
	}
	if (if_master.mode)
	{
		ifp->if_mode = if_master.mode;
		if_master.mode = 0;
	}
	return OK;
}

static int if_make_ifindex_type(struct interface *ifp)
{
	ifp->name_hash = if_name_hash_make(ifp->name);

	ifp->ifindex = IF_IFINDEX_SET(ifp->if_type, ifp->uspv);

	if (ifp->if_type == IF_VLAN || ifp->if_type == IF_LAG || ifp->if_type == IF_LOOPBACK)
	{
		ifp->ifindex |= if_loopback_ifindex_create(ifp->if_type, ifp->name);
		//fprintf(stderr, "%s (%s):ifindex=0x%x", __func__,ifp->name, ifp->ifindex);
	}

	if ((ifp->if_type == IF_ETHERNET ||
		 ifp->if_type == IF_GIGABT_ETHERNET) &&
		(IF_IS_SUBIF_GET(ifp->ifindex)))
		ifp->encavlan = IF_IFINDEX_ID_GET(ifp->ifindex);
	return OK;
}

struct interface *
if_create_vrf_dynamic(const char *name, int namelen, vrf_id_t vrf_id)
{
	struct interface *ifp;
	struct list *intf_list = intfList;
	IF_DATA_LOCK();
	ifp = XCALLOC(MTYPE_IF, sizeof(struct interface));
	ifp->ifindex = IFINDEX_INTERNAL;

	assert(name);
	assert(namelen <= INTERFACE_NAMSIZ); /* Need space for '\0' at end. */
	ifp->if_type = if_iftype_make(name);

	ifp->dynamic = TRUE;

	if (if_create_make_ifname(ifp, name, namelen) != OK)
	{
		zlog_err(MODULE_DEFAULT, "if_create(%s): corruption detected  %u!", name, namelen);

		XFREE(MTYPE_IF, ifp);
		IF_DATA_UNLOCK();
		return NULL;
	}
	if (if_uspv_type_setting(ifp) == ERROR)
	{
		zlog_err(MODULE_DEFAULT, "if_create(%s): corruption detected -- interface with this "
						   "uspv in VRF %u!",
				 ifp->name, vrf_id);
		XFREE(MTYPE_IF, ifp);
		IF_DATA_UNLOCK();
		return NULL;
	}

	ifp->name[namelen] = '\0';
	ifp->vrf_id = vrf_id;

	ifp->connected = list_new();
	ifp->connected->del = (void (*)(void *))connected_free;

	if_make_ifindex_type(ifp);

	SET_FLAG(ifp->status, ZEBRA_INTERFACE_ACTIVE);

	ifp->mtu = IF_MTU_DEFAULT;
	ifp->mtu6 = IF_MTU_DEFAULT;
#ifdef USE_IPSTACK_KERNEL
	ifp->if_mode = IF_MODE_L3;
#else
	ifp->if_mode = IF_MODE_ACCESS_L2;
#endif
	ifp->flags |= IFF_UP | IFF_RUNNING;

	if_make_llc_type(ifp);

	if (if_lookup_by_name_vrf(ifp->name, vrf_id) == NULL)
		listnode_add_sort(intf_list, ifp);
	else
		zlog_err(MODULE_DEFAULT, "if_create(%s): corruption detected -- interface with this "
						   "name exists already in VRF %u!",
				 ifp->name, vrf_id);

	if (if_master.interface_add_cb)
		if_master.interface_add_cb(ifp);
	IF_DATA_UNLOCK();
	return ifp;
}

struct interface *
if_create_vrf(const char *name, int namelen, vrf_id_t vrf_id)
{
	struct interface *ifp;
	struct list *intf_list = intfList;
	IF_DATA_LOCK();
	ifp = XCALLOC(MTYPE_IF, sizeof(struct interface));
	ifp->ifindex = IFINDEX_INTERNAL;

	assert(name);
	assert(namelen <= INTERFACE_NAMSIZ); /* Need space for '\0' at end. */
	ifp->if_type = if_iftype_make(name);

	ifp->dynamic = FALSE;

	if (if_create_make_ifname(ifp, name, namelen) != OK)
	{
		XFREE(MTYPE_IF, ifp);
		IF_DATA_UNLOCK();
		return NULL;
	}
	if (if_uspv_type_setting(ifp) == ERROR)
	{
		zlog_err(MODULE_DEFAULT, "if_create(%s): corruption detected -- interface with this "
						   "uspv in VRF %u!",
				 ifp->name, vrf_id);
		XFREE(MTYPE_IF, ifp);
		IF_DATA_UNLOCK();
		return NULL;
	}

	ifp->name[namelen] = '\0';
	ifp->vrf_id = vrf_id;

	ifp->connected = list_new();
	ifp->connected->del = (void (*)(void *))connected_free;

	if_make_ifindex_type(ifp);

	SET_FLAG(ifp->status, ZEBRA_INTERFACE_ACTIVE);

	ifp->mtu = IF_MTU_DEFAULT;
	ifp->mtu6 = IF_MTU_DEFAULT;
#ifdef USE_IPSTACK_KERNEL
	ifp->if_mode = IF_MODE_L3;
#else
	ifp->if_mode = IF_MODE_ACCESS_L2;
#endif
	ifp->flags |= IFF_UP | IFF_RUNNING;

	if_make_llc_type(ifp);

	if (if_lookup_by_name_vrf(ifp->name, vrf_id) == NULL)
		listnode_add_sort(intf_list, ifp);
	else
		zlog_err(MODULE_DEFAULT, "if_create(%s): corruption detected -- interface with this "
						   "name exists already in VRF %u!",
				 ifp->name, vrf_id);

	if (if_master.interface_add_cb)
		if_master.interface_add_cb(ifp);
	IF_DATA_UNLOCK();
	return ifp;
}

struct interface *
if_create(const char *name, int namelen)
{
	return if_create_vrf(name, namelen, VRF_DEFAULT);
}

struct interface *
if_create_dynamic(const char *name, int namelen)
{
	return if_create_vrf_dynamic(name, namelen, VRF_DEFAULT);
}

/* Delete and free interface structure. */
void if_delete(struct interface *ifp)
{

	IF_DATA_LOCK();
	if (if_master.interface_delete_cb)
		if_master.interface_delete_cb(ifp);

	listnode_delete(intfList, ifp);
	list_delete_all_node(ifp->connected);
	list_free(ifp->connected);
	XFREE(MTYPE_IF, ifp);
	IF_DATA_UNLOCK();
}

/* Interface existance check by index. */
struct interface *
if_lookup_by_index_vrf(ifindex_t ifindex, vrf_id_t vrf_id)
{
	struct listnode *node;
	struct interface *ifp;

	for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
	{
		if ((ifp->ifindex && ifp->ifindex == ifindex) && (ifp->vrf_id == vrf_id))
			return ifp;
	}
	return NULL;
}

struct interface *
if_lookup_by_index(ifindex_t ifindex)
{
	struct listnode *node;
	struct interface *ifp;

	for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
	{
		if (ifp->ifindex && ifp->ifindex == ifindex)
			return ifp;
	}
	return NULL;
}

struct interface *
if_lookup_by_kernel_name_vrf(const char *name, vrf_id_t vrf_id)
{
	struct listnode *node;
	struct interface *ifp;

	if (name)
		for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
		{
			if (ifp->k_name_hash == if_name_hash_make(name) && (ifp->vrf_id == vrf_id))
				return ifp;
		}
	return NULL;
}

struct interface *
if_lookup_by_kernel_name(const char *name)
{
	struct listnode *node;
	struct interface *ifp;

	if (name)
		for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
		{
			if (ifp->k_name_hash == if_name_hash_make(name))
				return ifp;
		}
	return NULL;
	//return if_lookup_by_kernel_name_vrf(name, VRF_DEFAULT);
}

struct interface *
if_lookup_by_kernel_index_vrf(ifindex_t kifindex, vrf_id_t vrf_id)
{
	struct listnode *node;
	struct interface *ifp;
	for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
	{
		if (ifp->k_ifindex && ifp->k_ifindex == kifindex && (ifp->vrf_id == vrf_id))
			return ifp;
	}
	return NULL;
}

struct interface *
if_lookup_by_kernel_index(ifindex_t kifindex)
{
	struct listnode *node;
	struct interface *ifp;
	for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
	{
		if (ifp->k_ifindex && ifp->k_ifindex == kifindex)
			return ifp;
	}
	return NULL;
	//return if_lookup_by_kernel_index_vrf(kifindex, VRF_DEFAULT);
}

const char *ifkernelindex2kernelifname(ifindex_t kifindex)
{
	struct interface *ifp;
	return ((ifp = if_lookup_by_kernel_index(kifindex)) != NULL) ? ifp->k_name : NULL;
}

const char *ifkernelindex2kernelifname_vrf(ifindex_t kifindex, vrf_id_t vrf_id)
{
	struct interface *ifp;
	return ((ifp = if_lookup_by_kernel_index_vrf(kifindex, vrf_id)) != NULL) ? ifp->k_name : NULL;
}

ifindex_t ifname2kernelifindex(const char *ifname)
{
	struct interface *ifp;
	return ((ifp = if_lookup_by_kernel_name(ifname)) != NULL) ? ifp->k_ifindex : IFINDEX_INTERNAL;
}

ifindex_t ifname2kernelifindex_vrf(const char *ifname, vrf_id_t vrf_id)
{
	struct interface *ifp;
	return ((ifp = if_lookup_by_kernel_name_vrf(ifname, vrf_id)) != NULL) ? ifp->k_ifindex : IFINDEX_INTERNAL;
}

ifindex_t ifindex2ifkernel(ifindex_t ifindex)
{
	struct interface *ifp;
	return ((ifp = if_lookup_by_index(ifindex)) != NULL) ? ifp->k_ifindex : IFINDEX_INTERNAL;
}

ifindex_t ifkernel2ifindex(ifindex_t ifkindex)
{
	struct interface *ifp;
	return ((ifp = if_lookup_by_kernel_index(ifkindex)) != NULL) ? ifp->ifindex : IFINDEX_INTERNAL;
}

const char *
ifindex2ifname_vrf(ifindex_t ifindex, vrf_id_t vrf_id)
{
	struct interface *ifp;

	return ((ifp = if_lookup_by_index_vrf(ifindex, vrf_id)) != NULL) ? ifp->name : "unknown";
}

const char *
ifindex2ifname(ifindex_t ifindex)
{
	struct interface *ifp;

	return ((ifp = if_lookup_by_index(ifindex)) != NULL) ? ifp->name : "unknown";
	//return ifindex2ifname_vrf(ifindex, VRF_DEFAULT);
}

ifindex_t ifname2ifindex_vrf(const char *name, vrf_id_t vrf_id)
{
	struct interface *ifp;

	return ((ifp = if_lookup_by_name_vrf(name, vrf_id)) != NULL) ? ifp->ifindex : IFINDEX_INTERNAL;
}

ifindex_t ifname2ifindex(const char *name)
{
	struct interface *ifp;
	return ((ifp = if_lookup_by_name(name)) != NULL) ? ifp->ifindex : IFINDEX_INTERNAL;
}

/* Interface existance check by interface name. */
struct interface *
if_lookup_by_name_vrf(const char *name, vrf_id_t vrf_id)
{
	struct listnode *node;
	struct interface *ifp;

	if (name)
		for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
		{
			if (ifp->name_hash == if_name_hash_make(name) && (ifp->vrf_id == vrf_id))
				return ifp;
		}
	return NULL;
}

struct interface *
if_lookup_by_name(const char *name)
{
	struct listnode *node;
	struct interface *ifp;

	if (name)
		for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
		{
			if (ifp->name_hash == if_name_hash_make(name))
				return ifp;
		}
	return NULL;
	//return if_lookup_by_name_vrf(name, VRF_DEFAULT);
}

struct interface *
if_lookup_by_name_len_vrf(const char *name, size_t namelen, vrf_id_t vrf_id)
{
	struct listnode *node;
	struct interface *ifp;

	if (namelen > INTERFACE_NAMSIZ)
		return NULL;

	for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
	{
		if (ifp->name_hash == if_name_hash_make(name) && (ifp->vrf_id == vrf_id))
			return ifp;
	}
	return NULL;
}

struct interface *
if_lookup_by_name_len(const char *name, size_t namelen)
{
	return if_lookup_by_name(name);
}

unsigned int ifindex2vlan(ifindex_t ifindex)
{

	struct interface *ifp = if_lookup_by_index(ifindex);
	return ifp ? ifp->encavlan : IFINDEX_INTERNAL;
}

struct interface *if_lookup_by_encavlan(unsigned short encavlan)
{
	struct listnode *node;
	struct interface *ifp;
	if (!encavlan)
		return NULL;
	for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
	{
		if ((ifp->encavlan == encavlan))
			return ifp;
	}
	return NULL;
}
/* Lookup interface by IPv4 address. */
struct interface *
if_lookup_exact_address_vrf(struct in_addr src, vrf_id_t vrf_id)
{
	struct listnode *node;
	struct listnode *cnode;
	struct interface *ifp;
	struct prefix *p;
	struct connected *c;

	for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
	{
		if ((ifp->vrf_id == vrf_id))
		{
			for (ALL_LIST_ELEMENTS_RO(ifp->connected, cnode, c))
			{
				p = c->address;

				if (p && p->family == AF_INET)
				{
					if (IPV4_ADDR_SAME(&p->u.prefix4, &src))
						return ifp;
				}
			}
		}
	}
	return NULL;
}

struct interface *
if_lookup_exact_address(struct in_addr src)
{
	return if_lookup_exact_address_vrf(src, VRF_DEFAULT);
}

/* Lookup interface by IPv4 address. */
struct interface *
if_lookup_address_vrf(struct in_addr src, vrf_id_t vrf_id)
{
	struct listnode *node;
	struct prefix addr;
	int bestlen = 0;
	struct listnode *cnode;
	struct interface *ifp;
	struct connected *c;
	struct interface *match;

	addr.family = AF_INET;
	addr.u.prefix4 = src;
	addr.prefixlen = IPV4_MAX_BITLEN;

	match = NULL;

	for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
	{
		if (ifp->vrf_id == vrf_id)
		{
			for (ALL_LIST_ELEMENTS_RO(ifp->connected, cnode, c))
			{
				if (c->address && (c->address->family == AF_INET) && prefix_match(CONNECTED_PREFIX(c), &addr) && (c->address->prefixlen > bestlen))
				{
					bestlen = c->address->prefixlen;
					match = ifp;
				}
			}
		}
	}
	return match;
}

struct interface *
if_lookup_address(struct in_addr src)
{
	return if_lookup_address_vrf(src, VRF_DEFAULT);
}

/* Lookup interface by prefix */
struct interface *
if_lookup_prefix_vrf(struct prefix *prefix, vrf_id_t vrf_id)
{

	struct listnode *node;
	struct listnode *cnode;
	struct interface *ifp;
	struct connected *c;

	for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
	{
		if (ifp->vrf_id == vrf_id)
		{
			for (ALL_LIST_ELEMENTS_RO(ifp->connected, cnode, c))
			{
				if (prefix_cmp(c->address, prefix) == 0)
				{
					return ifp;
				}
			}
		}
	}
	return NULL;
}

struct interface *
if_lookup_prefix(struct prefix *prefix)
{
	return if_lookup_prefix_vrf(prefix, VRF_DEFAULT);
}

int if_count_lookup_type(if_type_t type)
{
	struct listnode *node;
	//struct listnode *cnode;
	struct interface *ifp;
	//struct connected *c;
	int count = 0;
	for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
	{
		if (IF_TYPE_GET(ifp->if_type) == type)
		{
			(count)++;
		}
	}
	return (count);
}
/* Get interface by name if given name interface doesn't exist create one. */

int if_up(struct interface *ifp)
{
	struct connected *connected;
	struct listnode *node;
	struct prefix *p;
#ifdef PL_NSM_MODULE
	/* Notify the protocol daemons. */
	zebra_interface_up_update(ifp);
#endif
	/* Install connected routes to the kernel. */
	for (ALL_LIST_ELEMENTS_RO(ifp->connected, node, connected))
	{
		p = connected->address;

		if (p->family == AF_INET)
			connected_up_ipv4(ifp, connected);
#ifdef HAVE_IPV6
		else if (p->family == AF_INET6)
			connected_up_ipv6(ifp, connected);
#endif /* HAVE_IPV6 */
	}
	ifp->flags |= IFF_UP | IFF_RUNNING;
/* Examine all static routes. */
#ifdef PL_NSM_MODULE
	rib_update(ifp->vrf_id);
#endif
	return OK;
}

int if_down(struct interface *ifp)
{
	struct connected *connected;
	struct listnode *node;
	struct prefix *p;
#ifdef PL_NSM_MODULE
	/* Notify to the protocol daemons. */
	zebra_interface_down_update(ifp);
#endif
	/* Delete connected routes from the kernel. */
	for (ALL_LIST_ELEMENTS_RO(ifp->connected, node, connected))
	{
		p = connected->address;
		if (p->family == AF_INET)
			connected_down_ipv4(ifp, connected);
#ifdef HAVE_IPV6
		else if (p->family == AF_INET6)
			connected_down_ipv6(ifp, connected);
#endif /* HAVE_IPV6 */
	}
	ifp->flags &= ~(IFF_UP | IFF_RUNNING);
/* Examine all static routes which direct to the interface. */
#ifdef PL_NSM_MODULE
	rib_update(ifp->vrf_id);
#endif
	return OK;
}
/* Does interface up ? */
int if_is_up(struct interface *ifp)
{
	return ifp->flags & IFF_UP;
}

/* Is interface running? */
int if_is_running(struct interface *ifp)
{
	return ifp->flags & IFF_RUNNING;
}

/* Is the interface operative, eg. either UP & RUNNING
 or UP & !ZEBRA_INTERFACE_LINK_DETECTION */
int if_is_operative(struct interface *ifp)
{
	return ((ifp->flags & IFF_UP) && (ifp->flags & IFF_RUNNING || !CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION)));
}

/* Is this loopback interface ? */
int if_is_loopback(struct interface *ifp)
{
	/* XXX: Do this better, eg what if IFF_WHATEVER means X on platform M
	 * but Y on platform N?
	 */
	return (ifp->flags & (IFF_LOOPBACK | IFF_NOXMIT | IFF_VIRTUAL));
}

/* Does this interface support broadcast ? */
int if_is_broadcast(struct interface *ifp)
{
	return ifp->flags & IFF_BROADCAST;
}

/* Does this interface support broadcast ? */
int if_is_pointopoint(struct interface *ifp)
{
	return ifp->flags & IFF_POINTOPOINT;
}

/* Does this interface support multicast ? */
int if_is_multicast(struct interface *ifp)
{
	return ifp->flags & IFF_MULTICAST;
}

int if_is_serial(struct interface *ifp)
{
	if (os_strstr(ifp->name, "serial"))
		return 1;
	if (ifp->if_type == IF_SERIAL)
		return 1;
	/*	if(ifp->zebra_link_type == IF_SERIAL)
		return 1;*/
	return 0;
}

int if_is_ethernet(struct interface *ifp)
{
	if (os_strstr(ifp->name, "ethernet"))
		return 1;
	if (ifp->if_type == IF_ETHERNET || ifp->if_type == IF_GIGABT_ETHERNET)
		return 1;
	return 0;
}

int if_is_tunnel(struct interface *ifp)
{
	if (os_strstr(ifp->name, "tunnel"))
		return 1;
	if (ifp->if_type == IF_TUNNEL)
		return 1;
	return 0;
}

int if_is_lag(struct interface *ifp)
{
	if (os_strstr(ifp->name, "port-channel"))
		return 1;
	if (ifp->if_type == IF_LAG)
		return 1;
	return 0;
}

int if_is_lag_member(struct interface *ifp)
{
	return CHECK_FLAG(ifp->ifmember, IF_TRUNK_MEM);
}

int if_is_vlan(struct interface *ifp)
{
	if (os_strstr(ifp->name, "vlan"))
		return 1;
	if (ifp->if_type == IF_VLAN)
		return 1;
	return 0;
}

int if_is_brigde(struct interface *ifp)
{
	if (ifp->if_type == IF_BRIGDE)
		return 1;
	return 0;
}

int if_is_brigde_member(struct interface *ifp)
{
	return CHECK_FLAG(ifp->ifmember, IF_BRIDGE_MEM);
}

int if_is_loop(struct interface *ifp)
{
	if (ifp->if_type == IF_LOOPBACK)
		return 1;
	return 0;
}

int if_is_wireless(struct interface *ifp)
{
	if (os_strstr(ifp->name, "wireless"))
		return 1;
	if (ifp->if_type == IF_WIRELESS)
		return 1;
	return 0;
}

int if_name_set(struct interface *ifp, const char *str)
{
	if (os_strlen(ifp->name) == 0)
		os_strcpy(ifp->name, str);
	ifp->name_hash = if_name_hash_make(ifp->name);
	return OK;
}

int if_kname_set(struct interface *ifp, const char *str)
{
	if (strlen(str))
	{
		char buf[INTERFACE_NAMSIZ + 1];
		os_memset(buf, 0, sizeof(buf));
		os_strcpy(buf, str);
		os_memset(ifp->k_name, 0, sizeof(ifp->k_name));
		os_strcpy(ifp->k_name, buf);
		ifp->k_name_hash = if_name_hash_make(ifp->k_name);
#ifdef PL_PAL_MODULE
		ifp->k_ifindex = pal_interface_ifindex(ifp->k_name);
#endif
	}
	else
	{
		os_memset(ifp->k_name, 0, sizeof(ifp->k_name));
		ifp->k_name_hash = 0;
		ifp->k_ifindex = 0;
	}
	return OK;
}

const char *if_enca_string(if_enca_t enca)
{
	switch (enca)
	{
	case IF_ENCA_NONE:
		return " ";
		break;
	case IF_ENCA_DOT1Q:
		return "dot1q";
		break;
	case IF_ENCA_DOT1Q_TUNNEL:
		return "dot1q-tunnel";
		break;
	case IF_ENCA_PPP:
		return "ppp";
		break;
	case IF_ENCA_PPPOE:
		return "pppoe";
		break;
	case IF_ENCA_SLIP:
		return "slip";
		break;
	case IF_ENCA_GRE:
		return "gre";
		break;
	case IF_ENCA_IPIP:
		return "ipip";
		break;
	case IF_ENCA_IPIPV6:
		return "ipipv6";
		break;
	case IF_ENCA_IPV6IP:
		return "ipv6ip";
		break;
	case IF_ENCA_SIT:
		return "sit";
		break;
	case IF_ENCA_HDLC:
		return "hdlc";
		break;
	case IF_ENCA_RAW:
		return "raw";
		break;
	default:
		break;
	}
	return " ";
}

int if_list_each(int (*cb)(struct interface *ifp, void *pVoid), void *pVoid)
{
	struct listnode *node;
	struct interface *ifp;
	for (ALL_LIST_ELEMENTS_RO(intfList, node, ifp))
	{
		if (ifp)
		{
			if (cb)
			{
				if (OK != (cb)(ifp, pVoid))
					break;
			}
		}
	}
	return OK;
}

/* Allocate connected structure. */
struct connected *
connected_new(void)
{
	return XCALLOC(MTYPE_CONNECTED, sizeof(struct connected));
}

/* Free connected structure. */
void connected_free(struct connected *connected)
{
	if (connected->address)
		prefix_free(connected->address);

	if (connected->destination)
		prefix_free(connected->destination);

	XFREE(MTYPE_CONNECTED, connected);
}

/* If two connected address has same prefix return 1. */
static int connected_same_prefix(struct prefix *p1, struct prefix *p2)
{
	if (p1->family == p2->family)
	{
		if (p1->family == AF_INET &&
			IPV4_ADDR_SAME(&p1->u.prefix4, &p2->u.prefix4))
			return 1;
#ifdef HAVE_IPV6
		if (p1->family == AF_INET6 &&
			IPV6_ADDR_SAME(&p1->u.prefix6, &p2->u.prefix6))
			return 1;
#endif /* HAVE_IPV6 */
	}
	return 0;
}

struct connected *
connected_delete_by_prefix(struct interface *ifp, struct prefix *p)
{
	struct listnode *node;
	struct listnode *next;
	struct connected *ifc;

	/* In case of same prefix come, replace it with new one. */
	for (node = listhead(ifp->connected); node; node = next)
	{
		ifc = listgetdata(node);
		next = node->next;

		if (connected_same_prefix(ifc->address, p))
		{
			listnode_delete(ifp->connected, ifc);
			return ifc;
		}
	}
	return NULL;
}

/* Find the IPv4 address on our side that will be used when packets
 are sent to dst. */
struct connected *
connected_lookup_address(struct interface *ifp, struct in_addr dst)
{
	struct prefix addr;
	struct listnode *cnode;
	struct connected *c;
	struct connected *match;

	addr.family = AF_INET;
	addr.u.prefix4 = dst;
	addr.prefixlen = IPV4_MAX_BITLEN;

	match = NULL;

	for (ALL_LIST_ELEMENTS_RO(ifp->connected, cnode, c))
	{
		if (c->address && (c->address->family == AF_INET) && prefix_match(CONNECTED_PREFIX(c), &addr) && (!match || (c->address->prefixlen > match->address->prefixlen)))
			match = c;
	}
	return match;
}

struct connected *
connected_add_by_prefix(struct interface *ifp, struct prefix *p,
						struct prefix *destination)
{
	struct connected *ifc;

	/* Allocate new connected address. */
	ifc = connected_new();
	ifc->ifp = ifp;

	/* Fetch interface address */
	ifc->address = prefix_new();
	memcpy(ifc->address, p, sizeof(struct prefix));

	/* Fetch dest address */
	if (destination)
	{
		ifc->destination = prefix_new();
		memcpy(ifc->destination, destination, sizeof(struct prefix));
	}

	/* Add connected address to the interface. */
	listnode_add(ifp->connected, ifc);
	return ifc;
}

/* If same interface address is already exist... */
struct connected *
connected_check(struct interface *ifp, struct prefix *p)
{
	struct connected *ifc;
	struct listnode *node;

	for (ALL_LIST_ELEMENTS_RO(ifp->connected, node, ifc))
		if (prefix_same(ifc->address, p))
			return ifc;

	return NULL;
}

/* Print if_addr structure. */
static void __attribute__((unused))
connected_log(struct connected *connected, char *str)
{
	struct prefix *p;
	struct interface *ifp;
	char logbuf[BUFSIZ];
	char buf[BUFSIZ];

	ifp = connected->ifp;
	p = connected->address;

	snprintf(logbuf, BUFSIZ, "%s interface %s vrf %u %s %s/%d ", str, ifp->name,
			 ifp->vrf_id, prefix_family_str(p),
			 inet_ntop(p->family, &p->u.prefix, buf, BUFSIZ), p->prefixlen);

	p = connected->destination;
	if (p)
	{
		strncat(logbuf, inet_ntop(p->family, &p->u.prefix, buf, BUFSIZ),
				BUFSIZ - strlen(logbuf));
	}
	zlog_info(MODULE_DEFAULT, "%s", logbuf);
}

/* Printout flag information into log */
const char *
if_flag_dump(unsigned long flag)
{
	int separator = 0;
	static char logbuf[BUFSIZ];

#define IFF_OUT_LOG(X, STR)               \
	if (flag & (X))                       \
	{                                     \
		if (separator)                    \
			strlcat(logbuf, ",", BUFSIZ); \
		else                              \
			separator = 1;                \
		strlcat(logbuf, STR, BUFSIZ);     \
	}

	strlcpy(logbuf, "<", BUFSIZ);
	IFF_OUT_LOG(IFF_UP, "UP");
	IFF_OUT_LOG(IFF_BROADCAST, "BROADCAST");
	IFF_OUT_LOG(IFF_DEBUG, "DEBUG");
	IFF_OUT_LOG(IFF_LOOPBACK, "LOOPBACK");
	IFF_OUT_LOG(IFF_POINTOPOINT, "POINTOPOINT");
	IFF_OUT_LOG(IFF_NOTRAILERS, "NOTRAILERS");
	IFF_OUT_LOG(IFF_RUNNING, "RUNNING");
	IFF_OUT_LOG(IFF_NOARP, "NOARP");
	IFF_OUT_LOG(IFF_PROMISC, "PROMISC");
	IFF_OUT_LOG(IFF_ALLMULTI, "ALLMULTI");
	IFF_OUT_LOG(IFF_OACTIVE, "OACTIVE");
	IFF_OUT_LOG(IFF_SIMPLEX, "SIMPLEX");
	IFF_OUT_LOG(IFF_LINK0, "LINK0");
	IFF_OUT_LOG(IFF_LINK1, "LINK1");
	IFF_OUT_LOG(IFF_LINK2, "LINK2");
	IFF_OUT_LOG(IFF_MULTICAST, "MULTICAST");
	IFF_OUT_LOG(IFF_NOXMIT, "NOXMIT");
	IFF_OUT_LOG(IFF_NORTEXCH, "NORTEXCH");
	IFF_OUT_LOG(IFF_VIRTUAL, "VIRTUAL");
	IFF_OUT_LOG(IFF_IPV4, "IPv4");
	IFF_OUT_LOG(IFF_IPV6, "IPv6");

	strlcat(logbuf, ">", BUFSIZ);

	return logbuf;
#undef IFF_OUT_LOG
}

/* For debugging */
static void if_dump(const struct interface *ifp)
{
	struct listnode *node;
	struct connected *c __attribute__((unused));

	for (ALL_LIST_ELEMENTS_RO(ifp->connected, node, c))
		zlog_info(MODULE_DEFAULT, "Interface %s vrf %u index %d metric %d mtu %d "
#ifdef HAVE_IPV6
							"mtu6 %d "
#endif /* HAVE_IPV6 */
							"%s",
				  ifp->name, ifp->vrf_id, ifp->ifindex, ifp->metric,
				  ifp->mtu,
#ifdef HAVE_IPV6
				  ifp->mtu6,
#endif /* HAVE_IPV6 */
				  if_flag_dump(ifp->flags));
}

/* Interface printing for all interface. */
void if_dump_all(void)
{
	struct listnode *node;
	void *p;
	for (ALL_LIST_ELEMENTS_RO(intfList, node, p))
		if_dump(p);
}

/* Initialize interface list. */
void if_init()
{
	if(intfList == NULL)
	{
		intfList = list_new();
		if (ifMutex == NULL)
			ifMutex = os_mutex_init();
		(intfList)->cmp = (int (*)(void *, void *))if_cmp_func;
	}
}

void if_terminate()
{
	for (;;)
	{
		struct interface *ifp;

		ifp = listnode_head(intfList);
		if (ifp == NULL)
			break;

		if_delete(ifp);
	}

	list_delete(intfList);
	intfList = NULL;
}

const char *
if_link_type_str(enum zebra_link_type llt)
{
	switch (llt)
	{
#define llts(T, S) \
	case (T):      \
		return (S)
		llts(ZEBRA_LLT_UNKNOWN, "Unknown");
		llts(ZEBRA_LLT_SERIAL, "Serial");
		llts(ZEBRA_LLT_ETHER, "Ethernet");
		llts(ZEBRA_LLT_GIETHER, "Gigabt-Ethernet");
		llts(ZEBRA_LLT_TUNNEL, "Tunnel");
		llts(ZEBRA_LLT_VLAN, "Vlan");
		llts(ZEBRA_LLT_LAG, "Lag");
		llts(ZEBRA_LLT_ATM, "ATM");
		llts(ZEBRA_LLT_SLIP, "SLIP");
		llts(ZEBRA_LLT_CSLIP, "Compressed SLIP");
		llts(ZEBRA_LLT_SLIP6, "SLIPv6");
		llts(ZEBRA_LLT_CSLIP6, "Compressed SLIPv6");
		llts(ZEBRA_LLT_PPP, "PPP");
		llts(ZEBRA_LLT_CHDLC, "Cisco HDLC");
		llts(ZEBRA_LLT_RAWHDLC, "Raw HDLC");
		llts(ZEBRA_LLT_IPIP, "IPIP Tunnel");
		llts(ZEBRA_LLT_IPIP6, "IPIP6 Tunnel");
		llts(ZEBRA_LLT_LOOPBACK, "Loopback");
		llts(ZEBRA_LLT_SIT, "IPv6-in-IPv4 SIT");
		llts(ZEBRA_LLT_IPGRE, "GRE over IP");
		llts(ZEBRA_LLT_IP6GRE, "GRE over IPV6");
		llts(ZEBRA_LLT_BRIGDE, "Brigde");
		llts(ZEBRA_LLT_WIFI, "Wifi");
		llts(ZEBRA_LLT_MODEM, "Modem");
		llts(ZEBRA_LLT_WIRELESS, "Wireless");
	default:
		zlog_warn(MODULE_DEFAULT, "Unknown value %d", llt);
		return "Unknown type!";
#undef llts
	}
	return NULL;
}

enum zebra_link_type
netlink_to_zebra_link_type(unsigned int hwt)
{
	switch (hwt)
	{
	case ARPHRD_ETHER:
		return ZEBRA_LLT_ETHER;
	case ARPHRD_ATM:
		return ZEBRA_LLT_ATM;
	case ARPHRD_SLIP:
		return ZEBRA_LLT_SLIP;
	case ARPHRD_CSLIP:
		return ZEBRA_LLT_CSLIP;
	case ARPHRD_SLIP6:
		return ZEBRA_LLT_SLIP6;
	case ARPHRD_CSLIP6:
		return ZEBRA_LLT_CSLIP6;
	case ARPHRD_PPP:
		return ZEBRA_LLT_PPP;
	case ARPHRD_CISCO:
		return ZEBRA_LLT_CHDLC;
	case ARPHRD_RAWHDLC:
		return ZEBRA_LLT_RAWHDLC;
	case ARPHRD_TUNNEL:
		return ZEBRA_LLT_IPIP;
	case ARPHRD_TUNNEL6:
		return ZEBRA_LLT_IPIP6;
	case ARPHRD_LOOPBACK:
		return ZEBRA_LLT_LOOPBACK;
	case ARPHRD_SIT:
		return ZEBRA_LLT_SIT;
	case ARPHRD_IPGRE:
		return ZEBRA_LLT_IPGRE;

		// case ARPHRD_IEEE802: return ZEBRA_LLT_IPGRE;
	case ARPHRD_IEEE80211:
		return ZEBRA_LLT_WIRELESS;
		// case ARPHRD_IEEE802154: return ZEBRA_LLT_ZIGBEE;

#ifdef ARPHRD_IP6GRE
	case ARPHRD_IP6GRE:
		return ZEBRA_LLT_IP6GRE;
#endif
	default:
		return ZEBRA_LLT_UNKNOWN;
	}
}
