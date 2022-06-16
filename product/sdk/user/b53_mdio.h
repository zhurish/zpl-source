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
struct mido_device
{
	int pape;
	int regnum;
	union 
	{
		u8 val[8];
		u8 val8;
		u16 val16;
		u32 val32;
		u64 val48;
		u64 val64;
	}val;
};

struct b53mac_cmd
{
	u16 port;
	u16 vrfid;
	u16 vlan;
	u8 mac[6];
	u16 pri:13;
	u16 is_valid:1;
	u16 is_age:1;
	u16 is_static:1;
};

struct b53mac_data
{
	u16 cnt;
	struct b53mac_cmd macdata[4096];
};

struct b53vlan_cmd
{
	u16 subcmd;
	u16 port;
	u16 vrfid;
	u16 vlan;
	u16 tag;
};
#pragma pack(0)

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
#define B53_IO	's'

#define B53_IO_W8	_IOW(B53_IO, 0x10, struct mido_device)
#define B53_IO_R8	_IOR(B53_IO, 0x11, struct mido_device)
#define B53_IO_W16	_IOW(B53_IO, 0x12, struct mido_device)
#define B53_IO_R16	_IOR(B53_IO, 0x13, struct mido_device)
#define B53_IO_W32	_IOW(B53_IO, 0x14, struct mido_device)
#define B53_IO_R32	_IOR(B53_IO, 0x15, struct mido_device)
#define B53_IO_W48	_IOW(B53_IO, 0x16, struct mido_device)
#define B53_IO_R48	_IOR(B53_IO, 0x17, struct mido_device)
#define B53_IO_W64	_IOW(B53_IO, 0x18, struct mido_device)
#define B53_IO_R64	_IOR(B53_IO, 0x19, struct mido_device)

#define B53_IO_WMAC	_IOWR(B53_IO, 0x05, struct b53mac_cmd)
#define B53_IO_RMAC	 _IOWR(B53_IO, 0x06, struct b53mac_cmd)
#define B53_IO_MACDUMP _IOWR(B53_IO, 0x07, struct b53mac_cmd)
#define B53_IO_MACCNT _IOWR(B53_IO, 0x08, struct b53mac_cmd)

#define B53_IO_VLAN	_IOW(B53_IO, 0x50, struct b53vlan_cmd)
/*********************************************************************************/
/*********************************************************************************/
int b53125_mdio_read(struct b53_mdio_device *dev, u8 page, u8 reg);
#endif

extern int b53125_mdio_probe(struct b53_mdio_device *mdio);

#ifdef __cplusplus
}
#endif

#endif /* __B53_MDIO_H__ */
