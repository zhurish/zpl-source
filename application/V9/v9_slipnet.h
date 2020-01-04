/*
 * v9_slipnet.h
 *
 *  Created on: 2019年11月13日
 *      Author: DELL
 */

#ifndef __V9_SLIPNET_H__
#define __V9_SLIPNET_H__

#define V9_SLIPNET_CTL_NAME	"/dev/ttyS2"
#define V9_SLIPNET_SPEED_RATE 115200

extern int v9_app_slipnet_init(v9_serial_t *serial, char *devname, u_int32 speed);
extern int v9_app_slipnet_exit(v9_serial_t *serial);


#endif /* __V9_SLIPNET_H__ */
