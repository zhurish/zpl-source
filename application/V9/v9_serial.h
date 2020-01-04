/*
 * v9_serial.h
 *
 *  Created on: 2019年11月13日
 *      Author: DELL
 */

#ifndef __V9_SERIAL_H__
#define __V9_SERIAL_H__

#define V9_SERIAL_CTL_NAME	"/dev/ttyS1"


#define V9_APP_DEBUG_EVENT		0X01
#define V9_APP_DEBUG_HEX		0X02
#define V9_APP_DEBUG_RECV		0X04
#define V9_APP_DEBUG_SEND		0X08
#define V9_APP_DEBUG_UPDATE		0X10
#define V9_APP_DEBUG_TIME		0X20
#define V9_APP_DEBUG_WEB		0X40
#define V9_APP_DEBUG_MSG		0X80
#define V9_APP_DEBUG_STATE		0X100
#define V9_APP_DEBUG_UCI		0X200

#define V9_APP_DEBUG(n)		(V9_APP_DEBUG_ ## n & v9_serial->debug)
#define V9_APP_DEBUG_ON(n)	(v9_serial->debug |= (V9_APP_DEBUG_ ## n ))
#define V9_APP_DEBUG_OFF(n)	(v9_serial->debug &= ~(V9_APP_DEBUG_ ## n ))





typedef struct v9_serial_s
{
	BOOL			enable;
	void			*master;
	void			*mutex;
	u_int32			task_id;
	void			*r_thread;

	struct tty_com *tty;

	struct tty_com *slipnet;
	void			*r_slipnet;


	u_int8			status;

	u_int8			id;
	u_int8			seqnum;
	char			buf[512];
	int				len;

	char			sbuf[512];
	int				slen;
	int				debug;
} v9_serial_t;

extern v9_serial_t *v9_serial;

extern int v9_serial_init(char *devname, u_int32 speed);
extern int v9_serial_exit(void);


extern int v9_serial_task_init();
extern int v9_serial_task_exit();

extern void cmd_app_v9_init(void);


int v9_app_hex_debug(v9_serial_t *mgt, char *hdr, int rx);


#endif /* __V9_SERIAL_H__ */
