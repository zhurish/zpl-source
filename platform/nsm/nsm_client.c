/* Zebra's client library.
 * Copyright (C) 1999 Kunihiro Ishiguro
 * Copyright (C) 2005 Andrew J. Schorr
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "zebra.h"
#include "buffer.h"
#include "if.h"
#include "log.h"
#include "memory.h"
#include "network.h"
#include "prefix.h"
#include "stream.h"
#include "table.h"
#include "thread.h"
#include "vty.h"
#include "zserv.h"
#include "nsm_client.h"

#include "pal_interface.h"
#ifdef PL_HAL_MODULE
#include "hal_port.h"
#endif
static struct list *nsmlist;


static int nsm_client_cmp(struct nsm_client *p1, struct nsm_client *p2)
{
//	int if_cmp_func(struct interface *ifp1, struct interface *ifp2)
	if(p1->module > p2->module)
		return 1;
	else if(p1->module < p2->module)
		return -1;
	else
		return 0;
}

/* Allocate zclient structure. */
struct nsm_client * nsm_client_new (void)
{
  struct nsm_client *client;
  client = XCALLOC (MTYPE_ZCLIENT, sizeof (struct nsm_client));
  return client;
}

/* This function is only called when exiting, because
   many parts of the code do not check for I/O errors, so they could
   reference an invalid pointer if the structure was ever freed.

   Free zclient structure. */
void nsm_client_free (struct nsm_client *client)
{
	if(client)
	{
		listnode_delete (nsmlist, client);
		XFREE (MTYPE_ZCLIENT, client);
	}
}

/* Initialize zebra client.  Argument redist_default is unwanted
   redistribute route type. */
void nsm_client_install (struct nsm_client *client, int module)
{
	client->module = module;
	listnode_add_sort(nsmlist, client);
	//listnode_add (nsmlist, client);
}

struct nsm_client * nsm_client_lookup (int module)
{
	struct listnode *node = NULL;
	struct nsm_client *client = NULL;
	for (ALL_LIST_ELEMENTS_RO(nsmlist, node, client))
	{
		if(client->module == module)
			return client;
	}
	return NULL;
}

void nsm_client_init (void)
{
	nsmlist = list_new();
	nsmlist->cmp =  nsm_client_cmp;
}


int nsm_client_write_config (int module, struct vty *vty)
{
	int ret = 0;
	struct listnode *node;
	struct nsm_client *client;
	for (ALL_LIST_ELEMENTS_RO(nsmlist, node, client))
	{
		if(client->write_config_cb)
		{
			ret = 0;
			if(module)
			{
				if(client->module == module)
					ret = (client->write_config_cb)(vty);
				if(ret)
					vty_out(vty, "!%s", VTY_NEWLINE);
			}
			else
			{
				ret = (client->write_config_cb)(vty);
				if(ret)
					vty_out(vty, "!%s", VTY_NEWLINE);
			}
		}
	}
	return ret;
}

int nsm_client_service_write_config (int module, struct vty *vty)
{
	int ret = 0;
	struct listnode *node;
	struct nsm_client *client;
	for (ALL_LIST_ELEMENTS_RO(nsmlist, node, client))
	{
		if(client->service_write_config_cb)
		{
			ret = 0;
			if(module)
			{
				if(client->module == module)
					ret = (client->service_write_config_cb)(vty);
				if(ret)
					vty_out(vty, "!%s", VTY_NEWLINE);
			}
			else
			{
				ret = (client->service_write_config_cb)(vty);
				if(ret)
					vty_out(vty, "!%s", VTY_NEWLINE);
			}
		}
	}
	return ret;
}

int nsm_client_debug_write_config (int module, struct vty *vty)
{
	int ret = 0;
	struct listnode *node;
	struct nsm_client *client;
	for (ALL_LIST_ELEMENTS_RO(nsmlist, node, client))
	{
		if(client->debug_write_config_cb)
		{
			if(module)
			{
				if(client->module == module)
					ret |= (client->debug_write_config_cb)(vty);
			}
			else
				ret |= (client->debug_write_config_cb)(vty);
		}
	}
	return ret;
}

int nsm_client_interface_write_config (int module, struct vty *vty, struct interface *ifp)
{
	int ret = 0;
	struct listnode *node;
	struct nsm_client *client;
	for (ALL_LIST_ELEMENTS_RO(nsmlist, node, client))
	{
		if(client->interface_write_config_cb)
		{
			if(module)
			{
				if(client->module == module)
					ret |= (client->interface_write_config_cb)(vty, ifp);
			}
			else
				ret |= (client->interface_write_config_cb)(vty, ifp);
		}
	}
	return ret;
}


int nsm_client_notify_interface_add(struct interface *ifp)
{
	int ret = 0;
	struct listnode *node;
	struct nsm_client *client;
	for (ALL_LIST_ELEMENTS_RO(nsmlist, node, client))
	{
		if(client->notify_add_cb)
		{
			ret |= (client->notify_add_cb)(ifp);
		}
	}
	return ret;
}

int nsm_client_notify_interface_delete (struct interface *ifp)
{
	int ret = 0;
	struct listnode *node;
	struct nsm_client *client;
	for (ALL_LIST_ELEMENTS_RO(nsmlist, node, client))
	{
		if(client->notify_delete_cb)
		{
			ret |= (client->notify_delete_cb)(ifp);
		}
	}
	return ret;
}

int nsm_client_notify_interface_up (struct interface *ifp)
{
	int ret = 0;
	struct listnode *node;
	struct nsm_client *client;
	for (ALL_LIST_ELEMENTS_RO(nsmlist, node, client))
	{
		if(client->notify_up_cb)
		{
			ret |= (client->notify_up_cb)(ifp);
		}
	}
	return ret;
}

int nsm_client_notify_interface_down (struct interface *ifp)
{
	int ret = 0;
	struct listnode *node;
	struct nsm_client *client;
	for (ALL_LIST_ELEMENTS_RO(nsmlist, node, client))
	{
		if(client->notify_down_cb)
		{
			ret |= (client->notify_down_cb)(ifp);
		}
	}
	return ret;
}

int nsm_client_notify_interface_add_ip (struct interface *ifp, struct connected *ifc, int sec)
{
	int ret = 0;
	struct listnode *node;
	struct nsm_client *client;
	for (ALL_LIST_ELEMENTS_RO(nsmlist, node, client))
	{
		if(client->notify_address_add_cb)
		{
			ret |= (client->notify_address_add_cb)(ifp, ifc, sec);
		}
	}
	return ret;
}

int nsm_client_notify_interface_del_ip (struct interface *ifp, struct connected *ifc, int sec)
{
	int ret = 0;
	struct listnode *node;
	struct nsm_client *client;
	for (ALL_LIST_ELEMENTS_RO(nsmlist, node, client))
	{
		if(client->notify_address_del_cb)
		{
			ret |= (client->notify_address_del_cb)(ifp, ifc, sec);
		}
	}
	return ret;
}

int nsm_client_notify_parameter_change (struct interface *ifp)
{
	int ret = 0;
	struct listnode *node;
	struct nsm_client *client;
	for (ALL_LIST_ELEMENTS_RO(nsmlist, node, client))
	{
		if(client->notify_parameter_change_cb)
		{
			ret |= (client->notify_parameter_change_cb)(ifp);
		}
	}
	return ret;
}


int nsm_pal_interface_add(struct interface *ifp)
{
	int ret = 0;
	if(os_strlen(ifp->k_name))
	{
		ret = pal_interface_create(ifp);
		if(ret != OK)
			return ret;
	}
	return ret;
}

int nsm_pal_interface_delete (struct interface *ifp)
{
	int ret = 0;
	ret = pal_interface_destroy(ifp);
	if(ret != OK)
		return ret;
	return ret;
}

int nsm_pal_interface_up (struct interface *ifp)
{
	int ret = 0;
	ret = pal_interface_up(ifp);
	if(ret != OK)
		return ret;
#ifdef PL_HAL_MODULE
	ret = hal_port_up(ifp->ifindex);
	if(ret != OK)
		return ret;
#endif
	return ret;
}

int nsm_pal_interface_down (struct interface *ifp)
{
	int ret = 0;
#ifdef PL_HAL_MODULE
	ret = hal_port_down(ifp->ifindex);
	if(ret != OK)
		return ret;
#endif
	ret = pal_interface_down(ifp);
	if(ret != OK)
		return ret;
	return ret;
}


int nsm_pal_interface_mtu (struct interface *ifp, int mtu)
{
	int ret = 0;
	ret = pal_interface_set_mtu(ifp, mtu);
	if(ret != OK)
		return ret;
#ifdef PL_HAL_MODULE
	ret = hal_port_mtu_set(ifp->ifindex, mtu);
#endif
	return ret;
}

int nsm_pal_interface_metric (struct interface *ifp, int metric)
{
	int ret = 0;
#ifdef PL_HAL_MODULE
	ret = hal_port_metric_set(ifp->ifindex, metric);
#endif
	return ret;
}

int nsm_pal_interface_vrf (struct interface *ifp, int vrf)
{
	int ret = 0;
	ret = pal_interface_set_vr(ifp, (vrf_id_t)vrf);
	if(ret != OK)
		return ret;
#ifdef PL_HAL_MODULE
	ret = hal_port_vrf_set(ifp->ifindex, vrf);
#endif
	return ret;
}

int nsm_pal_interface_multicast (struct interface *ifp, int multicast)
{
	int ret = 0;
#ifdef PL_HAL_MODULE
	ret = hal_port_multicast_set(ifp->ifindex, multicast);
#endif
	return ret;
}

int nsm_pal_interface_bandwidth (struct interface *ifp, int bandwidth)
{
	int ret = 0;
#ifdef PL_HAL_MODULE
	ret = hal_port_bandwidth_set(ifp->ifindex, bandwidth);
#endif
	return ret;
}

int nsm_pal_interface_set_address (struct interface *ifp, struct prefix *cp, int secondry)
{
	//printf("%s\r\n", __func__);
	int ret = 0;
	ret = pal_interface_ipv4_add(ifp, cp);
	if(ret != OK)
		return ret;
	//ret = hal_port_address_set(ifp->ifindex, cp, secondry);
	return ret;
}

int nsm_pal_interface_unset_address (struct interface *ifp, struct prefix *cp, int secondry)
{
	int ret = 0;
	//ret = hal_port_address_unset(ifp->ifindex, cp, secondry);
	//if(ret != OK)
	//	return ret;
	ret = pal_interface_ipv4_delete(ifp, cp);
	return ret;
}

int nsm_pal_interface_mac (struct interface *ifp, unsigned char *mac, int len)
{
	int ret = 0;
	ret = pal_interface_set_lladdr(ifp, mac, len);
	if(ret != OK)
		return ret;
	//ret = hal_port_mac_set(ifp->ifindex, mac, len);
	return ret;
}

int nsm_pal_interface_get_statistics (struct interface *ifp)
{
	int ret = 0;
	ret = pal_interface_update_statistics(ifp);
	if(ret != OK)
		return ret;
	return ret;
}

int nsm_pal_interface_speed (struct interface *ifp,  int speed )
{
	int ret = 0;
#ifdef PL_HAL_MODULE
	ret = hal_port_speed_set(ifp->ifindex, speed);
#endif
	return ret;
}



int nsm_pal_interface_mode (struct interface *ifp, int mode)
{
	int ret = 0;
#ifdef PL_HAL_MODULE
	ret = hal_port_mode_set(ifp->ifindex, mode);
#endif
	return ret;
}


int nsm_pal_interface_enca (struct interface *ifp, int mode, int value)
{
	int ret = 0;
	if(if_is_serial(ifp))
		ret = nsm_serial_interface_enca_set_api(ifp, mode);
	else if(if_is_ethernet(ifp) &&
			IF_IS_SUBIF_GET(ifp->ifindex) &&
			mode == IF_ENCA_DOT1Q &&
			ifp->encavlan != value)
	{
		ret = nsm_veth_interface_vid_set_api(ifp, value);
	}
	return ret;
}


int nsm_pal_interface_linkdetect (struct interface *ifp, int link)
{
	int ret = 0;
#ifdef PL_HAL_MODULE
	ret = hal_port_linkdetect_set(ifp->ifindex, link);
#endif
	return ret;
}

int nsm_pal_interface_stp (struct interface *ifp,  int stp )
{
	int ret = 0;
#ifdef PL_HAL_MODULE
	ret = hal_port_stp_set(ifp->ifindex, stp);
#endif
	return ret;
}

int nsm_pal_interface_loop (struct interface *ifp,  int loop )
{
	int ret = 0;
#ifdef PL_HAL_MODULE
	ret = hal_port_loop_set(ifp->ifindex, loop);
#endif
	return ret;
}

int nsm_pal_interface_8021x (struct interface *ifp, int mode)
{
	int ret = 0;
#ifdef PL_HAL_MODULE
	ret = hal_port_8021x_set(ifp->ifindex, mode);
#endif
	return ret;
}

int nsm_pal_interface_duplex (struct interface *ifp, int duplex)
{
	int ret = 0;
#ifdef PL_HAL_MODULE
	ret = hal_port_duplex_set(ifp->ifindex, duplex);
#endif
	return ret;
}



