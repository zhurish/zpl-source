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


//设置管理非管理功能
static int b53125_switch_manege(sdk_driver_t *dev, zpl_bool manege)
{
	int ret = 0;
	u8 mgmt;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, &mgmt);
	if (manege)
		mgmt |= SM_SW_FWD_MODE;
	else
		mgmt &= ~SM_SW_FWD_MODE;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_SWITCH_MODE, mgmt);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	if(ret == ERROR)
		return ERROR;
	return OK;
}

//禁止使能交换转发功能
static int b53125_switch_forwarding(sdk_driver_t *dev, zpl_bool enable)
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
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return OK;
}

//禁止使能BPDU报文进入CPU
static int b53125_enable_bpdu(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, &port_ctrl);
	if(enable)
		port_ctrl |= GC_RX_BPDU_EN;
	else
		port_ctrl &= ~GC_RX_BPDU_EN;

	ret |= b53125_write8(dev->sdk_device, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/*************************************************************************/
//设置MAC地址表有效期
static int b53125_aging_time(sdk_driver_t *dev, int agetime)
{
	int ret = 0;
	u32 port_ctrl = 0;
	port_ctrl = BRCM_AGE_CHANGE_EN | agetime;
	ret |= b53125_write32(dev->sdk_device, B53_MGMT_PAGE, B53_AGING_TIME, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
//禁止使能多播泛洪
static int b53125_multicast_flood(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
	reg &= ~ (B53_MC_FWD_EN);
	reg |= enable ? 0:B53_MC_FWD_EN;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
//禁止使能单播泛洪
static int b53125_unicast_flood(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, &reg);
	reg &= ~ (B53_UC_FWD_EN);
	reg |= enable ? 0:B53_UC_FWD_EN;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_IP_MULTICAST_CTRL, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
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
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/
//禁止使能学习多播报文源MAC地址
static int b53125_multicast_learning(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 reg = 0;
	ret |= b53125_read8(dev->sdk_device, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, &reg);
	reg &= ~ (B53_MC_LEARN_EN);
	reg |= enable ? B53_MC_LEARN_EN:0;
	ret |= b53125_write8(dev->sdk_device, B53_CTRL_PAGE, B53_MULTICAST_LEARNING, reg);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

/*************************************************************************/

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
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_jumbo_size(sdk_driver_t *dev, int size)
{
	u16 max_size = size;
	int ret = 0;
	ret |= b53125_write16(dev->sdk_device, B53_JUMBO_PAGE, ((b53_device_t*)(dev->sdk_device))->jumbo_size_reg, max_size);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
/* Jumbo Frame Control Register */
static int b53125_jumbo_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u32 port_ctrl = 0;
	ret |= b53125_read32(dev->sdk_device, B53_JUMBO_PAGE, ((b53_device_t*)(dev->sdk_device))->jumbo_pm_reg, &port_ctrl);
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
	ret |= b53125_write32(dev->sdk_device, B53_JUMBO_PAGE, ((b53_device_t*)(dev->sdk_device))->jumbo_pm_reg, port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}



int b53125_global_init(sdk_driver_t *dev)
{
	int ret = 0;
	sdk_maccb.sdk_mac_age_cb = b53125_aging_time;
	sdk_global.sdk_jumbo_size_cb = b53125_jumbo_size;
	sdk_global.sdk_switch_manege_cb = b53125_switch_manege;
	sdk_global.sdk_switch_forward_cb = b53125_switch_forwarding;
	sdk_global.sdk_multicast_flood_cb = b53125_multicast_flood;
	sdk_global.sdk_unicast_flood_cb = b53125_unicast_flood;
	sdk_global.sdk_multicast_learning_cb = b53125_multicast_learning;
	sdk_global.sdk_bpdu_enable_cb = b53125_enable_bpdu;//全局使能接收BPDU报文
	sdk_global.sdk_aging_time_cb = b53125_aging_time;
	sdk_port.sdk_port_jumbo_cb = b53125_jumbo_enable;

	ret |= b53125_switch_manege(dev, zpl_true);//设置为managed mode
	ret |= b53125_switch_forwarding(dev, zpl_false);//禁止转发
	ret |= b53125_multicast_flood(dev, zpl_true);//使能多播泛洪
	ret |= b53125_unicast_flood(dev, zpl_true);//使能单播泛洪
	ret |= b53125_multicast_learning(dev, zpl_true);//使能多播报文学习源MAC
	return ret;
}

int b53125_global_start(sdk_driver_t *dev)
{
	int ret = 0;
	ret |= b53125_switch_forwarding(dev, zpl_true);//禁止转发
	return ret;
}