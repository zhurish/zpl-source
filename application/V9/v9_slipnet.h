/*
 * v9_slipnet.h
 *
 *  Created on: 2019年11月13日
 *      Author: DELL
 */

#ifndef __V9_SLIPNET_H__
#define __V9_SLIPNET_H__


#ifdef __cplusplus
extern "C" {
#endif


#ifdef V9_SLIPNET_ENABLE

#define APP_SLIPNET_QUEUE_SIZE			1460
#pragma pack(1)

#define APP_SLIPNET_TYPE_DATA 1
#define APP_SLIPNET_TYPE_CMD 2

typedef struct app_slipnet_data_s
{
	ospl_uint8 type;
	ospl_uint16 len;
	ospl_uint8 data[APP_SLIPNET_QUEUE_SIZE];
} app_slipnet_data_t;

#pragma pack()



#ifdef V9_SLIPNET_UDP
#define V9_SLIPNET_UDPSRV_PORT 6666
#define V9_SLIPNET_UDPSRV_HOST "111.111.111.112"
#else
#define V9_SLIPNET_CTL_NAME	"/dev/ttyS1"
#define V9_SLIPNET_SPEED_RATE 115200
#endif

extern int v9_app_slipnet_init(v9_serial_t *serial, char *devname, ospl_uint32 speed);
extern int v9_app_slipnet_exit(v9_serial_t *serial);
#endif /* V9_SLIPNET_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* __V9_SLIPNET_H__ */
