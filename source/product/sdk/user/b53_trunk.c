/*
 * b53_trunk.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_trunk.h"

/* Trunk Registers */
/*************************************************************************/
static int b53125_trunk_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 regval = 0;
	ret |= b53125_read8(dev->sdk_device, B53_TRUNK_PAGE, B53_TRUNK_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TRUNK_PAGE, B53_TRUNK_CTL, regval);
	if(enable)
		regval |= B53_MAC_BASE_TRNK_EN;
	else
		regval &= ~B53_MAC_BASE_TRNK_EN;

	ret |= b53125_write8(dev->sdk_device, B53_TRUNK_PAGE, B53_TRUNK_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TRUNK_PAGE, B53_TRUNK_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_trunk_create(sdk_driver_t *dev, int id, zpl_bool enable)
{
	int ret = 0;
	u8 regval = 0;
	ret |= b53125_read8(dev->sdk_device, B53_TRUNK_PAGE, B53_TRUNK_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TRUNK_PAGE, B53_TRUNK_CTL, regval);
	if(enable)
		regval |= B53_MAC_BASE_TRNK_EN;
	else
		regval &= ~B53_MAC_BASE_TRNK_EN;

	ret |= b53125_write8(dev->sdk_device, B53_TRUNK_PAGE, B53_TRUNK_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TRUNK_PAGE, B53_TRUNK_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}
/*************************************************************************/
static int b53125_trunk_mode(sdk_driver_t *dev, int id, int mode)
{
	int ret = 0;
	u8 regval = 0;
	ret |= b53125_read8(dev->sdk_device, B53_TRUNK_PAGE, B53_TRUNK_CTL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TRUNK_PAGE, B53_TRUNK_CTL, regval);
	regval &= ~B53_TRK_HASH_ILLEGAL;
	regval |= mode;
	ret |= b53125_write8(dev->sdk_device, B53_TRUNK_PAGE, B53_TRUNK_CTL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TRUNK_PAGE, B53_TRUNK_CTL, regval);
	sdk_handle_return(ret);
	return ret;
}

/*************************************************************************/
static int b53125_trunk_add(sdk_driver_t *dev, int id, zpl_phyport_t port)
{
	int ret = 0;
	u16 regval = 0;
	u16 reg = 0;
	if(id == 0)
		reg = B53_TRUNK_GROUP0;
	else
		reg = B53_TRUNK_GROUP1;
	ret |= b53125_read16(dev->sdk_device, B53_TRUNK_PAGE, reg, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TRUNK_PAGE, reg, regval);
	regval |= BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_TRUNK_PAGE, reg, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TRUNK_PAGE, reg, regval);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_trunk_del(sdk_driver_t *dev, int id, zpl_phyport_t port)
{
	int ret = 0;
	u16 regval = 0;
	u16 reg = 0;
	if(id == 0)
		reg = B53_TRUNK_GROUP0;
	else
		reg = B53_TRUNK_GROUP1;
	ret |= b53125_read16(dev->sdk_device, B53_TRUNK_PAGE, reg, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TRUNK_PAGE, reg, regval);
	regval &= ~BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_TRUNK_PAGE, reg, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_TRUNK_PAGE, reg, regval);
	sdk_handle_return(ret);
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
