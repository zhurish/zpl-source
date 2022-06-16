/*
 * b53_mdio.c
 *
 *  Created on: May 1, 2019
 *      Author: zhurish
 */

#ifdef ZPL_SDK_USER
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
#include "b53_driver.h"
#include "b53_mdio.h"
#include "b53_global.h"
#else
#include <linux/kernel.h>
#include <linux/phy.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/brcmphy.h>
#include <linux/rtnetlink.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include "b53_mdio.h"
#endif

#ifdef ZPL_SDK_USER
int b53125_mdio_read(struct b53_mdio_device *dev, u8 page, u8 reg)
{
	int ret;
	struct mido_device data;
	memset(&data, 0, sizeof(data));
	data.pape = page;
	data.regnum = reg;
	ret = read(dev->fd, &data, sizeof(struct mido_device));
	if(ret >= 0)
	{
		return data.val.val16;
	}
	return -1;
}

static int b53125_mdio_read8(struct b53_mdio_device *dev, u8 page, u8 reg, u8 *val)
{
	int ret;
	struct mido_device data;
	memset(&data, 0, sizeof(data));
	data.pape = page;
	data.regnum = reg;
	ret = ioctl(dev->fd, B53_IO_R8, &data);
	if(ret == 0)
	{
		*val = 	data.val.val8;
		return 0;
	}
	return ret;
}

static int b53125_mdio_read16(struct b53_mdio_device *dev, u8 page, u8 reg, u16 *val)
{
	int ret;
	struct mido_device data;
	memset(&data, 0, sizeof(data));
	data.pape = page;
	data.regnum = reg;
	ret = ioctl(dev->fd, B53_IO_R16, &data);
	if(ret == 0)
	{
		*val = 	data.val.val16;
		return 0;
	}
	return ret;
}

static int b53125_mdio_read32(struct b53_mdio_device *dev, u8 page, u8 reg, u32 *val)
{
	int ret;
	struct mido_device data;
	memset(&data, 0, sizeof(data));
	data.pape = page;
	data.regnum = reg;
	ret = ioctl(dev->fd, B53_IO_R32, &data);
	if(ret == 0)
	{
		*val = 	data.val.val32;
		return 0;
	}
	return ret;
}

static int b53125_mdio_read48(struct b53_mdio_device *dev, u8 page, u8 reg, u64 *val)
{
	int ret;
	struct mido_device data;
	memset(&data, 0, sizeof(data));
	data.pape = page;
	data.regnum = reg;
	ret = ioctl(dev->fd, B53_IO_R48, &data);
	if(ret == 0)
	{
		*val = 	data.val.val48;
		return 0;
	}
	return ret;
}

static int b53125_mdio_read64(struct b53_mdio_device *dev, u8 page, u8 reg, u64 *val)
{
	int ret;
	struct mido_device data;
	memset(&data, 0, sizeof(data));
	data.pape = page;
	data.regnum = reg;
	ret = ioctl(dev->fd, B53_IO_R64, &data);
	if(ret == 0)
	{
		*val = 	data.val.val64;
		return 0;
	}
	return ret;
}

static int b53125_mdio_write8(struct b53_mdio_device *dev, u8 page, u8 reg, u8 value)
{
	int ret;
	struct mido_device data;
	memset(&data, 0, sizeof(data));
	data.pape = page;
	data.regnum = reg;
	data.val.val8 = value;
	ret = ioctl(dev->fd, B53_IO_W8, &data);
	if(ret == 0)
	{
		return 0;
	}
	return ret;
}

static int b53125_mdio_write16(struct b53_mdio_device *dev, u8 page, u8 reg,
			    u16 value)
{
	int ret;
	struct mido_device data;
	memset(&data, 0, sizeof(data));
	data.pape = page;
	data.regnum = reg;
	data.val.val16 = value;
	ret = ioctl(dev->fd, B53_IO_W16, &data);
	if(ret == 0)
	{
		return 0;
	}
	return ret;
}

static int b53125_mdio_write32(struct b53_mdio_device *dev, u8 page, u8 reg,
			    u32 value)
{
	int ret;
	struct mido_device data;
	memset(&data, 0, sizeof(data));
	data.pape = page;
	data.regnum = reg;
	data.val.val32 = value;
	ret = ioctl(dev->fd, B53_IO_W32, &data);
	if(ret == 0)
	{
		return 0;
	}
	return ret;
}

static int b53125_mdio_write48(struct b53_mdio_device *dev, u8 page, u8 reg,
			    u64 value)
{
	int ret;
	struct mido_device data;
	memset(&data, 0, sizeof(data));
	data.pape = page;
	data.regnum = reg;
	data.val.val48 = value;
	ret = ioctl(dev->fd, B53_IO_W48, &data);
	if(ret == 0)
	{
		return 0;
	}
	return ret;
}

static int b53125_mdio_write64(struct b53_mdio_device *dev, u8 page, u8 reg,
			    u64 value)
{
	int ret;
	struct mido_device data;
	memset(&data, 0, sizeof(data));
	data.pape = page;
	data.regnum = reg;
	data.val.val64 = value;
	ret = ioctl(dev->fd, B53_IO_W64, &data);
	if(ret == 0)
	{
		return 0;
	}
	return ret;
}

/*
static int b53125_mdio_phy_read16(struct b53_mdio_device *dev, int addr, int reg,
			       u16 *value)
{
	*value = __mdio_read(dev, addr, reg);
	return 0;
}

static int b53125_mdio_phy_write16(struct b53_mdio_device *dev, int addr, int reg,
				u16 value)
{
	return __mdio_write(dev, addr, reg, value);
}
*/

static const struct b53_mdio_ops b53125_mdio_ops = {
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

int b53125_mdio_probe(struct b53_mdio_device *mdio)
{
	mdio->ops = &b53125_mdio_ops;
	return OK;
}
#else
static int b53125_mdio_op(struct b53_mdio_device *dev, u8 page, u8 reg, u16 op)
{
	int i;
	u16 v;
	int ret;
	struct mii_bus *bus = dev->bus;

	if (dev->reg_page != page) {
		/* set page number */
		v = (page << 8) | REG_MII_PAGE_ENABLE;
		ret = mdiobus_write_nested(bus, BRCM_PSEUDO_PHY_ADDR,
					   REG_MII_PAGE, v);
		if (ret)
			return ret;
		dev->reg_page = page;
	}

	/* set register address */
	v = (reg << 8) | op;
	ret = mdiobus_write_nested(bus, BRCM_PSEUDO_PHY_ADDR, REG_MII_ADDR, v);
	if (ret)
		return ret;

	/* check if operation completed */
	for (i = 0; i < 5; ++i) {
		v = mdiobus_read_nested(bus, BRCM_PSEUDO_PHY_ADDR,
					REG_MII_ADDR);
		if (!(v & (REG_MII_ADDR_WRITE | REG_MII_ADDR_READ)))
			break;
		usleep_range(10, 100);
	}

	if (WARN_ON(i == 5))
		return -EIO;

	return 0;
}

static int b53125_mdio_read8(struct b53_mdio_device *dev, u8 page, u8 reg, u8 *val)
{
	struct mii_bus *bus = dev->bus;
	int ret;

	ret = b53125_mdio_op(dev, page, reg, REG_MII_ADDR_READ);
	if (ret)
		return ret;

	*val = mdiobus_read_nested(bus, BRCM_PSEUDO_PHY_ADDR,
				   REG_MII_DATA0) & 0xff;

	return 0;
}

static int b53125_mdio_read16(struct b53_mdio_device *dev, u8 page, u8 reg, u16 *val)
{
	struct mii_bus *bus = dev->bus;
	int ret;

	ret = b53125_mdio_op(dev, page, reg, REG_MII_ADDR_READ);
	if (ret)
		return ret;

	*val = mdiobus_read_nested(bus, BRCM_PSEUDO_PHY_ADDR, REG_MII_DATA0);

	return 0;
}

static int b53125_mdio_read32(struct b53_mdio_device *dev, u8 page, u8 reg, u32 *val)
{
	struct mii_bus *bus = dev->bus;
	int ret;

	ret = b53125_mdio_op(dev, page, reg, REG_MII_ADDR_READ);
	if (ret)
		return ret;

	*val = mdiobus_read_nested(bus, BRCM_PSEUDO_PHY_ADDR, REG_MII_DATA0);
	*val |= mdiobus_read_nested(bus, BRCM_PSEUDO_PHY_ADDR,
				    REG_MII_DATA1) << 16;

	return 0;
}

static int b53125_mdio_read48(struct b53_mdio_device *dev, u8 page, u8 reg, u64 *val)
{
	struct mii_bus *bus = dev->bus;
	u64 temp = 0;
	int i;
	int ret;

	ret = b53125_mdio_op(dev, page, reg, REG_MII_ADDR_READ);
	if (ret)
		return ret;

	for (i = 2; i >= 0; i--) {
		temp <<= 16;
		temp |= mdiobus_read_nested(bus, BRCM_PSEUDO_PHY_ADDR,
				     REG_MII_DATA0 + i);
	}

	*val = temp;

	return 0;
}

static int b53125_mdio_read64(struct b53_mdio_device *dev, u8 page, u8 reg, u64 *val)
{
	struct mii_bus *bus = dev->bus;
	u64 temp = 0;
	int i;
	int ret;

	ret = b53125_mdio_op(dev, page, reg, REG_MII_ADDR_READ);
	if (ret)
		return ret;

	for (i = 3; i >= 0; i--) {
		temp <<= 16;
		temp |= mdiobus_read_nested(bus, BRCM_PSEUDO_PHY_ADDR,
					    REG_MII_DATA0 + i);
	}

	*val = temp;

	return 0;
}

static int b53125_mdio_write8(struct b53_mdio_device *dev, u8 page, u8 reg, u8 value)
{
	struct mii_bus *bus = dev->bus;
	int ret;

	ret = mdiobus_write_nested(bus, BRCM_PSEUDO_PHY_ADDR,
				   REG_MII_DATA0, value);
	if (ret)
		return ret;

	return b53125_mdio_op(dev, page, reg, REG_MII_ADDR_WRITE);
}

static int b53125_mdio_write16(struct b53_mdio_device *dev, u8 page, u8 reg,
			    u16 value)
{
	struct mii_bus *bus = dev->bus;
	int ret;

	ret = mdiobus_write_nested(bus, BRCM_PSEUDO_PHY_ADDR,
				   REG_MII_DATA0, value);
	if (ret)
		return ret;

	return b53125_mdio_op(dev, page, reg, REG_MII_ADDR_WRITE);
}

static int b53125_mdio_write32(struct b53_mdio_device *dev, u8 page, u8 reg,
			    u32 value)
{
	struct mii_bus *bus = dev->bus;
	unsigned int i;
	u32 temp = value;

	for (i = 0; i < 2; i++) {
		int ret = mdiobus_write_nested(bus, BRCM_PSEUDO_PHY_ADDR,
					       REG_MII_DATA0 + i,
					       temp & 0xffff);
		if (ret)
			return ret;
		temp >>= 16;
	}

	return b53125_mdio_op(dev, page, reg, REG_MII_ADDR_WRITE);
}

static int b53125_mdio_write48(struct b53_mdio_device *dev, u8 page, u8 reg,
			    u64 value)
{
	struct mii_bus *bus = dev->bus;
	unsigned int i;
	u64 temp = value;

	for (i = 0; i < 3; i++) {
		int ret = mdiobus_write_nested(bus, BRCM_PSEUDO_PHY_ADDR,
					       REG_MII_DATA0 + i,
					       temp & 0xffff);
		if (ret)
			return ret;
		temp >>= 16;
	}

	return b53125_mdio_op(dev, page, reg, REG_MII_ADDR_WRITE);
}

static int b53125_mdio_write64(struct b53_mdio_device *dev, u8 page, u8 reg,
			    u64 value)
{
	struct mii_bus *bus = dev->bus;
	unsigned int i;
	u64 temp = value;

	for (i = 0; i < 4; i++) {
		int ret = mdiobus_write_nested(bus, BRCM_PSEUDO_PHY_ADDR,
					       REG_MII_DATA0 + i,
					       temp & 0xffff);
		if (ret)
			return ret;
		temp >>= 16;
	}

	return b53125_mdio_op(dev, page, reg, REG_MII_ADDR_WRITE);
}

static int b53125_mdio_phy_read16(struct b53_mdio_device *dev, int addr, int reg,
			       u16 *value)
{
	struct mii_bus *bus = dev->bus;

	*value = mdiobus_read_nested(bus, addr, reg);

	return 0;
}

static int b53125_mdio_phy_write16(struct b53_mdio_device *dev, int addr, int reg,
				u16 value)
{
	struct mii_bus *bus = dev->bus;

	return mdiobus_write_nested(bus, addr, reg, value);
}

static const struct b53_mdio_ops b53125_mdio_ops = {
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
	.phy_read16 = b53125_mdio_phy_read16,
	.phy_write16 = b53125_mdio_phy_write16,
};


static struct mdio_device * b53mdio_device_get(char *name)
{
	struct mdio_device *mdiodev = NULL;
	struct device *d;
	int rc;

	d = bus_find_device_by_name(&mdio_bus_type, NULL, name);
	if (!d) {
		printk(KERN_ERR " Mdio Device %s not found\n", name);
		return mdiodev;
	}
	mdiodev = to_mdio_device(d);
	return mdiodev;
}

int b53125_mdio_probe(struct b53_mdio_device *mdio)
{
	struct mdio_device * mdiodev = b53mdio_device_get("stmmac-0:1e");
	if(mdiodev)
	{
		mdio->bus = mdiodev->bus;
		mdio->reg_page = 0xff;
		mdio->ops = &b53125_mdio_ops;
		return 0;
	}
	return -1;
}
#endif