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


#if 1
int __mdio_read(struct b53_mdio_device *dev, int addr, u32 regnum)
{
	struct mido_device data;
	data.addr = addr;
	data.regnum = regnum;
	if(ioctl(dev->fd, B53_IO_R, &data) == 0)
		return data.value;
	printf("==============%s\r\n", strerror(ipstack_errno));
/*	if(read(dev->fd, &data, sizeof(struct mido_data_b53)) > 0)
		return data.value;*/
	return 0;
}

static int __mdio_write(struct b53_mdio_device *dev, int addr, u32 regnum, u16 val)
{
	struct mido_device data;
	data.addr = addr;
	data.regnum = regnum;
	data.value = val;
	if(ioctl(dev->fd, B53_IO_W, &data) == 0)
		return 0;
	printf("==============%s\r\n", strerror(ipstack_errno));
/*	if(write(dev->fd, &data, sizeof(struct mido_data_b53)) > 0)
		return 0;*/
	return 1;
}
#else
static int __mdio_read(struct b53_mdio_device *dev, int addr, u32 regnum)
{
	struct ipstack_ifreq ifr;
	struct mii_ioctl_data mii_val;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name)-1);
	ifr.ifr_data = &mii_val;
	if(ioctl(dev->fd, SIOCGMIIPHY, &ifr) == -1)
	{
		printf("==============%s\r\n", strerror(ipstack_errno));
		return -1;
	}
	//mii_val->phy_id,
	printf("==============%x\r\n", mii_val.phy_id);
	mii_val.reg_num = regnum;
	if(ioctl(dev->fd, SIOCGMIIREG, &ifr) == -1)
		return 0;
	return mii_val.val_out;
}

static int __mdio_write(struct b53_mdio_device *dev, int addr, u32 regnum, u16 val)
{
	struct ipstack_ifreq ifr;
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


static int b53125_mdio_op(struct b53_mdio_device *dev, u8 page, u8 reg, u16 op)
{
	zpl_uint32 i;
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
	for (i = 0; i < 1000; ++i) {
		v = __mdio_read(dev, BRCM_PSEUDO_PHY_ADDR,
					REG_MII_ADDR);
		if (!(v & (REG_MII_ADDR_WRITE | REG_MII_ADDR_READ)))
			break;
		os_usleep(1);
	}

	if ((i == 1000))
		return ERROR;
	return OK;
}

static int b53125_mdio_read8(struct b53_mdio_device *dev, u8 page, u8 reg, u8 *val)
{
	int ret;

	ret = b53125_mdio_op(dev, page, reg, REG_MII_ADDR_READ);
	if (ret == ERROR)
		return ret;

	*val = __mdio_read(dev, BRCM_PSEUDO_PHY_ADDR,
				   REG_MII_DATA0) & 0xff;

	return 0;
}

static int b53125_mdio_read16(struct b53_mdio_device *dev, u8 page, u8 reg, u16 *val)
{
	int ret;

	ret = b53125_mdio_op(dev, page, reg, REG_MII_ADDR_READ);
	if (ret == ERROR)
		return ret;

	*val = __mdio_read(dev, BRCM_PSEUDO_PHY_ADDR, REG_MII_DATA0);

	return 0;
}

static int b53125_mdio_read32(struct b53_mdio_device *dev, u8 page, u8 reg, u32 *val)
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

static int b53125_mdio_read48(struct b53_mdio_device *dev, u8 page, u8 reg, u64 *val)
{
	u64 temp = 0;
	zpl_uint32 i;
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

static int b53125_mdio_read64(struct b53_mdio_device *dev, u8 page, u8 reg, u64 *val)
{
	u64 temp = 0;
	zpl_uint32 i;
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

static int b53125_mdio_write8(struct b53_mdio_device *dev, u8 page, u8 reg, u8 value)
{
	int ret;

	ret = __mdio_write(dev, BRCM_PSEUDO_PHY_ADDR,
				   REG_MII_DATA0, value);
	if (ret)
		return ret;

	return b53125_mdio_op(dev, page, reg, REG_MII_ADDR_WRITE);
}

static int b53125_mdio_write16(struct b53_mdio_device *dev, u8 page, u8 reg,
			    u16 value)
{
	int ret;

	ret = __mdio_write(dev, BRCM_PSEUDO_PHY_ADDR,
				   REG_MII_DATA0, value);
	if (ret)
		return ret;

	return b53125_mdio_op(dev, page, reg, REG_MII_ADDR_WRITE);
}

static int b53125_mdio_write32(struct b53_mdio_device *dev, u8 page, u8 reg,
			    u32 value)
{
	zpl_uint32 i;
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

static int b53125_mdio_write48(struct b53_mdio_device *dev, u8 page, u8 reg,
			    u64 value)
{
	zpl_uint32 i;
	u64 temp = value;
_sdk_err( " b53125_mdio_write48");
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

static int b53125_mdio_write64(struct b53_mdio_device *dev, u8 page, u8 reg,
			    u64 value)
{
	zpl_uint32 i;
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
