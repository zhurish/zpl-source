/*
 * b53_eap.c
 *
 *  Created on: 2019年9月12日
 *      Author: DELL
 */



#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_eap.h"


//8021X
static int b53125_eap_enable(sdk_driver_t *dev, zpl_bool enable)
{
	int ret = 0;
	u8 regval = 0;
	ret |= b53125_read8(dev->sdk_device, B53_EAP_PAGE, B53_EAP_GLOBAL, &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_EAP_PAGE, B53_EAP_GLOBAL, regval);
	if(enable)
	{
		regval |= B53_EAP_RARP_EN|B53_EAP_BPDU_EN|B53_EAP_RMC_EN|
				B53_EAP_DHCP_EN|B53_EAP_ARP_EN|B53_EAP_2DIP_EN;
	}
	else
	{
		regval = 0;
	}
	ret |= b53125_write8(dev->sdk_device, B53_EAP_PAGE, B53_EAP_GLOBAL, regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_EAP_PAGE, B53_EAP_GLOBAL, regval);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_eap_mode_set(sdk_driver_t *dev, zpl_phyport_t port, int mode)
{
	int ret = 0;
	u64 regval = 0;
	u32 val1 = 0, val2 = 0;

	ret |= b53125_read64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_EAP_PAGE, B53_EAP_PORT(port), regval);
	val2 = regval & 0xffffffff;
	val1 = (regval>>32) & 0xffffffff;
	val1 &= ~(B53_EAP_MODE_MASK << (B53_EAP_MODE-32));
	val1 |= ((B53_EAP_MODE_MASK & mode) << (B53_EAP_MODE-32));
	regval = val1;
	regval =  (regval<<32) | val2;
	ret |= b53125_write64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_EAP_PAGE, B53_EAP_PORT(port), regval);
	sdk_handle_return(ret);
	return ret;
}


static int b53125_eap_stat_set(sdk_driver_t *dev, zpl_phyport_t port, int stat)
{
	int ret = 0;
	u64 regval = 0;
	u32 val1 = 0, val2 = 0;

	ret |= b53125_read64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_EAP_PAGE, B53_EAP_PORT(port), regval);
	val2 = regval & 0xffffffff;
	val1 = (regval>>32) & 0xffffffff;
	val1 &= ~(B53_EAP_BLK_MODE_M << (B53_EAP_BLK_MODE-32));
	val1 |= ((B53_EAP_BLK_MODE_M & stat) << (B53_EAP_BLK_MODE-32));
	regval = val1;
	regval =  (regval<<32) | val2;
	ret |= b53125_write64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_EAP_PAGE, B53_EAP_PORT(port), regval);
	sdk_handle_return(ret);
	return ret;
}

static int b53125_eap_dmac_set(sdk_driver_t *dev, zpl_phyport_t port, mac_t *mac)
{
	int ret = 0;
	u64 regval = 0;
	u32 val1 = 0, val2 = 0;
	zpl_uint64 u = 0;
	zpl_uint32 i;
	ret |= b53125_read64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), &regval);
	sdk_debug_detail(dev, "read %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_EAP_PAGE, B53_EAP_PORT(port), regval);
	val2 = regval & 0xffffffff;
	val1 = (regval>>32) & 0xffffffff;
	val1 &= ~(1 << (B53_EAP_DA_EN-32));
	if(mac)
	{
		val1 |= ((1) << (B53_EAP_DA_EN-32));

		for (i = 0; i < ETH_ALEN; i++)
			u = u << 8 | mac[i];
	}
	val2 = 0;
	val1 &=	~(0x00007fff);
	regval = val1;
	regval =  (regval<<32) | val2;
	regval |= u;
	ret |= b53125_write64(dev->sdk_device, B53_EAP_PAGE, B53_EAP_PORT(port), regval);
	sdk_debug_detail(dev, "write %s(ret=%d) page=0x%x reg=0x%x val=0x%x", __func__, ret, B53_EAP_PAGE, B53_EAP_PORT(port), regval);
	sdk_handle_return(ret);
	return ret;
}
#if defined( _SDK_CLI_DEBUG_EN)	
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
#endif

int b53125_eap_init(sdk_driver_t *dev)
{
	#if defined( _SDK_CLI_DEBUG_EN)	
	install_element(SDK_NODE, CMD_CONFIG_LEVEL, &b53125_eap_test_cmd);
	install_element(SDK_NODE, CMD_CONFIG_LEVEL, &b53125_eap_port_test_cmd);
	#endif
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
