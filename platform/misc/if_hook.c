/*
 * if_hook.c
 *
 *  Created on: 2017��7��8��
 *      Author: zhurish
 */
#include "zebra.h"
#include "vty.h"
#include "linklist.h"


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
#include "nsm_vrf.h"

#include "if_hook.h"
#include "if_name.h"


/* if callback list. */
struct list *if_hook_list = NULL;

static int if_hook_cmp_func (struct if_hook *hook1, struct if_hook *hook2);

void if_hook_init (void)
{
	if_hook_list = list_new ();
	assert (if_hook_list);
	if_hook_list->cmp = (int (*)(void *, void *))if_hook_cmp_func;
}

void if_hook_exit (void)
{
	if (if_hook_list == NULL)
	{
		struct if_hook *callback;
		for (;;)
		{
			callback = listnode_head (if_hook_list);
			if (callback == NULL)
				break;
			listnode_delete (if_hook_list, callback);
			XFREE (MTYPE_IF_HOOK, callback);
		}
		//list_delete (if_hook_list);
		if_hook_list = NULL;
	}
}

static int if_hook_cmp_func (struct if_hook *hook1, struct if_hook *hook2)
{
	if(hook1->protocol < hook2->protocol)//�� protocol ��С��������
		return 1;
	return 0;
}

struct if_hook * if_hook_lookup(int protocol)
{
	struct listnode *node;
	struct if_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (if_hook_list, node, hook))
	{
	      if (hook && (hook->protocol == protocol))
	    	  return hook;
	}
	return NULL;
}

void if_new_hook_call(int protocol, struct interface *ifp)
{
	struct listnode *node;
	struct if_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (if_hook_list, node, hook))
	{
	      if (hook && (hook->protocol == protocol))
	    	  if(hook->if_new_hook)
	    		  (hook->if_new_hook)(ifp);
	}
	return;
}

void if_delete_hook_call(int protocol, struct interface *ifp)
{
	struct listnode *node;
	struct if_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (if_hook_list, node, hook))
	{
	      if (hook && (hook->protocol == protocol))
	    	  if(hook->if_delete_hook)
	    		  (hook->if_delete_hook)(ifp);
	}
	return;
}


void if_address_add_hook_call(int protocol, struct interface *ifp, struct connected *p)
{
	struct listnode *node;
	struct if_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (if_hook_list, node, hook))
	{
	      if (hook && (hook->protocol == protocol))
	    	  if(hook->if_address_add_hook)
	    		  (hook->if_address_add_hook)(ifp, p);
	}
	return;
}

void if_address_delete_hook_call(int protocol, struct interface *ifp, struct connected *p)
{
	struct listnode *node;
	struct if_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (if_hook_list, node, hook))
	{
	      if (hook && (hook->protocol == protocol))
	    	  if(hook->if_address_del_hook)
	    		  (hook->if_address_del_hook)(ifp, p);
	}
	return;
}

void if_status_up_hook_call(int protocol, struct interface *ifp)
{
	struct listnode *node;
	struct if_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (if_hook_list, node, hook))
	{
	      if (hook && (hook->protocol == protocol))
	    	  if(hook->if_status_up_hook)
	    		  (hook->if_status_up_hook)(ifp);
	}
	return;
}

void if_status_down_hook_call(int protocol, struct interface *ifp)
{
	struct listnode *node;
	struct if_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (if_hook_list, node, hook))
	{
	      if (hook && (hook->protocol == protocol))
	    	  if(hook->if_status_down_hook)
	    		  (hook->if_status_down_hook)(ifp);
	}
	return;
}

/*void if_show_hook_call(int protocol, struct interface *ifp)
{
	struct listnode *node;
	struct if_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (if_hook_list, node, hook))
	{
	      if (hook && (hook->protocol == protocol))
	    	  if(hook->if_show_hook)
	    		  (hook->if_show_hook)(ifp);
	}
	return;
}*/

void if_hook_all(int cmd, struct interface *ifp)
{
	struct listnode *node;
	struct if_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (if_hook_list, node, hook))
	{
	      if (hook && (IF_NEW_HOOK == cmd))
	      {
	    	  if(hook->if_new_hook)
	    		  (hook->if_new_hook)(ifp);
	      }
	      else if (hook && (IF_DELETE_HOOK == cmd))
	      {
	    	  if(hook->if_delete_hook)
	    		  (hook->if_delete_hook)(ifp);
	      }
	}
	return;
}

void if_address_hook_all(int cmd, struct interface *ifp, struct connected *p)
{
	struct listnode *node;
	struct if_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (if_hook_list, node, hook))
	{
	      if (hook && (IF_ADDR_NEW_HOOK == cmd))
	      {
	    	  if(hook->if_address_add_hook)
	    		  (hook->if_address_add_hook)(ifp, p);
	      }
	      else if (hook && (IF_ADDR_DELETE_HOOK == cmd))
	      {
	    	  if(hook->if_address_del_hook)
	    		  (hook->if_address_del_hook)(ifp, p);
	      }
	}
	return;
}

void if_status_hook_all(int cmd, struct interface *ifp)
{
	struct listnode *node;
	struct if_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (if_hook_list, node, hook))
	{
	      if (hook && (IF_STATUS_UP_HOOK == cmd))
	      {
	    	  if(hook->if_status_up_hook)
	    		  (hook->if_status_up_hook)(ifp);
	      }
	      else if (hook && (IF_STATUS_DOWN_HOOK == cmd))
	      {
	    	  if(hook->if_status_down_hook)
	    		  (hook->if_status_down_hook)(ifp);
	      }
	}
	return;
}

/*void if_show_hook_all(struct interface *ifp)
{
#if 1
	int i = 0;
	for(i = ZEBRA_ROUTE_SYSTEM; i < ZEBRA_ROUTE_MAX; i++)
	{
		if_show_hook_call(i, ifp);
	}
	return;
#else
	struct listnode *node;
	struct if_hook *hook;
	for (ALL_LIST_ELEMENTS_RO (if_hook_list, node, hook))
	{
		if(hook->if_show_hook)
			(hook->if_show_hook)(ifp);
	}
	return;
#endif
}*/


void if_add_hook (int cmd, int protocol, int (*func)(struct interface *))
{
	int add = 0;
	struct if_hook *hook;
	hook = if_hook_lookup(protocol);
	if(hook == NULL)
	{
		hook = XMALLOC(MTYPE_IF_HOOK, sizeof(struct if_hook));
		add = 1;
	}
	assert (hook);
	if(cmd == IF_NEW_HOOK)
	{
		hook->protocol = protocol;
		hook->if_new_hook = func;
//		OS_DEBUG("%s:%d add\r\n",__func__,protocol);
		/* Add connected address to the interface. */
//		listnode_add (if_hook_list, hook);
	}
	else if(cmd == IF_DELETE_HOOK)
	{
		hook->protocol = protocol;
		hook->if_delete_hook = func;
//		OS_DEBUG("%s:%d del\r\n",__func__,protocol);
		/* Add connected address to the interface. */
//		listnode_add (if_hook_list, hook);
	}
	if(add)
		listnode_add_sort (if_hook_list, hook);

	//listnode_add_before (struct list *list, struct listnode *pp, void *val)
	//listnode_add (if_hook_list, hook);
}

void if_status_hook (int cmd, int protocol, int (*func)(struct interface *))
{
	int add = 0;
	struct if_hook *hook;
	hook = if_hook_lookup(protocol);
	if(hook == NULL)
	{
		hook = XMALLOC(MTYPE_IF_HOOK, sizeof(struct if_hook));
		add = 1;
	}
	assert (hook);
	if(cmd == IF_STATUS_UP_HOOK)
	{
		hook->protocol = protocol;
		hook->if_status_up_hook = func;
		/* Add connected address to the interface. */
//		listnode_add (if_hook_list, hook);
	}
	else if(cmd == IF_STATUS_DOWN_HOOK)
	{
		hook->protocol = protocol;
		hook->if_status_down_hook = func;
		/* Add connected address to the interface. */
//		listnode_add (if_hook_list, hook);
	}
	if(add)
		listnode_add_sort (if_hook_list, hook);
}

void if_address_hook (int cmd, int protocol, int (*func)(struct interface *,struct connected *))
{
	int add = 0;
	struct if_hook *hook;
	hook = if_hook_lookup(protocol);
	if(hook == NULL)
	{
		hook = XMALLOC(MTYPE_IF_HOOK, sizeof(struct if_hook));
		add = 1;
	}
	assert (hook);
	if(cmd == IF_ADDR_NEW_HOOK)
	{
		hook->protocol = protocol;
		hook->if_address_add_hook = func;
		/* Add connected address to the interface. */
//		listnode_add (if_hook_list, hook);
	}
	else if(cmd == IF_ADDR_DELETE_HOOK)
	{
		hook->protocol = protocol;
		hook->if_address_del_hook = func;
		/* Add connected address to the interface. */
//		listnode_add (if_hook_list, hook);
	}
	if(add)
		listnode_add_sort (if_hook_list, hook);
}
/*
void if_show_hook (int protocol, int (*func)(struct interface *))
{

	int add = 0;
	struct if_hook *hook;
	hook = if_hook_lookup(protocol);
	if(hook == NULL)
	{
		hook = XMALLOC(MTYPE_IF_HOOK, sizeof(struct if_hook));
		add = 1;
	}
	assert (hook);
	//if(cmd == IF_SHOW_HOOK)
	{
		hook->protocol = protocol;
		hook->if_show_hook = func;
	}
	if(add)
		listnode_add_sort (if_hook_list, hook);
}*/
