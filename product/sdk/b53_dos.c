/*
 * b53_dos.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */


#include <zebra.h>
#include "nsm_dos.h"
#include "hal_dos.h"

#include "b53_mdio.h"
#include "b53_regs.h"
#include "sdk_driver.h"

/*************************************************************************/
int b53125_dos_icmpv6_longping_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_ICMP6_LONGPING_DROP_EN;
	else
		port_ctrl &= ~B53_ICMP6_LONGPING_DROP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_icmpv4_longping_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_ICMP4_LONGPING_DROP_EN;
	else
		port_ctrl &= ~B53_ICMP4_LONGPING_DROP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_icmpv6_fragment_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_ICMP6_FRAGMENT_DROP_EN;
	else
		port_ctrl &= ~B53_ICMP6_FRAGMENT_DROP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_icmpv4_fragment_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_ICMP4_FRAGMENT_DROP_EN;
	else
		port_ctrl &= ~B53_ICMP4_FRAGMENT_DROP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}

/*************************************************************************/
int b53125_dos_tcp_fragerror_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_FRAGERROR_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_FRAGERROR_DROP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_tcp_shorthdr_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_SHORTHDR_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_SHORTHDR_DROP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_tcp_synerror_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_SYNERROR_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_SYNERROR_DROP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_tcp_synfinscan_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_SYNFINSCAN_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_SYNFINSCAN_DROP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_tcp_nullscan_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_NULLSCAN_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_NULLSCAN_DROP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_tcp_xmassscan_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_XMASSCAN_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_XMASSCAN_DROP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_udp_blat_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_UDP_BLAT_DROP_EN;
	else
		port_ctrl &= ~B53_UDP_BLAT_DROP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_tcp_blat_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_TCP_BLAT_DROP_EN;
	else
		port_ctrl &= ~B53_TCP_BLAT_DROP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_ip_lan_drip_drop(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_DOS_PAGE, B53_DOS_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_IP_LAN_DRIP_EN;
	else
		port_ctrl &= ~B53_IP_LAN_DRIP_EN;

	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_DOS_CTL, port_ctrl);
	return ret;
}

/*************************************************************************/
int b53125_dos_tcphdr_minsize(struct b53125_device *dev, int minsize)
{
	int ret = 0;
	u8 port_ctrl = 0;
	port_ctrl = minsize;
	ret |= b53125_write8(dev, B53_DOS_PAGE, B53_MIN_TCPHDR_SIZE_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_icmp6_maxsize(struct b53125_device *dev, int minsize)
{
	int ret = 0;
	u32 port_ctrl = 0;
	port_ctrl = minsize;
	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_MAX_ICMPV6_SIZE_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_icmp4_maxsize(struct b53125_device *dev, int minsize)
{
	int ret = 0;
	u32 port_ctrl = 0;
	port_ctrl = minsize;
	ret |= b53125_write32(dev, B53_DOS_PAGE, B53_MAX_ICMPV4_SIZE_CTL, port_ctrl);
	return ret;
}
/*************************************************************************/
int b53125_dos_disable_lean(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	if(enable)
		port_ctrl |= BIT(0);
	else
		port_ctrl &= ~BIT(0);
	ret |= b53125_write8(dev, B53_DOS_PAGE, B53_DOS_DIS_LEARN_CTL, port_ctrl);
	return ret;
}

/*************************************************************************/
/*************************************************************************/


