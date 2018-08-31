/*
 * modem_serial.c
 *
 *  Created on: Jul 29, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "os_util.h"
#include "tty_com.h"

#include "modem.h"
#include "modem_attty.h"
#include "modem_client.h"
#include "modem_message.h"
#include "modem_event.h"
#include "modem_machine.h"
#include "modem_dhcp.h"
#include "modem_pppd.h"
#include "modem_driver.h"
#include "modem_serial.h"

#include "modem_product.h"



static modem_serial_main gModemSerialmain;



int modem_serial_init(void)
{
	os_memset(&gModemSerialmain, 0, sizeof(modem_serial_main));
	gModemSerialmain.list = XMALLOC(MTYPE_MODEM, sizeof(LIST));
	if(gModemSerialmain.list)
	{
		gModemSerialmain.mutex = os_mutex_init();
		lstInit(gModemSerialmain.list);
		return OK;
	}
	return ERROR;
}

int modem_serial_exit(void)
{
	if(gModemSerialmain.list)
	{
		if(lstCount(gModemSerialmain.list))
			lstFree(gModemSerialmain.list);
		XFREE(MTYPE_MODEM, gModemSerialmain.list);
	}
	if(gModemSerialmain.mutex)
		os_mutex_exit(gModemSerialmain.mutex);
	return OK;
}


static modem_serial_t * modem_serial_lookup_node(const char *name)
{
	NODE index;
	modem_serial_t *pstNode = NULL;
	assert(name);
	char pname[MODEM_STRING_MAX];
	os_memset(pname, 0, MODEM_STRING_MAX);
	os_strcpy(pname, name);
	for(pstNode = (modem_serial_t *)lstFirst(gModemSerialmain.list);
			pstNode != NULL;  pstNode = (modem_serial_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(os_memcmp(pname, pstNode->name, MODEM_STRING_MAX) == 0)
		{
			return pstNode;
		}
	}
	return NULL;
}


static int modem_serial_add_node(const char *name)
{
	if(name)
	{
		modem_serial_t *node = XMALLOC(MTYPE_MODEM, sizeof(modem_serial_t));
		if(node)
		{
			os_memset(node, 0, sizeof(modem_serial_t));
			os_strcpy(node->name, name);
			lstAdd(gModemSerialmain.list, (NODE*)node);
			return OK;
		}
		return ERROR;
	}
	return ERROR;
}

static int modem_serial_del_node(modem_serial_t *node)
{
	assert(node);
	if(node)
	{
		lstDelete(gModemSerialmain.list, (NODE*)node);
		XFREE(MTYPE_MODEM, node);
		return OK;
	}
	return ERROR;
}


int modem_serial_add_api(const char *name)
{
	int ret = 0;
	assert(name);
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	if(!modem_serial_lookup_node(name))
		ret = modem_serial_add_node(name);
	else
		ret = ERROR;
	if(gModemSerialmain.mutex)
		os_mutex_unlock(gModemSerialmain.mutex);
	return ret;
}

int modem_serial_del_api(const char *name)
{
	int ret = ERROR;
	assert(name);
	modem_serial_t *node = NULL;
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	node = modem_serial_lookup_node(name);
	ret = modem_serial_del_node(node);
	if(gModemSerialmain.mutex)
		os_mutex_unlock(gModemSerialmain.mutex);
	return ret;
}

int modem_serial_bind_api(const char *name, void *client)
{
	int ret = 0;
	assert(name);
	modem_serial_t *node = NULL;
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	node = modem_serial_lookup_node(name);
	if(node)
	{
		node->client = client;
		node->active = TRUE;
		node->driver = ((modem_client_t *)client)->driver;
		if(node->modem)
		{
			modem_t *modem = node->modem;
			modem->serial = node;
			modem->client = node->client;
		}
		if(client)
		{
			((modem_client_t *)client)->serial = node;
			((modem_client_t *)client)->modem = node->modem;
		}
	}
	if(gModemSerialmain.mutex)
		os_mutex_unlock(gModemSerialmain.mutex);
	return ret;
}

int modem_serial_unbind_api(const char *name)
{
	int ret = 0;
	assert(name);
	modem_serial_t *node = NULL;
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	node = modem_serial_lookup_node(name);
	if(node)
	{
		node->driver = NULL;
		node->client = NULL;
		node->active = FALSE;
	}
	if(gModemSerialmain.mutex)
		os_mutex_unlock(gModemSerialmain.mutex);
	return ret;
}

/*
int modem_serial_interface_bind_api(const char *name, void *ifp)
{
	int ret = 0;
	assert(name);
	modem_serial_t *node = NULL;
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	node = modem_serial_lookup_node(name);
	if(node)
	{
		node->ifp = ifp;
		if(node->modem)
		{
			modem_t *modem = node->modem;
			modem->ifp = ifp;
		}
	}
	if(gModemSerialmain.mutex)
		os_mutex_unlock(gModemSerialmain.mutex);
	return ret;
}

int modem_serial_interface_unbind_api(const char *name)
{
	int ret = 0;
	assert(name);
	modem_serial_t *node = NULL;
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	node = modem_serial_lookup_node(name);
	if(node)
	{
		node->ifp = NULL;
		if(node->modem)
		{
			modem_t *modem = node->modem;
			modem->ifp = NULL;
		}
	}
	if(gModemSerialmain.mutex)
		os_mutex_unlock(gModemSerialmain.mutex);
	return ret;
}*/

modem_serial_t * modem_serial_lookup_api(const char *name)
{
	modem_serial_t *node = NULL;
	assert(name);
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	node = modem_serial_lookup_node(name);
	if(gModemSerialmain.mutex)
		os_mutex_unlock(gModemSerialmain.mutex);
	return node;
}


int modem_serial_callback_api(modem_serial_cb cb, void *pVoid)
{
	NODE index;
	modem_serial_t *pstNode = NULL;
	if(lstCount(gModemSerialmain.list) ==  0)
		return OK;
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	for(pstNode = (modem_serial_t *)lstFirst(gModemSerialmain.list);
			pstNode != NULL;  pstNode = (modem_serial_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(cb)
			(cb)(pstNode, pVoid);
	}
	if(gModemSerialmain.mutex)
		os_mutex_unlock(gModemSerialmain.mutex);
	return OK;
}


const char *modem_serial_channel_name(modem_driver_t *input)
{
	modem_driver_t *driver = (modem_driver_t *)input;
	static char format[MODEM_STRING_MAX];
	assert(driver);
	os_memset(format,0, sizeof(format));
	//snprintf(format, sizeof(format), "modem%d%d", driver->bus, driver->device);
	snprintf(format, sizeof(format), "modem%d%d", driver->vendor, driver->product);
	return format;
}
