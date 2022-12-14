/*
 * b53_global.c
 *
 *  Created on: May 3, 2019
 *      Author: zhurish
 */

#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_port.h"

//设置管理非管理功能
static int b53125_switch_software_forwarding_mode(sdk_driver_t *dev, zpl_bool manege)
{
	int ret = 0;
	u8 regval;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_SWITCH_MODE, regval);
	if (manege)
		regval |= SM_SW_FWD_MODE;
	else
		regval &= ~SM_SW_FWD_MODE;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, regval);

	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_SWITCH_MODE, regval);

	sdk_handle_return(ret);
	return ret;
}

//禁止使能交换转发功能
static int b53125_switch_software_forwarding(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 regval = 0;
	//return OK;//mdf
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_SWITCH_MODE, regval);
	if (enable == zpl_false)
		regval |= (SM_SW_FWD_EN);
	else
		regval &= ~SM_SW_FWD_EN;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_SWITCH_MODE, regval);
	sdk_handle_return(ret);
	return ret;
}

//禁止使能BPDU报文进入CPU
static int b53125_enable_bpdu(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 regval = 0;
	ret |= b53125_read8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, regval);
	if(enable)
		regval |= GC_RX_BPDU_EN;
	else
		regval &= ~GC_RX_BPDU_EN;

	ret |= b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, regval);
	sdk_handle_return(ret);
	return ret;
}
/*************************************************************************/
//设置MAC地址表有效期
static int b53125_aging_time(sdk_driver_t *dev, int agetime)
{
	int ret = 0;
	u32 regval = 0;
	regval = BRCM_AGE_CHANGE_EN | agetime;
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_AGING_TIME, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_MGMT_PAGE, B53_AGING_TIME, regval);
	sdk_handle_return(ret);
	return ret;
}

/*************************************************************************/
//禁止使能多播泛洪
static int b53125_unknow_multicast_flood(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	reg &= ~ (B53_MC_FWD_EN);
	reg |= enable ? 0:B53_MC_FWD_EN;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	sdk_handle_return(ret);
	return ret;
}
//禁止使能单播泛洪
static int b53125_unknow_unicast_flood(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	reg &= ~ (B53_UC_FWD_EN);
	reg |= enable ? 0:B53_UC_FWD_EN;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	sdk_handle_return(ret);
	return ret;
}

int b53125_range_error(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	reg &= ~ (B53_INRANGE_ERR_DIS|B53_OUTRANGE_ERR_DIS);
	reg |= enable ? B53_INRANGE_ERR_DIS|B53_OUTRANGE_ERR_DIS:0;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	sdk_handle_return(ret);
	return ret;
}

/*************************************************************************/
//禁止使能学习多播报文源MAC地址
static int b53125_multicast_learning(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, &reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, reg);
	reg &= ~ (B53_MC_LEARN_EN);
	reg |= enable ? B53_MC_LEARN_EN:0;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, reg);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, reg);
	sdk_handle_return(ret);
	return ret;
}

/*************************************************************************/

int b53125_multicast_forward(sdk_driver_t *dev, u8 *mac, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, &reg);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, reg);
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
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, reg);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_jumbo_size(sdk_driver_t *dev, int size)
{
	u16 max_size = size;
	int ret = 0;
	ret |= b53125_write16(dev->sdk_device, B53_JUMBO_PAGE, ((b53_device_t*)(dev->sdk_device))->jumbo_size_reg, max_size);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_JUMBO_PAGE, ((b53_device_t*)(dev->sdk_device))->jumbo_size_reg, max_size);
	sdk_handle_return(ret);
	return ret;
}
/* Jumbo Frame Control Register */
static int b53125_jumbo_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u32 regval = 0;
	ret |= b53125_read32(dev->sdk_device, B53_JUMBO_PAGE, ((b53_device_t*)(dev->sdk_device))->jumbo_pm_reg, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_JUMBO_PAGE, ((b53_device_t*)(dev->sdk_device))->jumbo_pm_reg, regval);
	if(port == dev->cpu_port)
	{
		if(enable)
			regval |= BIT(8);
		else
			regval &= ~BIT(8);
	}
	else
	{
		if(enable)
			regval |= BIT(port);
		else
			regval &= ~BIT(port);
	}
	ret |= b53125_write32(dev->sdk_device, B53_JUMBO_PAGE, ((b53_device_t*)(dev->sdk_device))->jumbo_pm_reg, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_JUMBO_PAGE, ((b53_device_t*)(dev->sdk_device))->jumbo_pm_reg, regval);
	sdk_handle_return(ret);
	return ret;
}


int b53125_reset(sdk_driver_t *dev)
{
	u8 val;
	int ret = 0, timeout = 1000;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SOFTRESET, &val);
	val |= EN_SW_RST|SW_RST|EN_CH_RST;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SOFTRESET, val);
	while(timeout--)
	{
		ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SOFTRESET, &val);
		if (!(val & SW_RST))
			break;
		usleep_range(1000, 2000);
	}
	sdk_handle_return(ret);
	return ret;
}



#if defined( _SDK_CLI_DEBUG_EN)
DEFUN (sdk_manage_mode,
		sdk_manage_mode_cmd,
		"manage (enable|disable)",
		"Manage\n"
		"enable\n"
		"disbale\n")
{
	int ret = 0;
	if(memcmp(argv[0], "enable", 3) == 0)
	{
		ret = b53125_switch_software_forwarding_mode(__msdkdriver, zpl_true);
	}
	else if(memcmp(argv[0], "disable", 3) == 0)
	{
		ret = b53125_switch_software_forwarding_mode(__msdkdriver, zpl_false);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (sdk_sw_forwarding,
		sdk_sw_forwarding_cmd,
		"software forwarding (enable|disable)",
		"software\n"
		"forwarding\n"
		"enable\n"
		"disbale\n")
{
	int ret = 0;
	if(memcmp(argv[0], "enable", 3) == 0)
	{
		ret = b53125_switch_software_forwarding(__msdkdriver, zpl_true);
	}
	else if(memcmp(argv[0], "disable", 3) == 0)
	{
		ret = b53125_switch_software_forwarding(__msdkdriver, zpl_false);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
DEFUN (sdk_reset,
		sdk_reset_cmd,
		"reset",
		"reset\n")
{
	int ret = 0;
	ret = b53125_reset(__msdkdriver);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif

int b53125_global_init(sdk_driver_t *dev)
{
	int ret = 0;
#if defined( _SDK_CLI_DEBUG_EN)	
	install_element(SDK_NODE, CMD_CONFIG_LEVEL, &sdk_manage_mode_cmd);
	install_element(SDK_NODE, CMD_CONFIG_LEVEL, &sdk_sw_forwarding_cmd);
	install_element(SDK_NODE, CMD_CONFIG_LEVEL, &sdk_reset_cmd);
#endif	
	sdk_maccb.sdk_mac_age_cb = b53125_aging_time;
	sdk_global.sdk_jumbo_size_cb = b53125_jumbo_size;
	sdk_global.sdk_switch_manege_cb = b53125_switch_software_forwarding_mode;
	sdk_global.sdk_switch_forward_cb = b53125_switch_software_forwarding;
	sdk_global.sdk_multicast_flood_cb = b53125_unknow_multicast_flood;
	sdk_global.sdk_unicast_flood_cb = b53125_unknow_unicast_flood;
	sdk_global.sdk_multicast_learning_cb = b53125_multicast_learning;
	sdk_global.sdk_bpdu_enable_cb = b53125_enable_bpdu;//全局使能接收BPDU报文
	sdk_global.sdk_aging_time_cb = b53125_aging_time;
	sdk_port.sdk_port_jumbo_cb = b53125_jumbo_enable;

	ret |= b53125_switch_software_forwarding_mode(dev, zpl_true);//设置为managed mode
	ret |= b53125_switch_software_forwarding(dev, zpl_false);//禁止转发
	ret |= b53125_unknow_multicast_flood(dev, zpl_true);//使能多播泛洪
	ret |= b53125_unknow_unicast_flood(dev, zpl_true);//使能单播泛洪
	ret |= b53125_multicast_learning(dev, zpl_true);//使能多播报文学习源MAC 
	
	return ret;
}

int b53125_global_start(sdk_driver_t *dev)
{
	int ret = 0;
	ret |= b53125_switch_software_forwarding(dev, zpl_true);//禁止转发
	return ret;
}


