/*
 * modem_serial.c
 *
 *  Created on: Jul 29, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_SERIAL_H__
#define __MODEM_SERIAL_H__

#include "modem.h"
//#include "modem_client.h"

#define MODEM_CHANNEL_DB_MAX	8
/*
 * serial profile <name>
 */
typedef struct modem_serial_s
{
	NODE				node;
	char				name[MODEM_STRING_MAX];

	BOOL				active;
	u_int8				hw_channel;	// 1-16
	void				*modem;
	void				*client;
	void				*driver;
	//void				*ifp;

}modem_serial_t;


typedef struct modem_serial_main_s
{
	LIST	*list;
	void	*mutex;

}modem_serial_main;


typedef int (*modem_serial_cb)(modem_serial_t *, void *);



extern int modem_serial_init(void);
extern int modem_serial_exit(void);


extern int modem_serial_add_api(const char *name);
extern modem_serial_t * modem_serial_lookup_api(const char *name, u_int8 hw_channel);
extern int modem_serial_del_api(const char *name);
extern int modem_serial_channel_api(const char *name, u_int8 hw_channel);

extern int modem_serial_callback_api(modem_serial_cb cb, void *pVoid);

extern int modem_serial_bind_api(const char *name, u_int8 hw_channel, void *client);
extern int modem_serial_unbind_api(const char *name, u_int8 hw_channel);



/*
extern int modem_serial_interface_bind_api(const char *name, void *ifp);
extern int modem_serial_interface_unbind_api(const char *name);
*/

extern const char *modem_serial_channel_name(modem_driver_t *input);


#endif /* __MODEM_SERIAL_H__ */
