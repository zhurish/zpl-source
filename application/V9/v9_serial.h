/*
 * v9_serial.h
 *
 *  Created on: 2019年11月13日
 *      Author: DELL
 */

#ifndef __V9_SERIAL_H__
#define __V9_SERIAL_H__

#include "v9_video.h"

#define V9_SERIAL_CTL_NAME	"/dev/ttyS2"

#define V9_SLIPNET_ENABLE
#define V9_SLIPNET_UDP


#define V9_SERIAL_BUF_MAX 512

#define V9_APP_DEBUG(n)		(V9_APP_DEBUG_ ## n & v9_serial->debug)
#define V9_APP_DEBUG_ON(n)	(v9_serial->debug |= (V9_APP_DEBUG_ ## n ))
#define V9_APP_DEBUG_OFF(n)	(v9_serial->debug &= ~(V9_APP_DEBUG_ ## n ))


#define V9_APP_SYNC_PIPE


typedef struct v9_serial_s
{
	BOOL			enable;
	void			*master;
	void			*mutex;
	u_int32			task_id;
	void			*r_thread;

	struct tty_com *tty;
#ifdef V9_SLIPNET_ENABLE
	struct tty_com *slipnet;
	void			*r_slipnet;
#else
	u_int8			timer_sync;
#endif /* V9_SLIPNET_ENABLE */
	u_int8			sntp_sync;
	u_int8			status;

	u_int8			id;
	u_int8			seqnum;
	char			buf[V9_SERIAL_BUF_MAX];
	int				len;

	char			sbuf[V9_SERIAL_BUF_MAX];
	int				slen;
	int				debug;

	char			tmpbuf[V9_SERIAL_BUF_MAX];
} v9_serial_t;

extern v9_serial_t *v9_serial;

extern int v9_serial_init(char *devname, u_int32 speed);
extern int v9_serial_exit(void);


extern int v9_serial_task_init();
extern int v9_serial_task_exit();

extern void cmd_app_v9_init(void);


int v9_app_hex_debug(v9_serial_t *mgt, char *hdr, int rx);


#endif /* __V9_SERIAL_H__ */
