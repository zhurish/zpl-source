/*
 * b53_eap.c
 *
 *  Created on: 2019年9月12日
 *      Author: DELL
 */


#include <zpl_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"



//8021X
static int b53125_eap_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 port_ctrl = 0;
	ret |= b53125_read8(dev->sdk_device, B53_EAP_PAGE, B53_EAP_GLOBAL, &port_ctrl);
	if(enable)
	{
		port_ctrl |= B53_EAP_RARP_EN|B53_EAP_BPDU_EN|B53_EAP_RMC_EN|
				B53_EAP_DHCP_EN|B53_EAP_ARP_EN|B53_EAP_2DIP_EN;
	}
	else
	{
		port_ctrl = 0;
	}
	ret |= b53125_write8(dev->sdk_device, B53_EAP_PAGE, B53_EAP_GLOBAL, port_ctrl);
	return ret;
}



int b53125_eap_bypass_ipaddr_set(sdk_driver_t *dev, zpl_uint32 index, u32 address, u32 mask)
{
	int ret = 0;
	u64 port_ctrl = address << 32 | mask;
	ret |= b53125_read64(dev->sdk_device, B53_EAP_PAGE, index ? B53_EAP_DST_IP_ADDR1:B53_EAP_DST_IP_ADDR0, &port_ctrl);
	port_ctrl |= B53_EAP_MPORT(address);
	ret |= b53125_write64(dev->sdk_device, B53_EAP_PAGE, index ? B53_EAP_DST_IP_ADDR1:B53_EAP_DST_IP_ADDR0, port_ctrl);
	return ret;
}

static int b53125_eap_mode_set(sdk_driver_t *dev, zpl_phyport_t port)
{
	int ret = 0;
	u64 port_ctrl = 0;
	u32 mode = 0x3;
	ret |= b53125_read64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), &port_ctrl);
	port_ctrl &= ~(B53_EAP_MODE_MASK << B53_EAP_MODE);
	port_ctrl |= ((B53_EAP_MODE_MASK & mode) << B53_EAP_MODE);
	ret |= b53125_write64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), port_ctrl);
	return ret;
}

static int b53125_eap_port_enable(sdk_driver_t *dev, zpl_phyport_t port, zpl_bool enable)
{
	int ret = 0;
	u32 mode = 1;
	u64 port_ctrl = 0;
	if(enable)
		mode = 1;
	else
		mode = 0;	
	ret |= b53125_read64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), &port_ctrl);
	port_ctrl &= ~(B53_EAP_BLK_MODE_M << B53_EAP_BLK_MODE);
	port_ctrl |= ((B53_EAP_BLK_MODE_M & mode) << B53_EAP_BLK_MODE) | B53_EAP_DA_EN;
	ret |= b53125_write64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), port_ctrl);
	ret |= b53125_eap_mode_set(dev, port);
	return ret;
}

DEFUN (b53125_sdk_eap_test,
		b53125_sdk_eap_test_cmd,
		"sdk-eap <0-4> (enable|disable)",
		"sdk vlan\n"
		"create\n"
		"enable\n"
		"vlanid\n")
{
	int ret = 0;
	b53125_eap_enable(__msdkdriver, 1);
	if(strcmp(argv[1], "enable") == 0)
	ret = b53125_eap_port_enable(__msdkdriver, atoi(argv[0]), 1);
	else
	ret = b53125_eap_port_enable(__msdkdriver, atoi(argv[0]), 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (b53125_sdk_eapip_test,
		b53125_sdk_eapip_test_cmd,
		"sdk-eap A.B.C.D/M (enable|disable)",
		"sdk vlan\n"
		"create\n"
		"enable\n"
		"vlanid\n")
{
	int ret = 0;
	zpl_uint32 index;
	u32 address = 0xc0a86464;
	u32 mask = 0xffffff00;
	if(strcmp(argv[1], "enable") == 0)
	{
		address = mask = 0;
	}
	else
	{
		address = mask = 0;
	}
	ret = b53125_eap_bypass_ipaddr_set(__msdkdriver, index, address, mask);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


int b53125_eap_init(void)
{
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_sdk_eap_test_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_sdk_eapip_test_cmd);
	sdk_8021x_cb.sdk_8021x_enable_cb = b53125_eap_enable;
	sdk_8021x_cb.sdk_8021x_port_enable_cb = b53125_eap_port_enable;
	sdk_8021x_cb.sdk_8021x_port_state_cb = NULL;
	//sdk_8021x_cb.sdk_8021x_port_mode_cb = b53125_eap_blk_mode_set;
	sdk_8021x_cb.sdk_8021x_auth_bypass_cb = NULL;
	sdk_8021x_cb.sdk_8021x_port_addmac = NULL;
	sdk_8021x_cb.sdk_8021x_port_delmac = NULL;
	sdk_8021x_cb.sdk_8021x_port_delallmac = NULL;
	return OK;
}

//EEE
/*static int sdk_eee_enable(ifindex_t ifindex, zpl_bool enable)
{
	struct b53xxx_ioctl data;
	memset(&data, 0, sizeof(data));
	data.cmd = CMD_EEE_ENABLE;
	data.module = SDK_MODULE_EEE;
	data.port = if_ifindex2phy(ifindex);
	data.enable = enable ? zpl_true:zpl_false;
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
