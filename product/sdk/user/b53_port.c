/*
 * b53_port.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */


#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_port.h"
struct b53_mib_desc {
	u8 size;
	u8 offset;
	const char *name;
};
/* MIB counters */
static const struct b53_mib_desc b53_mibs[] = {
	{ 8, 0x00, "TxOctets" },
	{ 4, 0x08, "TxDropPkts" },
	{ 4, 0x10, "TxBroadcastPkts" },
	{ 4, 0x14, "TxMulticastPkts" },
	{ 4, 0x18, "TxUnicastPkts" },
	{ 4, 0x1c, "TxCollisions" },
	{ 4, 0x20, "TxSingleCollision" },
	{ 4, 0x24, "TxMultipleCollision" },
	{ 4, 0x28, "TxDeferredTransmit" },
	{ 4, 0x2c, "TxLateCollision" },
	{ 4, 0x30, "TxExcessiveCollision" },
	{ 4, 0x38, "TxPausePkts" },
	{ 8, 0x50, "RxOctets" },
	{ 4, 0x58, "RxUndersizePkts" },
	{ 4, 0x5c, "RxPausePkts" },
	{ 4, 0x60, "Pkts64Octets" },
	{ 4, 0x64, "Pkts65to127Octets" },
	{ 4, 0x68, "Pkts128to255Octets" },
	{ 4, 0x6c, "Pkts256to511Octets" },
	{ 4, 0x70, "Pkts512to1023Octets" },
	{ 4, 0x74, "Pkts1024to1522Octets" },
	{ 4, 0x78, "RxOversizePkts" },
	{ 4, 0x7c, "RxJabbers" },
	{ 4, 0x80, "RxAlignmentErrors" },
	{ 4, 0x84, "RxFCSErrors" },
	{ 8, 0x88, "RxGoodOctets" },
	{ 4, 0x90, "RxDropPkts" },
	{ 4, 0x94, "RxUnicastPkts" },
	{ 4, 0x98, "RxMulticastPkts" },
	{ 4, 0x9c, "RxBroadcastPkts" },
	{ 4, 0xa0, "RxSAChanges" },
	{ 4, 0xa4, "RxFragments" },
	{ 4, 0xa8, "RxJumboPkts" },
	{ 4, 0xac, "RxSymbolErrors" },
	{ 4, 0xc0, "RxDiscarded" },
};

#define B53_MIBS_SIZE	sizeof(b53_mibs)/sizeof(b53_mibs[0])
/*************************************************************************/
/*
static int b53125_port_reset_mib(sdk_driver_t *dev)
{
	u8 gc;
	b53125_read8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, &gc);
	b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, gc | GC_RESET_MIB);
	os_msleep(1);
	b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, gc & ~GC_RESET_MIB);
	os_msleep(1);
	return 0;
}
*/
static int b53125_port_get_stats(sdk_driver_t *dev, zpl_phyport_t port, struct b53_mib_stats *state)
{
	const struct b53_mib_desc *s;
	unsigned int i;
	u64 val = 0;
	u64 *val64 = (u64 *)state;
	u32 *val32 = (u32 *)state;
	u8 *val8 = (u8 *)state;

	for (i = 0; i < B53_MIBS_SIZE; i++) {
		s = &b53_mibs[i];

		if (s->size == 8) {
			b53125_read64(dev->sdk_device, B53_MIB_PAGE(port), s->offset, &val);
			val64 = (u64*)val8;
			*val64 = (u64)val;
			val8 += sizeof(u64);
		} else {
			u32 val32tmp;
			b53125_read32(dev->sdk_device, B53_MIB_PAGE(port), s->offset, &val32tmp);
			val32 = (u32*)val8;
			*val32 = (u32)val32tmp;
			val8 += sizeof(u32);
		}
	}
	return 0;
}
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
		_sdk_debug( "======%s page=0x%x reg=0x%x val=0x%x", __func__, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);	
		ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_CTRL(port), &reg);
		reg &= ~(PORT_CTRL_RX_DISABLE | PORT_CTRL_TX_DISABLE);
		ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_CTRL(port), reg);
		_sdk_debug( "======%s page=0x%x reg=0x%x val=0x%x", __func__, B53_CTRL_PAGE, B53_PORT_CTRL(port), reg);
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
		b53125_mac_address_clr(dev, port, 0, 0);
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
	_sdk_debug( "======%s page=0x%x reg=0x%x val=0x%x", __func__, B53_CTRL_PAGE, B53_UC_FLOOD_MASK, reg);
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
	_sdk_debug( "======%s page=0x%x reg=0x%x val=0x%x", __func__, B53_CTRL_PAGE, B53_MC_FLOOD_MASK, reg);
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
	_sdk_debug( "======%s page=0x%x reg=0x%x val=0x%x", __func__, B53_CTRL_PAGE, B53_IPMC_FLOOD_MASK, reg);
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
	_sdk_debug( "======%s page=0x%x reg=0x%x val=0x%x", __func__, B53_CTRL_PAGE, B53_DIS_LEARNING, port_ctrl);
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
	_sdk_debug( "======%s page=0x%x reg=0x%x val=0x%x", __func__, B53_CTRL_PAGE, B53_SOFTWARE_LEARNING, port_ctrl);
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




static int b53125_port_stats(sdk_driver_t *dev, zpl_phyport_t port, struct if_stats *state)
{
	struct b53_mib_stats bspstate;
	b53125_port_get_stats(dev, port, &bspstate);

	state->rx_packets = bspstate.RxOctets;									   /* total packets received       */
	state->tx_packets = bspstate.TxUnicastPkts;								   /* total packets transmitted    */
	state->rx_bytes = bspstate.RxOctets;									   /* total bytes received         */
	state->tx_bytes = bspstate.TxOctets;									   /* total bytes transmitted      */
	state->rx_errors = bspstate.RxOctets - bspstate.RxGoodOctets;			   /* bad packets received         */
	state->tx_errors = bspstate.TxDeferredTransmit;							   /* packet transmit problems     */
	state->rx_dropped = bspstate.RxDropPkts;								   /* no space in linux buffers    */
	state->tx_dropped = bspstate.TxDropPkts;								   /* no space available in linux  */
	state->rx_multicast = bspstate.RxMulticastPkts + bspstate.RxBroadcastPkts; /* multicast packets received   */
	state->collisions = bspstate.TxCollisions;

	/* detailed rx_errors: */
	state->rx_length_errors = bspstate.RxAlignmentErrors;
	state->rx_over_errors = 0;						   /* receiver ring buff overflow  */
	state->rx_crc_errors = 0;						   /* recved pkt with crc error    */
	state->rx_frame_errors = 0;						   /* recv'd frame alignment error */
	state->rx_fifo_errors = 0;						   /* recv'r fifo overrun          */
	state->rx_missed_errors = bspstate.RxSymbolErrors; /* receiver missed packet     */
	/* detailed tx_errors */
	state->tx_aborted_errors = 0;
	state->tx_carrier_errors = 0;
	state->tx_fifo_errors = 0;
	state->tx_heartbeat_errors = 0;
	state->tx_window_errors = 0;
	/* for cslip etc */
	state->rx_compressed = 0;
	state->tx_compressed = 0;
	return 0;
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

	//风暴
	sdk_port.sdk_port_storm_rate_cb = b53125_strom_rate;

	sdk_port.sdk_port_stat_cb = b53125_port_stats;

	//sdk_port.sdk_port_wan_enable_cb = b53125_port_wan_enable;
	//sdk_port.sdk_port_mac_cb)(void *, zpl_phyport_t, zpl_uint8 *, zpl_bool);
	//sdk_port.sdk_port_mtu_cb)(void *, zpl_phyport_t, zpl_uint32);
	//sdk_port.sdk_port_vrf_cb)(void *, zpl_phyport_t, zpl_uint32);
	//sdk_port.sdk_port_mode_cb)(void *, zpl_phyport_t, zpl_uint32);
	//ret |= b53125_enable_learning(dev, ((b53_device_t *)dev->sdk_device)->cpu_port, zpl_true);
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