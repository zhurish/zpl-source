/*
 * mdiodev.h
 *
 *  Created on: May 1, 2019
 *      Author: zhurish
 */

#ifndef __MDIODEV_H__
#define __MDIODEV_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/phy.h>
#include <linux/etherdevice.h>
#include <net/dsa.h>

#define MDIO_DEVICE_NAME	"mdiodev"
#define MDIO_MODULE_NAME 	"mdiodev"


struct mdiodev
{
	dev_t dev_num;
	struct class *class;
	struct mii_bus *bus;
	struct device *dev;
};
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
/*********************************************************************************/
#define MDIODEV_IO	's'

#define B53_IO_W	_IOW(MDIODEV_IO, 0x10, struct midodev_cmd)
#define B53_IO_R	_IOR(MDIODEV_IO, 0x11, struct midodev_cmd)

/*********************************************************************************/
/*********************************************************************************/




#ifdef __cplusplus
}
#endif

#endif /* __MDIODEV_H__ */
