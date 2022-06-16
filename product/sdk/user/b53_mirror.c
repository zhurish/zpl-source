/*
 * b53_mirror.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */


#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_mirror.h"
/****************************************************************************************/
//禁止使能镜像功能 //设置镜像目的端口
static int b53125_mirror_enable_set(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	u16 reg;//, loc;
	int ret = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_MIR_CAP_CTL, &reg);
	reg &= ~(CAP_PORT_MASK|MIRROR_EN|BLK_NOT_MIR);
	if(enable)
		reg |= BLK_NOT_MIR|MIRROR_EN|BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_MIR_CAP_CTL, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
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
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
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
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/***************************************************************************************************/

/***************************************************************************************************/
//设置镜像源端口
static int b53125_mirror_ingress_source(sdk_driver_t *dev, zpl_phyport_t port)
{
	int ret = 0;
	u16 source = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_IG_MIR_CTL, &source);
	source &= ~(MIRROR_MASK|(MIRROR_FILTER_MASK<<MIRROR_FILTER_SHIFT));
	source |= BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_IG_MIR_CTL, source);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_mirror_egress_source(sdk_driver_t *dev, zpl_phyport_t port)
{
	int ret = 0;
	u16 source = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_EG_MIR_CTL, &source);
	source &= ~(MIRROR_MASK|(MIRROR_FILTER_MASK<<MIRROR_FILTER_SHIFT));
	source |= BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_EG_MIR_CTL, source);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
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
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
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
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_mirror_source_enable_set(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable,  mirror_dir_en dir)
{
	int ret = 0;
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
//void *, zpl_bool enable, mirror_filter_t filter, mirror_dir_en type, mac_t *mac, mac_t *mac1
static int b53125_mirror_source_mactype_set(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable,
	mirror_filter_t mode, mirror_dir_en dir, mac_t *mac)
{
	int ret = 0;
		if(dir == MIRROR_INGRESS)
		{
			ret = b53125_mirror_ingress_source(dev,  port);
			ret |= b53125_mirror_ingress_filter(dev,  mode);
    		if(mode == MIRROR_FILTER_DA || mode == MIRROR_FILTER_SA)
				ret |= b53125_mirror_ingress_mac(dev, mac);
		}
		else if(dir == MIRROR_EGRESS)	
		{
			ret |= b53125_mirror_egress_source(dev,  port);
			ret |= b53125_mirror_egress_filter(dev,  mode);
    		if(mode == MIRROR_FILTER_DA || mode == MIRROR_FILTER_SA)
				ret |= b53125_mirror_egress_mac(dev, mac);
		}
		return ret;
}

int b53125_mirror_init(sdk_driver_t *dev)
{
	sdk_mirror.sdk_mirror_enable_cb = b53125_mirror_enable_set;
	sdk_mirror.sdk_mirror_source_enable_cb = b53125_mirror_source_enable_set;
	sdk_mirror.sdk_mirror_source_filter_enable_cb = b53125_mirror_source_mactype_set;
	return OK;
}
