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
#include "auto_include.h"
#include "zpl_type.h"
#include "os_sem.h"
#include "module.h"
#include "zmemory.h"
#include "str.h"
#include "hash.h"
#include "linklist.h"
#include "prefix.h"
#include "if.h"
#include "if_utsp.h"

#include "log.h"
#ifdef ZPL_VRF_MODULE
#include "ipvrf.h"
#endif





static struct if_master _zpl_if_master_lst;

int if_data_lock(void)
{
	if (_zpl_if_master_lst.ifdatamutex)
		os_mutex_lock(_zpl_if_master_lst.ifdatamutex, OS_WAIT_FOREVER);
	return OK;
}

int if_data_unlock(void)
{
	if (_zpl_if_master_lst.ifdatamutex)
		os_mutex_unlock(_zpl_if_master_lst.ifdatamutex);
	return OK;
}

int if_master_lock(void)
{
	if (_zpl_if_master_lst.ifMutex)
		os_mutex_lock(_zpl_if_master_lst.ifMutex, OS_WAIT_FOREVER);
	return OK;
}

int if_master_unlock(void)
{
	if (_zpl_if_master_lst.ifMutex)
		os_mutex_unlock(_zpl_if_master_lst.ifMutex);
	return OK;
}

struct list *if_list_get(void)
{
	return _zpl_if_master_lst.intfList;
}

int if_hook_add(int (*add_cb)(struct interface *), int (*del_cb)(struct interface *))
{
	_zpl_if_master_lst.if_master_add_cb = add_cb;
	_zpl_if_master_lst.if_master_del_cb = del_cb;
	return OK;
}

int if_new_llc_type_mode(zpl_uint32 llc, zpl_uint32 mode)
{
	_zpl_if_master_lst.llc = llc;
	_zpl_if_master_lst.mode = mode;
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
	zpl_uint32  l1 = 0, l2 = 0;
	zpl_uint32 x1 = 0, x2 = 0;
	zpl_char *p1 = NULL, *p2 = NULL;
	int res = 0;

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
static int if_ifname_setup(struct interface *ifp, const char *name, zpl_uint32 namelen)
{
	if (ifp->if_type == IF_SERIAL ||
		ifp->if_type == IF_ETHERNET ||
		ifp->if_type == IF_GIGABT_ETHERNET ||
		ifp->if_type == IF_XGIGABT_ETHERNET ||
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
		ifp->ll_type = IF_LLT_SERIAL;
		break;
	case IF_ETHERNET:
		ifp->ll_type = IF_LLT_ETHER;
		break;
	case IF_GIGABT_ETHERNET:
	case IF_XGIGABT_ETHERNET:
		ifp->ll_type = IF_LLT_GIETHER;
		break;
	case IF_WIRELESS:
		ifp->ll_type = IF_LLT_WIRELESS;
		break;
	case IF_TUNNEL:
		ifp->ll_type = IF_LLT_TUNNEL;
		break;
	case IF_VLAN:
		ifp->ll_type = IF_LLT_VLAN;
		break;
	case IF_LAG:
		ifp->ll_type = IF_LLT_LAG;
		break;
	case IF_LOOPBACK:
		ifp->ll_type = IF_LLT_LOOPBACK;
		break;

	case IF_BRIGDE:
		ifp->ll_type = IF_LLT_BRIGDE;
		break;
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:
		ifp->ll_type = IF_LLT_WIFI;
		break;
	case IF_MODEM:
		ifp->ll_type = IF_LLT_MODEM;
		break;
#endif
	default:
		break;
	}
	if (_zpl_if_master_lst.llc)
	{
		ifp->ll_type = _zpl_if_master_lst.llc;
		_zpl_if_master_lst.llc = 0;
	}
	if (_zpl_if_master_lst.mode)
	{
		ifp->if_mode = _zpl_if_master_lst.mode;
		_zpl_if_master_lst.mode = 0;
	}
	return OK;
}


static int if_mode_default(struct interface *ifp)
{
	switch (ifp->if_type)
	{
	case IF_SERIAL:
		ifp->if_mode = IF_MODE_L3;
		ifp->have_kernel = zpl_true; 
		break;
	case IF_ETHERNET:
		ifp->if_mode = IF_MODE_ACCESS_L2;
		break;
	case IF_GIGABT_ETHERNET:
	case IF_XGIGABT_ETHERNET:
		ifp->if_mode = IF_MODE_ACCESS_L2;
		break;
	case IF_WIRELESS:
		ifp->if_mode = IF_MODE_L3;
		ifp->have_kernel = zpl_true; 
		break;
	case IF_TUNNEL:
		ifp->if_mode = IF_MODE_L3;
		ifp->have_kernel = zpl_true; 
		break;
	case IF_VLAN:
		ifp->if_mode = IF_MODE_L3;
		ifp->have_kernel = zpl_true; 
		break;
	case IF_LAG:
		ifp->if_mode = IF_MODE_ACCESS_L2;
		break;
	case IF_LOOPBACK:
		ifp->if_mode = IF_MODE_L3;
		ifp->flags |= IPSTACK_IFF_LOOPBACK;
		ifp->have_kernel = zpl_true; 
		break;

	case IF_BRIGDE:
		ifp->if_mode = IF_MODE_ACCESS_L2;
		break;
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:
		ifp->if_mode = IF_MODE_L3;
		ifp->have_kernel = zpl_true; 
		break;
	case IF_MODEM:
		ifp->if_mode = IF_MODE_L3;
		ifp->have_kernel = zpl_true; 
		break;
#endif
	default:
		break;
	}
	return OK;
}

static int if_ifindex_setup(struct interface *ifp)
{
	ifp->name_hash = if_name_hash_make(ifp->name);

	ifp->ifindex = IF_IFINDEX_SET(ifp->if_type, ifp->uspv);

	ifp->phyid = IFPHYID_INTERNAL;
	
	if (ifp->if_type == IF_VLAN || ifp->if_type == IF_LAG || ifp->if_type == IF_LOOPBACK)
	{
		ifp->ifindex |= if_loopback_ifindex_create(ifp->if_type, ifp->name);
	}
	
	return OK;
}
#ifdef ZPL_IPCOM_MODULE
int if_kernelname_set(struct interface *ifp)
{
	switch (ifp->if_type)
	{
	case IF_SERIAL:
	case IF_ETHERNET:
	case IF_GIGABT_ETHERNET:
	case IF_XGIGABT_ETHERNET:
	case IF_TUNNEL:
	case IF_BRIGDE:
	case IF_WIRELESS:
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:
	case IF_MODEM:
#endif
		if (IF_IFINDEX_ID_GET(ifp->ifindex))
			sprintf(ifp->ker_name, "%s%d/%d/%d.%d", getkernelname(ifp->if_type),
					IF_IFINDEX_UNIT_GET(ifp->ifindex),
					IF_IFINDEX_SLOT_GET(ifp->ifindex),
					IF_IFINDEX_PORT_GET(ifp->ifindex),
					IF_IFINDEX_ID_GET(ifp->ifindex));
		else
			sprintf(ifp->ker_name, "%s%d/%d/%d", getkernelname(ifp->if_type),
					IF_IFINDEX_UNIT_GET(ifp->ifindex),
					IF_IFINDEX_SLOT_GET(ifp->ifindex),
					IF_IFINDEX_PORT_GET(ifp->ifindex));
		break;
	case IF_LOOPBACK:
	case IF_VLAN:
	case IF_LAG:
		if (IF_IFINDEX_ID_GET(ifp->ifindex))
			sprintf(ifp->ker_name, "%s%d", getkernelname(ifp->if_type),
					IF_IFINDEX_ID_GET(ifp->ifindex));
		else
			sprintf(ifp->ker_name, "%s%d", getkernelname(ifp->if_type),
					IF_IFINDEX_PORT_GET(ifp->ifindex));
		break;
	default:
		break;
	}
	ifp->ker_name_hash = if_name_hash_make(ifp->ker_name);
	return OK;
}
#else
int if_kernelname_set(struct interface *ifp)
{
	zpl_char ker_name[64];
	os_memset(ker_name, 0, sizeof(ker_name));
	switch (ifp->if_type)
	{
#if defined(ZPL_NSM_TUNNEL)||defined(ZPL_NSM_SERIAL)||defined(ZPL_NSM_BRIGDE)		
	case IF_SERIAL:
	case IF_TUNNEL:
	case IF_BRIGDE:	
		sprintf(ker_name, "%s%d%d", getkernelname(ifp->if_type),
				IF_IFINDEX_SLOT_GET(ifp->ifindex), IF_IFINDEX_PORT_GET(ifp->ifindex));
		break;
#endif		
	case IF_ETHERNET:
	case IF_GIGABT_ETHERNET:
	case IF_XGIGABT_ETHERNET:
	case IF_WIRELESS:
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:
	case IF_MODEM:
#endif
		if (!IF_IFINDEX_ID_GET(ifp->ifindex))
			sprintf(ker_name, "%s%d%d", getkernelname(ifp->if_type),
					IF_IFINDEX_SLOT_GET(ifp->ifindex), IF_IFINDEX_PORT_GET(ifp->ifindex));
		else
		{
			ifkernindex_t root_kifindex = ifindex2ifkernel(IF_IFINDEX_ROOT_GET(ifp->ifindex));
			const char *root_kname = ifkernelindex2kernelifname(root_kifindex);
			if (root_kname)
				sprintf(ker_name, "%s.%d", root_kname,
						IF_IFINDEX_VLAN_GET(ifp->ifindex));
			else
				sprintf(ker_name, "%s%d%d.%d", getkernelname(ifp->if_type),
						IF_IFINDEX_SLOT_GET(ifp->ifindex), IF_IFINDEX_PORT_GET(ifp->ifindex),
						IF_IFINDEX_VLAN_GET(ifp->ifindex));
		}

		break;
	case IF_LOOPBACK:
	case IF_VLAN:	
	case IF_LAG:
#if defined(ZPL_NSM_TRUNK)||defined(ZPL_NSM_VLAN)
		if (IF_IFINDEX_ID_GET(ifp->ifindex))
			sprintf(ker_name, "%s%d", getkernelname(ifp->if_type),
					IF_IFINDEX_ID_GET(ifp->ifindex));
		else
			sprintf(ker_name, "%s%d", getkernelname(ifp->if_type),
					IF_IFINDEX_PORT_GET(ifp->ifindex));
		break;
#endif
	default:
		break;
	}
	if (os_strlen(ker_name))
		if_kname_set(ifp, ker_name);
	return OK;
}
#endif


zpl_bool if_can_create(struct interface *ifp)
{
	if_type_t type = IF_TYPE_GET(ifp->ifindex);
	switch (type)
	{
	case IF_ETHERNET:
	case IF_GIGABT_ETHERNET:
	case IF_XGIGABT_ETHERNET:
	case IF_WIRELESS:
		if (IF_ID_GET(ifp->ifindex))
		{
			return zpl_true;
		}
		else
		{
			return zpl_false;
		}
		break;
#ifdef ZPL_NSM_BRIGDE
	case IF_BRIGDE:
		if (IF_BRIGDE_MAX > if_count_lookup_type(IF_BRIGDE))
		{
			return zpl_true;
		}
		break;
#endif
#ifdef ZPL_NSM_SERIAL
	case IF_SERIAL:
		return zpl_false;
		break;
#endif		
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:
	case IF_MODEM:
		return zpl_false;
		break;
#endif
#ifdef ZPL_NSM_TUNNEL
	case IF_TUNNEL:
		if (IF_SLOT_GET(ifp->ifindex) != IF_TUNNEL_SLOT)
		{
			return zpl_false;
		}
		if (if_count_lookup_type(IF_TUNNEL) >= IF_TUNNEL_MAX)
		{
			return zpl_false;
		}
		return zpl_true;
		break;
#endif
	case IF_LOOPBACK:
		if (IF_LOOPBACK_MAX > if_count_lookup_type(IF_LOOPBACK))
		{
			return zpl_true;
		}
		break;
#ifdef ZPL_NSM_VLAN
	case IF_VLAN:
		if (IF_VLAN_MAX > if_count_lookup_type(IF_VLAN))
		{
			return zpl_true;
		}
		break;
#endif
#ifdef ZPL_NSM_TRUNK	
	case IF_LAG:
		if (IF_LAG_MAX > if_count_lookup_type(IF_LAG))
		{
			return zpl_true;
		}
		break;
#endif
	default:
		break;
	}
	return zpl_false;
}

zpl_bool if_can_delete(struct interface *ifp)
{
	zpl_bool delete = zpl_false;
	if (ifp->if_type == IF_ETHERNET || 
		ifp->if_type == IF_GIGABT_ETHERNET || 
		ifp->if_type == IF_XGIGABT_ETHERNET ||
		ifp->if_type == IF_SERIAL || 
		ifp->if_type == IF_WIRELESS)
	{
		if (IF_ID_GET(ifp->uspv))
			delete = zpl_true;
	}
	else if (ifp->if_type == IF_TUNNEL ||
			 ifp->if_type == IF_VLAN || ifp->if_type == IF_LAG || ifp->if_type == IF_LOOPBACK || ifp->if_type == IF_BRIGDE
#ifdef CUSTOM_INTERFACE
	/* || ifp->if_type == IF_WIFI
	|| ifp->if_type == IF_MODEM */
#endif
	)
	{
		delete = zpl_true;
	}
	return delete;
}


struct interface *
if_create_vrf_dynamic(const char *name, zpl_uint32 namelen, vrf_id_t vrf_id)
{
	struct interface *ifp = NULL;
	struct list *intf_list = _zpl_if_master_lst.intfList;
	assert(name);
	assert(namelen <= IF_NAME_MAX); /* Need space for '\0' at end. */
	
	ifp = XCALLOC(MTYPE_IF, sizeof(struct interface));
	if (ifp == NULL)
	{
		return NULL;
	}
	ifp->ifindex = IFINDEX_INTERNAL;

	ifp->if_type = if_iftype_make(name);

	ifp->dynamic = zpl_true;

	if (if_ifname_setup(ifp, name, namelen) != OK)
	{
		zlog_err(MODULE_LIB, "if_create(%s): corruption detected  %u!", name, namelen);

		XFREE(MTYPE_IF, ifp);
		return NULL;
	}

	ifp->ifindex = if_ifindex_make(name, NULL);
	ifp->uspv = IF_TYPE_CLR(ifp->ifindex);

	ifp->name[namelen] = '\0';
	ifp->vrf_id = vrf_id;

	ifp->connected = list_new();
	ifp->connected->del = (void (*)(void *))connected_free;

	if_ifindex_setup(ifp);

	SET_FLAG(ifp->status, IF_INTERFACE_ACTIVE);

	ifp->mtu = IF_MTU_DEFAULT;
	ifp->mtu6 = IF_MTU_DEFAULT;

	ifp->flags |= IPSTACK_IFF_UP | IPSTACK_IFF_RUNNING;
	if_make_llc_type(ifp);
	if_mode_default(ifp);
	IF_MASTER_LOCK();
	if (if_lookup_by_name_vrf(ifp->name, vrf_id) == NULL)
		listnode_add_sort(intf_list, ifp);
	else
		zlog_err(MODULE_LIB, "if_create(%s): corruption detected -- interface with this "
						   "name exists already in VRF %u!",
				 ifp->name, vrf_id);

	zlog_debug(MODULE_LIB, "================if_create(%s): if_type  %x ifindex  0x%x uspv  %x id %d", 
		ifp->name, ifp->if_type, ifp->ifindex, ifp->uspv, IF_IFINDEX_ID_GET(ifp->ifindex));

	if (_zpl_if_master_lst.if_master_add_cb)
		(_zpl_if_master_lst.if_master_add_cb)(ifp);
	IF_MASTER_UNLOCK();
	return ifp;
}

struct interface *
if_create_vrf(const char *name, zpl_uint32 namelen, vrf_id_t vrf_id)
{
	struct interface *ifp = NULL;
	struct list *intf_list = _zpl_if_master_lst.intfList;
	assert(name);
	assert(namelen <= IF_NAME_MAX); /* Need space for '\0' at end. */
	ifp = XCALLOC(MTYPE_IF, sizeof(struct interface));
	if (ifp == NULL)
	{
		return NULL;
	}
	ifp->ifindex = IFINDEX_INTERNAL;

	ifp->if_type = if_iftype_make(name);

	ifp->dynamic = zpl_false;

	if (if_ifname_setup(ifp, name, namelen) != OK)
	{
		XFREE(MTYPE_IF, ifp);
		return NULL;
	}

	ifp->ifindex = if_ifindex_make(name, NULL);
	ifp->uspv = IF_TYPE_CLR(ifp->ifindex);
	ifp->name[namelen] = '\0';
	ifp->vrf_id = vrf_id;

	ifp->connected = list_new();
	ifp->connected->del = (void (*)(void *))connected_free;

	if_ifindex_setup(ifp);

	SET_FLAG(ifp->status, IF_INTERFACE_ACTIVE);

	ifp->mtu = IF_MTU_DEFAULT;
	ifp->mtu6 = IF_MTU_DEFAULT;

	ifp->flags |= IPSTACK_IFF_UP | IPSTACK_IFF_RUNNING;
	if_make_llc_type(ifp);
	if_mode_default(ifp);
	IF_MASTER_LOCK();
	if (if_lookup_by_name_vrf(ifp->name, vrf_id) == NULL)
		listnode_add_sort(intf_list, ifp);
	else
		zlog_err(MODULE_LIB, "if_create(%s): corruption detected -- interface with this "
						   "name exists already in VRF %u!",
				 ifp->name, vrf_id);

	zlog_debug(MODULE_LIB, "if_create(%s): if_type  %x ifindex  0x%x uspv  %x id %d", 
		ifp->name, ifp->if_type, ifp->ifindex, ifp->uspv, IF_IFINDEX_ID_GET(ifp->ifindex));

	if (_zpl_if_master_lst.if_master_add_cb)
		_zpl_if_master_lst.if_master_add_cb(ifp);
	IF_MASTER_UNLOCK();
	return ifp;
}

struct interface *
if_create(const char *name, zpl_uint32 namelen)
{
	return if_create_vrf(name, namelen, VRF_DEFAULT);
}

struct interface *
if_create_dynamic(const char *name, zpl_uint32 namelen)
{
	return if_create_vrf_dynamic(name, namelen, VRF_DEFAULT);
}

/* Delete and free interface structure. */
void if_delete(struct interface *ifp)
{
	IF_MASTER_LOCK();
	if (_zpl_if_master_lst.if_master_del_cb)
		(_zpl_if_master_lst.if_master_del_cb)(ifp);

	listnode_delete(_zpl_if_master_lst.intfList, ifp);
	list_delete_all_node(ifp->connected);
	list_free(ifp->connected);
	XFREE(MTYPE_IF, ifp);
	IF_MASTER_UNLOCK();
}

/* Interface existance check by index. */
struct interface *
if_lookup_by_index_vrf(ifindex_t ifindex, vrf_id_t vrf_id)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	IF_MASTER_LOCK();
	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
	{
		if ((ifp->ifindex && ifp->ifindex == ifindex) && (ifp->vrf_id == vrf_id))
		{
			IF_MASTER_UNLOCK();
			return ifp;
		}
	}
	IF_MASTER_UNLOCK();
	return NULL;
}

struct interface *
if_lookup_by_index(ifindex_t ifindex)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	IF_MASTER_LOCK();
	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
	{
		if (ifp->ifindex && ifp->ifindex == ifindex)
		{
			IF_MASTER_UNLOCK();
			return ifp;
		}
	}
	IF_MASTER_UNLOCK();
	return NULL;
}

struct interface *
if_lookup_by_kernel_name_vrf(const char *name, vrf_id_t vrf_id)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	IF_MASTER_LOCK();
	if (name)
	{
		for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
		{
			if (ifp->ker_name_hash == if_name_hash_make(name) && (ifp->vrf_id == vrf_id))
			{
				IF_MASTER_UNLOCK();
				return ifp;
			}
		}
	}
	IF_MASTER_UNLOCK();
	return NULL;
}

struct interface *
if_lookup_by_kernel_name(const char *name)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	IF_MASTER_LOCK();
	if (name)
	{
		for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
		{
			if (ifp->ker_name_hash == if_name_hash_make(name))
			{
				IF_MASTER_UNLOCK();
				return ifp;
			}
		}
	}
	IF_MASTER_UNLOCK();
	return NULL;
}

struct interface *
if_lookup_by_kernel_index_vrf(ifkernindex_t kifindex, vrf_id_t vrf_id)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	IF_MASTER_LOCK();
	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
	{
		if (ifp->ker_ifindex && ifp->ker_ifindex == kifindex && (ifp->vrf_id == vrf_id))
		{
			IF_MASTER_UNLOCK();
			return ifp;
		}
	}
	IF_MASTER_UNLOCK();
	return NULL;
}

struct interface * if_lookup_by_kernel_index(ifkernindex_t kifindex)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	IF_MASTER_LOCK();
	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
	{
		if (ifp->ker_ifindex && ifp->ker_ifindex == kifindex)
		{
			IF_MASTER_UNLOCK();
			return ifp;
		}
	}
	IF_MASTER_UNLOCK();
	return NULL;
}

const char *ifkernelindex2kernelifname(ifkernindex_t kifindex)
{
	struct interface *ifp = NULL;
	return ((ifp = if_lookup_by_kernel_index(kifindex)) != NULL) ? ifp->ker_name : NULL;
}

const char *ifkernelindex2kernelifname_vrf(ifkernindex_t kifindex, vrf_id_t vrf_id)
{
	struct interface *ifp = NULL;
	return ((ifp = if_lookup_by_kernel_index_vrf(kifindex, vrf_id)) != NULL) ? ifp->ker_name : NULL;
}

ifkernindex_t ifname2kernelifindex(const char *ifname)
{
	struct interface *ifp = NULL;
	return ((ifp = if_lookup_by_kernel_name(ifname)) != NULL) ? ifp->ker_ifindex : IFINDEX_INTERNAL;
}

ifkernindex_t ifname2kernelifindex_vrf(const char *ifname, vrf_id_t vrf_id)
{
	struct interface *ifp = NULL;
	return ((ifp = if_lookup_by_kernel_name_vrf(ifname, vrf_id)) != NULL) ? ifp->ker_ifindex : IFINDEX_INTERNAL;
}

ifkernindex_t ifindex2ifkernel(ifindex_t ifindex)
{
	struct interface *ifp = NULL;
	return ((ifp = if_lookup_by_index(ifindex)) != NULL) ? ifp->ker_ifindex : IFINDEX_INTERNAL;
}

ifindex_t ifkernel2ifindex(ifkernindex_t ifkindex)
{
	struct interface *ifp = NULL;
	return ((ifp = if_lookup_by_kernel_index(ifkindex)) != NULL) ? ifp->ifindex : IFINDEX_INTERNAL;
}

const char *
ifindex2ifname_vrf(ifindex_t ifindex, vrf_id_t vrf_id)
{
	struct interface *ifp = NULL;
	return ((ifp = if_lookup_by_index_vrf(ifindex, vrf_id)) != NULL) ? ifp->name : "unknown";
}

const char *
ifindex2ifname(ifindex_t ifindex)
{
	struct interface *ifp = NULL;
	return ((ifp = if_lookup_by_index(ifindex)) != NULL) ? ifp->name : "unknown";
}

ifindex_t ifname2ifindex_vrf(const char *name, vrf_id_t vrf_id)
{
	struct interface *ifp = NULL;
	return ((ifp = if_lookup_by_name_vrf(name, vrf_id)) != NULL) ? ifp->ifindex : IFINDEX_INTERNAL;
}

ifindex_t ifname2ifindex(const char *name)
{
	struct interface *ifp = NULL;
	return ((ifp = if_lookup_by_name(name)) != NULL) ? ifp->ifindex : IFINDEX_INTERNAL;
}

/* Interface existance check by interface name. */
struct interface *
if_lookup_by_name_vrf(const char *name, vrf_id_t vrf_id)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	IF_MASTER_LOCK();
	if (name)
	{
		for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
		{
			if (ifp->name_hash == if_name_hash_make(name) && (ifp->vrf_id == vrf_id))
			{
				IF_MASTER_UNLOCK();
				return ifp;
			}
		}
	}
	IF_MASTER_UNLOCK();
	return NULL;
}

struct interface *
if_lookup_by_name(const char *name)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	IF_MASTER_LOCK();
	if (name)
	{
		for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
		{
			if (ifp->name_hash == if_name_hash_make(name))
			{
				IF_MASTER_UNLOCK();
				return ifp;
			}
		}
	}
	IF_MASTER_UNLOCK();
	return NULL;
}

struct interface *
if_lookup_by_name_len_vrf(const char *name, zpl_uint32 namelen, vrf_id_t vrf_id)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;

	if (namelen > IF_NAME_MAX)
		return NULL;
	IF_MASTER_LOCK();
	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
	{
		if (ifp->name_hash == if_name_hash_make(name) && (ifp->vrf_id == vrf_id))
		{
			IF_MASTER_UNLOCK();
			return ifp;
		}
	}
	IF_MASTER_UNLOCK();
	return NULL;
}

struct interface *
if_lookup_by_name_len(const char *name, zpl_uint32 namelen)
{
	return if_lookup_by_name(name);
}

ifindex_t  vlanif2ifindex(vlan_t vid)
{
	zpl_uint32 iuspv = IF_USPV_SET(0, 0, 0, vid);
	ifindex_t ifindex = IF_IFINDEX_SET(IF_VLAN, iuspv);
	return ifindex;
}

zpl_phyport_t  if_ifindex2phy(ifindex_t ifindex)
{
	struct interface *ifp = if_lookup_by_index(ifindex);
	return ifp ? ifp->phyid : IFPHYID_INTERNAL;
}

ifindex_t  if_phy2ifindex(zpl_phyport_t phyid)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	IF_MASTER_LOCK();
	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
	{
		if (ifp->phyid && ifp->phyid == phyid)
		{
			IF_MASTER_UNLOCK();
			return ifp->ifindex;
		}
	}
	IF_MASTER_UNLOCK();
	return IFINDEX_INTERNAL;
}

zpl_phyport_t  if_ifindex2l3intfid(ifindex_t ifindex)
{
	struct interface *ifp = if_lookup_by_index(ifindex);
	return ifp ? ifp->l3intfid : IFPHYID_INTERNAL;
}

ifindex_t  if_l3intfid2ifindex(zpl_phyport_t l3intfid)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	IF_MASTER_LOCK();
	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
	{
		if (ifp->l3intfid && ifp->l3intfid == l3intfid)
		{
			IF_MASTER_UNLOCK();
			return ifp->ifindex;
		}
	}
	IF_MASTER_UNLOCK();
	return IFINDEX_INTERNAL;
}

int if_update_phyid(ifindex_t ifindex, zpl_phyport_t phyid)
{
	struct interface *ifp = if_lookup_by_index(ifindex);
	if(ifp)
		ifp->phyid = phyid;
	return OK;
}

int if_update_phyid2(struct interface *ifp, zpl_phyport_t phyid)
{
	ifp->phyid = phyid;
	return OK;
}

int if_update_l3intfid(ifindex_t ifindex, zpl_phyport_t l3intfid)
{
	struct interface *ifp = if_lookup_by_index(ifindex);
	if(ifp)
		ifp->l3intfid = l3intfid;
	return OK;
}

int if_update_l3intfid2(struct interface *ifp, zpl_phyport_t l3intfid)
{
	ifp->l3intfid = l3intfid;
	return OK;
}

void *if_module_data(struct interface *ifp, module_t mid)
{
	if(mid >= MODULE_NONE && mid < MODULE_MAX)
		return ifp->info[mid];
	return NULL;	
}

vrf_id_t  if_ifindex2vrfid(ifindex_t ifindex)
{
	struct interface *ifp = if_lookup_by_index(ifindex);
	return ifp ? ifp->vrf_id : IFINDEX_INTERNAL;
}

/* Lookup interface by IPv4 address. */
struct interface *
if_lookup_exact_address_vrf(struct ipstack_in_addr src, vrf_id_t vrf_id)
{
	struct listnode *node = NULL;
	struct listnode *cnode = NULL;
	struct interface *ifp = NULL;
	struct prefix *p = NULL;
	struct connected *c = NULL;
	IF_MASTER_LOCK();
	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
	{
		if ((ifp->vrf_id == vrf_id))
		{
			for (ALL_LIST_ELEMENTS_RO(ifp->connected, cnode, c))
			{
				p = c->address;

				if (p && p->family == IPSTACK_AF_INET)
				{
					if (IPV4_ADDR_SAME(&p->u.prefix4, &src))
					{
						IF_MASTER_UNLOCK();
						return ifp;
					}
				}
			}
		}
	}
	IF_MASTER_UNLOCK();
	return NULL;
}

struct interface *
if_lookup_exact_address(struct ipstack_in_addr src)
{
	return if_lookup_exact_address_vrf(src, VRF_DEFAULT);
}

/* Lookup interface by IPv4 address. */
struct interface *
if_lookup_address_vrf(struct ipstack_in_addr src, vrf_id_t vrf_id)
{
	struct listnode *node = NULL;
	struct prefix addr;
	zpl_uint32 bestlen = 0;
	struct listnode *cnode = NULL;
	struct interface *ifp = NULL;
	struct connected *c = NULL;
	struct interface *match = NULL;
	IF_MASTER_LOCK();
	addr.family = IPSTACK_AF_INET;
	addr.u.prefix4 = src;
	addr.prefixlen = IPV4_MAX_BITLEN;

	match = NULL;

	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
	{
		if (ifp->vrf_id == vrf_id)
		{
			for (ALL_LIST_ELEMENTS_RO(ifp->connected, cnode, c))
			{
				if (c->address && (c->address->family == IPSTACK_AF_INET) && prefix_match(CONNECTED_PREFIX(c), &addr) && (c->address->prefixlen > bestlen))
				{
					bestlen = c->address->prefixlen;
					match = ifp;
				}
			}
		}
	}
	IF_MASTER_UNLOCK();
	return match;
}

struct interface *
if_lookup_address(struct ipstack_in_addr src)
{
	return if_lookup_address_vrf(src, VRF_DEFAULT);
}

/* Lookup interface by prefix */
struct interface *
if_lookup_prefix_vrf(struct prefix *prefix, vrf_id_t vrf_id)
{

	struct listnode *node = NULL;
	struct listnode *cnode = NULL;
	struct interface *ifp = NULL;
	struct connected *c = NULL;
	IF_MASTER_LOCK();
	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
	{
		if (ifp->vrf_id == vrf_id)
		{
			for (ALL_LIST_ELEMENTS_RO(ifp->connected, cnode, c))
			{
				if (prefix_cmp(c->address, prefix) == 0)
				{
					IF_MASTER_UNLOCK();
					return ifp;
				}
			}
		}
	}
	IF_MASTER_UNLOCK();
	return NULL;
}

struct interface *
if_lookup_prefix(struct prefix *prefix)
{
	return if_lookup_prefix_vrf(prefix, VRF_DEFAULT);
}

zpl_uint32 if_count_lookup_type(if_type_t type)
{
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	int count = 0;
	IF_MASTER_LOCK();
	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
	{
		if (IF_TYPE_GET(ifp->if_type) == type)
		{
			(count)++;
		}
	}
	IF_MASTER_UNLOCK();
	return (count);
}
/* Get interface by name if given name interface doesn't exist create one. */

/* Does interface up ? */
zpl_bool if_is_up(struct interface *ifp)
{
	return ifp->flags & IPSTACK_IFF_UP;
}

/* Is interface running? */
zpl_bool if_is_running(struct interface *ifp)
{
	return ifp->flags & IPSTACK_IFF_RUNNING;
}

/* Is the interface operative, eg. either UP & RUNNING
 or UP & !IF_INTERFACE_LINK_DETECTION */
zpl_bool if_is_operative(struct interface *ifp)
{
	return ((ifp->flags & IPSTACK_IFF_UP) && (ifp->flags & IPSTACK_IFF_RUNNING || !CHECK_FLAG(ifp->status, IF_INTERFACE_LINKDETECTION)));
}

/* Is this loopback interface ? */
zpl_bool if_is_loopback(struct interface *ifp)
{
	/* XXX: Do this better, eg what if IFF_WHATEVER means X on platform M
	 * but Y on platform N?
	 */
	return (ifp->flags & (IPSTACK_IFF_LOOPBACK | IPSTACK_IFF_NOXMIT | IPSTACK_IFF_VIRTUAL));
}

/* Does this interface support broadcast ? */
zpl_bool if_is_broadcast(struct interface *ifp)
{
	return ifp->flags & IPSTACK_IFF_BROADCAST;
}

/* Does this interface support broadcast ? */
zpl_bool if_is_pointopoint(struct interface *ifp)
{
	return ifp->flags & IPSTACK_IFF_POINTOPOINT;
}

/* Does this interface support multicast ? */
zpl_bool if_is_multicast(struct interface *ifp)
{
	return ifp->flags & IPSTACK_IFF_MULTICAST;
}

zpl_bool if_is_serial(struct interface *ifp)
{
	/*if (os_strstr(ifp->name, "serial"))
		return zpl_true;*/
	if (ifp->if_type == IF_SERIAL)
		return zpl_true;
	return zpl_false;
}

zpl_bool if_is_ethernet(struct interface *ifp)
{
	/*if (os_strstr(ifp->name, "ethernet"))
		return zpl_true;*/
	if (ifp->if_type == IF_ETHERNET || ifp->if_type == IF_GIGABT_ETHERNET || ifp->if_type == IF_XGIGABT_ETHERNET)
		return zpl_true;
	return zpl_false;
}

zpl_bool if_is_tunnel(struct interface *ifp)
{
	/*if (os_strstr(ifp->name, "tunnel"))
		return zpl_true;*/
	if (ifp->if_type == IF_TUNNEL)
		return zpl_true;
	return zpl_false;
}

zpl_bool if_is_lag(struct interface *ifp)
{
	/*if (os_strstr(ifp->name, "port-channel"))
		return zpl_true;*/
	if (ifp->if_type == IF_LAG)
		return zpl_true;
	return zpl_false;
}

zpl_bool if_is_lag_member(struct interface *ifp)
{
	return CHECK_FLAG(ifp->ifmember, IF_TRUNK_MEM);
}

zpl_bool if_is_vlan(struct interface *ifp)
{
	if (ifp->if_type == IF_VLAN)
		return zpl_true;
	return zpl_false;
}

zpl_bool if_is_brigde(struct interface *ifp)
{
	if (ifp->if_type == IF_BRIGDE)
		return zpl_true;
	return zpl_false;
}

zpl_bool if_is_brigde_member(struct interface *ifp)
{
	return CHECK_FLAG(ifp->ifmember, IF_BRIDGE_MEM);
}

zpl_bool if_is_loop(struct interface *ifp)
{
	if (ifp->if_type == IF_LOOPBACK)
		return zpl_true;
	return zpl_false;
}

zpl_bool if_is_wireless(struct interface *ifp)
{
	/*if (os_strstr(ifp->name, "wireless"))
		return zpl_true;*/
	if (ifp->if_type == IF_WIRELESS)
		return zpl_true;
	return zpl_false;
}

zpl_bool if_is_online(struct interface *ifp)
{
	if (ifp->online)
		return zpl_true;
	return zpl_false;
}
zpl_bool if_is_l3intf(struct interface *ifp)
{
	if (ifp->if_mode == IF_MODE_L3)
		return zpl_true;
	return zpl_false;
}

zpl_bool if_have_kernel(struct interface *ifp)
{
	return (ifp->have_kernel);
}

int if_online(struct interface *ifp, zpl_bool enable)
{
	ifp->online = enable;
	return OK;
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
		zpl_char buf[IF_NAME_MAX + 1];
		os_memset(buf, 0, sizeof(buf));
		os_strcpy(buf, str);
		os_memset(ifp->ker_name, 0, sizeof(ifp->ker_name));
		os_strcpy(ifp->ker_name, buf);
		ifp->ker_name_hash = if_name_hash_make(ifp->ker_name);
#ifdef ZPL_PAL_MODULE
		ifp->ker_ifindex = if_nametoindex(ifp->ker_name);
#endif
	}
	else
	{
		os_memset(ifp->ker_name, 0, sizeof(ifp->ker_name));
		ifp->ker_name_hash = 0;
		ifp->ker_ifindex = 0;
	}
	return OK;
}
#ifdef IF_ENCAPSULATION_ENABLE
const char *if_encapsulation_string(if_enca_t enca)
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
#endif
int if_list_each(int (*cb)(struct interface *ifp, void *pVoid), void *pVoid)
{
	int ret = OK;
	struct listnode *node = NULL;
	struct interface *ifp = NULL;
	IF_MASTER_LOCK();
	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, ifp))
	{
		if (ifp)
		{
			if (cb)
			{
				ret = (cb)(ifp, pVoid);
				if (OK != ret)
					break;
			}
		}
	}
	IF_MASTER_UNLOCK();
	return ret;
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
		if (p1->family == IPSTACK_AF_INET &&
			IPV4_ADDR_SAME(&p1->u.prefix4, &p2->u.prefix4))
			return 1;
#ifdef ZPL_BUILD_IPV6
		if (p1->family == IPSTACK_AF_INET6 &&
			IPV6_ADDR_SAME(&p1->u.prefix6, &p2->u.prefix6))
			return 1;
#endif /* ZPL_BUILD_IPV6 */
	}
	return 0;
}

struct connected *
connected_delete_by_prefix(struct interface *ifp, struct prefix *p)
{
	struct listnode *node = NULL;
	struct listnode *next = NULL;
	struct connected *ifc = NULL;

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
connected_lookup_address(struct interface *ifp, struct ipstack_in_addr dst)
{
	struct prefix addr;
	struct listnode *cnode = NULL;
	struct connected *c = NULL;
	struct connected *match = NULL;

	addr.family = IPSTACK_AF_INET;
	addr.u.prefix4 = dst;
	addr.prefixlen = IPV4_MAX_BITLEN;

	match = NULL;

	for (ALL_LIST_ELEMENTS_RO(ifp->connected, cnode, c))
	{
		if (c->address && (c->address->family == IPSTACK_AF_INET) && prefix_match(CONNECTED_PREFIX(c), &addr) && (!match || (c->address->prefixlen > match->address->prefixlen)))
			match = c;
	}
	return match;
}

struct connected *
connected_add_by_prefix(struct interface *ifp, struct prefix *p,
						struct prefix *destination)
{
	struct connected *ifc = NULL;

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
	struct connected *ifc = NULL;
	struct listnode *node = NULL;

	for (ALL_LIST_ELEMENTS_RO(ifp->connected, node, ifc))
		if (prefix_same(ifc->address, p))
			return ifc;

	return NULL;
}


struct connected *
connected_lookup(struct interface *ifp, struct prefix *p)
{
	return connected_check(ifp, p);
}

/* Check if two ifc's describe the same address in the same state */
int connected_same(struct connected *ifc1, struct connected *ifc2)
{
	if (ifc1->ifp != ifc2->ifp)
		return 0;

	if (ifc1->destination)
		if (!ifc2->destination)
			return 0;
	if (ifc2->destination)
		if (!ifc1->destination)
			return 0;

	if (ifc1->destination && ifc2->destination)
		if (!prefix_same(ifc1->destination, ifc2->destination))
			return 0;

	if (ifc1->flags != ifc2->flags)
		return 0;

	if (ifc1->conf != ifc2->conf)
		return 0;

	return 1;
}

/* Print if_addr structure. */
static void __attribute__((unused))
connected_log(struct connected *connected, zpl_char *str)
{
	struct prefix *p = NULL;
	struct interface *ifp = NULL;
	zpl_char logbuf[BUFSIZ];
	zpl_char buf[BUFSIZ];

	ifp = connected->ifp;
	p = connected->address;

	snprintf(logbuf, BUFSIZ, "%s interface %s vrf %u %s %s/%d ", str, ifp->name,
			 ifp->vrf_id, prefix_family_str(p),
			 ipstack_inet_ntop(p->family, &p->u.prefix, buf, BUFSIZ), p->prefixlen);

	p = connected->destination;
	if (p)
	{
		strncat(logbuf, ipstack_inet_ntop(p->family, &p->u.prefix, buf, BUFSIZ),
				BUFSIZ - strlen(logbuf));
	}
	zlog_info(MODULE_LIB, "%s", logbuf);
}

/* Printout flag information into log */
const char *
if_flag_dump(zpl_ulong flag)
{
	zpl_uint32 separator = 0;
	static zpl_char logbuf[BUFSIZ];

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
	IFF_OUT_LOG(IPSTACK_IFF_UP, "UP");
	IFF_OUT_LOG(IPSTACK_IFF_BROADCAST, "BROADCAST");
	IFF_OUT_LOG(IPSTACK_IFF_DEBUG, "DEBUG");
	IFF_OUT_LOG(IPSTACK_IFF_LOOPBACK, "LOOPBACK");
	IFF_OUT_LOG(IPSTACK_IFF_POINTOPOINT, "POINTOPOINT");
	IFF_OUT_LOG(IFF_NOTRAILERS, "NOTRAILERS");
	IFF_OUT_LOG(IPSTACK_IFF_RUNNING, "RUNNING");
	IFF_OUT_LOG(IPSTACK_IFF_NOARP, "NOARP");
	IFF_OUT_LOG(IPSTACK_IFF_PROMISC, "PROMISC");
	IFF_OUT_LOG(IPSTACK_IFF_ALLMULTI, "ALLMULTI");
	#ifdef IPSTACK_IFF_OACTIVE
	IFF_OUT_LOG(IPSTACK_IFF_OACTIVE, "OACTIVE");
	#endif
	#ifdef IPSTACK_IFF_SIMPLEX
	IFF_OUT_LOG(IPSTACK_IFF_SIMPLEX, "SIMPLEX");
	#endif
	#ifdef IPSTACK_IFF_LINK0
	IFF_OUT_LOG(IPSTACK_IFF_LINK0, "LINK0");
	#endif
	#ifdef IPSTACK_IFF_LINK1
	IFF_OUT_LOG(IPSTACK_IFF_LINK1, "LINK1");
	#endif
	#ifdef IPSTACK_IFF_LINK2
	IFF_OUT_LOG(IPSTACK_IFF_LINK2, "LINK2");
	#endif
	IFF_OUT_LOG(IPSTACK_IFF_MULTICAST, "MULTICAST");
	IFF_OUT_LOG(IPSTACK_IFF_NOXMIT, "NOXMIT");
	IFF_OUT_LOG(IPSTACK_IFF_NORTEXCH, "NORTEXCH");
	IFF_OUT_LOG(IPSTACK_IFF_VIRTUAL, "VIRTUAL");
	IFF_OUT_LOG(IPSTACK_IFF_IPV4, "IPv4");
	IFF_OUT_LOG(IPSTACK_IFF_IPV6, "IPv6");

	strlcat(logbuf, ">", BUFSIZ);

	return logbuf;
#undef IFF_OUT_LOG
}

/* For debugging */
static void if_dump(const struct interface *ifp)
{
	struct listnode *node = NULL;
	struct connected *c __attribute__((unused));

	for (ALL_LIST_ELEMENTS_RO(ifp->connected, node, c))
		zlog_info(MODULE_LIB, "Interface %s vrf %u index %d metric %d mtu %d "
#ifdef ZPL_BUILD_IPV6
							"mtu6 %d "
#endif /* ZPL_BUILD_IPV6 */
							"%s",
				  ifp->name, ifp->vrf_id, ifp->ifindex, ifp->metric,
				  ifp->mtu,
#ifdef ZPL_BUILD_IPV6
				  ifp->mtu6,
#endif /* ZPL_BUILD_IPV6 */
				  if_flag_dump(ifp->flags));
}

/* Interface printing for all interface. */
void if_dump_all(void)
{
	struct listnode *node = NULL;
	void *p = NULL;
	IF_MASTER_LOCK();
	for (ALL_LIST_ELEMENTS_RO(_zpl_if_master_lst.intfList, node, p))
		if_dump(p);
	IF_MASTER_UNLOCK();	
}

/* Initialize interface list. */
void if_init(void)
{
	memset(&_zpl_if_master_lst, 0, sizeof(struct if_master));
	if(_zpl_if_master_lst.intfList == NULL)
	{
		_zpl_if_master_lst.intfList = list_new();
		if (_zpl_if_master_lst.ifMutex == NULL)
			_zpl_if_master_lst.ifMutex = os_mutex_name_create("ifMutex");
		if (_zpl_if_master_lst.ifdatamutex == NULL)
			_zpl_if_master_lst.ifdatamutex = os_mutex_name_create("if_data_mutex");
			
		(_zpl_if_master_lst.intfList)->cmp = (zpl_int (*)(void *, void *))if_cmp_func;
	}
}

void if_terminate(void)
{
	IF_MASTER_LOCK();
	for (;;)
	{
		struct interface *ifp = NULL;

		ifp = listnode_head(_zpl_if_master_lst.intfList);
		if (ifp == NULL)
			break;

		if_delete(ifp);
	}

	list_delete(_zpl_if_master_lst.intfList);
	_zpl_if_master_lst.intfList = NULL;
	IF_MASTER_UNLOCK();
	if(_zpl_if_master_lst.ifdatamutex)
	{
		os_mutex_destroy(_zpl_if_master_lst.ifdatamutex);
		_zpl_if_master_lst.ifdatamutex = NULL;
	}
	if(_zpl_if_master_lst.ifMutex)
	{
		os_mutex_destroy(_zpl_if_master_lst.ifMutex);
		_zpl_if_master_lst.ifMutex = NULL;
	}
}

const char *
if_link_type_str(enum if_link_type llt)
{
	switch (llt)
	{
#define llts(T, S) \
	case (T):      \
		return (S)
		llts(IF_LLT_UNKNOWN, "Unknown");
		llts(IF_LLT_SERIAL, "Serial");
		llts(IF_LLT_ETHER, "Ethernet");
		llts(IF_LLT_GIETHER, "Gigabt-Ethernet");
		llts(IF_LLT_TUNNEL, "Tunnel");
		llts(IF_LLT_VLAN, "Vlan");
		llts(IF_LLT_LAG, "Lag");
		llts(IF_LLT_ATM, "ATM");
		llts(IF_LLT_SLIP, "SLIP");
		llts(IF_LLT_CSLIP, "Compressed SLIP");
		llts(IF_LLT_SLIP6, "SLIPv6");
		llts(IF_LLT_CSLIP6, "Compressed SLIPv6");
		llts(IF_LLT_PPP, "PPP");
		llts(IF_LLT_CHDLC, "Cisco HDLC");
		llts(IF_LLT_RAWHDLC, "Raw HDLC");
		llts(IF_LLT_IPIP, "IPIP Tunnel");
		llts(IF_LLT_IPIP6, "IPIP6 Tunnel");
		llts(IF_LLT_LOOPBACK, "Loopback");
		llts(IF_LLT_SIT, "IPv6-in-IPv4 SIT");
		llts(IF_LLT_IPGRE, "GRE over IP");
		llts(IF_LLT_IP6GRE, "GRE over IPV6");
		llts(IF_LLT_BRIGDE, "Brigde");
		llts(IF_LLT_WIFI, "Wifi");
		llts(IF_LLT_MODEM, "Modem");
		llts(IF_LLT_WIRELESS, "Wireless");
	default:
		zlog_warn(MODULE_LIB, "Unknown value %d", llt);
		return "Unknown type!";
#undef llts
	}
	return NULL;
}

enum if_link_type
netlink_to_if_link_type(zpl_uint32  hwt)
{
	switch (hwt)
	{
	case IPSTACK_ARPHRD_ETHER:
		return IF_LLT_ETHER;
	case IPSTACK_ARPHRD_ATM:
		return IF_LLT_ATM;
	case IPSTACK_ARPHRD_SLIP:
		return IF_LLT_SLIP;
	case IPSTACK_ARPHRD_CSLIP:
		return IF_LLT_CSLIP;
	case IPSTACK_ARPHRD_SLIP6:
		return IF_LLT_SLIP6;
	case IPSTACK_ARPHRD_CSLIP6:
		return IF_LLT_CSLIP6;
	case IPSTACK_ARPHRD_PPP:
		return IF_LLT_PPP;
	case IPSTACK_ARPHRD_CISCO:
		return IF_LLT_CHDLC;
	case IPSTACK_ARPHRD_RAWHDLC:
		return IF_LLT_RAWHDLC;
	case IPSTACK_ARPHRD_TUNNEL:
		return IF_LLT_IPIP;
	case IPSTACK_ARPHRD_TUNNEL6:
		return IF_LLT_IPIP6;
	case IPSTACK_ARPHRD_LOOPBACK:
		return IF_LLT_LOOPBACK;
	case IPSTACK_ARPHRD_SIT:
		return IF_LLT_SIT;
	case IPSTACK_ARPHRD_IPGRE:
		return IF_LLT_IPGRE;

		// case ARPHRD_IEEE802: return IF_LLT_IPGRE;
	case IPSTACK_ARPHRD_IEEE80211:
		return IF_LLT_WIRELESS;
		// case ARPHRD_IEEE802154: return IF_LLT_ZIGBEE;

#ifdef IPSTACK_ARPHRD_IP6GRE
	case IPSTACK_ARPHRD_IP6GRE:
		return IF_LLT_IP6GRE;
#endif
	default:
		return IF_LLT_UNKNOWN;
	}
}
