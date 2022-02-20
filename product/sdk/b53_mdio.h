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

#include "zpl_include.h"
#include "sdk_driver.h"


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
};


struct b53_mdio_device
{
	int fd;
	u8 reg_page;

	struct b53_mdio_ops *ops;
};

#pragma pack(1)
struct mido_device
{
	int addr;
	int regnum;
	u16 value;
};

struct b53mac_cmd
{
	int port;
	int vrfid;
	u16 vlan;
	u8 mac[6];
	int pri;
};

struct b53vlan_cmd
{
	int port;
	int vrfid;
	u16 vlan;
};
#pragma pack(0)

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
#define B53_IO	's'
#define B53_IO_CTL	_IO(B53_IO, 0x01)
#define B53_IO_W	_IOW(B53_IO, 0x02, struct mido_device)
#define B53_IO_R	_IOR(B53_IO, 0x03, struct mido_device)
#define B53_IO_WR	_IOWR(B53_IO, 0x04, struct mido_device)
#define B53_IO_WMAC	_IOWR(B53_IO, 0x05, struct b53mac_cmd)
#define B53_IO_RMAC	 _IOWR(B53_IO, 0x06, struct b53mac_cmd)
#define B53_IO_MACDUMP _IOWR(B53_IO, 0x07, struct b53mac_cmd)
/*********************************************************************************/
/*********************************************************************************/
int __mdio_read(struct b53_mdio_device *dev, int addr, u32 regnum);

int b53125_mdio_probe(struct b53_mdio_device *mdio);

#ifdef __cplusplus
}
#endif

#endif /* __B53_MDIO_H__ */
