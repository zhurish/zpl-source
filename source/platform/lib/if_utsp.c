/*
 * if_utsp.c
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zpl_type.h"
#include "os_list.h"
#include "os_sem.h"
#include "module.h"
#include "zmemory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "if.h"
#include "if_utsp.h"
#ifdef ZPL_SHELL_MODULE
#include "vty_include.h"
#endif



struct utsp_manage
{
	LIST *unit_board_mgt_list;
	os_mutex_t *mgt_mutex;
	zpl_uint32 bmgt_debug;
};

struct utsp_manage _utsp_manage;


int unit_board_init(void)
{
	memset(&_utsp_manage, 0, sizeof(struct utsp_manage));
	if (_utsp_manage.unit_board_mgt_list == NULL)
	{
		_utsp_manage.unit_board_mgt_list = os_malloc(sizeof(LIST));
		if (_utsp_manage.unit_board_mgt_list)
		{
			lstInit(_utsp_manage.unit_board_mgt_list);
		}
	}
	if (_utsp_manage.mgt_mutex == NULL)
	{
		_utsp_manage.mgt_mutex = os_mutex_name_create("mgtmutex");
	}
	_utsp_manage.bmgt_debug = BMGT_DEBUG_EVENT|BMGT_DEBUG_DETAIL;
	return OK;
}

int unit_board_exit(void)
{
	if (_utsp_manage.mgt_mutex)
	{
		if (os_mutex_destroy(_utsp_manage.mgt_mutex) == OK)
			_utsp_manage.mgt_mutex = NULL;
	}
	if (_utsp_manage.unit_board_mgt_list)
	{
		lstFree(_utsp_manage.unit_board_mgt_list);
		_utsp_manage.unit_board_mgt_list = NULL;
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
		if(IS_BMGT_DEBUG_EVENT(_utsp_manage.bmgt_debug))
			zlog_debug(MODULE_NSM, "Unit Board Add unit %d slot %d ", unit, slot);
		// t->lport = lport;
		// t->phyid = phyid;
		t->port_list = os_malloc(sizeof(LIST));
		if (t->port_list)
		{
			lstInitFree(t->port_list, free);
		}
		t->mutex = os_mutex_name_create("utspmutex");
		t->state = UBMG_STAT_INIT;
		if (_utsp_manage.mgt_mutex)
			os_mutex_lock(_utsp_manage.mgt_mutex, OS_WAIT_FOREVER);
		if (_utsp_manage.unit_board_mgt_list)
			lstAdd(_utsp_manage.unit_board_mgt_list, (NODE *)t);
		if (_utsp_manage.mgt_mutex)
			os_mutex_unlock(_utsp_manage.mgt_mutex);
		return t;
	}
	return NULL;
}

int unit_board_del(zpl_uint8 unit, zpl_uint8 slot)
{
	NODE node;
	unit_board_mgt_t *t;
	if (_utsp_manage.mgt_mutex)
		os_mutex_lock(_utsp_manage.mgt_mutex, OS_WAIT_FOREVER);

	for (t = (unit_board_mgt_t *)lstFirst(_utsp_manage.unit_board_mgt_list); t != NULL; t = (unit_board_mgt_t *)lstNext(&node))
	{
		node = t->node;
		if (t && t->unit == unit && t->slot == slot)
		{
			if(IS_BMGT_DEBUG_EVENT(_utsp_manage.bmgt_debug))
				zlog_debug(MODULE_NSM, "Unit Board Del unit %d slot %d ", unit, slot);			
	
			lstDelete(_utsp_manage.unit_board_mgt_list, (NODE *)t);
			if (t->mutex)
				os_mutex_lock(t->mutex, OS_WAIT_FOREVER);
			if (t->port_list)
				lstFree(t->port_list);
			if (t->mutex)
				os_mutex_unlock(t->mutex);
			if (t->port_list)
				free(t->port_list);
			if (t->mutex)
				os_mutex_destroy(t->mutex);
			free(t);
			break;
		}
	}
	if (_utsp_manage.mgt_mutex)
		os_mutex_unlock(_utsp_manage.mgt_mutex);
	return OK;
}

unit_board_mgt_t * unit_board_lookup(zpl_uint8 unit, zpl_uint8 slot)
{
	NODE node;
	unit_board_mgt_t *t = NULL;
	if (_utsp_manage.mgt_mutex)
		os_mutex_lock(_utsp_manage.mgt_mutex, OS_WAIT_FOREVER);

	for (t = (unit_board_mgt_t *)lstFirst(_utsp_manage.unit_board_mgt_list); t != NULL; t = (unit_board_mgt_t *)lstNext(&node))
	{
		node = t->node;
		if (t && t->unit == unit && t->slot == slot)
		{
			break;
		}
	}
	if (_utsp_manage.mgt_mutex)
		os_mutex_unlock(_utsp_manage.mgt_mutex);
	return t;
}

int unit_board_foreach(int (*func)(unit_board_mgt_t *, void *), void *p)
{
	NODE node;
	unit_board_mgt_t *t;
	if (_utsp_manage.mgt_mutex)
		os_mutex_lock(_utsp_manage.mgt_mutex, OS_WAIT_FOREVER);

	for (t = (unit_board_mgt_t *)lstFirst(_utsp_manage.unit_board_mgt_list); t != NULL; t = (unit_board_mgt_t *)lstNext(&node))
	{
		node = t->node;
		if (t && func)
		{
			(func)(t, p);
		}
	}
	if (_utsp_manage.mgt_mutex)
		os_mutex_unlock(_utsp_manage.mgt_mutex);
	return OK;
}

unit_port_mgt_t *unit_board_port_add(unit_board_mgt_t *board, if_type_t type, zpl_uint8 lport, zpl_phyport_t phyid)
{
	unit_port_mgt_t *t = NULL;
	if (t == NULL)
		t = os_malloc(sizeof(unit_port_mgt_t));
	if (t)
	{
		t->type = (zpl_uint32)type;
		t->lport = lport;
		t->phyid = phyid;
		if(IS_BMGT_DEBUG_EVENT(_utsp_manage.bmgt_debug))
			zlog_debug(MODULE_NSM, "Unit Board Add Port unit %d slot %d type %d lport %d phyid %d", board->unit, board->slot, type, lport, phyid);
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

int unit_board_port_del(unit_board_mgt_t *board, if_type_t type, zpl_uint8 lport, zpl_phyport_t phyid)
{
	NODE node;
	unit_port_mgt_t *t;
	if (board->mutex)
		os_mutex_lock(board->mutex, OS_WAIT_FOREVER);
	for (t = (unit_port_mgt_t *)lstFirst(board->port_list); t != NULL; t = (unit_port_mgt_t *)lstNext(&node))
	{
		node = t->node;
		if (t && t->type == type && t->lport == lport && t->phyid == phyid)
		{
			if(IS_BMGT_DEBUG_EVENT(_utsp_manage.bmgt_debug))
				zlog_debug(MODULE_NSM, "Unit Board Del Port unit %d slot %d type %d lport %d phyid %d", board->unit, board->slot, type, lport, phyid);
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

static int unit_board_port_show(unit_board_mgt_t *t, void *pvoid, zpl_char datil)
{
	NODE pnode;
	unit_port_mgt_t *mgt = NULL;
	zpl_char *state_str[] = {"Init", "Pand", "Acti", "UnAc"};
#ifdef ZPL_SHELL_MODULE
	struct vty *vty = (struct vty *)pvoid;
#endif

#ifdef ZPL_SHELL_MODULE
	if (t->mutex)
		os_mutex_lock(t->mutex, OS_WAIT_FOREVER);

	for (mgt = (unit_port_mgt_t *)lstFirst(t->port_list); mgt != NULL; mgt = (unit_port_mgt_t *)lstNext(&pnode))
	{
		pnode = mgt->node;
		if (mgt)
		{
			vty_out(vty, "%-8s %-4d %-4d %-4d %-4s %-4d %s%s%s", getabstractname(mgt->type), t->unit, t->slot, 
				mgt->lport, state_str[t->state], mgt->phyid, t->online ? "U":"D", t->b_install?"I":"N", VTY_NEWLINE);
		}
	}
	if (t->mutex)
		os_mutex_unlock(t->mutex);
#else
	for (mgt = (unit_port_mgt_t *)lstFirst(t->port_list); mgt != NULL; mgt = (unit_port_mgt_t *)lstNext(&pnode))
	{
		pnode = mgt->node;
		if (mgt)
		{
			fprintf(stdout, "%-8s %-4d %-4d %-4d %-4s %-4d %s%s%s", getabstractname(mgt->type), t->unit, t->slot, 
				mgt->lport, state_str[t->state], mgt->phyid, t->online ? "U":"D", t->b_install?"I":"N", "\r\n");
		}
	}
	if (t->mutex)
		os_mutex_unlock(t->mutex);
#endif
	return OK;
}

int unit_board_show(void *pvoid)
{
	zpl_char datil = 0;
	NODE *node = NULL;
	unit_board_mgt_t *t = NULL;

#ifdef ZPL_SHELL_MODULE
	struct vty *vty = (struct vty *)pvoid;
#endif
	if (_utsp_manage.mgt_mutex)
		os_mutex_lock(_utsp_manage.mgt_mutex, OS_WAIT_FOREVER);
#ifdef ZPL_SHELL_MODULE
	if (lstCount(_utsp_manage.unit_board_mgt_list))
	{
		vty_out(vty, "%-8s %-4s %-4s %-4s %-4s %-4s %-4s%s", "--------", "----", "----", "----", "----", "----", "----",VTY_NEWLINE);
		vty_out(vty, "%-8s %-4s %-4s %-4s %-4s %-4s %-4s%s", "TYPE", "UNIT", "SLOT", "PORT", "STAT", "PHY", "STUS", VTY_NEWLINE);
		vty_out(vty, "%-8s %-4s %-4s %-4s %-4s %-4s %-4s%s", "--------", "----", "----", "----", "----", "----", "----",VTY_NEWLINE);
		datil = 1;
	}
	for (node = lstFirst(_utsp_manage.unit_board_mgt_list); node != NULL; node = lstNext(node))
	{
		t = (unit_board_mgt_t *)node;
		if (node)
		{
			
			if(t->state < UBMG_STAT_INIT || t->state > UBMG_STAT_UNACTIVE)
			{
				vty_out(vty, "=====================state=%d%s", t->state, "\r\n");
				t->state = UBMG_STAT_ACTIVE;
			}
			unit_board_port_show(t, pvoid, datil);
		}
	}
	if(datil)
	{
		vty_out(vty, " STUS: online:U offline:D; install:I Uninstall:N%s", VTY_NEWLINE);
	}
#else
	if (lstCount(_utsp_manage.unit_board_mgt_list) || lstCount(job_unused_list))
	{
		fprintf(stdout, "%-8s %-4s %-4s %-4s %-4s %-4s %-4s%s", "--------", "----", "----", "----", "----", "----", "----", VTY_NEWLINE);
		fprintf(stdout, "%-8s %-4s %-4s %-4s %-4s %-4s %-4s%s", "TYPE", "UNIT", "SLOT", "PORT", "STAT", "PHY", "STUS", VTY_NEWLINE);
		fprintf(stdout, "%-8s %-4s %-4s %-4s %-4s %-4s %-4s%s", "--------", "----", "----", "----", "----" "----", "----", VTY_NEWLINE);
		datil = 1;
	}
	for (node = lstFirst(_utsp_manage.unit_board_mgt_list); node != NULL; node = lstNext(node))
	{
		t = (unit_board_mgt_t *)node;
		if (node)
		{
			if(t->state < UBMG_STAT_INIT || t->state > UBMG_STAT_UNACTIVE)
			{
				fprintf(stdout, "=====================state=%d%s", t->state, "\r\n");
				t->state = UBMG_STAT_ACTIVE;
			}
			unit_board_port_show(t, pvoid, datil);
		}
	}	
	if(datil)
	{
		fprintf(stdout, " STUS: online:U offline:D; install:I Uninstall:N%s", "\r\n");
	}
#endif
	if (_utsp_manage.mgt_mutex)
		os_mutex_unlock(_utsp_manage.mgt_mutex);
	return OK;
}

int unit_board_waitting(void)
{
	os_sleep(10);
	return OK;
}


static int unit_board_slot_port(zpl_bool online, if_type_t type, zpl_uint8 u, zpl_uint8 s, zpl_uint8 p, zpl_phyport_t phyid)
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
	else if (type == IF_XGIGABT_ETHERNET)
		sprintf(name, "xgigabitethernet %d/%d/%d", u, s, i);
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
		if(IS_BMGT_DEBUG_EVENT(_utsp_manage.bmgt_debug))
			zlog_debug(MODULE_NSM, " if_online %s", name);
		ifp = if_lookup_by_name(name);
		if(ifp)
		{
			if_online(ifp, online);
		}
	}
	else// if(online)
	{
		if(IS_BMGT_DEBUG_EVENT(_utsp_manage.bmgt_debug))
			zlog_debug(MODULE_NSM, " if_create %s", name);
		if (os_strlen(name) && !if_lookup_by_name(name))
			ifp = if_create(name, strlen(name));
		if(ifp)
			if_update_phyid2(ifp, phyid);
	}
	return OK;
}

static int unit_board_port_installfunc(unit_port_mgt_t *mgt, void *p)
{
	unit_board_mgt_t *board = p;
	zlog_debug(MODULE_NSM, " unit_board_port_installfunc online=%d %d %d/%d/%d", 
		board->online, mgt->type, board->unit, board->slot, mgt->lport);
	if (board->state != UBMG_STAT_ACTIVE && board->online)
	{
		if (unit_board_slot_port(board->online, mgt->type, board->unit, board->slot, mgt->lport, mgt->phyid) == OK)
			;//board->state = UBMG_STAT_ACTIVE;
	}
	else if (board->state == UBMG_STAT_ACTIVE && !board->online)
	{
		if (unit_board_slot_port(board->online, mgt->type, board->unit, board->slot, mgt->lport, mgt->phyid) == OK)
			;//board->state = UBMG_STAT_UNACTIVE;
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
			if (mgt->mutex)
				os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
			mgt->b_install = zpl_true;
			mgt->state = UBMG_STAT_ACTIVE;
			zlog_debug(MODULE_NSM, "===================install unit_board_installfunc online=%d state=%d install=%d", 
				mgt->online, mgt->state, mgt->b_install);
			if (mgt->mutex)
				os_mutex_unlock(mgt->mutex);
		}
		else if (mgt->state == UBMG_STAT_ACTIVE && mgt->b_install)
		{
			unit_board_port_foreach(mgt, unit_board_port_installfunc, mgt);
			if (mgt->mutex)
				os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
			mgt->state = UBMG_STAT_UNACTIVE;
			zlog_debug(MODULE_NSM, "===================uninstall unit_board_installfunc online=%d state=%d install=%d", 
				mgt->online, mgt->state, mgt->b_install);
			if (mgt->mutex)
				os_mutex_unlock(mgt->mutex);
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
		zlog_debug(MODULE_NSM, " unit_board_dynamic_install %d/%d %d", unit, slot, enable);
		unit_board_installfunc(mgt, &online);
	}
	return OK;
}

int unit_board_startup(void)
{
	zpl_bool online = zpl_true;
#ifdef ZPL_KERNEL_MODULE
	if_ktest_init();
#endif
	unit_board_foreach(unit_board_installfunc, &online);

	return OK;
}


#ifdef ZPL_KERNEL_MODULE

static int if_slot_kernel_read(void)
{
	struct interface * ifp = NULL;
	char buf[512];
	ifindex_t ifindex;
	unit_board_mgt_t *board = NULL;
	char kname[64], name[128];
	FILE *fp = fopen(SLOT_PORT_CONF, "r");
	if (fp)
	{
		char *s = NULL;
		int n = 0;//, *p;
		//printf("========================%s========================open %s\r\n", __func__, SLOT_PORT_CONF);
		os_memset(buf, 0, sizeof(buf));
		board = unit_board_add(0, 0);
		while (fgets(buf, sizeof(buf), fp))
		{
			os_memset(kname, 0, sizeof(kname));
			os_memset(name, 0, sizeof(name));
			s = strstr(buf, ":");
			if(s)
			{
				os_memcpy(name, buf, s - buf);
				s++;
				n = strspn(s, "qwertyuiopasdfghjklzxcvbnm.1234567890");
				if(strstr(s, "-"))
					os_strcpy(kname, "br-lan");
				else
					os_strncpy(kname, s, n);
				//kname[strlen(kname)-1] = '\0';

				ifp = if_create (name, strlen(name));
				//os_msleep(1);
				ifindex = if_ifindex_make(name, NULL);
				//printf("========================%s========================%s(%d)-->%s\r\n", __func__, name, ifindex, kname);
				if(ifp && ifindex != 0)
				{
					if_kname_set(ifp, kname);

					if(strstr(name,"eth"))
						unit_board_port_add(board,IF_ETHERNET, 1, if_nametoindex(kname));
					else if(strstr(name,"wireless"))
						unit_board_port_add(board,IF_WIRELESS, 1, if_nametoindex(kname));						
				}
			}
		}
		fclose(fp);
	}
	else
	{
		//printf("========================%s========================can not open %s\r\n", __func__, SLOT_PORT_CONF);
	}
	return 0;
}


#endif

#ifdef ZPL_KERNEL_MODULE
int if_ktest_init(void)
{
	if_slot_kernel_read();
	return OK;
}
#endif




