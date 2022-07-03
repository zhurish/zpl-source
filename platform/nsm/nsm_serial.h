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


#define NSM_SERIAL_FLOW_CTL_DEFAULT		TTY_FLOW_CTL_NONE
#define NSM_SERIAL_PARITY_DEFAULT		TTY_PARITY_NONE
#define NSM_SERIAL_DATA_DEFAULT			TTY_DATA_8BIT
#define NSM_SERIAL_STOP_DEFAULT			TTY_STOP_1BIT

#define NSM_SERIAL_CLOCK_DEFAULT		115200


typedef struct nsm_serial_s
{
	struct interface *ifp;
	tty_type_en		serial_type;
	struct tty_com	serial;

	zpl_uint32	 serial_index;

	int	(*encapsulation)(zpl_char *, zpl_uint32, zpl_char *, zpl_uint32);
	int	(*decapsulation)(zpl_char *, zpl_uint32, zpl_char *, zpl_uint32);

}nsm_serial_t;


extern int nsm_serial_init(void);
extern int nsm_serial_exit(void);

extern zpl_uint32 serial_index_make(const char *sname);
/* 创建删除一个serial 接口 */
extern int nsm_serial_interface_create_api(struct interface *ifp);
extern int nsm_serial_interface_del_api(struct interface *ifp);

extern int nsm_serial_interface_clock(struct interface *ifp, zpl_uint32 clock);
extern int nsm_serial_interface_databit(struct interface *ifp, zpl_uint32 data);
extern int nsm_serial_interface_stopbit(struct interface *ifp, zpl_uint32 stopbit);
extern int nsm_serial_interface_parity(struct interface *ifp, zpl_uint32 parity);
extern int nsm_serial_interface_flow_control(struct interface *ifp, zpl_uint32 flow_control);
/* 设置serial接口的驱动 */
extern int nsm_serial_interface_devname(struct interface *ifp, zpl_char * devname);
extern int nsm_serial_interface_kernel(struct interface *ifp, zpl_char *kname);
#ifdef IF_ENCAPSULATION_ENABLE
extern int nsm_serial_interface_enca_set_api(struct interface *ifp, if_enca_t mode);
//extern int nsm_serial_interface_enca_get_api(struct interface *ifp, if_enca_t *mode);
#endif

extern int nsm_serial_ppp_encapsulation(zpl_char *input, zpl_uint32 inlen, zpl_char *output, zpl_uint32 outlen);
extern int nsm_serial_ppp_decapsulation(zpl_char *input, zpl_uint32 inlen, zpl_char *output, zpl_uint32 outlen);

extern int nsm_serial_slip_encapsulation(zpl_char *input, zpl_uint32 inlen, zpl_char *output, zpl_uint32 outlen);
extern int nsm_serial_slip_decapsulation(zpl_char *input, zpl_uint32 inlen, zpl_char *output, zpl_uint32 outlen);

#ifdef ZPL_SHELL_MODULE
extern int nsm_serial_interface_write_config(struct vty *vty, struct interface *ifp);
extern void cmd_serial_init(void);
#endif


 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_SERIAL_H__ */
