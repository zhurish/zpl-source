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
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	return (nsm_serial_t *)nsm->nsm_client[NSM_SERIAL];
}


static int nsm_serial_add_interface(struct interface *ifp)
{
	nsm_serial_t * serial = NULL;
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(!if_is_serial(ifp))
		return OK;
	if(!nsm->nsm_client[NSM_SERIAL])
		nsm->nsm_client[NSM_SERIAL] = XMALLOC(MTYPE_SERIAL, sizeof(nsm_serial_t));
	zassert(nsm->nsm_client[NSM_SERIAL]);
	os_memset(nsm->nsm_client[NSM_SERIAL], 0, sizeof(nsm_serial_t));
	serial = nsm->nsm_client[NSM_SERIAL];

	serial->serial.databit = NSM_SERIAL_DATA_DEFAULT;
	serial->serial.stopbit = NSM_SERIAL_STOP_DEFAULT;
	serial->serial.flow_control = NSM_SERIAL_FLOW_CTL_DEFAULT;
	serial->serial.parity = NSM_SERIAL_PARITY_DEFAULT;
	serial->serial.speed = NSM_SERIAL_CLOCK_DEFAULT;

	serial->ifp = ifp;
	//serial->serial_index = serial_index_make();
	return OK;
}


static int nsm_serial_del_interface(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(!if_is_serial(ifp))
		return OK;
	if(nsm->nsm_client[NSM_SERIAL])
		XFREE(MTYPE_SERIAL, nsm->nsm_client[NSM_SERIAL]);
	nsm->nsm_client[NSM_SERIAL] = NULL;
	return OK;
}

int nsm_serial_interface_kernel(struct interface *ifp, char *kname)
{
	//nsm_serial_t * serial = nsm_serial_get(ifp);
	if(ifp)
	{
		if_kname_set(ifp, kname);
		if(!pal_interface_ifindex(ifp->k_name))
			nsm_pal_interface_add(ifp);
		pal_interface_refresh_flag(ifp);
		ifp->k_ifindex = pal_interface_ifindex(ifp->k_name);
		pal_interface_get_lladdr(ifp);
		SET_FLAG(ifp->status, ZEBRA_INTERFACE_ATTACH);
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_clock(struct interface *ifp, int clock)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		serial->serial.speed = clock;
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_data(struct interface *ifp, int data)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		serial->serial.databit = data;
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_stop(struct interface *ifp, int stop)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		serial->serial.stopbit = stop;
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_parity(struct interface *ifp, int parity)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		serial->serial.parity = parity;
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_flow_control(struct interface *ifp, int flow_control)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		serial->serial.flow_control = flow_control;
		return OK;
	}
	return ERROR;
}


int nsm_serial_interface_devname(struct interface *ifp, char * devname)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		if(devname)
		{
			char *p;//, *k;
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
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_enca_set_api(struct interface *ifp, if_enca_t mode)
{
	if(mode == IF_ENCA_SLIP || mode == IF_ENCA_HDLC)	//SLIP
	{

	}
	return OK;
}





int nsm_serial_ppp_encapsulation(char *input, int inlen, char *output, int outlen)
{
	return OK;
}

int nsm_serial_ppp_decapsulation(char *input, int inlen, char *output, int outlen)
{
	return OK;
}

int nsm_serial_slip_encapsulation(char *input, int inlen, char *output, int outlen)
{
	return OK;
}

int nsm_serial_slip_decapsulation(char *input, int inlen, char *output, int outlen)
{
	return OK;
}




int nsm_serial_interface_write_config(struct vty *vty, struct interface *ifp)
{
	if(if_is_serial(ifp))
	{
		char *parity[] = { "none", "even", "odd", "mark", "space" };
		char *flow[] = { "none", "sorfware", "hardware"};
		//char *parity[] = { "none", "even", "odd", "mark", "space" };

		nsm_serial_t * serial = nsm_serial_get(ifp);
		if(serial)
		{
			if(serial->serial.speed != NSM_SERIAL_CLOCK_DEFAULT)
				vty_out(vty, " clock rate %d%s", serial->serial.speed, VTY_NEWLINE);

			if(serial->serial.stopbit != NSM_SERIAL_STOP_DEFAULT)
				vty_out(vty, " stop bit %d%s", serial->serial.stopbit, VTY_NEWLINE);

			if(serial->serial.databit != NSM_SERIAL_DATA_DEFAULT)
				vty_out(vty, " data bit %d%s", serial->serial.databit, VTY_NEWLINE);

			if(serial->serial.parity != NSM_SERIAL_PARITY_DEFAULT)
				vty_out(vty, " parity mode %s%s", parity[serial->serial.parity], VTY_NEWLINE);

			if(serial->serial.flow_control != NSM_SERIAL_FLOW_CTL_DEFAULT)
				vty_out(vty, " flow-control %s%s", flow[serial->serial.flow_control], VTY_NEWLINE);

			//vty_out(vty, " clock %d%s", serial->serial.speed, VTY_NEWLINE);
			//vty_out(vty, " clock %d%s", serial->serial.speed, VTY_NEWLINE);
		}
	}
	return OK;
}



int nsm_serial_client_init()
{
	struct nsm_client *nsm = nsm_client_new ();
	nsm->notify_add_cb = nsm_serial_add_interface;
	nsm->notify_delete_cb = nsm_serial_del_interface;
	nsm->interface_write_config_cb = nsm_serial_interface_write_config;
	nsm_client_install (nsm, NSM_SERIAL);
	return OK;
}

int nsm_serial_client_exit()
{
	struct nsm_client *nsm = nsm_client_lookup (NSM_SERIAL);
	if(nsm)
		nsm_client_free (nsm);
	return OK;
}
