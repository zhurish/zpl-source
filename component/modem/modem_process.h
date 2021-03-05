/*
 * modem_process.h
 *
 *  Created on: Jul 29, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_PROCESS_H__
#define __MODEM_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "modem.h"
#include "modem_event.h"


typedef struct modem_process_s
{
	NODE 		node;
//	int			ready;
	modem_event	event;
	void		*argv;

}modem_process_t;

typedef struct modem_main_process
{
	LIST	*ready;
	LIST	*list;
	LIST	*unlist;
	void	*mutex;

	modem_event	clean;
	void	*clean_argv;
}modem_main_process_t;




typedef int (*modem_process_cb)(modem_process_t *, void *);



extern int modem_process_init(void);
extern int modem_process_exit(void);
extern int modem_process_add_api(modem_event event, void *argv, ospl_bool lock);
extern int modem_process_del_api(modem_event event, void *argv, ospl_bool lock);

extern int modem_process_callback_api(modem_process_cb cb, void *pVoid);



#ifdef __cplusplus
}
#endif


#endif /* __MODEM_PROCESS_H__ */
