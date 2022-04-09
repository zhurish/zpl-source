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


/*************************************************************************/
/*************************************************************************/
//禁止使能接口收发功能
static int b53125_port_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	if(enable)
	{
		u8 reg = 0;
		ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
		reg |= (B53_INRANGE_ERR_DIS|B53_OUTRANGE_ERR_DIS);
		ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);		
		ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_CTRL(port), &reg);
		reg &= ~(PORT_CTRL_RX_DISABLE | PORT_CTRL_TX_DISABLE);
		ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_CTRL(port), reg);
	}
	else
	{
		u8 reg = 0;
		ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
		reg |= (B53_INRANGE_ERR_DIS|B53_OUTRANGE_ERR_DIS);
		ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);		
		ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_CTRL(port), &reg);
		reg |= PORT_CTRL_RX_DISABLE | PORT_CTRL_TX_DISABLE;
		ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_CTRL(port), reg);
	}
	ret |= b53125_phy_powerdown(dev,  port,  enable);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
//禁止使能端口保护
static int b53125_port_protected_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_CTRL_PAGE, B53_PROTECTED_CTRL, &reg);
	if(port == ((b53_device_t *)dev->sdk_device)->cpu_port)
	{
		if(enable)
			reg |= BIT(8);
		else
			reg &= ~BIT(8);
	}
	else
	{
		if(enable)
			reg |= BIT(port);
		else
			reg &= ~BIT(port);
	}
	ret |= b53125_write16(dev->sdk_device, B53_CTRL_PAGE, B53_PROTECTED_CTRL, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
/*************************************************************************/
static int b53125_pause_pass_rx(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_CTRL_PAGE, B53_PAUSE_PASS_RX, &reg);
	if(port == dev->cpu_port)
	{
		if(enable)
			reg |= (BIT(8));
		else
			reg &= ~(BIT(8));
	}
	else
	{
		if(enable)
			reg |= (BIT(port));
		else
			reg &= ~(BIT(port));
	}
	ret |= b53125_write16(dev->sdk_device, B53_CTRL_PAGE, B53_PAUSE_PASS_RX, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_pause_pass_tx(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_CTRL_PAGE, B53_PAUSE_PASS_TX, &reg);
	if(port == dev->cpu_port)
	{
		if(enable)
			reg |= (BIT(8));
		else
			reg &= ~(BIT(8));
	}
	else
	{
		if(enable)
			reg |= (BIT(port));
		else
			reg &= ~(BIT(port));
	}
	ret |= b53125_write16(dev->sdk_device, B53_CTRL_PAGE, B53_PAUSE_PASS_TX, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/* Pause Frame Detection Control Register (8 bit) */
static int b53125_puase_frame_detection(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_PAUSE_FRAME_DETECTION, &reg);
	reg &= ~ (PAUSE_IGNORE_DA);
	reg |= enable ? PAUSE_IGNORE_DA:0;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_PAUSE_FRAME_DETECTION, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_pasue_transmit_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u32 reg = 0;
	ret |= b53125_puase_frame_detection(dev, enable);
	ret |= b53125_read32(dev->sdk_device, B53_CTRL_PAGE, B53_PAUSE_CAP, &reg);
	reg &= ~(EN_PAUSE_CAP_MASK << EN_TX_PAUSE_CAP);
	if(port == dev->cpu_port)
	{
		if(enable)
			reg |= (BIT(8) << EN_TX_PAUSE_CAP);
		else
			reg &= ~(BIT(8) << EN_TX_PAUSE_CAP);
	}
	else
	{
		if(enable)
			reg |= (BIT(port) << EN_TX_PAUSE_CAP);
		else
			reg &= ~(BIT(port) << EN_TX_PAUSE_CAP);
	}
	ret |= b53125_write32(dev->sdk_device, B53_CTRL_PAGE, B53_PAUSE_CAP, reg);
	ret |= b53125_pause_pass_tx(dev,  port,  enable);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_pasue_receive_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u32 reg = 0;
	ret |= b53125_puase_frame_detection(dev, enable);
	ret |= b53125_read32(dev->sdk_device, B53_CTRL_PAGE, B53_PAUSE_CAP, &reg);
	reg &= ~(EN_PAUSE_CAP_MASK << EN_RX_PAUSE_CAP);
	if(port == dev->cpu_port)
	{
		if(enable)
			reg |= (BIT(8) << EN_RX_PAUSE_CAP);
		else
			reg &= ~(BIT(8) << EN_RX_PAUSE_CAP);
	}
	else
	{
		if(enable)
			reg |= (BIT(port) << EN_RX_PAUSE_CAP);
		else
			reg &= ~(BIT(port) << EN_RX_PAUSE_CAP);
	}
	ret |= b53125_write32(dev->sdk_device, B53_CTRL_PAGE, B53_PAUSE_CAP, reg);
	ret |= b53125_pause_pass_rx(dev,  port,  enable);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_pasue_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool tx, zpl_bool rx)
{
	int ret = 0;
	ret |= b53125_pasue_transmit_enable(dev,  port,  tx);
	ret |= b53125_pasue_receive_enable(dev,  port,  rx);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;	
}
/*************************************************************************/
//设置MAC地址表查找失败的报文转发的目的端口
int b53125_unknow_unicast_forward_port(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_CTRL_PAGE, B53_UC_FLOOD_MASK, &reg);
	if(port == dev->cpu_port)
	{
		if(enable)
			reg |= (BIT(8));
		else
			reg &= ~(BIT(8));
	}
	else
	{
		if(enable)
			reg |= (BIT(port));
		else
			reg &= ~(BIT(port));
	}
	ret |= b53125_write16(dev->sdk_device, B53_CTRL_PAGE, B53_UC_FLOOD_MASK, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

int b53125_unknow_multicast_forward_port(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_CTRL_PAGE, B53_MC_FLOOD_MASK, &reg);
	if(port == dev->cpu_port)
	{
		if(enable)
			reg |= (BIT(8));
		else
			reg &= ~(BIT(8));
	}
	else
	{
		if(enable)
			reg |= (BIT(port));
		else
			reg &= ~(BIT(port));
	}
	ret |= b53125_write16(dev->sdk_device, B53_CTRL_PAGE, B53_MC_FLOOD_MASK, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

int b53125_unknow_ipmulticast_forward_port(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_CTRL_PAGE, B53_IPMC_FLOOD_MASK, &reg);
	if(port == dev->cpu_port)
	{
		if(enable)
			reg |= (BIT(8));
		else
			reg &= ~(BIT(8));
	}
	else
	{
		if(enable)
			reg |= (BIT(port));
		else
			reg &= ~(BIT(port));
	}
	ret |= b53125_write16(dev->sdk_device, B53_CTRL_PAGE, B53_IPMC_FLOOD_MASK, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


/*************************************************************************/
//禁止使能端口学习
int b53125_enable_learning(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev->sdk_device, B53_CTRL_PAGE, B53_DIS_LEARNING, &port_ctrl);
	if(port == dev->cpu_port)
	{
		if(enable)
			port_ctrl &= ~BIT(8);
		else
			port_ctrl |= BIT(8);
	}
	else
	{
		if(enable)
			port_ctrl &= ~BIT(port);
		else
			port_ctrl |= BIT(port);
	}
	ret |= b53125_write16(dev->sdk_device, B53_CTRL_PAGE, B53_DIS_LEARNING, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

//禁止使能端口软学习（未知报文进入CPU）
static int b53125_software_learning(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev->sdk_device, B53_CTRL_PAGE, B53_SOFTWARE_LEARNING, &port_ctrl);
	if(port == dev->cpu_port)
	{
		if(enable)
			port_ctrl |= BIT(8);
		else
			port_ctrl &= ~BIT(8);
	}
	else
	{
		if(enable)
			port_ctrl |= BIT(port);
		else
			port_ctrl &= ~BIT(port);
	}
	ret |= b53125_write16(dev->sdk_device, B53_CTRL_PAGE, B53_SOFTWARE_LEARNING, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
/*************************************************************************/
static int b53125_port_set_speed(sdk_driver_t *dev, zpl_phyport_t port, int speed)
{
	int ret = 0;
	u8 reg, val, off;

	/* Override the port settings */
	if (port == dev->cpu_port) {
		off = B53_PORT_OVERRIDE_CTRL;
		val = PORT_OVERRIDE_EN;
	} else {
		off = B53_GMII_PORT_OVERRIDE_CTRL(port);
		val = GMII_PO_EN;
	}

	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, off, &reg);
	reg &= ~(3<<GMII_PO_SPEED_S);
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
	default:
		return ERROR;
	}
	reg |= val;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, off, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_port_set_duplex(sdk_driver_t *dev, zpl_phyport_t port, int duplex)
{
	int ret = 0;
	u8 reg, val, off;

	/* Override the port settings */
	if (port == dev->cpu_port) {
		off = B53_PORT_OVERRIDE_CTRL;
		val = PORT_OVERRIDE_EN;
	} else {
		off = B53_GMII_PORT_OVERRIDE_CTRL(port);
		val = GMII_PO_EN;
	}

	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, off, &reg);
	reg &= ~(GMII_PO_FULL_DUPLEX);

	if (duplex == DUPLEX_FULL)
		reg |= PORT_OVERRIDE_FULL_DUPLEX;
	else
		reg &= ~PORT_OVERRIDE_FULL_DUPLEX;
	reg |= val;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, off, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_port_set_flow(sdk_driver_t *dev, zpl_phyport_t port, int tx, int rx)
{
	int ret = 0;
	u8 reg, val, off;

	/* Override the port settings */
	if (port == dev->cpu_port) {
		off = B53_PORT_OVERRIDE_CTRL;
		val = PORT_OVERRIDE_EN;
	} else {
		off = B53_GMII_PORT_OVERRIDE_CTRL(port);
		val = GMII_PO_EN;
	}

	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, off, &reg);
	reg &= ~(PORT_OVERRIDE_RX_FLOW|PORT_OVERRIDE_TX_FLOW);

	if (rx)
		reg |= PORT_OVERRIDE_RX_FLOW;
	if (tx)
		reg |= PORT_OVERRIDE_TX_FLOW;

	reg |= val;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, off, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_port_set_link_force(sdk_driver_t *dev, zpl_phyport_t port, int link)
{
	int ret = 0;
	u8 reg, val, off;
	if (port == dev->cpu_port) {
		off = B53_PORT_OVERRIDE_CTRL;
		val = PORT_OVERRIDE_EN;
	} else {
		off = B53_GMII_PORT_OVERRIDE_CTRL(port);
		val = GMII_PO_EN;
	}
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, off, &reg);
	reg |= val;
	if (link)
		reg |= PORT_OVERRIDE_LINK;
	else
		reg &= ~PORT_OVERRIDE_LINK;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, off, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/

/*************************************************************************/

//获取端口状态
static zpl_bool b53125_port_get_link(sdk_driver_t *dev, zpl_phyport_t port)
{
	zpl_bool link;
	u16 sts;

	b53125_read16(dev->sdk_device, B53_STAT_PAGE, B53_LINK_STAT, &sts);
	link = !!(sts & BIT(port));
	//dsa_port_phylink_mac_change(ds, port, link);
	
	return link;
}

static zpl_uint32 b53125_port_get_speed(sdk_driver_t *dev, zpl_phyport_t port)
{
	u32 sts;

	b53125_read32(dev->sdk_device, B53_STAT_PAGE, B53_SPEED_STAT, &sts);
	//dsa_port_phylink_mac_change(ds, port, link);
	return SPEED_PORT_GE(sts, port);
}

static zpl_uint32 b53125_port_get_duplex(sdk_driver_t *dev, zpl_phyport_t port)
{
	u16 sts;

	b53125_read16(dev->sdk_device, B53_STAT_PAGE, B53_DUPLEX_STAT_GE, &sts);
	//dsa_port_phylink_mac_change(ds, port, link);
	return (sts & BIT(port))>>(port);
}






int b53125_port_init(sdk_driver_t *dev)
{
	int ret= 0;
	sdk_port.sdk_port_enable_cb = b53125_port_enable;
	sdk_port.sdk_port_link_cb = b53125_port_set_link_force;
	sdk_port.sdk_port_speed_cb = b53125_port_set_speed;
	sdk_port.sdk_port_duplex_cb = b53125_port_set_duplex;
	sdk_port.sdk_port_flow_cb = b53125_port_set_flow;
	
	sdk_port.sdk_port_state_get_cb = b53125_port_get_link;
	sdk_port.sdk_port_speed_get_cb = b53125_port_get_speed;
	sdk_port.sdk_port_duplex_get_cb = b53125_port_get_duplex;
	sdk_port.sdk_port_loop_cb = b53125_phy_loopback;

	sdk_port.sdk_port_learning_enable_cb = b53125_enable_learning;
	sdk_port.sdk_port_swlearning_enable_cb = b53125_software_learning;
	sdk_port.sdk_port_protected_enable_cb = b53125_port_protected_enable;
	sdk_port.sdk_port_pause_cb = b53125_pasue_enable;

	//sdk_port.sdk_port_wan_enable_cb = b53125_port_wan_enable;
	//sdk_port.sdk_port_mac_cb)(void *, zpl_phyport_t, zpl_uint8 *, zpl_bool);
	//sdk_port.sdk_port_mtu_cb)(void *, zpl_phyport_t, zpl_uint32);
	//sdk_port.sdk_port_vrf_cb)(void *, zpl_phyport_t, zpl_uint32);
	//sdk_port.sdk_port_mode_cb)(void *, zpl_phyport_t, zpl_uint32);
	ret |= b53125_enable_learning(dev, ((b53_device_t *)dev->sdk_device)->cpu_port, zpl_true);
	return ret;
}

int b53125_port_start(sdk_driver_t *dev)
{
	int ret= 0;

	ret |= b53125_enable_learning(dev, 0, zpl_true);
	ret |= b53125_enable_learning(dev, 1, zpl_true);
	ret |= b53125_enable_learning(dev, 2, zpl_true);
	ret |= b53125_enable_learning(dev, 3, zpl_true);
	ret |= b53125_enable_learning(dev, 4, zpl_true);
	ret |= b53125_enable_learning(dev, 6, zpl_true);
	return ret;
}