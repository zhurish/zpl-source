#include <zebra.h>

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "vrf.h"
#include "command.h"
#include "interface.h"

#include "if_manage.h"
#include "nsm_trunk.h"
#include "nsm_vlan.h"
#include "hal_vlan.h"
#include "hal_port.h"

#include "bcm53125_sdk.h"

//#include "b53xxx_ioctl.h"


static int sdk_drv_open(char *name)
{
	return open(name, 0);
}

static int sdk_drv_close(int fd)
{
	return close(fd);
}


static int sdk_drv_ioctl(int fd, caddr_t pVoid)
{
	return ioctl(fd, B53_IO_WR, pVoid);
}

static int sdk_drv_cmd_ioctl(caddr_t pVoid)
{
	int ret = -1;
	int fd = sdk_drv_open(SDK_DRV);
	if(fd)
	{
		ret = sdk_drv_ioctl(fd,  pVoid);
		sdk_drv_close(fd);
	}
	return ret;
}


int sdk_cmd_ioctl(caddr_t pVoid)
{
	return sdk_drv_cmd_ioctl(pVoid);
}

/*
 * vlan
 */

static int sdk_vlan_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_ENABLE;
	data.module = SDK_MODULE_VLAN;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_vlan_create(vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_ADD;
	data.module = SDK_MODULE_VLAN;
	data.vid = vlan;
	return sdk_cmd_ioctl(&data);
}

static int sdk_vlan_delete(vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_DEL;
	data.module = SDK_MODULE_VLAN;
	data.vid = vlan;
	return sdk_cmd_ioctl(&data);
}

static int sdk_vlan_batch_create(vlan_t start, vlan_t end)
{
	struct b53xxx_ioctl data;
	struct vlan_table vlan;
	memset(&data, 0, sizeof(data));
	memset(&vlan, 0, sizeof(vlan));
	data.cmd = CMD_VLAN_ADD;
	data.module = SDK_MODULE_VLAN;
	vlan.start_vid = start;
	vlan.end_vid = end;
	data.data = &vlan;
	return sdk_cmd_ioctl(&data);
}

static int sdk_vlan_batch_delete(vlan_t start, vlan_t end)
{
	struct b53xxx_ioctl data;
	struct vlan_table vlan;
	memset(&data, 0, sizeof(data));
	memset(&vlan, 0, sizeof(vlan));
	data.cmd = CMD_VLAN_DEL;
	data.module = SDK_MODULE_VLAN;
	vlan.start_vid = start;
	vlan.end_vid = end;
	data.data = &vlan;
	return sdk_cmd_ioctl(&data);
}

static int sdk_vlan_add_untag_port(ifindex_t ifindex, vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_ADD_UNTAG_PORT;
	data.module = SDK_MODULE_VLAN;
	data.vid = vlan;
	data.port = if_ifindex2phy(ifindex);
	return sdk_cmd_ioctl(&data);
}
static int sdk_vlan_del_untag_port(ifindex_t ifindex, vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_DEL_UNTAG_PORT;
	data.module = SDK_MODULE_VLAN;
	data.vid = vlan;
	data.port = if_ifindex2phy(ifindex);
	return sdk_cmd_ioctl(&data);
}
static int sdk_vlan_add_tag_port(ifindex_t ifindex, vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_ADD_TAG_PORT;
	data.module = SDK_MODULE_VLAN;
	data.vid = vlan;
	data.port = if_ifindex2phy(ifindex);
	return sdk_cmd_ioctl(&data);
}
static int sdk_vlan_del_tag_port(ifindex_t ifindex, vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_DEL_TAG_PORT;
	data.module = SDK_MODULE_VLAN;
	data.vid = vlan;
	data.port = if_ifindex2phy(ifindex);
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_set_native_vlan(ifindex_t ifindex, vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_ADD_DEFAULT_PORT;
	data.module = SDK_MODULE_VLAN;
	data.port = if_ifindex2phy(ifindex);
	data.vid = vlan;
	if(sdk_cmd_ioctl(&data) == 0)
	{
		data.cmd = CMD_VLAN_ADD_TAG_PORT;
		return sdk_cmd_ioctl(&data);
	}
	return -1;
}
static int sdk_port_unset_native_vlan(ifindex_t ifindex, vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_DEL_DEFAULT_PORT;
	data.module = SDK_MODULE_VLAN;
	data.port = if_ifindex2phy(ifindex);
	data.vid = vlan;
	if(sdk_cmd_ioctl(&data) == 0)
	{
		data.cmd = CMD_VLAN_DEL_TAG_PORT;
		return sdk_cmd_ioctl(&data);
	}
	return -1;
}


static int sdk_port_add_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_ADD_TAG_PORT;
	data.module = SDK_MODULE_VLAN;
	data.vid = vlan;
	return sdk_cmd_ioctl(&data);
}
static int sdk_port_del_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_DEL_TAG_PORT;
	data.module = SDK_MODULE_VLAN;
	data.port = if_ifindex2phy(ifindex);
	data.vid = vlan;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_add_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end)
{
	struct b53xxx_ioctl data;
	struct vlan_table vlan;
	memset(&data, 0, sizeof(data));
	memset(&vlan, 0, sizeof(vlan));
	data.cmd = CMD_VLAN_ADD_TAG_PORT;
	data.module = SDK_MODULE_VLAN;
	data.port = if_ifindex2phy(ifindex);
	vlan.start_vid = start;
	vlan.end_vid = end;
	data.data = &vlan;
	return sdk_cmd_ioctl(&data);
}
static int sdk_port_del_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end)
{
	struct b53xxx_ioctl data;
	struct vlan_table vlan;
	memset(&data, 0, sizeof(data));
	memset(&vlan, 0, sizeof(vlan));
	data.cmd = CMD_VLAN_DEL_TAG_PORT;
	data.module = SDK_MODULE_VLAN;
	data.port = if_ifindex2phy(ifindex);
	vlan.start_vid = start;
	vlan.end_vid = end;
	data.data = &vlan;
	return sdk_cmd_ioctl(&data);
}
static int sdk_port_set_pvid_vlan(ifindex_t ifindex, vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_ADD_DEFAULT_PORT;
	data.module = SDK_MODULE_VLAN;
	data.port = if_ifindex2phy(ifindex);
	data.vid = vlan;
	return sdk_cmd_ioctl(&data);
}
static int sdk_port_unset_pvid_vlan(ifindex_t ifindex, vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_VLAN_DEL_DEFAULT_PORT;
	data.module = SDK_MODULE_VLAN;
	data.port = if_ifindex2phy(ifindex);
	data.vid = vlan;
	return sdk_cmd_ioctl(&data);
}

int sdk_vlan_init()
{
	sdk_vlan.sdk_vlan_enable = sdk_vlan_enable;
	sdk_vlan.sdk_vlan_create = sdk_vlan_create;
	sdk_vlan.sdk_vlan_delete = sdk_vlan_delete;
	sdk_vlan.sdk_vlan_batch_create = sdk_vlan_batch_create;
	sdk_vlan.sdk_vlan_batch_delete = sdk_vlan_batch_delete;

	sdk_vlan.sdk_vlan_add_untag_port = sdk_vlan_add_untag_port;
	sdk_vlan.sdk_vlan_del_untag_port = sdk_vlan_del_untag_port;
	sdk_vlan.sdk_vlan_add_tag_port = sdk_vlan_add_tag_port;
	sdk_vlan.sdk_vlan_del_tag_port = sdk_vlan_del_tag_port;

	sdk_vlan.sdk_port_set_native_vlan = sdk_port_set_native_vlan;
	sdk_vlan.sdk_port_unset_native_vlan = sdk_port_unset_native_vlan;

	sdk_vlan.sdk_port_add_allowed_tag_vlan = sdk_port_add_allowed_tag_vlan;
	sdk_vlan.sdk_port_del_allowed_tag_vlan = sdk_port_del_allowed_tag_vlan;

	sdk_vlan.sdk_port_add_allowed_tag_batch_vlan = sdk_port_add_allowed_tag_batch_vlan;
	sdk_vlan.sdk_port_del_allowed_tag_batch_vlan = sdk_port_del_allowed_tag_batch_vlan;

	sdk_vlan.sdk_port_set_pvid_vlan = sdk_port_set_pvid_vlan;
	sdk_vlan.sdk_port_unset_pvid_vlan = sdk_port_unset_pvid_vlan;

	return OK;
}



/*
 * QINQ
 */

static int sdk_qinq_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QINQ_ENABLE;
	data.module = SDK_MODULE_QINQ;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qinq_vlan_tpid(int tpid)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QINQ_DEFAULT_TPID;
	data.module = SDK_MODULE_QINQ;
	data.vid = tpid & 0xffff;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qinq_port_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = enable ? CMD_QINQ_ADD_PORT:CMD_QINQ_DEL_PORT;
	data.module = SDK_MODULE_QINQ;
	data.port = if_ifindex2phy(ifindex);
	return sdk_cmd_ioctl(&data);
}



/*
 * MAC
 */
static int sdk_mac_table_add(ifindex_t ifindex, u8 *mac, int pri)
{
	struct b53xxx_ioctl data;
	struct mac_table mact;
	vlan_t vlan = IF_IFINDEX_VLAN_GET(ifindex);
	memset(&data, 0, sizeof(data));
	memset(&mact, 0, sizeof(mact));
	data.cmd = CMD_MAC_ADD;
	data.module = SDK_MODULE_MAC;
	data.port = if_ifindex2phy(ifindex);
	mact.port = data.port;
	memcpy(mact.mac, mac, ETH_ALEN);
	mact.vid = vlan;
	mact.pri = pri;
	data.data = &mact;
	return sdk_cmd_ioctl(&data);
}

static int sdk_mac_table_del(ifindex_t ifindex, u8 *mac, int pri)
{
	struct b53xxx_ioctl data;
	struct mac_table mact;
	vlan_t vlan = IF_IFINDEX_VLAN_GET(ifindex);
	memset(&data, 0, sizeof(data));
	memset(&mact, 0, sizeof(mact));
	data.cmd = CMD_MAC_DEL;
	data.module = SDK_MODULE_MAC;
	data.port = if_ifindex2phy(ifindex);
	mact.port = data.port;
	memcpy(mact.mac, mac, ETH_ALEN);
	mact.vid = vlan;
	mact.pri = pri;
	data.data = &mact;
	return sdk_cmd_ioctl(&data);
}

static int sdk_mac_table_age(int age)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MAC_AGE;
	data.module = SDK_MODULE_MAC;
	data.value = age;
	return sdk_cmd_ioctl(&data);
}

static int sdk_mac_table_clear(ifindex_t ifindex, vlan_t vlan)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MAC_AGE;
	data.module = SDK_MODULE_MAC;
	data.port = if_ifindex2phy(ifindex);
	data.vid = vlan;
	return sdk_cmd_ioctl(&data);
}

static int sdk_mac_table_read(ifindex_t ifindex, vlan_t vlan)
{
	struct b53xxx_ioctl data;
	u8 mact[4096];
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MAC_GET;
	data.module = SDK_MODULE_MAC;
	data.data = mact;
	return sdk_cmd_ioctl(&data);
}


/*
 * trunk
 */

static int sdk_trunk_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_TRUNK_ENABLE;
	data.module = SDK_MODULE_TRUNK;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_trunk_mode(int mode)
{
	struct b53xxx_ioctl data;

	memset(&data, 0, sizeof(data));
	data.cmd = CMD_TRUNK_MODE;
	data.module = SDK_MODULE_TRUNK;
	//data.value = mode;
	if(mode == TRUNK_LOAD_BALANCE_DSTMAC)
		data.value = 1;
	else if(mode == TRUNK_LOAD_BALANCE_SRCMAC)
		data.value = 2;
	else if(mode == TRUNK_LOAD_BALANCE_DSTSRCMAC)
		data.value = 0;

	return sdk_cmd_ioctl(&data);
}
static int sdk_trunk_add_port(ifindex_t ifindex, int trunkid)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_TRUNK_ADD;
	data.module = SDK_MODULE_TRUNK;
	data.port = if_ifindex2phy(ifindex);
	data.value = hal_trunkid(trunkid);
	return sdk_cmd_ioctl(&data);
}

static int sdk_trunk_del_port(ifindex_t ifindex, int trunkid)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_TRUNK_ADD;
	data.module = CMD_TRUNK_DEL;
	data.port = if_ifindex2phy(ifindex);
	data.value = hal_trunkid(trunkid);
	return sdk_cmd_ioctl(&data);
}

/*
 * DOS
 */

static int sdk_dos_enable(BOOL enable, int type)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_DOS_ENABLE;
	data.module = SDK_MODULE_DOS;
	data.value = type;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_dos_tcp_hdr(int size)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_DOS_SET_TCP_HDR;
	data.module = SDK_MODULE_DOS;
	data.value = size;
	return sdk_cmd_ioctl(&data);
}

static int sdk_dos_icmp_size(BOOL ipv6, int size)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_DOS_SET_ICMP_SIZE;
	data.module = SDK_MODULE_DOS;
	data.value = size;
	data.ipv6 = ipv6;
	return sdk_cmd_ioctl(&data);
}


/*
 * jumbo
 */

static int sdk_jumbo_enable(BOOL enable, ifindex_t ifindex)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_JUMBO_ENABLE;
	data.module = SDK_MODULE_JUMBO;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_jumbo_size(int size)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_JUMBO_SIZE;
	data.module = SDK_MODULE_JUMBO;
	data.value = size;
	return sdk_cmd_ioctl(&data);
}


//mstp/stp
static int sdk_mstp_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MSTP_ENABLE;
	data.module = SDK_MODULE_MSTP;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_mstp_age(int age)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MSTP_AGE;
	data.module = SDK_MODULE_MSTP;
	data.value = age;
	return sdk_cmd_ioctl(&data);
}

static int sdk_mstp_bypass(BOOL enable, int type)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MSTP_BYPASS;
	data.module = SDK_MODULE_MSTP;
	data.value = type;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_mstp_state(ifindex_t ifindex, int mstp, int state)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MSTP_STATE;
	data.module = SDK_MODULE_MSTP;
	data.port = if_ifindex2phy(ifindex);
	data.value = mstp;
	data.value1 = state;
	return sdk_cmd_ioctl(&data);
}

static int sdk_stp_state(ifindex_t ifindex, int state)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_STP_STATE;
	data.module = SDK_MODULE_MSTP;
	data.port = if_ifindex2phy(ifindex);
	data.value = state;
	return sdk_cmd_ioctl(&data);
}


//mirror
static int sdk_mirror_dst_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MIRROR_ENABLE;
	data.module = SDK_MODULE_MIRROR;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_mirror_src_enable(BOOL enable, ifindex_t ifindex, int mode)
{
	struct b53xxx_ioctl data;
	struct mirror_ctl ctl;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MIRROR_SOURCE;
	data.module = SDK_MODULE_MIRROR;
	data.port = if_ifindex2phy(ifindex);
	data.value = mode;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_mirror_src_filter_enable(BOOL enable, BOOL dst, u8 *mac, int mode)
{
	struct b53xxx_ioctl data;
	struct mirror_ctl ctl;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MIRROR_SOURCE_FILTER;
	data.module = SDK_MODULE_MIRROR;
	//data.port = if_ifindex2phy(ifindex);
	data.value = mode;
	if(mac)
	{
		ctl.dst = dst;
		memcpy(ctl.mac, mac, ETH_ALEN);
		data.data = &ctl;
	}
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

/*
 * snooping
 */

static int sdk_snooping_enable(BOOL enable, int mode, BOOL ipv6)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = mode;
	data.module = SDK_MODULE_SNOOPING;
	data.ipv6 = ipv6;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}


/*
 * Rate limit
 */

static int sdk_rate_limit_port(ifindex_t ifindex, BOOL ingress, int rate, int busrt)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = ingress ? CMD_INGRESS_RATE_ENABLE:CMD_EGRESS_RATE_ENABLE;
	data.module = SDK_MODULE_RATE;
	data.port = if_ifindex2phy(ifindex);
	data.value = rate;
	data.value1 = busrt;
	return sdk_cmd_ioctl(&data);
}

static int sdk_rate_limit_cpu(int rate)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_CPU_RATE_ENABLE;
	data.module = SDK_MODULE_RATE;
	//data.port = if_ifindex2phy(ifindex);
	data.value = rate;
	return sdk_cmd_ioctl(&data);
}

/*
 * Strom
 */
static int sdk_strom_port_enable(ifindex_t ifindex, int rate)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_STROM_ENABLE;
	data.module = SDK_MODULE_STORM;
	data.port = if_ifindex2phy(ifindex);
	data.value = rate;
	return sdk_cmd_ioctl(&data);
}

static int sdk_strom_port_rate(ifindex_t ifindex, int rate, int busrt)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_STROM_INGRESS_RATE_ENABLE;
	data.module = SDK_MODULE_STORM;
	data.port = if_ifindex2phy(ifindex);
	data.value = rate;
	data.value1 = busrt;
	return sdk_cmd_ioctl(&data);
}

/*
 * QOS
 */
static int sdk_qos_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QOS_PORT_ENABLE;
	data.module = SDK_MODULE_QOS;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_8021q_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QOS_8021Q_ENABLE;
	data.module = SDK_MODULE_QOS;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_diffserv_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QOS_DIFFSERV_ENABLE;
	data.module = SDK_MODULE_QOS;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_8021q_map_pri(ifindex_t ifindex, int v8021p, int pri)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QOS_PRI_TO_PRI;
	data.module = SDK_MODULE_QOS;
	data.port = if_ifindex2phy(ifindex);
	data.value = v8021p;
	data.value1 = pri;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_diffserv_map_pri(int diffserv, int pri)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QOS_DIFFSERV_TO_PRI;
	data.module = SDK_MODULE_QOS;
	//data.port = if_ifindex2phy(ifindex);
	data.value = diffserv;
	data.value1 = pri;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_pri_map_class(int pri, int class)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QOS_PRI_TO_CLASS;
	data.module = SDK_MODULE_QOS;
	//data.port = if_ifindex2phy(ifindex);
	data.value = pri;
	data.value1 = class;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_cpu_map_class(BOOL enable,int pri, int class)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QOS_CPU_TO_CLASS;
	data.module = SDK_MODULE_QOS;
	data.enable = enable ? TRUE:FALSE;
	//data.port = if_ifindex2phy(ifindex);
	data.value = pri;
	data.value1 = class;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_tx_queue_mode(int mode)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QOS_TX_QUEUE_MODE;
	data.module = SDK_MODULE_QOS;
	//data.enable = enable ? TRUE:FALSE;
	//data.port = if_ifindex2phy(ifindex);
	data.value = mode;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_tx_queue_weight(int queue, int weight)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QOS_TX_QUEUE_WEIGHT;
	data.module = SDK_MODULE_QOS;
	//data.enable = enable ? TRUE:FALSE;
	//data.port = if_ifindex2phy(ifindex);
	data.value = queue;
	data.value1 = weight;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_class4_weight(int mode, int weight)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QOS_CLASS4_QUEUE_MODE;
	data.module = SDK_MODULE_QOS;
	//data.enable = enable ? TRUE:FALSE;
	//data.port = if_ifindex2phy(ifindex);
	data.value = mode;
	data.value1 = weight;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_remarking_enable(ifindex_t ifindex, int value, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QOS_REMARKING_ENABLE;
	data.module = SDK_MODULE_QOS;
	data.enable = enable ? TRUE:FALSE;
	data.port = if_ifindex2phy(ifindex);
	data.value = value;
	//data.value1 = weight;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_pri_map_8021q(ifindex_t ifindex, int pri, int v8021q, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_QOS_PRI_MAP_8021P;
	data.module = SDK_MODULE_QOS;
	data.enable = enable ? TRUE:FALSE;
	data.port = if_ifindex2phy(ifindex);
	data.value = pri;
	data.value1 = v8021q;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_cpu_rate_limit(int pps)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_CPU_RATE_ENABLE;
	data.module = SDK_MODULE_RATE;
	//data.enable = enable ? TRUE:FALSE;
	//data.port = if_ifindex2phy(ifindex);
	data.value = pps;
	//data.value1 = v8021q;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_wan_rate_limit(int pps)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_WAN_RATE_ENABLE;
	data.module = SDK_MODULE_RATE;
	//data.enable = enable ? TRUE:FALSE;
	//data.port = if_ifindex2phy(ifindex);
	data.value = pps;
	//data.value1 = v8021q;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_ingress_rate_limit(ifindex_t ifindex, int rate, int busrt)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_INGRESS_RATE_ENABLE;
	data.module = SDK_MODULE_RATE;
	data.port = if_ifindex2phy(ifindex);
	data.value = rate;
	data.value1 = busrt;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_egress_rate_limit(ifindex_t ifindex, int rate, int busrt)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_EGRESS_RATE_ENABLE;
	data.module = SDK_MODULE_RATE;
	data.port = if_ifindex2phy(ifindex);
	data.value = rate;
	data.value1 = busrt;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_strom_enable(ifindex_t ifindex, int type, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_STROM_ENABLE;
	data.module = SDK_MODULE_STORM;
	data.enable = enable ? TRUE:FALSE;
	data.port = if_ifindex2phy(ifindex);
	data.value = type;
	//data.value1 = busrt;
	return sdk_cmd_ioctl(&data);
}

static int sdk_qos_strom_rate_limit(ifindex_t ifindex, int rate, int busrt)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_STROM_ENABLE;
	data.module = SDK_MODULE_STORM;
	//data.enable = enable ? TRUE:FALSE;
	data.port = if_ifindex2phy(ifindex);
	data.value = rate;
	data.value1 = busrt;
	return sdk_cmd_ioctl(&data);
}

/*
 * Port
 */
static int sdk_port_enable(ifindex_t ifindex)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_ENABLE;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.enable = TRUE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_disable(ifindex_t ifindex)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_ENABLE;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.enable = FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_learning_disable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_DISABLE_LRN;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_swlearning_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_ENABLE_SLRN;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_protected_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_PROTECTED;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_forward_mode(int mode, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_FORWARD_MODE;
	data.module = SDK_MODULE_PORT;
	data.value = mode;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}


static int sdk_port_pause_capability_enable(ifindex_t ifindex, BOOL enable, int mode)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_PASUE_CAPABILITY;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.value = mode;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_frame_detection_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_FAUSE_FRAME_DETECTION;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_pass_through_enable(ifindex_t ifindex, BOOL enable, BOOL ingress)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = ingress ? CMD_PORT_FAUSE_PASS_THROUGH_RX:CMD_PORT_FAUSE_PASS_THROUGH_TX;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_speed(ifindex_t ifindex, int speed)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_SPEED;
	data.module = SDK_MODULE_PORT;
	data.port = ifindex;//if_ifindex2phy(ifindex);
	data.value = speed;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_duplex(ifindex_t ifindex, int duplex)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_DUPLEX;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.value = duplex;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_link_state(ifindex_t ifindex, int *link)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_LINK;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	if(link)
		*link = data.value;
	return sdk_cmd_ioctl(&data);
}


static int sdk_port_flow_enable(ifindex_t ifindex, int value, int value1)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_FLOW;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.value = value;
	data.value1 = value1;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_multicast_forward_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_MULTICAST_FORWARD;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_port_mib_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_MIB;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

//Port Status
static int sdk_port_get_link(ifindex_t ifindex, int *link)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_LINK_STATE;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	if(sdk_cmd_ioctl(&data) == 0)
	{
		if(link)
			*link = data.value;
		return OK;
	}
	return -1;
}
static int sdk_port_get_speed(ifindex_t ifindex, int *speed)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_SPEED_STATE;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	if(sdk_cmd_ioctl(&data) == 0)
	{
		if(speed)
			*speed = data.value;
		return OK;
	}
	return -1;
}
static int sdk_port_get_duplex(ifindex_t ifindex, int *duplex)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_PORT_DUPLEX_STATE;
	data.module = SDK_MODULE_PORT;
	data.port = if_ifindex2phy(ifindex);
	if(sdk_cmd_ioctl(&data) == 0)
	{
		if(duplex)
			*duplex = data.value;
		return OK;
	}
	return -1;
}

/*
 * CPU port
 */
static int sdk_cpu_port_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_CPU_PORT_ENABLE;
	data.module = SDK_MODULE_CPU_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_cpu_port_duplex(ifindex_t ifindex, int duplex)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_CPU_PORT_DUPLEX;
	data.module = SDK_MODULE_CPU_PORT;
	data.port = if_ifindex2phy(ifindex);
	return sdk_cmd_ioctl(&data);
}

static int sdk_cpu_port_speed(ifindex_t ifindex, int speed)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_CPU_PORT_SPEED;
	data.module = SDK_MODULE_CPU_PORT;
	data.port = if_ifindex2phy(ifindex);
	return sdk_cmd_ioctl(&data);
}

static int sdk_cpu_port_get_link(ifindex_t ifindex, int *link)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_CPU_PORT_LINK;
	data.module = SDK_MODULE_CPU_PORT;
	data.port = if_ifindex2phy(ifindex);
	if(sdk_cmd_ioctl(&data) == 0)
	{
		if(link)
			*link = data.value;
		return OK;
	}
	return -1;
}

static int sdk_cpu_port_flow(ifindex_t ifindex, BOOL rx, BOOL tx)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_CPU_PORT_SPEED;
	data.module = SDK_MODULE_CPU_PORT;
	data.port = if_ifindex2phy(ifindex);
	data.value = rx;
	data.value1 = tx;
	return sdk_cmd_ioctl(&data);
}


/*
 * Global
 */
static int sdk_forward_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_SWITCH_FWDG;
	data.module = SDK_MODULE_GLOBAL;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_manage_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_SWITCH_MODE;
	data.module = SDK_MODULE_GLOBAL;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_reset_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_SWITCH_RESET;
	data.module = SDK_MODULE_GLOBAL;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_mib_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_SWITCH_EN_MIB;
	data.module = SDK_MODULE_GLOBAL;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_mib_reset_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_SWITCH_RESET_MIB;
	data.module = SDK_MODULE_GLOBAL;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_switch_init()
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_SWITCH_INIT;
	data.module = SDK_MODULE_GLOBAL;
	return sdk_cmd_ioctl(&data);
}

static int sdk_bpdu_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_BPDU_RX_ENABLE;
	data.module = SDK_MODULE_GLOBAL;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

/*
 * multicast
 */
static int sdk_multicast_mode(int type, int mode)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MULTICAST_MODE;
	data.module = SDK_MODULE_MULTICAST;
	data.value = type;
	data.value1 = mode;
	return sdk_cmd_ioctl(&data);
}

static int sdk_multicast_add(int type, int mode, char *mac)
{
	struct b53xxx_ioctl data;
	u8 dmac[6];
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MULTICAST_ADD;
	data.module = SDK_MODULE_MULTICAST;
	data.value = type;
	data.value1 = mode;
	data.data = dmac;
	return sdk_cmd_ioctl(&data);
}

static int sdk_multicast_del(int type, int mode, char *mac)
{
	struct b53xxx_ioctl data;
	u8 dmac[6];
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MULTICAST_DEL;
	data.module = SDK_MODULE_MULTICAST;
	data.value = type;
	data.value1 = mode;
	data.data = dmac;
	return sdk_cmd_ioctl(&data);
}

static int sdk_multicast_fwd_mode(ifindex_t ifindex, int type, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MULTICAST_FWD;
	data.module = SDK_MODULE_MULTICAST;
	data.port = if_ifindex2phy(ifindex);
	data.value = type;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

//8021X
static int sdk_8021x_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_8021X_ENABLE;
	data.module = SDK_MODULE_8021X;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_8021x_dst_mac(ifindex_t ifindex, char *mac)
{
	struct b53xxx_ioctl data;
	u8 dmac[6];
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_8021X_ENABLE;
	data.module = SDK_MODULE_8021X;
	data.port = if_ifindex2phy(ifindex);
	data.data = dmac;
	return sdk_cmd_ioctl(&data);
}

static int sdk_8021x_ip_address(int index, u32 address, u32 mask)
{
	struct b53xxx_ioctl data;
	u8 dmac[6];
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_8021X_MODE;
	data.module = SDK_MODULE_8021X;
	data.value = index;
	data.value1 = address;
	data.data = dmac;
	return sdk_cmd_ioctl(&data);
}

static int sdk_8021x_mult_address(BOOL enable, u32 address)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_8021X_MAC;
	data.module = SDK_MODULE_8021X;
	data.value = address;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_8021x_bypass(BOOL enable, u32 address)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_8021X_BYPASS;
	data.module = SDK_MODULE_8021X;
	data.value = address;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

//EEE
static int sdk_eee_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_EEE_ENABLE;
	data.module = SDK_MODULE_EEE;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_eee_value(ifindex_t ifindex, int value)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_EEE_SET;
	data.module = SDK_MODULE_EEE;
	data.port = if_ifindex2phy(ifindex);
	data.value = value;
	return sdk_cmd_ioctl(&data);
}


//Misc
static int sdk_igmp_snooping(BOOL enable, BOOL ipv6)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_IGMP_SNOOPING_ENABLE;
	data.module = SDK_MODULE_SNOOPING;
	data.ipv6 = ipv6;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}
static int sdk_icmp_snooping(BOOL enable, BOOL ipv6)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_ICMP_SNOOPING_ENABLE;
	data.module = SDK_MODULE_SNOOPING;
	data.ipv6 = ipv6;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_dhcp_snooping(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_DHCP_SNOOPING_ENABLE;
	data.module = SDK_MODULE_SNOOPING;
	//data.ipv6 = ipv6;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_rarp_snooping(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_RARP_SNOOPING_ENABLE;
	data.module = SDK_MODULE_SNOOPING;
	//data.ipv6 = ipv6;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}
static int sdk_arp_snooping(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_ARP_SNOOPING_ENABLE;
	data.module = SDK_MODULE_SNOOPING;
	//data.ipv6 = ipv6;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_wan_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MISC_WAN_PORT_ENABLE;
	data.module = SDK_MODULE_MISC;
	//data.ipv6 = ipv6;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_res_mult_lrn_enable(BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MISC_RES_MULTI_LRN;
	data.module = SDK_MODULE_MISC;
	//data.ipv6 = ipv6;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_res_mult_fwd_enable(ifindex_t ifindex, int value, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_MISC_RES_MULTI_FWD_MODE;
	data.module = SDK_MODULE_MISC;
	data.port = if_ifindex2phy(ifindex);
	data.value = value;
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

int sdk_port_init()
{
	extern sdk_port_t sdk_port;
	sdk_port.sdk_port_up_cb = sdk_port_enable;
	sdk_port.sdk_port_down_cb = sdk_port_disable;

	//sdk_port.sdk_port_set_address_cb) (ifindex_t, struct prefix *cp, int secondry);
	//sdk_port.sdk_port_unset_address_cb) (ifindex_t, struct prefix *cp, int secondry);

	//sdk_port.sdk_port_mac_cb = sdk_mac_table_add;//) (ifindex_t, unsigned char *, int);
	//sdk_port.sdk_port_mac_cb = sdk_mac_table_del;

	//sdk_port.sdk_port_mtu_cb) (ifindex_t, int);
	//sdk_port.sdk_port_metric_cb) (ifindex_t, int);
	//sdk_port.sdk_port_vrf_cb) (ifindex_t, int);
	//sdk_port.sdk_port_multicast_cb) (ifindex_t, int);
	//sdk_port.sdk_port_bandwidth_cb) (ifindex_t, int);
	sdk_port.sdk_port_speed_cb = sdk_port_speed;// (ifindex_t, int);

	//sdk_port.sdk_port_mode_cb) (ifindex_t, int);

	//sdk_port.sdk_port_linkdetect_cb) (ifindex_t, int);

	//sdk_port.sdk_port_stp_cb) (ifindex_t, int);
	//sdk_port.sdk_port_loop_cb) (ifindex_t, int);
	//sdk_port.sdk_port_8021x_cb) (ifindex_t, int);
	sdk_port.sdk_port_duplex_cb = sdk_port_duplex;//) (ifindex_t, int);
	return OK;
}

