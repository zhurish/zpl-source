/*
 * modem_client.c
 *
 *  Created on: Jul 24, 2018
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

#include "modem_attty.h"



static modem_client_list_t gModemClient;



int modem_client_init(void)
{
	os_memset(&gModemClient, 0, sizeof(modem_client_list_t));
	gModemClient.list = XMALLOC(MTYPE_MODEM, sizeof(LIST));
	if(gModemClient.list)
	{
		gModemClient.mutex = os_mutex_init();
		lstInit(gModemClient.list);
		return OK;
	}
	return ERROR;
}

int modem_client_exit(void)
{
	if(gModemClient.list)
	{
		if(lstCount(gModemClient.list))
			lstFree(gModemClient.list);
		XFREE(MTYPE_MODEM, gModemClient.list);
	}
	if(gModemClient.mutex)
		os_mutex_exit(gModemClient.mutex);
	return OK;
}


static modem_client_t * modem_client_lookup_node(int vendor, int product)
{
	modem_client_t *pstNode = NULL;
	NODE index;
	for(pstNode = (modem_client_t *)lstFirst(gModemClient.list);
			pstNode != NULL;  pstNode = (modem_client_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if((product == pstNode->product) &&
				(vendor == pstNode->vendor))
		{
			return pstNode;
		}
	}
	return NULL;
}


static modem_client_t * modem_client_lookup_node_by_name(char *name)
{
	modem_client_t *pstNode = NULL;
	NODE index;
	char product_name[MODEM_PRODUCT_NAME_MAX];
	assert(name);
	os_memset(product_name, 0, MODEM_PRODUCT_NAME_MAX);
	os_memcpy(product_name, name, MIN(MODEM_PRODUCT_NAME_MAX, os_strlen(name)));
	for(pstNode = (modem_client_t *)lstFirst(gModemClient.list);
			pstNode != NULL;  pstNode = (modem_client_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(os_memcmp(pstNode->module_name, product_name, MODEM_PRODUCT_NAME_MAX) == 0)
		{
			return pstNode;
		}
	}
	return NULL;
}

/*modem_client_t * modem_client_add_node(modem_client_t *client)
{
	assert(client);
	if(modem_client_lookup_node(client->vendor, client->product) == NULL )
	{
		modem_client_t *node = XMALLOC(MTYPE_MODEM_CLIENT, sizeof(modem_client_t));
		if(node)
		{
			os_memcpy(node, client, sizeof(modem_client_t));
			node->nw_register_type = NW_REGISTER_ENABLE_LOCAL;
			lstAdd(gModemClient.list, (NODE*)node);
			return OK;
		}
		if(MODEM_IS_DEBUG(CLIENT))
			zlog_debug(ZLOG_MODEM, "can't malloc modem client memory");
		return ERROR;
	}
	if(MODEM_IS_DEBUG(CLIENT))
		zlog_debug(ZLOG_MODEM, "modem client is exist, vendor:%x product:%x",
				client->vendor, client->product);
	return ERROR;
}*/

modem_client_t * modem_client_alloc(int vendor, int product)
{
	if(modem_client_lookup_node(vendor, product) == NULL )
	{
		modem_client_t *node = XMALLOC(MTYPE_MODEM_CLIENT, sizeof(modem_client_t));
		if(node)
		{
			os_memset(node, 0, sizeof(modem_client_t));
			node->vendor = vendor;
			node->product = product;
			node->nw_register_type = NW_REGISTER_ENABLE_LOCAL;
			return node;
		}
		if(MODEM_IS_DEBUG(CLIENT))
			zlog_debug(ZLOG_MODEM, "can't malloc modem client memory");
		return NULL;
	}
	if(MODEM_IS_DEBUG(CLIENT))
		zlog_debug(ZLOG_MODEM, "modem client is exist, vendor:%x product:%x",
				vendor, product);
	return NULL;
}

int modem_client_free(modem_client_t *client)
{
	assert(client);
	XFREE(MTYPE_MODEM, client);
	return OK;
}


/*modem_client_t *  modem_client_lookup_api(int product, int id)
{
	modem_client_t *node = NULL;
	if(gModemClient.mutex)
		os_mutex_lock(gModemClient.mutex, OS_WAIT_FOREVER);
	node = modem_client_lookup_node(product, id);
	if(gModemClient.mutex)
		os_mutex_unlock(gModemClient.mutex);
	return node;
}*/

modem_client_t *  modem_client_lookup_by_name_api(char *name)
{
	modem_client_t *node = NULL;
	assert(name);
	if(gModemClient.mutex)
		os_mutex_lock(gModemClient.mutex, OS_WAIT_FOREVER);
	node = modem_client_lookup_node_by_name(name);
	if(gModemClient.mutex)
		os_mutex_unlock(gModemClient.mutex);
	return node;
}

/*int modem_client_add_api(modem_client_t *client)
{
	int ret = ERROR;
	if(gModemClient.mutex)
		os_mutex_lock(gModemClient.mutex, OS_WAIT_FOREVER);
	ret = modem_client_add_node(client);
	if(gModemClient.mutex)
		os_mutex_unlock(gModemClient.mutex);
	return ret;
}*/

int modem_client_del_api(modem_client_t *client)
{
	int ret = ERROR;
	modem_client_t *node = NULL;
	assert(client);
	if(gModemClient.mutex)
		os_mutex_lock(gModemClient.mutex, OS_WAIT_FOREVER);
	node = modem_client_lookup_node(client->vendor, client->product);
	if(node)
	{
		lstDelete(gModemClient.list, (NODE*)node);
		XFREE(MTYPE_MODEM_CLIENT, node);
		ret = OK;
	}
	if(gModemClient.mutex)
		os_mutex_unlock(gModemClient.mutex);
	return ret;
}

int modem_client_del_by_product_api(int vendor, int product)
{
	int ret = ERROR;
	modem_client_t *node = NULL;
	if(gModemClient.mutex)
		os_mutex_lock(gModemClient.mutex, OS_WAIT_FOREVER);
	node = modem_client_lookup_node(vendor, product);
	if(node)
	{
		lstDelete(gModemClient.list, (NODE*)node);
		XFREE(MTYPE_MODEM_CLIENT, node);
		ret = OK;
	}
	if(gModemClient.mutex)
		os_mutex_unlock(gModemClient.mutex);
	return ret;
}

int modem_client_del_by_product_name_api(char *name)
{
	int ret = ERROR;
	modem_client_t *node = NULL;
	assert(name);
	if(gModemClient.mutex)
		os_mutex_lock(gModemClient.mutex, OS_WAIT_FOREVER);
	node = modem_client_lookup_node_by_name(name);
	if(node)
	{
		lstDelete(gModemClient.list, (NODE*)node);
		XFREE(MTYPE_MODEM_CLIENT, node);
		ret = OK;
	}
	if(gModemClient.mutex)
		os_mutex_unlock(gModemClient.mutex);
	return ret;
}

int modem_client_add_api(modem_client_t *client)
{
	int ret = 0;
	assert(client);
	if(gModemClient.mutex)
		os_mutex_lock(gModemClient.mutex, OS_WAIT_FOREVER);
	if(modem_client_lookup_node(client->vendor, client->product) != NULL )
	{

		if(gModemClient.mutex)
			os_mutex_unlock(gModemClient.mutex);
		return ERROR;
	}
	if(client)
	{
		lstAdd(gModemClient.list, (NODE*)client);
		ret = OK;
	}
	else
		ret = ERROR;
	if(gModemClient.mutex)
		os_mutex_unlock(gModemClient.mutex);
	return ret;
}

int modem_client_register_api(modem_client_t *client)
{
	int ret = 0;
	assert(client);
	if(gModemClient.mutex)
		os_mutex_lock(gModemClient.mutex, OS_WAIT_FOREVER);
	if(modem_client_lookup_node(client->vendor, client->product) != NULL )
	{

		if(gModemClient.mutex)
			os_mutex_unlock(gModemClient.mutex);
		return ERROR;
	}
	if(client)
	{
		lstAdd(gModemClient.list, (NODE*)client);
		ret = OK;
	}
	else
		ret = ERROR;
	if(gModemClient.mutex)
		os_mutex_unlock(gModemClient.mutex);
	return ret;
}

modem_client_t * modem_client_lookup_api(int vendor, int product)
{
	modem_client_t *pstNode = NULL;
	if(gModemClient.mutex)
		os_mutex_lock(gModemClient.mutex, OS_WAIT_FOREVER);
	pstNode = modem_client_lookup_node( vendor,  product);
	if(gModemClient.mutex)
		os_mutex_unlock(gModemClient.mutex);
	return pstNode;
}

int modem_client_callback_api(modem_client_cb cb, void *pVoid)
{
	modem_client_t *pstNode = NULL;
	NODE index;
	if(gModemClient.mutex)
		os_mutex_lock(gModemClient.mutex, OS_WAIT_FOREVER);
	for(pstNode = (modem_client_t *)lstFirst(gModemClient.list);
			pstNode != NULL;  pstNode = (modem_client_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode)
		{
			if(cb)
			{
				(cb)(pstNode, pVoid);
			}
		}
	}
	if(gModemClient.mutex)
		os_mutex_unlock(gModemClient.mutex);
	return OK;
}
