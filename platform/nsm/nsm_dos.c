/*
 * nsm_dos.c
 *
 *  Created on: May 8, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"
#include "vrf.h"
#include "prefix.h"
#include "vty.h"
#include "zmemory.h"
#include "template.h"
#include "nsm_dos.h"

#ifdef ZPL_HAL_MODULE
#include "hal_dos.h"
#endif
static Gl2dos_t gdos;

static int nsm_dos_config_write(struct vty *vty, void *p);

int nsm_dos_init(void)
{
	template_t * temp = NULL;
	memset(&gdos, 0, sizeof(gdos));
	gdos.mutex = os_mutex_init();
	temp = lib_template_new (zpl_true);
	if(temp)
	{
		temp->module = 0;
		strcpy(temp->name, "dos");
		//strcpy(temp->prompt, "service-mqtt"); /* (config-app-esp)# */
		temp->write_template = nsm_dos_config_write;
		temp->pVoid = NULL;
		lib_template_config_list_install(temp, 0);
	}
	return OK;
}

int nsm_dos_exit(void)
{
	if(gdos.mutex)
		os_mutex_exit(gdos.mutex);
	memset(&gdos, 0, sizeof(gdos));
	return OK;
}

int nsm_dos_enable(void)
{
	int ret = OK;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
	gdos.enable = zpl_true;
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}

zpl_bool nsm_dos_is_enable(void)
{
	return gdos.enable;
}

int nsm_dos_enable_api(dos_type_en type)
{
	int ret = ERROR;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
	ret = hal_dos_enable(zpl_true, type);
#else
	ret = OK;
#endif
	if(ret == OK)
	{
		switch(type)
		{
		case DOS_IP_LAN_DRIP:
			gdos.ip_lan_drip = zpl_true;
			break;
		case TCP_BLAT_DROP:
			gdos.tcp_blat = zpl_true;
			break;
		case UDP_BLAT_DROP:
			gdos.udp_blat = zpl_true;
			break;
		case TCP_NULLSCAN_DROP:
			gdos.tcp_nullscan = zpl_true;
			break;
		case TCP_XMASSCAN_DROP:
			gdos.tcp_xmasscan = zpl_true;
			break;
		case TCP_SYNFINSCAN_DROP:
			gdos.tcp_synfinscan = zpl_true;
			break;
		case TCP_SYNERROR_DROP:
			gdos.tcp_synerror = zpl_true;
			break;
		case TCP_SHORTHDR_DROP:
			gdos.tcp_shorthdr = zpl_true;
			break;
		case TCP_FRAGERROR_DROP:
			gdos.tcp_fragerror = zpl_true;
			break;
		case ICMPv4_FRAGMENT_DROP:
			gdos.icmpv4_fragment = zpl_true;
			break;
		case ICMPv6_FRAGMENT_DROP:
			gdos.icmpv6_fragment = zpl_true;
			break;
		case ICMPv4_LONGPING_DROP:
			gdos.icmpv4_longping = zpl_true;
			break;
		case ICMPv6_LONGPING_DROP:
			gdos.icmpv6_longping = zpl_true;
			break;
		}
	}
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}

int nsm_dos_disable_api(dos_type_en type)
{
	int ret = ERROR;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
	ret = hal_dos_enable(zpl_false, type);
#else
	ret = OK;
#endif
	if(ret == OK)
	{
		switch(type)
		{
		case DOS_IP_LAN_DRIP:
			gdos.ip_lan_drip = zpl_false;
			break;
		case TCP_BLAT_DROP:
			gdos.tcp_blat = zpl_false;
			break;
		case UDP_BLAT_DROP:
			gdos.udp_blat = zpl_false;
			break;
		case TCP_NULLSCAN_DROP:
			gdos.tcp_nullscan = zpl_false;
			break;
		case TCP_XMASSCAN_DROP:
			gdos.tcp_xmasscan = zpl_false;
			break;
		case TCP_SYNFINSCAN_DROP:
			gdos.tcp_synfinscan = zpl_false;
			break;
		case TCP_SYNERROR_DROP:
			gdos.tcp_synerror = zpl_false;
			break;
		case TCP_SHORTHDR_DROP:
			gdos.tcp_shorthdr = zpl_false;
			break;
		case TCP_FRAGERROR_DROP:
			gdos.tcp_fragerror = zpl_false;
			break;
		case ICMPv4_FRAGMENT_DROP:
			gdos.icmpv4_fragment = zpl_false;
			break;
		case ICMPv6_FRAGMENT_DROP:
			gdos.icmpv6_fragment = zpl_false;
			break;
		case ICMPv4_LONGPING_DROP:
			gdos.icmpv4_longping = zpl_false;
			break;
		case ICMPv6_LONGPING_DROP:
			gdos.icmpv6_longping = zpl_false;
			break;
		}
	}
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}


int nsm_dos_tcp_hdr_min(zpl_uint32 size)
{
	int ret = OK;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
	if(hal_dos_tcp_hdr_size(size) == OK)
#endif
		gdos.tcp_hdr_min = size;
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}

int nsm_dos_icmpv4_max(zpl_uint32 size)
{
	int ret = OK;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
	if(hal_dos_icmp_size(zpl_false, size) == OK)
#endif
		gdos.icmpv4_max = size;
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}

int nsm_dos_icmpv6_max(zpl_uint32 size)
{
	int ret = OK;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
#ifdef ZPL_HAL_MODULE
	if(hal_dos_icmp_size(zpl_true, size) == OK)
#endif
		gdos.icmpv6_max = size;
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}



int nsm_dos_callback_api(l2dos_cb cb, void *pVoid)
{
	int ret = OK;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);

	if(cb)
		ret = (cb)(&gdos, pVoid);
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}

static int nsm_dos_config_write(struct vty *vty, void *p)
{
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
	if(gdos.enable)
	{
		if(gdos.icmpv6_longping)
			vty_out(vty, "ip icmpv6 long-ping%s", VTY_NEWLINE);
		if(gdos.icmpv4_longping)
			vty_out(vty, "ip icmpv4 long-ping%s", VTY_NEWLINE);
		if(gdos.icmpv6_fragment)
			vty_out(vty, "ip icmpv6 fragment%s", VTY_NEWLINE);
		if(gdos.icmpv4_fragment)
			vty_out(vty, "ip icmpv4 fragment%s", VTY_NEWLINE);
		if(gdos.tcp_fragerror)
			vty_out(vty, "ip tcp frag-error%s", VTY_NEWLINE);
		if(gdos.tcp_shorthdr)
			vty_out(vty, "ip tcp short-hdr%s", VTY_NEWLINE);
		if(gdos.tcp_synerror)
			vty_out(vty, "ip tcp syn-errorr%s", VTY_NEWLINE);
		if(gdos.tcp_synfinscan)
			vty_out(vty, "ip tcp synfin-scan%s", VTY_NEWLINE);
		if(gdos.tcp_xmasscan)
			vty_out(vty, "ip tcp xmas-scan%s", VTY_NEWLINE);
		if(gdos.tcp_nullscan)
			vty_out(vty, "ip tcp null-scan%s", VTY_NEWLINE);
		if(gdos.udp_blat)
			vty_out(vty, "ip udp blat%s", VTY_NEWLINE);
		if(gdos.tcp_blat)
			vty_out(vty, "ip tcp blat%s", VTY_NEWLINE);
		if(gdos.ip_lan_drip)
			vty_out(vty, "ip lan-drip%s", VTY_NEWLINE);

		if(gdos.tcp_hdr_min)
			vty_out(vty, "ip tcp-hdr min-size %d%s", gdos.tcp_hdr_min, VTY_NEWLINE);
		if(gdos.icmpv4_max)
			vty_out(vty, "ip icmpv4 max-size %d%s", gdos.icmpv4_max, VTY_NEWLINE);
		if(gdos.icmpv6_max)
			vty_out(vty, "ip icmpv4 max-size %d%s", gdos.icmpv6_max, VTY_NEWLINE);
	}
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return 1;
}