/*
 * b53_port.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_cpu.h"
#include "b53_port.h"

/**
[    1.663202] ==b53dev==:B53_CTRL_PAGE, B53_SWITCH_MODE val: 0x06                                                                        
[    1.663593] ==b53dev==:B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL val: 0x0a                                                                 
[    1.664090] ==b53dev==:B53_MGMT_PAGE, B53_GLOBAL_CONFIG val: 0x00 
 */
static int b53_brcm_hdr_setup(sdk_driver_t *dev, zpl_bool enable, zpl_phyport_t port)
{
	int ret = 0;
	u8 hdr_ctl = 0, val = 0;
	u16 reg = 0;

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
	hdr_ctl = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, &hdr_ctl);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_SWITCH_MODE, hdr_ctl);
	if (enable)
		hdr_ctl |= SM_SW_FWD_MODE;
	else
		hdr_ctl &= ~SM_SW_FWD_MODE;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, hdr_ctl);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_SWITCH_MODE, hdr_ctl);
	/* Configure the appropriate IMP port */
	hdr_ctl = 0;
	ret |= b53125_read8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, &hdr_ctl);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, hdr_ctl);
	if (port == 8)
		hdr_ctl |= B53_IMP_EN;
	else if (port == 5)
		hdr_ctl |= B53_DOUBLE_IMP_EN;
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, hdr_ctl);	
	ret |= b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, hdr_ctl);
	
	hdr_ctl = 0;
	/* Enable Broadcom tags for IMP port */
	ret |= b53125_read8(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR, &hdr_ctl);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_BRCM_HDR, hdr_ctl);
	if (enable)
		hdr_ctl |= val;
	else
		hdr_ctl &= ~val;
	ret |= b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR, hdr_ctl);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_BRCM_HDR, hdr_ctl);

	//return ret;
	/* Enable reception Broadcom tag for CPU TX (switch RX) to
	 * allow us to tag outgoing frames
	 */
	reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR_RX_DIS, &reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_BRCM_HDR_RX_DIS, reg);
	if (enable)
		reg &= ~BIT(port);
	else
		reg |= BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR_RX_DIS, reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_BRCM_HDR_RX_DIS, reg);
	/* Enable transmission of Broadcom tags from the switch (CPU RX) to
	 * allow delivering frames to the per-port net_devices
	 */
	reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR_TX_DIS, &reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_BRCM_HDR_TX_DIS, reg);
	if (enable)
		reg &= ~BIT(port);
	else
		reg |= BIT(port);
	ret |= b53125_write16(dev->sdk_device, B53_MGMT_PAGE, B53_BRCM_HDR_TX_DIS, reg);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_BRCM_HDR_TX_DIS, reg);
	sdk_handle_return(ret);
	return ret;
}

//设置IMP接口模式（在managed mode模式下有效）
int b53125_imp_port_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 regval = 0;
	/* Include IMP port in dumb forwarding mode
	 */
	ret |= b53125_read8(dev->sdk_device, B53_MGMT_PAGE, B53_MGMT_CTRL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_MGMT_CTRL, regval);
	if(enable)
		regval |= B53_IMP_EN;
	else
		regval &= ~B53_IMP_EN;	
	ret |= b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_MGMT_CTRL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_MGMT_CTRL, regval);
	regval = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_SWITCH_MODE, regval);
	if(enable)
		regval |= SM_SW_FWD_MODE;
	else
		regval &= ~SM_SW_FWD_MODE;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_SWITCH_MODE, regval);

	sdk_handle_return(ret);
	return ret;
}

//使能禁止IMP接口收发功能
static int b53125_imp_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 regval = 0;
	if(enable)
		regval = PORT_CTRL_RX_BCST_EN |
		    PORT_CTRL_RX_MCST_EN |
		    PORT_CTRL_RX_UCST_EN;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_IMP_CTRL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_IMP_CTRL, regval);
	sdk_handle_return(ret);
	return ret;
}

int b53125_imp_mii_overwrite(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, &reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	if(enable)
		reg |= PORT_OVERRIDE_EN;
	else
		reg &= ~(PORT_OVERRIDE_EN);
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_imp_speed(sdk_driver_t *dev, int speed)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, &reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
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
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_imp_duplex(sdk_driver_t *dev, int duplex)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, &reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	reg &= ~ (PORT_OVERRIDE_FULL_DUPLEX);
	reg |= duplex;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_imp_flow(sdk_driver_t *dev, zpl_bool rx, zpl_bool tx)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, &reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	reg &= ~ (PORT_OVERRIDE_RX_FLOW|PORT_OVERRIDE_TX_FLOW);
	reg |= rx ? PORT_OVERRIDE_RX_FLOW:0;
	reg |= tx ? PORT_OVERRIDE_TX_FLOW:0;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_PORT_OVERRIDE_CTRL, reg);
	sdk_handle_return(ret);
	return ret;
}

//禁止使能wan接口（设置某个接口作为wan接口使用）
static int b53125_port_wan_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u16 reg = 0;
	ret |= b53125_read16(dev->sdk_device, B53_CTRL_PAGE, B53_WAN_CTRL, &reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_WAN_CTRL, reg);
	{
		if(enable)
			reg |= BIT(port);
		else
			reg &= ~BIT(port);
	}
	ret |= b53125_write16(dev->sdk_device, B53_CTRL_PAGE, B53_WAN_CTRL, reg);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_WAN_CTRL, reg);
	sdk_handle_return(ret);
	return ret;
}

#if defined( _SDK_CLI_DEBUG_EN)
DEFUN (sdk_imp_port_enable,
		sdk_imp_port_enable_cmd,
		"imp-port (enable|disable)",
		"imp-port\n"
		"enable\n"
		"disbale\n")
{
	int ret = 0;
	if(memcmp(argv[0], "enable", 3) == 0)
	{
		ret = b53125_imp_port_enable(__msdkdriver, zpl_true);
	}
	else if(memcmp(argv[0], "disable", 3) == 0)
	{
		ret = b53125_imp_port_enable(__msdkdriver, zpl_false);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
DEFUN (sdk_imp_mii_enable,
		sdk_imp_mii_enable_cmd,
		"imp-mii (enable|disable)",
		"imp-port\n"
		"enable\n"
		"disbale\n")
{
	int ret = 0;
	if(memcmp(argv[0], "enable", 3) == 0)
	{
		ret = b53125_imp_mii_overwrite(__msdkdriver, zpl_true);
	}
	else if(memcmp(argv[0], "disable", 3) == 0)
	{
		ret = b53125_imp_mii_overwrite(__msdkdriver, zpl_false);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif


int b53125_imp_init(sdk_driver_t *dev)
{
	int ret = 0;
	ret |= b53_brcm_hdr_setup(dev, zpl_true, ((b53_device_t *)dev->sdk_device)->cpu_port);
	sdk_debug_event(dev, "b53125 brcm hdr init %s", (ret == OK)?"OK":"ERROR");
	return ret;
	ret |= b53125_imp_enable(dev, zpl_true);//关闭IMP接口
	sdk_debug_event(dev, "b53125 imp init %s", (ret == OK)?"OK":"ERROR");
	ret |= b53125_imp_flow(dev, zpl_true, zpl_true);
	sdk_debug_event(dev, "b53125 imp flow init %s", (ret == OK)?"OK":"ERROR");
	ret |= b53125_imp_duplex(dev, PORT_OVERRIDE_FULL_DUPLEX);
	sdk_debug_event(dev, "b53125 imp duplex init %s", (ret == OK)?"OK":"ERROR");
	ret |= b53125_imp_speed(dev, PORT_OVERRIDE_SPEED_1000M);
	sdk_debug_event(dev, "b53125 imp speed init %s", (ret == OK)?"OK":"ERROR");

	return ret;
}

int b53125_cpu_init(sdk_driver_t *dev)
{
#if defined( _SDK_CLI_DEBUG_EN)	
	install_element(SDK_NODE, CMD_CONFIG_LEVEL, &sdk_imp_port_enable_cmd);
	install_element(SDK_NODE, CMD_CONFIG_LEVEL, &sdk_imp_mii_enable_cmd);
#endif	
	sdk_global.sdk_wan_port_cb = b53125_port_wan_enable;
	return 0;
}