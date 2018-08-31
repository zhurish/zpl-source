/*
 * modem_attty.h
 *
 *  Created on: Jul 24, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_ATTTY_H__
#define __MODEM_ATTTY_H__

#include "os_list.h"

#include "modem.h"
#include "modem_client.h"

#define MODEM_TIMEOUT_MAX	(180)
#define MODEM_TIMEOUT_S(n)	(n)





/****************************************************************************/
/****************************************************************************/
extern int modem_attty_isopen(struct tty_com *attty);
extern int modem_attty_open(struct tty_com *attty);
extern int modem_attty_close(struct tty_com *attty);

extern md_res_en modem_attty(struct tty_com *attty,
		int timeout, const char *waitkey, const char *format, ...);

extern int modem_attty_respone(struct tty_com *attty,
		int timeout, char *buf, int size, const char *format, ...);

/****************************************************************************/
/****************************************************************************/
extern int modem_client_attty_isopen(modem_client_t *client);
extern int modem_client_attty_open(modem_client_t *client);
extern int modem_client_attty_close(modem_client_t *client);

extern md_res_en modem_client_attty(modem_client_t *client,
		int timeout, const char *waitkey, const char *format, ...);

extern int modem_client_attty_respone(modem_client_t *client,
		int timeout, char *buf, int size, const char *format, ...);


#endif /* __MODEM_ATTTY_H__ */
