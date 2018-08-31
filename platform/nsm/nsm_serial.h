/*
 * nsm_serial.h
 *
 *  Created on: Aug 26, 2018
 *      Author: zhurish
 */

#ifndef __NSM_SERIAL_H__
#define __NSM_SERIAL_H__

#include "tty_com.h"


#define IF_SERIAL_INDEX_SET(t, n)		(((t)<<8)|(n))
#define IF_SERIAL_TYPE_GET(t)			(((t)>>8))
#define IF_SERIAL_ID_GET(t)				(((t)>>8)&0XFF)

typedef struct nsm_serial_s
{
	struct interface *ifp;
	tty_type_en		serial_type;
	struct tty_com	serial;

	int				serial_index;

}nsm_serial_t;


int nsm_serial_client_init();


extern int serial_index_make(const char *sname);

extern int nsm_serial_add_interface(struct interface *ifp);
extern int nsm_serial_del_interface(struct interface *ifp);

extern int nsm_serial_interface_clock(struct interface *ifp, int clock);
extern int nsm_serial_interface_data(struct interface *ifp, int data);
extern int nsm_serial_interface_stop(struct interface *ifp, int stop);
extern int nsm_serial_interface_parity(struct interface *ifp, int parity);
extern int nsm_serial_interface_flow_control(struct interface *ifp, int flow_control);

extern int nsm_serial_interface_devname(struct interface *ifp, char * devname);
extern int nsm_serial_interface_kernel(struct interface *ifp, char *kname);

extern int nsm_serial_interface_write_config(struct vty *vty, struct interface *ifp);


#endif /* __NSM_SERIAL_H__ */
