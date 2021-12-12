/*
 * bmgt.c
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */
#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "bmgt.h"

static LIST *unit_board_mgt_list = NULL;
static os_mutex_t *unit_board_mgt_mutex = NULL;

int unit_board_init()
{
	if (unit_board_mgt_list == NULL)
	{
		unit_board_mgt_list = os_malloc(sizeof(LIST));
		if (unit_board_mgt_list)
		{
			lstInit(unit_board_mgt_list);
		}
	}
	if (unit_board_mgt_mutex == NULL)
	{
		unit_board_mgt_mutex = os_mutex_init();
	}
	return OK;
}

int unit_board_exit()
{
	if (unit_board_mgt_mutex)
	{
		if (os_mutex_exit(unit_board_mgt_mutex) == OK)
			unit_board_mgt_mutex = NULL;
	}
	if (unit_board_mgt_list)
	{
		lstFree(unit_board_mgt_list);
		unit_board_mgt_list = NULL;
	}
	return OK;
}

unit_board_mgt_t *unit_board_add(zpl_uint8 unit, zpl_uint8 slot)
{
	unit_board_mgt_t *t = NULL;
	if (t == NULL)
		t = os_malloc(sizeof(unit_board_mgt_t));
	if (t)
	{
		// t->type = (zpl_uint32)type;
		t->unit = unit;
		t->slot = slot;
		// t->port = port;
		// t->phyid = phyid;
		t->port_list = os_malloc(sizeof(LIST));
		if (t->port_list)
		{
			lstInitFree(t->port_list, free);
		}
		t->mutex = os_mutex_init();
		t->state = UBMG_STAT_INIT;
		if (unit_board_mgt_mutex)
			os_mutex_lock(unit_board_mgt_mutex, OS_WAIT_FOREVER);
		if (unit_board_mgt_list)
			lstAdd(unit_board_mgt_list, (NODE *)t);
		if (unit_board_mgt_mutex)
			os_mutex_unlock(unit_board_mgt_mutex);
		return t;
	}
	return NULL;
}

int unit_board_del(zpl_uint8 unit, zpl_uint8 slot)
{
	NODE node;
	unit_board_mgt_t *t;
	if (unit_board_mgt_mutex)
		os_mutex_lock(unit_board_mgt_mutex, OS_WAIT_FOREVER);

	for (t = (unit_board_mgt_t *)lstFirst(unit_board_mgt_list); t != NULL; t = (unit_board_mgt_t *)lstNext(&node))
	{
		node = t->node;
		if (t && t->unit == unit && t->slot == slot)
		{
			lstDelete(unit_board_mgt_list, (NODE *)t);
			if (t->mutex)
				os_mutex_lock(t->mutex, OS_WAIT_FOREVER);
			if (t->port_list)
				lstFree(t->port_list);
			if (t->mutex)
				os_mutex_unlock(t->mutex);
			if (t->port_list)
				free(t->port_list);
			if (t->mutex)
				os_mutex_exit(t->mutex);
			free(t);
			break;
		}
	}
	if (unit_board_mgt_mutex)
		os_mutex_unlock(unit_board_mgt_mutex);
	return OK;
}

int unit_board_foreach(int (*func)(unit_board_mgt_t *, void *), void *p)
{
	NODE node;
	unit_board_mgt_t *t;
	if (unit_board_mgt_mutex)
		os_mutex_lock(unit_board_mgt_mutex, OS_WAIT_FOREVER);

	for (t = (unit_board_mgt_t *)lstFirst(unit_board_mgt_list); t != NULL; t = (unit_board_mgt_t *)lstNext(&node))
	{
		node = t->node;
		if (t && func)
		{
			(func)(t, p);
		}
	}
	if (unit_board_mgt_mutex)
		os_mutex_unlock(unit_board_mgt_mutex);
	return OK;
}

unit_port_mgt_t *unit_board_port_add(unit_board_mgt_t *board, zpl_uint8 type, zpl_uint8 port, zpl_uint32 phyid)
{
	unit_port_mgt_t *t = NULL;
	if (t == NULL)
		t = os_malloc(sizeof(unit_port_mgt_t));
	if (t)
	{
		t->type = (zpl_uint32)type;
		t->port = port;
		t->phyid = phyid;
		if (board->mutex)
			os_mutex_lock(board->mutex, OS_WAIT_FOREVER);
		if (board->port_list)
			lstAdd(board->port_list, (NODE *)t);
		if (board->mutex)
			os_mutex_unlock(board->mutex);
		return t;
	}
	return NULL;
}

int unit_board_port_del(unit_board_mgt_t *board, zpl_uint8 type, zpl_uint8 port, zpl_uint32 phyid)
{
	NODE node;
	unit_port_mgt_t *t;
	if (board->mutex)
		os_mutex_lock(board->mutex, OS_WAIT_FOREVER);
	for (t = (unit_port_mgt_t *)lstFirst(board->port_list); t != NULL; t = (unit_port_mgt_t *)lstNext(&node))
	{
		node = t->node;
		if (t && t->type == type && t->port == port && t->phyid == phyid)
		{
			lstDelete(board->port_list, (NODE *)t);
			break;
		}
	}
	if (board->mutex)
		os_mutex_unlock(board->mutex);
	return OK;
}

int unit_board_port_foreach(unit_board_mgt_t *board, int (*func)(unit_port_mgt_t *, void *), void *p)
{
	NODE node;
	unit_port_mgt_t *t;
	if (board->mutex)
		os_mutex_lock(board->mutex, OS_WAIT_FOREVER);

	for (t = (unit_port_mgt_t *)lstFirst(board->port_list); t != NULL; t = (unit_port_mgt_t *)lstNext(&node))
	{
		node = t->node;
		if (t && func)
		{
			(func)(t, p);
		}
	}
	if (board->mutex)
		os_mutex_unlock(board->mutex);
	return OK;
}



int unit_board_show(void *pvoid)
{
	zpl_uint32 i = 0;
	NODE *node;
	unit_board_mgt_t *t;
#ifdef ZPL_SHELL_MODULE
	struct vty *vty = (struct vty *)pvoid;
#endif
	if (unit_board_mgt_mutex)
		os_mutex_lock(unit_board_mgt_mutex, OS_WAIT_FOREVER);
#ifdef ZPL_SHELL_MODULE
	if (lstCount(unit_board_mgt_list))
	{
		vty_out(vty, "%-4s %-4s %-4s %-4s %-4s %s", "----", "----", "----", "----", "----", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-4s %-4s %-4s %s", "TYPE", "UNIT", "SLOT", "PORT", "STAT", VTY_NEWLINE);
		vty_out(vty, "%-4s %-4s %-4s %-4s %-4s %s", "----", "----", "----", "----", "----", VTY_NEWLINE);
	}
	for (node = lstFirst(unit_board_mgt_list); node != NULL; node = lstNext(node))
	{
		t = (unit_board_mgt_t *)node;
		if (node)
		{
			unit_port_mgt_t *mgt = NULL;
			NODE pnode;
			if (t->mutex)
				os_mutex_lock(t->mutex, OS_WAIT_FOREVER);

			for (mgt = (unit_port_mgt_t *)lstFirst(t->port_list); mgt != NULL; mgt = (unit_port_mgt_t *)lstNext(&pnode))
			{
				pnode = mgt->node;
				if (mgt)
				{
					vty_out(vty, "%04x  %-4d %-4d %-4d %-4d%s", mgt->type, t->unit, t->slot, mgt->port, t->state, VTY_NEWLINE);
				}
			}
			if (t->mutex)
				os_mutex_unlock(t->mutex);
		}
	}
#else
	if (lstCount(unit_board_mgt_list) || lstCount(job_unused_list))
	{
		fprintf(stdout, "%-4s %-4s %-4s %-4s %-4s %s", "----", "----", "----", "----", "----", VTY_NEWLINE);
		fprintf(stdout, "%-4s %-4s %-4s %-4s %-4s %s", "TYPE", "UNIT", "SLOT", "PORT", "STAT", VTY_NEWLINE);
		fprintf(stdout, "%-4s %-4s %-4s %-4s %-4s %s", "----", "----", "----", "----", "----" VTY_NEWLINE);
	}
	for (node = lstFirst(unit_board_mgt_list); node != NULL; node = lstNext(node))
	{
		t = (unit_board_mgt_t *)node;
		if (node)
		{
			unit_port_mgt_t *mgt = NULL;
			NODE pnode;
			if (t->mutex)
				os_mutex_lock(t->mutex, OS_WAIT_FOREVER);

			for (mgt = (unit_port_mgt_t *)lstFirst(t->port_list); mgt != NULL; mgt = (unit_port_mgt_t *)lstNext(&pnode))
			{
				pnode = mgt->node;
				if (mgt)
				{
					fprintf(stdout, "%04x  %-4d %-4d %-4d %-4d%s", mgt->type, t->unit, t->slot, mgt->port, t->state, "\r\n");
				}
			}
			if (t->mutex)
				os_mutex_unlock(t->mutex);
		}
	}
#endif
	if (unit_board_mgt_mutex)
		os_mutex_unlock(unit_board_mgt_mutex);
	return OK;
}

int unit_board_waitting()
{
	os_sleep(10);
	return OK;
}
/* if manage */
/*
static int if_slot_port_to_phy(int u, int s, int p)
{
	NODE node;
	unit_board_mgt_t *t;
	if (unit_board_mgt_mutex)
		os_mutex_lock(unit_board_mgt_mutex, OS_WAIT_FOREVER);

	for(t = (unit_board_mgt_t *)lstFirst(unit_board_mgt_list); t != NULL; t = (unit_board_mgt_t *)lstNext(&node))
	{
		node = t->node;
		if(t && t->unit == u && t->slot == s && t->port == p)
		{
			if(unit_board_mgt_mutex)
				os_mutex_unlock(unit_board_mgt_mutex);
			return t->phyid;
		}
	}
	if(unit_board_mgt_mutex)
		os_mutex_unlock(unit_board_mgt_mutex);
	return 0;
}


const int if_ifindex2phy(ifindex_t ifindex)
{
	if( IF_TYPE_GET(ifindex) == IF_SERIAL ||
			IF_TYPE_GET(ifindex) == IF_ETHERNET ||
			IF_TYPE_GET(ifindex) == IF_GIGABT_ETHERNET)
	{
		return if_slot_port_to_phy(IF_UNIT_GET(ifindex), IF_SLOT_GET(ifindex), IF_PORT_GET(ifindex));
	}
	else if(IF_TYPE_GET(ifindex) == IF_VLAN)
	{
		return IF_TYPE_CLR(ifindex);
	}
	else if(IF_TYPE_GET(ifindex) == IF_LAG)
	{
		return IF_TYPE_CLR(ifindex);
	}
	else if(IF_TYPE_GET(ifindex) == IF_LOOPBACK)
	{
		return IF_TYPE_CLR(ifindex);
	}
	else if(IF_TYPE_GET(ifindex) == IF_TUNNEL)
	{
		return IF_TYPE_CLR(ifindex);
	}
	return -1;
}


static int if_unit_slot_port(zpl_uint32 type, int u, int s, int p)
{
	struct interface * ifp = NULL;
	zpl_uint32 i = 0;
	char name[64];//[ethernet|gigabitethernet|tunnel|loopback]
	if(p == 0)
		return 0;
	for(i = 1; i <= p; i++)
	{
		memset(name, '\0', sizeof(name));

		if(type == IF_SERIAL)
			sprintf(name,"serial %d/%d/%d",u,s,i);
		else if(type == IF_ETHERNET)
			sprintf(name,"ethernet %d/%d/%d",u,s,i);
		else if(type == IF_GIGABT_ETHERNET)
			sprintf(name,"gigabitethernet %d/%d/%d",u,s,i);
		else if(type == IF_WIRELESS)
			sprintf(name,"wireless %d/%d/%d",u,s,i);
		else if(type == IF_TUNNEL)
			sprintf(name,"tunnel %d/%d/%d",u,s,i);
		else if(type == IF_BRIGDE)
			sprintf(name,"brigde %d/%d/%d",u,s,i);
#ifdef CUSTOM_INTERFACE
		else if(type == IF_WIFI)
			sprintf(name,"wifi %d/%d/%d",u,s,i);
		else if(type == IF_MODEM)
			sprintf(name,"modem %d/%d/%d",u,s,i);
#endif
		else if(type == IF_LOOPBACK)
			sprintf(name,"loopback%d",i);
		else if(type == IF_VLAN)
			sprintf(name,"vlan%d",i);
		else if(type == IF_LAG)
			sprintf(name,"port-channel%d",i);

		if(os_strlen(name))
			ifp = if_create (name, strlen(name));
	}
	return OK;
}
*/
