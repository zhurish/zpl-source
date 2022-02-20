/*
 * b53125_driver.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include <zpl_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"

#define B53_BRCM_OUI_1	0x0143bc00
#define B53_BRCM_OUI_2	0x03625c00
#define B53_BRCM_OUI_3	0x00406000
#define B53_BRCM_OUI_4	0x01410c00

struct b53_chip_data {
	u32 chip_id;
	const char *dev_name;
	u16 vlans;
	u16 enabled_ports;
	u8 cpu_port;
	u8 vta_regs[3];
	u8 arl_entries;
	u8 arl_bins;
	u16 arl_buckets;
	u8 duplex_reg;
	u8 jumbo_pm_reg;
	u8 jumbo_size_reg;
};

#define B53_VTA_REGS	\
	{ B53_VT_ACCESS, B53_VT_INDEX, B53_VT_ENTRY }
#define B53_VTA_REGS_9798 \
	{ B53_VT_ACCESS_9798, B53_VT_INDEX_9798, B53_VT_ENTRY_9798 }
#define B53_VTA_REGS_63XX \
	{ B53_VT_ACCESS_63XX, B53_VT_INDEX_63XX, B53_VT_ENTRY_63XX }

static const struct b53_chip_data b53_switch_chips[] = {
	{
		.chip_id = BCM53115_DEVICE_ID,
		.dev_name = "BCM53115",
		.vlans = 4096,
		.enabled_ports = 0x1f,
		.arl_entries = 4,
		.vta_regs = B53_VTA_REGS,
		.cpu_port = B53_CPU_PORT,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
	},
	{
		.chip_id = BCM53125_DEVICE_ID,
		.dev_name = "BCM53125",
		.vlans = 4096,
		.enabled_ports = 0xff,
		.arl_entries = 4,
		.arl_bins = 4,
		.arl_buckets = 1024,
		.cpu_port = B53_CPU_PORT,
		.vta_regs = B53_VTA_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		
	},
	{
		.chip_id = BCM53128_DEVICE_ID,
		.dev_name = "BCM53128",
		.vlans = 4096,
		.enabled_ports = 0x1ff,
		.arl_entries = 4,
		.cpu_port = B53_CPU_PORT,
		.vta_regs = B53_VTA_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
	},
};

#define ARRAY_SIZE(x) (int)(sizeof(x) / sizeof(x[0]))

b53_device_t * b53125_device_probe()
{
	u32 phy_id = 0;
	zpl_uint32 i = 0;
	b53_device_t* b53_device = malloc(sizeof(b53_device_t));
	if(b53_device == NULL)
	{
		_sdk_err( " Can not malloc b53125 device");
		return NULL;
	}
	memset(b53_device, 0, sizeof(b53_device_t));
	//b53_device->fd = ipstack_socket(IPSTACK_AF_INET, IPSTACK_SOCK_DGRAM, 0);
	b53_device->mido.fd = open("/dev/"B53_DEVICE_NAME, O_RDWR);
	if(b53_device->mido.fd <= 0)
	{
		free(b53_device);
		_sdk_err( "Can not open b53125 device '%s'", B53_DEVICE_NAME);
		return NULL;
	}
	b53125_mdio_probe(&b53_device->mido);

	phy_id = __mdio_read(&b53_device->mido, 0, 2) << 16;
	phy_id |= __mdio_read(&b53_device->mido, 0, 3);
	//============b53125_device_probe============phy_id=0x3625f24
	_sdk_debug("b53125 device probe phy_id=0x%x", phy_id);
	/* BCM5325, BCM539x (OUI_1)
	 * BCM53125, BCM53128 (OUI_2)
	 * BCM5365 (OUI_3)
#define B53_BRCM_OUI_1	0x0143bc00
#define B53_BRCM_OUI_2	0x03625c00
#define B53_BRCM_OUI_3	0x00406000
#define B53_BRCM_OUI_4	0x01410c00
	 */

	if ((phy_id & 0xfffffc00) != B53_BRCM_OUI_1 &&
	    (phy_id & 0xfffffc00) != B53_BRCM_OUI_2 &&
	    (phy_id & 0xfffffc00) != B53_BRCM_OUI_3 &&
	    (phy_id & 0xfffffc00) != B53_BRCM_OUI_4) {
		free(b53_device);
		_sdk_err( "Unsupported device: 0x%08x", phy_id);
		return NULL;
	}
	b53_device->mido.reg_page = 0xff;
	b53125_read32(b53_device, B53_MGMT_PAGE, B53_DEVICE_ID, &b53_device->chip_id);
	b53125_read8(b53_device, B53_MGMT_PAGE, B53_REV_ID,  &b53_device->core_rev);

	for (i = 0; i < ARRAY_SIZE(b53_switch_chips); i++)
	{
		const struct b53_chip_data *chip = &b53_switch_chips[i];
		if (chip->chip_id == b53_device->chip_id)
		{
			if (!b53_device->enabled_ports)
				b53_device->enabled_ports = chip->enabled_ports;
			b53_device->duplex_reg = chip->duplex_reg;
			b53_device->vta_regs[0] = chip->vta_regs[0];
			b53_device->vta_regs[1] = chip->vta_regs[1];
			b53_device->vta_regs[2] = chip->vta_regs[2];
			b53_device->jumbo_pm_reg = chip->jumbo_pm_reg;
			b53_device->cpu_port = chip->cpu_port;
			b53_device->num_vlans = chip->vlans;
			b53_device->num_arl_entries = chip->arl_entries;
			b53_device->num_arl_bins = chip->arl_bins;
			b53_device->num_arl_buckets = chip->arl_buckets;
			
			if (b53_device->chip_id == BCM53115_DEVICE_ID)
			{
				u64 strap_value;
				b53125_read48(b53_device, B53_STAT_PAGE, B53_STRAP_VALUE, &strap_value);
				if (strap_value & SV_GMII_CTRL_115)
					b53_device->cpu_port = 5;

				_sdk_notice( "Find b53115 device on '%s' ID:0x%x REV:0x%x", B53_DEVICE_NAME,
						b53_device->chip_id, b53_device->core_rev);
			}
			if (b53_device->chip_id == BCM53125_DEVICE_ID)
				_sdk_notice( "Find b53125 device on '%s' ID:0x%x REV:0x%x", B53_DEVICE_NAME,
					b53_device->chip_id, b53_device->core_rev);

			b53_device->num_ports = b53_device->cpu_port + 1;
			b53_device->enabled_ports |= BIT(b53_device->cpu_port);

			return b53_device;
		}
	}
	_sdk_err( "Can not find device by ID'0x%x'", b53_device->chip_id);
	return NULL;
}

int b53125_config_init(sdk_driver_t *dev)
{
	int ret = 0;
	if(dev == NULL)
	{
		return ERROR;
	}
	/*******global *******/
	ret |= b53125_switch_manege(dev, zpl_true);//设置为managed mode
	b53_brcm_hdr_setup(dev, zpl_true, ((b53_device_t *)dev->sdk_device)->cpu_port);

	ret |= b53125_imp_enable(dev, zpl_true);//关闭IMP接口
	ret |= b53125_switch_forwarding(dev, zpl_false);//禁止转发

	ret |= b53125_enable_learning(dev, ((b53_device_t *)dev->sdk_device)->cpu_port, zpl_true);
	ret |= b53125_enable_learning(dev, 0, zpl_true);
	ret |= b53125_enable_learning(dev, 1, zpl_true);
	ret |= b53125_enable_learning(dev, 2, zpl_true);
	ret |= b53125_enable_learning(dev, 3, zpl_true);
	ret |= b53125_enable_learning(dev, 4, zpl_true);
	ret |= b53125_enable_learning(dev, 6, zpl_true);

	ret |= b53125_multicast_flood(dev, zpl_true);//使能多播泛洪
	ret |= b53125_unicast_flood(dev, zpl_true);//使能单播泛洪

	ret |= b53125_multicast_learning(dev, zpl_true);//使能多播报文学习源MAC

	ret |= b53125_enable_bpdu(dev, zpl_true);

	ret |= b53125_switch_forwarding(dev, zpl_true);//使能转发
	

	ret = b53125_port_init();
	_sdk_debug( "b53125 port init %s", (ret == OK)?"OK":"ERROR");
	ret = b53_vlan_init();
	_sdk_debug( "b53125 vlan init %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_mstp_init();
	_sdk_debug( "b53125 mstp init %s", (ret == OK)?"OK":"ERROR");
	ret = b53_trunk_init();
	_sdk_debug( "b53125 trunk init %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_mirror_init();
	_sdk_debug( "b53125 mirror init %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_dos_init();
	_sdk_debug( "b53125 dos init %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_eap_init();
	_sdk_debug( "b53125 eap init %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_qos_init();
	_sdk_debug( "b53125 qos init %s", (ret == OK)?"OK":"ERROR");
	return ret;
}

#if 0
/******************************** trunk ***************************/
static int drv_trunk_enable(zpl_bool enable)
{
	return b53125_trunk_mac_base_enable(b53_device, enable);
}

static int drv_trunk_mode(drv_trunk_mode_t mode)
{
	return b53125_trunk_mode(b53_device, mode);
}

static int drv_trunk_add(ifindex_t ifindex, int id)
{
	return b53125_trunk_add(b53_device, id, if_ifindex2phy(ifindex));
}

static int drv_trunk_del(ifindex_t ifindex, int id)
{
	return b53125_trunk_del(b53_device, id, if_ifindex2phy(ifindex));
}

/******************************** STP/MSTP ***************************/
static int drv_stp_state(ifindex_t ifindex, drv_port_stp_state_t state)
{
	return b53125_set_stp_state(b53_device, if_ifindex2phy(ifindex), state);
}

static int drv_mstp_enable(zpl_bool enable)
{
	return b53125_mstp_enable(b53_device, enable);
}

static int drv_mstp_aging_time(int value)
{
	return b53125_mstp_aging_time(b53_device, value);
}

static int drv_mstp_state(ifindex_t ifindex, int id, drv_port_stp_state_t state)
{
	return b53125_mstp_state(b53_device, id, if_ifindex2phy(ifindex), state + 1);
}

static int drv_mstp_id_vlan(int id, vlan_t vlan)
{
	return b53125_vlan_mstp_id(b53_device, vlan, id);
}

/******************************** mirror ***************************/
static int drv_mirror_enable(ifindex_t ifindex, zpl_bool enable)
{
	if(b53125_mirror_enable(b53_device, enable) == OK)
	{
		return b53125_mirror_destination_set(b53_device, if_ifindex2phy(ifindex));
	}
	return ERROR;
}

static int drv_mirror_source_enable(zpl_bool enable, ifindex_t ifindex,
		drv_mirror_mode_t mode, drv_mirror_type_t type)
{
	int ret = -1;
	switch(mode)
	{
	case DRV_MIRROR_SOURCE_PORT:
		if(type >= DRV_MIRROR_EGRESS)
			ret |= b53125_mirror_egress_source(b53_device, if_ifindex2phy(ifindex));
		if(type == DRV_MIRROR_INGRESS || type == DRV_MIRROR_BOTH)
			ret |= b53125_mirror_ingress_source(b53_device, if_ifindex2phy(ifindex));
		break;
	case DRV_MIRROR_SOURCE_MAC:
		break;
	case DRV_MIRROR_SOURCE_DIV:
		break;
	default:
		break;
	}
	return ret;
}

static int drv_mirror_source_filter(zpl_bool enable,
		drv_mirror_filter_t filter, drv_mirror_type_t type, mac_t *mac, mac_t *mac1)
{
	//	int (*sdk_mirror_source_filter_enable_cb) (zpl_bool enable, zpl_bool dst, mac_t *mac, int mode);
	int ret = -1;
	switch(type)
	{
	case DRV_MIRROR_INGRESS:
		if(mac)
			ret |= b53125_mirror_ingress_mac(b53_device, mac);
		ret |= b53125_mirror_ingress_filter(b53_device, filter);
		break;
	case DRV_MIRROR_EGRESS:
		if(mac)
			ret |= b53125_mirror_egress_mac(b53_device, mac);
		ret |= b53125_mirror_egress_filter(b53_device, filter);
		break;
	case DRV_MIRROR_BOTH:
		if(mac)
			ret |= b53125_mirror_ingress_mac(b53_device, mac);
		if(mac1)
			ret |= b53125_mirror_egress_mac(b53_device, mac1);
		ret |= b53125_mirror_ingress_filter(b53_device, filter);
		ret |= b53125_mirror_egress_filter(b53_device, filter);
		break;
	default:
		break;
	}
	return ret;
}
/******************************** DOS ***************************/
static int drv_dos_enable(zpl_bool enable, drv_dos_type_t type)
{
	int ret = -1;
	switch(type)
	{
	case DRV_DOS_IP_LAN_DRIP:
		ret = b53125_dos_ip_lan_drip_drop(b53_device, enable);
		break;
	case DRV_DOS_TCP_BLAT:
		ret = b53125_dos_tcp_blat_drop(b53_device, enable);
		break;
	case DRV_DOS_UDP_BLAT:
		ret = b53125_dos_udp_blat_drop(b53_device, enable);
		break;
	case DRV_DOS_TCP_NULL_SCAN:
		ret = b53125_dos_tcp_nullscan_drop(b53_device, enable);
		break;
	case DRV_DOS_TCP_XMASS_SCAN:
		ret = b53125_dos_tcp_xmassscan_drop(b53_device, enable);
		break;
	case DRV_DOS_TCP_SYN_FIN_SCAN:
		ret = b53125_dos_tcp_synfinscan_drop(b53_device, enable);
		break;
	case DRV_DOS_TCP_SYN_ERROR:
		ret = b53125_dos_tcp_synerror_drop(b53_device, enable);
		break;
	case DRV_DOS_TCP_SHORT_HDR:
		ret = b53125_dos_tcp_zpl_int16hdr_drop(b53_device, enable);
		break;
	case DRV_DOS_TCP_FRAG_ERROR:
		ret = b53125_dos_tcp_fragerror_drop(b53_device, enable);
		break;
	case DRV_DOS_ICMPV4_FRAGMENTS:
		ret = b53125_dos_icmpv4_fragment_drop(b53_device, enable);
		break;
	case DRV_DOS_ICMPV6_FRAGMENTS:
		ret = b53125_dos_icmpv6_fragment_drop(b53_device, enable);
		break;
	case DRV_DOS_ICMPV4_LONG_PING:
		ret = b53125_dos_icmpv4_longping_drop(b53_device, enable);
		break;
	case DRV_DOS_ICMPV6_LONG_PING:
		ret = b53125_dos_icmpv6_longping_drop(b53_device, enable);
		break;
	default:
		break;
	}
	return ret;
}

static int drv_dos_tcp_hdr_size(int size)
{
	return b53125_dos_tcphdr_minsize(b53_device, size);
}
static int drv_dos_icmp_size(zpl_bool ipv6, int size)
{
    if(ipv6)
    	return b53125_dos_icmp6_maxsize(b53_device, size);
    else
    	return b53125_dos_icmp4_maxsize(b53_device, size);
}

static int drv_dos_disable_lean(zpl_bool enable)
{
	return b53125_dos_disable_lean(b53_device, enable);
}
/******************************** Jumbo ***************************/
static int drv_dos_jumbo_size(int size)
{
   return b53125_jumbo_size(b53_device, size);
}

static int drv_jumbo_enable(ifindex_t ifindex, zpl_bool enable)
{
	return b53125_jumbo_enable(b53_device, if_ifindex2phy(ifindex), enable);
}
/******************************** MAC ***************************/
static int drv_mac_add(ifindex_t ifindex, vlan_t vlan, mac_t *mac, int pri)
{
	return b53125_mac_tbl_add(b53_device, if_ifindex2phy(ifindex), mac, vlan);
}

static int drv_mac_del(ifindex_t ifindex, vlan_t vlan, mac_t *mac, int pri)
{
	return b53125_mac_tbl_del(b53_device, if_ifindex2phy(ifindex), mac, vlan);
}

static int drv_mac_clr(ifindex_t ifindex, vlan_t vlan)
{
	if(if_ifindex2phy(ifindex))
		return b53125_clear_mac_tbl_port(b53_device, if_ifindex2phy(ifindex));
	else if(vlan)
		return b53125_clear_mac_tbl_port(b53_device, vlan);
}

static int drv_mac_read(ifindex_t ifindex, vlan_t vlan)
{
/*	return b53125_fdb_dump(b53_device, if_ifindex2phy(ifindex),
			int (*cb)(u8 *, u16, zpl_bool, void *), void *data);*/
	return OK;
}


/******************************** vlan ***************************/
static int drv_vlan_enable(zpl_bool enable)
{
	return b53125_enable_vlan(b53_device,  enable,
			zpl_false);
}

static int drv_vlan_create(vlan_t vlan)
{
	return b53125_add_vlan_entry(b53_device, vlan);
}

static int drv_vlan_destroy(vlan_t vlan)
{
	return b53125_del_vlan_entry(b53_device, vlan);
}

static int drv_vlan_add_untag_port(ifindex_t ifindex, vlan_t vlan)
{
	return b53125_add_vlan_port(b53_device, vlan, if_ifindex2phy(ifindex), zpl_false);
}

static int drv_vlan_del_untag_port(ifindex_t ifindex, vlan_t vlan)
{
	return b53125_del_vlan_port(b53_device, vlan, if_ifindex2phy(ifindex), zpl_false);
}

static int drv_vlan_add_tag_port(ifindex_t ifindex, vlan_t vlan)
{
	return b53125_add_vlan_port(b53_device, vlan, if_ifindex2phy(ifindex), zpl_true);
}

static int drv_vlan_del_tag_port(ifindex_t ifindex, vlan_t vlan)
{
	return b53125_del_vlan_port(b53_device, vlan, if_ifindex2phy(ifindex), zpl_true);
}

static int drv_port_add_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan)
{
	return b53125_add_vlan_port(b53_device, vlan, if_ifindex2phy(ifindex), zpl_true);
}

static int drv_port_del_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan)
{
	return b53125_del_vlan_port(b53_device, vlan, if_ifindex2phy(ifindex), zpl_true);
}

static int drv_port_add_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end)
{
	zpl_uint32 i = 0, ret = 0;
	for(i = start; i <= end; i++)
	{
		ret |= b53125_add_vlan_port(b53_device, i, if_ifindex2phy(ifindex), zpl_true);
		if(ret != 0)
			break;
	}
	return ret;
}

static int drv_port_del_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end)
{
	zpl_uint32 i = 0, ret = 0;
	for(i = start; i <= end; i++)
	{
		ret |= b53125_del_vlan_port(b53_device, i, if_ifindex2phy(ifindex), zpl_true);
		if(ret != 0)
			break;
	}
	return ret;
}

static int drv_port_add_native_vlan(ifindex_t ifindex, vlan_t vlan)
{
	return b53125_port_pvid(b53_device, vlan, if_ifindex2phy(ifindex), 0);
}

static int drv_port_del_native_vlan(ifindex_t ifindex, vlan_t vlan)
{
	return b53125_port_pvid(b53_device, 1, if_ifindex2phy(ifindex), 0);
}

/*
static int drv_port_set_vlan(ifindex_t ifindex, vlan_t vlan);
static int drv_port_unset_vlan(ifindex_t ifindex, vlan_t vlan);


int b53125_port_pvid(struct b53125_device *dev, u16 vid, int port, int pri);
int b53125_vlan_double_tagging_tpid(struct b53125_device *dev, u16 tpid);
int b53125_ISP_port(struct b53125_device *dev, int port, zpl_bool enable);
*/

/******************************** rate limite ***************************/
//Set the limit and burst size to bucket 0(storm control use bucket1).
static int drv_cpu_limit_rate(int bps)
{
	//1000Mbps/((64B+8B+12B)×8bit)=1.488095pps
	int rate_index = 0;
	rate_index = bps/672;
	return b53125_qos_cpu_rate(b53_device, rate_index);
}

static int drv_port_limit_egress_rate(ifindex_t ifindex, int limitkb, int burst_size)
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
    } else if { /* 104MB ~ 1000MB */
    	cnt = (limitkb /8192) + 115;
	} else {
		return ERROR;
	}
	return b53125_qos_egress_rate(b53_device, if_ifindex2phy(ifindex), burst, limitbps);
}

static int drv_port_limit_ingress_rate(ifindex_t ifindex, int limitkb, int burst_size)
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
    } else if { /* 104MB ~ 1000MB */
    	cnt = (limitkb /8192) + 115;
	} else {
		return ERROR;
	}
	b53125_qos_buck_mode(b53_device, 0, zpl_true);
	b53125_qos_buck_type(b53_device, 0, 0x3f);
	b53125_qos_ingress_rate_mode(b53_device, if_ifindex2phy(ifindex),  3, zpl_true)

	return b53125_qos_ingress_rate(b53_device, if_ifindex2phy(ifindex), 0, burst, limitbps)
}

static int drv_port_strom_rate(ifindex_t ifindex, int limitkb, int burst_size)
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
    } else if{ /* 104MB ~ 1000MB */
    	cnt = (limitkb /8192) + 115;
	} else {
		return ERROR;
	}
	b53125_qos_buck_mode(b53_device, 1, zpl_true);
	b53125_qos_buck_type(b53_device, 1, 0x3f);

	return b53125_qos_ingress_rate(b53_device, if_ifindex2phy(ifindex), 1, burst, limitbps);
}

static int drv_port_strom_mode(ifindex_t ifindex, zpl_bool enable, int mode)
{
	return b53125_qos_ingress_rate_mode(b53_device, if_ifindex2phy(ifindex),  mode, enable);
}

/******************************** qos ***************************/
static int drv_qos_ipg(zpl_bool tx, zpl_bool enable)
{
	return b53125_qos_ingress_ipg(b53_device, tx, enable);
}

static int drv_qos_enable(zpl_bool enable)
{
	b53125_qos_base_port(b53_device, zpl_false);
	b53125_qos_layer_sel(b53_device, 3);
	return 0;
}

static int drv_qos_8021q_enable(ifindex_t ifindex, zpl_bool enable)
{
	return b53125_qos_8021p(b53_device, if_ifindex2phy(ifindex), enable);
}

static int drv_qos_diffserv_enable(ifindex_t ifindex, zpl_bool enable)
{
	return b53125_qos_diffserv(b53_device, if_ifindex2phy(ifindex), enable);
}

static int drv_qos_port_map_queue(ifindex_t ifindex, drv_priority_t pri, drv_queue_t queue)
{
	return b53125_qos_port_map_queue(b53_device, if_ifindex2phy(ifindex), pri, queue);
}

static int drv_qos_diffserv_queue(int diffserv, drv_queue_t queue)
{
	return b53125_qos_diffserv_map_queue(b53_device, diffserv, queue);
}

static int drv_qos_queue_class(drv_queue_t queue, drv_class_t class)
{
	return b53125_qos_queue_map_class(b53_device, queue, class);
}

static int drv_qos_class_scheduling(drv_class_t class, drv_class_sched_t mode)
{
	int sched_mode = 0;
	if(mode == DRV_CLASS_SCHED_STRICT)
	{
		if(class == DRV_CLASS_ID_0)
		{
			sched_mode = 3;
		}
		else if(class == DRV_CLASS_ID_1)
		{
			sched_mode = 3;
		}
		else if(class == DRV_CLASS_ID_2)
		{
			sched_mode = 2;
		}
		else if(class == DRV_CLASS_ID_3)
		{
			sched_mode = 1;
		}
		else if(class == DRV_CLASS_ID_4)
		{
			return b53125_qos_class4_weight(b53_device, zpl_true,  -1);
		}
	}
	else
		sched_mode = 0;
	if(class == DRV_CLASS_ID_4)
	{
		return b53125_qos_class4_weight(b53_device, zpl_false,  -1);
	}
	/*
	00 = all queues are weighted round robin
	01 = COS3 is strict priority, COS2-COS0 are weighted round
	robin.
	10 = COS3 and COS2 is strict priority, COS1-COS0 are weighted
	round robin.
	11 = COS3, COS2, COS1 and COS0 are in strict priority.
	 */
	return b53125_qos_class_scheduling(b53_device, sched_mode);
}

static int drv_qos_class_weight(drv_class_t class, int weight)
{
	if(class == DRV_CLASS_ID_4)
		return b53125_qos_class4_weight(b53_device, zpl_false,  weight);
	return b53125_qos_class_weight(b53_device, class, weight);
}


/******************************** global ***************************/
static int drv_global_manage(zpl_bool enable)
{
	return b53125_manege_enable(b53_device, enable);
}
static int drv_global_forwarding_enable(zpl_bool enable)
{
	return b53125_forwarding_enable(b53_device, enable);
}

int b53125_multicast_flood(struct b53125_device *dev, zpl_bool enable);
int b53125_unicast_flood(struct b53125_device *dev, zpl_bool enable);
int b53125_multicast_learning(struct b53125_device *dev, zpl_bool enable);
int b53125_enable_bpdu(struct b53125_device *dev, zpl_bool enable);
int b53125_aging_time(struct b53125_device *dev, int agetime);
int b53125_bpdu_forward(struct b53125_device *dev, u8 *mac, zpl_bool enable);
/******************************** cpu port ***************************/
static int drv_global_imp_enable(zpl_bool enable)
{
	return b53125_imp_enable(b53_device, enable);
}

static int drv_imp_speed(int speed)
{
	return b53125_imp_speed(b53_device, speed);
}

static int drv_imp_duplex(int duplex)
{
	return b53125_imp_duplex(b53_device, duplex);
}

static int drv_imp_flow(zpl_bool rx, zpl_bool tx)
{
	return b53125_imp_flow(b53_device, rx, tx);
}

/******************************** port ***************************/
static int drv_port_protected_enable(ifindex_t ifindex, zpl_bool enable)
{
	return b53125_protected_enable(b53_device, if_ifindex2phy(ifindex), enable);
}

static int drv_port_pasue_transmit_enable(ifindex_t ifindex, zpl_bool enable)
{
	return b53125_pasue_transmit_enable(b53_device, if_ifindex2phy(ifindex), enable);
}

static int drv_port_pasue_receive_enable(ifindex_t ifindex, zpl_bool enable)
{
	return b53125_pasue_receive_enable(b53_device, if_ifindex2phy(ifindex), enable);
}

static int drv_port_pasue_receive_enable(ifindex_t ifindex, zpl_bool enable)
{
	return b53125_pasue_receive_enable(b53_device, if_ifindex2phy(ifindex), enable);
}

int b53125_unicast_map(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_multicast_map(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_ip_multicast_map(struct b53125_device *dev, int port, zpl_bool enable);
//IEEE 802.3x 是全双工以太网数据链路层的流控方法。当客户终端向服务器发出请求后，自身系统或网络产生拥塞时，它会向服务器发出PAUSE帧，以延缓服务器向客户终端的数据传输
int b53125_pause_pass_rx(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_pause_pass_tx(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_enable_learning(struct b53125_device *dev, int port, zpl_bool enable);
int b53125_software_learning(struct b53125_device *dev, int port, zpl_bool enable);

int b53125_port_state_override_speed(struct b53125_device *dev, int port, int speed);
int b53125_port_state_override_duplex(struct b53125_device *dev, int port, int duplex);
int b53125_port_state_override_flow(struct b53125_device *dev, int port, int flow);
int b53125_port_state_override_link_force(struct b53125_device *dev, int port, int link);


int b53125_enable_port(struct b53125_device *dev, int port, zpl_bool enable);
zpl_bool b53125_port_link(struct b53125_device *dev, int port);

#endif
