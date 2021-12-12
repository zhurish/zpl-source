/*
 * b53_phy.c
 *
 *  Created on: 2019年9月12日
 *      Author: DELL
 */

#include <zpl_include.h>
#include "nsm_dos.h"
#include "hal_dos.h"

#include "b53_mdio.h"
#include "b53_regs.h"
#include "sdk_driver.h"



//
int b53125_phy_loopback(struct b53125_device *dev, int port, zpl_bool enable)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev, B53_PORT_MII_PAGE(port), B53_MII_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_INTERNAL_LOOPBACK;
	}
	else
	{
		port_ctrl &= ~B53_INTERNAL_LOOPBACK;
	}
	ret |= b53125_write16(dev, B53_EAP_PAGE, B53_EAP_GLOBAL, port_ctrl);
	return ret;
}


int b53125_snooping_enable(struct b53125_device *dev, zpl_uint32 type, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= type;
	}
	else
	{
		port_ctrl &= ~(type);
	}
	ret |= b53125_write32(dev, B53_EAP_PAGE, B53_EAP_GLOBAL, port_ctrl);
	return ret;
}
