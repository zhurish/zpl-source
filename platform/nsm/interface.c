/*
 * Interface function.
 * Copyright (C) 1997, 1999 Kunihiro Ishiguro
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

#include <zebra.h>

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "log.h"
#include "vrf.h"
#include "command.h"
#include "interface.h"

#include "if_name.h"
#include "rib.h"
#include "zserv.h"
#include "redistribute.h"
#include "debug.h"
#include "zclient.h"
#include "nsm_mac.h"
#include "nsm_vlan.h"
#include "nsm_serial.h"
#include "nsm_dhcp.h"

#include "nsm_tunnel.h"

#ifdef PL_VLAN_MODULE
#include "vlan.h"
#endif

#ifdef PL_PAL_MODULE
//#include "pal.h"
#endif


/* helper only for nsm_interface_linkdetect */
static void nsm_interface_linkdetect_set_val(struct interface *ifp,
		nsm_linkdetect_en val) {
	switch (val) {
	case IF_LINKDETECT_ON:
		SET_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION);
		break;
	case IF_LINKDETECT_OFF:
		UNSET_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION);
		break;
	default:
		break;
	}
}


static int nsm_interface_linkdetect_set(struct interface *ifp, nsm_linkdetect_en linkdetect)
{
	struct nsm_interface *zif = ifp->info[MODULE_NSM];
	assert(zif != NULL);
	int if_was_operative = if_is_operative(ifp);

	/* If user has explicitly configured for the interface, let that set */
	if (zif->linkdetect != linkdetect)
	{
		nsm_interface_linkdetect_set_val(ifp, linkdetect);
		/* When linkdetection is enabled, interface might come down */
		if (!if_is_operative(ifp) && if_was_operative)
			;//if_down(ifp);
		/* Alternatively, it may come up after disabling link detection */
		if (if_is_operative(ifp) && !if_was_operative)
			;//if_up(ifp);
	}
	return OK;
}

/* Wake up configured address if it is not in current kernel
   address. */
static void if_addr_wakeup (struct interface *ifp)
{
	struct listnode *node, *nnode;
	struct connected *ifc;
	struct prefix *p;
	//int ret;

	for (ALL_LIST_ELEMENTS(ifp->connected, node, nnode, ifc))
	{
		p = ifc->address;

		if (CHECK_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED))
		{
			/* Address check. */
			if (p->family == AF_INET)
			{
				if (!if_is_up(ifp))
				{
					/* Assume zebra is configured like following:
					 *
					 *   interface gre0
					 *    ip addr 192.0.2.1/24
					 *   !
					 *
					 * As soon as zebra becomes first aware that gre0 exists in the
					 * kernel, it will set gre0 up and configure its addresses.
					 *
					 * (This may happen at startup when the interface already exists
					 * or during runtime when the interface is added to the kernel)
					 *
					 * XXX: IRDP code is calling here via if_add_update - this seems
					 * somewhat weird.
					 * XXX: RUNNING is not a settable flag on any system
					 * I (paulj) am aware of.
					 */
					//if_set_flags(ifp, IFF_UP | IFF_RUNNING);
					//if_refresh(ifp);
					nsm_pal_interface_up(ifp);
					pal_interface_refresh_flag(ifp);
					//ifp->k_ifindex = pal_interface_ifindex(ifp->k_name);
				}
				if(nsm_pal_interface_set_address(ifp, ifc, 0) != OK)
				//ret = if_set_prefix(ifp, ifc);
				//if (ret < 0)
				{
					zlog_warn("Can't set interface's address: %s",
							safe_strerror(errno));
					continue;
				}

				//SET_FLAG(ifc->conf, ZEBRA_IFC_QUEUED);
				/* The address will be advertised to zebra clients when the notification
				 * from the kernel has been received.
				 * It will also be added to the interface's subnet list then. */
			}
#ifdef HAVE_IPV6
			if (p->family == AF_INET6)
			{
				if (!if_is_up(ifp))
				{
					/* See long comment above */
					//if_set_flags(ifp, IFF_UP | IFF_RUNNING);
					//if_refresh(ifp);
					nsm_pal_interface_up(ifp);
					pal_interface_refresh_flag(ifp);
					//ifp->k_ifindex = pal_interface_ifindex(ifp->k_name);
				}
				if(nsm_pal_interface_set_address(ifp, ifc, 0) != OK)
				//ret = if_prefix_add_ipv6(ifp, ifc);
				//if (ret < 0)
				{
					zlog_warn("Can't set interface's address: %s",
							safe_strerror(errno));
					continue;
				}

				//SET_FLAG(ifc->conf, ZEBRA_IFC_QUEUED);
				/* The address will be advertised to zebra clients when the notification
				 * from the kernel has been received. */
			}
#endif /* HAVE_IPV6 */
		}
	}
}


#ifdef USE_IPSTACK_IPCOM
static int nsm_interface_kname_set(struct interface *ifp)
{
	switch (ifp->if_type)
	{
	case IF_SERIAL:
	case IF_ETHERNET:
	case IF_GIGABT_ETHERNET:
	case IF_TUNNEL:
	case IF_BRIGDE:
	case IF_WIRELESS:
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:
	case IF_MODEM:
#endif
		if(IF_IFINDEX_ID_GET(ifp->ifindex))
			sprintf(ifp->k_name, "%s%d/%d/%d.%d", getkernelname(ifp->if_type),
					IF_IFINDEX_UNIT_GET(ifp->ifindex),
					IF_IFINDEX_SLOT_GET(ifp->ifindex),
					IF_IFINDEX_PORT_GET(ifp->ifindex),
					IF_IFINDEX_ID_GET(ifp->ifindex));
		else
			sprintf(ifp->k_name, "%s%d/%d/%d", getkernelname(ifp->if_type),
					IF_IFINDEX_UNIT_GET(ifp->ifindex),
					IF_IFINDEX_SLOT_GET(ifp->ifindex),
					IF_IFINDEX_PORT_GET(ifp->ifindex));
		break;
	case IF_LOOPBACK:
	case IF_VLAN:
	case IF_LAG:
		if(IF_IFINDEX_ID_GET(ifp->ifindex))
			sprintf(ifp->k_name, "%s%d", getkernelname(ifp->if_type),
					IF_IFINDEX_ID_GET(ifp->ifindex));
		else
			sprintf(ifp->k_name, "%s%d", getkernelname(ifp->if_type),
					IF_IFINDEX_PORT_GET(ifp->ifindex));
		break;
	default:
		break;
	}
	ifp->k_name_hash = if_name_hash_make(ifp->k_name);
	return OK;
}

static int nsm_interface_kmac_set(struct interface *ifp)
{
	char kmac[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
	switch (ifp->if_type)
	{
	case IF_SERIAL:
	case IF_ETHERNET:
	case IF_GIGABT_ETHERNET:
	case IF_TUNNEL:
	case IF_BRIGDE:
	case IF_WIRELESS:
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:
	case IF_MODEM:
#endif
		if(!IF_IFINDEX_ID_GET(ifp->ifindex))
		{
			kmac[5] = IF_IFINDEX_PORT_GET(ifp->ifindex);
			ifp->hw_addr_len = NSM_MAC_MAX;
			os_memcpy(ifp->hw_addr, kmac, ifp->hw_addr_len);
		}
		break;
	case IF_LOOPBACK:
	case IF_LAG:
		break;
	case IF_VLAN:
		if(!IF_IFINDEX_ID_GET(ifp->ifindex))
			kmac[5] = IF_IFINDEX_PORT_GET(ifp->ifindex);
		ifp->hw_addr_len = NSM_MAC_MAX;
		os_memcpy(ifp->hw_addr, kmac, ifp->hw_addr_len);
		break;
	default:
		break;
	}
	return OK;
}
#else
static int nsm_interface_kname_set(struct interface *ifp)
{
	char k_name[64];
	os_memset(k_name, 0, sizeof(k_name));
	switch (ifp->if_type)
	{
	case IF_SERIAL:
	case IF_TUNNEL:
	case IF_BRIGDE:
		sprintf(k_name, "%s%d%d", getkernelname(ifp->if_type),
				IF_IFINDEX_SLOT_GET(ifp->ifindex), IF_IFINDEX_PORT_GET(ifp->ifindex));
		break;
	case IF_ETHERNET:
	case IF_GIGABT_ETHERNET:
	case IF_WIRELESS:
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:
	case IF_MODEM:
#endif
		if(!IF_IFINDEX_ID_GET(ifp->ifindex))
			sprintf(k_name, "%s%d%d", getkernelname(ifp->if_type),
				IF_IFINDEX_SLOT_GET(ifp->ifindex), IF_IFINDEX_PORT_GET(ifp->ifindex));
		else
		{
			ifindex_t root_kifindex = ifindex2ifkernel(IF_IFINDEX_ROOT_GET(ifp->ifindex));
			const char *root_kname = ifkernelindex2kernelifname(root_kifindex);
			if(root_kname)
				sprintf(k_name, "%s.%d", root_kname,
					IF_IFINDEX_VLAN_GET(ifp->ifindex));
			else
				sprintf(k_name, "%s%d%d.%d", getkernelname(ifp->if_type),
						IF_IFINDEX_SLOT_GET(ifp->ifindex), IF_IFINDEX_PORT_GET(ifp->ifindex),
						IF_IFINDEX_VLAN_GET(ifp->ifindex));
		}

		break;
	case IF_LOOPBACK:
	case IF_VLAN:
	case IF_LAG:
		if(IF_IFINDEX_ID_GET(ifp->ifindex))
			sprintf(k_name, "%s%d", getkernelname(ifp->if_type),
					IF_IFINDEX_ID_GET(ifp->ifindex));
		else
			sprintf(k_name, "%s%d", getkernelname(ifp->if_type),
					IF_IFINDEX_PORT_GET(ifp->ifindex));
		break;
	default:
		break;
	}
	if(os_strlen(k_name))
		if_kname_set(ifp, k_name);
	return OK;
}

static int nsm_interface_kmac_set(struct interface *ifp)
{
	unsigned char kmac[NSM_MAC_MAX] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
	nsm_gmac_get_api(0, kmac, NSM_MAC_MAX);
	switch (ifp->if_type)
	{
	case IF_SERIAL:
	case IF_TUNNEL:
		if(!IF_IFINDEX_ID_GET(ifp->ifindex))
		{
			kmac[5] = IF_IFINDEX_PORT_GET(ifp->ifindex);
			ifp->hw_addr_len = NSM_MAC_MAX;
			os_memcpy(ifp->hw_addr, kmac, ifp->hw_addr_len);
		}
		else
		{
			kmac[5] = IF_IFINDEX_ID_GET(ifp->ifindex);
			ifp->hw_addr_len = NSM_MAC_MAX;
			os_memcpy(ifp->hw_addr, kmac, ifp->hw_addr_len);
		}
		break;
	case IF_ETHERNET:
	case IF_GIGABT_ETHERNET:
	case IF_BRIGDE:
	case IF_WIRELESS:
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:
	case IF_MODEM:
#endif
		if(!IF_IFINDEX_ID_GET(ifp->ifindex))
		{
			kmac[5] = IF_IFINDEX_PORT_GET(ifp->ifindex);
			ifp->hw_addr_len = NSM_MAC_MAX;
			os_memcpy(ifp->hw_addr, kmac, ifp->hw_addr_len);
		}
		break;
	case IF_LOOPBACK:
		break;
	case IF_LAG:
	case IF_VLAN:
		if(!IF_IFINDEX_ID_GET(ifp->ifindex))
			kmac[5] = IF_IFINDEX_PORT_GET(ifp->ifindex);
		ifp->hw_addr_len = NSM_MAC_MAX;
		os_memcpy(ifp->hw_addr, kmac, ifp->hw_addr_len);
		break;
	default:
		break;
	}
	return OK;
}
#endif

/* Called when new interface is added. */
static int nsm_interface_new_hook(struct interface *ifp)
{
	int ret = -1;
	struct nsm_interface *nsm_interface;

	nsm_interface = XCALLOC(MTYPE_IF, sizeof(struct nsm_interface));

	nsm_interface->multicast = IF_ZEBRA_MULTICAST_UNSPEC;
	nsm_interface->shutdown = IF_ZEBRA_SHUTDOWN_OFF;
	nsm_interface->duplex = NSM_IF_DUPLEX_AUTO;
	nsm_interface->speed = NSM_IF_SPEED_AUTO;
	nsm_interface->linkdetect = IF_LINKDETECT_ON;

	nsm_interface->ifp = ifp;

	//SET_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION);
	UNSET_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION);
	ifp->info[MODULE_NSM] = nsm_interface;

#ifdef USE_IPSTACK_KERNEL
	if(ifp->dynamic == FALSE && if_kernel_name_lookup(ifp->ifindex))
	{
		if_kname_set(ifp, if_kernel_name_lookup(ifp->ifindex));
	}
#endif
	if(!os_strlen(ifp->k_name))
	{
		if(nsm_interface_kname_set(ifp) == OK)
			nsm_interface_kmac_set(ifp);
	}

	nsm_client_notify_interface_add(ifp);

	ret = nsm_pal_interface_add(ifp);

	if(ret == OK && os_strlen(ifp->k_name))
	{
		SET_FLAG(ifp->status, ZEBRA_INTERFACE_ATTACH);
		ifp->k_ifindex = pal_interface_ifindex(ifp->k_name);
		pal_interface_refresh_flag(ifp);
		pal_interface_get_lladdr(ifp);
	}
	return OK;
}


int nsm_interface_update_kernel(struct interface *ifp, char *kname)
{
	if(ifp/* && ifp->dynamic == FALSE*/)
	{
		if_kname_set(ifp, kname);
		SET_FLAG(ifp->status, ZEBRA_INTERFACE_ATTACH);
		pal_interface_refresh_flag(ifp);
		pal_interface_get_lladdr(ifp);
		return OK;
	}
	return ERROR;
}

/* Called when interface is deleted. */
static int nsm_interface_delete_hook(struct interface *ifp)
{
	struct nsm_interface *nsm_interface;
	nsm_pal_interface_delete(ifp);
	if (ifp->info[MODULE_NSM])
	{
		nsm_interface = ifp->info[MODULE_NSM];
		/* Free installed address chains tree. */
		XFREE(MTYPE_IF, nsm_interface);
	}
	return OK;
}



BOOL nsm_interface_create_check_api(struct vty *vty, const char *ifname, const char *uspv)
{
	ifindex_t ifindex = if_ifindex_make(ifname, uspv);
	if_type_t type = IF_TYPE_GET(ifindex);
	switch (type)
	{
	case IF_ETHERNET:
	case IF_GIGABT_ETHERNET:
	case IF_WIRELESS:
		if (IF_ID_GET(ifindex))
		{
			return TRUE;
		}
		else
		{
			vty_out(vty,"Can not create PHY interface%s",VTY_NEWLINE);
			return FALSE;
		}
		break;

	case IF_BRIGDE:
		if(IF_BRIGDE_MAX > if_count_lookup_type(IF_BRIGDE))
		{
			return TRUE;
		}
		vty_out(vty,"Too much brigde interface%s",VTY_NEWLINE);
		break;
	case IF_SERIAL:
		vty_out(vty,"Can not create serial interface%s",VTY_NEWLINE);
		return FALSE;
		break;
#ifdef CUSTOM_INTERFACE
	case IF_WIFI:
	case IF_MODEM:
		return FALSE;
		break;
#endif
	case IF_TUNNEL:
		if (IF_SLOT_GET(ifindex) != IF_TUNNEL_SLOT)
		{
			vty_out(vty,"Tunnel interface of slot may be '%d'%s",IF_TUNNEL_SLOT, VTY_NEWLINE);
			return FALSE;
		}
/*		if (IF_ID_GET(ifindex) > IF_TUNNEL_MAX || IF_ID_GET(ifindex) < 1)
		{
			vty_out(vty,"Tunnel interface of slot may be '<%d-%d>'%s", 1, IF_TUNNEL_SLOT, VTY_NEWLINE);
			return FALSE;
		}*/
		if(if_count_lookup_type(IF_TUNNEL) >= IF_TUNNEL_MAX)
		{
			vty_out(vty,"Too much tunnel interface%s",VTY_NEWLINE);
			return FALSE;
		}
		return TRUE;
		break;
	case IF_LOOPBACK:
		if(IF_LOOPBACK_MAX > if_count_lookup_type(IF_LOOPBACK))
		{
			return TRUE;
		}
		vty_out(vty,"Too much loopback interface%s",VTY_NEWLINE);
		break;

	case IF_VLAN:
		if(IF_VLAN_MAX > if_count_lookup_type(IF_VLAN))
		{
			return TRUE;
		}
		vty_out(vty,"Too much vlan interface%s",VTY_NEWLINE);
		break;

	case IF_LAG:
		if(IF_LAG_MAX > if_count_lookup_type(IF_LAG))
		{
			return TRUE;
		}
		vty_out(vty,"Too much lag interface%s",VTY_NEWLINE);
		break;
	default:
		break;
	}
	vty_out(vty,"Can not Create interface %s %s(unknown error)%s",ifname, uspv, VTY_NEWLINE);
	return FALSE;
}


static int nsm_interface_delete(struct interface *ifp)
{
	int delete = 0;
	if (ifp->if_type == IF_ETHERNET
			|| ifp->if_type == IF_GIGABT_ETHERNET
			|| ifp->if_type == IF_SERIAL
			|| ifp->if_type == IF_WIRELESS)
	{
		if (IF_ID_GET(ifp->uspv))
			delete = 1;
	}
	else if (ifp->if_type == IF_TUNNEL ||
			ifp->if_type == IF_VLAN
			|| ifp->if_type == IF_LAG || ifp->if_type == IF_LOOPBACK
			|| ifp->if_type == IF_BRIGDE
#ifdef CUSTOM_INTERFACE
			/* || ifp->if_type == IF_WIFI
			|| ifp->if_type == IF_MODEM */
#endif
			)
	{
		if(ifp->if_type == IF_LAG)
		{
			u_int trunkId = 0;
			if(nsm_trunk_get_ID_interface_api(ifp->ifindex, &trunkId) == OK)
			{
				if(l2trunk_lookup_interface_count_api(trunkId))
				{
					return ERROR;
				}
			}
		}
		delete = 1;
	}
	if(delete)
	{
		zebra_interface_delete_update (ifp);
		nsm_client_notify_interface_delete(ifp);
		return OK;
	}
	return ERROR;
}

int nsm_interface_create_api(const char *ifname)
{
	int ret = ERROR;
	struct interface *ifp = NULL;
	IF_DATA_LOCK();
	ifp = if_create(ifname, os_strlen(ifname));
	if(ifp)
	{
		if(ifp->dynamic == FALSE)
			zebra_interface_add_update(ifp);
		IF_DATA_UNLOCK();
		return OK;
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_delete_api(struct interface *ifp)
{
	int ret = ERROR;
	IF_DATA_LOCK();
	ret = nsm_interface_delete(ifp);
	if(ret == OK)
		if_delete(ifp);
	IF_DATA_UNLOCK();
	return ret;
}


static int nsm_interface_ip_address_install(struct interface *ifp, struct prefix_ipv4 *cp)
{
	//int ret;
	struct nsm_interface *if_data;
	struct connected *ifc;
	struct prefix_ipv4 *p1, *p2;
	if_data = ifp->info[MODULE_NSM];
	ifc = connected_check(ifp, (struct prefix *) cp);
	if (!ifc)
	{
		ifc = connected_new();
		ifc->ifp = ifp;
		/* Address. */
		p1 = prefix_ipv4_new();
//		p = cp;
		prefix_copy ((struct prefix *)p1, (struct prefix *)cp);
		ifc->address = (struct prefix *) p1;
		/* Broadcast. */
		if (p1->prefixlen <= IPV4_MAX_PREFIXLEN - 2)
		{
			p2 = prefix_ipv4_new();
//			*p = cp;
			prefix_copy ((struct prefix *)p2, (struct prefix *)cp);
			p2->prefix.s_addr = ipv4_broadcast_addr(p2->prefix.s_addr, p2->prefixlen);
			ifc->destination = (struct prefix *) p2;
		}
		/* Add to linked list. */
		listnode_add(ifp->connected, ifc);
	}
	else
	{
		return ERROR;
	}
	/* This address is configured from zebra. */
	if (!CHECK_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED))
		SET_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED);

	/* In case of this route need to install kernel. */
	if (CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_ACTIVE)
			&& !(if_data && if_data->shutdown == IF_ZEBRA_SHUTDOWN_ON)) {

		/* Some system need to up the interface to set IP address. */
		if (!if_is_up(ifp)) {
			nsm_pal_interface_up(ifp);
			pal_interface_refresh_flag(ifp);
		}
		if(nsm_pal_interface_set_address(ifp, ifc, 0) != OK)
		{
			listnode_delete(ifp->connected, ifc);
			connected_free(ifc);
			//vty_out(vty, "%% Can't set interface IP address: %s.%s",
			//		safe_strerror(errno), VTY_NEWLINE);
			return ERROR;
		}
#ifdef PL_DHCP_MODULE
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
			SET_FLAG(ifc->conf, ZEBRA_IFC_DHCPC);
#endif
		connected_up_ipv4(ifp, ifc);
		nsm_client_notify_interface_add_ip(ifp, ifc, 0);
		zebra_interface_address_add_update (ifp, ifc);
		return OK;
	}
	return OK;
}

static int nsm_interface_ip_address_uninstall(struct interface *ifp, struct prefix_ipv4 *cp)
{
	//int ret;
	struct nsm_interface *if_data;
	struct connected *ifc;
	//struct prefix_ipv4 *p;
	if_data = ifp->info[MODULE_NSM];

	/* Check current interface address. */
	ifc = connected_check(ifp, (struct prefix *) cp);
	if (!ifc)
	{
		//printf("%s:connected_check\n",__func__);
		return ERROR;
	}

	/* This is not configured address. */
	if (!CHECK_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED))
		UNSET_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED);
		//return ERROR;

	zebra_interface_address_delete_update (ifp, ifc);
	nsm_client_notify_interface_del_ip(ifp, ifc, 0);
	while(ifc->raw_status != 0)
	{
		os_msleep(10);
	}
	/* This is real route. */

/*	ret = pal_kernel_if_unset_prefix(ifp, ifc);
	if (ret < 0) {
		vty_out(vty, "%% Can't unset interface IP address: %s.%s",
				safe_strerror(errno), VTY_NEWLINE);
		return CMD_WARNING;
	}*/
	//nsm_pal_interface_unset_address(ifp, ifc, 0);

	if(ifc->raw_status == 0)
	{
		if(nsm_pal_interface_unset_address(ifp, ifc, 0) != OK)
		{
			//printf("%s:nsm_pal_interface_unset_address\n",__func__);
			//vty_out(vty, "%% Can't unset interface IP address: %s.%s",
			//		safe_strerror(errno), VTY_NEWLINE);
			return ERROR;
		}
		//zebra_interface_address_delete_update(ifp, ifc);
		connected_down_ipv4(ifp, ifc);

		UNSET_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED);

		if (CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE)) {
			listnode_delete(ifp->connected, ifc);
			connected_free(ifc);
			//printf("%s:listnode_delete\n",__func__);
			return OK;
		}
		///printf("%s:listnode_delete raw_status = %d\n",__func__, ifp->status);
	}
	//printf("%s:if(ifc->raw_status == 0)\n",__func__);
	return ERROR;
}


#ifdef HAVE_IPV6
static int
nsm_interface_ipv6_address_install (struct interface *ifp,
		struct prefix_ipv6 *cp, int secondary)
{
	struct nsm_interface *if_data;
	struct connected *ifc;
	struct prefix_ipv6 *p1;//, *p2;
	int ret;

	if_data = ifp->info[MODULE_NSM];

	ifc = connected_check (ifp, (struct prefix *) cp);
	if (! ifc)
	{
		ifc = connected_new ();
		ifc->ifp = ifp;

		/* Address. */
		p1 = prefix_ipv6_new ();
		prefix_copy ((struct prefix *)p1, (struct prefix *)cp);
		ifc->address = (struct prefix *) p1;

		/* Secondary. */
		if (secondary)
			SET_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY);

		/* Add to linked list. */
		listnode_add(ifp->connected, ifc);
	}

	/* This address is configured from zebra. */
	if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
		SET_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED);

	/* In case of this route need to install kernel. */
	if (CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE)
		&& !(if_data && if_data->shutdown == IF_ZEBRA_SHUTDOWN_ON))
	{
		if (! if_is_up (ifp))
		{
			nsm_pal_interface_up(ifp);
			pal_interface_refresh_flag(ifp);
		}
		ret = nsm_pal_interface_set_address (ifp, ifc, secondary);
		if (ret < 0)
		{
			//vty_out (vty, "%% Can't set interface IP address: %s.%s",
			//		safe_strerror(errno), VTY_NEWLINE);
			listnode_delete(ifp->connected, ifc);
			connected_free(ifc);
			return CMD_WARNING;
		}
#ifdef PL_DHCP_MODULE
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
			SET_FLAG(ifc->conf, ZEBRA_IFC_DHCPC);
#endif
		connected_up_ipv6 (ifp, ifc);
		nsm_client_notify_interface_add_ip(ifp, ifc, secondary);
		zebra_interface_address_add_update (ifp, ifc);
		return OK;
	}
	return OK;
}

static int
nsm_interface_ipv6_address_uninstall (struct interface *ifp,
		struct prefix_ipv6 *cp, int secondry)
{
	struct connected *ifc;
	//int ret;
	/* Check current interface address. */
	ifc = connected_check (ifp, (struct prefix *) cp);
	if (! ifc)
	{
		return ERROR;
	}
	/* This is not configured address. */
	if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
		return ERROR;


	zebra_interface_address_delete_update (ifp, ifc);

	nsm_client_notify_interface_del_ip(ifp, ifc, secondry);

	while(ifc->raw_status != 0)
	{
		os_msleep(10);
	}
	/* This is real route. */
/*	ret = pal_kernel_if_prefix_delete_ipv6 (ifp, ifc);
	if (ret < 0)
	{
		vty_out (vty, "%% Can't unset interface IP address: %s.%s",
				safe_strerror(errno), VTY_NEWLINE);
		return CMD_WARNING;
	}*/

	if(ifc->raw_status == 0)
	{
		if(nsm_pal_interface_unset_address(ifp, ifc, secondry) != OK)
		{
			//vty_out(vty, "%% Can't unset interface IP address: %s.%s",
			//		safe_strerror(errno), VTY_NEWLINE);
			return ERROR;
		}
		connected_down_ipv6 (ifp, ifc);

		UNSET_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED);

		if (CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE)) {
			listnode_delete(ifp->connected, ifc);
			connected_free(ifc);
			return OK;
		}
	}
	return ERROR;
}
#endif

/********************************************************************/

int nsm_interface_ip_address_add(struct interface *ifp, struct prefix *cp,
		int secondary, int value)
{
	//int ret;
	struct nsm_interface *if_data;
	struct connected *ifc;
	struct prefix_ipv4 *p1, *p2;
	if_data = ifp->info[MODULE_NSM];
	ifc = connected_check(ifp, (struct prefix *) cp);
	if (!ifc)
	{
		ifc = connected_new();
		ifc->ifp = ifp;
		/* Address. */
		p1 = prefix_new();
		p1->family = cp->family;

		prefix_copy ((struct prefix *)p1, (struct prefix *)cp);
		ifc->address = (struct prefix *) p1;
		if(p1->family == AF_INET)
		{
			/* Broadcast. */
			if (p1->prefixlen <= IPV4_MAX_PREFIXLEN - 2)
			{
				p2 = prefix_new();
				p1->family = cp->family;
				prefix_copy ((struct prefix *)p2, (struct prefix *)cp);
				p2->prefix.s_addr = ipv4_broadcast_addr(p2->prefix.s_addr, p2->prefixlen);
				ifc->destination = (struct prefix *) p2;
			}
		}
		if (!CHECK_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED))
			SET_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED);
#ifdef PL_DHCP_MODULE
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
			SET_FLAG(ifc->conf, ZEBRA_IFC_DHCPC);
#endif
		/* Secondary. */
		if (secondary)
			SET_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY);

		if(cp->family == AF_INET)
			connected_up_ipv4(ifp, ifc);
#ifdef HAVE_IPV6
		else
			connected_up_ipv6(ifp, ifc);
#endif
		nsm_client_notify_interface_add_ip(ifp, ifc, 0);
		zebra_interface_address_add_update (ifp, ifc);
		/* Add to linked list. */
		listnode_add(ifp->connected, ifc);
		return OK;
	}
	else
	{
		return ERROR;
	}
}

int nsm_interface_ip_address_del(struct interface *ifp, struct prefix *cp,
		int secondary, int value)
{
	//int ret;
	//struct nsm_interface *if_data;
	struct connected *ifc;
	//struct prefix_ipv4 *p;
	//if_data = ifp->info[MODULE_NSM];

	/* Check current interface address. */
	ifc = connected_check(ifp, (struct prefix *) cp);
	if (!ifc)
	{
		return ERROR;
	}
	/* This is not configured address. */
	if (!CHECK_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED))
		UNSET_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED);

	zebra_interface_address_delete_update (ifp, ifc);

	nsm_client_notify_interface_del_ip(ifp, ifc, 0);

	if(cp->family == AF_INET)
		connected_down_ipv4(ifp, ifc);
#ifdef HAVE_IPV6
	else
		connected_down_ipv6 (ifp, ifc);
#endif
	UNSET_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED);

	if (CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE))
	{
		listnode_delete(ifp->connected, ifc);
		connected_free(ifc);
		return OK;
	}
	return ERROR;
}

int nsm_interface_address_get_api(struct interface *ifp, struct prefix *address)
{
	struct connected *ifc;
	IF_DATA_LOCK();
	ifc = (struct connected *)listnode_head(ifp->connected);
	if(ifc && ifc->address)
	{
		prefix_copy ((struct prefix *)address, (struct prefix *)ifc->address);
		IF_DATA_UNLOCK();
		return OK;
	}
	IF_DATA_UNLOCK();
	return ERROR;
}
/********************************************************************/

int nsm_interface_mode_set_api(struct interface *ifp, if_mode_t mode)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if(ifp->if_mode != mode)
	{
		ret = nsm_pal_interface_mode(ifp, mode);
		if(ret == OK)
		{
			ifp->if_mode = mode;
			zebra_interface_mode_update (ifp, mode);
		}
	}
	else
		ret = OK;
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_mode_get_api(struct interface *ifp, if_mode_t *mode)
{
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if(mode)
	{
		if(mode)
			*mode = ifp->if_mode;
	}
	IF_DATA_UNLOCK();
	return OK;
}

int nsm_interface_enca_set_api(struct interface *ifp, if_enca_t enca, int value)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if(ifp->if_enca != enca || ifp->encavlan != value)
	{
#ifdef USE_IPSTACK_KERNEL
		if(if_is_ethernet(ifp) &&
				IF_IS_SUBIF_GET(ifp->ifindex) &&
				enca == IF_ENCA_DOT1Q)
		{
			ifindex_t root_kifindex = ifindex2ifkernel(IF_IFINDEX_ROOT_GET(ifp->ifindex));
			const char *root_kname = ifkernelindex2kernelifname(root_kifindex);
			char k_name[64];
			os_memset(k_name, 0, sizeof(k_name));
			if(root_kname)
				sprintf(k_name, "%s.%d", root_kname, value);
			else
				sprintf(k_name, "%s%d%d.%d", getkernelname(ifp->if_type),
				IF_IFINDEX_SLOT_GET(ifp->ifindex), IF_IFINDEX_PORT_GET(ifp->ifindex), value);
			if(os_strlen(k_name))
				if_kname_set(ifp, k_name);
		}
#endif
		ret = nsm_pal_interface_enca(ifp, enca, value);
		if(ret == OK)
		{
			ifp->if_enca = enca;
			ifp->encavlan = value;
			//zebra_interface_mode_update (ifp, enca);
		}
	}
	else
		ret = OK;
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_enca_get_api(struct interface *ifp, if_enca_t *enca, int *value)
{
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if(enca)
	{
		if(enca)
			*enca = ifp->if_enca;
		if(value)
			*enca = ifp->encavlan;
	}
	IF_DATA_UNLOCK();
	return OK;
}

int nsm_interface_up_set_api(struct interface *ifp)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	struct nsm_interface *zif = ifp->info[MODULE_NSM];
	if(zif->shutdown != IF_ZEBRA_SHUTDOWN_OFF)
	{
		ret = nsm_pal_interface_up(ifp);
		if(ret == OK)
		{
			if_addr_wakeup(ifp);
			pal_interface_refresh_flag(ifp);
			if_up(ifp);
			zif->shutdown = IF_ZEBRA_SHUTDOWN_OFF;
			nsm_client_notify_interface_up(ifp);
			zebra_interface_up_update(ifp);
		}
	}
	else
		ret = OK;
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_down_set_api(struct interface *ifp)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	struct nsm_interface *zif = ifp->info[MODULE_NSM];
	if(zif->shutdown != IF_ZEBRA_SHUTDOWN_ON)
	{
		ret = nsm_pal_interface_down(ifp);
		if(ret == OK)
		{
			pal_interface_refresh_flag(ifp);
			if_down(ifp);
			zif->shutdown = IF_ZEBRA_SHUTDOWN_ON;
			zebra_interface_down_update(ifp);
			nsm_client_notify_interface_down(ifp);
		}
	}
	else
		ret = OK;
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_desc_set_api(struct interface *ifp, const char *desc)
{
	IF_DATA_LOCK();
	if(ifp->desc)
		XFREE(MTYPE_IF_DESC, ifp->desc);
	if(desc)
		ifp->desc = XSTRDUP(MTYPE_IF_DESC, desc);
	IF_DATA_UNLOCK();
	return OK;
}

int nsm_interface_linkdetect_set_api(struct interface *ifp, nsm_linkdetect_en linkdetect)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	struct nsm_interface *zif = ifp->info[MODULE_NSM];
	IF_DATA_LOCK();
	if(linkdetect != zif->linkdetect)
	{
		ret =  nsm_pal_interface_linkdetect(ifp, linkdetect);
		if(ret == OK)
			ret |= nsm_interface_linkdetect_set(ifp, linkdetect);
		if(ret == OK)
			nsm_client_notify_parameter_change(ifp);
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_linkdetect_get_api(struct interface *ifp, nsm_linkdetect_en *linkdetect)
{
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	struct nsm_interface *zif = ifp->info[MODULE_NSM];
	IF_DATA_LOCK();

	if(linkdetect)
	{
		*linkdetect = zif->linkdetect;
	}
	IF_DATA_UNLOCK();
	return OK;
}

int nsm_interface_statistics_get_api(struct interface *ifp, struct if_stats *stats)
{
	//IF_DATA_LOCK();
	if(stats)
	{
		nsm_pal_interface_get_statistics(ifp);
		if(stats)
			os_memcpy((struct if_stats *)stats, &(ifp->stats), sizeof(struct if_stats));
	}
	//IF_DATA_UNLOCK();
	return OK;
}

int nsm_interface_address_set_api(struct interface *ifp, struct prefix *cp, int secondry)
{
	int ret = ERROR;
	IF_DATA_LOCK();
	if(cp)
	{
		if(cp->family == AF_INET)
			ret = nsm_interface_ip_address_install(ifp, (struct prefix_ipv4 *)cp);
#ifdef HAVE_IPV6
		else
			ret = nsm_interface_ipv6_address_install(ifp, (struct prefix_ipv6 *)cp, secondry);
#endif
		//if(ret == OK)
		//	ret |= nsm_pal_interface_set_address(ifp, cp, secondry);
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_address_unset_api(struct interface *ifp, struct prefix *cp, int secondry)
{
	int ret = ERROR;
	IF_DATA_LOCK();
	if(cp)
	{
		if(cp->family == AF_INET)
			ret = nsm_interface_ip_address_uninstall(ifp, (struct prefix_ipv4 *)cp);
#ifdef HAVE_IPV6
		else
			ret = nsm_interface_ipv6_address_uninstall(ifp, (struct prefix_ipv6 *)cp, secondry);
#endif

		//if(ret == OK)
		//	ret |= nsm_pal_interface_unset_address(ifp, cp, secondry);
	}
	IF_DATA_UNLOCK();
	return ret;
}


int nsm_interface_multicast_set_api(struct interface *ifp, BOOL enable)
{
	int ret = ERROR;
	struct nsm_interface *if_data;
	u_char	multicast = enable ? IF_ZEBRA_MULTICAST_ON:IF_ZEBRA_MULTICAST_OFF;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if_data = ifp->info[MODULE_NSM];
	if(if_data->multicast != multicast)
	{
		ret = nsm_pal_interface_multicast(ifp, (int)enable);
		if(ret == OK)
			if_data->multicast = multicast;
		if(ret == OK)
			nsm_client_notify_parameter_change(ifp);
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_multicast_get_api(struct interface *ifp, BOOL *enable)
{
	int ret = OK;
	struct nsm_interface *if_data;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if_data = ifp->info[MODULE_NSM];
	if(if_data->multicast == IF_ZEBRA_MULTICAST_ON)
	{
		if(enable)
			*enable = TRUE;
	}
	else if(if_data->multicast == IF_ZEBRA_MULTICAST_OFF)
	{
		if(enable)
			*enable = FALSE;
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_bandwidth_set_api(struct interface *ifp, uint32_t bandwidth)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if(ifp->bandwidth != bandwidth)
	{
		ret = nsm_pal_interface_bandwidth(ifp, (int)bandwidth);
		if(ret == OK)
			ifp->bandwidth = bandwidth;
		if(ret == OK)
			nsm_client_notify_parameter_change(ifp);
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_bandwidth_get_api(struct interface *ifp, uint32_t *bandwidth)
{
	int ret = OK;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if(bandwidth)
		*bandwidth = ifp->bandwidth;
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_vrf_set_api(struct interface *ifp, vrf_id_t vrf_id)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if(ifp->vrf_id != vrf_id)
	{
		ret = nsm_pal_interface_vrf(ifp, (int)vrf_id);
		if(ret == OK)
			ifp->vrf_id = vrf_id;
		if(ret == OK)
			nsm_client_notify_parameter_change(ifp);
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_vrf_get_api(struct interface *ifp, vrf_id_t *vrf_id)
{
	int ret = OK;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if(vrf_id)
		*vrf_id = ifp->vrf_id;
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_metric_set_api(struct interface *ifp, int metric)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if(ifp->metric != metric)
	{
		ret = nsm_pal_interface_metric(ifp, (int)metric);
		if(ret == OK)
			ifp->metric = metric;
		if(ret == OK)
			nsm_client_notify_parameter_change(ifp);
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_metric_get_api(struct interface *ifp, int *metric)
{
	int ret = OK;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if(metric)
		*metric = ifp->metric;
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_mtu_set_api(struct interface *ifp, int mtu)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if(ifp->mtu != mtu)
	{
		if(if_is_tunnel(ifp))
			ret = nsm_tunnel_mtu_set_api(ifp, (int)mtu);
		else
			ret = nsm_pal_interface_mtu(ifp, (int)mtu);
		if(ret == OK)
			ifp->mtu = mtu;
		if(ret == OK)
			nsm_client_notify_parameter_change(ifp);
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_mtu_get_api(struct interface *ifp, int *mtu)
{
	int ret = OK;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	if(mtu)
		*mtu = ifp->mtu;
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_mac_set_api(struct interface *ifp, unsigned char *mac, int maclen)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
/*	if(mac == NULL)
		default mac*/
	ret = nsm_pal_interface_mac(ifp, mac, MIN(INTERFACE_HWADDR_MAX, maclen));
	if(ret == OK)
	{
		ifp->hw_addr_len = MIN(INTERFACE_HWADDR_MAX, maclen);
		os_memcpy(ifp->hw_addr, mac, ifp->hw_addr_len);
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_mac_get_api(struct interface *ifp, unsigned char *mac, int maclen)
{
	int ret = OK;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
/*	if(!ifp->hw_addr_len)
		ifp->hw_addr_len = MIN(INTERFACE_HWADDR_MAX, maclen);*/
	if(mac)
		os_memcpy(mac, ifp->hw_addr, MIN(ifp->hw_addr_len, maclen));
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_duplex_set_api(struct interface *ifp, nsm_duplex_en duplex)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	struct nsm_interface *if_data = ifp->info[MODULE_NSM];
	if(if_data)
	{
		if(if_data->duplex != duplex)
		{
			ret = nsm_pal_interface_duplex(ifp, (int)duplex);
			if(ret == OK)
				if_data->duplex = duplex;
		}
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_duplex_get_api(struct interface *ifp, nsm_duplex_en *duplex)
{
	int ret = OK;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	struct nsm_interface *if_data = ifp->info[MODULE_NSM];
	if(if_data)
	{
		if(duplex)
			*duplex = if_data->duplex;
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_speed_set_api(struct interface *ifp, nsm_speed_en speed)
{
	int ret = ERROR;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	struct nsm_interface *if_data = ifp->info[MODULE_NSM];
	if(if_data)
	{
		if(if_data->speed != speed)
		{
			ret = nsm_pal_interface_speed(ifp, (int)speed);
			if(ret == OK)
				if_data->speed = speed;
		}
	}
	IF_DATA_UNLOCK();
	return ret;
}

int nsm_interface_speed_get_api(struct interface *ifp, nsm_speed_en *speed)
{
	int ret = OK;
	zassert(ifp);
	zassert(ifp->info[MODULE_NSM]);
	IF_DATA_LOCK();
	struct nsm_interface *if_data = ifp->info[MODULE_NSM];
	if(if_data)
	{
		if(speed)
			*speed = if_data->speed;
	}
	IF_DATA_UNLOCK();
	return ret;
}


/* Output prefix string to vty. */
static int prefix_vty_out(struct vty *vty, struct prefix *p)
{
	char str[INET6_ADDRSTRLEN];

	inet_ntop(p->family, &p->u.prefix, str, sizeof(str));
	vty_out(vty, "%s", str);
	return strlen(str);
}

/* Dump if address information to vty. */
static void connected_dump_vty(struct vty *vty, struct connected *connected)
{
	struct prefix *p;

	/* Print interface address. */
	p = connected->address;
	vty_out(vty, "  %s ", prefix_family_str(p));
	prefix_vty_out(vty, p);
	vty_out(vty, "/%d", p->prefixlen);

	/* If there is destination address, print it. */
	if (connected->destination) {
		vty_out(vty, (CONNECTED_PEER(connected) ? " peer " : " broadcast "));
		prefix_vty_out(vty, connected->destination);
	}

	if (CHECK_FLAG(connected->flags, ZEBRA_IFA_SECONDARY))
		vty_out(vty, " secondary");

	if (CHECK_FLAG(connected->flags, ZEBRA_IFA_UNNUMBERED))
		vty_out(vty, " unnumbered");
	vty_out(vty, "%s", VTY_NEWLINE);
}



/* Interface's information print out to vty interface. */
void nsm_interface_show_api(struct vty *vty, struct interface *ifp)
{
	struct connected *connected;
	struct listnode *node;
	struct nsm_interface *nsm_interface;
	nsm_interface = ifp->info[MODULE_NSM];

	vty_out(vty, "Interface %s is ", ifp->name);
	if (if_is_up(ifp)) {
		vty_out(vty, "up, line protocol%s",VTY_NEWLINE);

/*		if (CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION)) {
			if (if_is_running(ifp))
				vty_out(vty, "is up%s", VTY_NEWLINE);
			else
				vty_out(vty, "is down%s", VTY_NEWLINE);
		} else {
			vty_out(vty, "detection is disabled%s", VTY_NEWLINE);
		}*/
	} else {
		vty_out(vty, "down%s", VTY_NEWLINE);
	}

	if(ifp->vrf_id != VRF_DEFAULT)
		vty_out(vty, "  ip forward vrf: %s%s", vrf_vrfid2name(ifp->vrf_id), VTY_NEWLINE);
	//vty_out(vty, "  vrf: %u%s", ifp->vrf_id, VTY_NEWLINE);

	if (ifp->desc)
		vty_out(vty, "  Description: %s%s", ifp->desc,VTY_NEWLINE);
	if (ifp->ifindex == IFINDEX_INTERNAL) {
		vty_out(vty, "  pseudo interface%s", VTY_NEWLINE);
		return;
	} else if (!CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_ACTIVE)) {
		vty_out(vty, "  index %d inactive interface%s", ifp->ifindex, VTY_NEWLINE);
		return;
	}

	vty_out(vty, "  index %d metric %d mtu %d ", ifp->ifindex, ifp->metric,
			ifp->mtu);
#ifdef HAVE_IPV6
	if (ifp->mtu6 != ifp->mtu)
	vty_out (vty, "mtu6 %d ", ifp->mtu6);
#endif 
	vty_out(vty, "%s  flags: %s%s", VTY_NEWLINE, if_flag_dump(ifp->flags),
			VTY_NEWLINE);

	/* Hardware address. */
	vty_out(vty, "  Type: %s%s", if_link_type_str(ifp->ll_type), VTY_NEWLINE);
	if (ifp->hw_addr_len != 0)
	{
		int i;
		if(if_is_pointopoint(ifp))
			vty_out(vty, "  Unspec: ");
		else
			vty_out(vty, "  HWaddr: ");
		for (i = 0; i < ifp->hw_addr_len; i++)
			vty_out(vty, "%s%02x", i == 0 ? "" : ":", ifp->hw_addr[i]);
		vty_out(vty, "%s", VTY_NEWLINE);
	}

	/* Bandwidth in kbps */
	if (ifp->bandwidth != 0) {
		vty_out(vty, "  bandwidth %u kbps", ifp->bandwidth);
		vty_out(vty, "%s", VTY_NEWLINE);
	}

	for (ALL_LIST_ELEMENTS_RO(ifp->connected, node, connected))
	{
		connected_dump_vty(vty, connected);
	}

	/* Statistics print out using proc file system. */
	nsm_interface_statistics_get_api(ifp, NULL);

	vty_out (vty, "    %lu input packets (%lu multicast), %lu bytes, "
			"%lu dropped%s",
			ifp->stats.rx_packets, ifp->stats.rx_multicast,
			ifp->stats.rx_bytes, ifp->stats.rx_dropped, VTY_NEWLINE);

	vty_out (vty, "    %lu input errors, %lu length, %lu overrun,"
			" %lu CRC, %lu frame%s",
			ifp->stats.rx_errors, ifp->stats.rx_length_errors,
			ifp->stats.rx_over_errors, ifp->stats.rx_crc_errors,
			ifp->stats.rx_frame_errors, VTY_NEWLINE);

	vty_out (vty, "    %lu fifo, %lu missed%s", ifp->stats.rx_fifo_errors,
			ifp->stats.rx_missed_errors, VTY_NEWLINE);

	vty_out (vty, "    %lu output packets, %lu bytes, %lu dropped%s",
			ifp->stats.tx_packets, ifp->stats.tx_bytes,
			ifp->stats.tx_dropped, VTY_NEWLINE);

	vty_out (vty, "    %lu output errors, %lu aborted, %lu carrier,"
			" %lu fifo, %lu heartbeat%s",
			ifp->stats.tx_errors, ifp->stats.tx_aborted_errors,
			ifp->stats.tx_carrier_errors, ifp->stats.tx_fifo_errors,
			ifp->stats.tx_heartbeat_errors, VTY_NEWLINE);

	vty_out (vty, "    %lu window, %lu collisions%s",
			ifp->stats.tx_window_errors, ifp->stats.collisions, VTY_NEWLINE);

	vty_out(vty, "%s", VTY_NEWLINE);
	return;
}

void nsm_interface_show_brief_api(struct vty *vty, struct interface *ifp, BOOL status, BOOL *head)
{
	struct connected *connected;
	struct listnode *node;
	struct prefix *p;
	struct nsm_interface *nsm_interface;
	char pstatus[32];
	int offset = 0;
	nsm_interface = ifp->info[MODULE_NSM];
	if(status)
	{
		if(head && *head)
		{
			vty_out(vty, "%-24s %-16s%s", "------------------------",
					"----------------", VTY_NEWLINE);
			vty_out(vty, "%-24s %-16s%s", "Interface", "Protocol/PHY", VTY_NEWLINE);
			vty_out(vty, "%-24s %-16s%s", "------------------------",
					"----------------", VTY_NEWLINE);
			*head = FALSE;
		}
	}
	else
	{
		if(head && *head)
		{
			vty_out(vty, "%-24s %-16s %-16s %-18s %-16s %s", "------------------------",
					"----------------", "----------------", "----------------", "----------------", VTY_NEWLINE);
			vty_out(vty, "%-24s %-16s %-16s %-18s %-16s %s", "Interface", "Protocol/PHY",
					"IP Address", "MAC Address", "VRF Name", VTY_NEWLINE);
			vty_out(vty, "%-24s %-16s %-16s %-18s %-16s %s", "------------------------",
					"----------------", "----------------", "----------------", "----------------", VTY_NEWLINE);
			*head = FALSE;
		}
	}
	os_memset(pstatus, 0, sizeof(pstatus));
	sprintf(pstatus, "%s/%s", if_is_running(ifp) ? "UP":"DOWN",if_is_up(ifp) ? "UP":"DOWN");
	vty_out(vty, "%-24s %-16s", ifp->name, pstatus);
	if(status)
	{
		vty_out(vty, "%s", VTY_NEWLINE);
	}
	else //if(!status)
	{
		offset = 0;
		os_memset(pstatus, 0, sizeof(pstatus));
		for (ALL_LIST_ELEMENTS_RO(ifp->connected, node, connected))
		{
			p = connected->address;
			inet_ntop(p->family, &p->u.prefix, pstatus, sizeof(pstatus));
			offset = strlen(pstatus);
			sprintf(pstatus + offset, "/%d", p->prefixlen);
			break;
		}
		vty_out(vty, " %-16s", pstatus);
		if (ifp->hw_addr_len != 0)
		{
			int i;
			offset = 0;
			os_memset(pstatus, 0, sizeof(pstatus));
			for (i = 0; i < ifp->hw_addr_len; i++)
			{
				sprintf(pstatus + offset, "%s%02x", i == 0 ? "" : ":", ifp->hw_addr[i]);
				offset = strlen(pstatus);
				//vty_out(vty, "%s%02x", i == 0 ? "" : ":", ifp->hw_addr[i]);
			}
			vty_out(vty, " %-18s", pstatus);
		}
		else
		{
			os_memset(pstatus, 0, sizeof(pstatus));
			vty_out(vty, " %-18s", pstatus);
		}
		if(ifp->vrf_id != VRF_DEFAULT)
			vty_out(vty, " %-16s%s", vrf_vrfid2name(ifp->vrf_id), VTY_NEWLINE);
		else
		{
			os_memset(pstatus, 0, sizeof(pstatus));
			vty_out(vty, " %-16s%s", pstatus, VTY_NEWLINE);
		}
	}
	return ;
}

/* Allocate and initialize interface vector. */
void nsm_interface_init(void)
{
	 if_hook_add(nsm_interface_new_hook, nsm_interface_delete_hook);
}
