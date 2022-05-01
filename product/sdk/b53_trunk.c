/*
 * b53_trunk.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include <zplos_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_trunk.h"

/* Trunk Registers */
/*************************************************************************/
static int b53125_trunk_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev->sdk_device, B53_TRUNK_PAGE, B53_TRUNK_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_MAC_BASE_TRNK_EN;
	else
		port_ctrl &= ~B53_MAC_BASE_TRNK_EN;

	ret |= b53125_write8(dev->sdk_device, B53_TRUNK_PAGE, B53_TRUNK_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_trunk_create(sdk_driver_t *dev, int id, zpl_bool enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev->sdk_device, B53_TRUNK_PAGE, B53_TRUNK_CTL, &port_ctrl);
	if(enable)
		port_ctrl |= B53_MAC_BASE_TRNK_EN;
	else
		port_ctrl &= ~B53_MAC_BASE_TRNK_EN;

	ret |= b53125_write8(dev->sdk_device, B53_TRUNK_PAGE, B53_TRUNK_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
static int b53125_trunk_mode(sdk_driver_t *dev, int id, int mode)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev->sdk_device, B53_TRUNK_PAGE, B53_TRUNK_CTL, &port_ctrl);
	port_ctrl &= ~B53_TRK_HASH_ILLEGAL;
	port_ctrl |= mode;
	ret |= b53125_write8(dev->sdk_device, B53_TRUNK_PAGE, B53_TRUNK_CTL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
static int b53125_trunk_add(sdk_driver_t *dev, int id, zpl_phyport_t port)
{
	int ret = 0;
	u16 port_ctrl = 0;
	u16 reg = 0;
	if(id == 0)
		reg = B53_TRUNK_GROUP0;
	else
		reg = B53_TRUNK_GROUP1;
	ret |= b53125_read16(dev->sdk_device, B53_TRUNK_PAGE, reg, &port_ctrl);
	port_ctrl |= BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_TRUNK_PAGE, reg, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_trunk_del(sdk_driver_t *dev, int id, zpl_phyport_t port)
{
	int ret = 0;
	u16 port_ctrl = 0;
	u16 reg = 0;
	if(id == 0)
		reg = B53_TRUNK_GROUP0;
	else
		reg = B53_TRUNK_GROUP1;
	ret |= b53125_read16(dev->sdk_device, B53_TRUNK_PAGE, reg, &port_ctrl);
	port_ctrl &= ~BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_TRUNK_PAGE, reg, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
int b53125_trunk_init(sdk_driver_t *dev)
{
    sdk_trunk.sdk_trunk_enable_cb = b53125_trunk_enable;
    sdk_trunk.sdk_trunk_create_cb = b53125_trunk_create;
    sdk_trunk.sdk_trunk_mode_cb = b53125_trunk_mode;
    sdk_trunk.sdk_trunk_addif_cb = b53125_trunk_add;
    sdk_trunk.sdk_trunk_delif_cb = b53125_trunk_del;
	return OK;
}
