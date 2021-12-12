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

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "nsm_include.h"


#ifdef ZPL_PAL_MODULE
#include "pal_driver.h"
#endif
#ifdef ZPL_HAL_MODULE
#include "hal_port.h"
#endif


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
#ifdef ZPL_HAL_MODULE
	ret = hal_port_up(ifp->ifindex);
	if(ret != OK)
		return ret;
#endif
	return ret;
}

int nsm_pal_interface_down (struct interface *ifp)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	ret = hal_port_down(ifp->ifindex);
	if(ret != OK)
		return ret;
#endif
	ret = pal_interface_down(ifp);
	if(ret != OK)
		return ret;
	return ret;
}


int nsm_pal_interface_mtu (struct interface *ifp, zpl_uint32 mtu)
{
	int ret = 0;
	ret = pal_interface_set_mtu(ifp, mtu);
	if(ret != OK)
		return ret;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_mtu_set(ifp->ifindex, mtu);
#endif
	return ret;
}

int nsm_pal_interface_metric (struct interface *ifp, zpl_uint32 metric)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_metric_set(ifp->ifindex, metric);
#endif
	return ret;
}

int nsm_pal_interface_vrf (struct interface *ifp, zpl_uint32 vrf)
{
	int ret = 0;
	ret = pal_interface_set_vr(ifp, (vrf_id_t)vrf);
	if(ret != OK)
		return ret;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_vrf_set(ifp->ifindex, vrf);
#endif
	return ret;
}

int nsm_pal_interface_multicast (struct interface *ifp, zpl_uint32 multicast)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_multicast_set(ifp->ifindex, multicast);
#endif
	return ret;
}

int nsm_pal_interface_bandwidth (struct interface *ifp, zpl_uint32 bandwidth)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_bandwidth_set(ifp->ifindex, bandwidth);
#endif
	return ret;
}

int nsm_pal_interface_set_address (struct interface *ifp, struct connected *ifc, zpl_bool secondry)
{
	//printf("%s\r\n", __func__);
	int ret = 0;
	ret = pal_interface_ipv4_add(ifp, ifc);
	if(ret != OK)
		return ret;
	//ret = hal_port_address_set(ifp->ifindex, cp, secondry);
	return ret;
}

int nsm_pal_interface_unset_address (struct interface *ifp, struct connected *ifc, zpl_bool secondry)
{
	int ret = 0;
	//ret = hal_port_address_unset(ifp->ifindex, cp, secondry);
	//if(ret != OK)
	//	return ret;
	ret = pal_interface_ipv4_delete(ifp, ifc);
	return ret;
}

int nsm_pal_interface_mac (struct interface *ifp, zpl_uchar *mac, zpl_uint32 len)
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

int nsm_pal_interface_speed (struct interface *ifp,  zpl_uint32 speed )
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	ret = hal_port_speed_set(ifp->ifindex, speed);
#endif
	return ret;
}



int nsm_pal_interface_mode (struct interface *ifp, zpl_uint32 mode)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_mode_set(ifp->ifindex, mode);
#endif
	return ret;
}


int nsm_pal_interface_enca (struct interface *ifp, zpl_uint32 mode, zpl_uint32 value)
{
	int ret = 0;
	#ifdef ZPL_NSM_SERIAL
	if(if_is_serial(ifp))
		ret = nsm_serial_interface_enca_set_api(ifp, mode);
	else if(if_is_ethernet(ifp) &&
	#else
	if(if_is_ethernet(ifp) &&
	#endif
			IF_IS_SUBIF_GET(ifp->ifindex) &&
			mode == IF_ENCA_DOT1Q &&
			ifp->encavlan != value)
	{
		#ifdef ZPL_NSM_VLANETH
		ret = nsm_vlaneth_interface_vid_set_api(ifp, value);
		#endif
	}
	return ret;
}


int nsm_pal_interface_linkdetect (struct interface *ifp, zpl_uint32 link)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_linkdetect_set(ifp->ifindex, link);
#endif
	return ret;
}

int nsm_pal_interface_stp (struct interface *ifp,  zpl_uint32 stp )
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_stp_set(ifp->ifindex, stp);
	ret = hal_stp_state(ifp->ifindex, stp);
#endif
	return ret;
}

int nsm_pal_interface_loop (struct interface *ifp,  zpl_uint32 loop )
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_loop_set(ifp->ifindex, loop);
#endif
	return ret;
}

int nsm_pal_interface_8021x (struct interface *ifp, zpl_uint32 mode)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	//ret = hal_port_8021x_set(ifp->ifindex, mode);
#endif
	return ret;
}

int nsm_pal_interface_duplex (struct interface *ifp, zpl_uint32 duplex)
{
	int ret = 0;
#ifdef ZPL_HAL_MODULE
	ret = hal_port_duplex_set(ifp->ifindex, duplex);
#endif
	return ret;
}



