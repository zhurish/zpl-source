/*
 * b53125_driver.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */


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


b53_device_t * b53125_device_probe(void)
{
	u32 phy_id = 0;
	u32 i = 0;
	b53_device_t* b53_device = XMALLOC(MTYPE_SDK, sizeof(b53_device_t));
	if(b53_device == NULL)
	{
		_sdk_err( " Can not malloc b53125 device");
		return NULL;
	}
	memset(b53_device, 0, sizeof(b53_device_t));
#ifdef ZPL_SDK_USER
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
#else
	if(b53125_mdio_probe(&b53_device->mido) != 0)
	{
		XFREE(MTYPE_SDK, b53_device);
		return NULL;
	}
	//phy_id = b53125_read8(&b53_device->mido, 0, 2);
	//i = b53125_read8(&b53_device->mido, 0, 3);
	phy_id = mdiobus_read(b53_device->mido.bus, 0, 2) << 16;
	phy_id |= mdiobus_read(b53_device->mido.bus, 0, 3);
#endif	
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
		XFREE(MTYPE_SDK, b53_device);
		_sdk_err( "Unsupported device: 0x%08x", phy_id);
		//return NULL;
	}
	b53_device->mido.reg_page = 0xff;
	b53125_read32(b53_device, B53_MGMT_PAGE, B53_DEVICE_ID, &b53_device->chip_id);
	b53125_read8(b53_device, B53_MGMT_PAGE, B53_REV_ID,  &b53_device->core_rev);

	for (i = 0; i < ZPL_ARRAY_SIZE(b53_switch_chips); i++)
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
				_sdk_notice( "Find b53125 device  ID:0x%x REV:0x%x",
					b53_device->chip_id, b53_device->core_rev);

			b53_device->num_ports = b53_device->cpu_port + 1;
			b53_device->enabled_ports |= BIT(b53_device->cpu_port);

			return b53_device;
		}
	}
	_sdk_err( "Can not find device by ID'0x%x'", b53_device->chip_id);
	XFREE(MTYPE_SDK, b53_device);
	return NULL;
}

int b53125_device_exit(struct b53125_device *b53_device)
{
	XFREE(MTYPE_SDK, b53_device);
	return 0;
}

int b53125_config_start(sdk_driver_t *dev)
{
	int ret = 0;
	if(dev == NULL)
	{
		return ERROR;
	}
	/*******global *******/
	dev->ports_table[0] = 0;
	dev->ports_table[1] = 1;
	dev->ports_table[2] = 2;
	dev->ports_table[3] = 3;
	dev->ports_table[4] = 4;
	dev->ports_table[5] = -1;
	dev->ports_table[6] = -1;
	dev->ports_table[7] = 8;


	ret |= b53_brcm_hdr_setup(dev, zpl_true, ((b53_device_t *)dev->sdk_device)->cpu_port);
	_sdk_debug( "b53125 brcm hdr init %s", (ret == OK)?"OK":"ERROR");
	ret |= b53125_imp_enable(dev, zpl_true);//关闭IMP接口
	_sdk_debug( "b53125 imp init %s", (ret == OK)?"OK":"ERROR");
	//b53125_switch_forwarding and b53125_vlan_init
	//ret |= b53125_imp_port_enable(dev);
	//_sdk_debug( "b53125 imp port %s", (ret == OK)?"OK":"ERROR");

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


/*
[    1.714407] ====b53_can_enable_brcm_tags tag_protocol=0 port=8
[    1.792396] ====b53_set_forwarding page=0x0 reg=0xb val=0x4
[    1.793126] ====b53_set_forwarding page=0x0 reg=0x22 val=0x40
[    1.793837] ====b53_set_forwarding page=0x0 reg=0x21 val=0xc3
[    1.798816] ====b53_enable_vlan page=0x0 reg=0xb val=0x4
[    1.798833] bcm53xx stmmac-0:1e: Port -1 VLAN enabled: 1, filtering: 0
[    1.803790] bcm53xx stmmac-0:1e: VID: 0, members: 0x01ff, untag: 0x01ff
[    1.806158] ====b53_set_forwarding page=0x0 reg=0xb val=0x6
[    1.806891] ====b53_set_forwarding page=0x0 reg=0x22 val=0x40
[    1.807617] ====b53_set_forwarding page=0x0 reg=0x21 val=0xc3
[    1.813678] ====b53_enable_cpu_port page=0x0 reg=0x8 val=0x1c
[    1.814416] ====b53_brcm_hdr_setup page=0x0 reg=0xb val=0x7
[    1.815266] ====b53_brcm_hdr_setup page=0x2 reg=0x0 val=0x80
[    1.815993] ====b53_brcm_hdr_setup page=0x2 reg=0x3 val=0x3
[    1.816844] ====b53_port_set_ucast_flood page=0x0 reg=0x32 val=0x100
[    1.817587] ====b53_port_set_mcast_flood page=0x0 reg=0x34 val=0x100
[    1.818347] ====b53_port_set_mcast_flood page=0x0 reg=0x36 val=0x100
[    1.819128] ====b53_port_set_learning page=0x0 reg=0x3c val=0x100
[    1.890385] ====b53_br_set_stp_state page=0x0 reg=0x5 val=0x23
[    1.891862] ====b53_br_set_stp_state page=0x0 reg=0x6 val=0x20
[    1.893273] ====b53_br_set_stp_state page=0x0 reg=0x7 val=0x20
[    1.912791] ====b53_br_set_stp_state page=0x0 reg=0x8 val=0xbc
[    3.656155] ====b53_port_set_ucast_flood page=0x0 reg=0x32 val=0x108
[    3.660012] ====b53_port_set_mcast_flood page=0x0 reg=0x34 val=0x108
[    3.661561] ====b53_port_set_mcast_flood page=0x0 reg=0x36 val=0x108
[    3.662344] ====b53_port_set_learning page=0x0 reg=0x3c val=0x108
[    3.664249] ====b53_imp_vlan_setup page=0x31 reg=0x0 val=0x1ff
[    3.664975] ====b53_imp_vlan_setup page=0x31 reg=0x2 val=0x1ff
[    3.665697] ====b53_imp_vlan_setup page=0x31 reg=0x4 val=0x1ff
[    3.666420] ====b53_imp_vlan_setup page=0x31 reg=0x6 val=0x108
[    3.667145] ====b53_imp_vlan_setup page=0x31 reg=0x8 val=0x1ff
[    3.667866] ====b53_imp_vlan_setup page=0x31 reg=0xa val=0x1ff
[    3.668590] ====b53_imp_vlan_setup page=0x31 reg=0xc val=0x100
[    3.669314] ====b53_imp_vlan_setup page=0x31 reg=0xe val=0x100
[    3.670038] ====b53_imp_vlan_setup page=0x31 reg=0x10 val=0x1ff
[    3.670880] ====b53_br_set_stp_state page=0x0 reg=0x3 val=0xa0
[    3.695169] ====b53_port_set_ucast_flood page=0x0 reg=0x32 val=0x109
[    3.696309] ====b53_port_set_mcast_flood page=0x0 reg=0x34 val=0x109
[    3.697330] ====b53_port_set_mcast_flood page=0x0 reg=0x36 val=0x109
[    3.698041] ====b53_port_set_learning page=0x0 reg=0x3c val=0x109
[    3.699861] ====b53_imp_vlan_setup page=0x31 reg=0x0 val=0x101
[    3.700553] ====b53_imp_vlan_setup page=0x31 reg=0x2 val=0x1ff
[    3.701243] ====b53_imp_vlan_setup page=0x31 reg=0x4 val=0x1ff
[    3.701979] ====b53_imp_vlan_setup page=0x31 reg=0x6 val=0x108
[    3.702692] ====b53_imp_vlan_setup page=0x31 reg=0x8 val=0x1ff
[    3.703386] ====b53_imp_vlan_setup page=0x31 reg=0xa val=0x1ff
[    3.704110] ====b53_imp_vlan_setup page=0x31 reg=0xc val=0x100
[    3.704836] ====b53_imp_vlan_setup page=0x31 reg=0xe val=0x100
[    3.705559] ====b53_imp_vlan_setup page=0x31 reg=0x10 val=0x1ff
[    3.706404] ====b53_br_set_stp_state page=0x0 reg=0x0 val=0xa0
[    3.727562] ====b53_port_set_ucast_flood page=0x0 reg=0x32 val=0x10b
[    3.728714] ====b53_port_set_mcast_flood page=0x0 reg=0x34 val=0x10b
[    3.729782] ====b53_port_set_mcast_flood page=0x0 reg=0x36 val=0x10b
[    3.730845] ====b53_port_set_learning page=0x0 reg=0x3c val=0x10b
[    3.732927] ====b53_imp_vlan_setup page=0x31 reg=0x0 val=0x101
[    3.733622] ====b53_imp_vlan_setup page=0x31 reg=0x2 val=0x102
[    3.734314] ====b53_imp_vlan_setup page=0x31 reg=0x4 val=0x1ff
[    3.735004] ====b53_imp_vlan_setup page=0x31 reg=0x6 val=0x108
[    3.735696] ====b53_imp_vlan_setup page=0x31 reg=0x8 val=0x1ff
[    3.736387] ====b53_imp_vlan_setup page=0x31 reg=0xa val=0x1ff
[    3.737077] ====b53_imp_vlan_setup page=0x31 reg=0xc val=0x100
[    3.737784] ====b53_imp_vlan_setup page=0x31 reg=0xe val=0x100
[    3.738499] ====b53_imp_vlan_setup page=0x31 reg=0x10 val=0x1ff
[    3.739346] ====b53_br_set_stp_state page=0x0 reg=0x1 val=0xa0
[    3.760532] ====b53_port_set_ucast_flood page=0x0 reg=0x32 val=0x10f
[    3.761878] ====b53_port_set_mcast_flood page=0x0 reg=0x34 val=0x10f
[    3.762960] ====b53_port_set_mcast_flood page=0x0 reg=0x36 val=0x10f
[    3.764015] ====b53_port_set_learning page=0x0 reg=0x3c val=0x10f
[    3.766069] ====b53_imp_vlan_setup page=0x31 reg=0x0 val=0x101
[    3.766764] ====b53_imp_vlan_setup page=0x31 reg=0x2 val=0x102
[    3.767454] ====b53_imp_vlan_setup page=0x31 reg=0x4 val=0x104
[    3.768145] ====b53_imp_vlan_setup page=0x31 reg=0x6 val=0x108
[    3.768836] ====b53_imp_vlan_setup page=0x31 reg=0x8 val=0x1ff
[    3.769526] ====b53_imp_vlan_setup page=0x31 reg=0xa val=0x1ff
[    3.770231] ====b53_imp_vlan_setup page=0x31 reg=0xc val=0x100
[    3.770944] ====b53_imp_vlan_setup page=0x31 reg=0xe val=0x100
[    3.771637] ====b53_imp_vlan_setup page=0x31 reg=0x10 val=0x1ff
[    3.772487] ====b53_br_set_stp_state page=0x0 reg=0x2 val=0xa0
[    3.793684] ====b53_port_set_ucast_flood page=0x0 reg=0x32 val=0x11f
[    3.794825] ====b53_port_set_mcast_flood page=0x0 reg=0x34 val=0x11f
[    3.795892] ====b53_port_set_mcast_flood page=0x0 reg=0x36 val=0x11f
[    3.796914] ====b53_port_set_learning page=0x0 reg=0x3c val=0x11f
[    3.798751] ====b53_imp_vlan_setup page=0x31 reg=0x0 val=0x101
[    3.799443] ====b53_imp_vlan_setup page=0x31 reg=0x2 val=0x102
[    3.800134] ====b53_imp_vlan_setup page=0x31 reg=0x4 val=0x104
[    3.800824] ====b53_imp_vlan_setup page=0x31 reg=0x6 val=0x108
[    3.801523] ====b53_imp_vlan_setup page=0x31 reg=0x8 val=0x110
[    3.802240] ====b53_imp_vlan_setup page=0x31 reg=0xa val=0x1ff
[    3.802934] ====b53_imp_vlan_setup page=0x31 reg=0xc val=0x100
[    3.803654] ====b53_imp_vlan_setup page=0x31 reg=0xe val=0x100
[    3.804380] ====b53_imp_vlan_setup page=0x31 reg=0x10 val=0x1ff
[    3.805224] ====b53_br_set_stp_state page=0x0 reg=0x4 val=0xa0

*/