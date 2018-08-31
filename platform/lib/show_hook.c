/*
 * show_hook.c
 *
 *  Created on: 2017��7��16��
 *      Author: zhurish
 */
#include "zebra.h"
#include "buffer.h"
#include "command.h"
#include "if.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"
#include "vty.h"
#include "show_hook.h"


/* if callback list. */
struct list *show_hook_list = NULL;


static int show_hook_cmp_func (struct show_hook *hook1, struct show_hook *hook2);

void show_hook_init (void)
{
	show_hook_list = list_new ();
	assert (show_hook_list);
	show_hook_list->cmp = (int (*)(void *, void *))show_hook_cmp_func;
}

void show_hook_exit (void)
{
	if (show_hook_list == NULL)
	{
		struct show_hook *callback;
		for (;;)
		{
			callback = listnode_head (show_hook_list);
			if (callback == NULL)
				break;
			listnode_delete (show_hook_list, callback);
			XFREE (MTYPE_VTY_HOOK, callback);
		}
		//list_delete (if_hook_list);
		show_hook_list = NULL;
	}
}

static int show_hook_cmp_func (struct show_hook *hook1, struct show_hook *hook2)
{
	if(hook1->protocol > hook2->protocol)//�� protocol �ɴ�С����
		return 1;
	return 0;
}

static struct show_hook * show_hook_lookup(int protocol)
{
	struct listnode *node;
	struct show_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (show_hook_list, node, hook))
	{
	      if (hook && (hook->protocol == protocol))
	    	  return hook;
	}
	return NULL;
}

void show_hook_add (int protocol, int (*func)(struct vty *))
{
	int add = 0;
	struct show_hook *hook;
	hook = show_hook_lookup(protocol);
	if(hook == NULL)
	{
		hook = XMALLOC(MTYPE_VTY_HOOK, sizeof(struct show_hook));
		add = 1;
	}
	assert (hook);
	hook->protocol = protocol;
	hook->show_hook = func;
	if(add)
		listnode_add_sort (show_hook_list, hook);
}

void show_debug_hook_add (int protocol, int (*func)(struct vty *))
{
	int add = 0;
	struct show_hook *hook;
	hook = show_hook_lookup(protocol);
	if(hook == NULL)
	{
		hook = XMALLOC(MTYPE_VTY_HOOK, sizeof(struct show_hook));
		add = 1;
	}
	assert (hook);
	hook->protocol = protocol;
	hook->show_debug_hook = func;
	if(add)
		listnode_add_sort (show_hook_list, hook);
}

void show_interface_hook_add (int protocol, int (*func)(struct vty *,struct interface *))
{
	int add = 0;
	struct show_hook *hook;
	hook = show_hook_lookup(protocol);
	if(hook == NULL)
	{
		hook = XMALLOC(MTYPE_VTY_HOOK, sizeof(struct show_hook));
		add = 1;
	}
	assert (hook);
	hook->protocol = protocol;
	hook->show_interface_hook = func;
	if(add)
		listnode_add_sort (show_hook_list, hook);
}

void show_hook_all(struct vty *vty)
{
	struct listnode *node;
	struct show_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (show_hook_list, node, hook))
	{
		if(hook->show_hook)
			(hook->show_hook)(vty);
	}
	return;
}

void show_debug_hook_all(struct vty *vty)
{
	struct listnode *node;
	struct show_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (show_hook_list, node, hook))
	{
		if(hook->show_debug_hook)
			(hook->show_debug_hook)(vty);
	}
	return;
}

void show_interface_hook_all(struct vty *vty, struct interface *ifp)
{
	struct listnode *node;
	struct show_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (show_hook_list, node, hook))
	{
		if(hook->show_interface_hook)
			(hook->show_interface_hook)(vty, ifp);
	}
	return;
}
