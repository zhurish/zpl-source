/*
 * b53_mdio.h
 *
 *  Created on: May 1, 2019
 *      Author: zhurish
 */

#ifndef __B53_MDIO_H__
#define __B53_MDIO_H__

#ifdef	__cplusplus
extern "C" {
#endif
#ifdef ZPL_SDK_USER
#include "zplos_include.h"
#include "sdk_driver.h"
#else
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/phy.h>
#include <linux/etherdevice.h>
#include <net/dsa.h>
#endif

/*********************************************************************************/
/*********************************************************************************/
/* MII registers */
#define REG_MII_PAGE    0x10    /* MII Page register */
#define REG_MII_ADDR    0x11    /* MII Address register */
#define REG_MII_DATA0   0x18    /* MII Data register 0 */
#define REG_MII_DATA1   0x19    /* MII Data register 1 */
#define REG_MII_DATA2   0x1a    /* MII Data register 2 */
#define REG_MII_DATA3   0x1b    /* MII Data register 3 */

#define REG_MII_PAGE_ENABLE     BIT(0)
#define REG_MII_ADDR_WRITE      BIT(0)
#define REG_MII_ADDR_READ       BIT(1)

/*********************************************************************************/
#define BRCM_PSEUDO_PHY_ADDR           30

struct b53_mdio_device;

struct b53_mdio_ops {
	int (*read8)(struct b53_mdio_device *dev, u8 page, u8 reg, u8 *value);
	int (*read16)(struct b53_mdio_device *dev, u8 page, u8 reg, u16 *value);
	int (*read32)(struct b53_mdio_device *dev, u8 page, u8 reg, u32 *value);
	int (*read48)(struct b53_mdio_device *dev, u8 page, u8 reg, u64 *value);
	int (*read64)(struct b53_mdio_device *dev, u8 page, u8 reg, u64 *value);
	int (*write8)(struct b53_mdio_device *dev, u8 page, u8 reg, u8 value);
	int (*write16)(struct b53_mdio_device *dev, u8 page, u8 reg, u16 value);
	int (*write32)(struct b53_mdio_device *dev, u8 page, u8 reg, u32 value);
	int (*write48)(struct b53_mdio_device *dev, u8 page, u8 reg, u64 value);
	int (*write64)(struct b53_mdio_device *dev, u8 page, u8 reg, u64 value);
	int (*phy_read16)(struct b53_mdio_device *dev, int addr, int reg, u16 *value);
	int (*phy_write16)(struct b53_mdio_device *dev, int addr, int reg, u16 value);
};

#ifdef ZPL_SDK_USER
struct b53_mdio_device
{
	int fd;
	u8 reg_page;
	struct b53_mdio_ops *ops;
};
#else
struct b53_mdio_device
{
	u8 reg_page;
	struct mii_bus *bus;
	struct b53_mdio_ops *ops;
};
#endif
#ifdef ZPL_SDK_USER
#pragma pack(1)
struct midodev_cmd
{
	int addr;
	u32 regnum;
	u16 val;
};
#pragma pack(0)

/*********************************************************************************/
/*********************************************************************************/
#define MDIODEV_IO	's'

#define B53_IO_W	_IOW(MDIODEV_IO, 0x10, struct midodev_cmd)
#define B53_IO_R	_IOR(MDIODEV_IO, 0x11, struct midodev_cmd)
/*********************************************************************************/
/*********************************************************************************/
int b53125_mdio_read(struct b53_mdio_device *dev, u8 addr, u32 regnum);
#endif

extern int b53125_mdio_probe(struct b53_mdio_device *mdio);

#ifdef __cplusplus
}
#endif

#endif /* __B53_MDIO_H__ */
