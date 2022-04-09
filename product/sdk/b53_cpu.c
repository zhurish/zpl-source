/*
 * b53_port.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include <zplos_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"



int b53_brcm_hdr_setup(sdk_driver_t *dev, zpl_bool enable, zpl_phyport_t port)
{
	int ret = 0;
	u8 hdr_ctl, val;
	u16 reg;

	/* Resolve which bit controls the Broadcom tag */
	switch (port) {
	case 8:
		val = BRCM_HDR_P8_EN;
		break;
	case 7:
		val = BRCM_HDR_P7_EN;
		break;
	case 5:
		val = BRCM_HDR_P5_EN;
		break;
	default:
		val = 0;
		break;
	}
	/* Enable management mode if tagging is requested */
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, &hdr_ctl);
	if (enable)
		hdr_ctl |= SM_SW_FWD_MODE;
	else
		hdr_ctl &= ~SM_SW_FWD_MODE;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, hdr_ctl);

	/* Configure the appropriate IMP port */
	ret |= b53125_read8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, &hdr_ctl);
	if (port == 8)
		hdr_ctl |= GC_FRM_MGMT_PORT_MII;
	else if (port == 5)
		hdr_ctl |= GC_FRM_MGMT_PORT_M;
	ret |= b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, hdr_ctl);

	/* Enable Broadcom tags for IMP port */
	ret |= b53125_read8(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR, &hdr_ctl);
	if (enable)
		hdr_ctl |= val;
	else
		hdr_ctl &= ~val;
	ret |= b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR, hdr_ctl);

	/* Enable reception Broadcom tag for CPU TX (switch RX) to
	 * allow us to tag outgoing frames
	 */
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR_RX_DIS, &reg);
	if (enable)
		reg &= ~BIT(port);
	else
		reg |= BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR_RX_DIS, reg);

	/* Enable transmission of Broadcom tags from the switch (CPU RX) to
	 * allow delivering frames to the per-port net_devices
	 */
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR_TX_DIS, &reg);
	if (enable)
		reg &= ~BIT(port);
	else
		reg |= BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR_TX_DIS, reg);
	return ret;
}

//设置IMP接口模式（在managed mode模式下有效）
int b53125_imp_port_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u8 mgmt;
	/* Include IMP port in dumb forwarding mode
	 */
	ret |= b53125_read8(dev->sdk_device, B53_MGMT_PAGE, B53_MGMT_CTRL, &mgmt);
	mgmt |= B53_IMP_EN;
	ret |= b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_MGMT_CTRL, mgmt);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	if(ret == ERROR)
		return ERROR;
	return OK;
}

//使能禁止IMP接口收发功能
int b53125_imp_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	if(enable)
		port_ctrl = PORT_CTRL_RX_BCST_EN |
		    PORT_CTRL_RX_MCST_EN |
		    PORT_CTRL_RX_UCST_EN;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_IMP_CTRL, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

int b53125_imp_speed(sdk_driver_t *dev, int speed)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, &reg);
	reg &= ~ (3 << 2);
	switch (speed) {
	case SPEED_1000:
		reg |= PORT_OVERRIDE_SPEED_1000M;
		break;
	case SPEED_100:
		reg |= PORT_OVERRIDE_SPEED_100M;
		break;
	case SPEED_10:
		reg |= PORT_OVERRIDE_SPEED_10M;
		break;
	}
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

int b53125_imp_duplex(sdk_driver_t *dev, int duplex)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, &reg);
	reg &= ~ (PORT_OVERRIDE_FULL_DUPLEX);
	reg |= duplex;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

int b53125_imp_flow(sdk_driver_t *dev, zpl_bool rx, zpl_bool tx)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, &reg);
	reg &= ~ (PORT_OVERRIDE_RX_FLOW|PORT_OVERRIDE_TX_FLOW);
	reg |= rx ? PORT_OVERRIDE_RX_FLOW:0;
	reg |= tx ? PORT_OVERRIDE_TX_FLOW:0;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

//禁止使能wan接口（设置某个接口作为wan接口使用）
static int b53125_port_wan_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_CTRL_PAGE, B53_WAN_CTRL, &reg);
	{
		if(enable)
			reg |= BIT(port);
		else
			reg &= ~BIT(port);
	}
	ret |= b53125_write16(dev->sdk_device, B53_CTRL_PAGE, B53_WAN_CTRL, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


int b53125_cpu_init(sdk_driver_t *dev)
{
	sdk_global.sdk_wan_port_cb = b53125_port_wan_enable;
	return 0;
}