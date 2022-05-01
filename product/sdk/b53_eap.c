/*
 * b53_eap.c
 *
 *  Created on: 2019年9月12日
 *      Author: DELL
 */


#include <zplos_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_eap.h"


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
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_eap_mode_set(sdk_driver_t *dev, zpl_phyport_t port, int mode)
{
	int ret = 0;
	u64 port_ctrl = 0;
	u32 val1 = 0, val2 = 0;

	ret |= b53125_read64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), &port_ctrl);
	val2 = port_ctrl & 0xffffffff;
	val1 = (port_ctrl>>32) & 0xffffffff;
	val1 &= ~(B53_EAP_MODE_MASK << (B53_EAP_MODE-32));
	val1 |= ((B53_EAP_MODE_MASK & mode) << (B53_EAP_MODE-32));
	port_ctrl = val1;
	port_ctrl =  (port_ctrl<<32) | val2;
	ret |= b53125_write64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}


static int b53125_eap_stat_set(sdk_driver_t *dev, zpl_phyport_t port, int stat)
{
	int ret = 0;
	u64 port_ctrl = 0;
	u32 val1 = 0, val2 = 0;

	ret |= b53125_read64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), &port_ctrl);
	val2 = port_ctrl & 0xffffffff;
	val1 = (port_ctrl>>32) & 0xffffffff;
	val1 &= ~(B53_EAP_BLK_MODE_M << (B53_EAP_BLK_MODE-32));
	val1 |= ((B53_EAP_BLK_MODE_M & stat) << (B53_EAP_BLK_MODE-32));
	port_ctrl = val1;
	port_ctrl =  (port_ctrl<<32) | val2;
	ret |= b53125_write64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}

static int b53125_eap_dmac_set(sdk_driver_t *dev, zpl_phyport_t port, mac_t *mac)
{
	int ret = 0;
	u64 port_ctrl = 0;
	u32 val1 = 0, val2 = 0;
	zpl_uint64 u = 0;
	zpl_uint32 i;
	ret |= b53125_read64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), &port_ctrl);
	val2 = port_ctrl & 0xffffffff;
	val1 = (port_ctrl>>32) & 0xffffffff;
	val1 &= ~(1 << (B53_EAP_DA_EN-32));
	if(mac)
	{
		val1 |= ((1) << (B53_EAP_DA_EN-32));

		for (i = 0; i < ETH_ALEN; i++)
			u = u << 8 | mac[i];
	}
	val2 = 0;
	val1 &=	~(0x00007fff);
	port_ctrl = val1;
	port_ctrl =  (port_ctrl<<32) | val2;
	port_ctrl |= u;
	ret |= b53125_write64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), port_ctrl);
	_sdk_debug( "%s %s", __func__, (ret == OK)?"OK":"ERROR");
	return ret;
}
DEFUN (b53125_eap_test,
		b53125_eap_test_cmd,
		"sdk-eap enable",
		"sdk eap\n"
		"enable\n")
{
	int ret = 0;
	ret = b53125_eap_enable(__msdkdriver, 1);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
DEFUN (b53125_eap_port_test,
		b53125_eap_port_test_cmd,
		"sdk-eap port <0-4> stat <0-4>",
		"sdk eap\n"
		"port\n"
		"phyport id\n"
		"state\n"
		"state id\n")
{
	int ret = 0;
	ret = b53125_eap_stat_set(__msdkdriver, atoi(argv[0]), atoi(argv[1]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

int b53125_eap_init(sdk_driver_t *dev)
{
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_eap_test_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &b53125_eap_port_test_cmd);
	sdk_8021x_cb.sdk_8021x_enable_cb = b53125_eap_enable;
	sdk_8021x_cb.sdk_8021x_port_state_cb = b53125_eap_stat_set;
	sdk_8021x_cb.sdk_8021x_port_mode_cb = b53125_eap_mode_set;
	sdk_8021x_cb.sdk_8021x_auth_dmac_cb = b53125_eap_dmac_set;
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
