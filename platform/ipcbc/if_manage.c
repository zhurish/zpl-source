/*
 * if_manage.c
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */
#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "if_manage.h"
#include "bmgt.h"


static int if_unit_slot_port(zpl_uint32 type, int u, int s, int p)
{
	struct interface * ifp = NULL;
	zpl_uint32 i = p;
	char name[64];//[ethernet|gigabitethernet|tunnel|loopback]
	//if(p == 0)
	//	return 0;
	//for(i = 1; i <= 1; i++)
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
		
		if(os_strlen(name) && !if_lookup_by_name(name))
			ifp = if_create (name, strlen(name));

/*		if(strstr(name, "ethernet 0/0/1"))
		{
			if_kname_set(ifp, "enp0s25");
		}*/
	}
	return OK;
}

static int unit_board_port_installfunc(unit_port_mgt_t *mgt, void *p)
{
	unit_board_mgt_t *board = p;
	if(board->state != UBMG_STAT_ACTIVE)
	{
		//unit_board_port_foreach(mgt, unit_board_port_installfunc, mgt);
		if(if_unit_slot_port(mgt->type, board->unit, board->slot, mgt->port) == OK)
			board->state = UBMG_STAT_ACTIVE;
	}
	return OK;
}
static int unit_board_installfunc(unit_board_mgt_t *mgt, void *p)
{
	if(mgt)
	{
		if(mgt->state != UBMG_STAT_ACTIVE)
		{
			unit_board_port_foreach(mgt, unit_board_port_installfunc, mgt);
			//if(if_unit_slot_port(mgt->type, mgt->unit, mgt->slot, mgt->port) == OK)
			//	mgt->state = UBMG_STAT_ACTIVE;
		}
	}
	return 0;
}

int bsp_usp_module_init()
{
	unit_board_waitting();
#ifdef ZPL_KERNEL_STACK_MODULE
	if_ktest_init();
#endif
	unit_board_foreach(unit_board_installfunc, NULL);
	return OK;
}


