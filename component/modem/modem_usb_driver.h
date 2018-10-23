/*
 * modem_usb_driver.h
 *
 *  Created on: Jul 26, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_USB_DRIVER_H__
#define __MODEM_USB_DRIVER_H__


#define MODEM_USB_DRIVER_MAX		256

#define USB_KERNEL_UEVENT


#ifndef USB_KERNEL_UEVENT
#define MODEM_USB_DRIVER_TTL		5
#endif


#define MODEM_USB_DRIVER_ACTIVE		1
#define MODEM_USB_DRIVER_INACTIVE	0

//#define MODEM_USB_SERIAL_BASE	"/sys/bus/usb-serial/drivers/option1/"
#define MODEM_SYS_USB_SERIAL_BASE	"/sys/bus/usb-serial/devices"
#define MODEM_PROC_USB_SERIAL_BASE	"/proc/tty/driver/usbserial"

#define MODEM_SYS_USB_NET_BASE	"/sys/class/net"

/*
* /proc/tty/driver/usbserial
*/

#define USB_SERIAL_NAME_MAX		64

#define USB_KEY_CHANNEL_MAX		16


typedef struct tty_devname_s
{
	char		devname[USB_SERIAL_NAME_MAX];
	unsigned char flag;
}tty_devname;

typedef struct modem_usb_driver_s
{
	int			bus;
	int			device;
	int			vendor;
	int			product;
	int			active;
	tty_devname devname[TTY_USB_MAX];
	int			devcnt;
	char		netdevname[USB_SERIAL_NAME_MAX];
	char		usbkey[USB_SERIAL_NAME_MAX];
	u_int8		hw_channel;					//1-16
	enum		{ USB_DRIVER_NONE, USB_DRIVER_ACTIVE, USB_DRIVER_LOAD } flag;
#ifdef USB_KERNEL_UEVENT
	enum		{ USB_EVENT_NONE, USB_EVENT_ADD, USB_EVENT_DEL } change;
	int			commit;
#else
	int			ttl;
#endif
}modem_usb_driver;


extern int modem_usb_driver_init(void);
extern int modem_usb_driver_exit(void);
extern int modem_usb_driver_lookup(modem_usb_driver *driver);
extern int modem_usb_driver_add(modem_usb_driver *driver);
extern int modem_usb_driver_del(modem_usb_driver *driver);

extern int modem_usb_driver_hardware_channel(int vendor, int product);
//extern int modem_usb_driver_detection(modem_usb_driver *driver);

extern int show_modem_usb_driver(struct vty *vty);

extern int modem_product_detection(void);


//#define __MODEM_DR_DEBUG

#ifdef __MODEM_DR_DEBUG
#define MODEM_DR_DEBUG(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define MODEM_DR_WARN(fmt,...)		modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define MODEM_DR_ERROR(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#else
#define MODEM_DR_DEBUG(fmt,...)
#define MODEM_DR_WARN(fmt,...)
#define MODEM_DR_ERROR(fmt,...)
#endif


#endif /* __MODEM_USB_DRIVER_H__ */
