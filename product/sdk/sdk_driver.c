/*
 * b53125_driver.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include <zebra.h>
#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "nsm_vrf.h"
#include "command.h"
#include "nsm_interface.h"
#include "if_manage.h"


#include "nsm_trunk.h"
#include "nsm_vlan.h"
#include "nsm_dos.h"
#include "nsm_mirror.h"
#include "nsm_qos.h"

#include "hal_8021x.h"
#include "hal_dos.h"
#include "hal_mac.h"
#include "hal_mirror.h"
#include "hal_misc.h"
#include "hal_mstp.h"
#include "hal_port.h"
#include "hal_qinq.h"
#include "hal_trunk.h"
#include "hal_vlan.h"


#include "b53_mdio.h"
#include "sdk_driver.h"
//#include "sdk_config.h"
/*************************************************************************/
/****************************    DOS   ***********************************/
static int drv_dos_enable(sdk_driver_t *dev, ospl_bool enable, dos_type_en type)
{
	int ret = 0;
	if(dev && dev->b53_device)
	{
		switch(type)
		{
		case DOS_IP_LAN_DRIP:
			ret = b53125_dos_ip_lan_drip_drop(dev->b53_device, enable);
			break;
		case TCP_BLAT_DROP:
			ret = b53125_dos_tcp_blat_drop(dev->b53_device, enable);
			break;
		case UDP_BLAT_DROP:
			ret = b53125_dos_udp_blat_drop(dev->b53_device, enable);
			break;
		case TCP_NULLSCAN_DROP:
			ret = b53125_dos_tcp_nullscan_drop(dev->b53_device, enable);
			break;
		case TCP_XMASSCAN_DROP:
			ret = b53125_dos_tcp_xmassscan_drop(dev->b53_device, enable);
			break;
		case TCP_SYNFINSCAN_DROP:
			ret = b53125_dos_tcp_synfinscan_drop(dev->b53_device, enable);
			break;
		case TCP_SYNERROR_DROP:
			ret = b53125_dos_tcp_synerror_drop(dev->b53_device, enable);
			break;

		case TCP_SHORTHDR_DROP:
			ret = b53125_dos_tcp_ospl_int16hdr_drop(dev->b53_device, enable);
			break;
		case TCP_FRAGERROR_DROP:
			ret = b53125_dos_tcp_fragerror_drop(dev->b53_device, enable);
			break;
		case ICMPv4_FRAGMENT_DROP:
			ret = b53125_dos_icmpv4_fragment_drop(dev->b53_device, enable);
			break;
		case ICMPv6_FRAGMENT_DROP:
			ret = b53125_dos_icmpv6_fragment_drop(dev->b53_device, enable);
			break;

		case ICMPv4_LONGPING_DROP:
			ret = b53125_dos_icmpv4_longping_drop(dev->b53_device, enable);
			break;
		case ICMPv6_LONGPING_DROP:
			ret = b53125_dos_icmpv6_longping_drop(dev->b53_device, enable);
			break;
		default:
			ret = ERROR;
			break;
		}
		//int b53125_dos_disable_lean(struct b53125_device *dev, ospl_bool enable);
		return ret;
	}
	return ERROR;
}

static int drv_dos_tcp_hdr_size(sdk_driver_t *dev, int size)
{
	if(dev && dev->b53_device)
		return b53125_dos_tcphdr_minsize(dev->b53_device, size);
	return ERROR;
}

static int drv_dos_icmp_maxsize(sdk_driver_t *dev, ospl_bool ipv6, int size)
{
	if(dev && dev->b53_device)
	{
		if(ipv6)
			return b53125_dos_icmp6_maxsize(dev->b53_device, size);
		else
			return b53125_dos_icmp4_maxsize(dev->b53_device, size);
	}
	return ERROR;
}

/****************************    STP/MSTP   ***********************************/
static int drv_stp_state(sdk_driver_t *dev, ifindex_t ifindex, hal_port_stp_state_t state)
{
	if(dev && dev->b53_device)
		return b53125_set_stp_state(dev->b53_device, if_ifindex2phy(ifindex), state);
	return ERROR;
}

static int drv_mstp_enable(sdk_driver_t *dev, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_mstp_enable(dev->b53_device, enable);
	return ERROR;
}

static int drv_mstp_aging_time(sdk_driver_t *dev, int aging)
{
	if(dev && dev->b53_device)
		return b53125_mstp_aging_time(dev->b53_device, aging);
	return ERROR;
}

static int drv_mstp_state(sdk_driver_t *dev, int id, ifindex_t ifindex, hal_port_stp_state_t state)
{
	if(dev && dev->b53_device)
		return b53125_mstp_state(dev->b53_device, id, if_ifindex2phy(ifindex), state);
	return ERROR;
}

static int drv_mstp_bypass(sdk_driver_t *dev, int id, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_mstp_bypass(dev->b53_device, id, enable);
	return ERROR;
}

static int drv_mstp_vlan_id(sdk_driver_t *dev, u16 vid, int id)
{
	if(dev && dev->b53_device)
		return b53125_vlan_mstp_id(dev->b53_device, vid, id);
	return ERROR;
}
/****************************    MAC   ***********************************/

/*
int b53125_fdb_dump(struct b53125_device *priv, int port,
		int (*cb)(u8 *, u16, ospl_bool, void *), void *data);
*/

/******************************** MAC ***************************/
static int drv_mac_add(sdk_driver_t *dev, ifindex_t ifindex, vlan_t vlan, mac_t *mac, int pri)
{
	if(dev && dev->b53_device)
		return b53125_mac_tbl_add(dev->b53_device, if_ifindex2phy(ifindex), mac, vlan);
	return ERROR;
}

static int drv_mac_del(sdk_driver_t *dev, ifindex_t ifindex, vlan_t vlan, mac_t *mac, int pri)
{
	if(dev && dev->b53_device)
		return b53125_mac_tbl_del(dev->b53_device, if_ifindex2phy(ifindex), mac, vlan);
	return ERROR;
}

static int drv_mac_clr(sdk_driver_t *dev, ifindex_t ifindex, vlan_t vlan)
{
	if(dev && dev->b53_device)
	{
		if(if_ifindex2phy(ifindex))
			return b53125_clear_mac_tbl_port(dev->b53_device, if_ifindex2phy(ifindex));
		else if(vlan)
			return b53125_clear_mac_tbl_vlan(dev->b53_device, vlan);
	}
	return ERROR;
}

static int drv_mac_read(sdk_driver_t *dev, ifindex_t ifindex, vlan_t vlan)
{
/*	return b53125_fdb_dump(b53_device, if_ifindex2phy(ifindex),
			int (*cb)(u8 *, u16, ospl_bool, void *), void *data);*/
	return OK;
}


/******************************** trunk ***************************/
static int drv_trunk_enable(sdk_driver_t *dev, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_trunk_mac_base_enable(dev->b53_device, enable);
	return ERROR;
}

static int drv_trunk_mode(sdk_driver_t *dev, hal_trunk_mode_t mode)
{
	if(dev && dev->b53_device)
		return b53125_trunk_mode(dev->b53_device, mode);
	return ERROR;
}

static int drv_trunk_add(sdk_driver_t *dev, ifindex_t ifindex, int id)
{
	if(dev && dev->b53_device)
		return b53125_trunk_add(dev->b53_device, id, if_ifindex2phy(ifindex));
	return ERROR;
}

static int drv_trunk_del(sdk_driver_t *dev, ifindex_t ifindex, int id)
{
	if(dev && dev->b53_device)
		return b53125_trunk_del(dev->b53_device, id, if_ifindex2phy(ifindex));
	return ERROR;
}


/******************************** mirror ***************************/
static int drv_mirror_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(b53125_mirror_enable(dev->b53_device, enable) == OK)
	{
		return b53125_mirror_destination_set(dev->b53_device, if_ifindex2phy(ifindex));
	}
	return ERROR;
}

static int drv_mirror_source_enable(sdk_driver_t *dev, ospl_bool enable, ifindex_t ifindex,
		hal_mirror_mode_t mode, hal_mirror_type_t type)
{
	int ret = -1;
	if(dev && dev->b53_device)
	{
		switch(mode)
		{
		case HAL_MIRROR_SOURCE_PORT:
			if(type >= HAL_MIRROR_EGRESS)
				ret |= b53125_mirror_egress_source(dev->b53_device, if_ifindex2phy(ifindex));
			if(type == HAL_MIRROR_INGRESS || type == HAL_MIRROR_BOTH)
				ret |= b53125_mirror_ingress_source(dev->b53_device, if_ifindex2phy(ifindex));
			break;
		case HAL_MIRROR_SOURCE_MAC:
			break;
		case HAL_MIRROR_SOURCE_DIV:
			break;
		default:
			break;
		}
	}
	return ret;
}

static int drv_mirror_source_filter(sdk_driver_t *dev, ospl_bool enable,
		hal_mirror_filter_t filter, hal_mirror_type_t type, mac_t *mac, mac_t *mac1)
{
	//	int (*sdk_mirror_source_filter_enable_cb) (ospl_bool enable, ospl_bool dst, mac_t *mac, int mode);
	int ret = -1;
	if(dev && dev->b53_device)
	{
		switch(type)
		{
		case HAL_MIRROR_INGRESS:
			if(mac)
				ret |= b53125_mirror_ingress_mac(dev->b53_device, mac);
			ret |= b53125_mirror_ingress_filter(dev->b53_device, filter);
			break;
		case HAL_MIRROR_EGRESS:
			if(mac)
				ret |= b53125_mirror_egress_mac(dev->b53_device, mac);
			ret |= b53125_mirror_egress_filter(dev->b53_device, filter);
			break;
		case HAL_MIRROR_BOTH:
			if(mac)
				ret |= b53125_mirror_ingress_mac(dev->b53_device, mac);
			if(mac1)
				ret |= b53125_mirror_egress_mac(dev->b53_device, mac1);
			ret |= b53125_mirror_ingress_filter(dev->b53_device, filter);
			ret |= b53125_mirror_egress_filter(dev->b53_device, filter);
			break;
		default:
			break;
		}
	}
	return ret;
}


/******************************** Jumbo ***************************/
static int drv_dos_jumbo_size(sdk_driver_t *dev, int size)
{
	if(dev && dev->b53_device)
		return b53125_jumbo_size(dev->b53_device, size);
	return ERROR;
}

static int drv_jumbo_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_jumbo_enable(dev->b53_device, if_ifindex2phy(ifindex), enable);
	return ERROR;
}


/******************************** rate limite ***************************/
//Set the limit and burst size to bucket 0(storm control use bucket1).
static int drv_cpu_limit_rate(sdk_driver_t *dev, int bps, int b)
{
	//1000Mbps/((64B+8B+12B)×8bit)=1.488095pps
	int rate_index = 0;
	rate_index = bps/672;
	if(dev && dev->b53_device)
		return b53125_qos_cpu_rate(dev->b53_device, rate_index);
	return ERROR;
}

static int drv_port_limit_egress_rate(sdk_driver_t *dev, ifindex_t ifindex, ospl_uint32 limitkb, ospl_uint32 burst_size)
{
	//1000Mbps/((64B+8B+12B)×8bit)=1.488095pps
	int burst = 0, cnt = 0;
	int burst_kbyte = burst_size / 8;
    if (burst_size > (500 * 8)) { /* 500 KB */
        return ERROR;
    }
    if (burst_kbyte <= 4) { /* 4KB */
    	burst = DRV_BUCKET_SIZE_4K;
    } else if (burst_kbyte <= 8) { /* 8KB */
    	burst = DRV_BUCKET_SIZE_8K;
    } else if (burst_kbyte <= 16) { /* 16KB */
    	burst = DRV_BUCKET_SIZE_16K;
    } else if (burst_kbyte <= 32) { /* 32KB */
    	burst = DRV_BUCKET_SIZE_32K;
    } else if (burst_kbyte <= 64) { /* 64KB */
    	burst = DRV_BUCKET_SIZE_64K;
    } else if (burst_kbyte <= 500) { /* 500KB */
    	burst = DRV_BUCKET_SIZE_500K;
    }

    /* refresh count  (fixed type)*/
    if(limitkb <= 1792)
    {/* 64KB ~ 1.792MB */
    	cnt = ((limitkb-1) / 64) +1;
    }
    else if (limitkb <= 102400)
    { /* 2MB ~ 100MB */
    	cnt = (limitkb /1024 ) + 27;
    }
    else// if
	{ /* 104MB ~ 1000MB */
    	cnt = (limitkb /8192) + 115;
	}
/*    else
    {
		return ERROR;
	}*/
    if(dev && dev->b53_device)
    	return b53125_qos_egress_rate(dev->b53_device, if_ifindex2phy(ifindex), burst, limitkb);
    return ERROR;
}

static int drv_port_limit_ingress_rate(sdk_driver_t *dev, ifindex_t ifindex, ospl_uint32 limitkb, ospl_uint32 burst_size)
{
	//1000Mbps/((64B+8B+12B)×8bit)=1.488095pps
	int burst = 0, cnt = 0;
	int burst_kbyte = burst_size / 8;
    if (burst_size > (500 * 8)) { /* 500 KB */
        return ERROR;
    }
    if (burst_kbyte <= 4) { /* 4KB */
    	burst = DRV_BUCKET_SIZE_4K;
    } else if (burst_kbyte <= 8) { /* 8KB */
    	burst = DRV_BUCKET_SIZE_8K;
    } else if (burst_kbyte <= 16) { /* 16KB */
    	burst = DRV_BUCKET_SIZE_16K;
    } else if (burst_kbyte <= 32) { /* 32KB */
    	burst = DRV_BUCKET_SIZE_32K;
    } else if (burst_kbyte <= 64) { /* 64KB */
    	burst = DRV_BUCKET_SIZE_64K;
    } else if (burst_kbyte <= 500) { /* 500KB */
    	burst = DRV_BUCKET_SIZE_500K;
    }

    /* refresh count  (fixed type)*/
    if (limitkb <= 1792) { /* 64KB ~ 1.792MB */
    	cnt = ((limitkb-1) / 64) +1;
    } else if (limitkb <= 102400){ /* 2MB ~ 100MB */
    	cnt = (limitkb /1024 ) + 27;
    } else /*if*/ { /* 104MB ~ 1000MB */
    	cnt = (limitkb /8192) + 115;
	} /*else {
		return ERROR;
	}*/
    if(dev && dev->b53_device)
    {
		b53125_qos_buck_mode(dev->b53_device, 0, ospl_true);
		b53125_qos_buck_type(dev->b53_device, 0, 0x3f);
		b53125_qos_ingress_rate_mode(dev->b53_device, if_ifindex2phy(ifindex),  3, ospl_true);

		return b53125_qos_ingress_rate(dev->b53_device, if_ifindex2phy(ifindex), 0, burst, limitkb);
    }
    return ERROR;
}

static int drv_port_strom_rate(sdk_driver_t *dev, ifindex_t ifindex, ospl_uint32 limitkb, ospl_uint32 burst_size)
{
	//1000Mbps/((64B+8B+12B)×8bit)=1.488095pps
	int burst = 0, cnt = 0;
	int burst_kbyte = burst_size / 8;
    if (burst_size > (500 * 8)) { /* 500 KB */
        return ERROR;
    }
    if (burst_kbyte <= 4) { /* 4KB */
    	burst = DRV_BUCKET_SIZE_4K;
    } else if (burst_kbyte <= 8) { /* 8KB */
    	burst = DRV_BUCKET_SIZE_8K;
    } else if (burst_kbyte <= 16) { /* 16KB */
    	burst = DRV_BUCKET_SIZE_16K;
    } else if (burst_kbyte <= 32) { /* 32KB */
    	burst = DRV_BUCKET_SIZE_32K;
    } else if (burst_kbyte <= 64) { /* 64KB */
    	burst = DRV_BUCKET_SIZE_64K;
    } else if (burst_kbyte <= 500) { /* 500KB */
    	burst = DRV_BUCKET_SIZE_500K;
    }

    /* refresh count  (fixed type)*/
    if (limitkb <= 1792) { /* 64KB ~ 1.792MB */
    	cnt = ((limitkb-1) / 64) +1;
    } else if (limitkb <= 102400){ /* 2MB ~ 100MB */
    	cnt = (limitkb /1024 ) + 27;
    } else /*if*/{ /* 104MB ~ 1000MB */
    	cnt = (limitkb /8192) + 115;
	}/* else {
		return ERROR;
	}*/
    if(dev && dev->b53_device)
    {
		b53125_qos_buck_mode(dev->b53_device, 1, ospl_true);
		b53125_qos_buck_type(dev->b53_device, 1, 0x3f);

		return b53125_qos_ingress_rate(dev->b53_device, if_ifindex2phy(ifindex), 1, burst, limitkb);
    }
    return ERROR;
}

static int drv_port_strom_mode(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable, int mode)
{
	if(dev && dev->b53_device)
		return b53125_qos_ingress_rate_mode(dev->b53_device, if_ifindex2phy(ifindex),  mode, enable);
	return ERROR;
}

/******************************** qos ***************************/
static int drv_qos_ipg(sdk_driver_t *dev, ospl_bool tx, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_qos_ingress_ipg(dev->b53_device, tx, enable);
	return ERROR;
}

static int drv_qos_enable(sdk_driver_t *dev, ospl_bool enable)
{
	if(dev && dev->b53_device)
	{
		b53125_qos_base_port(dev->b53_device, ospl_false);
		return b53125_qos_layer_sel(dev->b53_device, 3);
	}
	return ERROR;
}

static int drv_qos_8021q_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_qos_8021p(dev->b53_device, if_ifindex2phy(ifindex), enable);
	return ERROR;
}

static int drv_qos_diffserv_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_qos_diffserv(dev->b53_device, if_ifindex2phy(ifindex), enable);
	return ERROR;
}

static int drv_qos_port_map_queue(sdk_driver_t *dev, ifindex_t ifindex, nsm_qos_priority_e pri, nsm_qos_queue_e queue)
{
	if(dev && dev->b53_device)
		return b53125_qos_port_map_queue(dev->b53_device, if_ifindex2phy(ifindex), pri - 1, queue - 1);
	return ERROR;
}

static int drv_qos_diffserv_queue(sdk_driver_t *dev, int diffserv, nsm_qos_queue_e queue)
{
	if(dev && dev->b53_device)
		return b53125_qos_diffserv_map_queue(dev->b53_device, diffserv, queue - 1);
	return ERROR;
}

static int drv_qos_queue_class(sdk_driver_t *dev, nsm_qos_queue_e queue, nsm_qos_class_e class)
{
	if(dev && dev->b53_device)
		return b53125_qos_queue_map_class(dev->b53_device, queue - 1, class - 1);
	return ERROR;
}

static int drv_qos_class_scheduling(sdk_driver_t *dev, nsm_qos_class_e class, nsm_class_sched_t mode)
{
	if(!dev || !dev->b53_device)
		return ERROR;
	int sched_mode = 0;
	if(mode == NSM_CLASS_SCHED_STRICT)
	{
		if(class == NSM_QOS_CLASS_0)
		{
			sched_mode = 3;
		}
		else if(class == NSM_QOS_CLASS_1)
		{
			sched_mode = 3;
		}
		else if(class == NSM_QOS_CLASS_2)
		{
			sched_mode = 2;
		}
		else if(class == NSM_QOS_CLASS_3)
		{
			sched_mode = 1;
		}
		else if(class == NSM_QOS_CLASS_4)
		{
			return b53125_qos_class4_weight(dev->b53_device, ospl_true,  -1);
		}
	}
	else
		sched_mode = 0;
	if(class == NSM_QOS_CLASS_4)
	{
		return b53125_qos_class4_weight(dev->b53_device, ospl_false,  -1);
	}
	/*
	00 = all queues are weighted round robin
	01 = COS3 is strict priority, COS2-COS0 are weighted round
	robin.
	10 = COS3 and COS2 is strict priority, COS1-COS0 are weighted
	round robin.
	11 = COS3, COS2, COS1 and COS0 are in strict priority.
	 */
	return b53125_qos_class_scheduling(dev->b53_device, sched_mode);
}

static int drv_qos_class_weight(sdk_driver_t *dev, nsm_qos_class_e class, int weight)
{
	if(dev && dev->b53_device)
	{
		if(class == NSM_QOS_CLASS_4)
			return b53125_qos_class4_weight(dev->b53_device, ospl_false,  weight);
		return b53125_qos_class_weight(dev->b53_device, class - 1, weight);
	}
	return ERROR;
}


/******************************** vlan ***************************/
static int drv_vlan_enable(sdk_driver_t *dev, ospl_bool enable)
{
	if(dev && dev->b53_device)
	return b53125_enable_vlan(dev->b53_device,  enable,
			ospl_false);
	return ERROR;
}

static int drv_vlan_create(sdk_driver_t *dev, vlan_t vlan)
{
	if(dev && dev->b53_device)
	return b53125_add_vlan_entry(dev->b53_device, vlan);
	return ERROR;
}

static int drv_vlan_destroy(sdk_driver_t *dev, vlan_t vlan)
{
	if(dev && dev->b53_device)
	return b53125_del_vlan_entry(dev->b53_device, vlan);
	return ERROR;
}

static int drv_vlan_add_untag_port(sdk_driver_t *dev, ifindex_t ifindex, vlan_t vlan)
{
	if(dev && dev->b53_device)
	return b53125_add_vlan_port(dev->b53_device, vlan, if_ifindex2phy(ifindex), ospl_false);
	return ERROR;
}

static int drv_vlan_del_untag_port(sdk_driver_t *dev, ifindex_t ifindex, vlan_t vlan)
{
	if(dev && dev->b53_device)
	return b53125_del_vlan_port(dev->b53_device, vlan, if_ifindex2phy(ifindex), ospl_false);
	return ERROR;
}

static int drv_vlan_add_tag_port(sdk_driver_t *dev, ifindex_t ifindex, vlan_t vlan)
{
	if(dev && dev->b53_device)
	return b53125_add_vlan_port(dev->b53_device, vlan, if_ifindex2phy(ifindex), ospl_true);
	return ERROR;
}

static int drv_vlan_del_tag_port(sdk_driver_t *dev, ifindex_t ifindex, vlan_t vlan)
{
	if(dev && dev->b53_device)
	return b53125_del_vlan_port(dev->b53_device, vlan, if_ifindex2phy(ifindex), ospl_true);
	return ERROR;
}

static int drv_port_add_allowed_tag_vlan(sdk_driver_t *dev, ifindex_t ifindex, vlan_t vlan)
{
	if(dev && dev->b53_device)
	return b53125_add_vlan_port(dev->b53_device, vlan, if_ifindex2phy(ifindex), ospl_true);
	return ERROR;
}

static int drv_port_del_allowed_tag_vlan(sdk_driver_t *dev, ifindex_t ifindex, vlan_t vlan)
{
	if(dev && dev->b53_device)
	return b53125_del_vlan_port(dev->b53_device, vlan, if_ifindex2phy(ifindex), ospl_true);
	return ERROR;
}

static int drv_port_add_allowed_tag_batch_vlan(sdk_driver_t *dev, ifindex_t ifindex, vlan_t start, vlan_t end)
{
	if(!dev || !dev->b53_device)
		return ERROR;
	ospl_uint32 i = 0, ret = 0;
	for(i = start; i <= end; i++)
	{
		ret |= b53125_add_vlan_port(dev->b53_device, i, if_ifindex2phy(ifindex), ospl_true);
		if(ret != 0)
			break;
	}
	return ret;
}

static int drv_port_del_allowed_tag_batch_vlan(sdk_driver_t *dev, ifindex_t ifindex, vlan_t start, vlan_t end)
{
	if(!dev || !dev->b53_device)
		return ERROR;
	ospl_uint32 i = 0, ret = 0;
	for(i = start; i <= end; i++)
	{
		ret |= b53125_del_vlan_port(dev->b53_device, i, if_ifindex2phy(ifindex), ospl_true);
		if(ret != 0)
			break;
	}
	return ret;
}

static int drv_port_add_native_vlan(sdk_driver_t *dev, ifindex_t ifindex, vlan_t vlan)
{
	if(dev && dev->b53_device)
	return b53125_port_pvid(dev->b53_device, vlan, if_ifindex2phy(ifindex), 0);
	return ERROR;
}

static int drv_port_del_native_vlan(sdk_driver_t *dev, ifindex_t ifindex, vlan_t vlan)
{
	if(dev && dev->b53_device)
	return b53125_port_pvid(dev->b53_device, 1, if_ifindex2phy(ifindex), 0);
	return ERROR;
}

/*static int drv_port_vlan_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(dev && dev->b53_device)
	return b53125_port_vlan(dev->b53_device, if_ifindex2phy(ifindex), enable);
	return ERROR;
}*/

static int drv_port_vlan_default(sdk_driver_t *dev, ifindex_t ifindex, u16 vid)
{
	if(dev && dev->b53_device)
	return b53125_port_pvid(dev->b53_device, vid, if_ifindex2phy(ifindex), 0);
	return ERROR;
}

/*

int b53125_vlan_double_tagging_tpid(struct b53125_device *dev, u16 tpid)

int b53125_ISP_port(struct b53125_device *dev, int port, ospl_bool enable)
*/


/******************************** global ***************************/
static int drv_switch_manage(sdk_driver_t *dev, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_switch_manege(dev->b53_device, enable);
	return ERROR;
}
static int drv_switch_forwarding(sdk_driver_t *dev, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_switch_forwarding(dev->b53_device, enable);
	return ERROR;
}
static int drv_multicast_flood(sdk_driver_t *dev, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_multicast_flood(dev->b53_device, enable);
	return ERROR;
}
static int drv_unicast_flood(sdk_driver_t *dev, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_unicast_flood(dev->b53_device, enable);
	return ERROR;
}

static int drv_multicast_learning(sdk_driver_t *dev, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_multicast_learning(dev->b53_device, enable);
	return ERROR;
}

static int drv_enable_bpdu(sdk_driver_t *dev, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_enable_bpdu(dev->b53_device, enable);
	return ERROR;
}

static int drv_aging_time(sdk_driver_t *dev, int enable)
{
	if(dev && dev->b53_device)
		return b53125_aging_time(dev->b53_device, enable);
	return ERROR;
}

//int b53125_bpdu_forward(struct b53125_device *dev, u8 *mac, ospl_bool enable);

/******************************** cpu port ***************************/
static int drv_global_imp_enable(sdk_driver_t *dev, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_imp_enable(dev->b53_device, enable);
	return ERROR;
}

static int drv_imp_speed(sdk_driver_t *dev, ospl_uint32 speed)
{
	if(dev && dev->b53_device)
		return b53125_imp_speed(dev->b53_device, speed);
	return ERROR;
}

static int drv_imp_duplex(sdk_driver_t *dev, ospl_uint32 duplex)
{
	if(dev && dev->b53_device)
		return b53125_imp_duplex(dev->b53_device, duplex);
	return ERROR;
}

static int drv_imp_flow(sdk_driver_t *dev, ospl_bool rx, ospl_bool tx)
{
	if(dev && dev->b53_device)
		return b53125_imp_flow(dev->b53_device, rx, tx);
	return ERROR;
}

/******************************** port ***************************/
static int drv_port_protected_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_port_protected_enable(dev->b53_device, if_ifindex2phy(ifindex), enable);
	return ERROR;
}

/*
static int drv_port_pasue_transmit_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_pasue_transmit_enable(dev->b53_device, if_ifindex2phy(ifindex), enable);
	return ERROR;
}

static int drv_port_pasue_receive_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_pasue_receive_enable(dev->b53_device, if_ifindex2phy(ifindex), enable);
	return ERROR;
}

static int drv_port_pasue_pass_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool rx)
{
	if(dev && dev->b53_device)
	{
		if(rx)
			return b53125_pause_pass_rx(dev->b53_device, if_ifindex2phy(ifindex), ospl_true);
		else
			return b53125_pause_pass_tx(dev->b53_device, if_ifindex2phy(ifindex), ospl_true);
	}
	return ERROR;
}

static int drv_port_pasue_pass_disable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool rx)
{
	if(dev && dev->b53_device)
	{
		if(rx)
			return b53125_pause_pass_rx(dev->b53_device, if_ifindex2phy(ifindex), ospl_false);
		else
			return b53125_pause_pass_tx(dev->b53_device, if_ifindex2phy(ifindex), ospl_false);
	}
	return ERROR;
}
*/



static int drv_port_wan_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_port_wan_enable(dev->b53_device, if_ifindex2phy(ifindex), enable);
	return ERROR;
}


static int drv_port_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_port_enable(dev->b53_device, if_ifindex2phy(ifindex), enable);
	return ERROR;
}

static int drv_port_learning_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_enable_learning(dev->b53_device, if_ifindex2phy(ifindex), enable);
	return ERROR;
}

static int drv_port_software_learning_enable(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_software_learning(dev->b53_device, if_ifindex2phy(ifindex), enable);
	return ERROR;
}

static ospl_bool drv_port_get_link(sdk_driver_t *dev, ifindex_t ifindex)
{
	if(dev && dev->b53_device)
		return b53125_port_get_link(dev->b53_device, if_ifindex2phy(ifindex));
	return ospl_false;
}

static ospl_uint32 drv_port_get_speed(sdk_driver_t *dev, ifindex_t ifindex)
{
	if(dev && dev->b53_device)
		return b53125_port_get_speed(dev->b53_device, if_ifindex2phy(ifindex));
	return ospl_false;
}
static ospl_uint32 drv_port_get_duplex(sdk_driver_t *dev, ifindex_t ifindex)
{
	if(dev && dev->b53_device)
		return b53125_port_get_duplex(dev->b53_device, if_ifindex2phy(ifindex));
	return ospl_false;
}

static int drv_port_speed(sdk_driver_t *dev, ifindex_t ifindex, ospl_uint32 value)
{
	if(dev && dev->b53_device)
		return b53125_port_set_speed(dev->b53_device, if_ifindex2phy(ifindex), value);
	return ERROR;
}

static int drv_port_duplex(sdk_driver_t *dev, ifindex_t ifindex, ospl_uint32 value)
{
	if(dev && dev->b53_device)
		return b53125_port_set_duplex(dev->b53_device, if_ifindex2phy(ifindex), value);
	return ERROR;
}

static int drv_port_flow(sdk_driver_t *dev, ifindex_t ifindex, ospl_uint32 value)
{
	if(dev && dev->b53_device)
		return b53125_port_set_flow(dev->b53_device, if_ifindex2phy(ifindex), value);
	return ERROR;
}

static int drv_port_set_link(sdk_driver_t *dev, ifindex_t ifindex, ospl_bool enable)
{
	if(dev && dev->b53_device)
		return b53125_port_set_link_force(dev->b53_device, if_ifindex2phy(ifindex), enable);
	return ERROR;
}


static sdk_dos_t sdk_dos_drv = {
	drv_dos_enable,
	drv_dos_tcp_hdr_size,
	drv_dos_icmp_maxsize,
	NULL,
};

static sdk_mstp_t sdk_mstp_drv = {
		drv_mstp_enable,
		drv_mstp_aging_time,
		drv_mstp_bypass,
		drv_mstp_state,
		drv_mstp_vlan_id,
		drv_stp_state,
		NULL,
};

static sdk_mac_t sdk_mac_drv = {
		NULL,
		drv_mac_add,
		drv_mac_del,
		drv_mac_clr,
		drv_mac_read,
		NULL,
};


static sdk_trunk_t sdk_trunk_drv = {
		drv_trunk_enable,
		drv_trunk_mode,
		drv_trunk_add,
		drv_trunk_del,
		NULL,
};

static sdk_mirror_t sdk_mirror_drv = {
		drv_mirror_enable,
		drv_mirror_source_enable,
		drv_mirror_source_filter,
		NULL,
};

static sdk_misc_t sdk_misc_drv = {
		drv_jumbo_enable,
		drv_dos_jumbo_size,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
};


static sdk_vlan_t sdk_vlan_drv = {
		drv_vlan_enable,
		drv_vlan_create,
		drv_vlan_destroy,
		NULL,
		NULL,
		drv_vlan_add_untag_port,
		drv_vlan_del_untag_port,
		drv_vlan_add_tag_port,
		drv_vlan_del_tag_port,

		drv_port_add_native_vlan,
		drv_port_del_native_vlan,

		drv_port_add_allowed_tag_vlan,
		drv_port_del_allowed_tag_vlan,

		drv_port_add_allowed_tag_batch_vlan,
		drv_port_del_allowed_tag_batch_vlan,

		drv_port_vlan_default,
		NULL,
		NULL,
};

/*static sdk_qinq_t sdk_qinq_drv = {
		drv_jumbo_enable,
		drv_dos_jumbo_size,
		NULL,
		NULL,
};*/


static sdk_qos_t sdk_qos_drv = {
		drv_qos_enable,
		drv_qos_ipg,
		NULL,

		drv_qos_8021q_enable,
		drv_qos_diffserv_enable,

		drv_qos_port_map_queue,
		drv_qos_diffserv_queue,

		drv_qos_queue_class,
		drv_qos_class_scheduling,
		drv_qos_class_weight,

		//风暴
		drv_port_strom_mode,
		drv_port_strom_rate,

		//端口限速
		drv_port_limit_egress_rate,
		drv_port_limit_ingress_rate,


		//CPU
		drv_cpu_limit_rate,

		//remarking
		NULL,
		NULL,
		NULL,
};

static sdk_port_t sdk_port_drv = {
		drv_port_enable,//sdk_port_enable_cb
		drv_port_set_link,//sdk_port_link_cb
		drv_port_speed,//sdk_port_speed_cb
		drv_port_duplex,//sdk_port_duplex_cb
		drv_port_flow,//sdk_port_flow_cb
		drv_jumbo_enable,//sdk_port_jumbo_cb

		drv_port_get_link,//sdk_port_state_get_cb
		drv_port_get_speed,//sdk_port_speed_get_cb
		drv_port_get_duplex,//sdk_port_duplex_get_cb


		NULL,//sdk_port_8021x_cb

		drv_port_learning_enable,//sdk_port_learning_enable_cb
		drv_port_software_learning_enable,//sdk_port_swlearning_enable_cb
		drv_port_protected_enable,//sdk_port_protected_enable_cb
		drv_port_wan_enable,//sdk_port_wan_enable_cb
		NULL,//sdk_port_mac_cb
		NULL,//sdk_port_mtu_cb
		NULL,//sdk_port_vrf_cb,
		NULL,//sdk_port_mode_cb
		NULL,
};


static sdk_global_t sdk_global_drv = {
		drv_switch_manage,//sdk_switch_mode_cb
		drv_switch_forwarding,//sdk_switch_forward_cb
		drv_multicast_flood,//sdk_multicast_flood_cb
		drv_unicast_flood,//sdk_unicast_flood_cb
		drv_multicast_learning,//sdk_multicast_learning_cb
		drv_enable_bpdu,//sdk_bpdu_enable_cb
		drv_aging_time,//sdk_aging_time_cb,
};

static sdk_cpu_t sdk_cpu_drv = {
		NULL,//sdk_cpu_mode_cb
		drv_global_imp_enable,//sdk_cpu_enable_cb
		drv_imp_speed,//sdk_cpu_speed_cb
		drv_imp_duplex,//sdk_cpu_duplex_cb,
		drv_imp_flow,//sdk_cpu_flow_cb
};




int sdk_module_init(hal_driver_t *hal)
{
	sdk_driver_t *sdk;
	if(hal->driver)
		return OK;
	hal->driver = malloc(sizeof(sdk_driver_t));
	if(hal->driver == NULL)
	{
		zlog_debug(MODULE_HAL, " Can not malloc b53125 device");
		return ERROR;
	}
	memset(hal->driver, 0, sizeof(sdk_driver_t));
	sdk = hal->driver;
#ifdef PL_SDK_BCM53125

	sdk->b53_device = b53125_mdio_probe();
	if(sdk->b53_device)
	{
		//hal->driver = sdk->b53_device;
		if(b53125_config_init(sdk->b53_device) == OK)
		{
			hal->global_tbl = &sdk_global_drv;
			hal->cpu_tbl = &sdk_cpu_drv;

			hal->dos_tbl = &sdk_dos_drv;
			hal->mstp_tbl = &sdk_mstp_drv;
			hal->mac_tbl = &sdk_mac_drv;
			hal->trunk_tbl = &sdk_trunk_drv;
			hal->mirror_tbl = &sdk_mirror_drv;
			hal->misc_tbl = &sdk_misc_drv;
			hal->vlan_tbl = &sdk_vlan_drv;
			hal->qos_tbl = &sdk_qos_drv;
			hal->port_tbl = &sdk_port_drv;

/*			sdk_8021x_t		*q8021x_tbl;
			sdk_port_t		*port_tbl;
			sdk_qinq_t		*qinq_tbl;*/
		}
	}
#endif
	//sdk_vlan_init();
	return OK;
}

