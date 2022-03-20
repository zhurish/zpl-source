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
#include "b53_driver.h"
#include "b53_mdio.h"
#include "b53_regs.h"

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
