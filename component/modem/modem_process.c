/*
 * modem_process.c
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
#include "modem_event.h"
#include "modem_serial.h"
#include "modem_process.h"



static modem_main_process_t gModeProcess;


int modem_process_init(void)
{
	os_memset(&gModeProcess, 0, sizeof(modem_main_process_t));
	gModeProcess.list = XMALLOC(MTYPE_MODEM, sizeof(LIST));
	gModeProcess.unlist = XMALLOC(MTYPE_MODEM, sizeof(LIST));
	gModeProcess.ready = XMALLOC(MTYPE_MODEM, sizeof(LIST));
	if(gModeProcess.list && gModeProcess.unlist && gModeProcess.ready)
	{
		gModeProcess.mutex = os_mutex_init();
		lstInit(gModeProcess.list);
		lstInit(gModeProcess.unlist);
		lstInit(gModeProcess.ready);
		return OK;
	}
	if(gModeProcess.ready)
	{
		if(lstCount(gModeProcess.ready))
			lstFree(gModeProcess.ready);
		XFREE(MTYPE_MODEM, gModeProcess.ready);
	}
	if(gModeProcess.list)
	{
		if(lstCount(gModeProcess.list))
			lstFree(gModeProcess.list);
		XFREE(MTYPE_MODEM, gModeProcess.list);
	}
	if(gModeProcess.unlist)
	{
		if(lstCount(gModeProcess.unlist))
			lstFree(gModeProcess.unlist);
		XFREE(MTYPE_MODEM, gModeProcess.unlist);
	}
	return ERROR;
}

int modem_process_exit(void)
{
	if(gModeProcess.ready)
	{
		if(lstCount(gModeProcess.ready))
			lstFree(gModeProcess.ready);
		XFREE(MTYPE_MODEM, gModeProcess.ready);
	}
	if(gModeProcess.list)
	{
		if(lstCount(gModeProcess.list))
			lstFree(gModeProcess.list);
		XFREE(MTYPE_MODEM, gModeProcess.list);
	}
	if(gModeProcess.unlist)
	{
		if(lstCount(gModeProcess.unlist))
			lstFree(gModeProcess.unlist);
		XFREE(MTYPE_MODEM, gModeProcess.unlist);
	}
	if(gModeProcess.mutex)
		os_mutex_exit(gModeProcess.mutex);
	return OK;
}


static modem_process_t *modem_process_get(modem_event event, void *argv)
{
	modem_process_t *node = NULL;
	if(gModeProcess.unlist)
	{
		if(lstCount(gModeProcess.unlist))
		{
			node = (modem_process_t *)lstFirst(gModeProcess.unlist);
			if(node)
			{
				lstDelete(gModeProcess.unlist, (NODE*)node);
				node->argv = argv;
				node->event = event;
				return node;
			}
		}
	}
	node = XMALLOC(MTYPE_MODEM, sizeof(modem_process_t));
	if(node)
	{
		os_memset(node, 0, sizeof(modem_process_t));
		node->argv = argv;
		node->event = event;
		return node;
	}
	return node;
}


static int modem_process_lookup_node(modem_event event, void *argv)
{
	NODE index;
	modem_process_t *pstNode = NULL;

	if(lstCount(gModeProcess.list))
	{
		for(pstNode = (modem_process_t *)lstFirst(gModeProcess.list);
				pstNode != NULL;  pstNode = (modem_process_t *)lstNext((NODE*)&index))
		{
			index = pstNode->node;
			if(pstNode)
			{
				if((pstNode->event == event) && (pstNode->argv == argv) )
					return 1;
			}
		}
	}
	return 0;
}
static int modem_process_add_node(modem_event event, void *argv)
{
	modem_process_t *node = modem_process_get(event, argv);
	if(node)
	{
		//node->ready = 0;
		if(modem_process_lookup_node(event, argv) == 0)
			lstAdd(gModeProcess.list, (NODE*)node);
		else
			lstAdd(gModeProcess.unlist, (NODE*)node);
		return OK;
	}
	return ERROR;
}


static int modem_process_del_node(modem_process_t *node)
{
	if(node)
	{
		lstDelete(gModeProcess.ready, (NODE*)node);
		lstAdd(gModeProcess.unlist, (NODE*)node);
		return OK;
	}
	return ERROR;
}



int modem_process_add_api(modem_event event, void *argv, BOOL lock)
{
	int ret = 0;
	NODE index;
	modem_process_t *pstNode = NULL;
	if(lock/*event == MODEM_EV_INSTER || event == MODEM_EV_REMOVE*/)
	{
		if(gModeProcess.mutex)
			os_mutex_lock(gModeProcess.mutex, OS_WAIT_FOREVER);
	}
	ret = modem_process_add_node( event, argv);

	if(lstCount(gModeProcess.ready))
	{
		if(lock/*event == MODEM_EV_INSTER || event == MODEM_EV_REMOVE*/)
		{
			if(gModeProcess.mutex)
				os_mutex_unlock(gModeProcess.mutex);
		}
		return ret;
	}
	if(lstCount(gModeProcess.list))
	{
		for(pstNode = (modem_process_t *)lstFirst(gModeProcess.list);
				pstNode != NULL;  pstNode = (modem_process_t *)lstNext((NODE*)&index))
		{
			index = pstNode->node;
			if(pstNode)
			{
				lstDelete(gModeProcess.list, (NODE*)pstNode);
				lstAdd(gModeProcess.ready, (NODE*)pstNode);
			}
		}
	}
	if(lock/*event == MODEM_EV_INSTER || event == MODEM_EV_REMOVE*/)
	{
		if(gModeProcess.mutex)
			os_mutex_unlock(gModeProcess.mutex);
	}
	return ret;
}

int modem_process_callback_api(modem_process_cb cb, void *pVoid)
{
	NODE index;
	modem_process_t *pstNode = NULL;
	if(gModeProcess.mutex)
		os_mutex_lock(gModeProcess.mutex, OS_WAIT_FOREVER);
	if(lstCount(gModeProcess.ready) !=  0)
	{
		for(pstNode = (modem_process_t *)lstFirst(gModeProcess.ready);
				pstNode != NULL;  pstNode = (modem_process_t *)lstNext((NODE*)&index))
		{
			index = pstNode->node;
			if(cb && pstNode)
			{
				//MODEM_DEBUG("modem process event:%s",modem_event_string(pstNode->event));
				(cb)(pstNode, pVoid);
			}
			modem_process_del_node(pstNode);
		}
	}
	if(lstCount(gModeProcess.list) !=  0)
	{
		for(pstNode = (modem_process_t *)lstFirst(gModeProcess.list);
				pstNode != NULL;  pstNode = (modem_process_t *)lstNext((NODE*)&index))
		{
			index = pstNode->node;
			if(pstNode)
			{
				lstDelete(gModeProcess.list, (NODE*)pstNode);
				lstAdd(gModeProcess.ready, (NODE*)pstNode);
			}
		}
	}
	if(gModeProcess.mutex)
		os_mutex_unlock(gModeProcess.mutex);
	return OK;
}
