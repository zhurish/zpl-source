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
#include "interface.h"
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
	gdos.enable = TRUE;
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}

BOOL nsm_dos_is_enable(void)
{
	return gdos.enable;
}

int nsm_dos_enable_api(dos_type_en type)
{
	int ret = ERROR;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
#ifdef PL_HAL_MODULE
	ret = hal_dos_enable(TRUE, type);
#else
	ret = OK;
#endif
	if(ret == OK)
	{
		switch(type)
		{
		case DOS_IP_LAN_DRIP:
			gdos.ip_lan_drip = TRUE;
			break;
		case TCP_BLAT_DROP:
			gdos.tcp_blat = TRUE;
			break;
		case UDP_BLAT_DROP:
			gdos.udp_blat = TRUE;
			break;
		case TCP_NULLSCAN_DROP:
			gdos.tcp_nullscan = TRUE;
			break;
		case TCP_XMASSCAN_DROP:
			gdos.tcp_xmasscan = TRUE;
			break;
		case TCP_SYNFINSCAN_DROP:
			gdos.tcp_synfinscan = TRUE;
			break;
		case TCP_SYNERROR_DROP:
			gdos.tcp_synerror = TRUE;
			break;
		case TCP_SHORTHDR_DROP:
			gdos.tcp_shorthdr = TRUE;
			break;
		case TCP_FRAGERROR_DROP:
			gdos.tcp_fragerror = TRUE;
			break;
		case ICMPv4_FRAGMENT_DROP:
			gdos.icmpv4_fragment = TRUE;
			break;
		case ICMPv6_FRAGMENT_DROP:
			gdos.icmpv6_fragment = TRUE;
			break;
		case ICMPv4_LONGPING_DROP:
			gdos.icmpv4_longping = TRUE;
			break;
		case ICMPv6_LONGPING_DROP:
			gdos.icmpv6_longping = TRUE;
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
	ret = hal_dos_enable(FALSE, type);
#else
	ret = OK;
#endif
	if(ret == OK)
	{
		switch(type)
		{
		case DOS_IP_LAN_DRIP:
			gdos.ip_lan_drip = FALSE;
			break;
		case TCP_BLAT_DROP:
			gdos.tcp_blat = FALSE;
			break;
		case UDP_BLAT_DROP:
			gdos.udp_blat = FALSE;
			break;
		case TCP_NULLSCAN_DROP:
			gdos.tcp_nullscan = FALSE;
			break;
		case TCP_XMASSCAN_DROP:
			gdos.tcp_xmasscan = FALSE;
			break;
		case TCP_SYNFINSCAN_DROP:
			gdos.tcp_synfinscan = FALSE;
			break;
		case TCP_SYNERROR_DROP:
			gdos.tcp_synerror = FALSE;
			break;
		case TCP_SHORTHDR_DROP:
			gdos.tcp_shorthdr = FALSE;
			break;
		case TCP_FRAGERROR_DROP:
			gdos.tcp_fragerror = FALSE;
			break;
		case ICMPv4_FRAGMENT_DROP:
			gdos.icmpv4_fragment = FALSE;
			break;
		case ICMPv6_FRAGMENT_DROP:
			gdos.icmpv6_fragment = FALSE;
			break;
		case ICMPv4_LONGPING_DROP:
			gdos.icmpv4_longping = FALSE;
			break;
		case ICMPv6_LONGPING_DROP:
			gdos.icmpv6_longping = FALSE;
			break;
		}
	}
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}


int nsm_dos_tcp_hdr_min(u_int size)
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

int nsm_dos_icmpv4_max(u_int size)
{
	int ret = OK;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
#ifdef PL_HAL_MODULE
	if(hal_dos_icmp_size(FALSE, size) == OK)
#endif
		gdos.icmpv4_max = size;
	if(gdos.mutex)
		os_mutex_unlock(gdos.mutex);
	return ret;
}

int nsm_dos_icmpv6_max(u_int size)
{
	int ret = OK;
	if(gdos.mutex)
		os_mutex_lock(gdos.mutex, OS_WAIT_FOREVER);
#ifdef PL_HAL_MODULE
	if(hal_dos_icmp_size(TRUE, size) == OK)
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
