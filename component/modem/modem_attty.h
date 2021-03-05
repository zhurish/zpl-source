/*
 * modem_attty.h
 *
 *  Created on: Jul 24, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_ATTTY_H__
#define __MODEM_ATTTY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os_list.h"

#include "modem.h"
#include "modem_client.h"

#define MODEM_TIMEOUT_MAX	(180)
#define MODEM_TIMEOUT_S(n)	(n)

//#define __MODEM_TTY_DEBUG


#define MD_NL			0X0A //\r
#define MD_CR			0X0D //\n
#define MD_CTRL_Z		26
#define MD_CTRL_KEY1	'"'
#define MD_SPACE		0X20
#define MD_SMS_KEY		0X3E	//'>'
#define MD_SMS_KEY		0X3E	//'>'
#define MD_ASCCI(n)		0x20 <= (n) && (n) <= 0x7e
#define MD_abcd(n)		'a' <= (n) && (n) <= 'z'

/*
typedef enum
{
	RES_OK  = 0,
	RES_ERROR = -1,
	RES_TIMEOUT  = -2,
	RES_CLOSE  = -3,
	RES_AGAIN  = -4,
}md_res_en;

*/

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
extern int modem_attty_isopen(modem_client_t *client);
extern int modem_attty_open(modem_client_t *client);
extern int modem_attty_isclose(modem_client_t *client);
extern int modem_attty_close(modem_client_t *client);

extern md_res_en modem_attty_write(modem_client_t *client, const char *format, ...);

extern md_res_en modem_attty(modem_client_t *client,
		ospl_uint32 timeout, const char *waitkey, const char *format, ...);

extern md_res_en modem_attty_respone(modem_client_t *client,
		ospl_uint32 timeout, char *buf, ospl_size_t size, const char *format, ...);


extern md_res_en modem_attty_proxy_respone(modem_client_t *client,
		ospl_uint32 timeout, char *buf, ospl_size_t size, const char *format, ospl_size_t len);

extern md_res_en modem_attty_massage_respone(modem_client_t *client,
		ospl_uint32 timeout, const char *msg_cmd, const char *buf, ospl_size_t size);

#ifdef __MODEM_TTY_DEBUG
#define MODEM_TTY_DEBUG(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define MODEM_TTY_WARN(fmt,...)		modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define MODEM_TTY_ERROR(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#else
#define MODEM_TTY_DEBUG(fmt,...)
#define MODEM_TTY_WARN(fmt,...)
#define MODEM_TTY_ERROR(fmt,...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MODEM_ATTTY_H__ */
