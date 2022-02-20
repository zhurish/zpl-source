/*
 * b53_port.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include <zpl_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"

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
	return ret;
}


