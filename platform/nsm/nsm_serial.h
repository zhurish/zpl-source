/*
 * nsm_serial.h
 *
 *  Created on: Aug 26, 2018
 *      Author: zhurish
 */

#ifndef __NSM_SERIAL_H__
#define __NSM_SERIAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tty_com.h"


#define IF_SERIAL_INDEX_SET(t, n)		(((t)<<8)|(n))
#define IF_SERIAL_TYPE_GET(t)			(((t)>>8))
#define IF_SERIAL_ID_GET(t)				(((t)>>8)&0XFF)


#define NSM_SERIAL_FLOW_CTL_DEFAULT		FLOW_CTL_NONE
#define NSM_SERIAL_PARITY_DEFAULT		PARITY_NONE
#define NSM_SERIAL_DATA_DEFAULT			DATA_8BIT
#define NSM_SERIAL_STOP_DEFAULT			STOP_1BIT

#define NSM_SERIAL_CLOCK_DEFAULT		115200


typedef struct nsm_serial_s
{
	struct interface *ifp;
	tty_type_en		serial_type;
	struct tty_com	serial;

	ospl_uint32	 serial_index;

	int	(*encapsulation)(ospl_char *, ospl_uint32, ospl_char *, ospl_uint32);
	int	(*decapsulation)(ospl_char *, ospl_uint32, ospl_char *, ospl_uint32);

}nsm_serial_t;


extern int nsm_serial_client_init();
extern int nsm_serial_client_exit();

extern ospl_uint32 serial_index_make(const char *sname);

//extern int nsm_serial_add_interface(struct interface *ifp);
//extern int nsm_serial_del_interface(struct interface *ifp);

extern int nsm_serial_interface_clock(struct interface *ifp, ospl_uint32 clock);
extern int nsm_serial_interface_data(struct interface *ifp, ospl_uint32 data);
extern int nsm_serial_interface_stop(struct interface *ifp, ospl_uint32 stop);
extern int nsm_serial_interface_parity(struct interface *ifp, ospl_uint32 parity);
extern int nsm_serial_interface_flow_control(struct interface *ifp, ospl_uint32 flow_control);

extern int nsm_serial_interface_devname(struct interface *ifp, ospl_char * devname);
extern int nsm_serial_interface_kernel(struct interface *ifp, ospl_char *kname);

extern int nsm_serial_interface_write_config(struct vty *vty, struct interface *ifp);

extern int nsm_serial_interface_enca_set_api(struct interface *ifp, if_enca_t mode);
//extern int nsm_serial_interface_enca_get_api(struct interface *ifp, if_enca_t *mode);


extern int nsm_serial_ppp_encapsulation(ospl_char *input, ospl_uint32 inlen, ospl_char *output, ospl_uint32 outlen);
extern int nsm_serial_ppp_decapsulation(ospl_char *input, ospl_uint32 inlen, ospl_char *output, ospl_uint32 outlen);

extern int nsm_serial_slip_encapsulation(ospl_char *input, ospl_uint32 inlen, ospl_char *output, ospl_uint32 outlen);
extern int nsm_serial_slip_decapsulation(ospl_char *input, ospl_uint32 inlen, ospl_char *output, ospl_uint32 outlen);


extern void cmd_serial_init(void);

 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_SERIAL_H__ */
