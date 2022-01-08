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

static int if_unit_slot_port(zpl_bool online, zpl_uint32 type, int u, int s, int p)
{
	struct interface *ifp = NULL;
	zpl_uint32 i = p;
	char name[64]; //[ethernet|gigabitethernet|tunnel|loopback]

	memset(name, '\0', sizeof(name));

	if (type == IF_SERIAL)
		sprintf(name, "serial %d/%d/%d", u, s, i);
	else if (type == IF_ETHERNET)
		sprintf(name, "ethernet %d/%d/%d", u, s, i);
	else if (type == IF_GIGABT_ETHERNET)
		sprintf(name, "gigabitethernet %d/%d/%d", u, s, i);
	else if (type == IF_WIRELESS)
		sprintf(name, "wireless %d/%d/%d", u, s, i);
	else if (type == IF_TUNNEL)
		sprintf(name, "tunnel %d/%d/%d", u, s, i);
	else if (type == IF_BRIGDE)
		sprintf(name, "brigde %d/%d/%d", u, s, i);
#ifdef CUSTOM_INTERFACE
	else if (type == IF_WIFI)
		sprintf(name, "wifi %d/%d/%d", u, s, i);
	else if (type == IF_MODEM)
		sprintf(name, "modem %d/%d/%d", u, s, i);
#endif
	else if (type == IF_LOOPBACK)
		sprintf(name, "loopback%d", i);
	else if (type == IF_VLAN)
		sprintf(name, "vlan%d", i);
	else if (type == IF_LAG)
		sprintf(name, "port-channel%d", i);

	if(!online)
	{
		ifp = if_lookup_by_name(name);
		if(ifp)
		{
			if_online(ifp, online);
		}
	}
	else// if(online)
	{
		if (os_strlen(name) && !if_lookup_by_name(name))
			if_create(name, strlen(name));
	}
	return OK;
}

static int unit_board_port_installfunc(unit_port_mgt_t *mgt, void *p)
{
	unit_board_mgt_t *board = p;
	if (board->state != UBMG_STAT_ACTIVE && board->online)
	{
		if (if_unit_slot_port(board->online, mgt->type, board->unit, board->slot, mgt->port) == OK)
			board->state = UBMG_STAT_ACTIVE;
	}
	else if (board->state == UBMG_STAT_ACTIVE && !board->online)
	{
		if (if_unit_slot_port(board->online, mgt->type, board->unit, board->slot, mgt->port) == OK)
			board->state = UBMG_STAT_UNACTIVE;
	}
	return OK;
}

static int unit_board_installfunc(unit_board_mgt_t *mgt, void *p)
{
	zpl_bool *online = (zpl_bool *)p;
	if (mgt)
	{
		mgt->online = online?*online:zpl_true;
		if (mgt->state != UBMG_STAT_ACTIVE && !mgt->b_install)
		{
			unit_board_port_foreach(mgt, unit_board_port_installfunc, mgt);
			mgt->b_install = zpl_true;
		}
		else if (mgt->state == UBMG_STAT_ACTIVE && mgt->b_install)
		{
			unit_board_port_foreach(mgt, unit_board_port_installfunc, mgt);
		}
	}
	return 0;
}

int unit_board_dynamic_install(zpl_uint8 unit, zpl_uint8 slot, zpl_bool enable)
{
	unit_board_mgt_t *mgt = unit_board_lookup(unit, slot);
	zpl_bool online = enable;
	if(mgt)
	{
		unit_board_installfunc(mgt, &online);
	}
	return OK;
}

int bsp_usp_module_init()
{
	zpl_bool online = zpl_true;
#ifdef ZPL_KERNEL_STACK_MODULE
	if_ktest_init();
#endif
	unit_board_foreach(unit_board_installfunc, &online);

	return OK;
}
