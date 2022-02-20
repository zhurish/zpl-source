/*
 * b53_global.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */

#include <zpl_include.h>

#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"


void b53_brcm_hdr_setup(sdk_driver_t *dev, zpl_bool enable, zpl_phyport_t port)
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
	/* Enable management mode if tagging is requested */
	b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, &hdr_ctl);
	if (enable)
		hdr_ctl |= SM_SW_FWD_MODE;
	else
		hdr_ctl &= ~SM_SW_FWD_MODE;
	b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, hdr_ctl);

	/* Configure the appropriate IMP port */
	b53125_read8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, &hdr_ctl);
	if (port == 8)
		hdr_ctl |= GC_FRM_MGMT_PORT_MII;
	else if (port == 5)
		hdr_ctl |= GC_FRM_MGMT_PORT_M;
	b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, hdr_ctl);

	/* Enable Broadcom tags for IMP port */
	b53125_read8(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR, &hdr_ctl);
	if (enable)
		hdr_ctl |= val;
	else
		hdr_ctl &= ~val;
	b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR, hdr_ctl);

	/* Enable reception Broadcom tag for CPU TX (switch RX) to
	 * allow us to tag outgoing frames
	 */
	b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR_RX_DIS, &reg);
	if (enable)
		reg &= ~BIT(port);
	else
		reg |= BIT(port);
	b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR_RX_DIS, reg);

	/* Enable transmission of Broadcom tags from the switch (CPU RX) to
	 * allow delivering frames to the per-port net_devices
	 */
	b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR_TX_DIS, &reg);
	if (enable)
		reg &= ~BIT(port);
	else
		reg |= BIT(port);
	b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR_TX_DIS, reg);
}

//设置管理非管理功能
int b53125_switch_manege(sdk_driver_t *dev, zpl_bool manege)
{
	int ret = 0;
	u8 mgmt;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, &mgmt);
	if (manege)
		mgmt |= SM_SW_FWD_MODE;
	else
		mgmt &= ~SM_SW_FWD_MODE;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, mgmt);
	if(ret == ERROR)
		return ERROR;
	return OK;
}

//禁止使能交换转发功能
int b53125_switch_forwarding(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 mgmt;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, &mgmt);
	if (enable)
		mgmt |= SM_SW_FWD_EN;
	else
		mgmt &= ~SM_SW_FWD_EN;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, mgmt);
	if(ret == ERROR)
		return ERROR;
	/* Include IMP port in dumb forwarding mode
	 */
	b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_CTRL, &mgmt);
	mgmt |= B53_MII_DUMB_FWDG_EN;
	b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_CTRL, mgmt);

	return OK;
}

/*************************************************************************/
//禁止使能多播泛洪
int b53125_multicast_flood(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
	reg &= ~ (B53_MC_FWD_EN);
	reg |= enable ? 0:B53_MC_FWD_EN;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	return ret;
}
//禁止使能单播泛洪
int b53125_unicast_flood(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
	reg &= ~ (B53_UC_FWD_EN);
	reg |= enable ? 0:B53_UC_FWD_EN;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	return ret;
}

int b53125_range_error(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
	reg &= ~ (B53_INRANGE_ERR_DIS|B53_OUTRANGE_ERR_DIS);
	reg |= enable ? B53_INRANGE_ERR_DIS|B53_OUTRANGE_ERR_DIS:0;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	return ret;
}

/*************************************************************************/
//禁止使能学习多播报文源MAC地址
int b53125_multicast_learning(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, &reg);
	reg &= ~ (B53_MC_LEARN_EN);
	reg |= enable ? B53_MC_LEARN_EN:0;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, reg);
	return ret;
}





/*************************************************************************/
//禁止使能BPDU报文进入CPU
int b53125_enable_bpdu(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, &port_ctrl);
	if(enable)
		port_ctrl |= GC_RX_BPDU_EN;
	else
		port_ctrl &= ~GC_RX_BPDU_EN;

	ret |= b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, port_ctrl);
	return ret;
}
/*************************************************************************/
//设置MAC地址表有效期
int b53125_aging_time(sdk_driver_t *dev, int agetime)
{
	int ret = 0;
	u32 port_ctrl = 0;
	port_ctrl = BRCM_AGE_CHANGE_EN | agetime;
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_AGING_TIME, port_ctrl);
	return ret;
}


//禁止使能wan接口（设置某个接口作为wan接口使用）
int b53125_port_wan_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
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
	return ret;
}


int b53125_multicast_forward(sdk_driver_t *dev, u8 *mac, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, &reg);
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
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, reg);
	return ret;
}


int b53125_mldqry_tocpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl |= B53_MLD_QRY_FWD_MODE;
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_mldqry_cpoycpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl &= ~(B53_MLD_QRY_FWD_MODE);
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_mldqry_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_MLD_QRY_EN;
	}
	else
	{
		port_ctrl &= ~(B53_MLD_QRY_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_mld_tocpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl |= B53_MLD_RPTDONE_FWD_MODE;
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_mld_cpoycpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl &= ~(B53_MLD_RPTDONE_FWD_MODE);
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_mld_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_MLD_RPTDONE_EN;
	}
	else
	{
		port_ctrl &= ~(B53_MLD_RPTDONE_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_igmpunknow_tocpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl |= B53_IGMP_UKN_FWD_MODE;
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_igmpunknow_cpoycpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl &= ~(B53_IGMP_UKN_FWD_MODE);
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_igmpunknow_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_IGMP_UKN_EN;
	}
	else
	{
		port_ctrl &= ~(B53_IGMP_UKN_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_igmpqry_tocpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl |= B53_IGMP_QRY_FWD_MODE;
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_igmpqry_cpoycpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl &= ~(B53_IGMP_QRY_FWD_MODE);
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_igmpqry_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_IGMP_QRY_EN;
	}
	else
	{
		port_ctrl &= ~(B53_IGMP_QRY_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}

int b53125_igmp_tocpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl |= B53_IGMP_RPTLVE_FWD_MODE;
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_igmp_cpoycpu_enable(sdk_driver_t *dev)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	port_ctrl &= ~(B53_IGMP_RPTLVE_FWD_MODE);
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_igmp_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_IGMP_RPTLVE_EN;
	}
	else
	{
		port_ctrl &= ~(B53_IGMP_RPTLVE_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_igmp_ipcheck_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_IGMP_DIP_EN;
	}
	else
	{
		port_ctrl &= ~(B53_IGMP_DIP_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}

int b53125_arp_copycpu_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_ARP_EN;
	}
	else
	{
		port_ctrl &= ~(B53_ARP_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_rarp_copycpu_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_RARP_EN;
	}
	else
	{
		port_ctrl &= ~(B53_RARP_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}
int b53125_dhcp_copycpu_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_MGMT_PAGE, B53_HIGH_LEVEL_CTL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_DHCP_EN;
	}
	else
	{
		port_ctrl &= ~(B53_DHCP_EN);
	}
	ret |= b53125_write32(dev->sdk_device, B53_EAP_PAGE, B53_HIGH_LEVEL_CTL, port_ctrl);
	return ret;
}