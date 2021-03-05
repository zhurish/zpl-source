/*
 * nsm_dos.c
 *
 *  Created on: May 8, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "nsm_interface.h"
#include <log.h>
#include "nsm_client.h"
#include "os_list.h"

#include "nsm_dos.h"
#ifdef PL_HAL_MODULE
#include "hal_dos.h"
#endif
static Gl2dos_t gdos;


int nsm_dos_init()
{
	memset(&gdos, 0, sizeof(gdos));
	gdos.mutex = os_mutex_init();
	return OK;
}

int nsm_dos_exit()
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
	gdos.enable = ospl_true;
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}

ospl_bool nsm_dos_is_enable(void)
{
	return gdos.enable;
}

int nsm_dos_enable_api(dos_type_en type)
{
	int ret = ERROR;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
#ifdef PL_HAL_MODULE
	ret = hal_dos_enable(ospl_true, type);
#else
	ret = OK;
#endif
	if(ret == OK)
	{
		switch(type)
		{
		case DOS_IP_LAN_DRIP:
			gdos.ip_lan_drip = ospl_true;
			break;
		case TCP_BLAT_DROP:
			gdos.tcp_blat = ospl_true;
			break;
		case UDP_BLAT_DROP:
			gdos.udp_blat = ospl_true;
			break;
		case TCP_NULLSCAN_DROP:
			gdos.tcp_nullscan = ospl_true;
			break;
		case TCP_XMASSCAN_DROP:
			gdos.tcp_xmasscan = ospl_true;
			break;
		case TCP_SYNFINSCAN_DROP:
			gdos.tcp_synfinscan = ospl_true;
			break;
		case TCP_SYNERROR_DROP:
			gdos.tcp_synerror = ospl_true;
			break;
		case TCP_SHORTHDR_DROP:
			gdos.tcp_ospl_int16hdr = ospl_true;
			break;
		case TCP_FRAGERROR_DROP:
			gdos.tcp_fragerror = ospl_true;
			break;
		case ICMPv4_FRAGMENT_DROP:
			gdos.icmpv4_fragment = ospl_true;
			break;
		case ICMPv6_FRAGMENT_DROP:
			gdos.icmpv6_fragment = ospl_true;
			break;
		case ICMPv4_LONGPING_DROP:
			gdos.icmpv4_longping = ospl_true;
			break;
		case ICMPv6_LONGPING_DROP:
			gdos.icmpv6_longping = ospl_true;
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
#ifdef PL_HAL_MODULE
	ret = hal_dos_enable(ospl_false, type);
#else
	ret = OK;
#endif
	if(ret == OK)
	{
		switch(type)
		{
		case DOS_IP_LAN_DRIP:
			gdos.ip_lan_drip = ospl_false;
			break;
		case TCP_BLAT_DROP:
			gdos.tcp_blat = ospl_false;
			break;
		case UDP_BLAT_DROP:
			gdos.udp_blat = ospl_false;
			break;
		case TCP_NULLSCAN_DROP:
			gdos.tcp_nullscan = ospl_false;
			break;
		case TCP_XMASSCAN_DROP:
			gdos.tcp_xmasscan = ospl_false;
			break;
		case TCP_SYNFINSCAN_DROP:
			gdos.tcp_synfinscan = ospl_false;
			break;
		case TCP_SYNERROR_DROP:
			gdos.tcp_synerror = ospl_false;
			break;
		case TCP_SHORTHDR_DROP:
			gdos.tcp_ospl_int16hdr = ospl_false;
			break;
		case TCP_FRAGERROR_DROP:
			gdos.tcp_fragerror = ospl_false;
			break;
		case ICMPv4_FRAGMENT_DROP:
			gdos.icmpv4_fragment = ospl_false;
			break;
		case ICMPv6_FRAGMENT_DROP:
			gdos.icmpv6_fragment = ospl_false;
			break;
		case ICMPv4_LONGPING_DROP:
			gdos.icmpv4_longping = ospl_false;
			break;
		case ICMPv6_LONGPING_DROP:
			gdos.icmpv6_longping = ospl_false;
			break;
		}
	}
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}


int nsm_dos_tcp_hdr_min(ospl_uint32 size)
{
	int ret = OK;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
#ifdef PL_HAL_MODULE
	if(hal_dos_tcp_hdr_size(size) == OK)
#endif
		gdos.tcp_hdr_min = size;
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}

int nsm_dos_icmpv4_max(ospl_uint32 size)
{
	int ret = OK;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
#ifdef PL_HAL_MODULE
	if(hal_dos_icmp_size(ospl_false, size) == OK)
#endif
		gdos.icmpv4_max = size;
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}

int nsm_dos_icmpv6_max(ospl_uint32 size)
{
	int ret = OK;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
#ifdef PL_HAL_MODULE
	if(hal_dos_icmp_size(ospl_true, size) == OK)
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
