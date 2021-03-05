/*
 * b53_mdio.c
 *
 *  Created on: May 1, 2019
 *      Author: zhurish
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include "b53_mdio.h"
#include "b53_regs.h"


//struct b53125_device *b53_device = NULL;

#if 1
static int __mdio_read(struct b53125_device *dev, int addr, u32 regnum)
{
	struct mido_data_b53 data;
	data.mii_id = addr;
	data.regnum = regnum;
	data.len = 2;
	if(ioctl(dev->fd, B53_IO_R, &data) == 0)
		return data.value;
	printf("==============%s\r\n", strerror(errno));
/*	if(read(dev->fd, &data, sizeof(struct mido_data_b53)) > 0)
		return data.value;*/
	return 0;
}

static int __mdio_write(struct b53125_device *dev, int addr, u32 regnum, u16 val)
{
	struct mido_data_b53 data;
	data.mii_id = addr;
	data.regnum = regnum;
	data.len = 2;
	data.value = val;
	if(ioctl(dev->fd, B53_IO_W, &data) == 0)
		return 0;
	printf("==============%s\r\n", strerror(errno));
/*	if(write(dev->fd, &data, sizeof(struct mido_data_b53)) > 0)
		return 0;*/
	return 1;
}
#else
static int __mdio_read(struct b53125_device *dev, int addr, u32 regnum)
{
	struct ifreq ifr;
	struct mii_ioctl_data mii_val;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name)-1);
	ifr.ifr_data = &mii_val;
	if(ioctl(dev->fd, SIOCGMIIPHY, &ifr) == -1)
	{
		printf("==============%s\r\n", strerror(errno));
		return -1;
	}
	//mii_val->phy_id,
	printf("==============%x\r\n", mii_val.phy_id);
	mii_val.reg_num = regnum;
	if(ioctl(dev->fd, SIOCGMIIREG, &ifr) == -1)
		return 0;
	return mii_val.val_out;
}

static int __mdio_write(struct b53125_device *dev, int addr, u32 regnum, u16 val)
{
	struct ifreq ifr;
	struct mii_ioctl_data mii_val;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name)-1);
	ifr.ifr_data = &mii_val;
/*	if(ioctl(dev->fd, SIOCGMIIPHY, &ifr) != -1)
		return -1;*/
	mii_val.phy_id = addr;
	printf("==============%d", mii_val.phy_id);
	mii_val.reg_num = regnum;
	mii_val.val_in = val;
	if(ioctl(dev->fd, SIOCSMIIREG, &ifr) == -1)
		return -1;
	return 0;
}
#endif


static int b53125_mdio_op(struct b53125_device *dev, u8 page, u8 reg, u16 op)
{
	ospl_uint32 i;
	u16 v;
	int ret;

	if (dev->reg_page != page) {
		/* set page number */
		v = (page << 8) | REG_MII_PAGE_ENABLE;
		ret = __mdio_write(dev, BRCM_PSEUDO_PHY_ADDR,
					   REG_MII_PAGE, v);
		if (ret)
			return ERROR;
		dev->reg_page = page;
	}

	/* set register address */
	v = (reg << 8) | op;
	ret = __mdio_write(dev, BRCM_PSEUDO_PHY_ADDR, REG_MII_ADDR, v);
	if (ret)
		return ERROR;

	/* check if operation completed */
	for (i = 0; i < 50; ++i) {
		v = __mdio_read(dev, BRCM_PSEUDO_PHY_ADDR,
					REG_MII_ADDR);
		if (!(v & (REG_MII_ADDR_WRITE | REG_MII_ADDR_READ)))
			break;
		os_usleep(100);
	}

	if ((i == 50))
		return ERROR;
	return OK;
}

static int b53125_mdio_read8(struct b53125_device *dev, u8 page, u8 reg, u8 *val)
{
	int ret;

	ret = b53125_mdio_op(dev, page, reg, REG_MII_ADDR_READ);
	if (ret == ERROR)
		return ret;

	*val = __mdio_read(dev, BRCM_PSEUDO_PHY_ADDR,
				   REG_MII_DATA0) & 0xff;

	return 0;
}

static int b53125_mdio_read16(struct b53125_device *dev, u8 page, u8 reg, u16 *val)
{
	int ret;

	ret = b53125_mdio_op(dev, page, reg, REG_MII_ADDR_READ);
	if (ret == ERROR)
		return ret;

	*val = __mdio_read(dev, BRCM_PSEUDO_PHY_ADDR, REG_MII_DATA0);

	return 0;
}

static int b53125_mdio_read32(struct b53125_device *dev, u8 page, u8 reg, u32 *val)
{
	int ret;

	ret = b53125_mdio_op(dev, page, reg, REG_MII_ADDR_READ);
	if (ret == ERROR)
		return ret;

	*val = __mdio_read(dev, BRCM_PSEUDO_PHY_ADDR, REG_MII_DATA0);
	*val |= __mdio_read(dev, BRCM_PSEUDO_PHY_ADDR,
				    REG_MII_DATA1) << 16;

	return 0;
}

static int b53125_mdio_read48(struct b53125_device *dev, u8 page, u8 reg, u64 *val)
{
	u64 temp = 0;
	ospl_uint32 i;
	int ret;

	ret = b53125_mdio_op(dev, page, reg, REG_MII_ADDR_READ);
	if (ret == ERROR)
		return ret;

	for (i = 2; i >= 0; i--) {
		temp <<= 16;
		temp |= __mdio_read(dev, BRCM_PSEUDO_PHY_ADDR,
				     REG_MII_DATA0 + i);
	}

	*val = temp;

	return 0;
}

static int b53125_mdio_read64(struct b53125_device *dev, u8 page, u8 reg, u64 *val)
{
	u64 temp = 0;
	ospl_uint32 i;
	int ret;

	ret = b53125_mdio_op(dev, page, reg, REG_MII_ADDR_READ);
	if (ret == ERROR)
		return ret;

	for (i = 3; i >= 0; i--) {
		temp <<= 16;
		temp |= __mdio_read(dev, BRCM_PSEUDO_PHY_ADDR,
					    REG_MII_DATA0 + i);
	}

	*val = temp;

	return 0;
}

static int b53125_mdio_write8(struct b53125_device *dev, u8 page, u8 reg, u8 value)
{
	int ret;

	ret = __mdio_write(dev, BRCM_PSEUDO_PHY_ADDR,
				   REG_MII_DATA0, value);
	if (ret)
		return ret;

	return b53125_mdio_op(dev, page, reg, REG_MII_ADDR_WRITE);
}

static int b53125_mdio_write16(struct b53125_device *dev, u8 page, u8 reg,
			    u16 value)
{
	int ret;

	ret = __mdio_write(dev, BRCM_PSEUDO_PHY_ADDR,
				   REG_MII_DATA0, value);
	if (ret)
		return ret;

	return b53125_mdio_op(dev, page, reg, REG_MII_ADDR_WRITE);
}

static int b53125_mdio_write32(struct b53125_device *dev, u8 page, u8 reg,
			    u32 value)
{
	ospl_uint32 i;
	u32 temp = value;

	for (i = 0; i < 2; i++) {
		int ret = __mdio_write(dev, BRCM_PSEUDO_PHY_ADDR,
					       REG_MII_DATA0 + i,
					       temp & 0xffff);
		if (ret)
			return ret;
		temp >>= 16;
	}

	return b53125_mdio_op(dev, page, reg, REG_MII_ADDR_WRITE);
}

static int b53125_mdio_write48(struct b53125_device *dev, u8 page, u8 reg,
			    u64 value)
{
	ospl_uint32 i;
	u64 temp = value;

	for (i = 0; i < 3; i++) {
		int ret = __mdio_write(dev, BRCM_PSEUDO_PHY_ADDR,
					       REG_MII_DATA0 + i,
					       temp & 0xffff);
		if (ret)
			return ret;
		temp >>= 16;
	}

	return b53125_mdio_op(dev, page, reg, REG_MII_ADDR_WRITE);
}

static int b53125_mdio_write64(struct b53125_device *dev, u8 page, u8 reg,
			    u64 value)
{
	ospl_uint32 i;
	u64 temp = value;

	for (i = 0; i < 4; i++) {
		int ret = __mdio_write(dev, BRCM_PSEUDO_PHY_ADDR,
					       REG_MII_DATA0 + i,
					       temp & 0xffff);
		if (ret)
			return ret;
		temp >>= 16;
	}

	return b53125_mdio_op(dev, page, reg, REG_MII_ADDR_WRITE);
}

/*
static int b53125_mdio_phy_read16(struct b53125_device *dev, int addr, int reg,
			       u16 *value)
{
	*value = __mdio_read(dev, addr, reg);
	return 0;
}

static int b53125_mdio_phy_write16(struct b53125_device *dev, int addr, int reg,
				u16 value)
{
	return __mdio_write(dev, addr, reg, value);
}
*/

static const struct b53_io_ops b53125_mdio_ops = {
	.read8 = b53125_mdio_read8,
	.read16 = b53125_mdio_read16,
	.read32 = b53125_mdio_read32,
	.read48 = b53125_mdio_read48,
	.read64 = b53125_mdio_read64,
	.write8 = b53125_mdio_write8,
	.write16 = b53125_mdio_write16,
	.write32 = b53125_mdio_write32,
	.write48 = b53125_mdio_write48,
	.write64 = b53125_mdio_write64,
	//.phy_read16 = b53125_mdio_phy_read16,
	//.phy_write16 = b53125_mdio_phy_write16,
};

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
	u8 duplex_reg;
	u8 jumbo_pm_reg;
	u8 jumbo_size_reg;
};

#define B53_VTA_REGS	\
	{ B53_VT_ACCESS, B53_VT_INDEX, B53_VT_ENTRY }
#define B53_VTA_REGS_9798 \
	{ B53_VT_ACCESS_9798, B53_VT_INDEX_9798, B53_VT_ENTRY_9798 }
#define B53_VTA_REGS_63XX \
	{ B53_VT_ACCESS_63XX, B53_VT_INDEX_63XX, B53_VT_ENTRY_63XX }

static const struct b53_chip_data b53_switch_chips[] = {
	{
		.chip_id = BCM53115_DEVICE_ID,
		.dev_name = "BCM53115",
		.vlans = 4096,
		.enabled_ports = 0x1f,
		.arl_entries = 4,
		.vta_regs = B53_VTA_REGS,
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
		.cpu_port = B53_CPU_PORT,
		.vta_regs = B53_VTA_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
	},
	{
		.chip_id = BCM53128_DEVICE_ID,
		.dev_name = "BCM53128",
		.vlans = 4096,
		.enabled_ports = 0x1ff,
		.arl_entries = 4,
		.cpu_port = B53_CPU_PORT,
		.vta_regs = B53_VTA_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
	},
};

#define ARRAY_SIZE(x) (int)(sizeof(x) / sizeof(x[0]))

struct b53125_device * b53125_mdio_probe()
{
	u32 phy_id = 0;
	ospl_uint32 i = 0;
	struct b53125_device * b53_device = malloc(sizeof(struct b53125_device));
	if(b53_device == NULL)
	{
		zlog_debug(MODULE_HAL, " Can not malloc b53125 device");
		return NULL;
	}
	memset(b53_device, 0, sizeof(struct b53125_device));
	//b53_device->fd = socket(AF_INET, SOCK_DGRAM, 0);
	b53_device->fd = open("/dev/"B53_DEVICE_NAME, O_RDWR);
	if(b53_device->fd <= 0)
	{
		free(b53_device);
		zlog_debug(MODULE_HAL, "Can not open b53125 device '%s'", B53_DEVICE_NAME);
		return NULL;
	}
	b53_device->ops = &b53125_mdio_ops;

	phy_id = __mdio_read(b53_device, 0, 2) << 16;
	phy_id |= __mdio_read(b53_device, 0, 3);

	//printf("============%s============phy_id=0x%x\r\n", __func__, phy_id);
	/* BCM5325, BCM539x (OUI_1)
	 * BCM53125, BCM53128 (OUI_2)
	 * BCM5365 (OUI_3)
	 */
	if ((phy_id & 0xfffffc00) != B53_BRCM_OUI_1 &&
	    (phy_id & 0xfffffc00) != B53_BRCM_OUI_2 &&
	    (phy_id & 0xfffffc00) != B53_BRCM_OUI_3 &&
	    (phy_id & 0xfffffc00) != B53_BRCM_OUI_4) {
		free(b53_device);
		zlog_debug(MODULE_HAL, "Unsupported device: 0x%08x", phy_id);
		return NULL;
	}
	b53_device->reg_page = 0xff;
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
			if (b53_device->chip_id == BCM53115_DEVICE_ID)
			{
				u64 strap_value;
				b53125_read48(b53_device, B53_STAT_PAGE, B53_STRAP_VALUE, &strap_value);
				if (strap_value & SV_GMII_CTRL_115)
					b53_device->cpu_port = 5;

				zlog_debug(MODULE_HAL, "Find b53115 device on '%s' ID:0x%x REV:0x%x", B53_DEVICE_NAME,
						b53_device->chip_id, b53_device->core_rev);
			}
			if (b53_device->chip_id == BCM53125_DEVICE_ID)
				zlog_debug(MODULE_HAL, "Find b53125 device on '%s' ID:0x%x REV:0x%x", B53_DEVICE_NAME,
					b53_device->chip_id, b53_device->core_rev);

			b53_device->num_ports = b53_device->cpu_port + 1;
			b53_device->enabled_ports |= BIT(b53_device->cpu_port);

/*
			zlog_debug(MODULE_HAL, "Find b53125 device on '%s' ID:0x%x REV:0x%x", B53_DEVICE_NAME,
					b53_device->chip_id, b53_device->core_rev);

			zlog_debug(MODULE_HAL, "Find b53125 device on '%s' ID:0x%x REV:0x%x", B53_DEVICE_NAME,
					b53_device->chip_id, b53_device->core_rev);
*/
			b53_device->ops = &b53125_mdio_ops;
			return b53_device;
		}
	}
	zlog_debug(MODULE_HAL, "Can not find device by ID'0x%x'", b53_device->chip_id);
	return NULL;
}

int b53125_config_init(struct b53125_device *dev)
{
	int ret = 0;
	if(dev == NULL)
	{
		return ERROR;
	}
	/*******global *******/
	b53_brcm_hdr_setup(dev, ospl_true, dev->cpu_port);

	ret |= b53125_imp_enable(dev, ospl_false);//关闭IMP接口
	ret |= b53125_switch_forwarding(dev, ospl_false);//禁止转发


	ret |= b53125_switch_manege(dev, ospl_true);//设置为managed mode
	ret |= b53125_switch_forwarding(dev, ospl_true);//使能转发

	ret |= b53125_multicast_flood(dev, ospl_true);//使能多播泛洪
	ret |= b53125_unicast_flood(dev, ospl_true);//使能单播泛洪

	ret |= b53125_multicast_learning(dev, ospl_true);//使能多播报文学习源MAC

	ret |= b53125_enable_bpdu(dev, ospl_true);

	ret |= b53125_enable_learning(dev, dev->cpu_port, ospl_true);
	ret |= b53125_enable_learning(dev, 0, ospl_true);
	ret |= b53125_enable_learning(dev, 1, ospl_true);
	ret |= b53125_enable_learning(dev, 2, ospl_true);
	ret |= b53125_enable_learning(dev, 3, ospl_true);
	ret |= b53125_enable_learning(dev, 4, ospl_true);
	ret |= b53125_enable_learning(dev, 6, ospl_true);

	ret |= b53125_software_learning(dev, dev->cpu_port, ospl_true);
	ret |= b53125_software_learning(dev, 0, ospl_true);
	ret |= b53125_software_learning(dev, 1, ospl_true);
	ret |= b53125_software_learning(dev, 2, ospl_true);
	ret |= b53125_software_learning(dev, 3, ospl_true);
	ret |= b53125_software_learning(dev, 4, ospl_true);
	ret |= b53125_software_learning(dev, 6, ospl_true);

	ret |= b53125_imp_enable(dev, ospl_true);

	u8 mgmt = 0;
	//ret |= b53125_imp_port_enable(dev);
	ret |= b53125_read8(dev, B53_MGMT_PAGE, B53_MGMT_CTRL, &mgmt);
	printf("==============mgmt=%x=================\r\n", mgmt);
/*	void b53_brcm_hdr_setup(struct b53125_device *dev, ospl_bool enable, int port);
	int b53125_switch_mode(struct b53125_device *dev, ospl_bool manege);
	int b53125_switch_forwarding(struct b53125_device *dev, ospl_bool enable);
	int b53125_multicast_flood(struct b53125_device *dev, ospl_bool enable);
	int b53125_unicast_flood(struct b53125_device *dev, ospl_bool enable);
	int b53125_range_error(struct b53125_device *dev, ospl_bool enable);
	int b53125_multicast_learning(struct b53125_device *dev, ospl_bool enable);
	int b53125_puase_frame_detection(struct b53125_device *dev, ospl_bool enable);
	int b53125_enable_bpdu(struct b53125_device *dev, ospl_bool enable);
	int b53125_aging_time(struct b53125_device *dev, int agetime);
	int b53125_imp_mode(struct b53125_device *dev, ospl_bool enable);*/

	/******* IMP PORT *******/
/*		int b53125_imp_enable(struct b53125_device *dev, ospl_bool enable);
	int b53125_imp_speed(struct b53125_device *dev, int speed);
	int b53125_imp_duplex(struct b53125_device *dev, int duplex);
	int b53125_imp_flow(struct b53125_device *dev, ospl_bool rx, ospl_bool tx);*/
	return ret;
}
