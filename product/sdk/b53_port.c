/*
 * b53_port.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include <zebra.h>


#include "b53_mdio.h"
#include "b53_regs.h"
#include "sdk_driver.h"

//使能禁止IMP接口收发功能
int b53125_imp_enable(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	if(enable)
		port_ctrl = PORT_CTRL_RX_BCST_EN |
		    PORT_CTRL_RX_MCST_EN |
		    PORT_CTRL_RX_UCST_EN;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_IMP_CTRL, port_ctrl);
	return ret;
}

int b53125_imp_speed(struct b53125_device *dev, int speed)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, &reg);
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
	ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	return ret;
}

int b53125_imp_duplex(struct b53125_device *dev, int duplex)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, &reg);
	reg &= ~ (PORT_OVERRIDE_FULL_DUPLEX);
	reg |= duplex;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	return ret;
}

int b53125_imp_flow(struct b53125_device *dev, BOOL rx, BOOL tx)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, &reg);
	reg &= ~ (PORT_OVERRIDE_RX_FLOW|PORT_OVERRIDE_TX_FLOW);
	reg |= rx ? PORT_OVERRIDE_RX_FLOW:0;
	reg |= tx ? PORT_OVERRIDE_TX_FLOW:0;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	return ret;
}

/*************************************************************************/
/*************************************************************************/
//禁止使能接口收发功能
int b53125_port_enable(struct b53125_device *dev, int port, BOOL enable)
{
	int ret = 0;
	if(enable)
	{
		u8 reg = 0;
		ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_PORT_CTRL(port), &reg);
		reg &= ~(PORT_CTRL_RX_DISABLE | PORT_CTRL_TX_DISABLE);
		ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_PORT_CTRL(port), reg);
	}
	else
	{
		u8 reg = 0;
		ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_PORT_CTRL(port), &reg);
		reg |= PORT_CTRL_RX_DISABLE | PORT_CTRL_TX_DISABLE;
		ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_PORT_CTRL(port), reg);
	}
	return ret;
}
/*************************************************************************/
//禁止使能端口保护
int b53125_port_protected_enable(struct b53125_device *dev, int port, BOOL enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev, B53_CTRL_PAGE, B53_PROTECTED_CTRL, &reg);
	if(port == dev->cpu_port)
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
	ret |= b53125_write16(dev, B53_CTRL_PAGE, B53_PROTECTED_CTRL, reg);
	return ret;
}

//禁止使能wan接口（设置某个接口作为wan接口使用）
int b53125_port_wan_enable(struct b53125_device *dev, int port, BOOL enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev, B53_CTRL_PAGE, B53_WAN_CTRL, &reg);
	{
		if(enable)
			reg |= BIT(port);
		else
			reg &= ~BIT(port);
	}
	ret |= b53125_write16(dev, B53_CTRL_PAGE, B53_WAN_CTRL, reg);
	return ret;
}
/*************************************************************************/
int b53125_pasue_transmit_enable(struct b53125_device *dev, int port, BOOL enable)
{
	int ret = 0;
	u32 reg = 0;
	ret |= b53125_read32(dev, B53_CTRL_PAGE, B53_PAUSE_CAP, &reg);
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
	ret |= b53125_write32(dev, B53_CTRL_PAGE, B53_PAUSE_CAP, reg);
	return ret;
}

int b53125_pasue_receive_enable(struct b53125_device *dev, int port, BOOL enable)
{
	int ret = 0;
	u32 reg = 0;
	ret |= b53125_read32(dev, B53_CTRL_PAGE, B53_PAUSE_CAP, &reg);
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
	ret |= b53125_write32(dev, B53_CTRL_PAGE, B53_PAUSE_CAP, reg);
	return ret;
}

/*************************************************************************/
//设置非管理模式下BPDU报文转发还是丢弃
int b53125_bpdu_forward(struct b53125_device *dev, u8 *mac, BOOL enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, &reg);
	if(mac[5]== 0)
	{
		reg &= ~ (B53_BPDU_00);
		reg |= enable ? B53_BPDU_00:0;
	}
	else if(mac[5] >= 0x02 && mac[5] <= 0x0f)
	{
		reg &= ~ (B53_BPDU_02);
		reg |= enable ? B53_BPDU_02:0;
	}
	else if(mac[5] == 0x10)
	{
		reg &= ~ (B53_BPDU_10);
		reg |= enable ? B53_BPDU_10:0;
	}
	else if(mac[5] >= 0x11 && mac[5] <= 0x1f)
	{
		reg &= ~ (B53_BPDU_11);
		reg |= enable ? B53_BPDU_11:0;
	}
	else if(mac[5] >= 0x20 && mac[5] <= 0x2f)
	{
		reg &= ~ (B53_BPDU_20);
		reg |= enable ? B53_BPDU_20:0;
	}
	ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, reg);
	return ret;
}
/*************************************************************************/
//设置MAC地址表查找失败的报文转发的目的端口
int b53125_unknow_unicast_forward_port(struct b53125_device *dev, int port, BOOL enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev, B53_CTRL_PAGE, B53_UC_FLOOD_MASK, &reg);
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
	ret |= b53125_write16(dev, B53_CTRL_PAGE, B53_UC_FLOOD_MASK, reg);
	return ret;
}

int b53125_unknow_multicast_forward_port(struct b53125_device *dev, int port, BOOL enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev, B53_CTRL_PAGE, B53_MC_FLOOD_MASK, &reg);
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
	ret |= b53125_write16(dev, B53_CTRL_PAGE, B53_MC_FLOOD_MASK, reg);
	return ret;
}

int b53125_unknow_ipmulticast_forward_port(struct b53125_device *dev, int port, BOOL enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev, B53_CTRL_PAGE, B53_IPMC_FLOOD_MASK, &reg);
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
	ret |= b53125_write16(dev, B53_CTRL_PAGE, B53_IPMC_FLOOD_MASK, reg);
	return ret;
}
/*************************************************************************/
int b53125_pause_pass_rx(struct b53125_device *dev, int port, BOOL enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev, B53_CTRL_PAGE, B53_PAUSE_PASS_RX, &reg);
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
	ret |= b53125_write16(dev, B53_CTRL_PAGE, B53_PAUSE_PASS_RX, reg);
	return ret;
}

int b53125_pause_pass_tx(struct b53125_device *dev, int port, BOOL enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev, B53_CTRL_PAGE, B53_PAUSE_PASS_TX, &reg);
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
	ret |= b53125_write16(dev, B53_CTRL_PAGE, B53_PAUSE_PASS_TX, reg);
	return ret;
}
/*************************************************************************/
//禁止使能端口学习
int b53125_enable_learning(struct b53125_device *dev, int port, BOOL enable)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev, B53_CTRL_PAGE, B53_DIS_LEARNING, &port_ctrl);
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
	ret |= b53125_write16(dev, B53_CTRL_PAGE, B53_DIS_LEARNING, port_ctrl);
	return ret;
}

//禁止使能端口软学习（未知报文进入CPU）
int b53125_software_learning(struct b53125_device *dev, int port, BOOL enable)
{
	int ret = 0;
	u16 port_ctrl = 0;
	ret |= b53125_read16(dev, B53_CTRL_PAGE, B53_SOFTWARE_LEARNING, &port_ctrl);
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
	ret |= b53125_write16(dev, B53_CTRL_PAGE, B53_SOFTWARE_LEARNING, port_ctrl);
	return ret;
}
/*************************************************************************/
/*************************************************************************/
int b53125_port_set_speed(struct b53125_device *dev, int port, int speed)
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

	ret |= b53125_read8(dev, B53_CTRL_PAGE, off, &reg);
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
	ret |= b53125_write8(dev, B53_CTRL_PAGE, off, reg);
	return ret;
}

int b53125_port_set_duplex(struct b53125_device *dev, int port, int duplex)
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

	ret |= b53125_read8(dev, B53_CTRL_PAGE, off, &reg);
	reg &= ~(GMII_PO_FULL_DUPLEX);

	if (duplex == DUPLEX_FULL)
		reg |= PORT_OVERRIDE_FULL_DUPLEX;
	else
		reg &= ~PORT_OVERRIDE_FULL_DUPLEX;
	reg |= val;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, off, reg);
	return ret;
}

int b53125_port_set_flow(struct b53125_device *dev, int port, int flow)
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

	ret |= b53125_read8(dev, B53_CTRL_PAGE, off, &reg);
	reg &= ~(PORT_OVERRIDE_RX_FLOW|PORT_OVERRIDE_TX_FLOW);

	if (flow & MLO_PAUSE_RX)
		reg |= PORT_OVERRIDE_RX_FLOW;
	if (flow & MLO_PAUSE_TX)
		reg |= PORT_OVERRIDE_TX_FLOW;

	reg |= val;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, off, reg);
	return ret;
}

int b53125_port_set_link_force(struct b53125_device *dev, int port, int link)
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
	ret |= b53125_read8(dev, B53_CTRL_PAGE, off, &reg);
	reg |= val;
	if (link)
		reg |= PORT_OVERRIDE_LINK;
	else
		reg &= ~PORT_OVERRIDE_LINK;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, off, reg);
	return ret;
}
/*************************************************************************/

/*************************************************************************/

//获取端口状态
BOOL b53125_port_get_link(struct b53125_device *dev, int port)
{
	BOOL link;
	u16 sts;

	b53125_read16(dev, B53_STAT_PAGE, B53_LINK_STAT, &sts);
	link = !!(sts & BIT(port));
	//dsa_port_phylink_mac_change(ds, port, link);
	return link;
}

u_int b53125_port_get_speed(struct b53125_device *dev, int port)
{
	u32 sts;

	b53125_read32(dev, B53_STAT_PAGE, B53_SPEED_STAT, &sts);
	//dsa_port_phylink_mac_change(ds, port, link);
	return SPEED_PORT_GE(sts, port);
}

u_int b53125_port_get_duplex(struct b53125_device *dev, int port)
{
	u16 sts;

	b53125_read16(dev, B53_STAT_PAGE, B53_DUPLEX_STAT_GE, &sts);
	//dsa_port_phylink_mac_change(ds, port, link);
	return (sts & BIT(port))>>(port);
}




