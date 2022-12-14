/*
 * driver.c
 *
 *  Created on: Jul 25, 2018
 *      Author: zhurish
 */



#include "auto_include.h"
#include <zplos_include.h>
#include "zmemory.h"
#include "vty.h"
#include "command.h"
#include "if.h"
#include "vrf.h"
#include "nsm_interface.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_driver.h"
#include "modem_serial.h"
#include "modem_process.h"
#include "modem_usb_driver.h"

#define MODEM_DRIVER_MOJR(n)				(n) ? (n):++md_driver_id

static zpl_uint32 md_driver_id = 0;


int modem_driver_register(modem_driver_t *driver)
{
	assert(driver);
	modem_client_t *client = modem_client_alloc(driver->vendor, driver->product);
	if(client)
	{
		//os_memcpy(&client->driver, driver, sizeof(modem_driver_t));
		client->driver = driver;
		if(client->driver)
		{
			client->driver->client = client;
			driver->client = client;
		}
		client->bus = driver->bus;
		client->device = driver->device;
		//client->vendor = driver->vendor;
		//client->product = driver->product;
		os_memcpy(client->module_name, driver->module_name,
				MIN(MODEM_PRODUCT_NAME_MAX,os_strlen(driver->module_name)));

		client->attty = driver->attty;
		client->pppd = driver->pppd;

		driver->id = MODEM_DRIVER_MOJR(0);

		driver->modem_driver_init = modem_driver_init;
		driver->modem_driver_probe = modem_driver_probe;
		driver->modem_driver_exit = modem_driver_exit;
		driver->modem_driver_reboot = modem_driver_reboot;
		//os_memcpy(&client->attty, &driver->attty, sizeof(struct tty_com));

		return modem_client_register_api(client);
	}
	return ERROR;
}



int modem_driver_unregister(modem_driver_t *driver)
{
	assert(driver);
	assert(driver->client);
	return modem_client_del_api(driver->client);
	//return modem_client_del_by_product_api(driver->vendor, driver->product);
}


modem_driver_t * modem_driver_lookup(zpl_uint32 vendor, zpl_uint32 product)
{
	modem_client_t *client= modem_client_lookup_api(vendor, product);
	return client? client->driver:NULL;
}


int modem_driver_remove(zpl_uint32 vendor, zpl_uint32 product)
{
	modem_driver_t *driver = modem_driver_lookup(vendor, product);
	modem_client_t *client = NULL;
	if(!driver || !driver->client)
	{
		return ERROR;
	}
	if(MODEM_IS_DEBUG(DRIVER))
		zlog_warn(MODULE_MODEM, "Remove modem :%s %x %x ", driver->module_name,
				driver->vendor, driver->product);
	client = driver->client;
	if(client->modem)
		modem_event_add_api(((modem_client_t*)client)->modem, MODEM_EV_REMOVE, zpl_true);
	return OK;
}

int modem_driver_hw_channel(zpl_uint32 vendor, zpl_uint32 product, zpl_uint8 *hw_channel)
{
	int channel = 0;
	channel = modem_usb_driver_hardware_channel(vendor, product);
	if(channel)
	{
		if(hw_channel)
			*hw_channel = channel;
		return OK;
	}
	return ERROR;
}

int modem_driver_inster(zpl_uint32 vendor, zpl_uint32 product)
{
	zpl_uint8 hw_channel = 0;
	modem_driver_t *driver = modem_driver_lookup(vendor, product);
	modem_client_t *client = NULL;
	if(!driver || !driver->client)
	{
		return ERROR;
	}

	client = driver->client;

	if(MODEM_IS_DEBUG(DRIVER))
		zlog_debug(MODULE_MODEM, "Inster modem : %s %x %x (%s)",
				driver->module_name, driver->vendor, driver->product, client->module_name);

	if(modem_driver_hw_channel(vendor, product, &hw_channel) == OK)
	{
		if(modem_serial_lookup_api(NULL/*modem_serial_channel_name(driver)*/, hw_channel))
		{
			modem_serial_bind_api(NULL/*modem_serial_channel_name(driver)*/, hw_channel, client);
		}
		else
		{
			//zlog_warn(MODULE_MODEM, "Create modem-channel profile : %s",modem_serial_channel_name(driver));
			if(modem_serial_add_api(modem_serial_channel_name(driver)) == OK)
			{
				modem_serial_channel_api(modem_serial_channel_name(driver), hw_channel);
				modem_serial_bind_api(modem_serial_channel_name(driver), hw_channel, client);
			}
			else
			{
				if(modem_serial_lookup_api(modem_serial_channel_name(driver), 0))
					zlog_warn(MODULE_MODEM, "modem-channel profile '%s' is already exist.",
							modem_serial_channel_name(driver));
				else
				//if(MODEM_IS_DEBUG(DRIVER))
					zlog_warn(MODULE_MODEM, "Can not create modem-channel profile for : %s %x %x (%s)",
							driver->module_name, driver->vendor, driver->product, client->module_name);
			}
		}
		if(client->modem)
		{
			if(os_strlen(driver->eth_name) && (((modem_t *)client->modem)->dialtype != MODEM_DIAL_PPP))
				modem_interface_update_kernel(((modem_client_t *)client)->modem, driver->eth_name);
			modem_event_add_api(((modem_client_t*)client)->modem, MODEM_EV_INSTER, zpl_true);
		}
		return OK;
	}
	if(MODEM_IS_DEBUG(DRIVER))
		zlog_err(MODULE_MODEM, "Inster modem : %s %x %x (%s) can not find hw channel",
				driver->module_name, driver->vendor, driver->product, client->module_name);

	return ERROR;
}




int modem_driver_tty_probe(modem_driver_t *driver, char *devname[])
{
	zpl_uint32 i = 0;
	assert(driver);
	if(!driver)
	{
		return ERROR;
	}
	for(i = 0; i < TTY_USB_MAX; i++)
	{

		switch(driver->ttyseq[i])
		{
		case TTY_USB_DIAL:
			if(driver->dialog && devname[i])
			{
				os_memset(driver->dialog->devname, 0, sizeof(driver->dialog->devname));
				os_strcpy(driver->dialog->devname, devname[i]);
			}
			//MODEM_DR_DEBUG("%s dialog:%s",__func__, devname[i]);
			break;

		case TTY_USB_AT:
			if(driver->attty && devname[i])
			{
				os_memset(driver->attty->devname, 0, sizeof(driver->attty->devname));
				os_strcpy(driver->attty->devname, devname[i]);
			}
			//MODEM_DR_DEBUG("%s attty:%s",__func__, devname[i]);
			break;

		case TTY_USB_PPP:
			if(driver->pppd && devname[i])
			{
				os_memset(driver->pppd->devname, 0, sizeof(driver->pppd->devname));
				os_strcpy(driver->pppd->devname, devname[i]);
				if(driver->client && ((modem_client_t *)driver->client)->modem)
				{
					modem_serial_devname_update_kernel(((modem_client_t *)driver->client)->modem, driver->pppd->devname);
				}
			}
			//MODEM_DR_DEBUG("%s pppd:%s",__func__, devname[i]);
			break;

		case TTY_USB_TEST:
			if(driver->usetty && devname[i])
			{
				os_memset(driver->usetty->devname, 0, sizeof(driver->usetty->devname));
				os_strcpy(driver->usetty->devname, devname[i]);
			}
			//MODEM_DR_DEBUG("%s usetty:%s",__func__, devname[i]);
			break;

		default:
			break;
		}
	}
	if(os_strlen(driver->eth_name))
	{
		if(driver->client && ((modem_client_t *)driver->client)->modem)
		{
			modem_t *modem = ((modem_client_t *)driver->client)->modem;
			if(modem->dialtype != MODEM_DIAL_PPP)
				modem_interface_update_kernel(((modem_client_t *)driver->client)->modem, driver->eth_name);
		}
	}
	return OK;
}


/**********************************************************************/
int modem_driver_init(modem_driver_t *driver)
{
	assert(driver);
/*	if(driver->md_init)
		return (driver->md_init)(driver);*/
	return OK;
}

int modem_driver_probe(modem_driver_t *driver)
{
	assert(driver);

	if(os_strlen(driver->eth_name))
	{
		if(driver->client && ((modem_client_t *)driver->client)->modem)
		{
			modem_t *modem = ((modem_client_t *)driver->client)->modem;
			if(modem->dialtype != MODEM_DIAL_PPP)
				modem_interface_update_kernel(((modem_client_t *)driver->client)->modem, driver->eth_name);
		}
	}
	if(driver->pppd && os_strlen(driver->pppd->devname))
	{
		if(driver->client && ((modem_client_t *)driver->client)->modem)
		{
			modem_serial_devname_update_kernel(((modem_client_t *)driver->client)->modem, driver->pppd->devname);
		}
	}

	if(driver->client && ((modem_client_t *)driver->client)->modem)
	{
		if(os_strlen(driver->eth_name) && (((modem_t *)((modem_client_t *)driver->client)->modem)->dialtype != MODEM_DIAL_PPP))
			modem_interface_update_kernel(((modem_client_t *)driver->client)->modem, driver->eth_name);

		modem_process_add_api(MODEM_EV_INSTER, ((modem_client_t *)driver->client)->modem, zpl_false);
	}

	return OK;
}

int modem_driver_exit(modem_driver_t *driver)
{
	assert(driver);

	if(driver->client)
	{
		modem_event_del_api(((modem_client_t *)driver->client)->modem, MODEM_EV_MAX, zpl_true);
		((modem_client_t *)driver->client)->modem = NULL;
	}
/*	if(driver->md_probe)
		return (driver->md_probe)(driver);*/
	return OK;
}

int modem_driver_reboot(modem_driver_t *driver)
{
	assert(driver);
/*	if(driver->md_reboot)
		return (driver->md_reboot)(driver);*/
	return OK;
}

#undef MODEM_DRIVER_MOJR
