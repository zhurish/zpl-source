/*
 * nsm_serial.c
 *
 *  Created on: Aug 26, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"
#include "vrf.h"
#include "prefix.h"
#include "vty.h"
#include "zmemory.h"
#include "template.h"
#include "algorithm.h"
#include "nsm_include.h"
#include "hal_include.h"
#include "pal_include.h"

zpl_uint32 serial_index_make(const char *sname)
{
	zpl_uint32 i = 0;
	zpl_char *p = sname;
	zpl_uint32	inde = 0;
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
	return (nsm_serial_t *)nsm_intf_module_data(ifp, NSM_INTF_SERIAL);
}


int nsm_serial_interface_create_api(struct interface *ifp)
{
	nsm_serial_t * serial = (nsm_serial_t *)nsm_intf_module_data(ifp, NSM_INTF_SERIAL);
	if(!if_is_serial(ifp))
		return OK;
	if(serial == NULL)
	{
	serial = XMALLOC(MTYPE_SERIAL, sizeof(nsm_serial_t));
	zassert(serial);
	os_memset(serial, 0, sizeof(nsm_serial_t));


	serial->serial.databit = NSM_SERIAL_DATA_DEFAULT;
	serial->serial.stopbit = NSM_SERIAL_STOP_DEFAULT;
	serial->serial.flow_control = NSM_SERIAL_FLOW_CTL_DEFAULT;
	serial->serial.parity = NSM_SERIAL_PARITY_DEFAULT;
	serial->serial.speed = NSM_SERIAL_CLOCK_DEFAULT;

	serial->ifp = ifp;
	//serial->serial_index = serial_index_make();
	nsm_intf_module_data_set(ifp, NSM_INTF_SERIAL, serial);
	}
	return OK;
}


int nsm_serial_interface_del_api(struct interface *ifp)
{
	nsm_serial_t * serial = (nsm_serial_t *)nsm_intf_module_data(ifp, NSM_INTF_SERIAL);
	if(!if_is_serial(ifp))
		return OK;
	if(serial)
		XFREE(MTYPE_SERIAL, serial);
	nsm_intf_module_data_set(ifp, NSM_INTF_SERIAL, NULL);
	return OK;
}

int nsm_serial_interface_kernel(struct interface *ifp, zpl_char *kname)
{
	//nsm_serial_t * serial = nsm_serial_get(ifp);
	if(ifp)
	{
		if_kname_set(ifp, kname);
		if(!nsm_halpal_interface_ifindex(ifp->k_name))
			nsm_halpal_interface_add(ifp);
		ifp->k_ifindex = nsm_halpal_interface_ifindex(ifp->k_name);
		//pal_interface_get_lladdr(ifp);
		SET_FLAG(ifp->status, IF_INTERFACE_ATTACH);
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_clock(struct interface *ifp, zpl_uint32 clock)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		serial->serial.speed = clock;
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_databit(struct interface *ifp, zpl_uint32 databit)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		serial->serial.databit = databit;
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_stopbit(struct interface *ifp, zpl_uint32 stopbit)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		serial->serial.stopbit = stopbit;
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_parity(struct interface *ifp, zpl_uint32 parity)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		serial->serial.parity = parity;
		return OK;
	}
	return ERROR;
}

int nsm_serial_interface_flow_control(struct interface *ifp, zpl_uint32 flow_control)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		serial->serial.flow_control = flow_control;
		return OK;
	}
	return ERROR;
}


int nsm_serial_interface_devname(struct interface *ifp, zpl_char * devname)
{
	nsm_serial_t * serial = nsm_serial_get(ifp);
	if(serial)
	{
		if(devname)
		{
			zpl_char *p;//, *k;
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
#ifdef IF_ENCAPSULATION_ENABLE
int nsm_serial_interface_enca_set_api(struct interface *ifp, if_enca_t mode)
{
	if(mode == IF_ENCA_SLIP || mode == IF_ENCA_HDLC)	//SLIP
	{

	}
	return OK;
}
#endif




int nsm_serial_ppp_encapsulation(zpl_char *input, zpl_uint32 inlen, zpl_char *output, zpl_uint32 outlen)
{
	return OK;
}

int nsm_serial_ppp_decapsulation(zpl_char *input, zpl_uint32 inlen, zpl_char *output, zpl_uint32 outlen)
{
	return OK;
}

int nsm_serial_slip_encapsulation(zpl_char *input, zpl_uint32 inlen, zpl_char *output, zpl_uint32 outlen)
{
	return OK;
}

int nsm_serial_slip_decapsulation(zpl_char *input, zpl_uint32 inlen, zpl_char *output, zpl_uint32 outlen)
{
	return OK;
}



#ifdef ZPL_SHELL_MODULE
int nsm_serial_interface_write_config(struct vty *vty, struct interface *ifp)
{
	if(if_is_serial(ifp))
	{
		zpl_char *parity[] = { "none", "even", "odd", "mark", "space" };
		zpl_char *flow[] = { "none", "sorfware", "hardware"};
		//zpl_char *parity[] = { "none", "even", "odd", "mark", "space" };

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
#endif


int nsm_serial_init(void)
{
	nsm_interface_hook_add(NSM_INTF_SERIAL, nsm_serial_interface_create_api, nsm_serial_interface_del_api);
	return OK;
}

int nsm_serial_exit(void)
{
	return OK;
}
