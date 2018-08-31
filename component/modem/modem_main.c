/*
 * modem_main.c
 *
 *  Created on: Jul 28, 2018
 *      Author: zhurish
 */



#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "thread.h"
#include "os_util.h"
#include "tty_com.h"
#include "os_time.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_machine.h"
#include "modem_event.h"
#include "modem_error.h"
#include "modem_dhcp.h"
#include "modem_pppd.h"
#include "modem_attty.h"

#include "modem_dialog.h"
#include "modem_message.h"
#include "modem_product.h"
#include "modem_driver.h"
#include "modem_atcmd.h"
#include "modem_usim.h"
#include "modem_process.h"

static int modem_task_id = 0;

#ifdef __MODEM_DEBUG
int modem_start = 0;
#endif

/*************************************************************************/
/*
static int modem_main_handle(modem_t *modem, void *pVoid)
{
	assert(modem);
	modem_process_t *process = (modem_process_t *)pVoid;
	if(!process)
		return ERROR;
	if(modem->active && process->event)
	{
		modem_event	event = process->event;
		process->event = MODEM_EV_NONE;
		if(event)
		{
			modem_event_process(modem, event);
		}
		return OK;
	}
	return ERROR;
}
*/

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
static int modem_process_handle(modem_process_t *process, void *pVoid)
{
	assert(process);
	//return modem_main_callback_api(modem_main_handle, process);
	if(!process)
		return ERROR;
	//modem_process_t *inputprocess = (modem_process_t *)pVoid;
	assert(process->argv);
	modem_t	*modem = process->argv;


	if(modem->active && process->event)
	{
		modem_event	event = process->event;
		process->event = MODEM_EV_NONE;
		if(event)
		{
			if(gModemmain.mutex)
				os_mutex_lock(gModemmain.mutex, OS_WAIT_FOREVER);

			modem_event_process(modem, event);

			if(gModemmain.mutex)
				os_mutex_unlock(gModemmain.mutex);
		}
		return OK;
	}
	return ERROR;
}


int modem_main_process(void *pVoid)
{
	return modem_process_callback_api(modem_process_handle, pVoid);
}


static int modem_main_task(void *argv)
{
	os_sleep(5);
	while(1)
	{
		modem_product_detection();
#ifdef __MODEM_DEBUG
		if(modem_start)
#endif
			modem_main_process(NULL);
		modem_main_trywait(1);
	}
	return 0;
}

int modem_module_init ()
{
	/* Make master thread emulator. */
	master_thread[MODULE_MODEM] = thread_master_module_create (MODULE_MODEM);
	master_thread[MODULE_MODEM];
	modem_main_init();
	return 0;

}

int modem_task_init ()
{
	modem_task_id = os_task_create("modemTask", OS_TASK_DEFAULT_PRIORITY,
	               0, modem_main_task, NULL, OS_TASK_DEFAULT_STACK);
	if(modem_task_id)
		return OK;
	return ERROR;

}

int modem_module_exit ()
{
	return OK;
}
