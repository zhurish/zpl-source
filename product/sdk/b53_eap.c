/*
 * b53_eap.c
 *
 *  Created on: 2019年9月12日
 *      Author: DELL
 */


#include <zebra.h>
#include "nsm_dos.h"
#include "hal_dos.h"

#include "b53_mdio.h"
#include "b53_regs.h"
#include "sdk_driver.h"



//8021X
int b53125_eap_enable(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev, B53_EAP_PAGE, B53_EAP_GLOBAL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_EAP_RARP_EN|B53_EAP_BPDU_EN|B53_EAP_RMC_EN|
				B53_EAP_DHCP_EN|B53_EAP_ARP_EN|B53_EAP_2DIP_EN;
	}
	else
	{
		port_ctrl = 0;
	}
	ret |= b53125_write8(dev, B53_EAP_PAGE, B53_EAP_GLOBAL, port_ctrl);
	return ret;
}

int b53125_eap_ip_address_bypass_enable(struct b53125_device *dev, BOOL enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev, B53_EAP_PAGE, B53_EAP_GLOBAL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_EAP_2DIP_EN;
	}
	else
	{
		port_ctrl &= ~B53_EAP_2DIP_EN;
	}
	ret |= b53125_write8(dev, B53_EAP_PAGE, B53_EAP_GLOBAL, port_ctrl);
	return ret;
}

int b53125_eap_mult_address_bypass_enable(struct b53125_device *dev, BOOL enable, u32 address)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev, B53_EAP_PAGE, B53_EAP_MULT_ADDR, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_EAP_MPORT(address);
	}
	else
	{
		port_ctrl &= ~B53_EAP_MPORT(address);
	}
	ret |= b53125_write8(dev, B53_EAP_PAGE, B53_EAP_MULT_ADDR, port_ctrl);
	return ret;
}


int b53125_eap_ip_address_set(struct b53125_device *dev, int index, u32 address, u32 mask)
{
	int ret = 0;
	u64 port_ctrl = address << 32 | mask;
	//ret |= b53125_read64(dev, B53_EAP_PAGE, index ? B53_EAP_DST_IP_ADDR1:B53_EAP_DST_IP_ADDR0, &port_ctrl);

	//port_ctrl |= B53_EAP_MPORT(address);

	ret |= b53125_write64(dev, B53_EAP_PAGE, index ? B53_EAP_DST_IP_ADDR1:B53_EAP_DST_IP_ADDR0, port_ctrl);
	return ret;
}

int b53125_eap_mode_set(struct b53125_device *dev, int port, u32 mode)
{
	int ret = 0;
	u64 port_ctrl = 0;
	ret |= b53125_read64(dev, B53_EAP_PAGE, B53_EAP_PORT(port), &port_ctrl);
	port_ctrl &= ~(B53_EAP_MODE_MASK << B53_EAP_MODE);
	port_ctrl |= ((B53_EAP_MODE_MASK & mode) << B53_EAP_MODE);
	ret |= b53125_write64(dev, B53_EAP_PAGE, B53_EAP_PORT(port), port_ctrl);
	return ret;
}

int b53125_eap_blk_mode_set(struct b53125_device *dev, int port, u32 mode)
{
	int ret = 0;
	u64 port_ctrl = 0;
	ret |= b53125_read64(dev, B53_EAP_PAGE, B53_EAP_PORT(port), &port_ctrl);
	port_ctrl &= ~(B53_EAP_BLK_MODE_M << B53_EAP_BLK_MODE);
	port_ctrl |= ((B53_EAP_BLK_MODE_M & mode) << B53_EAP_BLK_MODE) | B53_EAP_DA_EN;
	ret |= b53125_write64(dev, B53_EAP_PAGE, B53_EAP_PORT(port), port_ctrl);
	return ret;
}

//EEE
/*static int sdk_eee_enable(ifindex_t ifindex, BOOL enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_EEE_ENABLE;
	data.module = SDK_MODULE_EEE;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? TRUE:FALSE;
	return sdk_cmd_ioctl(&data);
}

static int sdk_eee_value(ifindex_t ifindex, int value)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_EEE_SET;
	data.module = SDK_MODULE_EEE;
	data.port = if_ifindex2phy(ifindex);
	data.value = value;
	return sdk_cmd_ioctl(&data);
}*/
