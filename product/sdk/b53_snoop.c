/*
 * b53_global.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */

#include <zplos_include.h>

#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_snoop.h"

static int b53125_mldqry_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_MLD_QRY_EN;
	}
	else
	{
		port_ctrl &= ~(B53_MLD_QRY_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_mldqry_tocpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl |= B53_MLD_QRY_FWD_MODE;
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_mldqry_cpoycpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl &= ~(B53_MLD_QRY_FWD_MODE);
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_mldqry_snoop_enable(sdk_driver_t *dev, zpl_uint32 type, zpl_bool enable)
{
	int ret = 0;
	if(type)
	{
	ret = b53125_mldqry_enable(dev, enable);
	ret |= b53125_mldqry_cpoycpu_enable(dev);
	}
	else
	{
	ret = b53125_mldqry_enable(dev, enable);
	ret |= b53125_mldqry_tocpu_enable(dev);
	}
	return ret;
}



static int b53125_mld_tocpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl |= B53_MLD_RPTDONE_FWD_MODE;
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_mld_cpoycpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl &= ~(B53_MLD_RPTDONE_FWD_MODE);
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_mld_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_MLD_RPTDONE_EN;
	}
	else
	{
		port_ctrl &= ~(B53_MLD_RPTDONE_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_mld_snoop_enable(sdk_driver_t *dev, zpl_uint32 type, zpl_bool enable)
{
	int ret = 0;
	if(type)
	{
	ret = b53125_mld_enable(dev, enable);
	ret |= b53125_mld_cpoycpu_enable(dev);
	}
	else
	{
		ret = b53125_mld_enable(dev, enable);
	ret |= b53125_mld_tocpu_enable(dev);
	}
	return ret;
}



static int b53125_igmpunknow_tocpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl |= B53_IGMP_UKN_FWD_MODE;
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_igmpunknow_cpoycpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl &= ~(B53_IGMP_UKN_FWD_MODE);
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_igmpunknow_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_IGMP_UKN_EN;
	}
	else
	{
		port_ctrl &= ~(B53_IGMP_UKN_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_igmpunknow_snoop_enable(sdk_driver_t *dev, zpl_uint32 type, zpl_bool enable)
{
	int ret = 0;
	if(type)
	{
	ret = b53125_igmpunknow_enable(dev, enable);
	ret |= b53125_igmpunknow_cpoycpu_enable(dev);
	}
	else
	{
	ret = b53125_igmpunknow_enable(dev, enable);
	ret |= b53125_igmpunknow_tocpu_enable(dev);
	}
	return ret;
}


static int b53125_igmpqry_tocpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl |= B53_IGMP_QRY_FWD_MODE;
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_igmpqry_cpoycpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl &= ~(B53_IGMP_QRY_FWD_MODE);
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_igmpqry_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_IGMP_QRY_EN;
	}
	else
	{
		port_ctrl &= ~(B53_IGMP_QRY_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_igmpqry_snoop_enable(sdk_driver_t *dev, zpl_uint32 type, zpl_bool enable)
{
	int ret = 0;
	if(type)
	{
	ret = b53125_igmpqry_enable(dev, enable);
	ret |= b53125_igmpqry_cpoycpu_enable(dev);
	}
	else
	{
	ret = b53125_igmpqry_enable(dev, enable);
	ret |= b53125_igmpqry_tocpu_enable(dev);
	}
	return ret;
}



static int b53125_igmp_tocpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl |= B53_IGMP_RPTLVE_FWD_MODE;
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_igmp_cpoycpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl &= ~(B53_IGMP_RPTLVE_FWD_MODE);
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_igmp_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_IGMP_RPTLVE_EN;
	}
	else
	{
		port_ctrl &= ~(B53_IGMP_RPTLVE_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


static int b53125_igmp_snoop_enable(sdk_driver_t *dev, zpl_uint32 type, zpl_bool enable)
{
	int ret = 0;
	if(type)
	{
		ret = b53125_igmp_enable(dev, enable);
		ret |= b53125_igmp_cpoycpu_enable(dev);
	}
	else
	{
		ret = b53125_igmp_enable(dev, enable);
		ret |= b53125_igmp_tocpu_enable(dev);
	}
	return ret;
}


static int b53125_igmp_ipcheck_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_IGMP_DIP_EN;
	}
	else
	{
		port_ctrl &= ~(B53_IGMP_DIP_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_arp_copycpu_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_ARP_EN;
	}
	else
	{
		port_ctrl &= ~(B53_ARP_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_rarp_copycpu_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_RARP_EN;
	}
	else
	{
		port_ctrl &= ~(B53_RARP_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
static int b53125_dhcp_copycpu_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_DHCP_EN;
	}
	else
	{
		port_ctrl &= ~(B53_DHCP_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

int b53125_snooping_init(sdk_driver_t *dev)
{
	sdk_snooping.sdk_mldqry_snoop_cb = b53125_mldqry_snoop_enable;
	sdk_snooping.sdk_mld_snoop_cb = b53125_mld_snoop_enable;
	sdk_snooping.sdk_igmpunknow_snoop_cb = b53125_igmpunknow_snoop_enable;
	sdk_snooping.sdk_igmpqry_snoop_cb = b53125_igmpqry_snoop_enable;
	sdk_snooping.sdk_igmp_snoop_cb = b53125_igmp_snoop_enable;
	sdk_snooping.sdk_igmp_ipcheck_cb = b53125_igmp_ipcheck_enable;
	sdk_snooping.sdk_arp_snoop_cb = b53125_arp_copycpu_enable;//全局使能接收BPDU报文
	sdk_snooping.sdk_rarp_snoop_cb = b53125_rarp_copycpu_enable;
	sdk_snooping.sdk_dhcp_snoop_cb = b53125_dhcp_copycpu_enable;
	return 0;
}
