/*
 * b53_phy.c
 *
 *  Created on: 2019年9月12日
 *      Author: DELL
 */

#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_port.h"


//
int b53125_phy_loopback(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 regval = 0;
	ret |= b53125_read16(dev->sdk_device, B53_PORT_MII_PAGE(port), B53_MII_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_PORT_MII_PAGE(port), B53_MII_CTL, regval);
	if(enable)
	{
		regval |= B53_INTERNAL_LOOPBACK;
	}
	else
	{
		regval &= ~B53_INTERNAL_LOOPBACK;
	}
	ret |= b53125_write16(dev->sdk_device, B53_PORT_MII_PAGE(port), B53_MII_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_PORT_MII_PAGE(port), B53_MII_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}


int b53125_phy_powerdown(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 regval = 0;
	ret |= b53125_read16(dev->sdk_device, B53_PORT_MII_PAGE(port), B53_MII_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_PORT_MII_PAGE(port), B53_MII_CTL, regval);
	if(enable)
	{
		regval |= B53_POWERDOWN;
	}
	else
	{
		regval &= ~B53_POWERDOWN;
	}
	ret |= b53125_write16(dev->sdk_device, B53_PORT_MII_PAGE(port), B53_MII_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_PORT_MII_PAGE(port), B53_MII_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}