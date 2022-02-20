/*
 * b53_mirror.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include <zpl_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"

/****************************************************************************************/
//禁止使能镜像功能
static int b53125_mirror_enable(sdk_driver_t *dev, zpl_bool enable)
{
	u16 reg;//, loc;
	int ret = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_MIR_CAP_CTL, &reg);
	reg &= ~(CAP_PORT_MASK|MIRROR_EN);
	reg |= enable ? MIRROR_EN:0;
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_MIR_CAP_CTL, reg);
	return ret;
}

//设置镜像目的端口
static int b53125_mirror_destination_set(sdk_driver_t *dev, zpl_phyport_t port)
{
	u16 reg;//, loc;
	int ret = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_MIR_CAP_CTL, &reg);
	reg &= ~CAP_PORT_MASK;
	reg |= port;
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_MIR_CAP_CTL, reg);
	return ret;
}
static int b53125_mirror_enable_set(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = b53125_mirror_enable(dev,  enable);
	ret |= b53125_mirror_destination_set(dev,  port);
	return ret;
}
/***************************************************************************************************/
//设置镜像源MAC地址
static int b53125_mirror_ingress_mac(sdk_driver_t *dev, u8 *mac)
{
	u64 reg = 0;
	u16 value;
	int ret = 0;
	zpl_uint32 i;
	for (i = 0; i < 6; i+=2)
	{
		value = mac[i]<<16 | mac[i+1];
		reg |= value;
		reg <<= 16;
	}

	ret |= b53125_write48(dev->sdk_device, B53_MGMT_PAGE, B53_IG_MIR_MAC, reg);
	return ret;
}

static int b53125_mirror_egress_mac(sdk_driver_t *dev, u8 *mac)
{
	u64 reg = 0;
	u16 value;
	int ret = 0;
	zpl_uint32 i;
	for (i = 0; i < 6; i+=2)
	{
		value = mac[i]<<16 | mac[i+1];
		reg |= value;
		reg <<= 16;
	}

	ret |= b53125_write48(dev->sdk_device, B53_MGMT_PAGE, B53_EG_MIR_MAC, reg);
	return ret;
}
/***************************************************************************************************/
//设置镜像ID
static int b53125_mirror_ingress_div(sdk_driver_t *dev, u16 div)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_EG_MIR_CTL, &reg);
	if(div)
		reg |= DIV_EN;
	else
		reg &= ~DIV_EN;
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_EG_MIR_CTL, reg);

	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_IG_MIR_DIV, div);
	return ret;
}

static int b53125_mirror_egress_div(sdk_driver_t *dev, u16 div)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_EG_MIR_CTL, &reg);
	if(div)
		reg |= DIV_EN;
	else
		reg &= ~DIV_EN;
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_EG_MIR_CTL, reg);

	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_EG_MIR_DIV, div);
	return ret;
}
/***************************************************************************************************/
//设置镜像源端口
static int b53125_mirror_ingress_source(sdk_driver_t *dev, zpl_phyport_t port)
{
	int ret = 0;
	u16 source = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_IG_MIR_CTL, &source);
	source &= ~MIRROR_MASK;
	source |= BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_IG_MIR_CTL, source);
	return ret;
}

static int b53125_mirror_egress_source(sdk_driver_t *dev, zpl_phyport_t port)
{
	int ret = 0;
	u16 source = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_EG_MIR_CTL, &source);
	source &= ~MIRROR_MASK;
	source |= BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_EG_MIR_CTL, source);
	return ret;
}
/***************************************************************************************************/
static int b53125_mirror_ingress_filter(sdk_driver_t *dev, zpl_uint32 type)
{
	int ret = 0;
	u16 source = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_IG_MIR_CTL, &source);
	source &= ~(MIRROR_FILTER_MASK << MIRROR_FILTER_SHIFT);
	source |= type << MIRROR_FILTER_SHIFT;
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_IG_MIR_CTL, source);
	return ret;
}

static int b53125_mirror_egress_filter(sdk_driver_t *dev, zpl_uint32 type)
{
	int ret = 0;
	u16 source = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_EG_MIR_CTL, &source);
	source &= ~(MIRROR_FILTER_MASK << MIRROR_FILTER_SHIFT);
	source |= type << MIRROR_FILTER_SHIFT;
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_EG_MIR_CTL, source);
	return ret;
}

static int b53125_mirror_source_enable_set(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable, mirror_mode_t mode, mirror_dir_en dir)
{
	int ret = 0;
	if(MIRROR_SOURCE_PORT == mode)
	{
		if(dir == MIRROR_INGRESS)
			ret = b53125_mirror_ingress_source(dev,  port);
		else if(dir == MIRROR_EGRESS)	
			ret = b53125_mirror_egress_source(dev,  port);
		else
		{
			ret = b53125_mirror_ingress_source(dev,  port);	
			ret |= b53125_mirror_egress_source(dev,  port);
		}
		return ret;
	}
	else if(MIRROR_SOURCE_MAC == mode)
	{
		if(dir == MIRROR_INGRESS)
			ret = b53125_mirror_ingress_filter(dev,  port);
		else if(dir == MIRROR_EGRESS)	
			ret = b53125_mirror_egress_filter(dev,  port);
		else
		{
			ret = b53125_mirror_ingress_filter(dev,  port);	
			ret |= b53125_mirror_egress_filter(dev,  port);
		}
		return ret;
	}
	else if(MIRROR_SOURCE_DIV == mode)
	{
		if(dir == MIRROR_INGRESS)
			ret = b53125_mirror_ingress_div(dev,  port);
		else if(dir == MIRROR_EGRESS)	
			ret = b53125_mirror_egress_div(dev,  port);
		else
		{
			ret = b53125_mirror_ingress_div(dev,  port);	
			ret |= b53125_mirror_egress_div(dev,  port);
		}
		return ret;
	}
	return ERROR;
}
//void *, zpl_bool enable, mirror_filter_t filter, mirror_dir_en type, mac_t *mac, mac_t *mac1
int b53125_mirror_source_mactype_set(sdk_driver_t *dev, mac_t *mac, mirror_filter_t mode, mirror_dir_en dir)
{
	if(mode == MIRROR_FILTER_BOTH)
		mode = 0;

	int ret = 0;
		if(dir == MIRROR_INGRESS)
		{
			ret = b53125_mirror_ingress_filter(dev,  mode);
    		if(mode == MIRROR_FILTER_DA || mode == MIRROR_FILTER_SA)
				ret |= b53125_mirror_ingress_mac(dev, mac);
		}
		else if(dir == MIRROR_EGRESS)	
		{
			ret = b53125_mirror_egress_filter(dev,  mode);
    		if(mode == MIRROR_FILTER_DA || mode == MIRROR_FILTER_SA)
				ret |= b53125_mirror_egress_mac(dev, mac);
		}
		else
		{
			ret = b53125_mirror_ingress_filter(dev,  mode);	
			ret |= b53125_mirror_egress_filter(dev,  mode);
    		if(mode == MIRROR_FILTER_DA || mode == MIRROR_FILTER_SA)
				ret |= b53125_mirror_ingress_mac(dev, mac);
    		if(mode == MIRROR_FILTER_DA || mode == MIRROR_FILTER_SA)
				ret |= b53125_mirror_egress_mac(dev, mac);
		}
		return ret;
}

int b53125_mirror_init(void)
{
	sdk_mirror.sdk_mirror_enable_cb = b53125_mirror_enable_set;
	sdk_mirror.sdk_mirror_source_enable_cb = b53125_mirror_source_enable_set;
	sdk_mirror.sdk_mirror_source_filter_enable_cb = NULL;
	return OK;
}
