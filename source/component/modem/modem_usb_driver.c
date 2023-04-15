/*
 * modem_usb_driver.c
 *
 *  Created on: Jul 26, 2018
 *      Author: zhurish
 */



#include "auto_include.h"
#include <zplos_include.h>
#include "zmemory.h"
#include "vty.h"
#include "command.h"
#include "if.h"

#include "str.h"
#include "nsm_include.h"
#include <dirent.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "modem.h"
#include "modem_event.h"
#include "modem_client.h"
#include "modem_dialog.h"
#include "modem_attty.h"
#include "modem_serial.h"
#include "modem_usb_driver.h"


#ifdef USB_KERNEL_UEVENT
#define UEVENT_BUFFER_SIZE 2048
static int usb_event_socket_handle(zpl_uint32 timeout);
#endif

static modem_usb_driver musb_driver[MODEM_USB_DRIVER_MAX];
static int usb_driver_startup_init = 0;

static const char *usbkey_channel[USB_KEY_CHANNEL_MAX] =
{
	"none",
	"2-1:1",
	"2-1:2",
	"2-1:3",
	"2-1:4",
	"2-2:1",
	"2-2:2",
	"2-2:3",
	"2-2:4",
	"2-3:1",
	"2-3:2",
	"2-3:3",
	"2-3:4",
	"2-4:1",
	"2-4:2",
	"none",
};

int modem_usb_driver_init(void)
{
	os_memset(musb_driver, 0, sizeof(musb_driver));
	return OK;
}

int modem_usb_driver_exit(void)
{
	os_memset(musb_driver, 0, sizeof(musb_driver));
	return OK;
}


static int modem_usb_key_channel(char *usbkey)
{
	zpl_uint32 i = 0;
	for(i = 0; i < USB_KEY_CHANNEL_MAX; i++)
	{
		if(strstr(usbkey, usbkey_channel[i]))
			return i;
	}
	return 0;
}


static int modem_usb_driver_get_empty(void)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODEM_USB_DRIVER_MAX; i++)
	{
		if(musb_driver[i].flag == USB_DRIVER_NONE)
			return i;
	}
	return -1;
}

int modem_usb_driver_lookup(modem_usb_driver *driver)
{
	zpl_uint32 i = 0;
	assert(driver);
	for(i = 0; i < MODEM_USB_DRIVER_MAX; i++)
	{
		if(musb_driver[i].flag > USB_DRIVER_NONE)
		{
			if(musb_driver[i].vendor == driver->vendor &&
				musb_driver[i].product == driver->product)
			{
				//MODEM_DR_DEBUG("%s: %x:%x", __func__,driver->vendor,driver->product);
				return i;
			}
		}
	}
	return ERROR;
}

int modem_usb_driver_add(modem_usb_driver *driver)
{
	zpl_int32 i = 0;
	assert(driver);
	if(modem_usb_driver_lookup(driver) != ERROR)
	{
#ifndef USB_KERNEL_UEVENT
		i = modem_usb_driver_lookup(driver);
		musb_driver[i].ttl = MODEM_USB_DRIVER_TTL;
#endif
		return ERROR;
	}
	if(!modem_driver_lookup(driver->vendor, driver->product))
	{
		if(MODEM_IS_DEBUG(DRIVER))
			zlog_warn(MODULE_MODEM, "not support this modem ID:%x %x ",
				driver->vendor, driver->product);
		return ERROR;
	}
	i = modem_usb_driver_get_empty();
	if(i >= 0)
	{
		MODEM_DR_DEBUG("%s: %x:%x", __func__,driver->vendor,driver->product);
		musb_driver[i].bus = driver->bus;
		musb_driver[i].device = driver->device;
		musb_driver[i].vendor = driver->vendor;
		musb_driver[i].product = driver->product;
#ifndef USB_KERNEL_UEVENT
		musb_driver[i].ttl = MODEM_USB_DRIVER_TTL;
#endif
		musb_driver[i].active = MODEM_USB_DRIVER_ACTIVE;
		musb_driver[i].flag = USB_DRIVER_ACTIVE;

		os_memcpy(musb_driver[i].netdevname, driver->netdevname, sizeof(musb_driver[i].netdevname));
		os_memcpy(musb_driver[i].usbkey, driver->usbkey, sizeof(musb_driver[i].usbkey));
		os_memcpy(musb_driver[i].devname, driver->devname, sizeof(driver->devname));

		musb_driver[i].hw_channel = modem_usb_key_channel(musb_driver[i].usbkey);
		musb_driver[i].devcnt = driver->devcnt;
#ifdef USB_KERNEL_UEVENT
		musb_driver[i].change = driver->change;
		musb_driver[i].commit = driver->commit;
#endif
		return OK;
	}
	return ERROR;
}

int modem_usb_driver_del(modem_usb_driver *driver)
{
	assert(driver);
	zpl_int32 i = modem_usb_driver_lookup(driver);
	if(i == ERROR)
		return -1;
	if(i >= 0)
	{
		MODEM_DR_DEBUG("%s: %x:%x", __func__,driver->vendor,driver->product);
		modem_driver_remove(driver->vendor,driver->product);

		musb_driver[i].bus = 0;
		musb_driver[i].device = 0;
		musb_driver[i].vendor = 0;
		musb_driver[i].product = 0;
		musb_driver[i].flag = USB_DRIVER_NONE;
		musb_driver[i].active = MODEM_USB_DRIVER_INACTIVE;
		musb_driver[i].hw_channel = 0;
		os_memset(musb_driver[i].netdevname, 0, sizeof(musb_driver[i].netdevname));
		os_memset(musb_driver[i].usbkey, 0, sizeof(musb_driver[i].usbkey));
#ifndef USB_KERNEL_UEVENT
		musb_driver[i].ttl = 0;
#endif
#ifdef USB_KERNEL_UEVENT
		musb_driver[i].change = USB_EVENT_NONE;
		musb_driver[i].commit = 0;
#endif
		musb_driver[i].devcnt = 0;
		os_memset(musb_driver[i].devname, 0, sizeof(musb_driver[i].devname));
		os_memset(&musb_driver[i], 0, sizeof(musb_driver[i]));
		return OK;
	}
	return ERROR;
}


int modem_usb_driver_hardware_channel(zpl_uint32 vendor, zpl_uint32 product)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODEM_USB_DRIVER_MAX; i++)
	{
		if(musb_driver[i].flag > USB_DRIVER_NONE)
		{
			if(musb_driver[i].vendor == vendor &&
				musb_driver[i].product == product &&
				musb_driver[i].hw_channel)
			{
				//MODEM_DR_DEBUG("%s: %x:%x", __func__,driver->vendor,driver->product);
				return musb_driver[i].hw_channel;
			}
		}
	}
	return 0;
}


static int modem_usb_driver_lookup_ttyusb(modem_usb_driver *driver, char *devname)
{
	zpl_uint32 j = 0;
	tty_devname tmp;
	os_memset(&tmp, 0, sizeof(tmp));
	os_strcpy(tmp.devname, "/dev/");
	os_strcat(tmp.devname, devname);
	tmp.flag = 1;
	for(j = 0; j < TTY_USB_MAX; j++)
	{
		 //if(!strisempty(&driver->devname[j], sizeof(driver->devname[j])))
		 if((driver->devname[j].flag))
		 {
			 if(os_strncmp(driver->devname[j].devname, tmp.devname, sizeof(tmp.devname)) == 0)
			 {
				 MODEM_DR_DEBUG("lookup tty:%s\n", driver->devname[j].devname, tmp.devname);
				 return 1;
			 }
		 }
	}
	return 0;
}

static int modem_usb_driver_ttyusb_sort(tty_devname *arr, zpl_uint32 sz)
{
    zpl_uint32 i = 0;
    zpl_uint32 j = 0;
    tty_devname tmp;
    assert(arr);
    for(i = 0; i < sz - 1; i++)
    {
        for(j = 0; j < sz - i - 1; j++)
        {
            //if(arr[j]>arr[j+1])
            if(os_strcmp(arr[j].devname, arr[j+1].devname) > 0)
            {
				os_memset(&tmp, 0, sizeof(tmp));
				os_memcpy(&tmp, &arr[j], sizeof(tmp));

                os_memcpy(&arr[j], &arr[j+1], sizeof(tmp));

				os_memset(&arr[j+1], 0, sizeof(tmp));
				os_memcpy(&arr[j+1], &tmp, sizeof(tmp));
            }
        }
    }
    return OK;
}

static int modem_usb_driver_add_ttyusb(modem_usb_driver *driver, char *devname)
{
	zpl_uint32 j = 0;
	if(modem_usb_driver_lookup_ttyusb(driver, devname))
		return 0;
	for(j = 0; j < TTY_USB_MAX; j++)
	{
		 if((driver->devname[j].flag == 0))
		 {
			 os_strcpy(driver->devname[j].devname, "/dev/");
			 os_strcat(driver->devname[j].devname, devname);
			 //os_strcpy(driver->devname[j].devname, devname);
			 driver->devname[j].flag = 1;
			 driver->devcnt ++;
			 MODEM_DR_DEBUG("add tty %d: %s\n", j, driver->devname[j].devname);
			 break;
		 }
	}
	modem_usb_driver_ttyusb_sort(driver->devname, driver->devcnt);
	return OK;
}


static int modem_usb_driver_usbkey_lookup(char *usbkey)
{
	zpl_uint32 i = 0;
	assert(usbkey);
	char key[USB_SERIAL_NAME_MAX];
	os_memset(key, 0, sizeof(key));
	os_strcpy(key, usbkey);
	for(i = 0; i < MODEM_USB_DRIVER_MAX; i++)
	{
		if(musb_driver[i].flag > USB_DRIVER_NONE)
		{
			 if(os_strncmp(musb_driver[i].usbkey, key, sizeof(key)) == 0)
				 return i;
		}
	}
	return ERROR;
}


int show_modem_usb_driver(struct vty *vty)
{
	zpl_uint32 i = 0, j;
	assert(vty);
	for(i = 0; i < MODEM_USB_DRIVER_MAX; i++)
	{
		if(musb_driver[i].active == MODEM_USB_DRIVER_ACTIVE)
		{
			vty_out(vty, " bus		:%x%s", musb_driver[i].bus, VTY_NEWLINE);
			vty_out(vty, " device		:%x%s", musb_driver[i].device, VTY_NEWLINE);
			vty_out(vty, " vendor		:%x%s", musb_driver[i].vendor, VTY_NEWLINE);
			vty_out(vty, " product	:%x%s", musb_driver[i].product, VTY_NEWLINE);

			vty_out(vty, " netdevname	:%s%s", os_strlen(musb_driver[i].netdevname) ?
					musb_driver[i].netdevname : "NULL", VTY_NEWLINE);
			vty_out(vty, " usbkey		:%s%s", os_strlen(musb_driver[i].usbkey) ?
					musb_driver[i].usbkey : "NULL", VTY_NEWLINE);

			vty_out(vty, " TTY	:%s", VTY_NEWLINE);
			for(j = 0; j < TTY_USB_MAX; j++)
			{
				if(musb_driver[i].devname[j].flag)
					vty_out(vty, " 	:%s%s", musb_driver[i].devname[j].devname, VTY_NEWLINE);
			}
		}
	}
	return ERROR;
}

int show_modem_usb_key_driver(struct vty *vty)
{
	zpl_uint32 i = 0;
	assert(vty);
	for(i = 0; i < USB_KEY_CHANNEL_MAX; i++)
	{
		if(i == 0)
			vty_out(vty, " USB HW Channel Information:%s", VTY_NEWLINE);
		vty_out(vty, " HW Channel    %d : %s%s", i, usbkey_channel[i], VTY_NEWLINE);
		//vty_out(vty, " TTY	:%s", VTY_NEWLINE);
	}
	return OK;
}

static int modem_usb_driver_tty_probe(modem_usb_driver *driver)
{
	zpl_uint32 i = 0;
	assert(driver);
	char *devname[TTY_USB_MAX];
	modem_driver_t *modem_driver = modem_driver_lookup(driver->vendor, driver->product);
	if(!modem_driver)
	{
		return ERROR;
	}

	for(i = 0; i < TTY_USB_MAX; i++)
	{
		devname[i] = driver->devname[i].devname;
	}
	if(os_strlen(driver->netdevname))
	{
		os_memset(modem_driver->eth_name, 0, sizeof(modem_driver->eth_name));
		os_strcpy(modem_driver->eth_name, driver->netdevname);
	}
	modem_driver_tty_probe(modem_driver, devname);
	return OK;
}



static int modem_usb_driver_commit(void)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODEM_USB_DRIVER_MAX; i++)
	{
		if(musb_driver[i].flag == USB_DRIVER_ACTIVE)
		{
			if(os_strlen(musb_driver[i].usbkey))
			{
				musb_driver[i].hw_channel = modem_usb_key_channel(musb_driver[i].usbkey);
				MODEM_DR_DEBUG("usbkey %s set hw channel:%d",
						musb_driver[i].usbkey, musb_driver[i].hw_channel);
			}
			modem_usb_driver_tty_probe(&musb_driver[i]);

			if(modem_driver_inster(musb_driver[i].vendor, musb_driver[i].product) == OK)
				musb_driver[i].flag = USB_DRIVER_LOAD;
		}
	}
	return OK;
}

#ifndef USB_KERNEL_UEVENT
static int modem_usb_driver_update_ttl(void)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODEM_USB_DRIVER_MAX; i++)
	{
		if(musb_driver[i].active == MODEM_USB_DRIVER_ACTIVE)
		{
			musb_driver[i].ttl -= 1;
		}
	}
	for(i = 0; i < MODEM_USB_DRIVER_MAX; i++)
	{
		if(musb_driver[i].active == MODEM_USB_DRIVER_ACTIVE &&
				musb_driver[i].ttl <= 0)
		{
			modem_usb_driver_del(&musb_driver[i]);
		}
	}
	return OK;
}
#endif
/*
 * /proc/tty/driver/usbserial
 */
static int modem_proc_usb_driver_devname_update(char * input)
{
	zpl_uint32 i = 0;//, j = 0;
	assert(input);
	char devname0[USB_SERIAL_NAME_MAX];
	modem_driver_t *modem_driver = NULL;
	modem_usb_driver driver;
	os_memset(devname0, 0, sizeof(devname0));
	os_memset(&driver, 0, sizeof(driver));
	if(os_strlen(input) >=
			os_strlen("usbserinfo:1.0 driver:2.0"))
	{
		char *vendorstr, *productstr;
		//char tmp[64];
		vendorstr = os_strstr(input, "vendor");
		productstr = os_strstr(input, "product");
		if(vendorstr && productstr)
		{
			vendorstr += strlen("vendor:");
			productstr += strlen("product:");
			driver.vendor = strtol(vendorstr, NULL, 16);
			driver.product = strtol(productstr, NULL, 16);
		}
	}
	i = modem_usb_driver_lookup(&driver);
	if(i == ERROR)
	{
		 return ERROR;
	}
	modem_driver = modem_driver_lookup(driver.vendor, driver.product);
	if(modem_driver)
	{
		os_memset(devname0, 0, sizeof(devname0));
		switch (modem_driver->tty_type)
		{
		case TTY_USB:
			snprintf(devname0, sizeof(devname0), "%s%d", USB_SERIAL_BASE,
					atoi(input));
			break;
		case TTY_CAM:
			snprintf(devname0, sizeof(devname0), "%s%d", CAM_SERIAL_BASE,
					atoi(input));
			break;
		case TTY_S:
			snprintf(devname0, sizeof(devname0), "%s%d", UART_SERIAL_BASE,
					atoi(input));
			break;
		default:
			break;
		}
		if(os_strlen(devname0))
		{
			//MODEM_DR_DEBUG("%s:%s",__func__, devname0);
			modem_usb_driver_add_ttyusb(&driver, devname0);
		}
	}
	return OK;
}




static int modem_proc_usb_driver_product(char * input, modem_usb_driver *driver)
{
	assert(driver);
	assert(input);
	if(os_strlen(input) >=
			os_strlen("usbserinfo:1.0 driver:2.0"))
	{
		char *vendor, *product;//, tmp[64];
		vendor = os_strstr(input, "vendor");
		product = os_strstr(input, "product");
		if(vendor && product)
		{
			vendor += os_strlen("vendor:");
			product += os_strlen("product:");
			driver->vendor = strtol(vendor, NULL, 16);
			driver->product = strtol(product, NULL, 16);
			//product++;
			//sscanf(vendor, "%*s:%x", tmp, &driver->vendor);
			//sscanf(product, "%*s:%x",tmp, &driver->product);
		}
	}
	if(driver->vendor && driver->product)
	{
		//MODEM_DR_DEBUG("%s:%x %x",__func__, driver->vendor , driver->product);
		return OK;
	}
	return ERROR;
}

static int modem_sys_usb_detection_devname(modem_usb_driver *driver);

static int modem_proc_product_load(void)
{
	FILE *fp = NULL;
	char buf[1024];
	modem_usb_driver driver;
	os_memset(buf, 0, sizeof(buf));
	fp = fopen(MODEM_PROC_USB_SERIAL_BASE, "r");
	if(fp)
	{
		while(fgets(buf, sizeof(buf), fp))
		{
			os_memset(&driver, 0, sizeof(driver));
			if(modem_proc_usb_driver_product(buf, &driver) == OK)
				modem_usb_driver_add(&driver);

			modem_proc_usb_driver_devname_update(buf);
		}
		fclose(fp);
	}
	else
	{
		MODEM_DR_DEBUG("%s:%s",__func__, ipstack_strerror(ipstack_errno));
	}
	modem_sys_usb_detection_devname(&driver);
	modem_usb_driver_commit();

	return OK;
}



/*
 * /sys/bus/usb-serial/drivers
 *
 */
static int modem_sys_usb_detection_netdevname(modem_usb_driver *driver, char *usbkey)
{
	DIR *dir;
	struct dirent *d;
	char fullpath[256];
	char path[256];
	os_memset(path, 0, sizeof(path));
	os_memset(fullpath, 0, sizeof(fullpath));
	dir = opendir(MODEM_SYS_USB_NET_BASE);
	if (!dir)
	{
		printf("cannot open %s", MODEM_SYS_USB_NET_BASE);
		return -1;
	}
	/* Walk through the directory. */
	while ((d = readdir(dir)) != NULL)
	{

		{
			os_memset(path, 0, sizeof(path));
			os_memset(fullpath, 0, sizeof(fullpath));
			os_snprintf(path, sizeof(path), "%s/%s", MODEM_SYS_USB_NET_BASE,
					d->d_name);
			if (realpath(path, fullpath))
			{
				if ((os_strstr(fullpath, usbkey)) != 0)
				{
					os_strcpy(driver->netdevname, d->d_name);
					MODEM_DR_DEBUG("add netdevname: %s\n", driver->netdevname);
					break;
				}
			}
		}
	}
	closedir(dir);
	return OK;
}

//: /sys/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.0/ttyUSB0
//: /sys/devices/pci0000:00/0000:00:14.0/usb2/2-2/2-2:1.0/ttyUSB0
static int modem_usb_key_detection(char *input, char *output)
{
	zpl_uint32 offset = 0;
	if(!input || !os_strlen(input))
		return ERROR;
	char *brk = os_strstr(input, "usb");
	if(brk)
	{
		//offset = brk - input;
		// usb2/2-2/2-2:1.0/ttyUSB0
		//MODEM_DR_DEBUG("add usb key0: %s\n", input);
		//MODEM_DR_DEBUG("add usb key0: %s\n", brk);
		offset = strccntlast(brk, '/', 2);

		if (os_strlen(brk) > (offset + 4))
		{
			brk = brk + offset + 1;
			// 2-2:1.0/ttyUSB0
			if(brk)
			{
				//MODEM_DR_DEBUG("add usb key1: %s\n", brk);
				offset = strccntlast(brk, '.', 1);
				// .0/ttyUSB0
				if(output)
				{
					os_strncpy(output, brk, offset);
					// devices/pci0000:00/0000:00:14
					//MODEM_DR_DEBUG("add usb key: %s\n", output);
				}
				return OK;
			}
			//MODEM_DR_DEBUG("%s\n", musb_driver[i].usbkey);
		}
	}
	return ERROR;
}

static int modem_sys_usb_detection_devname(modem_usb_driver *driver)
{
	DIR *dir;
	zpl_uint32 i = 0;
	struct dirent *d;
	zpl_uint32 offset = 0;
	char fullpath[256];
	char path[256];
	char path1[512];
	char vender[128];
	os_memset(path, 0, sizeof(path));
	os_memset(fullpath, 0, sizeof(fullpath));
	dir = opendir(MODEM_SYS_USB_SERIAL_BASE);
	if (!dir)
	{
		printf("cannot open %s", MODEM_SYS_USB_SERIAL_BASE);
		return -1;
	}
	/* Walk through the directory. */
	while ((d = readdir(dir)) != NULL)
	{
		/* See if this is a process */
		if ((os_strstr(d->d_name, "tty")) != 0)
		{
			//MODEM_DR_DEBUG("%s\n", d->d_name);
			os_memset(path, 0, sizeof(path));
			os_memset(fullpath, 0, sizeof(fullpath));
			os_snprintf(path, sizeof(path), "%s/%s", MODEM_SYS_USB_SERIAL_BASE, d->d_name);

			if (realpath(path, fullpath))
			{
				// /sys/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.0/ttyUSB0
				if (strccnt(fullpath, '/') >= 6)
				{
					offset = strccntlast(fullpath, '/', 7);
					if (offset < os_strlen(fullpath))
					{
						os_memset(vender, 0, sizeof(vender));

						os_memset(path1, 0, sizeof(path1));
						os_memcpy(path1, fullpath, offset);

						snprintf(path, sizeof(path), "%s/%s", path1, "idVendor");

						//MODEM_DR_DEBUG("%s\n", path);

						if (os_read_file(path, vender, sizeof(vender)) == OK)
						{
							driver->vendor = strtoul(vender, NULL, 16);
							//MODEM_DR_DEBUG("vendor=%x\n", driver->vendor);
						}

						os_memset(path, 0, sizeof(path));
						os_memset(vender, 0, sizeof(vender));
						snprintf(path, sizeof(path), "%s/%s", path1, "idProduct");

						if (os_read_file(path, vender, sizeof(vender)) == OK)
						{
							driver->product = strtoul(vender, NULL, 16);
							//MODEM_DR_DEBUG("product=%x\n", driver->product);
						}
						i = modem_usb_driver_lookup(driver);
						if(i != ERROR)
						{
							modem_usb_driver_add_ttyusb(&musb_driver[i], d->d_name);

							//: /sys/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.0/ttyUSB0
							//: /sys/devices/pci0000:00/0000:00:14.0/usb2/2-2/2-2:1.0/ttyUSB0
							if(strisempty(musb_driver[i].usbkey, sizeof(musb_driver[i].usbkey)))
							{
								modem_usb_key_detection(fullpath, musb_driver[i].usbkey);
/*								char *brk = os_strstr(fullpath, 'usb');
								if(brk)
								{
									offset = brk - fullpath;
									int offset1 = strccntlast(fullpath + offset, '/', 2);
									if (os_strlen(fullpath) > (offset1 + 4))
									{
										brk = fullpath + offset + offset1 + 1;
										if(brk)
										{
											offset1 = strccntlast(brk, ':', 1);
											if(offset1 > 0 && offset1 < 4)
												os_strncpy(musb_driver[i].usbkey, brk, offset1 - 1);
										}
										//MODEM_DR_DEBUG("%s\n", musb_driver[i].usbkey);
									}
								}*/
							}
							if(strisempty(musb_driver[i].netdevname, sizeof(musb_driver[i].netdevname)))
							{
								modem_sys_usb_detection_netdevname(&musb_driver[i], musb_driver[i].usbkey);
							}
						}
						else
						{
							MODEM_DR_DEBUG("can not lookup: vendor=%x product=%x\n", driver->vendor, driver->product);
						}
					}
				}
			}
		}
	}
	closedir(dir);
	return OK;
}

static int modem_sys_product_detection(void)
{
	modem_usb_driver driver;
	os_memset(&driver, 0, sizeof(driver));
	modem_sys_usb_detection_devname(&driver);
	return OK;
}

static int modem_sys_lsusb_driver_split(char * input, modem_usb_driver *driver)
{
	if(os_strlen(input) >= os_strlen("Bus 002 Device 002: ID 2c7c:0125"))
		sscanf(input, "%*s %x %*s %x: %*s %x:%x", &driver->bus,
			&driver->device, &driver->vendor, &driver->product);
	if(driver->bus && driver->device && driver->vendor && driver->product)
		return OK;
	return ERROR;
}


static int modem_sys_lsusb_product_check(void)
{
	FILE *fp = NULL;
	char buf[1024];
	int ret = 0;
	modem_usb_driver driver;
	os_memset(buf, 0, sizeof(buf));
	remove("/tmp/modem-usb.info");
	super_system("lsusb > /tmp/modem-usb.info");
	fp = fopen("/tmp/modem-usb.info", "r");
	if(fp)
	{
		while(fgets(buf, sizeof(buf), fp))
		{
			os_memset(&driver, 0, sizeof(driver));
			if(modem_sys_lsusb_driver_split(buf, &driver) == OK)
			{
				if(modem_driver_lookup(driver.vendor, driver.product))
				{
					modem_usb_driver_add(&driver);
					ret ++;
				}
			}
		}
		fclose(fp);
	}
	return ret;
}


static int modem_sys_product_load(void)
{
	if(modem_sys_lsusb_product_check())
	{
		modem_sys_product_detection();
		modem_usb_driver_commit();
	}
	return OK;
}



#ifdef USB_KERNEL_UEVENT

static int usb_event_isok_string(char *buf, zpl_uint32 len)
{
	/*
	add@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.0/ttyUSB0/tty/ttyUSB0
	add@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.4/usbmisc/cdc-wdm0
	add@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.4/net/wwan0
	add@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.4/net/wwan0/queues/tx-0

	remove@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.0/ttyUSB0/tty/ttyUSB0
	remove@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.4/net/wwan0/queues/tx-0
	remove@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.4/net/wwan0
	remove@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.4/usbmisc/cdc-wdm0
	*/
	if(os_strstr(buf, "add@/") && os_strstr(buf, "tty"))
		return OK;

	else if(os_strstr(buf, "add@/") && os_strstr(buf, "net/"))
		return OK;

	else if(os_strstr(buf, "remove@/") && os_strstr(buf, "tty"))
		return OK;

	else if(os_strstr(buf, "remove@/") && os_strstr(buf, "net/"))
		return OK;

	return ERROR;
}

static int usb_vendor_product_read(modem_usb_driver *driver, char *buf, zpl_uint32 len)
{
	if (driver->product == 0 && driver->vendor == 0)
	{
		char *brk = NULL, *brk1 = NULL;
		zpl_uint32 offset = 0;
		if(strccnt(buf, '/') >= 6)
		{
			offset = strccntlast(buf, '/', 1);
			brk = buf + offset + 1;

			offset = strccntlast(buf, '/', 6);
			brk1 = buf + offset;

			if (brk1 && (brk1 > brk))
			{
				char path[512];
				char path1[512];
				char vender[128];
				os_memset(vender, 0, sizeof(vender));
				os_memset(path, 0, sizeof(path));
				os_memset(path1, 0, sizeof(path1));
				os_memcpy(path1, brk, brk1 - brk);

				snprintf(path, sizeof(path), "/sys/%s/%s", path1, "idVendor");

				if(os_read_file(path, vender, sizeof(vender)) == OK)
				{
					driver->vendor = strtoul(vender, NULL, 16);
				}

				os_memset(path, 0, sizeof(path));
				os_memset(vender, 0, sizeof(vender));
				snprintf(path, sizeof(path), "/sys/%s/%s", path1, "idProduct");

				if(os_read_file(path, vender, sizeof(vender)) == OK)
				{
					driver->product = strtoul(vender, NULL, 16);
				}
				return OK;
			}
		}
	}
	return OK;
}

static int usb_event_key_detection(modem_usb_driver *driver, char *buf, zpl_uint32 len)
{
	////add@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.0/ttyUSB0
	return modem_usb_key_detection(buf, driver->usbkey);
}

static int usb_event_add_detection(modem_usb_driver *driver, char *buf, zpl_uint32 len)
{
	if(os_strstr(buf, "add@/"))
	{
		//add@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.0/ttyUSB0
		char devname[USB_SERIAL_NAME_MAX];
		char *brk = os_strstr(buf, "tty");
		//brk += os_strlen("tty");
		if(brk && !os_strstr(brk + 1, "/"))
		{
			//MODEM_DR_DEBUG("%s\n", buf);
			os_memset(devname, 0, sizeof(devname));
			os_strcpy(devname, brk);
			//devname[os_strlen(devname)-1] = '\0';
			//MODEM_DR_DEBUG("%s(%s)\n", buf, devname);
			modem_usb_driver_add_ttyusb(driver, devname);
			//driver->change = USB_EVENT_ADD;
			return OK;
		}
		//add@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.4/net/wwan0

		if(os_strstr(buf, "net"))
		{
			brk = os_strstr(buf, "net");
			brk += os_strlen("net") + 1;
			if(brk && !os_strstr(brk, "/"))
			{
				usb_event_key_detection(driver, buf, len);

				os_memset(devname, 0, sizeof(devname));
				os_strcpy(devname, brk);
				//devname[os_strlen(devname)-1] = '\0';
				os_strcpy(driver->netdevname, devname);
			//	MODEM_DR_DEBUG("%s(%s)\n", buf, driver->netdevname);
				driver->change = USB_EVENT_ADD;

				return OK;
			}
		}
	}
	return ERROR;
}


static int usb_event_del_detection(char *buf, zpl_uint32 len)
{
	if(os_strstr(buf, "remove@/"))
	{
		//add@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.0/ttyUSB0/tty/ttyUSB0
		//add@/devices/platform/101c0000.ehci/usb2/2-1/2-1:1.4/net/wwan0
		if(os_strstr(buf, "tty"))
		{
			modem_usb_driver driver;
			os_memset(&driver, 0, sizeof(driver));

			usb_event_key_detection(&driver, buf, len);
			//MODEM_DR_DEBUG("usbkey:(%s)\n", driver.usbkey);

			if(!strisempty(driver.usbkey, sizeof(driver.usbkey)))
			//if(os_strlen(driver.usbkey))
			{
				int in = modem_usb_driver_usbkey_lookup(driver.usbkey);
				if(in != ERROR)
				{
					modem_usb_driver_del(&musb_driver[in]);
				}
			}
		}
	}
	return OK;
}

static int usb_event_handle_detection(char *buf, zpl_uint32 len)
{
	if(os_strstr(buf, "add@/"))
	{
		modem_usb_driver driver;
		os_memset(&driver, 0, sizeof(driver));
		usb_vendor_product_read(&driver, buf, len);
		if(driver.vendor && driver.product)
		{
			int in = modem_usb_driver_lookup(&driver);
			if(in != ERROR)
				usb_event_add_detection(&musb_driver[in], buf, len);
			else
			{
				modem_usb_driver_add(&driver);
				in = modem_usb_driver_lookup(&driver);
				if(in != ERROR)
					usb_event_add_detection(&musb_driver[in], buf, len);
			}
		}

	}
	else
		usb_event_del_detection(buf, len);

	return OK;
}

static int usb_event_handle_finsh_one(modem_usb_driver *driver)
{
	if(!driver)
		return ERROR;

	if(driver->change == USB_EVENT_NONE)
		return OK;
	else if(driver->change == USB_EVENT_ADD)
	{
		if( driver->devcnt >= 3 &&
				!strisempty(driver->netdevname, sizeof(driver->netdevname)) &&
				//os_strlen(driver->netdevname) &&
				driver->vendor &&
				driver->product)
		{
			driver->change = USB_EVENT_NONE;
			//modem_usb_driver_add(driver);
			modem_usb_driver_commit();

			//os_memset(driver, 0, sizeof(modem_usb_driver));
		}
		return OK;
	}
	else if(driver->change == USB_EVENT_DEL)
	{
		driver->change = USB_EVENT_NONE;
		//os_memset(driver, 0, sizeof(modem_usb_driver));
		return OK;//modem_usb_driver_del(driver);
	}
	return ERROR;
}

static int usb_event_handle_finsh(void)
{
	zpl_uint32 i = 0;
	for(i = 0; i < MODEM_USB_DRIVER_MAX; i++)
	{
		if(musb_driver[i].active == MODEM_USB_DRIVER_ACTIVE)
		{
			usb_event_handle_finsh_one(&musb_driver[i]);
		}
	}
	return OK;
}

#if 0
static int usb_event_socket = 0;

static int usb_event_socket_init()
{
	if(usb_event_socket == 0)
	{
		struct sockaddr_nl client;
	    os_memset(&client, 0, sizeof(client));
		usb_event_socket = socket(IPSTACK_AF_NETLINK, IPSTACK_SOCK_RAW, NETLINK_KOBJECT_UEVENT);
		if(usb_event_socket)
		{
			int buffersize = UEVENT_BUFFER_SIZE;
			client.nl_family = IPSTACK_AF_NETLINK;
			client.nl_pid = getpid();
			client.nl_groups = 1; /* receive broadcast message*/
			setsockopt(usb_event_socket, IPSTACK_SOL_SOCKET, IPSTACK_SO_RCVBUF, &buffersize, sizeof(buffersize));
			if(bind(usb_event_socket, (struct sockaddr*)&client, sizeof(client)) == 0)
				return OK;
			close(usb_event_socket);
			usb_event_socket = 0;
		}
	}
	return ERROR;
}

static int usb_event_socket_exit()
{
	if(usb_event_socket == 0)
	{
		close(usb_event_socket);
		usb_event_socket = 0;
	}
}

static int usb_event_socket_handle(int timeout)
{
	fd_set fds;
	struct timeval tv;
	int rcvlen, ret;
	char buf[UEVENT_BUFFER_SIZE] = { 0 };
	//modem_usb_driver driver;
	if (usb_event_socket == 0)
	{
		usb_event_socket_init();
		if (usb_event_socket == 0)
		{
			return OK;
		}
	}
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	//os_memset(&driver, 0, sizeof(driver));
	while (1)
	{
		FD_ZERO(&fds);
		FD_SET(usb_event_socket, &fds);
		ret = select(usb_event_socket + 1, &fds, NULL, NULL, &tv);
		if (ret < 0)
		{
			if ( ipstack_errno == EPIPE || ipstack_errno == EBADF || ipstack_errnock_errno == EIO)
			{
				usb_event_socket_exit();
				return ERROR;
			}
			else if(ipstack_errnock_errnock_errnock_errno == EAGAIN || ipstack_errno == EBUSY || ipstack_errno == EINTR)
			{
				continue;
			}
			usb_event_socket_exit();
			return ERROR;
		}
		if(ret == 0)
		{
			break;
		}
		if ((ret > 0 && FD_ISSET(usb_event_socket, &fds)))
		{
			os_memset(buf, 0, sizeof(buf));
			rcvlen = recv(usb_event_socket, buf, sizeof(buf), 0);
			if (rcvlen > 0)
			{
				if(usb_event_isok_string(buf, rcvlen) == OK)
				{
					usb_event_handle_detection(buf, rcvlen);
				}
			}
		}
	}
	usb_event_handle_finsh();
	return OK;
}
#else

static int usb_event_socket_read(void *pVoid);

static int usb_event_socket_handle(zpl_uint32 timeout)
{
	int sock = 0;
	struct sockaddr_nl client;
    os_memset(&client, 0, sizeof(client));
    sock = socket(IPSTACK_AF_NETLINK, IPSTACK_SOCK_RAW, NETLINK_KOBJECT_UEVENT);
	if(sock)
	{
		zpl_uint32 buffersize = UEVENT_BUFFER_SIZE * 4;
		client.nl_family = IPSTACK_AF_NETLINK;
		client.nl_pid = getpid();
		client.nl_groups = 1; /* receive broadcast message*/

		if(bind(sock, (struct sockaddr*)&client, sizeof(client)) == 0)
		{
			//os_set_nonblocking(sock);
			setsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_RCVBUF, &buffersize, sizeof(buffersize));
			//return os_ansync_register_api(modem_ansync_lst, OS_ANSYNC_INPUT, usb_event_socket_read, NULL, sock);
			return modem_ansync_add(usb_event_socket_read, sock, "usb_event_socket_read");
		}
		close(sock);
		sock = 0;
	}
	return ERROR;
}


static int usb_event_socket_read(void *pVoid)
{
	zpl_uint32 rcvlen;//, ret;
	char buf[UEVENT_BUFFER_SIZE] = { 0 };
	int fd = OS_ANSYNC_FD(pVoid);
try_agent:
	ipstack_errno = 0;
	os_memset(buf, 0, sizeof(buf));
	rcvlen = recv(fd, buf, sizeof(buf), 0);
	if (rcvlen > 0)
	{
		if(usb_event_isok_string(buf, rcvlen) == OK)
		{
			usb_event_handle_detection(buf, rcvlen);
			usb_event_handle_finsh();
		}
		//MODEM_DR_DEBUG("add usb_event_socket_read\n");
		//modem_ansync_add(usb_event_socket_read, fd, "usb_event_socket_read");;
		return OK;
	}
	else
	{
		if (ipstack_errno == EINTR)
			goto try_agent;
		else if(ipstack_errno == EAGAIN)
		{
			//os_ansync_register_api(modem_ansync_lst, OS_ANSYNC_INPUT, usb_event_socket_read, NULL, fd);
			return OK;
		}
		else if(ipstack_errno == ENOBUFS)
		{

		}
		else
		{
			modem_ansync_del(fd);
			close(fd);
			usb_event_socket_handle(1);
		}
	}
	//MODEM_DR_DEBUG("usb_event_socket_read(%d)(%s)\n", fd, strerror(ipstack_errno));
	return ERROR;
}



#endif

#endif


int modem_product_detection(void)
{
	if(usb_driver_startup_init == 0)
	{
		if(access(MODEM_PROC_USB_SERIAL_BASE, F_OK) == 0)
		{
			usb_driver_startup_init = 1;
			modem_proc_product_load();
		}
		else
		{
			usb_driver_startup_init = 1;
			modem_sys_product_load();
		}
	}
#ifdef USB_KERNEL_UEVENT
	return usb_event_socket_handle(1);
#else
	usb_driver_startup_init = 0;
	modem_usb_driver_update_ttl();
#endif
	return OK;
}



