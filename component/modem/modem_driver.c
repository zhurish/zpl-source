/*
 * driver.c
 *
 *  Created on: Jul 25, 2018
 *      Author: zhurish
 */



#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"

#include "os_util.h"
#include "tty_com.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_driver.h"


#define MODEM_DRIVER_MOJR(n)				(n) ? (n):++md_driver_id

static int md_driver_id = 0;


int modem_driver_register(modem_driver_t *driver)
{
	assert(driver);
	modem_client_t *client = modem_client_alloc(driver->vendor, driver->product);
	if(client)
	{
		//os_memcpy(&client->driver, driver, sizeof(modem_driver_t));
		client->driver = driver;
		if(client->driver)
			client->driver->client = client;
		client->bus = driver->bus;
		client->device = driver->device;
		//client->vendor = driver->vendor;
		//client->product = driver->product;
		os_memcpy(client->module_name, driver->module_name,
				MIN(MODEM_PRODUCT_NAME_MAX,os_strlen(driver->module_name)));

		client->attty = driver->attty;
		client->pppd = driver->pppd;

		driver->id = MODEM_DRIVER_MOJR(0);

		//os_memcpy(&client->attty, &driver->attty, sizeof(struct tty_com));

		return modem_client_register_api(client);
	}
	return ERROR;
}



int modem_driver_unregister(modem_driver_t *driver)
{
	assert(driver);
	return modem_client_del_by_product_api(driver->vendor, driver->product);
}


modem_driver_t * modem_driver_lookup(int vendor, int product)
{
	modem_client_t *client= modem_client_lookup_api(vendor, product);
	return client? client->driver:NULL;
}


int modem_driver_remove(int vendor, int product)
{
	modem_driver_t *driver = modem_driver_lookup(vendor, product);
	modem_client_t *client = NULL;
	if(!driver || !driver->client)
	{
		return ERROR;
	}
	if(MODEM_IS_DEBUG(DRIVER))
		zlog_warn(ZLOG_MODEM, "Remove modem :%s %x %x ", driver->module_name,
				driver->vendor, driver->product);
	client = driver->client;
	if(client->modem)
		modem_event_add_api(((modem_client_t*)client)->modem, MODEM_EV_REMOVE, TRUE);
	return OK;
}


int modem_driver_inster(int vendor, int product)
{
	modem_driver_t *driver = modem_driver_lookup(vendor, product);
	modem_client_t *client = NULL;
	if(!driver || !driver->client)
	{
		return ERROR;
	}
	if(MODEM_IS_DEBUG(DRIVER))
		zlog_debug(ZLOG_MODEM, "Inster modem : %s %x %x ",
				driver->module_name, driver->vendor, driver->product);

	client = driver->client;
	if(modem_serial_lookup_api(modem_serial_channel_name(driver)))
	{
		modem_serial_bind_api(modem_serial_channel_name(driver), client);
	}
	else
	{
		if(modem_serial_add_api(modem_serial_channel_name(driver)) == OK)
			modem_serial_bind_api(modem_serial_channel_name(driver), client);
	}
	if(client->modem)
		modem_event_add_api(((modem_client_t*)client)->modem, MODEM_EV_INSTER, TRUE);
	return OK;
}




int modem_driver_tty_probe(modem_driver_t *driver, char *devname[])
{
	int i = 0;
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
				if(driver->client)
				{
					modem_serial_devname_add(((modem_client_t *)driver->client)->modem, driver->pppd->devname);
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
		if(driver->client)
		{
			modem_interface_add(((modem_client_t *)driver->client)->modem, driver->eth_name);
		}
	}
	return OK;
}


int modem_driver_probe(modem_driver_t *driver)
{
	assert(driver);
/*	if(driver->md_probe)
		return (driver->md_probe)(driver);*/
	return OK;
}

int modem_driver_init(modem_driver_t *driver)
{
	assert(driver);
/*	if(driver->md_init)
		return (driver->md_init)(driver);*/
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
