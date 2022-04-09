/*
 * b53_dos.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */


#include <zplos_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"

/*************************************************************************/
static int b53125_dos_icmpv6_longping_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_ICMP6_LONGPING_DROP_EN;
	else
		port_ctrl &= ~B53_ICMP6_LONGPING_DROP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_icmpv4_longping_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_ICMP4_LONGPING_DROP_EN;
	else
		port_ctrl &= ~B53_ICMP4_LONGPING_DROP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_icmpv6_fragment_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_ICMP6_FRAGMENT_DROP_EN;
	else
		port_ctrl &= ~B53_ICMP6_FRAGMENT_DROP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_icmpv4_fragment_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_ICMP4_FRAGMENT_DROP_EN;
	else
		port_ctrl &= ~B53_ICMP4_FRAGMENT_DROP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
static int b53125_dos_tcp_fragerror_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_FRAGERROR_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_FRAGERROR_DROP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_tcp_shorthdr_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_SHORTHDR_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_SHORTHDR_DROP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_tcp_synerror_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_SYNERROR_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_SYNERROR_DROP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_tcp_synfinscan_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_SYNFINSCAN_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_SYNFINSCAN_DROP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_tcp_nullscan_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_NULLSCAN_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_NULLSCAN_DROP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_tcp_xmassscan_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_XMASSCAN_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_XMASSCAN_DROP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_udp_blat_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_UDP_BLAT_DROP_EN;
	else
		port_ctrl &= ~B53_UDP_BLAT_DROP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_tcp_blat_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_BLAT_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_BLAT_DROP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_ip_lan_drip_drop(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_IP_LAN_DRIP_EN;
	else
		port_ctrl &= ~B53_IP_LAN_DRIP_EN;

	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
static int b53125_dos_tcphdr_minsize(sdk_driver_t *dev, int minsize)
{
	int ret = 0;
	u8 port_ctrl = 0;
	port_ctrl = minsize;
	ret |= b53125_write8(dev->sdk_device, B53_DOS_PAGE, B53_MIN_TCPHDR_SIZE_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_icmp6_maxsize(sdk_driver_t *dev, int minsize)
{
	int ret = 0;
	u32 port_ctrl = 0;
	port_ctrl = minsize;
	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_MAX_ICMPV6_SIZE_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_dos_icmp4_maxsize(sdk_driver_t *dev, int minsize)
{
	int ret = 0;
	u32 port_ctrl = 0;
	port_ctrl = minsize;
	ret |= b53125_write32(dev->sdk_device, B53_DOS_PAGE, B53_MAX_ICMPV4_SIZE_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
int b53125_dos_disable_lean(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	if(enable)
		port_ctrl |= BIT(0);
	else
		port_ctrl &= ~BIT(0);
	ret |= b53125_write8(dev->sdk_device, B53_DOS_PAGE, B53_DOS_DIS_LEARN_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
/*************************************************************************/
static int b53125_dos_type_enable(sdk_driver_t *dev, zpl_uint32 cmd, zpl_bool enable)
{
	int ret = 0;
	switch(cmd)
	{
	case HAL_DOS_CMD_IP_LAN_DRIP:
	ret = b53125_dos_ip_lan_drip_drop(dev, enable);
		break;
	case HAL_DOS_CMD_TCP_BLAT_DROP:
	ret = b53125_dos_tcp_blat_drop(dev, enable);
		break;
	case HAL_DOS_CMD_UDP_BLAT_DROP:
	ret = b53125_dos_udp_blat_drop(dev, enable);
		break;
	case HAL_DOS_CMD_TCP_NULLSCAN_DROP:
	ret = b53125_dos_tcp_nullscan_drop(dev, enable);
		break;
	case HAL_DOS_CMD_TCP_XMASSCAN_DROP:
	ret = b53125_dos_tcp_xmassscan_drop(dev, enable);
		break;
	case HAL_DOS_CMD_TCP_SYNFINSCAN_DROP:
	ret = b53125_dos_tcp_synfinscan_drop(dev, enable);
		break;
	case HAL_DOS_CMD_TCP_SYNERROR_DROP:
	ret = b53125_dos_tcp_synerror_drop(dev, enable);
		break;
	case HAL_DOS_CMD_TCP_SHORTHDR_DROP:
	ret = b53125_dos_tcp_shorthdr_drop(dev, enable);
		break;
	case HAL_DOS_CMD_TCP_FRAGERROR_DROP:
	ret = b53125_dos_tcp_fragerror_drop(dev, enable);
		break;
	case HAL_DOS_CMD_ICMPv4_FRAGMENT_DROP:
	ret = b53125_dos_icmpv4_fragment_drop(dev, enable);
		break;
	case HAL_DOS_CMD_ICMPv6_FRAGMENT_DROP:
	ret = b53125_dos_icmpv6_fragment_drop(dev, enable);
		break;

	case HAL_DOS_CMD_ICMPv4_LONGPING_DROP:
	ret = b53125_dos_icmpv4_longping_drop(dev, enable);
		break;
	case HAL_DOS_CMD_ICMPv6_LONGPING_DROP:
	ret = b53125_dos_icmpv6_longping_drop(dev, enable);
		break;
	}
	return ret;
}

static int b53125_dos_type_size(sdk_driver_t *dev, zpl_uint32 cmd, zpl_uint32 value)
{
	int ret = 0;
	switch(cmd)
	{
	case HAL_DOS_CMD_TCP_HDR_SIZE:
	ret = b53125_dos_tcphdr_minsize(dev, value);
		break;
	case HAL_DOS_CMD_ICMPv4_SIZE:
	ret = b53125_dos_icmp4_maxsize(dev, value);
		break;
	case HAL_DOS_CMD_ICMPv6_SIZE:
	ret = b53125_dos_icmp6_maxsize(dev, value);
	break;
	}
	return ret;
}

DEFUN (b53125_sdk_dos_test,
		b53125_sdk_dos_test_cmd,
		"sdk-dos (iplandrip|tcpblatdrop|udpblatdrop|tcpnull|tcpxma|tcpsynfinsh|tcpsynerr|"
		"shorthdr|fragerror|icmpv4frage|icmpv6frage|icmpv4long|icmpv6long) (enable|disable)",
		"sdk vlan\n"
		"create\n"
		"enable\n"
		"vlanid\n")
{
	int ret = 0;
	zpl_uint32 cmd = 0;
	if(strcmp(argv[0], "iplandrip") == 0)
	cmd = HAL_DOS_CMD_IP_LAN_DRIP;
	else if(strcmp(argv[0], "tcpblatdrop") == 0)
	cmd = HAL_DOS_CMD_TCP_BLAT_DROP;
	else if(strcmp(argv[0], "udpblatdrop") == 0)
	cmd = HAL_DOS_CMD_UDP_BLAT_DROP;
	else if(strcmp(argv[0], "tcpnull") == 0)
	cmd = HAL_DOS_CMD_TCP_NULLSCAN_DROP;
	else if(strcmp(argv[0], "tcpxma") == 0)
	cmd = HAL_DOS_CMD_TCP_XMASSCAN_DROP;
	else if(strcmp(argv[0], "tcpsynfinsh") == 0)
	cmd = HAL_DOS_CMD_TCP_SYNFINSCAN_DROP;
	else if(strcmp(argv[0], "tcpsynerr") == 0)
	cmd = HAL_DOS_CMD_TCP_SYNERROR_DROP;
	else if(strcmp(argv[0], "shorthdr") == 0)
	cmd = HAL_DOS_CMD_TCP_SHORTHDR_DROP;
	else if(strcmp(argv[0], "fragerror") == 0)
	cmd = HAL_DOS_CMD_TCP_FRAGERROR_DROP;
	else if(strcmp(argv[0], "icmpv4frage") == 0)
	cmd = HAL_DOS_CMD_ICMPv4_FRAGMENT_DROP;
	else if(strcmp(argv[0], "icmpv6frage") == 0)
	cmd = HAL_DOS_CMD_ICMPv6_FRAGMENT_DROP;
	else if(strcmp(argv[0], "icmpv4long") == 0)
	cmd = HAL_DOS_CMD_ICMPv4_LONGPING_DROP;
	else if(strcmp(argv[0], "icmpv6long") == 0)
	cmd = HAL_DOS_CMD_ICMPv6_LONGPING_DROP;
	if(strcmp(argv[1], "enable") == 0)
	ret = b53125_dos_type_enable(__msdkdriver, cmd, 1);
	else
	ret = b53125_dos_type_enable(__msdkdriver, cmd, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

int b53125_dos_init(sdk_driver_t *dev)
{
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_sdk_dos_test_cmd);
	sdk_dos.sdk_dos_enable_cb = b53125_dos_type_enable;
	sdk_dos.sdk_dos_tcp_hdr_size_cb = b53125_dos_type_size;
	sdk_dos.sdk_dos_icmp_size_cb = b53125_dos_type_size;
	sdk_dos.sdk_dos_icmpv6_size_cb = b53125_dos_type_size;
	return OK;
}
