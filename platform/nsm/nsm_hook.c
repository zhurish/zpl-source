/*
 * nsm_hook.c
 *
 *  Created on: 2019年7月30日
 *      Author: DELL
 */

#include "zebra.h"
#include "buffer.h"
#include "if.h"
#include "log.h"
#include "memory.h"
#include "network.h"
#include "prefix.h"
#include "stream.h"
#include "table.h"
#include "thread.h"
#include "vty.h"
#include "nsm_zserv.h"
#include "nsm_hook.h"

#include "pal_driver.h"
#ifdef PL_HAL_MODULE
#include "hal_port.h"
#endif


static struct nsm_hook *nsm_hook_lst = NULL;


int nsm_hook_module_init (void)
{
	if(nsm_hook_lst == NULL)
		nsm_hook_lst = XCALLOC (MTYPE_HOOK, sizeof (struct nsm_hook));
	return OK;
}

void nsm_hook_module_exit (void)
{
	if(nsm_hook_lst)
	{
		if(nsm_hook_lst->ifplist)
		{
			XFREE (MTYPE_NSM_HOOK, nsm_hook_lst->ifplist);
			nsm_hook_lst->ifplist = NULL;
		}
		if(nsm_hook_lst->ifp_state_list)
		{
			XFREE (MTYPE_NSM_HOOK, nsm_hook_lst->ifp_state_list);
			nsm_hook_lst->ifp_state_list = NULL;
		}
		if(nsm_hook_lst->ifp_address_list)
		{
			XFREE (MTYPE_NSM_HOOK, nsm_hook_lst->ifp_address_list);
			nsm_hook_lst->ifp_address_list = NULL;
		}
		if(nsm_hook_lst->ifp_show_list)
		{
			XFREE (MTYPE_NSM_HOOK, nsm_hook_lst->ifp_show_list);
			nsm_hook_lst->ifp_show_list = NULL;
		}
		if(nsm_hook_lst->ifp_write_list)
		{
			XFREE (MTYPE_NSM_HOOK, nsm_hook_lst->ifp_write_list);
			nsm_hook_lst->ifp_write_list = NULL;
		}
		if(nsm_hook_lst->service_list)
		{
			XFREE (MTYPE_NSM_HOOK, nsm_hook_lst->service_list);
			nsm_hook_lst->service_list = NULL;
		}
		if(nsm_hook_lst->debug_list)
		{
			XFREE (MTYPE_NSM_HOOK, nsm_hook_lst->debug_list);
			nsm_hook_lst->debug_list = NULL;
		}
		XFREE (MTYPE_HOOK, nsm_hook_lst);
		nsm_hook_lst = NULL;
	}
}


void * nsm_hook_lookup (nsm_hook_em type)
{
	int i = 0;
	nsm_hook_node_t *hooklist = NULL;
	switch(type)
	{
	case NSM_HOOK_IFP_ADD:	//增加接口
	case NSM_HOOK_IFP_DEL:	//删除接口
		hooklist = nsm_hook_lst->ifplist;
		if(hooklist == NULL)
			return NULL;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == (nsm_hook_em)type && hooklist[i].hook.ifp_cb)
				return hooklist[i].hook.ifp_cb;
		}
		break;
	case NSM_HOOK_IFP_UP:	//接口UP/DOWN
	case NSM_HOOK_IFP_DOWN:
		hooklist = nsm_hook_lst->ifp_state_list;
		if(hooklist == NULL)
			return NULL;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_state_cb)
				return hooklist[i].hook.ifp_state_cb;
		}
		break;
	case NSM_HOOK_IP_ADD:	//接口添加IP
	case NSM_HOOK_IP_DEL:	//接口删除IP
		hooklist = nsm_hook_lst->ifp_address_list;
		if(hooklist == NULL)
			return NULL;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_address_cb)
				return hooklist[i].hook.ifp_address_cb;
		}
		break;
	case NSM_HOOK_IFP_CHANGE://接口参数变化
		hooklist = nsm_hook_lst->ifp_change_list;
		if(hooklist == NULL)
			return NULL;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_change_cb)
				return hooklist[i].hook.ifp_change_cb;
		}
		break;
	case NSM_HOOK_IFP_SHOW:	//show 接口信息
		hooklist = nsm_hook_lst->ifp_show_list;
		if(hooklist == NULL)
			return NULL;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_show_cb)
				return hooklist[i].hook.ifp_show_cb;
		}
		break;
	case NSM_HOOK_IFP_CONFIG://write config 接口信息
		hooklist = nsm_hook_lst->ifp_write_list;
		if(hooklist == NULL)
			return NULL;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_write_cb)
				return hooklist[i].hook.ifp_write_cb;
		}
		break;
	case NSM_HOOK_SERVICE:
		hooklist = nsm_hook_lst->service_list;
		if(hooklist == NULL)
			return NULL;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.service_cb)
				return hooklist[i].hook.service_cb;
		}
		break;
	case NSM_HOOK_DEBUG:
		hooklist = nsm_hook_lst->debug_list;
		if(hooklist == NULL)
			return NULL;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.debug_cb)
				return hooklist[i].hook.debug_cb;
		}
		break;
	default:
		break;
	}
	return NULL;
}


int nsm_hook_install (nsm_hook_em type, void *hook)
{
	int i = 0;
	nsm_hook_node_t *hooklist = NULL;
	switch(type)
	{
	case NSM_HOOK_IFP_ADD:	//增加接口
	case NSM_HOOK_IFP_DEL:	//删除接口
		hooklist = nsm_hook_lst->ifplist;
		if(hooklist == NULL)
		{
			nsm_hook_lst->ifplist = XCALLOC (MTYPE_NSM_HOOK, sizeof (nsm_hook_node_t) * NSM_HOOK_TBL_MAX);
			hooklist = nsm_hook_lst->ifplist;
		}
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_cb == hook)
			{
				return OK;
			}
		}
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == 0)
			{
				hooklist[i].hook.ifp_cb = hook;
				hooklist[i].type = type;
				return OK;
			}
		}
		break;
	case NSM_HOOK_IFP_UP:	//接口UP/DOWN
	case NSM_HOOK_IFP_DOWN:
		hooklist = nsm_hook_lst->ifp_state_list;
		if(hooklist == NULL)
		{
			nsm_hook_lst->ifp_state_list = XCALLOC (MTYPE_NSM_HOOK, sizeof (nsm_hook_node_t) * NSM_HOOK_TBL_MAX);
			hooklist = nsm_hook_lst->ifp_state_list;
		}
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_state_cb == hook)
			{
				return OK;
			}
		}
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == 0)
			{
				hooklist[i].hook.ifp_state_cb = hook;
				hooklist[i].type = type;
				return OK;
			}
		}
		break;
	case NSM_HOOK_IP_ADD:	//接口添加IP
	case NSM_HOOK_IP_DEL:	//接口删除IP
		hooklist = nsm_hook_lst->ifp_address_list;
		if(hooklist == NULL)
		{
			nsm_hook_lst->ifp_address_list = XCALLOC (MTYPE_NSM_HOOK, sizeof (nsm_hook_node_t) * NSM_HOOK_TBL_MAX);
			hooklist = nsm_hook_lst->ifp_address_list;
		}
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_address_cb == hook)
			{
				return OK;
			}
		}
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == 0)
			{
				hooklist[i].hook.ifp_address_cb = hook;
				hooklist[i].type = type;
				return OK;
			}
		}
		break;
	case NSM_HOOK_IFP_CHANGE://接口参数变化
		hooklist = nsm_hook_lst->ifp_change_list;
		if(hooklist == NULL)
		{
			nsm_hook_lst->ifp_change_list = XCALLOC (MTYPE_NSM_HOOK, sizeof (nsm_hook_node_t) * NSM_HOOK_TBL_MAX);
			hooklist = nsm_hook_lst->ifp_change_list;
		}
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_change_cb == hook)
			{
				return OK;
			}
		}
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == 0)
			{
				hooklist[i].hook.ifp_change_cb = hook;
				hooklist[i].type = type;
				return OK;
			}
		}
		break;
	case NSM_HOOK_IFP_SHOW:	//show 接口信息
		hooklist = nsm_hook_lst->ifp_show_list;
		if(hooklist == NULL)
		{
			nsm_hook_lst->ifp_show_list = XCALLOC (MTYPE_NSM_HOOK, sizeof (nsm_hook_node_t) * NSM_HOOK_TBL_MAX);
			hooklist = nsm_hook_lst->ifp_show_list;
		}
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_show_cb == hook)
			{
				return OK;
			}
		}
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == 0)
			{
				hooklist[i].hook.ifp_show_cb = hook;
				hooklist[i].type = type;
				return OK;
			}
		}
		break;
	case NSM_HOOK_IFP_CONFIG://write config 接口信息
		hooklist = nsm_hook_lst->ifp_write_list;
		if(hooklist == NULL)
		{
			nsm_hook_lst->ifp_write_list = XCALLOC (MTYPE_NSM_HOOK, sizeof (nsm_hook_node_t) * NSM_HOOK_TBL_MAX);
			hooklist = nsm_hook_lst->ifp_write_list;
		}
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_write_cb == hook)
			{
				return OK;
			}
		}
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == 0)
			{
				hooklist[i].hook.ifp_write_cb = hook;
				hooklist[i].type = type;
				return OK;
			}
		}
		break;
	case NSM_HOOK_SERVICE:
		hooklist = nsm_hook_lst->service_list;
		if(hooklist == NULL)
		{
			nsm_hook_lst->service_list = XCALLOC (MTYPE_NSM_HOOK, sizeof (nsm_hook_node_t) * NSM_HOOK_TBL_MAX);
			hooklist = nsm_hook_lst->service_list;
		}
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.service_cb == hook)
			{
				return OK;
			}
		}
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == 0)
			{
				hooklist[i].hook.service_cb = hook;
				hooklist[i].type = type;
				return OK;
			}
		}
		break;
	case NSM_HOOK_DEBUG:
		hooklist = nsm_hook_lst->debug_list;
		if(hooklist == NULL)
		{
			nsm_hook_lst->debug_list = XCALLOC (MTYPE_NSM_HOOK, sizeof (nsm_hook_node_t) * NSM_HOOK_TBL_MAX);
			hooklist = nsm_hook_lst->debug_list;
		}
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.debug_cb == hook)
			{
				return OK;
			}
		}
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == 0)
			{
				hooklist[i].hook.debug_cb = hook;
				hooklist[i].type = type;
				return OK;
			}
		}
		break;
	default:
		break;
	}
	return ERROR;
}

int nsm_hook_uninstall (nsm_hook_em type, void *hook)
{
	int i = 0;
	nsm_hook_node_t *hooklist = NULL;
	switch(type)
	{
	case NSM_HOOK_IFP_ADD:	//增加接口
	case NSM_HOOK_IFP_DEL:	//删除接口
		hooklist = nsm_hook_lst->ifplist;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_cb == hook)
			{
				hooklist[i].type = NSM_HOOK_NONE;
				hooklist[i].hook.ifp_cb = NULL;
				return OK;
			}
		}
		break;
	case NSM_HOOK_IFP_UP:	//接口UP/DOWN
	case NSM_HOOK_IFP_DOWN:
		hooklist = nsm_hook_lst->ifp_state_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_state_cb == hook)
			{
				hooklist[i].type = NSM_HOOK_NONE;
				hooklist[i].hook.ifp_state_cb = NULL;
				return OK;
			}
		}
		break;
	case NSM_HOOK_IP_ADD:	//接口添加IP
	case NSM_HOOK_IP_DEL:	//接口删除IP
		hooklist = nsm_hook_lst->ifp_address_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_address_cb == hook)
			{
				hooklist[i].type = NSM_HOOK_NONE;
				hooklist[i].hook.ifp_address_cb = NULL;
				return OK;
			}
		}
		break;
	case NSM_HOOK_IFP_CHANGE://接口参数变化
		hooklist = nsm_hook_lst->ifp_change_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_change_cb == hook)
			{
				hooklist[i].type = NSM_HOOK_NONE;
				hooklist[i].hook.ifp_change_cb = NULL;
				return OK;
			}
		}
		break;
	case NSM_HOOK_IFP_SHOW:	//show 接口信息
		hooklist = nsm_hook_lst->ifp_show_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_show_cb == hook)
			{
				hooklist[i].type = NSM_HOOK_NONE;
				hooklist[i].hook.ifp_show_cb = NULL;
				return OK;
			}
		}
		break;
	case NSM_HOOK_IFP_CONFIG://write config 接口信息
		hooklist = nsm_hook_lst->ifp_write_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_write_cb == hook)
			{
				hooklist[i].type = NSM_HOOK_NONE;
				hooklist[i].hook.ifp_write_cb = NULL;
				return OK;
			}
		}
		break;
	case NSM_HOOK_SERVICE:
		hooklist = nsm_hook_lst->service_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.service_cb == hook)
			{
				hooklist[i].type = NSM_HOOK_NONE;
				hooklist[i].hook.service_cb = NULL;
				return OK;
			}
		}
		break;
	case NSM_HOOK_DEBUG:
		hooklist = nsm_hook_lst->debug_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.debug_cb == hook)
			{
				hooklist[i].type = NSM_HOOK_NONE;
				hooklist[i].hook.debug_cb = NULL;
				return OK;
			}
		}
		break;
	default:
		break;
	}
	return ERROR;
}


int nsm_hook_execute (nsm_hook_em type, void *p1, void *p2, BOOL b)
{
	int i = 0;
	nsm_hook_node_t *hooklist = NULL;
	switch(type)
	{
	case NSM_HOOK_IFP_ADD:	//增加接口
	case NSM_HOOK_IFP_DEL:	//删除接口
		hooklist = nsm_hook_lst->ifplist;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_cb)
				(hooklist[i].hook.ifp_cb)(p1, b);
		}
		break;
	case NSM_HOOK_IFP_UP:	//接口UP/DOWN
	case NSM_HOOK_IFP_DOWN:
		hooklist = nsm_hook_lst->ifp_state_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_state_cb)
				(hooklist[i].hook.ifp_state_cb)(p1, b);
		}
		break;
	case NSM_HOOK_IP_ADD:	//接口添加IP
	case NSM_HOOK_IP_DEL:	//接口删除IP
		hooklist = nsm_hook_lst->ifp_address_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_address_cb)
				(hooklist[i].hook.ifp_address_cb)(p1, p2, b);
		}
		break;
	case NSM_HOOK_IFP_CHANGE://接口参数变化
		hooklist = nsm_hook_lst->ifp_change_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_change_cb)
				(hooklist[i].hook.ifp_change_cb)(p1, b);
		}
		break;
	case NSM_HOOK_IFP_SHOW:	//show 接口信息
		hooklist = nsm_hook_lst->ifp_show_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_show_cb)
				(hooklist[i].hook.ifp_show_cb)(p1, p2, b);
		}
		break;
	case NSM_HOOK_IFP_CONFIG://write config 接口信息
		hooklist = nsm_hook_lst->ifp_write_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.ifp_write_cb)
				(hooklist[i].hook.ifp_write_cb)(p1, p2, b);
		}
		break;
	case NSM_HOOK_SERVICE:
		hooklist = nsm_hook_lst->service_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.service_cb)
				(hooklist[i].hook.service_cb)(p1, b);
		}
		break;
	case NSM_HOOK_DEBUG:
		hooklist = nsm_hook_lst->debug_list;
		if(hooklist == NULL)
			return ERROR;
		for (i = 0; i < NSM_HOOK_TBL_MAX; i ++)
		{
			if(hooklist[i].type == type && hooklist[i].hook.debug_cb)
				(hooklist[i].hook.debug_cb)(p1, b);
		}
		break;
	default:
		break;
	}
	return OK;
}
