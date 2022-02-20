/*
 * b53_phy.c
 *
 *  Created on: 2019年9月12日
 *      Author: DELL
 */
#include <zpl_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"



//
int b53125_phy_loopback(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev->sdk_device, B53_PORT_MII_PAGE(port), B53_MII_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_INTERNAL_LOOPBACK;
	}
	else
	{
		port_ctrl &= ~B53_INTERNAL_LOOPBACK;
	}
	ret |= b53125_write16(dev->sdk_device, B53_PORT_MII_PAGE(port), B53_MII_CTL, port_ctrl);
	return ret;
}


