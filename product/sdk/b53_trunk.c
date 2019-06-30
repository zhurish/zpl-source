/*
 * b53_trunk.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include <zebra.h>
#include "b53_mdio.h"
#include "b53_regs.h"
#include "b53_driver.h"

/* Trunk Registers */
/*************************************************************************/
int b53125_trunk_mac_base_enable(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev, B53_TRUNK_PAGE, B53_TRUNK_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_MAC_BASE_TRNK_EN;
	else
		port_ctrl &= ~B53_MAC_BASE_TRNK_EN;

	ret |= b53125_write8(dev, B53_TRUNK_PAGE, B53_TRUNK_CTL, port_ctrl);
	return ret;
}

/*************************************************************************/
int b53125_trunk_mode(struct b53125_device *dev, int mode)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev, B53_TRUNK_PAGE, B53_TRUNK_CTL, &port_ctrl);
	port_ctrl &= ~B53_TRK_HASH_ILLEGAL;
	port_ctrl |= mode;
	ret |= b53125_write8(dev, B53_TRUNK_PAGE, B53_TRUNK_CTL, port_ctrl);
	return ret;
}

/*************************************************************************/
int b53125_trunk_add(struct b53125_device *dev, int id, int port)
{
	int ret = 0;
	u16 port_ctrl = 0;
	u16 reg = 0;
	if(id == 0)
		reg = B53_TRUNK_GROUP0;
	else
		reg = B53_TRUNK_GROUP1;
	ret |= b53125_read16(dev, B53_TRUNK_PAGE, reg, &port_ctrl);
	port_ctrl |= BIT(port);
	ret |= b53125_write16(dev, B53_TRUNK_PAGE, reg, port_ctrl);
	return ret;
}

int b53125_trunk_del(struct b53125_device *dev, int id, int port)
{
	int ret = 0;
	u16 port_ctrl = 0;
	u16 reg = 0;
	if(id == 0)
		reg = B53_TRUNK_GROUP0;
	else
		reg = B53_TRUNK_GROUP1;
	ret |= b53125_read16(dev, B53_TRUNK_PAGE, reg, &port_ctrl);
	port_ctrl &= ~BIT(port);
	ret |= b53125_write16(dev, B53_TRUNK_PAGE, reg, port_ctrl);
	return ret;
}
