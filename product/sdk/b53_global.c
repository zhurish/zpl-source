/*
 * b53_global.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */

#include <zpl_include.h>

#include "hal_driver.h"

#include "b53_mdio.h"
#include "b53_regs.h"
#include "sdk_driver.h"


void b53_brcm_hdr_setup(struct b53125_device *dev, zpl_bool enable, int port)
{
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

	/* Enable Broadcom tags for IMP port */
	b53125_read8(dev, B53_MGMT_PAGE, B53_BRCM_HDR, &hdr_ctl);
	if (enable)
		hdr_ctl |= val;
	else
		hdr_ctl &= ~val;
	b53125_write8(dev, B53_MGMT_PAGE, B53_BRCM_HDR, hdr_ctl);

	/* Enable reception Broadcom tag for CPU TX (switch RX) to
	 * allow us to tag outgoing frames
	 */
	b53125_read16(dev, B53_MGMT_PAGE, B53_BRCM_HDR_RX_DIS, &reg);
	if (enable)
		reg &= ~BIT(port);
	else
		reg |= BIT(port);
	b53125_write16(dev, B53_MGMT_PAGE, B53_BRCM_HDR_RX_DIS, reg);

	/* Enable transmission of Broadcom tags from the switch (CPU RX) to
	 * allow delivering frames to the per-port net_devices
	 */
	b53125_read16(dev, B53_MGMT_PAGE, B53_BRCM_HDR_TX_DIS, &reg);
	if (enable)
		reg &= ~BIT(port);
	else
		reg |= BIT(port);
	b53125_write16(dev, B53_MGMT_PAGE, B53_BRCM_HDR_TX_DIS, reg);
}

//设置管理非管理功能
int b53125_switch_manege(struct b53125_device *dev, zpl_bool manege)
{
	int ret = 0;
	u8 mgmt;
	ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_SWITCH_MODE, &mgmt);
	if (manege)
		mgmt |= SM_SW_FWD_MODE;
	else
		mgmt &= ~SM_SW_FWD_MODE;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_SWITCH_MODE, mgmt);
	if(ret == ERROR)
		return ERROR;
	return OK;
}

//禁止使能交换转发功能
int b53125_switch_forwarding(struct b53125_device *dev, zpl_bool enable)
{
	int ret = 0;
	u8 mgmt;
	ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_SWITCH_MODE, &mgmt);
	if (enable)
		mgmt |= SM_SW_FWD_EN;
	else
		mgmt &= ~SM_SW_FWD_EN;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_SWITCH_MODE, mgmt);
	if(ret == ERROR)
		return ERROR;
	/* Include IMP port in dumb forwarding mode
	 */
	b53125_read8(dev, B53_CTRL_PAGE, B53_SWITCH_CTRL, &mgmt);
	mgmt |= B53_MII_DUMB_FWDG_EN;
	b53125_write8(dev, B53_CTRL_PAGE, B53_SWITCH_CTRL, mgmt);

	return OK;
}

/*************************************************************************/
//禁止使能多播泛洪
int b53125_multicast_flood(struct b53125_device *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
	reg &= ~ (B53_MC_FWD_EN);
	reg |= enable ? 0:B53_MC_FWD_EN;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	return ret;
}
//禁止使能单播泛洪
int b53125_unicast_flood(struct b53125_device *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
	reg &= ~ (B53_UC_FWD_EN);
	reg |= enable ? 0:B53_UC_FWD_EN;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	return ret;
}

int b53125_range_error(struct b53125_device *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
	reg &= ~ (B53_INRANGE_ERR_DIS|B53_OUTRANGE_ERR_DIS);
	reg |= enable ? B53_INRANGE_ERR_DIS|B53_OUTRANGE_ERR_DIS:0;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	return ret;
}

/*************************************************************************/
//禁止使能学习多播报文源MAC地址
int b53125_multicast_learning(struct b53125_device *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, &reg);
	reg &= ~ (B53_MC_LEARN_EN);
	reg |= enable ? B53_MC_LEARN_EN:0;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, reg);
	return ret;
}



/* Pause Frame Detection Control Register (8 bit) */
int b53125_puase_frame_detection(struct b53125_device *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev, B53_CTRL_PAGE, B53_PAUSE_FRAME_DETECTION, &reg);
	reg &= ~ (PAUSE_IGNORE_DA);
	reg |= enable ? PAUSE_IGNORE_DA:0;
	ret |= b53125_write8(dev, B53_CTRL_PAGE, B53_PAUSE_FRAME_DETECTION, reg);
	return ret;
}

/*************************************************************************/
//禁止使能BPDU报文进入CPU
int b53125_enable_bpdu(struct b53125_device *dev, zpl_bool enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, &port_ctrl);
	if(enable)
		port_ctrl |= GC_RX_BPDU_EN;
	else
		port_ctrl &= ~GC_RX_BPDU_EN;

	ret |= b53125_write8(dev, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, port_ctrl);
	return ret;
}
/*************************************************************************/
//设置MAC地址表有效期
int b53125_aging_time(struct b53125_device *dev, int agetime)
{
	int ret = 0;
	u32 port_ctrl = 0;
	port_ctrl = BRCM_AGE_CHANGE_EN | agetime;
	ret |= b53125_write32(dev, B53_MGMT_PAGE, B53_AGING_TIME, port_ctrl);
	return ret;
}

//设置IMP接口模式（在managed mode模式下有效）
int b53125_imp_port_enable(struct b53125_device *dev)
{
	int ret = 0;
	u8 mgmt;
	/* Include IMP port in dumb forwarding mode
	 */
	ret |= b53125_read8(dev, B53_MGMT_PAGE, B53_MGMT_CTRL, &mgmt);
	mgmt |= B53_IMP_EN;
	ret |= b53125_write8(dev, B53_MGMT_PAGE, B53_MGMT_CTRL, mgmt);
	if(ret == ERROR)
		return ERROR;
	return OK;
}

/*
#define B53_MGMT_CTRL			0x00
#define  B53_IMP_EN				(2<<6)
#define  B53_DOUBLE_IMP_EN		(3<<6)
#define  B53_IMP_DIS			(0<<6)
*/

