/*
 * b53_mirror.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include <zebra.h>
#include "b53_mdio.h"
#include "b53_regs.h"
#include "sdk_driver.h"

/****************************************************************************************/
//禁止使能镜像功能
int b53125_mirror_enable(struct b53125_device *dev, ospl_bool enable)
{
	u16 reg;//, loc;
	int ret = 0;
	ret |= b53125_read16(dev, B53_MGMT_PAGE, B53_MIR_CAP_CTL, &reg);
	reg &= ~(CAP_PORT_MASK|MIRROR_EN);
	reg |= enable ? MIRROR_EN:0;
	ret |= b53125_write16(dev, B53_MGMT_PAGE, B53_MIR_CAP_CTL, reg);
	return ret;
}

//设置镜像目的端口
int b53125_mirror_destination_set(struct b53125_device *dev, int port)
{
	u16 reg;//, loc;
	int ret = 0;
	ret |= b53125_read16(dev, B53_MGMT_PAGE, B53_MIR_CAP_CTL, &reg);
	reg &= ~CAP_PORT_MASK;
	reg |= port;
	ret |= b53125_write16(dev, B53_MGMT_PAGE, B53_MIR_CAP_CTL, reg);
	return ret;
}

/***************************************************************************************************/
//设置镜像源MAC地址
int b53125_mirror_ingress_mac(struct b53125_device *dev, u8 *mac)
{
	u64 reg = 0;
	u16 value;
	int ret = 0;
	ospl_uint32 i;
	for (i = 0; i < 6; i+=2)
	{
		value = mac[i]<<16 | mac[i+1];
		reg |= value;
		reg <<= 16;
	}

	ret |= b53125_write48(dev, B53_MGMT_PAGE, B53_IG_MIR_MAC, reg);
	return ret;
}

int b53125_mirror_egress_mac(struct b53125_device *dev, u8 *mac)
{
	u64 reg = 0;
	u16 value;
	int ret = 0;
	ospl_uint32 i;
	for (i = 0; i < 6; i+=2)
	{
		value = mac[i]<<16 | mac[i+1];
		reg |= value;
		reg <<= 16;
	}

	ret |= b53125_write48(dev, B53_MGMT_PAGE, B53_EG_MIR_MAC, reg);
	return ret;
}
/***************************************************************************************************/
//设置镜像ID
int b53125_mirror_ingress_div(struct b53125_device *dev, u16 div)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev, B53_MGMT_PAGE, B53_EG_MIR_CTL, &reg);
	if(div)
		reg |= DIV_EN;
	else
		reg &= ~DIV_EN;
	ret |= b53125_write16(dev, B53_MGMT_PAGE, B53_EG_MIR_CTL, reg);

	ret |= b53125_write16(dev, B53_MGMT_PAGE, B53_IG_MIR_DIV, div);
	return ret;
}

int b53125_mirror_egress_div(struct b53125_device *dev, u16 div)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev, B53_MGMT_PAGE, B53_EG_MIR_CTL, &reg);
	if(div)
		reg |= DIV_EN;
	else
		reg &= ~DIV_EN;
	ret |= b53125_write16(dev, B53_MGMT_PAGE, B53_EG_MIR_CTL, reg);

	ret |= b53125_write16(dev, B53_MGMT_PAGE, B53_EG_MIR_DIV, div);
	return ret;
}
/***************************************************************************************************/
//设置镜像源端口
int b53125_mirror_ingress_source(struct b53125_device *dev, int port)
{
	int ret = 0;
	u16 source = 0;
	ret |= b53125_read16(dev, B53_MGMT_PAGE, B53_IG_MIR_CTL, &source);
	source &= ~MIRROR_MASK;
	source |= BIT(port);
	ret |= b53125_write16(dev, B53_MGMT_PAGE, B53_IG_MIR_CTL, source);
	return ret;
}

int b53125_mirror_egress_source(struct b53125_device *dev, int port)
{
	int ret = 0;
	u16 source = 0;
	ret |= b53125_read16(dev, B53_MGMT_PAGE, B53_EG_MIR_CTL, &source);
	source &= ~MIRROR_MASK;
	source |= BIT(port);
	ret |= b53125_write16(dev, B53_MGMT_PAGE, B53_EG_MIR_CTL, source);
	return ret;
}
/***************************************************************************************************/
int b53125_mirror_ingress_filter(struct b53125_device *dev, ospl_uint32 type)
{
	int ret = 0;
	u16 source = 0;
	ret |= b53125_read16(dev, B53_MGMT_PAGE, B53_IG_MIR_CTL, &source);
	source &= ~(MIRROR_FILTER_MASK << MIRROR_FILTER_SHIFT);
	source |= type << MIRROR_FILTER_SHIFT;
	ret |= b53125_write16(dev, B53_MGMT_PAGE, B53_IG_MIR_CTL, source);
	return ret;
}

int b53125_mirror_egress_filter(struct b53125_device *dev, ospl_uint32 type)
{
	int ret = 0;
	u16 source = 0;
	ret |= b53125_read16(dev, B53_MGMT_PAGE, B53_EG_MIR_CTL, &source);
	source &= ~(MIRROR_FILTER_MASK << MIRROR_FILTER_SHIFT);
	source |= type << MIRROR_FILTER_SHIFT;
	ret |= b53125_write16(dev, B53_MGMT_PAGE, B53_EG_MIR_CTL, source);
	return ret;
}
