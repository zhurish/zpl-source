/*
 * b53125_driver.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include <zplos_include.h>
#include "hal_driver.h"
#include "sdk_driver.h"
#include "b53_driver.h"
#include "b53_port.h"

#define B53_BRCM_OUI_1	0x0143bc00
#define B53_BRCM_OUI_2	0x03625c00
#define B53_BRCM_OUI_3	0x00406000
#define B53_BRCM_OUI_4	0x01410c00

struct b53_chip_data {
	u32 chip_id;
	const char *dev_name;
	u16 vlans;
	u16 enabled_ports;
	u8 cpu_port;
	u8 vta_regs[3];
	u8 arl_entries;
	u8 arl_bins;
	u16 arl_buckets;
	u8 duplex_reg;
	u8 jumbo_pm_reg;
	u8 jumbo_size_reg;
};

#define B53_VLAN_TBL_REGS	\
	{ B53_VLAN_TBL_ACCESS, B53_VLAN_TBL_INDEX, B53_VLAN_TBL_ENTRY }

static const struct b53_chip_data b53_switch_chips[] = {
	{
		.chip_id = BCM53115_DEVICE_ID,
		.dev_name = "BCM53115",
		.vlans = 4096,
		.enabled_ports = 0x1f,
		.arl_entries = 4,
		.vta_regs = B53_VLAN_TBL_REGS,
		.cpu_port = B53_CPU_PORT,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
	},
	{
		.chip_id = BCM53125_DEVICE_ID,
		.dev_name = "BCM53125",
		.vlans = 4096,
		.enabled_ports = 0xff,
		.arl_entries = 4,
		.arl_bins = 4,
		.arl_buckets = 1024,
		.cpu_port = B53_CPU_PORT,
		.vta_regs = B53_VLAN_TBL_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		
	},
};

#define ARRAY_SIZE(x) (int)(sizeof(x) / sizeof(x[0]))

b53_device_t * b53125_device_probe()
{
	u32 phy_id = 0;
	zpl_uint32 i = 0;
	b53_device_t* b53_device = malloc(sizeof(b53_device_t));
	if(b53_device == NULL)
	{
		_sdk_err( " Can not malloc b53125 device");
		return NULL;
	}
	memset(b53_device, 0, sizeof(b53_device_t));
	//b53_device->fd = ipstack_socket(IPSTACK_AF_INET, IPSTACK_SOCK_DGRAM, 0);
	b53_device->mido.fd = open("/dev/"B53_DEVICE_NAME, O_RDWR);
	if(b53_device->mido.fd <= 0)
	{
		free(b53_device);
		_sdk_err( "Can not open b53125 device '%s'", B53_DEVICE_NAME);
		return NULL;
	}
	b53125_mdio_probe(&b53_device->mido);
	phy_id = b53125_mdio_read(&b53_device->mido, 0, 2);
	i = b53125_mdio_read(&b53_device->mido, 0, 3);
	//b53125_read8(b53_device, 0, 2, &phy_id) ;
	//b53125_read8(b53_device, 0, 3, &i);
	phy_id = (phy_id<<16)|i;
	//============b53125_device_probe============phy_id=0x3625f24
	_sdk_debug("b53125 device probe phy_id=0x%x", phy_id);
	/* BCM5325, BCM539x (OUI_1)
	 * BCM53125, BCM53128 (OUI_2)
	 * BCM5365 (OUI_3)
#define B53_BRCM_OUI_1	0x0143bc00
#define B53_BRCM_OUI_2	0x03625c00
#define B53_BRCM_OUI_3	0x00406000
#define B53_BRCM_OUI_4	0x01410c00
	 */

	if ((phy_id & 0xfffffc00) != B53_BRCM_OUI_1 &&
	    (phy_id & 0xfffffc00) != B53_BRCM_OUI_2 &&
	    (phy_id & 0xfffffc00) != B53_BRCM_OUI_3 &&
	    (phy_id & 0xfffffc00) != B53_BRCM_OUI_4) {
		free(b53_device);
		_sdk_err( "Unsupported device: 0x%08x", phy_id);
		//return NULL;
	}
	b53_device->mido.reg_page = 0xff;
	b53125_read32(b53_device, B53_MGMT_PAGE, B53_DEVICE_ID, &b53_device->chip_id);
	b53125_read8(b53_device, B53_MGMT_PAGE, B53_REV_ID,  &b53_device->core_rev);

	for (i = 0; i < ARRAY_SIZE(b53_switch_chips); i++)
	{
		const struct b53_chip_data *chip = &b53_switch_chips[i];
		if (chip->chip_id == b53_device->chip_id)
		{
			if (!b53_device->enabled_ports)
				b53_device->enabled_ports = chip->enabled_ports;
			b53_device->duplex_reg = chip->duplex_reg;
			b53_device->vta_regs[0] = chip->vta_regs[0];
			b53_device->vta_regs[1] = chip->vta_regs[1];
			b53_device->vta_regs[2] = chip->vta_regs[2];
			b53_device->jumbo_pm_reg = chip->jumbo_pm_reg;
			b53_device->cpu_port = chip->cpu_port;
			b53_device->num_vlans = chip->vlans;
			b53_device->num_arl_entries = chip->arl_entries;
			b53_device->num_arl_bins = chip->arl_bins;
			b53_device->num_arl_buckets = chip->arl_buckets;
			

			if (b53_device->chip_id == BCM53125_DEVICE_ID)
				_sdk_notice( "Find b53125 device on '%s' ID:0x%x REV:0x%x", B53_DEVICE_NAME,
					b53_device->chip_id, b53_device->core_rev);

			b53_device->num_ports = b53_device->cpu_port + 1;
			b53_device->enabled_ports |= BIT(b53_device->cpu_port);

			return b53_device;
		}
	}
	_sdk_err( "Can not find device by ID'0x%x'", b53_device->chip_id);
	return NULL;
}

int b53125_config_init(sdk_driver_t *dev)
{
	int ret = 0;
	if(dev == NULL)
	{
		return ERROR;
	}
	/*******global *******/

	ret |= b53_brcm_hdr_setup(dev, zpl_true, ((b53_device_t *)dev->sdk_device)->cpu_port);
	_sdk_debug( "b53125 brcm hdr init %s", (ret == OK)?"OK":"ERROR");
	ret |= b53125_imp_enable(dev, zpl_true);//关闭IMP接口
	_sdk_debug( "b53125 imp init %s", (ret == OK)?"OK":"ERROR");
	ret |= b53125_imp_port_enable(dev);
	_sdk_debug( "b53125 imp port %s", (ret == OK)?"OK":"ERROR");

	ret = b53125_global_init(dev);
	_sdk_debug( "b53125 global init %s", (ret == OK)?"OK":"ERROR");
	
	ret = b53125_cpu_init(dev);
	_sdk_debug( "b53125 cpu init %s", (ret == OK)?"OK":"ERROR");

	ret = b53125_mac_init(dev);
	_sdk_debug( "b53125 mac init %s", (ret == OK)?"OK":"ERROR");

	ret = b53125_port_init(dev);
	_sdk_debug( "b53125 port init %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_vlan_init(dev);
	_sdk_debug( "b53125 vlan init %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_mstp_init(dev);
	_sdk_debug( "b53125 mstp init %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_trunk_init(dev);
	_sdk_debug( "b53125 trunk init %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_mirror_init(dev);
	_sdk_debug( "b53125 mirror init %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_dos_init(dev);
	_sdk_debug( "b53125 dos init %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_eap_init(dev);
	_sdk_debug( "b53125 eap init %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_qos_init(dev);
	_sdk_debug( "b53125 qos init %s", (ret == OK)?"OK":"ERROR");

	ret = b53125_snooping_init(dev);
	_sdk_debug( "b53125 snooping init %s", (ret == OK)?"OK":"ERROR");

	ret = b53125_global_start(dev);
	_sdk_debug( "b53125  start %s", (ret == OK)?"OK":"ERROR");
	ret = b53125_port_start(dev);
	_sdk_debug( "b53125 port start %s", (ret == OK)?"OK":"ERROR");

	return ret;
}


