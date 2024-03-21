/*
 * modem_serial.c
 *
 *  Created on: Jul 29, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zpl_type.h"
#include "zmemory.h"
#include "log.h"
#include "log.h"
#include "str.h"
#include "prefix.h"

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
#include "modem_string.h"
#include "modem_usb_driver.h"



static modem_serial_main gModemSerialmain;

static int modem_serial_cleanall(void);
//static zpl_uint8 modem_channel_db[MODEM_CHANNEL_DB_MAX];

int modem_serial_init(void)
{
	os_memset(&gModemSerialmain, 0, sizeof(modem_serial_main));
	gModemSerialmain.list = XMALLOC(MTYPE_MODEM, sizeof(LIST));
	if(gModemSerialmain.list)
	{
		gModemSerialmain.mutex = os_mutex_name_create("gModemSerialmain.mutex");
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
		{
			modem_serial_cleanall();
			lstFree(gModemSerialmain.list);
		}
		XFREE(MTYPE_MODEM, gModemSerialmain.list);
	}
	if(gModemSerialmain.mutex)
		os_mutex_destroy(gModemSerialmain.mutex);
	return OK;
}


static modem_serial_t * modem_serial_lookup_node(const char *name, zpl_uint8 hw_channel)
{
	NODE index;
	modem_serial_t *pstNode = NULL;
	//assert(name);
	char pname[MODEM_STRING_MAX];
	os_memset(pname, 0, MODEM_STRING_MAX);
	if(name)
		os_strcpy(pname, name);
	for(pstNode = (modem_serial_t *)lstFirst(gModemSerialmain.list);
			pstNode != NULL;  pstNode = (modem_serial_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(name)
		{
			if(os_memcmp(pname, pstNode->name, MODEM_STRING_MAX) == 0)
			{
				return pstNode;
			}
		}
		else if(hw_channel)
		{
			if(pstNode->hw_channel == hw_channel)
			{
				return pstNode;
			}
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

static int modem_serial_cleanall(void)
{
	NODE index;
	modem_serial_t *pstNode = NULL;
	for(pstNode = (modem_serial_t *)lstFirst(gModemSerialmain.list);
			pstNode != NULL;  pstNode = (modem_serial_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			lstDelete(gModemSerialmain.list, (NODE*)pstNode);
			pstNode->modem = NULL;
			XFREE(MTYPE_MODEM, pstNode);
			pstNode = NULL;
		}
	}
	return OK;
}

int modem_serial_add_api(const char *name)
{
	int ret = OK;
	assert(name);
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	if(!modem_serial_lookup_node(name, 0))
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
	node = modem_serial_lookup_node(name, 0);
	ret = modem_serial_del_node(node);
	if(gModemSerialmain.mutex)
		os_mutex_unlock(gModemSerialmain.mutex);
	return ret;
}


int modem_serial_channel_api(const char *name, zpl_uint8 hw_channel)
{
	int ret = 0;
	modem_serial_t *node = NULL;
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	node = modem_serial_lookup_node(name, hw_channel);
	if(node)
	{
		node->hw_channel = hw_channel;
	}
	if(gModemSerialmain.mutex)
		os_mutex_unlock(gModemSerialmain.mutex);
	return ret;
}

int modem_serial_bind_api(const char *name, zpl_uint8 hw_channel, void *client)
{
	int ret = 0;
	modem_serial_t *node = NULL;
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	node = modem_serial_lookup_node(name, hw_channel);
	if(node)
	{
		node->client = client;
		node->active = zpl_true;
		node->driver = ((modem_client_t *)client)->driver;
		if(node->modem)
		{
			modem_t *modem = node->modem;
			modem->serial = node;
			modem->client = node->client;
		}
		else
		{

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

int modem_serial_unbind_api(const char *name, zpl_uint8 hw_channel)
{
	int ret = 0;
	modem_serial_t *node = NULL;
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	node = modem_serial_lookup_node(name, hw_channel);
	if(node)
	{
		if(node->modem)
		{
			modem_t *modem = node->modem;
			modem->serial = node;
			modem->client = NULL;
		}
		if(node->client)
		{
			((modem_client_t *)node->client)->serial = NULL;
			((modem_client_t *)node->client)->modem = NULL;
		}
		node->driver = NULL;
		node->client = NULL;
		node->active = zpl_false;
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

modem_serial_t * modem_serial_lookup_api(const char *name, zpl_uint8 hw_channel)
{
	modem_serial_t *node = NULL;
	if(gModemSerialmain.mutex)
		os_mutex_lock(gModemSerialmain.mutex, OS_WAIT_FOREVER);
	node = modem_serial_lookup_node(name, hw_channel);
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
	//char tmp[MODEM_STRING_MAX];
	assert(driver);
	os_memset(format,0, sizeof(format));
	//os_memset(tmp,0, sizeof(tmp));
	os_strcpy(format, driver->module_name);
	strchr_empty(format, ' ');
	return strlwr(format);
}

