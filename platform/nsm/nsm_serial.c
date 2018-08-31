/*
 * nsm_serial.c
 *
 *  Created on: Aug 26, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "interface.h"
#include <log.h>
#include "nsm_client.h"
#include "os_list.h"
#include "tty_com.h"

#include "nsm_serial.h"


int serial_index_make(const char *sname)
{
	int i;
	char *p = sname;
	int	inde = 0;
	tty_type_en type = 0;
	if(os_strstr(sname, "USB"))
		type = IF_TTY_USB;
	else if(os_strstr(sname, "CAM"))
		type = IF_TTY_CAM;
	else if(os_strstr(sname, "S"))
		type = IF_TTY_S;

	for(i = 0; i < os_strlen(sname); i++)
	{
		if(isdigit(p[i]))
		{
			inde = atoi(&p[i]);
			break;
		}
	}
	return IF_SERIAL_INDEX_SET(type, inde);
}


static nsm_serial_t * nsm_serial_get(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	return nsm->nsm_client[NSM_SERIAL];
}


int nsm_serial_add_interface(struct interface *ifp)
{
	nsm_serial_t * serial = NULL;
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	if(!nsm->nsm_client[NSM_SERIAL])
		nsm->nsm_client[NSM_SERIAL] = XMALLOC(MTYPE_IF, sizeof(nsm_serial_t));
	zassert(nsm->nsm_client[NSM_SERIAL]);
	os_memset(nsm->nsm_client[NSM_SERIAL], 0, sizeof(nsm_serial_t));
	serial = nsm->nsm_client[NSM_SERIAL];

	serial->ifp = ifp;
	//serial->serial_index = serial_index_make();
	return OK;
}


int nsm_serial_del_interface(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[ZLOG_NSM];
	if(nsm->nsm_client[NSM_SERIAL])
		XFREE(MTYPE_IF, nsm->nsm_client[NSM_SERIAL]);
	return OK;
}

int nsm_serial_interface_kernel(struct interface *ifp, char *kname)
{
	//nsm_serial_t * serial = nsm_serial_get(ifp);
	if(ifp)
	{
		IF_DATA_LOCK();
		os_memset(ifp->k_name, 0, sizeof(ifp->k_name));
		os_strcpy(ifp->k_name, kname);
		ifp->k_name_hash = if_name_hash_make(ifp->k_name);
		if_manage_kernel_update (ifp);
		nsm_client_interface_add(ifp);
		SET_FLAG(ifp->status, ZEBRA_INTERFACE_ATTACH);
		IF_DATA_UNLOCK();
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_clock(struct interface *ifp, int clock)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		IF_DATA_LOCK();
		serial->serial.speed = clock;
		IF_DATA_UNLOCK();
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_data(struct interface *ifp, int data)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		IF_DATA_LOCK();
		serial->serial.databit = data;
		IF_DATA_UNLOCK();
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_stop(struct interface *ifp, int stop)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		IF_DATA_LOCK();
		serial->serial.stopbit = stop;
		IF_DATA_UNLOCK();
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_parity(struct interface *ifp, int parity)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		IF_DATA_LOCK();
		serial->serial.parity = parity;
		IF_DATA_UNLOCK();
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_flow_control(struct interface *ifp, int flow_control)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		IF_DATA_LOCK();
		serial->serial.flow_control = flow_control;
		IF_DATA_UNLOCK();
		return OK;
	}
	return ERROR;
}


int nsm_serial_interface_devname(struct interface *ifp, char * devname)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		IF_DATA_LOCK();
		if(devname)
		{
			char *p, *k;
			os_memset(serial->serial.devname, 0, sizeof(serial->serial.devname));
			p = devname;
			while(p)
			{
				p = os_strstr(p, "/");
				if(p)
					p++;
			}
			os_strcpy(serial->serial.devname, p);
			serial->serial_index = serial_index_make(serial->serial.devname);
		}
		else
		{
			os_memset(serial->serial.devname, 0, sizeof(serial->serial.devname));
			serial->serial_index = 0;//serial_index_make();
		}
		IF_DATA_UNLOCK();
		return OK;
	}
	return ERROR;
}



int nsm_serial_interface_write_config(struct vty *vty, struct interface *ifp)
{
	if(if_is_serial(ifp))
	{
		nsm_serial_t * serial = nsm_serial_get(ifp);
		if(serial)
		{
			vty_out(vty, " clock %d%s", serial->serial.speed, VTY_NEWLINE);
			vty_out(vty, " stop %d%s", serial->serial.stopbit, VTY_NEWLINE);
			vty_out(vty, " data %d%s", serial->serial.databit, VTY_NEWLINE);
			//vty_out(vty, " clock %d%s", serial->serial.speed, VTY_NEWLINE);
			//vty_out(vty, " clock %d%s", serial->serial.speed, VTY_NEWLINE);
		}
	}
	return OK;
}



int nsm_serial_client_init()
{
	struct nsm_client *nsm = nsm_client_new ();
	nsm->interface_add_cb = NULL;
	nsm->interface_delete_cb = NULL;
	nsm->interface_write_config_cb = nsm_serial_interface_write_config;
	nsm_client_install (nsm, NSM_SERIAL);
	return OK;
}
